/**
 * @file ws2812_led.h
 * @brief WS2812B (NeoPixel) LED ドライバ API
 *
 * PIO を使用して WS2812B LEDチェーンを駆動。
 * スロット表示用に3個のLEDをGP22で制御。
 */

#ifndef WS2812_LED_H
#define WS2812_LED_H

#include <stdint.h>

/* WS2812B チェーン設定 */
#define WS2812_PIN       22   /* GP22: データ出力 */
#define WS2812_NUM_LEDS  3    /* スロット表示用3個 */
#define WS2812_FREQ      800000  /* 800kHz */

/**
 * WS2812B ドライバ初期化 (PIOセットアップ)
 */
void ws2812_init(void);

/**
 * 指定LEDの色を設定 (バッファのみ、送信はshow()で)
 * @param index LED番号 (0-2)
 * @param r 赤 (0-255)
 * @param g 緑 (0-255)
 * @param b 青 (0-255)
 */
void ws2812_set_pixel(uint8_t index, uint8_t r, uint8_t g, uint8_t b);

/**
 * 全LEDを消灯に設定 (バッファのみ)
 */
void ws2812_clear(void);

/**
 * バッファの色データをLEDチェーンに送信
 */
void ws2812_show(void);

#endif /* WS2812_LED_H */
