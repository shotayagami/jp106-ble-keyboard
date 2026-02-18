/**
 * @file ws2812_led.c
 * @brief WS2812B (NeoPixel) LED ドライバ実装
 *
 * PIO ステートマシンを使って WS2812B の 800kHz プロトコルを生成。
 * 3個のLEDをデイジーチェーン接続 (GP22 → LED0 → LED1 → LED2)。
 */

#include "ws2812_led.h"

#include <string.h>
#include "hardware/pio.h"
#include "ws2812.pio.h"  /* ビルド時に ws2812.pio から自動生成 */

/* PIOインスタンスとステートマシン */
static PIO pio_instance;
static uint pio_sm;

/* ピクセルバッファ (GRB順) */
static uint32_t pixel_buf[WS2812_NUM_LEDS];

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    /* WS2812B: GRB順、MSBファースト → 左シフトで上位ビットに配置 */
    return ((uint32_t)g << 24) | ((uint32_t)r << 16) | ((uint32_t)b << 8);
}

void ws2812_init(void) {
    pio_instance = pio0;
    pio_sm = pio_claim_unused_sm(pio_instance, true);

    uint offset = pio_add_program(pio_instance, &ws2812_program);
    ws2812_program_init(pio_instance, pio_sm, offset,
                        WS2812_PIN, WS2812_FREQ);

    memset(pixel_buf, 0, sizeof(pixel_buf));
    ws2812_show();
}

void ws2812_set_pixel(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index >= WS2812_NUM_LEDS) return;
    pixel_buf[index] = urgb_u32(r, g, b);
}

void ws2812_clear(void) {
    memset(pixel_buf, 0, sizeof(pixel_buf));
}

void ws2812_show(void) {
    for (int i = 0; i < WS2812_NUM_LEDS; i++) {
        pio_sm_put_blocking(pio_instance, pio_sm, pixel_buf[i]);
    }
}
