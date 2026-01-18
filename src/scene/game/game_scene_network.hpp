#pragma once

#include "common/player_input.h"
#include "common/player_swing.h"

// 前方宣言
struct GameScene;

/**
 * @brief プレイヤー入力をサーバーに送信
 *
 * Joy-Conからの入力データをサーバーに送信する。
 * player_id == 1 の場合、カメラが反対側を向いているため入力方向を反転:
 * - front/back を入れ替え
 * - left/right を入れ替え
 *
 * @param scene ゲームシーン
 * @param player_input プレイヤー入力（nullptrの場合は送信しない）
 */
void game_scene_send_player_input(GameScene *scene, PlayerInput *player_input);

/**
 * @brief プレイヤーのスイングデータをサーバーに送信
 *
 * Joy-Conの加速度データから検出したスイングをサーバーに送信する。
 * プレイヤーIDに応じて座標系を補正:
 * - player_id == 0: Z軸を反転（Joy-Con座標系→ゲーム座標系）
 * - player_id == 1: X軸を反転（カメラ反対側の補正）
 *
 * @param scene ゲームシーン
 * @param player_swing スイングデータ（nullptrの場合は送信しない）
 */
void game_scene_send_player_swing(GameScene *scene, PlayerSwing *player_swing);
