# これはPico SDKのインポート用ファイルです
# PICO_SDK_PATH環境変数が設定されている場合、そのパスからSDKをインポートします
# 設定されていない場合は、このファイルと同じディレクトリにpico-sdkがあることを期待します

# PICO_SDK_PATHが設定されているか確認
if (NOT PICO_SDK_PATH)
    # 環境変数から読み取る
    if (DEFINED ENV{PICO_SDK_PATH})
        set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
    else()
        # 環境変数が設定されていない場合、このファイルのディレクトリを基準に探す
        get_filename_component(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/../pico-sdk" ABSOLUTE)
    endif()
endif()

# SDKのインポートファイルを読み込む
if (EXISTS "${PICO_SDK_PATH}/pico_sdk_init.cmake")
    include("${PICO_SDK_PATH}/pico_sdk_init.cmake")
else()
    message(FATAL_ERROR "Pico SDK not found. Please set PICO_SDK_PATH environment variable or place pico-sdk in the expected location.")
endif()
