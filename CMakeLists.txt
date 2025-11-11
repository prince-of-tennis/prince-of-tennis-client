# 1. 基本設定
# -----------------
cmake_minimum_required(VERSION 3.16)
project(SuperDimensionalTennisClient CXX)

# C++17規格を必須にする
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. ライブラリの検索
# -----------------
# システム (apt-get) でインストールしたライブラリを探す
# (成功すると SDL2_INCLUDE_DIRS と SDL2_LIBRARIES 変数が設定される)
find_package(SDL2 REQUIRED)

# 3. ソースファイルの定義
# -----------------
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c")

# 4. 実行ファイルの定義
# -----------------
add_executable(client ${SOURCES})

# 5. インクルードパスの指定 (★修正箇所)
# -----------------
target_include_directories(client PRIVATE
    "src"
    ${SDL2_INCLUDE_DIRS} # SDL2のヘッダファイルへのパスを追加
)

# 6. ライブラリのリンク (★修正箇所)
# -----------------
target_link_libraries(client PRIVATE

    # --- システムライブラリ ---
    ${SDL2_LIBRARIES} # SDL2::SDL2 の代わりにこの変数を使う
)

# (オプション) アセットフォルダを実行ファイルと同じ場所にコピー
file(COPY assets/ DESTINATION ${CMAKE_BINARY_DIR})