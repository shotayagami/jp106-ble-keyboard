/**
 * @file keymap.c
 * @brief 日本語106キー配列テーブル
 *
 * 8行×14列マトリクスからHIDキーコードへのマッピング。
 * Modifierキーは 0xE0-0xE7 の内部エンコーディングを使用。
 * 空ポジションは KEY_NONE (0x00)。
 */

#include "keymap.h"
#include "hid_keycodes.h"

/* 日本語106キー配列: [row][col] -> HID keycode */
static const uint8_t keymap[MATRIX_ROWS][MATRIX_COLS] = {
    /* Row 0: 半全, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, -, ^, ¥ */
    {
        KEY_JIS_HANKAKU, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6,
        KEY_7, KEY_8, KEY_9, KEY_0, KEY_MINUS, KEY_CARET, KEY_JIS_YEN
    },
    /* Row 1: Tab, Q, W, E, R, T, Y, U, I, O, P, @, [, BS */
    {
        KEY_TAB, KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y,
        KEY_U, KEY_I, KEY_O, KEY_P, KEY_AT, KEY_LBRACKET, KEY_BACKSPACE
    },
    /* Row 2: CapsLock, A, S, D, F, G, H, J, K, L, ;, :, ], Enter */
    {
        KEY_CAPSLOCK, KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H,
        KEY_J, KEY_K, KEY_L, KEY_SEMICOLON, KEY_COLON, KEY_RBRACKET, KEY_ENTER
    },
    /* Row 3: LShift, Z, X, C, V, B, N, M, ,, ., /, ＼, Up, RShift */
    {
        KC_LSHIFT, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N,
        KEY_M, KEY_COMMA, KEY_PERIOD, KEY_SLASH, KEY_JIS_BACKSLASH, KEY_UP, KC_RSHIFT
    },
    /* Row 4: LCtrl, Win, LAlt, 無変換, Fn(L), Space, Fn(R), 変換, かな, RAlt, RCtrl, Left, Down, Right */
    {
        KC_LCTRL, KC_LGUI, KC_LALT, KEY_JIS_MUHENKAN, KEY_FN, KEY_SPACE, KEY_FN,
        KEY_JIS_HENKAN, KEY_JIS_KATAKANA, KC_RALT, KC_RCTRL, KEY_LEFT, KEY_DOWN, KEY_RIGHT
    },
    /* Row 5: F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, Esc, (空) */
    {
        KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7,
        KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12, KEY_ESCAPE, KEY_NONE
    },
    /* Row 6: PrtSc, ScrLk, Pause, Ins, Home, PgUp, Del, End, PgDn, NumLk, KP/, KP*, KP-, (空) */
    {
        KEY_PRINTSCREEN, KEY_SCROLLLOCK, KEY_PAUSE, KEY_INSERT, KEY_HOME, KEY_PAGEUP,
        KEY_DELETE, KEY_END, KEY_PAGEDOWN, KEY_NUMLOCK, KEY_KP_DIVIDE, KEY_KP_MULTIPLY,
        KEY_KP_MINUS, KEY_NONE
    },
    /* Row 7: KP7, KP8, KP9, KP4, KP5, KP6, KP+, KP1, KP2, KP3, KP0, KP., KPEnter, (空) */
    {
        KEY_KP_7, KEY_KP_8, KEY_KP_9, KEY_KP_4, KEY_KP_5, KEY_KP_6,
        KEY_KP_PLUS, KEY_KP_1, KEY_KP_2, KEY_KP_3, KEY_KP_0, KEY_KP_DOT,
        KEY_KP_ENTER, KEY_NONE
    },
};

uint8_t keymap_get_keycode(uint8_t row, uint8_t col) {
    if (row >= MATRIX_ROWS || col >= MATRIX_COLS) return KEY_NONE;
    return keymap[row][col];
}

bool keymap_is_modifier(uint8_t row, uint8_t col) {
    uint8_t kc = keymap_get_keycode(row, col);
    return IS_MODIFIER(kc);
}

uint8_t keymap_get_modifier_bit(uint8_t row, uint8_t col) {
    uint8_t kc = keymap_get_keycode(row, col);
    if (IS_MODIFIER(kc)) return MODIFIER_BIT(kc);
    return 0;
}
