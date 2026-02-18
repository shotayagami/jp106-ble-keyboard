/**
 * @file trackball.h
 * @brief I2C トラックボールドライバ API (Pimoroni Trackball Breakout 互換)
 *
 * Pimoroni Trackball Breakout (PIM447) を I2C1 で読み取り、
 * マウスデルタ (X, Y) とボタン状態を返す。
 *
 * I2Cピン:
 *   SDA = GP26 (I2C1)
 *   SCL = GP27 (I2C1)
 *
 * I2Cアドレス: 0x0A (デフォルト)
 */

#ifndef TRACKBALL_H
#define TRACKBALL_H

#include <stdint.h>
#include <stdbool.h>

/* I2C設定 */
#define TRACKBALL_I2C         i2c1
#define TRACKBALL_SDA_PIN     26
#define TRACKBALL_SCL_PIN     27
#define TRACKBALL_I2C_ADDR    0x0A
#define TRACKBALL_I2C_FREQ    100000  /* 100kHz */

/* Pimoroni Trackball レジスタ */
#define TRACKBALL_REG_LED_RED     0x00
#define TRACKBALL_REG_LED_GRN     0x01
#define TRACKBALL_REG_LED_BLU     0x02
#define TRACKBALL_REG_LED_WHT     0x03
#define TRACKBALL_REG_LEFT        0x04
#define TRACKBALL_REG_RIGHT       0x05
#define TRACKBALL_REG_UP          0x06
#define TRACKBALL_REG_DOWN        0x07
#define TRACKBALL_REG_SWITCH      0x08

/* トラックボール状態 */
typedef struct {
    int8_t  delta_x;    /* X移動量 (右が正) */
    int8_t  delta_y;    /* Y移動量 (下が正) */
    bool    button;     /* ボタン押下状態 */
    bool    changed;    /* 前回読み取りから変化あり */
} trackball_state_t;

/**
 * トラックボール初期化 (I2C設定 + デバイス検出)
 * @return true: 初期化成功 (デバイス検出), false: 未接続
 */
bool trackball_init(void);

/**
 * トラックボール状態を読み取り
 * I2Cからデルタとボタン状態を取得。
 * デルタは前回読み取りからの累積移動量。
 */
void trackball_read(trackball_state_t *state);

/**
 * トラックボールLEDを設定
 * @param r 赤 (0-255)
 * @param g 緑 (0-255)
 * @param b 青 (0-255)
 * @param w 白 (0-255)
 */
void trackball_set_led(uint8_t r, uint8_t g, uint8_t b, uint8_t w);

/**
 * トラックボールが接続されているか
 */
bool trackball_is_connected(void);

#endif /* TRACKBALL_H */
