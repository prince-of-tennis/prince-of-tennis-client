#include "game_scene_network.hpp"

#include "network/packet_util.hpp"
#include "game_scene.hpp"
#include "network/network.hpp"
#include "util/log.hpp"

void game_scene_send_player_input(GameScene *scene, PlayerInput *player_input)
{
    if (player_input == nullptr)
    {
        return;
    }

    PlayerInput adjusted_input = *player_input;

    // player_id == 1 の場合、カメラが反対側なので入力方向を反転
    // これにより、プレイヤーの主観的な「前」「左」が正しくゲーム内に反映される
    if (scene->context->player_id == 1)
    {
        // front/back を入れ替え
        bool temp_front = adjusted_input.front;
        adjusted_input.front = adjusted_input.back;
        adjusted_input.back = temp_front;

        // left/right を入れ替え
        bool temp_left = adjusted_input.left;
        adjusted_input.left = adjusted_input.right;
        adjusted_input.right = temp_left;
    }

    Packet packet;
    if (!packet_serialize(&packet, PACKET_TYPE_PLAYER_INPUT, adjusted_input))
    {
        LOG_ERROR("プレイヤー入力のシリアライズに失敗");
        return;
    }
    network_send_to_server(scene->network.get(), &packet);
}

void game_scene_send_player_swing(GameScene *scene, PlayerSwing *player_swing)
{
    if (player_swing == nullptr)
    {
        return;
    }

    PlayerSwing adjusted_swing = *player_swing;

    // プレイヤーIDに応じてJoy-Conの座標系をゲーム座標系に変換
    if (scene->context->player_id == 0)
    {
        // player_id == 0: Z軸を反転（Joy-Con座標系とゲーム座標系の違いを補正）
        adjusted_swing.acc_z = -adjusted_swing.acc_z;
    }
    else if (scene->context->player_id == 1)
    {
        // player_id == 1: カメラが反対側なのでX軸を反転（Z軸はそのまま）
        adjusted_swing.acc_x = -adjusted_swing.acc_x;
    }

    Packet packet;
    if (!packet_serialize(&packet, PACKET_TYPE_PLAYER_SWING, adjusted_swing))
    {
        LOG_ERROR("スイングデータのシリアライズに失敗");
        return;
    }
    network_send_to_server(scene->network.get(), &packet);
}
