/**
 * @file btstack_config.h
 * @brief BTstack コンパイル時設定
 *
 * BTstackが名前で検索するため、このファイル名は変更不可。
 * BLE HID キーボード用のミニマル設定。
 */

#ifndef BTSTACK_CONFIG_H
#define BTSTACK_CONFIG_H

/* ============================================================
 * ログ設定
 * ============================================================ */
#define ENABLE_LOG_ERROR
#if defined(DEBUG_ENABLED) && DEBUG_ENABLED
#define ENABLE_LOG_INFO
#endif
#define ENABLE_PRINTF_HEXDUMP

/* ============================================================
 * BLE 機能
 * ============================================================ */
#define ENABLE_BLE
#define ENABLE_LE_PERIPHERAL
#define ENABLE_LE_SECURE_CONNECTIONS
#define ENABLE_L2CAP_LE_CREDIT_BASED_FLOW_CONTROL_MODE

/* ============================================================
 * 暗号化
 * ============================================================ */
#define ENABLE_SOFTWARE_AES128
#define ENABLE_MICRO_ECC_FOR_LE_SECURE_CONNECTIONS

/* ============================================================
 * 接続パラメータ
 * ============================================================ */
#define MAX_NR_HCI_CONNECTIONS           1    /* キーボードは1台のみ接続 */
#define MAX_NR_SM_LOOKUP_ENTRIES         3    /* ボンディングデバイス数 */
#define MAX_NR_L2CAP_CHANNELS            4
#define MAX_ATT_DB_SIZE                  512

/* ============================================================
 * CYW43439 フロー制御
 * ============================================================ */
#define ENABLE_HCI_CONTROLLER_TO_HOST_FLOW_CONTROL
#define MAX_NR_CONTROLLER_ACL_BUFFERS    3

/* ============================================================
 * タイミング
 * ============================================================ */
#define HAVE_EMBEDDED_TIME_MS

/* ============================================================
 * ATT MTU (NKRO 22バイトレポートに対応)
 * ============================================================ */
#define MAX_ATT_MTU                      64

#endif /* BTSTACK_CONFIG_H */
