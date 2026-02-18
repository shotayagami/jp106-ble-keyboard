/**
 * @file keyboard_matrix.c
 * @brief キーマトリクス スキャン・デバウンス実装
 *
 * アクティブLOWスキャン方式:
 *   - 行ピンを1本ずつLOWに駆動
 *   - 列ピン(内部プルアップ)を読み取り
 *   - LOWなら押下、HIGHなら開放
 *
 * デバウンス: キー単位のタイマー方式 (DEBOUNCE_MS 間安定で確定)
 */

#include "keyboard_matrix.h"
#include "hardware/gpio.h"
#include "pico/time.h"
#include <string.h>

/* 行GPIOピン (アクティブLOW出力) */
static const uint8_t row_pins[MATRIX_ROWS] = {0, 1, 2, 3, 4, 5, 6, 7};

/* 列GPIOピン (プルアップ入力) */
static const uint8_t col_pins[MATRIX_COLS] = {
    8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21
};

/* 生スキャン結果 (現在スキャン) */
static bool raw_matrix[MATRIX_ROWS][MATRIX_COLS];

/* デバウンス済み状態 */
static bool debounced_matrix[MATRIX_ROWS][MATRIX_COLS];

/* キー毎のデバウンスタイマー (変化検出時刻, 0=非アクティブ) */
static uint32_t debounce_timer[MATRIX_ROWS][MATRIX_COLS];

/* 状態変化フラグ */
static bool state_changed;

/* 前回のBootレポート (変化検出用) */
static uint8_t prev_boot_report[BOOT_REPORT_SIZE];

/* 前回のNKROレポート (変化検出用) */
static uint8_t prev_nkro_report[NKRO_REPORT_SIZE];

void matrix_init(void) {
    /* 行ピンを出力に設定、初期状態HIGH (非アクティブ) */
    for (int r = 0; r < MATRIX_ROWS; r++) {
        gpio_init(row_pins[r]);
        gpio_set_dir(row_pins[r], GPIO_OUT);
        gpio_put(row_pins[r], 1);
    }

    /* 列ピンを入力に設定、内部プルアップ有効 */
    for (int c = 0; c < MATRIX_COLS; c++) {
        gpio_init(col_pins[c]);
        gpio_set_dir(col_pins[c], GPIO_IN);
        gpio_pull_up(col_pins[c]);
    }

    memset(raw_matrix, 0, sizeof(raw_matrix));
    memset(debounced_matrix, 0, sizeof(debounced_matrix));
    memset(debounce_timer, 0, sizeof(debounce_timer));
    memset(prev_boot_report, 0, sizeof(prev_boot_report));
    memset(prev_nkro_report, 0, sizeof(prev_nkro_report));
    state_changed = false;
}

void matrix_scan(void) {
    uint32_t now = to_ms_since_boot(get_absolute_time());

    for (int r = 0; r < MATRIX_ROWS; r++) {
        /* この行をLOWに駆動 */
        gpio_put(row_pins[r], 0);

        /* 信号安定待ち (10us) */
        sleep_us(10);

        /* 全列を読み取り */
        for (int c = 0; c < MATRIX_COLS; c++) {
            bool pressed = !gpio_get(col_pins[c]);  /* LOW=押下 */
            raw_matrix[r][c] = pressed;

            if (raw_matrix[r][c] != debounced_matrix[r][c]) {
                if (debounce_timer[r][c] == 0) {
                    /* デバウンスタイマー開始 */
                    debounce_timer[r][c] = now;
                } else if ((now - debounce_timer[r][c]) >= DEBOUNCE_MS) {
                    /* デバウンス期間経過 → 新状態を確定 */
                    debounced_matrix[r][c] = raw_matrix[r][c];
                    debounce_timer[r][c] = 0;
                    state_changed = true;
                }
            } else {
                /* 生値とデバウンス値が一致 → タイマーリセット */
                debounce_timer[r][c] = 0;
            }
        }

        /* 行をHIGHに復帰 (非アクティブ) */
        gpio_put(row_pins[r], 1);
    }
}

bool matrix_has_changed(void) {
    return state_changed;
}

void matrix_build_boot_report(uint8_t *report) {
    memset(report, 0, BOOT_REPORT_SIZE);

    bool fn_active = matrix_fn_is_pressed();
    uint8_t modifier_byte = 0;
    int keycode_index = 2;  /* report[2..7] = keycodes (最大6キー) */

    for (int r = 0; r < MATRIX_ROWS; r++) {
        for (int c = 0; c < MATRIX_COLS; c++) {
            if (!debounced_matrix[r][c]) continue;

            uint8_t kc = keymap_get_keycode(r, c);
            if (kc == KEY_NONE || kc == KEY_FN) continue;

            /* Fn押下中は 1/2/3 キーをレポートに含めない (スロット切替に使用) */
            if (fn_active && (kc == KEY_1 || kc == KEY_2 || kc == KEY_3)) continue;

            if (IS_MODIFIER(kc)) {
                modifier_byte |= MODIFIER_BIT(kc);
            } else if (keycode_index < BOOT_REPORT_SIZE) {
                report[keycode_index++] = kc;
            }
            /* 6キー超は無視 (6KRO制限) */
        }
    }

    report[0] = modifier_byte;
    report[1] = 0x00;  /* Reserved */

    /* 変化検出 */
    if (memcmp(report, prev_boot_report, BOOT_REPORT_SIZE) != 0) {
        memcpy(prev_boot_report, report, BOOT_REPORT_SIZE);
        state_changed = true;
    } else {
        state_changed = false;
    }
}

void matrix_build_nkro_report(uint8_t *report) {
    memset(report, 0, NKRO_REPORT_SIZE);

    bool fn_active = matrix_fn_is_pressed();
    uint8_t modifier_byte = 0;

    for (int r = 0; r < MATRIX_ROWS; r++) {
        for (int c = 0; c < MATRIX_COLS; c++) {
            if (!debounced_matrix[r][c]) continue;

            uint8_t kc = keymap_get_keycode(r, c);
            if (kc == KEY_NONE || kc == KEY_FN) continue;

            /* Fn押下中は 1/2/3 キーをレポートに含めない */
            if (fn_active && (kc == KEY_1 || kc == KEY_2 || kc == KEY_3)) continue;

            if (IS_MODIFIER(kc)) {
                modifier_byte |= MODIFIER_BIT(kc);
            } else {
                /* ビットマップ: report[1 + kc/8] の bit (kc%8) をセット */
                uint8_t byte_index = 1 + (kc / 8);
                uint8_t bit_index = kc % 8;
                if (byte_index < NKRO_REPORT_SIZE) {
                    report[byte_index] |= (1 << bit_index);
                }
            }
        }
    }

    report[0] = modifier_byte;

    /* 変化検出 */
    if (memcmp(report, prev_nkro_report, NKRO_REPORT_SIZE) != 0) {
        memcpy(prev_nkro_report, report, NKRO_REPORT_SIZE);
        state_changed = true;
    } else {
        state_changed = false;
    }
}

bool matrix_key_is_pressed(uint8_t row, uint8_t col) {
    if (row >= MATRIX_ROWS || col >= MATRIX_COLS) return false;
    return debounced_matrix[row][col];
}

bool matrix_fn_is_pressed(void) {
    /* Fnキーの位置をマトリクス全体から検索 */
    for (int r = 0; r < MATRIX_ROWS; r++) {
        for (int c = 0; c < MATRIX_COLS; c++) {
            if (keymap_get_keycode(r, c) == KEY_FN && debounced_matrix[r][c]) {
                return true;
            }
        }
    }
    return false;
}

int8_t matrix_get_fn_slot_action(void) {
    if (!matrix_fn_is_pressed()) return -1;

    /* Fn + 1/2/3 でスロット切替 */
    for (int r = 0; r < MATRIX_ROWS; r++) {
        for (int c = 0; c < MATRIX_COLS; c++) {
            if (!debounced_matrix[r][c]) continue;
            uint8_t kc = keymap_get_keycode(r, c);
            if (kc == KEY_1) return 0;
            if (kc == KEY_2) return 1;
            if (kc == KEY_3) return 2;
        }
    }
    return -1;
}
