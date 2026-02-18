/**
 * @file hid_keycodes.h
 * @brief USB HID キーコード定数 (HID Usage Tables)
 *
 * 日本語106キーボード対応。
 * キーコードは物理キー位置を表し、OS側のキーボードドライバが
 * シンボル変換を行う。
 */

#ifndef HID_KEYCODES_H
#define HID_KEYCODES_H

/* ============================================================
 * Modifier ビットマスク (HIDレポート byte 0)
 * ============================================================ */
#define MOD_LCTRL   0x01
#define MOD_LSHIFT  0x02
#define MOD_LALT    0x04
#define MOD_LGUI    0x08
#define MOD_RCTRL   0x10
#define MOD_RSHIFT  0x20
#define MOD_RALT    0x40
#define MOD_RGUI    0x80

/* ============================================================
 * 標準キーコード (HID Usage ID)
 * ============================================================ */
#define KEY_NONE          0x00

/* アルファベット A-Z (0x04-0x1D) */
#define KEY_A             0x04
#define KEY_B             0x05
#define KEY_C             0x06
#define KEY_D             0x07
#define KEY_E             0x08
#define KEY_F             0x09
#define KEY_G             0x0A
#define KEY_H             0x0B
#define KEY_I             0x0C
#define KEY_J             0x0D
#define KEY_K             0x0E
#define KEY_L             0x0F
#define KEY_M             0x10
#define KEY_N             0x11
#define KEY_O             0x12
#define KEY_P             0x13
#define KEY_Q             0x14
#define KEY_R             0x15
#define KEY_S             0x16
#define KEY_T             0x17
#define KEY_U             0x18
#define KEY_V             0x19
#define KEY_W             0x1A
#define KEY_X             0x1B
#define KEY_Y             0x1C
#define KEY_Z             0x1D

/* 数字 1-0 (0x1E-0x27) */
#define KEY_1             0x1E
#define KEY_2             0x1F
#define KEY_3             0x20
#define KEY_4             0x21
#define KEY_5             0x22
#define KEY_6             0x23
#define KEY_7             0x24
#define KEY_8             0x25
#define KEY_9             0x26
#define KEY_0             0x27

/* 制御キー */
#define KEY_ENTER         0x28
#define KEY_ESCAPE        0x29
#define KEY_BACKSPACE     0x2A
#define KEY_TAB           0x2B
#define KEY_SPACE         0x2C

/* 記号キー (JIS配列では印字が異なるがキーコードは物理位置) */
#define KEY_MINUS         0x2D   /* JIS: - = */
#define KEY_CARET         0x2E   /* JIS: ^ ~  (US: = +) */
#define KEY_AT            0x2F   /* JIS: @ `  (US: [ {) */
#define KEY_LBRACKET      0x30   /* JIS: [ {  (US: ] }) */
#define KEY_RBRACKET      0x32   /* JIS: ] }  (US: Non-US # ~) */
#define KEY_SEMICOLON     0x33   /* JIS: ; + */
#define KEY_COLON         0x34   /* JIS: : *  (US: ' ") */
#define KEY_GRAVE         0x35   /* JIS: 半角/全角  (US: ` ~) */
#define KEY_COMMA         0x36   /* , < */
#define KEY_PERIOD        0x37   /* . > */
#define KEY_SLASH         0x38   /* / ? */
#define KEY_CAPSLOCK      0x39

/* ファンクションキー F1-F12 (0x3A-0x45) */
#define KEY_F1            0x3A
#define KEY_F2            0x3B
#define KEY_F3            0x3C
#define KEY_F4            0x3D
#define KEY_F5            0x3E
#define KEY_F6            0x3F
#define KEY_F7            0x40
#define KEY_F8            0x41
#define KEY_F9            0x42
#define KEY_F10           0x43
#define KEY_F11           0x44
#define KEY_F12           0x45

/* ナビゲーションクラスタ */
#define KEY_PRINTSCREEN   0x46
#define KEY_SCROLLLOCK    0x47
#define KEY_PAUSE         0x48
#define KEY_INSERT        0x49
#define KEY_HOME          0x4A
#define KEY_PAGEUP        0x4B
#define KEY_DELETE        0x4C
#define KEY_END           0x4D
#define KEY_PAGEDOWN      0x4E

/* 矢印キー */
#define KEY_RIGHT         0x4F
#define KEY_LEFT          0x50
#define KEY_DOWN          0x51
#define KEY_UP            0x52

/* テンキー */
#define KEY_NUMLOCK       0x53
#define KEY_KP_DIVIDE     0x54
#define KEY_KP_MULTIPLY   0x55
#define KEY_KP_MINUS      0x56
#define KEY_KP_PLUS       0x57
#define KEY_KP_ENTER      0x58
#define KEY_KP_1          0x59
#define KEY_KP_2          0x5A
#define KEY_KP_3          0x5B
#define KEY_KP_4          0x5C
#define KEY_KP_5          0x5D
#define KEY_KP_6          0x5E
#define KEY_KP_7          0x5F
#define KEY_KP_8          0x60
#define KEY_KP_9          0x61
#define KEY_KP_0          0x62
#define KEY_KP_DOT        0x63

/* ============================================================
 * 日本語固有キー (International / Language)
 * ============================================================ */
#define KEY_INT1          0x87   /* JIS: ＼ _ (Ro, 右Shift左) */
#define KEY_INT2          0x88   /* カタカナ/ひらがな */
#define KEY_INT3          0x89   /* JIS: ¥ | (Yen) */
#define KEY_INT4          0x8A   /* 変換 (Henkan) */
#define KEY_INT5          0x8B   /* 無変換 (Muhenkan) */

/* 日本語キーのエイリアス */
#define KEY_JIS_BACKSLASH KEY_INT1
#define KEY_JIS_KATAKANA  KEY_INT2
#define KEY_JIS_YEN       KEY_INT3
#define KEY_JIS_HENKAN    KEY_INT4
#define KEY_JIS_MUHENKAN  KEY_INT5
#define KEY_JIS_HANKAKU   KEY_GRAVE   /* 半角/全角 = US grave位置 */

/* ============================================================
 * ファームウェア内部専用キーコード (HIDには送信しない)
 * ============================================================ */
#define KEY_FN            0xF0   /* Fnキー (ファームウェアレイヤー切替) */

/* Fn + 数字キーでデバイススロット切替 */
#define MAX_DEVICE_SLOTS  3

/* ============================================================
 * NKROビットマップ用定数
 * ============================================================ */
#define NKRO_BITMAP_BYTES   21   /* Usage 0x00-0xA7 = 168 bits = 21 bytes */
#define NKRO_REPORT_SIZE    22   /* 1 modifier + 21 bitmap */
#define BOOT_REPORT_SIZE    8    /* 1 modifier + 1 reserved + 6 keycodes */

/* ============================================================
 * コンポジットデバイス Report ID
 * ============================================================ */
#define HID_REPORT_ID_KEYBOARD  1
#define HID_REPORT_ID_MOUSE     2

/* マウスレポートサイズ (Report ID含まず) */
#define MOUSE_REPORT_SIZE       4   /* buttons(1) + X(1) + Y(1) + wheel(1) */
#define BOOT_MOUSE_REPORT_SIZE  3   /* buttons(1) + X(1) + Y(1) */

/* マウスボタンビット */
#define MOUSE_BTN_LEFT    0x01
#define MOUSE_BTN_RIGHT   0x02
#define MOUSE_BTN_MIDDLE  0x04

#endif /* HID_KEYCODES_H */
