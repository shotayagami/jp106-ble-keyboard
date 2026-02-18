/**
 * @file ble_hid.c
 * @brief BLE HID サービス実装 (コンポジットデバイス)
 *
 * BTstack HOG (HID over GATT) を使用した BLE キーボード+マウス実装。
 *
 * コンポジットデバイス:
 *   - Report ID 1: キーボード (NKRO ビットマップ)
 *   - Report ID 2: マウス (ボタン + X/Y移動 + ホイール)
 *
 * デュアルプロトコル (キーボード):
 *   - Boot Protocol (6KRO): BIOS/UEFI 互換。8バイト標準レポート。
 *   - Report Protocol (NKRO): OS用。Report ID付きレポート。
 *
 * マウスは Report Protocol のみ対応。
 * Boot Protocol ではトラックボール入力は無視される。
 *
 * フロー制御:
 *   BLE は任意のタイミングで送信不可。CAN_SEND_NOW イベントを待ち、
 *   その時点でバッファ済みレポートを送信する。
 *   キーボードレポートがマウスレポートより優先。
 */

#include "ble_hid.h"
#include "hid_keycodes.h"
#include "project_config.h"
#include "device_slot.h"

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/btstack_cyw43.h"

#include "btstack.h"
#include "ble/gatt-service/battery_service_server.h"
#include "ble/gatt-service/device_information_service_server.h"
#include "ble/gatt-service/hids_device.h"

/* ビルド時自動生成 (hog_keyboard.gatt → hog_keyboard.h) */
#include "hog_keyboard.h"

/* ============================================================
 * HID Report Descriptor (コンポジット: キーボード + マウス)
 *
 * Report ID 1: キーボード
 *   byte 0:     modifier keys (8 bits)
 *   bytes 1-21: NKRO bitmap (168 bits, usage 0x00-0xA7)
 *
 * Report ID 2: マウス
 *   byte 0:     buttons (3 bits + 5 padding)
 *   byte 1:     X movement (int8)
 *   byte 2:     Y movement (int8)
 *   byte 3:     wheel (int8)
 * ============================================================ */
static const uint8_t hid_report_descriptor[] = {
    /* ===== Keyboard Collection (Report ID 1) ===== */
    0x05, 0x01,        /* Usage Page (Generic Desktop) */
    0x09, 0x06,        /* Usage (Keyboard) */
    0xA1, 0x01,        /* Collection (Application) */
    0x85, 0x01,        /*   Report ID (1) */

    /* --- Modifier byte (8 bits) --- */
    0x05, 0x07,        /*   Usage Page (Keyboard/Keypad) */
    0x19, 0xE0,        /*   Usage Minimum (Left Control) */
    0x29, 0xE7,        /*   Usage Maximum (Right GUI) */
    0x15, 0x00,        /*   Logical Minimum (0) */
    0x25, 0x01,        /*   Logical Maximum (1) */
    0x75, 0x01,        /*   Report Size (1 bit) */
    0x95, 0x08,        /*   Report Count (8) */
    0x81, 0x02,        /*   Input (Data, Variable, Absolute) */

    /* --- NKRO bitmap (168 bits = 21 bytes) --- */
    0x95, 0xA8,        /*   Report Count (168) */
    0x75, 0x01,        /*   Report Size (1 bit) */
    0x15, 0x00,        /*   Logical Minimum (0) */
    0x25, 0x01,        /*   Logical Maximum (1) */
    0x05, 0x07,        /*   Usage Page (Keyboard/Keypad) */
    0x19, 0x00,        /*   Usage Minimum (0x00) */
    0x29, 0xA7,        /*   Usage Maximum (0xA7) */
    0x81, 0x02,        /*   Input (Data, Variable, Absolute) */

    /* --- LED output report (5 bits + 3 padding) --- */
    0x95, 0x05,        /*   Report Count (5) */
    0x75, 0x01,        /*   Report Size (1 bit) */
    0x05, 0x08,        /*   Usage Page (LEDs) */
    0x19, 0x01,        /*   Usage Minimum (Num Lock) */
    0x29, 0x05,        /*   Usage Maximum (Kana) */
    0x91, 0x02,        /*   Output (Data, Variable, Absolute) */
    0x95, 0x01,        /*   Report Count (1) */
    0x75, 0x03,        /*   Report Size (3 bits) */
    0x91, 0x01,        /*   Output (Constant) - padding */

    0xC0,              /* End Collection (Keyboard) */

    /* ===== Mouse Collection (Report ID 2) ===== */
    0x05, 0x01,        /* Usage Page (Generic Desktop) */
    0x09, 0x02,        /* Usage (Mouse) */
    0xA1, 0x01,        /* Collection (Application) */
    0x85, 0x02,        /*   Report ID (2) */
    0x09, 0x01,        /*   Usage (Pointer) */
    0xA1, 0x00,        /*   Collection (Physical) */

    /* --- Buttons (3 bits + 5 padding) --- */
    0x05, 0x09,        /*     Usage Page (Button) */
    0x19, 0x01,        /*     Usage Minimum (Button 1) */
    0x29, 0x03,        /*     Usage Maximum (Button 3) */
    0x15, 0x00,        /*     Logical Minimum (0) */
    0x25, 0x01,        /*     Logical Maximum (1) */
    0x75, 0x01,        /*     Report Size (1 bit) */
    0x95, 0x03,        /*     Report Count (3) */
    0x81, 0x02,        /*     Input (Data, Variable, Absolute) */
    0x95, 0x01,        /*     Report Count (1) */
    0x75, 0x05,        /*     Report Size (5 bits) */
    0x81, 0x01,        /*     Input (Constant) - padding */

    /* --- X, Y movement (2 bytes, signed) --- */
    0x05, 0x01,        /*     Usage Page (Generic Desktop) */
    0x09, 0x30,        /*     Usage (X) */
    0x09, 0x31,        /*     Usage (Y) */
    0x15, 0x81,        /*     Logical Minimum (-127) */
    0x25, 0x7F,        /*     Logical Maximum (127) */
    0x75, 0x08,        /*     Report Size (8 bits) */
    0x95, 0x02,        /*     Report Count (2) */
    0x81, 0x06,        /*     Input (Data, Variable, Relative) */

    /* --- Wheel (1 byte, signed) --- */
    0x09, 0x38,        /*     Usage (Wheel) */
    0x15, 0x81,        /*     Logical Minimum (-127) */
    0x25, 0x7F,        /*     Logical Maximum (127) */
    0x75, 0x08,        /*     Report Size (8 bits) */
    0x95, 0x01,        /*     Report Count (1) */
    0x81, 0x06,        /*     Input (Data, Variable, Relative) */

    0xC0,              /*   End Collection (Physical) */
    0xC0,              /* End Collection (Mouse) */
};

/* ============================================================
 * BLE 状態管理
 * ============================================================ */
static hci_con_handle_t con_handle = HCI_CON_HANDLE_INVALID;
static uint8_t protocol_mode = 1;   /* 0=Boot, 1=Report */
static bool can_send_now = false;
static uint8_t battery_level = 100;
static bool btstack_ready = false;

/* キーボード送信バッファ (優先度: 高) */
#define MAX_KB_REPORT_SIZE  (1 + NKRO_REPORT_SIZE)  /* Report ID + NKRO */
static uint8_t pending_kb_report[MAX_KB_REPORT_SIZE];
static uint8_t pending_kb_len = 0;
static bool kb_pending = false;
static bool kb_is_boot = false;  /* Boot Protocolフラグ */

/* マウス送信バッファ (優先度: 低) */
#define MAX_MOUSE_REPORT_SIZE  (1 + MOUSE_REPORT_SIZE)  /* Report ID + Mouse */
static uint8_t pending_mouse_report[MAX_MOUSE_REPORT_SIZE];
static bool mouse_pending = false;

/* コールバック登録 */
static btstack_packet_callback_registration_t hci_event_callback_registration;
static btstack_packet_callback_registration_t sm_event_callback_registration;

/* ============================================================
 * 内部関数: 送信処理
 * ============================================================ */

static void send_pending_reports(void) {
    if (!can_send_now) return;
    if (con_handle == HCI_CON_HANDLE_INVALID) return;

    /* キーボードレポート優先 */
    if (kb_pending) {
        can_send_now = false;
        kb_pending = false;

        if (kb_is_boot) {
            hids_device_send_boot_keyboard_input_report(
                con_handle, pending_kb_report, pending_kb_len);
        } else {
            hids_device_send_input_report(
                con_handle, pending_kb_report, pending_kb_len);
        }

        /* マウスも保留中なら次の CAN_SEND_NOW を要求 */
        if (mouse_pending) {
            hids_device_request_can_send_now_event(con_handle);
        }
        return;
    }

    /* マウスレポート */
    if (mouse_pending) {
        can_send_now = false;
        mouse_pending = false;

        hids_device_send_input_report(
            con_handle, pending_mouse_report, MAX_MOUSE_REPORT_SIZE);
        return;
    }
}

/* ============================================================
 * アドバタイジングデータ
 * ============================================================ */
static const uint8_t adv_data[] = {
    /* Flags: General Discoverable + BR/EDR Not Supported */
    0x02, BLUETOOTH_DATA_TYPE_FLAGS, 0x06,
    /* Appearance: HID Keyboard (0x03C1) */
    0x03, BLUETOOTH_DATA_TYPE_APPEARANCE, 0xC1, 0x03,
    /* Complete Local Name: "JP106 Keyboard" */
    0x0F, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME,
    'J', 'P', '1', '0', '6', ' ', 'K', 'e', 'y', 'b', 'o', 'a', 'r', 'd',
    /* Incomplete list of 16-bit Service UUIDs: HID Service (0x1812) */
    0x03, BLUETOOTH_DATA_TYPE_INCOMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS,
    0x12, 0x18,
};

/* ============================================================
 * 内部関数: アドバタイジング開始
 * ============================================================ */
static void start_advertising(void) {
    if (!btstack_ready) return;

    gap_advertisements_set_data(sizeof(adv_data), adv_data);
    gap_advertisements_enable(1);
    DEBUG_PRINT("BLE advertising started (slot %d)", device_slot_get_active());
}

/* ============================================================
 * BLE イベントハンドラ
 * ============================================================ */
static void packet_handler(uint8_t packet_type, uint16_t channel,
                           uint8_t *packet, uint16_t size) {
    UNUSED(channel);
    UNUSED(size);

    if (packet_type != HCI_EVENT_PACKET) return;

    uint8_t event_type = hci_event_packet_get_type(packet);

    switch (event_type) {
        case BTSTACK_EVENT_STATE:
            if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
                btstack_ready = true;
                start_advertising();
            }
            break;

        case HCI_EVENT_DISCONNECTION_COMPLETE:
            con_handle = HCI_CON_HANDLE_INVALID;
            can_send_now = false;
            kb_pending = false;
            mouse_pending = false;
            /* 切断後にアドバタイジング再開 */
            start_advertising();
            DEBUG_PRINT("BLE disconnected, re-advertising");
            break;

        case HCI_EVENT_HIDS_META:
            switch (hci_event_hids_meta_get_subevent_code(packet)) {
                case HIDS_SUBEVENT_INPUT_REPORT_ENABLE:
                    con_handle = hids_subevent_input_report_enable_get_con_handle(packet);
                    DEBUG_PRINT("BLE HID input report enabled");
                    break;
                case HIDS_SUBEVENT_BOOT_KEYBOARD_INPUT_REPORT_ENABLE:
                    con_handle = hids_subevent_boot_keyboard_input_report_enable_get_con_handle(packet);
                    DEBUG_PRINT("BLE HID boot keyboard report enabled");
                    break;
                case HIDS_SUBEVENT_PROTOCOL_MODE:
                    protocol_mode = hids_subevent_protocol_mode_get_protocol_mode(packet);
                    DEBUG_PRINT("BLE protocol mode: %s",
                                protocol_mode ? "Report (NKRO+Mouse)" : "Boot (6KRO)");
                    break;
                case HIDS_SUBEVENT_CAN_SEND_NOW:
                    can_send_now = true;
                    send_pending_reports();
                    break;
            }
            break;

        case SM_EVENT_JUST_WORKS_REQUEST:
            sm_just_works_confirm(sm_event_just_works_request_get_handle(packet));
            DEBUG_PRINT("BLE pairing: Just Works confirmed");
            break;

        case SM_EVENT_PAIRING_COMPLETE: {
            uint8_t status = sm_event_pairing_complete_get_status(packet);
            if (status == ERROR_CODE_SUCCESS) {
                /* ペアリング成功: 接続先アドレスをスロットに保存 */
                hci_con_handle_t handle = sm_event_pairing_complete_get_con_handle(packet);
                bd_addr_t peer_addr;
                gap_le_get_own_address(/* unused */ NULL, peer_addr);

                /* 接続ハンドルからピアアドレスを取得 */
                hci_connection_t *conn = hci_connection_for_handle(handle);
                if (conn) {
                    device_slot_save_pairing(conn->address, conn->address_type);
                }
                DEBUG_PRINT("BLE pairing complete (success)");
            } else {
                DEBUG_PRINT("BLE pairing failed (status=%d)", status);
            }
            break;
        }
    }
}

/* ============================================================
 * Public API
 * ============================================================ */

void ble_hid_init(void) {
    /* CYW43 初期化 (WiFi/BTチップ) */
    if (cyw43_arch_init()) {
        DEBUG_PRINT("ERROR: cyw43_arch_init failed");
        return;
    }

    /* L2CAP 初期化 */
    l2cap_init();

    /* Security Manager: ボンディング + Just Works ペアリング */
    sm_init();
    sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
    sm_set_authentication_requirements(SM_AUTHREQ_SECURE_CONNECTION | SM_AUTHREQ_BONDING);

    /* ATT Server 初期化 (GATTデータベース登録) */
    att_server_init(profile_data, NULL, NULL);

    /* GATT サービス初期化 */
    battery_service_server_init(battery_level);
    device_information_service_server_init();

    /* HID Device サービス初期化 (コンポジット: キーボード + マウス) */
    hids_device_init(0, hid_report_descriptor, sizeof(hid_report_descriptor));

    /* イベントハンドラ登録 */
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    sm_event_callback_registration.callback = &packet_handler;
    sm_add_event_handler(&sm_event_callback_registration);

    hids_device_register_packet_handler(packet_handler);

    /* 接続パラメータ: 低レイテンシ (キーボード+ポインティング向け) */
    gap_set_connection_parameters(6, 9, 25, 200);

    /* HCI 電源ON → BTstack起動 */
    hci_power_on();

    DEBUG_PRINT("BLE HID initialized (composite: keyboard + mouse)");
}

void ble_hid_send_report(const uint8_t *report, uint8_t len) {
    if (con_handle == HCI_CON_HANDLE_INVALID) return;

    if (protocol_mode == 0) {
        /* Boot Protocol: Report IDなし、そのまま送信 */
        if (!can_send_now) {
            uint8_t copy_len = (len > BOOT_REPORT_SIZE) ? BOOT_REPORT_SIZE : len;
            memcpy(pending_kb_report, report, copy_len);
            pending_kb_len = copy_len;
            kb_pending = true;
            kb_is_boot = true;
            hids_device_request_can_send_now_event(con_handle);
            return;
        }
        can_send_now = false;
        hids_device_send_boot_keyboard_input_report(con_handle, report, len);
    } else {
        /* Report Protocol: Report ID 1 を先頭に付与して送信 */
        uint8_t buf[1 + NKRO_REPORT_SIZE];
        buf[0] = HID_REPORT_ID_KEYBOARD;
        uint8_t data_len = (len > NKRO_REPORT_SIZE) ? NKRO_REPORT_SIZE : len;
        memcpy(buf + 1, report, data_len);
        uint8_t total_len = 1 + data_len;

        if (!can_send_now) {
            memcpy(pending_kb_report, buf, total_len);
            pending_kb_len = total_len;
            kb_pending = true;
            kb_is_boot = false;
            hids_device_request_can_send_now_event(con_handle);
            return;
        }
        can_send_now = false;
        hids_device_send_input_report(con_handle, buf, total_len);
    }

    /* マウスが保留中なら次のCAN_SEND_NOWを要求 */
    if (mouse_pending) {
        hids_device_request_can_send_now_event(con_handle);
    }
}

void ble_hid_send_mouse_report(uint8_t buttons, int8_t delta_x,
                                int8_t delta_y, int8_t wheel) {
    if (con_handle == HCI_CON_HANDLE_INVALID) return;
    if (protocol_mode == 0) return;  /* Boot Protocolではマウス無効 */

    /* Report ID 2 + マウスデータ */
    uint8_t buf[1 + MOUSE_REPORT_SIZE];
    buf[0] = HID_REPORT_ID_MOUSE;
    buf[1] = buttons;
    buf[2] = (uint8_t)delta_x;
    buf[3] = (uint8_t)delta_y;
    buf[4] = (uint8_t)wheel;

    if (!can_send_now) {
        memcpy(pending_mouse_report, buf, sizeof(buf));
        mouse_pending = true;
        if (!kb_pending) {
            hids_device_request_can_send_now_event(con_handle);
        }
        return;
    }

    can_send_now = false;
    hids_device_send_input_report(con_handle, buf, sizeof(buf));
}

bool ble_hid_is_connected(void) {
    return con_handle != HCI_CON_HANDLE_INVALID;
}

uint8_t ble_hid_get_protocol_mode(void) {
    return protocol_mode;
}

void ble_hid_poll(void) {
    cyw43_arch_poll();
}

void ble_hid_send_key_release(void) {
    if (protocol_mode == 0) {
        uint8_t empty[BOOT_REPORT_SIZE] = {0};
        ble_hid_send_report(empty, BOOT_REPORT_SIZE);
    } else {
        uint8_t empty[NKRO_REPORT_SIZE] = {0};
        ble_hid_send_report(empty, NKRO_REPORT_SIZE);
    }
}

void ble_hid_update_battery(uint8_t level) {
    battery_level = level;
    battery_service_server_set_battery_value(level);
}

void ble_hid_disconnect_and_readvertise(void) {
    if (con_handle != HCI_CON_HANDLE_INVALID) {
        /* 全キー解放を送信してから切断 */
        ble_hid_send_key_release();
        gap_disconnect(con_handle);
        /* 切断完了はイベントハンドラで処理、そこで再アドバタイジング */
    } else {
        /* 未接続なら即座に再アドバタイジング */
        start_advertising();
    }
    DEBUG_PRINT("BLE disconnect requested for slot switch");
}
