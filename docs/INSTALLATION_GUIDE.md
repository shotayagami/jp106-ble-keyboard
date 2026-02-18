# Raspberry Pi Pico 2W 開発環境セットアップガイド（Windows）

このガイドでは、Windows環境でRaspberry Pi Pico 2Wの開発環境をセットアップする手順を説明します。

## 目次

1. [必要なツール](#必要なツール)
2. [CMakeのインストール](#cmakeのインストール)
3. [ARM GCCコンパイラのインストール](#arm-gccコンパイラのインストール)
4. [Python 3のインストール](#python-3のインストール)
5. [Raspberry Pi Pico SDKのインストール](#raspberry-pi-pico-sdkのインストール)
6. [環境変数の設定](#環境変数の設定)
7. [動作確認](#動作確認)

---

## 必要なツール

以下のツールが必要です：

1. **CMake** (バージョン 3.13以上) - ビルドシステム
2. **ARM GCC コンパイラ** - クロスコンパイラ
3. **Python 3** (バージョン 3.6以上) - Pico SDKのビルドツール用
4. **Git** - SDKのダウンロード用
5. **Raspberry Pi Pico SDK** - 開発SDK

---

## CMakeのインストール

### 方法1: インストーラーを使用（推奨）

1. **CMakeのダウンロード**
   - https://cmake.org/download/ にアクセス
   - "Windows x64 Installer"をダウンロード
   - 最新の安定版（例: cmake-3.xx.x-windows-x86_64.msi）を選択

2. **インストール**
   - ダウンロードしたインストーラーを実行
   - 「Add CMake to the system PATH for all users」または「Add CMake to the system PATH for current user」を選択（重要）
   - インストールを完了

3. **確認**
   ```powershell
   cmake --version
   ```
   バージョンが表示されればOKです。

### 方法2: Chocolateyを使用

Chocolateyがインストールされている場合：

```powershell
choco install cmake
```

### 方法3: Scoopを使用

Scoopがインストールされている場合：

```powershell
scoop install cmake
```

### トラブルシューティング

**問題**: `cmake`コマンドが認識されない
- **解決**: PowerShellを再起動するか、環境変数PATHを確認
- **解決**: インストール時にPATHへの追加を選択し忘れた場合は、手動で追加
  - `C:\Program Files\CMake\bin`をPATHに追加

---

## ARM GCCコンパイラのインストール

### 方法1: 公式インストーラーを使用（推奨）

1. **ARM GCCのダウンロード**
   - https://developer.arm.com/downloads/-/gnu-rm にアクセス
   - 最新版をダウンロード（例: `gcc-arm-none-eabi-10.3-2021.10-win32.zip`）
   - または、より新しいバージョンがあればそれを選択

2. **インストール**
   - ZIPファイルを解凍
   - 適切な場所に配置（例: `C:\Program Files\ARM\gcc-arm-none-eabi-10.3-2021.10`）
   - フォルダ名にスペースが含まれない場所を推奨

3. **PATHに追加**
   - 解凍したフォルダの`bin`ディレクトリをPATHに追加
   - 例: `C:\Program Files\ARM\gcc-arm-none-eabi-10.3-2021.10\bin`

### 方法2: Chocolateyを使用

```powershell
choco install gcc-arm-embedded
```

### 方法3: Scoopを使用

```powershell
scoop install gcc-arm-none-eabi
```

### 確認

```powershell
arm-none-eabi-gcc --version
```

バージョン情報が表示されればOKです。

---

## Python 3のインストール

### 方法1: 公式インストーラーを使用（推奨）

1. **Pythonのダウンロード**
   - https://www.python.org/downloads/ にアクセス
   - 最新のPython 3.xをダウンロード（Python 3.8以上を推奨）

2. **インストール**
   - インストーラーを実行
   - **重要**: 「Add Python to PATH」にチェックを入れる
   - 「Install Now」をクリック

3. **確認**
   ```powershell
   python --version
   ```
   または
   ```powershell
   python3 --version
   ```

### 方法2: Microsoft Storeを使用

1. Microsoft Storeを開く
2. "Python 3.11"または"Python 3.12"を検索
3. インストール

### トラブルシューティング

**問題**: `python`コマンドが認識されない
- **解決**: PowerShellを再起動
- **解決**: 環境変数PATHを確認（`C:\Users\<ユーザー名>\AppData\Local\Programs\Python\Python3x\`と`C:\Users\<ユーザー名>\AppData\Local\Programs\Python\Python3x\Scripts\`が含まれているか）

---

## Gitのインストール

Gitがインストールされていない場合：

### 方法1: 公式インストーラーを使用

1. **Gitのダウンロード**
   - https://git-scm.com/download/win にアクセス
   - インストーラーをダウンロード

2. **インストール**
   - インストーラーを実行
   - デフォルト設定でインストール（PATHへの追加が自動で行われます）

### 方法2: Chocolateyを使用

```powershell
choco install git
```

### 確認

```powershell
git --version
```

---

## Raspberry Pi Pico SDKのインストール

### 1. SDKのクローン

適切な場所（例: `C:\pico`）にSDKをクローンします：

```powershell
# 作業ディレクトリを作成
mkdir C:\pico
cd C:\pico

# Pico SDKをクローン
git clone https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk

# サブモジュールを初期化
git submodule update --init
```

### 2. 環境変数の設定

Pico SDKのパスを環境変数に設定します。

#### 方法1: PowerShellで一時的に設定（現在のセッションのみ）

```powershell
$env:PICO_SDK_PATH = "C:\pico\pico-sdk"
```

#### 方法2: システム環境変数に永続的に設定（推奨）

1. **環境変数の設定画面を開く**
   - Windowsキー + R を押す
   - `sysdm.cpl`と入力してEnter
   - 「詳細設定」タブ → 「環境変数」をクリック

2. **ユーザー環境変数を追加**
   - 「ユーザー環境変数」セクションで「新規」をクリック
   - 変数名: `PICO_SDK_PATH`
   - 変数値: `C:\pico\pico-sdk`（実際のパスに合わせて変更）
   - 「OK」をクリック

3. **PowerShellを再起動**
   - 環境変数の変更を反映するため、PowerShellを再起動

#### 方法3: PowerShellプロファイルで設定

```powershell
# プロファイルを編集
notepad $PROFILE

# 以下の行を追加
$env:PICO_SDK_PATH = "C:\pico\pico-sdk"
```

### 3. 確認

```powershell
echo $env:PICO_SDK_PATH
```

設定したパスが表示されればOKです。

---

## 環境変数の設定（まとめ）

以下の環境変数が設定されていることを確認してください：

| 環境変数 | 説明 | 例 |
|---------|------|-----|
| `PICO_SDK_PATH` | Pico SDKのパス | `C:\pico\pico-sdk` |
| `PATH` | 実行ファイルの検索パス | CMake、ARM GCC、Pythonのbinディレクトリが含まれる |

### PATHに追加する必要があるディレクトリ

- CMake: `C:\Program Files\CMake\bin`
- ARM GCC: `C:\Program Files\ARM\gcc-arm-none-eabi-10.3-2021.10\bin`（インストール場所に応じて変更）
- Python: `C:\Users\<ユーザー名>\AppData\Local\Programs\Python\Python3x\`と`C:\Users\<ユーザー名>\AppData\Local\Programs\Python\Python3x\Scripts\`

---

## 動作確認

### 1. 各ツールのバージョン確認

```powershell
# CMake
cmake --version

# ARM GCC
arm-none-eabi-gcc --version

# Python
python --version

# Git
git --version

# Pico SDKパス
echo $env:PICO_SDK_PATH
```

すべてのコマンドが正常に実行されれば、環境セットアップは完了です。

### 2. プロジェクトのビルドテスト

プロジェクトディレクトリで以下を実行：

```powershell
cd y:\git\RaspberryPiPico2Wproj
mkdir build
cd build
cmake ..
```

エラーなく完了すれば、環境は正しく設定されています。

---

## トラブルシューティング

### CMakeが見つからない

**症状**: `cmake: command not found`または`cmake は認識されません`

**解決方法**:
1. PowerShellを再起動
2. 環境変数PATHにCMakeのbinディレクトリが含まれているか確認
3. 手動でPATHに追加:
   ```powershell
   $env:PATH += ";C:\Program Files\CMake\bin"
   ```

### ARM GCCが見つからない

**症状**: `arm-none-eabi-gcc: command not found`

**解決方法**:
1. ARM GCCが正しくインストールされているか確認
2. PATHにbinディレクトリを追加:
   ```powershell
   $env:PATH += ";C:\Program Files\ARM\gcc-arm-none-eabi-10.3-2021.10\bin"
   ```

### PICO_SDK_PATHが見つからない

**症状**: CMake実行時に`Pico SDK not found`エラー

**解決方法**:
1. 環境変数が設定されているか確認:
   ```powershell
   echo $env:PICO_SDK_PATH
   ```
2. 設定されていない場合は設定:
   ```powershell
   $env:PICO_SDK_PATH = "C:\pico\pico-sdk"
   ```
3. パスが正しいか確認（`pico_sdk_init.cmake`ファイルが存在するか）

### Gitの所有権エラー（ネットワークドライブ）

**症状**: `fatal: detected dubious ownership in repository`エラー

**原因**: ネットワークドライブ上のリポジトリで、現在のユーザーとリポジトリの所有者が異なる場合に発生します。

**解決方法1: 安全なディレクトリとして追加（推奨）**

エラーメッセージに表示されたコマンドを実行します：

```powershell
git config --global --add safe.directory '%(prefix)///OPENMEDIAVAULT/yagamin/git/RaspberryPiPico2Wproj/pico-sdk'
```

**解決方法2: すべてのディレクトリを安全として設定（注意が必要）**

すべてのディレクトリを安全として設定する場合（セキュリティリスクあり）：

```powershell
git config --global --add safe.directory '*'
```

**解決方法3: ローカルディレクトリにクローン（推奨）**

ネットワークドライブではなく、ローカルディレクトリ（例: `C:\pico`）にクローンすることを推奨します：

```powershell
# ローカルディレクトリにクローン
cd C:\
mkdir pico
cd pico
git clone https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init
```

その後、環境変数をローカルのパスに設定：

```powershell
$env:PICO_SDK_PATH = "C:\pico\pico-sdk"
```

### Pythonが見つからない

**症状**: `python: command not found`

**解決方法**:
1. Pythonがインストールされているか確認
2. PATHにPythonのディレクトリを追加
3. `py`コマンドを試す（Windows版Pythonでは`py`コマンドも使用可能）

---

## 次のステップ

環境セットアップが完了したら、以下を参照してください：

- [開発ガイド](DEVELOPMENT_GUIDE.md) - 開発の進め方とベストプラクティス
- [README.md](../README.md) - プロジェクトの概要とクイックスタート

---

## 参考リンク

- [CMake公式サイト](https://cmake.org/)
- [ARM GCC公式サイト](https://developer.arm.com/downloads/-/gnu-rm)
- [Python公式サイト](https://www.python.org/)
- [Git公式サイト](https://git-scm.com/)
- [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk)

---

## 更新履歴

- 2026-02-18: 初版作成
