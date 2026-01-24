#pragma once

#include "audio/audio.hpp"
#include "core/context.hpp"
#include "joycon/joycon.hpp"
#include "network/network.hpp"
#include "opengl/2d/EZ_2d.h"
#include "opengl/camera/EZ_Camera.hpp"
#include "opengl/light/EZ_Light.hpp"
#include "opengl/object/EZ_Object.hpp"
#include "opengl/shader/EZ_Shader.hpp"

// マッチングフェーズ
enum MatchingPhase
{
    MATCHING_PHASE_JOYCON,   // Joy-Con接続中
    MATCHING_PHASE_SERVER,   // サーバー接続中
    MATCHING_PHASE_WAITING,  // マッチング待機中
    MATCHING_PHASE_COMPLETE  // 完了
};

// マッチング結果
enum MatchingResult
{
    MATCHING_RESULT_NONE,     // 継続
    MATCHING_RESULT_SUCCESS,  // 成功→ゲームへ
    MATCHING_RESULT_CANCEL    // キャンセル→タイトルへ
};

// リトライ待機フレーム（3秒 = 180フレーム）
constexpr int RETRY_WAIT_FRAMES = 180;
// 完了表示フレーム（1秒 = 60フレーム）
constexpr int COMPLETE_DISPLAY_FRAMES = 60;
// ドットアニメーション間隔
constexpr int DOT_ANIMATION_INTERVAL = 20;

struct MatchingScene
{
    Context *context;
    Joycon *joycon;
    Network *network;  // SceneManager が所有、ポインタで参照
    EZ_2D_Font font;

    // 3D 背景（タイトルと同じ）
    EZ_Shader shader;
    EZ_Camera camera;
    EZ_Light light;
    EZ_Object court_object;
    float court_rotation;

    // 接続状態
    MatchingPhase phase;
    int retry_count;
    int retry_wait_frames;     // リトライまでの待機フレーム
    bool last_attempt_failed;  // 前回の試行が失敗したか
    const char *status_message;

    // アニメーション用
    int dot_animation_counter;  // 「...」のアニメーション
    int complete_counter;       // 完了表示用カウンター

    // オーディオ
    Audio audio;
    int se_decide;

    // 初期化フラグ（SceneManager から参照）
    bool *joycon_initialized_ptr;
    bool *network_initialized_ptr;
};

/// @brief マッチングシーンの初期化
bool matching_scene_init(MatchingScene *scene);

/// @brief マッチングシーンの更新
MatchingResult matching_scene_update(MatchingScene *scene);

/// @brief マッチングシーンの描画
void matching_scene_draw(MatchingScene *scene);

/// @brief マッチングシーンの終了処理
void matching_scene_fini(MatchingScene *scene);
