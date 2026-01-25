#pragma once

#include "audio/audio.hpp"
#include "connection/connection_manager.hpp"
#include "core/context.hpp"
#include "joycon/joycon.hpp"
#include "opengl/2d/EZ_2d.h"
#include "opengl/camera/EZ_Camera.hpp"
#include "opengl/light/EZ_Light.hpp"
#include "opengl/object/EZ_Object.hpp"
#include "opengl/shader/EZ_Shader.hpp"

// タイトル表示設定
constexpr float TITLE_FONT_SIZE = 72.0f;
constexpr float SUBTITLE_FONT_SIZE = 28.0f;
constexpr float MENU_FONT_SIZE = 36.0f;
constexpr float START_MSG_FONT_SIZE = 32.0f;

// アニメーション設定
constexpr int BLINK_INTERVAL = 30;
constexpr int FADE_IN_FRAMES = 60;
constexpr float COURT_ROTATION_SPEED = 0.2f;

// メニュー設定
constexpr int MENU_ITEM_COUNT = 2;
constexpr float MENU_START_Y = 500.0f;
constexpr float MENU_ITEM_SPACING = 60.0f;

// メニュー項目
enum TitleMenuItem
{
    MENU_START = 0,
    MENU_EXIT = 1
};

// タイトルシーンの結果
enum TitleSceneResult
{
    TITLE_RESULT_NONE = 0,
    TITLE_RESULT_START = 1,
    TITLE_RESULT_EXIT = 2
};

struct TitleScene
{
    Context *context;
    Joycon *joycon;
    EZ_2D_Font font;
    int blink_counter;
    bool show_start_message;

    // 3D背景
    EZ_Shader shader;
    EZ_Camera camera;
    EZ_Light light;
    EZ_Object court_object;
    float court_rotation;

    // アニメーション
    int frame_counter;
    float title_alpha;
    bool fade_in_complete;

    // メニュー
    int selected_menu;

    // スティック入力状態（エッジ検出用）
    bool prev_stick_up;
    bool prev_stick_down;

    // オーディオ
    Audio audio;
    int se_cursor_move;
    int se_decide;

    // 接続管理
    ConnectionManager *connection_manager;
    bool *joycon_initialized_ptr;
};

/// @brief タイトルシーンの初期化
bool title_scene_init(TitleScene *scene);

/// @brief タイトルシーンの更新
/// @return TitleSceneResultを返す
TitleSceneResult title_scene_update(TitleScene *scene);

/// @brief タイトルシーンの描画
void title_scene_draw(TitleScene *scene);

/// @brief タイトルシーンの終了処理
void title_scene_fini(TitleScene *scene);
