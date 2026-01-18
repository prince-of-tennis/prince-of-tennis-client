#pragma once

#include "common/GameScore.h"
#include "common/ball.h"
#include "common/player.h"
#include "common/player_input.h"
#include "common/player_swing.h"
#include "core/context.hpp"
#include "network/network.hpp"
#include "opengl/2d/EZ_2d.h"
#include "opengl/camera/EZ_Camera.hpp"
#include "opengl/light/EZ_Light.hpp"
#include "opengl/object/EZ_Object.hpp"
#include "opengl/shader/EZ_Shader.hpp"
#include "util/helper.hpp"

#define PLAYER_MAX 2

// カメラ設定（Blenderから変換）
// Blender座標: (-0.015194m, -67.5868m, 13.8167m)
// Blender回転: (77.3446°, -0.271264°, 1.10547°)
// BlenderはY-up/Z-forward、OpenGLはY-up/-Z-forward
// 変換ルール: X_gl = X_bl, Y_gl = Z_bl, Z_gl = -Y_bl
constexpr float CAMERA_POS_X = -0.015194f;
constexpr float CAMERA_POS_Y = 13.8167f;
constexpr float CAMERA_POS_Z = 67.5868f;
constexpr float CAMERA_TARGET_X = 0.0f;
constexpr float CAMERA_TARGET_Y = 5.0f;
constexpr float CAMERA_TARGET_Z = 0.0f;

// プレイヤーのローカル移動速度
constexpr float PLAYER_MOVE_SPEED = 0.3f;
// サーバー補正を行うフレーム間隔
constexpr int SERVER_SYNC_INTERVAL = 60;  // 60フレーム（約1秒）に1回

// テニスコートのスケール（Blenderモデルの単位をゲーム単位に変換）
constexpr float TENNIS_COURT_SCALE = 0.511242f;

// プレイヤー初期座標（player_id 0 と 1）
constexpr float PLAYER_INITIAL_X[PLAYER_MAX] = {0.0f, 10.0f};
constexpr float PLAYER_INITIAL_Y = 0.0f;
constexpr float PLAYER_INITIAL_Z = 0.0f;

// スコア表示設定
constexpr float SCORE_OFFSET_X = 100.0f;  // 中央からのオフセット
constexpr float SCORE_POS_Y = 50.0f;
constexpr float SCORE_FONT_SIZE = 60.0f;

struct GameScene
{
    EZ_Shader shader;
    EZ_Camera camera;
    EZ_Light light;
    Context *context;

    UniquePtr<Network> network;

    // ボール
    EZ_Object ball_object;
    Ball ball_data;

    // プレイヤー
    EZ_Object player_objects[PLAYER_MAX];
    Player player_data[PLAYER_MAX];

    // テニスコート
    EZ_Object court_object;

    // グラウンド
    EZ_Object ground_object;

    // 前フレームの入力（変化検出用）
    PlayerInput last_player_input;

    // サーバー同期用カウンター
    int server_sync_counter;

    // 2D描画用
    EZ_2D_Font font;
    EZ_2D_Image test_image;

    // スコアデータ
    GameScore game_score;
};

bool game_scene_init(GameScene *scene);

bool game_scene_update(GameScene *scene, PlayerInput *player_input, PlayerSwing *player_swing);

void game_scene_draw(GameScene *scene);

// スコア描画用ヘルパー関数
void draw_score(GameScene *scene);
