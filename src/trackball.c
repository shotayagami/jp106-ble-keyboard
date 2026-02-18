/**
 * @file trackball.c
 * @brief I2C トラックボールドライバ実装
 *
 * Pimoroni Trackball Breakout (PIM447) からI2Cでデルタ読み取り。
 *
 * レジスタ読み取り (5バイト):
 *   [0x04] left  - 左方向移動カウント
 *   [0x05] right - 右方向移動カウント
 *   [0x06] up    - 上方向移動カウント
 *   [0x07] down  - 下方向移動カウント
 *   [0x08] switch - ボタン状態 (0=OFF, 128=ON)
 *
 * デルタ算出:
 *   delta_x = right - left
 *   delta_y = down - up
 */

#include "trackball.h"
#include "project_config.h"

#include "hardware/i2c.h"
#include "hardware/gpio.h"

static bool connected = false;

bool trackball_init(void) {
    /* I2C1 初期化 */
    i2c_init(TRACKBALL_I2C, TRACKBALL_I2C_FREQ);

    /* GPIOをI2C機能に設定 */
    gpio_set_function(TRACKBALL_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(TRACKBALL_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(TRACKBALL_SDA_PIN);
    gpio_pull_up(TRACKBALL_SCL_PIN);

    /* デバイス検出: レジスタ0x00を読み取れるか */
    uint8_t reg = TRACKBALL_REG_LED_RED;
    uint8_t dummy;
    int ret = i2c_write_blocking(TRACKBALL_I2C, TRACKBALL_I2C_ADDR,
                                  &reg, 1, true);
    if (ret < 0) {
        DEBUG_PRINT("Trackball: not detected on I2C (addr=0x%02X)", TRACKBALL_I2C_ADDR);
        connected = false;
        return false;
    }

    ret = i2c_read_blocking(TRACKBALL_I2C, TRACKBALL_I2C_ADDR,
                             &dummy, 1, false);
    if (ret < 0) {
        connected = false;
        return false;
    }

    connected = true;
    DEBUG_PRINT("Trackball: detected on I2C (addr=0x%02X)", TRACKBALL_I2C_ADDR);

    /* 初期LED設定 (消灯) */
    trackball_set_led(0, 0, 0, 0);

    return true;
}

void trackball_read(trackball_state_t *state) {
    state->delta_x = 0;
    state->delta_y = 0;
    state->button = false;
    state->changed = false;

    if (!connected) return;

    /* レジスタ0x04から5バイト連続読み取り */
    uint8_t reg = TRACKBALL_REG_LEFT;
    int ret = i2c_write_blocking(TRACKBALL_I2C, TRACKBALL_I2C_ADDR,
                                  &reg, 1, true);
    if (ret < 0) return;

    uint8_t buf[5];
    ret = i2c_read_blocking(TRACKBALL_I2C, TRACKBALL_I2C_ADDR,
                             buf, 5, false);
    if (ret < 0) return;

    uint8_t left  = buf[0];
    uint8_t right = buf[1];
    uint8_t up    = buf[2];
    uint8_t down  = buf[3];
    uint8_t sw    = buf[4];

    /* デルタ算出 (int16_tで計算してint8_tにクランプ) */
    int16_t dx = (int16_t)right - (int16_t)left;
    int16_t dy = (int16_t)down - (int16_t)up;

    if (dx > 127)  dx = 127;
    if (dx < -127) dx = -127;
    if (dy > 127)  dy = 127;
    if (dy < -127) dy = -127;

    state->delta_x = (int8_t)dx;
    state->delta_y = (int8_t)dy;
    state->button = (sw >= 128);
    state->changed = (dx != 0 || dy != 0 || state->button);
}

void trackball_set_led(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    if (!connected) return;

    uint8_t buf[5] = { TRACKBALL_REG_LED_RED, r, g, b, w };
    i2c_write_blocking(TRACKBALL_I2C, TRACKBALL_I2C_ADDR,
                        buf, 5, false);
}

bool trackball_is_connected(void) {
    return connected;
}
