#pragma once

#include "ability/ability_manager.hpp"
#include "audio/audio.hpp"
#include "common/GamePhase.h"
#include "common/GameScore.h"
#include "common/ball.h"
#include "common/player.h"
#include "common/player_input.h"
#include "common/player_swing.h"
#include "connection/connection_manager.hpp"
#include "core/context.hpp"
#include "network/network.hpp"
#include "opengl/2d/EZ_2d.h"
#include "opengl/camera/EZ_Camera.hpp"
#include "opengl/light/EZ_Light.hpp"
#include "opengl/object/EZ_Object.hpp"
#include "opengl/shader/EZ_Shader.hpp"
#include "util/helper.hpp"

// 前方宣言
struct Joycon;

// ゲームシーンの結果
enum GameSceneResult
{
    GAME_RESULT_CONTINUE,      // ゲーム継続
    GAME_RESULT_FINISHED,      // ゲーム終了
    GAME_RESULT_RETURN_TITLE,  // タイトルに戻る
    GAME_RESULT_NETWORK_ERROR  // ネットワークエラー
};

#define PLAYER_MAX 2

// カメラ設定（Blenderから変換）
// Blender座標: (-0.015194m, -67.5868m, 13.8167m)
// Blender回転: (77.3446°, -0.271264°, 1.10547°)
// BlenderはY-up/Z-forward、OpenGLはY-up/-Z-forward
// 変換ルール: X_gl = X_bl, Y_gl = Z_bl, Z_gl = -Y_bl
constexpr float CAMERA_POS_X = -0.015194f;
constexpr float CAMERA_POS_Y = 19.5f;  // 少し上から見る角度
constexpr float CAMERA_POS_Z = 32.0f;  // 適度な距離
constexpr float CAMERA_TARGET_X = 0.0f;
constexpr float CAMERA_TARGET_Y = 3.0f;  // 注視点
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
// セット数表示設定
constexpr float SET_SCORE_OFFSET_Y = 40.0f;  // ポイントスコアからのY方向オフセット（上寄り）
constexpr float SET_SCORE_FONT_SIZE = 30.0f;
constexpr float SET_SCORE_OFFSET_X = 50.0f;  // 中央からのオフセット（少し中央寄り）

struct GameScene
{
    EZ_Shader shader;
    EZ_Camera camera;
    EZ_Light light;
    Context *context;
    Joycon *joycon;

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
    GameScore prev_game_score;  // 前フレームのスコア（変化検出用）

    // ボール打撃検出用
    int prev_hit_count;  // 前フレームのhit_count

    // オーディオ
    Audio audio;
    int se_hit_ball;  // ボールを打った時のSE
    int se_yatta;     // 得点時のSE

    // 能力マネージャー
    AbilityManager ability_manager;

    // ゲーム終了関連
    bool is_game_finished;    // 試合終了フラグ
    int winner_id;            // 勝者のプレイヤーID（-1: 未確定）
    GamePhase current_phase;  // 現在のゲームフェーズ

    // 接続管理
    ConnectionManager *connection_manager;
    bool *joycon_initialized_ptr;  // SceneManagerのjoycon_initializedへのポインタ
    bool network_error;          // ネットワークエラーフラグ
    int network_error_counter;   // エラー表示カウンター
};

/// @brief ゲームシーンの初期化
/// @param scene ゲームシーン
/// @param network SceneManager が所有する Network（マッチング済み）
bool game_scene_init(GameScene *scene, Network *network);

/// @brief ゲームシーンの終了処理
/// @param scene ゲームシーン
void game_scene_fini(GameScene *scene);

GameSceneResult game_scene_update(GameScene *scene, PlayerInput *player_input,
                                  PlayerSwing *player_swing);

void game_scene_draw(GameScene *scene);

// スコア描画用ヘルパー関数
void draw_score(GameScene *scene);
