# Raspberry Pi Pico 2W 開発ガイド

## 目次

1. [環境セットアップ](#環境セットアップ)
2. [プロジェクト構造](#プロジェクト構造)
3. [ビルド方法](#ビルド方法)
4. [フラッシュ方法](#フラッシュ方法)
5. [開発の進め方](#開発の進め方)
6. [開発で気をつける点](#開発で気をつける点)
7. [デバッグ方法](#デバッグ方法)
8. [トラブルシューティング](#トラブルシューティング)

---

## 環境セットアップ

開発環境のセットアップについては、詳細な手順を [インストールガイド](INSTALLATION_GUIDE.md) に記載しています。

### 必要なツール

1. **CMake** (バージョン 3.13以上) - ビルドシステム
2. **ARM GCC コンパイラ** - クロスコンパイラ
3. **Python 3** (バージョン 3.6以上) - Pico SDKのビルドツール用
4. **Git** - SDKのダウンロード用
5. **Raspberry Pi Pico SDK** - 開発SDK

### 環境変数の設定

Pico SDKのパスを環境変数に設定する必要があります：

**Windows (PowerShell):**
```powershell
$env:PICO_SDK_PATH = "C:\path\to\pico-sdk"
```

**Linux/macOS:**
```bash
export PICO_SDK_PATH=/path/to/pico-sdk
```

詳細なインストール手順と環境変数の設定方法は [インストールガイド](INSTALLATION_GUIDE.md) を参照してください。

---

## プロジェクト構造

```
RaspberryPiPico2Wproj/
├── src/                    # メインソースコード
│   └── main.c             # エントリーポイント
├── include/                # ヘッダーファイル
│   └── project_config.h   # プロジェクト設定
├── docs/                   # ドキュメント
│   └── DEVELOPMENT_GUIDE.md  # このファイル
├── build/                  # ビルド出力（.gitignoreに含める）
├── CMakeLists.txt          # CMakeビルド設定
├── pico_sdk_import.cmake   # Pico SDKインポート設定
├── .gitignore              # Git除外設定
├── .vscode/                # VS Code設定
│   └── settings.json       # エディタ設定
└── README.md               # プロジェクト概要
```

### ディレクトリの役割

- **src/**: メインのソースコード（.cファイル）
- **include/**: ヘッダーファイル（.hファイル）
- **docs/**: プロジェクトドキュメント
- **build/**: CMakeのビルド出力（自動生成、Git管理外）

---

## ビルド方法

### 初回ビルド

```bash
# ビルドディレクトリを作成
mkdir build
cd build

# CMakeでビルド設定を生成
cmake ..

# ビルド実行
make
# または
cmake --build .
```

### 再ビルド

```bash
cd build
make
```

### クリーンビルド

```bash
cd build
rm -rf *
cmake ..
make
```

### ビルド成果物

ビルドが成功すると、`build/`ディレクトリに以下が生成されます：

- `プロジェクト名.uf2`: Picoにフラッシュするファイル
- `プロジェクト名.elf`: ELF実行ファイル
- `プロジェクト名.bin`: バイナリファイル
- `プロジェクト名.dis`: 逆アセンブルファイル（デバッグ用）

---

## フラッシュ方法

### BOOTSELモードでのフラッシュ

1. **Pico 2WをBOOTSELモードで起動**
   - BOOTSELボタンを押しながらUSBケーブルを接続
   - または、リセットボタンを押しながらBOOTSELボタンを押す

2. **USBドライブとして認識される**
   - Windows: `RPI-RP2`というドライブが表示される
   - Linux/macOS: `/media/RPI-RP2/`にマウントされる

3. **UF2ファイルをコピー**
   ```bash
   # Windows
   copy build\プロジェクト名.uf2 R:\

   # Linux/macOS
   cp build/プロジェクト名.uf2 /media/RPI-RP2/
   ```

4. **自動的にフラッシュされる**
   - コピーが完了すると自動的にフラッシュが開始される
   - フラッシュ完了後、Picoが自動的にリセットされる

### 注意点

- BOOTSELモードでは、Picoは通常のプログラムを実行しない
- フラッシュ後は自動的に通常モードで起動する
- フラッシュ中はUSBケーブルを抜かないこと

---

## 開発の進め方

### 1. プロジェクトの初期化

```bash
# プロジェクトディレクトリを作成
mkdir RaspberryPiPico2Wproj
cd RaspberryPiPico2Wproj

# ディレクトリ構造を作成
mkdir src include build docs
```

### 2. CMakeLists.txtの作成

プロジェクトのルートに`CMakeLists.txt`を作成し、Pico SDKを設定します。

### 3. ソースコードの作成

`src/main.c`にメインコードを記述します。

### 4. ビルドとテスト

```bash
# ビルド
cd build
cmake ..
make

# フラッシュ
# UF2ファイルをPicoにコピー
```

### 5. デバッグと改善

- シリアル出力でデバッグ情報を確認
- 必要に応じてログを追加
- 段階的に機能を追加

### 開発フロー

```
コード記述 → ビルド → フラッシュ → テスト → デバッグ → 改善
     ↑                                                      ↓
     └──────────────────────────────────────────────────────┘
```

---

## 開発で気をつける点

### 1. メモリ管理

- **RAM容量**: Pico 2Wは264KBのRAMを持つ
- **スタックオーバーフロー**: 大きな配列は静的メモリ（グローバル変数）に配置
- **ヒープ使用**: `malloc()`は使用可能だが、メモリリークに注意
- **推奨**: 可能な限り静的メモリを使用

```c
// 推奨: 静的メモリ
static uint8_t buffer[1024];

// 注意: スタックオーバーフローの可能性
void function() {
    uint8_t buffer[1024];  // 大きな配列は避ける
}
```

### 2. 割り込み処理

- **割り込みハンドラは短く**: 長時間の処理は避ける
- **共有変数の保護**: 割り込みとメインループで共有する変数は`volatile`を使用
- **クリティカルセクション**: `__disable_irq()`/`__enable_irq()`で保護

```c
volatile bool flag = false;

void irq_handler() {
    flag = true;  // 短い処理のみ
}

int main() {
    while (1) {
        if (flag) {
            // メインループで処理
            flag = false;
        }
    }
}
```

### 3. タイミングと遅延

- **busy_wait**: 正確なタイミングが必要な場合
- **sleep_ms/sleep_us**: 通常の遅延
- **タイマー**: 定期的な処理にはタイマーを使用

```c
#include "pico/time.h"

// 正確な遅延（マイクロ秒）
sleep_us(1000);

// ミリ秒単位の遅延
sleep_ms(100);

// タイマーを使用した定期的な処理
absolute_time_t timeout = make_timeout_time_ms(1000);
```

### 4. GPIO操作

- **初期化**: `gpio_init()`で初期化
- **方向設定**: `gpio_set_dir()`で入力/出力を設定
- **プルアップ/プルダウン**: 必要に応じて設定
- **デバウンス**: スイッチ入力にはデバウンス処理を実装

```c
#include "hardware/gpio.h"

gpio_init(PIN);
gpio_set_dir(PIN, GPIO_OUT);
gpio_put(PIN, 1);

// 入力の場合
gpio_set_dir(PIN, GPIO_IN);
gpio_pull_up(PIN);
```

### 5. シリアル通信（UART）

- **デバッグ出力**: `stdio`を使用（USBシリアル）
- **ボーレート**: 115200が標準
- **バッファリング**: 必要に応じてバッファを実装

```c
#include "pico/stdio.h"

stdio_init_all();

printf("Hello, Pico 2W!\n");
```

### 6. WiFi機能（Pico 2W特有）

- **初期化**: `cyw43_arch_init()`でWiFiを初期化
- **電力管理**: WiFiは電力消費が大きいため、不要な時はオフにする
- **非同期処理**: WiFi操作は時間がかかるため、非同期処理を検討

```c
#include "pico/cyw43_arch.h"

cyw43_arch_init();
cyw43_arch_enable_sta_mode();
```

### 7. エラーハンドリング

- **戻り値のチェック**: 関数の戻り値を必ず確認
- **エラーログ**: エラー時は適切なログを出力
- **リセット**: 致命的なエラー時は`reset()`を呼ぶ

```c
if (gpio_init(PIN) < 0) {
    printf("GPIO initialization failed\n");
    return -1;
}
```

### 8. パフォーマンス最適化

- **コンパイラ最適化**: `-O2`または`-O3`を使用
- **インライン関数**: 小さな関数は`inline`で宣言
- **不要な浮動小数点演算**: 可能な限り整数演算を使用
- **キャッシュ効率**: メモリアクセスパターンを考慮

### 9. 電源管理

- **スリープモード**: 不要な時はスリープモードを使用
- **周辺機器の無効化**: 使用しない周辺機器は無効化
- **クロック速度**: 必要に応じてクロック速度を調整

### 10. デバッグのベストプラクティス

- **printfデバッグ**: シリアル出力で状態を確認
- **LEDインジケーター**: 状態表示にLEDを使用
- **ログレベル**: デバッグ/情報/警告/エラーのレベル分け
- **アサーション**: `assert()`で前提条件をチェック

---

## デバッグ方法

### 1. シリアル出力デバッグ

```c
#include "pico/stdio.h"

stdio_init_all();
printf("Debug: value = %d\n", value);
```

### 2. LEDインジケーター

```c
#include "hardware/gpio.h"

#define LED_PIN 25

gpio_init(LED_PIN);
gpio_set_dir(LED_PIN, GPIO_OUT);

// 点滅で状態を表示
gpio_put(LED_PIN, 1);
sleep_ms(100);
gpio_put(LED_PIN, 0);
```

### 3. GDBデバッグ（SWD経由）

- **OpenOCD**: デバッグプローブ経由でデバッグ
- **VS Code統合**: VS Codeのデバッグ機能を使用
- **ブレークポイント**: コード実行を一時停止

### 4. ログシステムの実装

```c
#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO  1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_ERROR 3

#define CURRENT_LOG_LEVEL LOG_LEVEL_DEBUG

#define LOG(level, fmt, ...) \
    do { \
        if (level >= CURRENT_LOG_LEVEL) { \
            printf("[%s] " fmt "\n", #level, ##__VA_ARGS__); \
        } \
    } while(0)

LOG(LOG_LEVEL_DEBUG, "Debug message: %d", value);
```

---

## トラブルシューティング

### ビルドエラー

**問題**: `pico_sdk_import.cmake`が見つからない
- **解決**: `PICO_SDK_PATH`環境変数を確認

**問題**: コンパイラが見つからない
- **解決**: ARM GCCのパスを`PATH`に追加

**問題**: CMakeのバージョンが古い
- **解決**: CMake 3.13以上にアップグレード

### フラッシュエラー

**問題**: UF2ファイルをコピーしても動作しない
- **解決**: BOOTSELモードで正しく認識されているか確認
- **解決**: UF2ファイルが正しく生成されているか確認

**問題**: プログラムが起動しない
- **解決**: シリアル出力でエラーメッセージを確認
- **解決**: ハードウェア接続を確認

### 実行時エラー

**問題**: メモリ不足
- **解決**: 大きな配列を静的メモリに移動
- **解決**: 不要な変数を削除

**問題**: GPIOが動作しない
- **解決**: ピン番号が正しいか確認
- **解決**: 初期化が正しく行われているか確認

**問題**: WiFiが接続できない
- **解決**: SSIDとパスワードが正しいか確認
- **解決**: WiFi初期化が完了しているか確認

---

## 参考リソース

- [Raspberry Pi Pico C/C++ SDK Documentation](https://datasheets.raspberrypi.com/pico/raspberry-pi-pico-c-sdk.pdf)
- [Pico SDK Examples](https://github.com/raspberrypi/pico-examples)
- [Pico SDK API Reference](https://www.raspberrypi.com/documentation/pico-sdk/)
- [Raspberry Pi Pico 2W Datasheet](https://datasheets.raspberrypi.com/pico/pico2w-datasheet.pdf)

---

## 更新履歴

- 2026-02-18: 初版作成
