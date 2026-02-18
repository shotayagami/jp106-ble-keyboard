/**
 * @file device_slot.h
 * @brief デバイススロット管理 API (3台切替)
 *
 * 最大3台のBLEデバイスのボンディング情報をFlashに保存し、
 * Fn+1/2/3 でスロットを切り替える。
 *
 * 各スロットは接続先デバイスのBDアドレスとアドレスタイプを保持。
 * 空スロットは新規ペアリングモードでアドバタイジング。
 * 使用済みスロットはDirected Advertisingで高速再接続。
 *
 * スロットLED: WS2812B (NeoPixel) ×3 on GP22
 *   GP26/GP27 はトラックボール I2C に使用するため、
 *   直接GPIO LEDから WS2812B チェーンに変更。
 */

#ifndef DEVICE_SLOT_H
#define DEVICE_SLOT_H

#include <stdint.h>
#include <stdbool.h>
#include "hid_keycodes.h"

/* BDアドレス長 */
#define BD_ADDR_LEN  6

/**
 * スロット情報
 */
typedef struct {
    uint8_t  bd_addr[BD_ADDR_LEN]; /* 接続先BDアドレス */
    uint8_t  addr_type;            /* アドレスタイプ (0=public, 1=random) */
    bool     paired;               /* ペアリング済みフラグ */
} device_slot_info_t;

/**
 * デバイススロット初期化
 * Flashからスロット情報を読み込み、WS2812B LEDを初期化。
 */
void device_slot_init(void);

/**
 * 現在のアクティブスロット番号を取得
 * @return 0, 1, or 2
 */
uint8_t device_slot_get_active(void);

/**
 * アクティブスロットを切替
 * @param slot 切替先スロット番号 (0-2)
 * @return true: 切替成功, false: 無効なスロット番号
 */
bool device_slot_switch(uint8_t slot);

/**
 * 現在のスロットにペアリング情報を保存
 * BLEペアリング完了時に呼ぶ。Flashに永続化。
 */
void device_slot_save_pairing(const uint8_t *bd_addr, uint8_t addr_type);

/**
 * 現在のスロットのペアリング情報を取得
 * @return スロット情報。paired==false なら未ペアリング。
 */
const device_slot_info_t *device_slot_get_info(uint8_t slot);

/**
 * 現在のスロットのペアリングを解除 (Fn+長押し等で使用)
 */
void device_slot_clear_current(void);

/**
 * スロットLEDを更新 (WS2812B)
 * アクティブスロットのLEDを点灯、他を消灯。
 * スロット0=緑, スロット1=青, スロット2=赤。
 */
void device_slot_update_leds(void);

/**
 * スロットLEDを点滅表示 (切替直後のフィードバック)
 * @param slot 点滅するスロット番号
 * @param blink_count 点滅回数 (1-3)
 */
void device_slot_blink_led(uint8_t slot, uint8_t blink_count);

#endif /* DEVICE_SLOT_H */
