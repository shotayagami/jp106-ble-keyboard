/**
 * @file keyboard_matrix.h
 * @brief キーマトリクス スキャン・デバウンス API
 *
 * 8行×14列マトリクスのGPIOスキャンと、キー単位のデバウンス処理。
 * Boot Protocol (6KRO) と NKRO ビットマップ両方のレポート生成に対応。
 */

#ifndef KEYBOARD_MATRIX_H
#define KEYBOARD_MATRIX_H

#include <stdint.h>
#include <stdbool.h>
#include "keymap.h"
#include "hid_keycodes.h"

/* デバウンス時間 (ミリ秒) */
#define DEBOUNCE_MS  20

/**
 * マトリクスGPIOピンを初期化
 * 行ピン: GP0-GP7 (OUTPUT, HIGH)
 * 列ピン: GP8-GP21 (INPUT, プルアップ)
 */
void matrix_init(void);

/**
 * マトリクス全体を1回スキャン
 * 内部のデバウンス状態を更新する
 */
void matrix_scan(void);

/**
 * 前回のレポート生成以降にマトリクス状態が変化したか
 */
bool matrix_has_changed(void);

/**
 * Boot Protocol用 HIDレポート (8バイト, 6KRO) を生成
 * @param report 8バイトバッファ [modifier, reserved, key1..key6]
 */
void matrix_build_boot_report(uint8_t *report);

/**
 * NKRO用 HIDレポート (22バイト, ビットマップ) を生成
 * @param report 22バイトバッファ [modifier, bitmap[21]]
 */
void matrix_build_nkro_report(uint8_t *report);

/**
 * 特定キーの押下状態を取得 (デバウンス済み)
 */
bool matrix_key_is_pressed(uint8_t row, uint8_t col);

/**
 * Fnキーが現在押されているか
 */
bool matrix_fn_is_pressed(void);

/**
 * Fnレイヤーのアクション取得
 * Fn+数字キーでデバイススロット切替を検出。
 * @return 切替先スロット番号 (0-2)。切替なしなら -1。
 */
int8_t matrix_get_fn_slot_action(void);

#endif /* KEYBOARD_MATRIX_H */
