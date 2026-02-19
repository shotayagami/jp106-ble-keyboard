/**
 * @file project_config.h
 * @brief プロジェクト設定ファイル - JP106 BLE キーボード
 */

#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H

/* ============================================================
 * バッテリー監視GPIO
 * ============================================================ */
#define BATTERY_ADC_PIN      28   /* バッテリー電圧監視 (ADC2) */
#define BATTERY_ADC_CHANNEL  2

/* バッテリー監視間隔 (ミリ秒) */
#define BATTERY_CHECK_INTERVAL_MS  60000

/* ============================================================
 * トラックボール設定
 * ============================================================ */
#define TRACKBALL_POLL_INTERVAL_US  1000  /* ポーリング間隔 (1ms) */
#define TRACKBALL_SENSITIVITY       2     /* 感度倍率 (1-4) */

/* ============================================================
 * デバッグ設定
 * ============================================================ */
#define DEBUG_ENABLED 1

#if DEBUG_ENABLED
    #define DEBUG_PRINT(fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
    #define DEBUG_PRINT(fmt, ...)
#endif

#endif /* PROJECT_CONFIG_H */
