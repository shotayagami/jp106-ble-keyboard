/**
 * @file ble_hid.h
 * @brief BLE HID サービス API (コンポジットデバイス)
 *
 * BTstack の HOG (HID over GATT) プロファイルを使用。
 * キーボード + マウス のコンポジットHIDデバイス。
 *
 * キーボード (Report ID 1):
 *   Boot Protocol (6KRO) と Report Protocol (NKRO) のデュアル対応。
 *
 * マウス (Report ID 2):
 *   Report Protocol のみ。トラックボール入力をマウスとして送信。
 */

#ifndef BLE_HID_H
#define BLE_HID_H

#include <stdint.h>
#include <stdbool.h>

/**
 * BLEスタック初期化、GATTサービス登録、アドバタイジング開始
 * cyw43_arch_init() を含む。失敗時は内部でエラー処理。
 */
void ble_hid_init(void);

/**
 * キーボードHIDレポートを送信
 * 接続中かつ送信可能な場合のみ送信。
 * それ以外はバッファリングして CAN_SEND_NOW で送信。
 *
 * Boot Protocol: report = 8バイト標準フォーマット (Report IDなし)
 * Report Protocol: report = 22バイトNKRO (内部でReport ID 1を付与)
 *
 * @param report レポートデータ
 * @param len    レポート長 (Boot: 8, NKRO: 22)
 */
void ble_hid_send_report(const uint8_t *report, uint8_t len);

/**
 * マウスHIDレポートを送信
 * Report Protocol モードでのみ動作。Boot Protocolでは無視。
 *
 * @param buttons ボタンビットマスク (MOUSE_BTN_LEFT/RIGHT/MIDDLE)
 * @param delta_x X移動量 (-127 to 127)
 * @param delta_y Y移動量 (-127 to 127)
 * @param wheel   ホイール移動量 (-127 to 127)
 */
void ble_hid_send_mouse_report(uint8_t buttons, int8_t delta_x,
                                int8_t delta_y, int8_t wheel);

/**
 * BLE接続中かどうか
 */
bool ble_hid_is_connected(void);

/**
 * 現在のプロトコルモードを取得
 * @return 0=Boot Protocol, 1=Report Protocol
 */
uint8_t ble_hid_get_protocol_mode(void);

/**
 * BLEイベントをポーリング処理 (メインループから毎回呼ぶ)
 */
void ble_hid_poll(void);

/**
 * 全キー解放レポートを送信
 */
void ble_hid_send_key_release(void);

/**
 * バッテリーレベル更新
 * @param level バッテリー残量 (0-100)
 */
void ble_hid_update_battery(uint8_t level);

/**
 * 現在の接続を切断し、再アドバタイジングを開始
 * デバイススロット切替時に使用。
 */
void ble_hid_disconnect_and_readvertise(void);

#endif /* BLE_HID_H */
