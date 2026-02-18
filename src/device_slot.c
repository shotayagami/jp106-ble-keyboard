/**
 * @file device_slot.c
 * @brief デバイススロット管理実装
 *
 * Flash最終4KBセクタにスロット情報を永続化。
 * 書込みはerase-write方式 (Flashセクタ単位)。
 *
 * スロットLED: WS2812B (NeoPixel) ×3 on GP22
 *   スロット0=緑, スロット1=青, スロット2=赤
 *
 * Flash レイアウト (最終セクタ 4KB):
 *   offset 0x000: magic (4 bytes) "SLOT"
 *   offset 0x004: active_slot (1 byte)
 *   offset 0x005: reserved (3 bytes)
 *   offset 0x008: slot[0] (8 bytes: bd_addr[6] + addr_type + paired)
 *   offset 0x010: slot[1] (8 bytes)
 *   offset 0x018: slot[2] (8 bytes)
 */

#include "device_slot.h"
#include "project_config.h"
#include "ws2812_led.h"

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

/* ============================================================
 * Flash ストレージ定数
 * ============================================================ */
/* Pico 2W: 4MB flash, 最終セクタを使用 */
#define FLASH_TOTAL_SIZE     (4 * 1024 * 1024)
#define FLASH_SLOT_OFFSET    (FLASH_TOTAL_SIZE - FLASH_SECTOR_SIZE)
#define FLASH_SLOT_MAGIC     0x534C4F54  /* "SLOT" in little-endian */

/* Flash上のスロットデータ構造 */
typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint8_t  active_slot;
    uint8_t  reserved[3];
    struct __attribute__((packed)) {
        uint8_t bd_addr[BD_ADDR_LEN];
        uint8_t addr_type;
        uint8_t paired;
    } slots[MAX_DEVICE_SLOTS];
} flash_slot_data_t;

/* ============================================================
 * ランタイム状態
 * ============================================================ */
static device_slot_info_t slots[MAX_DEVICE_SLOTS];
static uint8_t active_slot = 0;

/* スロット別LED色 (R, G, B) - 輝度を抑えめに設定 */
static const uint8_t slot_colors[MAX_DEVICE_SLOTS][3] = {
    {  0, 32,  0 },   /* スロット0: 緑 */
    {  0,  0, 32 },   /* スロット1: 青 */
    { 32,  0,  0 },   /* スロット2: 赤 */
};

/* ============================================================
 * Flash 読み書き
 * ============================================================ */

/* FlashからXIPアドレスへの変換 */
static const flash_slot_data_t *flash_read_ptr(void) {
    return (const flash_slot_data_t *)(XIP_BASE + FLASH_SLOT_OFFSET);
}

static void flash_save_slots(void) {
    flash_slot_data_t data;
    memset(&data, 0xFF, sizeof(data));  /* Flash消去値 */

    data.magic = FLASH_SLOT_MAGIC;
    data.active_slot = active_slot;

    for (int i = 0; i < MAX_DEVICE_SLOTS; i++) {
        memcpy(data.slots[i].bd_addr, slots[i].bd_addr, BD_ADDR_LEN);
        data.slots[i].addr_type = slots[i].addr_type;
        data.slots[i].paired = slots[i].paired ? 1 : 0;
    }

    /* Flash書込み: 割り込み無効化 → セクタ消去 → 書込み → 割り込み復帰 */
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FLASH_SLOT_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_SLOT_OFFSET, (const uint8_t *)&data,
                        sizeof(flash_slot_data_t));
    restore_interrupts(ints);

    DEBUG_PRINT("Flash: slots saved (active=%d)", active_slot);
}

static void flash_load_slots(void) {
    const flash_slot_data_t *data = flash_read_ptr();

    if (data->magic != FLASH_SLOT_MAGIC) {
        /* 初回起動: デフォルト値 */
        DEBUG_PRINT("Flash: no slot data, initializing defaults");
        memset(slots, 0, sizeof(slots));
        active_slot = 0;
        return;
    }

    active_slot = data->active_slot;
    if (active_slot >= MAX_DEVICE_SLOTS) active_slot = 0;

    for (int i = 0; i < MAX_DEVICE_SLOTS; i++) {
        memcpy(slots[i].bd_addr, data->slots[i].bd_addr, BD_ADDR_LEN);
        slots[i].addr_type = data->slots[i].addr_type;
        slots[i].paired = (data->slots[i].paired == 1);
    }

    DEBUG_PRINT("Flash: slots loaded (active=%d)", active_slot);
}

/* ============================================================
 * Public API
 * ============================================================ */

void device_slot_init(void) {
    /* WS2812B LED 初期化 */
    ws2812_init();

    /* Flashからスロット情報読込 */
    flash_load_slots();

    /* アクティブスロットのLEDを点灯 */
    device_slot_update_leds();
}

uint8_t device_slot_get_active(void) {
    return active_slot;
}

bool device_slot_switch(uint8_t slot) {
    if (slot >= MAX_DEVICE_SLOTS) return false;
    if (slot == active_slot) return true;  /* 既にアクティブ */

    active_slot = slot;
    device_slot_update_leds();
    flash_save_slots();

    DEBUG_PRINT("Slot switched to %d (paired=%s)",
                slot, slots[slot].paired ? "yes" : "no");
    return true;
}

void device_slot_save_pairing(const uint8_t *bd_addr, uint8_t addr_type) {
    memcpy(slots[active_slot].bd_addr, bd_addr, BD_ADDR_LEN);
    slots[active_slot].addr_type = addr_type;
    slots[active_slot].paired = true;
    flash_save_slots();

    DEBUG_PRINT("Slot %d: pairing saved (addr=%02X:%02X:%02X:%02X:%02X:%02X)",
                active_slot,
                bd_addr[0], bd_addr[1], bd_addr[2],
                bd_addr[3], bd_addr[4], bd_addr[5]);
}

const device_slot_info_t *device_slot_get_info(uint8_t slot) {
    if (slot >= MAX_DEVICE_SLOTS) return NULL;
    return &slots[slot];
}

void device_slot_clear_current(void) {
    memset(&slots[active_slot], 0, sizeof(device_slot_info_t));
    flash_save_slots();
    DEBUG_PRINT("Slot %d: pairing cleared", active_slot);
}

void device_slot_update_leds(void) {
    ws2812_clear();
    /* アクティブスロットのLEDだけ点灯 */
    ws2812_set_pixel(active_slot,
                     slot_colors[active_slot][0],
                     slot_colors[active_slot][1],
                     slot_colors[active_slot][2]);
    ws2812_show();
}

void device_slot_blink_led(uint8_t slot, uint8_t blink_count) {
    if (slot >= MAX_DEVICE_SLOTS) return;

    /* 指定LEDを blink_count 回点滅 */
    for (uint8_t b = 0; b < blink_count; b++) {
        ws2812_clear();
        ws2812_set_pixel(slot,
                         slot_colors[slot][0],
                         slot_colors[slot][1],
                         slot_colors[slot][2]);
        ws2812_show();
        sleep_ms(150);

        ws2812_clear();
        ws2812_show();
        sleep_ms(150);
    }

    /* 最終状態: アクティブLED点灯 */
    device_slot_update_leds();
}
