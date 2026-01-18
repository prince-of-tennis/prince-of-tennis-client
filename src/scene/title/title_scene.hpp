#pragma once

#include "core/context.hpp"
#include "joycon/joycon.hpp"
#include "opengl/2d/EZ_2d.h"

// タイトル表示設定
constexpr float TITLE_FONT_SIZE = 72.0f;
constexpr float START_MSG_FONT_SIZE = 32.0f;

// 点滅間隔（フレーム数）
constexpr int BLINK_INTERVAL = 30;

struct TitleScene
{
    Context *context;
    Joycon *joycon;
    EZ_2D_Font font;
    int blink_counter;
    bool show_start_message;
};

/// @brief タイトルシーンの初期化
bool title_scene_init(TitleScene *scene);

/// @brief タイトルシーンの更新
/// @return trueならゲームシーンに遷移する
bool title_scene_update(TitleScene *scene);

/// @brief タイトルシーンの描画
void title_scene_draw(TitleScene *scene);

/// @brief タイトルシーンの終了処理
void title_scene_fini(TitleScene *scene);
