/**
 * @file main.c
 * @brief JP106 BLE キーボード+トラックボール - メインプログラム
 *
 * メインループ:
 *   1. BLEイベントポーリング
 *   2. マトリクススキャン
 *   3. Fnレイヤー処理 (デバイススロット切替: Fn+1/2/3)
 *   4. キーボードHIDレポート送信
 *   5. トラックボール読み取り + マウスレポート送信
 *   6. バッテリー監視
 *   7. LED更新
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"

#include "project_config.h"
#include "keyboard_matrix.h"
#include "hid_keycodes.h"
#include "ble_hid.h"
#include "device_slot.h"
#include "trackball.h"

/**
 * バッテリーレベル読み取り (GP28/ADC2, 分圧回路経由)
 */
static uint8_t read_battery_level(void) {
    adc_select_input(BATTERY_ADC_CHANNEL);
    uint16_t raw = adc_read();
    float vbat = (float)raw * 6.6f / 4095.0f;
    if (vbat >= 4.2f) return 100;
    if (vbat <= 3.0f) return 0;
    return (uint8_t)((vbat - 3.0f) / 1.2f * 100.0f);
}

int main(void) {
    stdio_init_all();

    /* ADC初期化 (バッテリー監視) */
    adc_init();
    adc_gpio_init(BATTERY_ADC_PIN);

    /* マトリクスGPIO初期化 */
    matrix_init();

    /* デバイススロット初期化 (Flash読込 + WS2812B LED初期化) */
    device_slot_init();

    /* トラックボール初期化 (I2C, オプショナル) */
    bool trackball_available = trackball_init();
    if (trackball_available) {
        /* トラックボールLED: 控えめな白色点灯 */
        trackball_set_led(0, 0, 0, 16);
    }

    /* BLE HID 初期化 (アドバタイジング開始) */
    ble_hid_init();

    /* 起動表示: オンボードLED + スロットLED */
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(200);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    device_slot_blink_led(device_slot_get_active(),
                          device_slot_get_active() + 1);

    DEBUG_PRINT("JP106 BLE Keyboard started (slot %d, trackball=%s)",
                device_slot_get_active(),
                trackball_available ? "yes" : "no");

    uint8_t hid_report[NKRO_REPORT_SIZE];
    uint32_t last_battery_check = 0;
    int8_t prev_fn_slot = -1;  /* Fn+数字の重複実行防止 */
    trackball_state_t tb_state;

    /* ============================================================
     * メインループ
     * ============================================================ */
    while (true) {
        /* 1. BLEイベント処理 (CYW43ポーリング) */
        ble_hid_poll();

        /* 2. マトリクススキャン */
        matrix_scan();

        /* 3. Fnレイヤー: デバイススロット切替 (Fn+1/2/3) */
        int8_t fn_slot = matrix_get_fn_slot_action();
        if (fn_slot >= 0 && fn_slot != prev_fn_slot) {
            uint8_t current = device_slot_get_active();
            if ((uint8_t)fn_slot != current) {
                DEBUG_PRINT("Slot switch: %d -> %d", current, fn_slot);
                ble_hid_disconnect_and_readvertise();
                device_slot_switch(fn_slot);
                device_slot_blink_led(fn_slot, fn_slot + 1);
            }
        }
        prev_fn_slot = fn_slot;

        /* 4. キーボードHIDレポート送信 (Fn押下中はキー入力を抑制) */
        if (matrix_has_changed() && !matrix_fn_is_pressed()) {
            if (ble_hid_is_connected()) {
                if (ble_hid_get_protocol_mode() == 0) {
                    uint8_t boot_report[BOOT_REPORT_SIZE];
                    matrix_build_boot_report(boot_report);
                    ble_hid_send_report(boot_report, BOOT_REPORT_SIZE);
                } else {
                    matrix_build_nkro_report(hid_report);
                    ble_hid_send_report(hid_report, NKRO_REPORT_SIZE);
                }
            } else {
                /* 未接続時: デバッグ出力 */
                uint8_t debug_report[BOOT_REPORT_SIZE];
                matrix_build_boot_report(debug_report);
                if (debug_report[0] != 0 || debug_report[2] != 0) {
                    DEBUG_PRINT("Key: mod=0x%02X keys=[0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X]",
                                debug_report[0],
                                debug_report[2], debug_report[3], debug_report[4],
                                debug_report[5], debug_report[6], debug_report[7]);
                }
            }
        }

        /* 5. トラックボール読み取り + マウスレポート送信 */
        if (trackball_available) {
            trackball_read(&tb_state);
            if (tb_state.changed && ble_hid_is_connected()) {
                uint8_t buttons = tb_state.button ? MOUSE_BTN_LEFT : 0;
                /* int16_tで計算してint8_t範囲にクランプ (オーバーフロー防止) */
                int16_t dx_raw = (int16_t)tb_state.delta_x * TRACKBALL_SENSITIVITY;
                int16_t dy_raw = (int16_t)tb_state.delta_y * TRACKBALL_SENSITIVITY;
                if (dx_raw > 127) dx_raw = 127;
                if (dx_raw < -127) dx_raw = -127;
                if (dy_raw > 127) dy_raw = 127;
                if (dy_raw < -127) dy_raw = -127;
                ble_hid_send_mouse_report(buttons, (int8_t)dx_raw, (int8_t)dy_raw, 0);
            }
        }

        /* 6. バッテリーレベル定期更新 */
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if ((now - last_battery_check) >= BATTERY_CHECK_INTERVAL_MS) {
            last_battery_check = now;
            uint8_t level = read_battery_level();
            ble_hid_update_battery(level);
        }

        /* 7. オンボードLED (BLE接続状態) */
        if (ble_hid_is_connected()) {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        } else {
            bool led_state = ((now / 500) % 2) == 0;
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_state);
        }

        /* 8. スキャンレート制御 (~1kHz) */
        sleep_us(500);
    }

    return 0;
}
