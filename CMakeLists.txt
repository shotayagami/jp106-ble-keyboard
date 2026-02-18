cmake_minimum_required(VERSION 3.13)

# ターゲットボード設定 (pico_sdk_init() より前に必要)
set(PICO_BOARD pico2_w)

# プロジェクト名
set(PROJECT_NAME jp106_ble_keyboard)

# Pico SDKインポート
include(pico_sdk_import.cmake)

# プロジェクト初期化
project(${PROJECT_NAME} C CXX ASM)

# C/C++標準
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Pico SDK初期化
pico_sdk_init()

# ============================================================
# 実行可能ファイル
# ============================================================
add_executable(${PROJECT_NAME}
    src/main.c
    src/keymap.c
    src/keyboard_matrix.c
    src/ble_hid.c
    src/device_slot.c
    src/ws2812_led.c
    src/trackball.c
)

# インクルードディレクトリ
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

# ============================================================
# ライブラリリンク
# ============================================================
target_link_libraries(${PROJECT_NAME}
    pico_stdlib              # 標準ライブラリ
    pico_btstack_ble         # BTstack BLEコア
    pico_btstack_cyw43       # CYW43439 BLEトランスポート
    pico_cyw43_arch_none     # CYW43 ポーリングモード
    hardware_gpio            # GPIOマトリクス操作
    hardware_adc             # バッテリー電圧監視
    hardware_timer           # タイマー/デバウンス
    hardware_flash           # デバイススロットFlash保存
    hardware_sync            # Flash書込み時の割り込み制御
    hardware_i2c             # トラックボール I2C通信
    hardware_pio             # WS2812B PIO駆動
)

# ============================================================
# PIO ヘッダ自動生成 (ws2812.pio → ws2812.pio.h)
# ============================================================
pico_generate_pio_header(${PROJECT_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio
)

# ============================================================
# GATT ヘッダ自動生成 (hog_keyboard.gatt → hog_keyboard.h)
# ============================================================
pico_btstack_make_gatt_header(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/hog_keyboard.gatt
)

# ============================================================
# 出力設定
# ============================================================
# USBシリアル出力 (デバッグ用。プロダクションでは0に変更)
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# UF2ファイル生成
pico_add_extra_outputs(${PROJECT_NAME})
