# JP106 BLE キーボード 開発ガイド

## 目次

1. [環境セットアップ](#環境セットアップ)
2. [プロジェクト構造](#プロジェクト構造)
3. [ビルド方法](#ビルド方法)
4. [フラッシュ方法](#フラッシュ方法)
5. [カスタマイズ](#カスタマイズ)
6. [アーキテクチャ概要](#アーキテクチャ概要)
7. [デバッグ方法](#デバッグ方法)
8. [トラブルシューティング](#トラブルシューティング)

---

## 環境セットアップ

詳細な手順は [インストールガイド](INSTALLATION_GUIDE.md) を参照。

### 必要なツール

- **CMake** (3.13以上)
- **ARM GCC クロスコンパイラ** (`arm-none-eabi-gcc`)
- **Python 3** (3.6以上) - Pico SDKのビルドツール用
- **Git**
- **Raspberry Pi Pico SDK** (2.x)
- **Ninja** (推奨ビルドジェネレータ)

### 環境変数

```bash
# Windows (PowerShell)
$env:PICO_SDK_PATH = "C:\path\to\pico-sdk"

# Linux/macOS
export PICO_SDK_PATH=/path/to/pico-sdk
```

---

## プロジェクト構造

```text
KeyBoard/
├── CMakeLists.txt              # ビルド設定 (ソース/ライブラリ/PIO/GATT)
├── pico_sdk_import.cmake       # Pico SDK インポート (変更不要)
├── hog_keyboard.gatt           # BLE GATT データベース定義
├── ws2812.pio                  # WS2812B PIO プログラム
├── include/
│   ├── project_config.h        # プロジェクト全体の設定・定数
│   ├── hid_keycodes.h          # USB HID キーコード定義
│   ├── keymap.h                # キーマップ API
│   ├── keyboard_matrix.h       # マトリクススキャン API
│   ├── ble_hid.h               # BLE HID API (キーボード+マウス)
│   ├── device_slot.h           # デバイススロット管理 API
│   ├── ws2812_led.h            # WS2812B LED ドライバ API
│   ├── trackball.h             # I2C トラックボール API
│   └── btstack_config.h        # BTstack コンパイル時設定
├── src/
│   ├── main.c                  # メインループ
│   ├── keymap.c                # JIS 106キー配列テーブル
│   ├── keyboard_matrix.c       # マトリクススキャン + デバウンス
│   ├── ble_hid.c               # BLE HID サービス実装
│   ├── device_slot.c           # 3デバイススロット + Flash保存
│   ├── ws2812_led.c            # WS2812B PIO ドライバ
│   └── trackball.c             # I2C トラックボールドライバ
├── docs/
│   ├── WIRING_GUIDE.md         # 配線ガイド
│   ├── DEVELOPMENT_GUIDE.md    # このファイル
│   └── INSTALLATION_GUIDE.md   # 環境構築ガイド
└── .gitignore
```

### 自動生成ファイル (build/ 内)

| ファイル | 生成元 | 用途 |
| -------- | ------ | ---- |
| `hog_keyboard.h` | `hog_keyboard.gatt` | GATT プロファイルデータ |
| `ws2812.pio.h` | `ws2812.pio` | PIO プログラムバイナリ |
| `jp106_ble_keyboard.uf2` | ソース全体 | Pico 書き込み用ファームウェア |

---

## ビルド方法

### 初回ビルド

```bash
mkdir build && cd build
cmake -G Ninja ..
ninja
```

### 再ビルド

```bash
cd build
ninja
```

### クリーンビルド

```bash
cd build
rm -rf * && cmake -G Ninja .. && ninja
```

成功すると `build/jp106_ble_keyboard.uf2` が生成される。

---

## フラッシュ方法

1. Pico 2W の **BOOTSEL** ボタンを押しながら USB 接続
2. `RPI-RP2` ドライブが表示される
3. `build/jp106_ble_keyboard.uf2` をドライブにコピー
4. 自動的にリセットされ、ファームウェアが起動

---

## カスタマイズ

### BLE デバイス名の変更

ペアリング画面に表示されるデバイス名は **3箇所** で定義されている。
全て同じ文字列に揃える必要がある。

#### 1. プロジェクト設定ヘッダ

ファイル: `include/project_config.h`

```c
#define DEVICE_NAME          "JP106 Keyboard"   // ← ここを変更
```

#### 2. GATT データベース

ファイル: `hog_keyboard.gatt`

```text
CHARACTERISTIC, GAP_DEVICE_NAME, READ, "JP106 Keyboard"
                                        ^^^^^^^^^^^^^^ ← ここを変更
```

#### 3. アドバタイジングデータ (バイト列)

ファイル: `src/ble_hid.c` の `adv_data[]` 配列

```c
static const uint8_t adv_data[] = {
    ...
    /* Complete Local Name */
    0x0F, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME,
    'J', 'P', '1', '0', '6', ' ', 'K', 'e', 'y', 'b', 'o', 'a', 'r', 'd',
    ...
};
```

**変更手順:**

1. 新しいデバイス名の文字数を数える
2. 先頭のバイト = **文字数 + 1** (タイプバイト分) に更新
3. 文字列を1文字ずつカンマ区切りで列挙

**例: `"MyKB"` (4文字) に変更する場合:**

```c
/* 先頭バイト: 4 + 1 = 0x05 */
0x05, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME,
'M', 'y', 'K', 'B',
```

**例: `"PicoBoard 106"` (13文字) に変更する場合:**

```c
/* 先頭バイト: 13 + 1 = 0x0E */
0x0E, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME,
'P', 'i', 'c', 'o', 'B', 'o', 'a', 'r', 'd', ' ', '1', '0', '6',
```

> **注意**: BLE アドバタイジングパケットの最大長は 31 バイト。
> 他のフィールド (Flags: 3, Appearance: 4, Service UUID: 4 = 計11バイト) を差し引くと、
> デバイス名は最大 **18文字** まで使用可能。

---

### キーマップの変更

ファイル: `src/keymap.c`

8×14 マトリクスのキーコード対応テーブル。
行(Row)と列(Col)の交点に HID キーコードを設定する。

```c
static const uint8_t keymap[MATRIX_ROWS][MATRIX_COLS] = {
    /* Row 0: Col0    Col1    Col2    ... */
    {  KEY_GRAVE, KEY_1, KEY_2, ... },
    ...
};
```

キーコード定数は `include/hid_keycodes.h` を参照。
キーを入れ替える場合は、対応するマトリクス位置の定数を変更するだけでよい。

---

### デバウンス時間の変更

ファイル: `include/keyboard_matrix.h`

```c
#define DEBOUNCE_MS  20    // デフォルト 20ms
```

- チャタリングが多い場合: 30-50ms に増加
- レスポンスを重視する場合: 5-10ms に短縮 (キースイッチの品質に依存)

---

### トラックボール感度の変更

ファイル: `include/project_config.h`

```c
#define TRACKBALL_SENSITIVITY  2    // 感度倍率 (1-4)
```

- `1`: 低感度 (精密操作向け)
- `2`: 標準 (デフォルト)
- `3-4`: 高感度 (大画面向け)

---

### スロットLED色の変更

ファイル: `src/device_slot.c`

```c
static const uint8_t slot_colors[MAX_DEVICE_SLOTS][3] = {
    {  0, 32,  0 },   /* スロット0: 緑  (R, G, B) */
    {  0,  0, 32 },   /* スロット1: 青 */
    { 32,  0,  0 },   /* スロット2: 赤 */
};
```

RGB値 (0-255) を変更して任意の色に設定可能。
値が大きいほど明るくなる。通常は 16-64 程度で十分。

---

### デバイススロット数の変更

ファイル: `include/hid_keycodes.h`

```c
#define MAX_DEVICE_SLOTS  3    // 最大3台 (1-3)
```

スロット数を減らす場合は `ws2812_led.h` の `WS2812_NUM_LEDS` も合わせて変更すること。
Fn+数字キーの対応も `keyboard_matrix.c` の `matrix_get_fn_slot_action()` で調整する。

---

### デバッグ出力の有効/無効

ファイル: `include/project_config.h`

```c
#define DEBUG_ENABLED 1    // 1=有効, 0=無効
```

プロダクション (本番) では `0` に設定し、USB シリアル出力も無効化する:

ファイル: `CMakeLists.txt`

```cmake
pico_enable_stdio_usb(${PROJECT_NAME} 0)    # 0 に変更
```

---

### バッテリー監視間隔の変更

ファイル: `include/project_config.h`

```c
#define BATTERY_CHECK_INTERVAL_MS  60000    // 60秒ごと
```

短くするとバッテリー残量の更新が早くなるが、消費電力がわずかに増加する。

---

## アーキテクチャ概要

### メインループ

```text
while (true) {
    1. ble_hid_poll()          ← BLE イベント処理 (CYW43 ポーリング)
    2. matrix_scan()           ← キーマトリクス全行スキャン + デバウンス
    3. Fn+1/2/3 検出           ← スロット切替処理
    4. HID キーボードレポート   ← 変化があれば NKRO/Boot レポート送信
    5. トラックボール読み取り   ← I2C デルタ取得 → マウスレポート送信
    6. バッテリー監視           ← 60秒ごとに ADC 読み取り
    7. LED 更新                ← オンボード LED (接続状態表示)
    8. sleep_us(500)           ← ~1kHz スキャンレート
}
```

### BLE 送信フロー制御

```text
send_report() 呼び出し
    │
    ├── can_send_now == true → 即座に送信、can_send_now = false
    │
    └── can_send_now == false → バッファに保存
                                  → CAN_SEND_NOW イベント要求
                                  → イベント発生時にバッファから送信
```

キーボードレポートがマウスレポートより優先される。

### コンポジット HID レポート

| Report ID | デバイス | サイズ | フォーマット |
| --------- | -------- | ------ | ------------ |
| 1 | キーボード (NKRO) | 22 bytes | modifier(1) + bitmap(21) |
| 2 | マウス | 4 bytes | buttons(1) + X(1) + Y(1) + wheel(1) |
| - | キーボード (Boot) | 8 bytes | modifier(1) + reserved(1) + keys(6) |

Report Protocol モード (通常): Report ID 付きで送信。
Boot Protocol モード (BIOS): キーボードのみ、Report ID なし。

### Flash ストレージ

デバイススロット情報は Flash の最終 4KB セクタに保存。

```text
Flash 4MB:
  0x000000 - 0x3FEFFF : ファームウェア
  0x3FF000 - 0x3FFFFF : スロットデータ (4KB)

スロットデータ構造:
  magic      (4B) : "SLOT" (0x534C4F54)
  active     (1B) : アクティブスロット番号
  reserved   (3B)
  slot[0]    (8B) : BD_ADDR(6) + addr_type(1) + paired(1)
  slot[1]    (8B)
  slot[2]    (8B)
```

---

## デバッグ方法

### USB シリアルデバッグ

ファームウェア書き込み後、USB シリアルポートが表示される。

```bash
# Windows: Tera Term または PuTTY で COM ポートに接続
# Linux/macOS:
screen /dev/ttyACM0 115200
```

`DEBUG_PRINT` マクロで出力されるログ例:

```text
[DEBUG] Flash: slots loaded (active=0)
[DEBUG] Trackball: detected on I2C (addr=0x0A)
[DEBUG] BLE HID initialized (composite: keyboard + mouse)
[DEBUG] BLE advertising started (slot 0)
[DEBUG] JP106 BLE Keyboard started (slot 0, trackball=yes)
[DEBUG] BLE pairing: Just Works confirmed
[DEBUG] BLE pairing complete (success)
[DEBUG] BLE protocol mode: Report (NKRO+Mouse)
```

### オンボード LED

| パターン | 意味 |
| -------- | ---- |
| 常時点灯 | BLE 接続中 |
| 1Hz 点滅 | アドバタイジング中 (未接続) |

### WS2812B スロット LED

| パターン | 意味 |
| -------- | ---- |
| 緑点灯 | スロット 1 アクティブ |
| 青点灯 | スロット 2 アクティブ |
| 赤点灯 | スロット 3 アクティブ |
| 色点滅 | スロット切替中 |

---

## トラブルシューティング

### ビルドエラー

| エラー | 原因 | 解決策 |
| ------ | ---- | ------ |
| `pico_sdk_import.cmake not found` | SDK パス未設定 | `PICO_SDK_PATH` 環境変数を確認 |
| `arm-none-eabi-gcc not found` | コンパイラ未インストール | ARM GCC をインストールし PATH に追加 |
| `hog_keyboard.h not found` | GATT ヘッダ未生成 | クリーンビルドを実行 |
| `ws2812.pio.h not found` | PIO ヘッダ未生成 | クリーンビルドを実行 |
| BTstack 関連エラー | SDK バージョン不一致 | Pico SDK 2.x を使用しているか確認 |

### BLE 接続の問題

| 症状 | 原因 | 解決策 |
| ---- | ---- | ------ |
| デバイスが見つからない | アドバタイジング未開始 | シリアルログで `advertising started` を確認 |
| ペアリングできない | ボンディング失敗 | ホスト側で既存ペアリングを削除して再試行 |
| キー入力が送信されない | プロトコルモード不一致 | シリアルログで protocol mode を確認 |
| トラックボールが動かない | Boot Protocol モード | OS 起動後に Report Protocol に切り替わるのを待つ |

### ハードウェアの問題

| 症状 | 原因 | 解決策 |
| ---- | ---- | ------ |
| キーが反応しない | ダイオードの向き間違い | カソード (帯) が Row 側か確認 |
| ゴーストキー | ダイオード未実装 | 全キーに 1N4148 を実装 |
| トラックボール未検出 | I2C 配線ミス | SDA/SCL の接続、プルアップ抵抗を確認 |
| WS2812B 点灯しない | データ線の接続 | GP22 → LED0 の DIN に接続されているか確認 |

---

## 参考リソース

- [Raspberry Pi Pico 2W Datasheet](https://datasheets.raspberrypi.com/pico/pico2w-datasheet.pdf)
- [Pico C/C++ SDK Documentation](https://datasheets.raspberrypi.com/pico/raspberry-pi-pico-c-sdk.pdf)
- [Pico SDK API Reference](https://www.raspberrypi.com/documentation/pico-sdk/)
- [BTstack Documentation](https://bluekitchen-gmbh.com/btstack/)
- [USB HID Usage Tables](https://usb.org/document-library/hid-usage-tables-15)
- [Pimoroni Trackball Breakout](https://shop.pimoroni.com/products/trackball-breakout)
