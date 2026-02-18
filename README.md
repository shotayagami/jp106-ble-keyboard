# JP106 BLE Keyboard - Raspberry Pi Pico 2W

Raspberry Pi Pico 2W で作る Bluetooth LE 日本語106キー無線キーボード。

## 特徴

- **Bluetooth 5.2 BLE HID** (HOG: HID over GATT)
- **日本語106キー配列** (JIS) - 無変換/変換/カナ等の固有キー対応
- **NKRO** (Nキーロールオーバー) - Report Protocol 時ビットマップ方式
- **Boot Protocol 互換** - BIOS/UEFI での使用可能 (6KRO)
- **バッテリー駆動** - LiPo バッテリー + USB-C 充電

## 開発言語

- **C言語** (C11標準)
- Raspberry Pi Pico SDK + BTstack

## プロジェクト構造

```
KeyBoard/
├── src/
│   ├── main.c                 # メインループ (BLE + マトリクススキャン)
│   ├── keymap.c               # JIS 106キー配列テーブル
│   ├── keyboard_matrix.c      # マトリクススキャン + デバウンス
│   └── ble_hid.c              # BLE HID サービス (NKRO対応)
├── include/
│   ├── project_config.h       # プロジェクト設定
│   ├── btstack_config.h       # BTstack 設定
│   ├── hid_keycodes.h         # HID キーコード定数
│   ├── keymap.h               # キーマップ API
│   ├── keyboard_matrix.h      # マトリクス API
│   └── ble_hid.h              # BLE HID API
├── hog_keyboard.gatt          # GATT データベース定義
├── CMakeLists.txt             # CMake ビルド設定
├── pico_sdk_import.cmake      # Pico SDK インポート
├── docs/
│   ├── WIRING_GUIDE.md        # 配線ガイド (回路図・パーツ)
│   ├── INSTALLATION_GUIDE.md  # 開発環境セットアップ
│   └── DEVELOPMENT_GUIDE.md   # 開発ガイド
└── README.md
```

## クイックスタート

### 前提条件

[インストールガイド](docs/INSTALLATION_GUIDE.md) に従って開発環境をセットアップしてください。

**必要ツール:**
- CMake 3.13以上
- ARM GCC コンパイラ
- Python 3
- Raspberry Pi Pico SDK (PICO_SDK_PATH 設定済み)

### ビルド

```bash
mkdir build
cd build
cmake ..
make
```

### フラッシュ

1. Pico 2W の BOOTSEL ボタンを押しながら USB 接続
2. `build/jp106_ble_keyboard.uf2` を RPI-RP2 ドライブにコピー

### 接続

1. PC/スマホの Bluetooth 設定を開く
2. 「JP106 Keyboard」を選択してペアリング
3. キーボードとして使用可能

## ハードウェア

詳細は [配線ガイド](docs/WIRING_GUIDE.md) を参照。

**主要パーツ:**

- Raspberry Pi Pico 2W × 1
- Cherry MX 互換スイッチ × 106
- 1N4148 ダイオード × 106
- LiPo 3.7V 2000mAh バッテリー × 1
- カスタムPCB (8×14 マトリクス)

## ドキュメント

- [配線ガイド](docs/WIRING_GUIDE.md) - 回路図、パーツリスト、組み立て手順
- [インストールガイド](docs/INSTALLATION_GUIDE.md) - 開発環境のセットアップ
- [開発ガイド](docs/DEVELOPMENT_GUIDE.md) - 開発の進め方
