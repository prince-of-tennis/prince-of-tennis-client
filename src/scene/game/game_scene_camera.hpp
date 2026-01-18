#pragma once

// 前方宣言
struct GameScene;

/**
 * @brief カメラを自プレイヤーに追従させる
 *
 * プレイヤーの後方にカメラを配置し、プレイヤーを注視する。
 * プレイヤーIDに応じてカメラのZ方向が反転する:
 * - player_id == 0: コート奥側から見る（Z正方向）
 * - player_id == 1: コート手前側から見る（Z負方向）
 *
 * @param scene ゲームシーン
 */
void game_scene_update_camera(GameScene *scene);

/**
 * @brief カメラを初期位置に設定
 *
 * player_id受信後に呼び出し、プレイヤーの視点方向に応じて
 * カメラの初期位置を設定する。
 *
 * @param scene ゲームシーン
 */
void game_scene_init_camera_position(GameScene *scene);
