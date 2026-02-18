/**
 * @file keymap.h
 * @brief 日本語106キー配列マッピング API
 */

#ifndef KEYMAP_H
#define KEYMAP_H

#include <stdint.h>
#include <stdbool.h>

/* マトリクスサイズ */
#define MATRIX_ROWS    8
#define MATRIX_COLS    14

/* Modifier キーの内部エンコーディング (HID Usage 0xE0-0xE7) */
#define KC_LCTRL    0xE0
#define KC_LSHIFT   0xE1
#define KC_LALT     0xE2
#define KC_LGUI     0xE3
#define KC_RCTRL    0xE4
#define KC_RSHIFT   0xE5
#define KC_RALT     0xE6
#define KC_RGUI     0xE7

/* Modifier判定マクロ */
#define IS_MODIFIER(kc)    ((kc) >= 0xE0 && (kc) <= 0xE7)
#define MODIFIER_BIT(kc)   (1 << ((kc) - 0xE0))

/**
 * マトリクス位置からHIDキーコードを取得
 * @return HIDキーコード。空ポジションは KEY_NONE (0x00)
 */
uint8_t keymap_get_keycode(uint8_t row, uint8_t col);

/**
 * 指定位置がModifierキーかどうか判定
 */
bool keymap_is_modifier(uint8_t row, uint8_t col);

/**
 * Modifierキーのビットマスクを取得
 * @return Modifierビット。Modifierでなければ0
 */
uint8_t keymap_get_modifier_bit(uint8_t row, uint8_t col);

#endif /* KEYMAP_H */
