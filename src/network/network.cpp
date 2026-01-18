#include "network/network.hpp"

#include "common/packet.h"
#include "network/packet_util.hpp"
#include "util/log.hpp"

// プレイヤー数の最大値
constexpr int NETWORK_PLAYER_MAX = 2;

bool network_init(Network *network, Context *context)
{
    network->context = context;

    if (SDLNet_Init() == -1)
    {
        LOG_ERROR("SDLNetの初期化に失敗しました: " << SDLNet_GetError());
        return false;
    }

    if (SDLNet_ResolveHost(&network->ip_address, context->network_host.c_str(),
                           context->network_port) == -1)
    {
        LOG_ERROR("名前解決に失敗しました");
        return false;
    }

    LOG_SUCCESS("名前解決完了");

    network->socket = SDLNet_TCP_Open(&network->ip_address);
    if (!network->socket)
    {
        LOG_ERROR("サーバに接続できませんでした");
        return false;
    }

    LOG_SUCCESS("サーバに接続完了");

    // 監視対象に追加
    network->socket_set = SDLNet_AllocSocketSet(1);
    SDLNet_TCP_AddSocket(network->socket_set, network->socket);

    LOG_SUCCESS("ソケット通信の初期化完了");

    return true;
}

bool network_listen_to_server(Network *network)
{
    while (SDLNet_CheckSockets(network->socket_set, 0) > 0)
    {
        Packet packet;
        if (SDLNet_TCP_Recv(network->socket, &packet, sizeof(Packet)) > 0)
        {
            // パケットを処理
            network_handle_packet(network, packet);
        }
        else
        {
            LOG_ERROR("サーバーとの通信に失敗しました: " << SDLNet_GetError());
            return false;
        }
    }

    return true;
}

bool network_handle_packet(Network *network, Packet packet)
{
    switch (packet.type)
    {
        // プレイヤーID割り当て: サーバーから自分のプレイヤーIDを受信
        case PACKET_TYPE_SET_PLAYER_ID:
        {
            int player_id;
            if (!packet_deserialize(packet, player_id))
            {
                LOG_ERROR("プレイヤーIDのデシリアライズに失敗");
                return false;
            }
            network->context->player_id = player_id;
            LOG_SUCCESS("プレイヤーID設定: " << player_id);
            break;
        }

        // ゲームフェーズ更新: ゲームの進行状態（サーブ待機、プレイ中など）
        case PACKET_TYPE_GAME_PHASE:
        {
            GamePhase game_phase;
            if (!packet_deserialize(packet, game_phase))
            {
                LOG_ERROR("ゲームフェーズのデシリアライズに失敗");
                return false;
            }
            LOG_DEBUG("GamePhase: " << game_phase);
            network->network_data_set.game_phase = game_phase;
            break;
        }

        // ボール状態更新: ボールの位置・速度情報
        case PACKET_TYPE_BALL_STATE:
        {
            Ball ball;
            if (!packet_deserialize(packet, ball))
            {
                LOG_ERROR("ボール状態のデシリアライズに失敗");
                return false;
            }
            network->network_data_set.ball = ball;
            break;
        }

        // プレイヤー状態更新: 各プレイヤーの位置情報
        case PACKET_TYPE_PLAYER_STATE:
        {
            Player player;
            if (!packet_deserialize(packet, player))
            {
                LOG_ERROR("プレイヤー状態のデシリアライズに失敗");
                return false;
            }

            // player_idの範囲チェック
            if (player.player_id < 0 || player.player_id >= NETWORK_PLAYER_MAX)
            {
                LOG_ERROR("不正なplayer_idを受信: " << player.player_id);
                return false;
            }
            // プレイヤーデータを更新
            network->network_data_set.players[player.player_id] = player;
            break;
        }

        // スコア更新: ゲームスコア情報
        case PACKET_TYPE_SCORE_UPDATE:
        {
            GameScore game_score;
            if (!packet_deserialize(packet, game_score))
            {
                LOG_ERROR("スコアデータのデシリアライズに失敗");
                return false;
            }
            network->network_data_set.game_score = game_score;
            break;
        }
    }

    return true;
}

bool network_send_to_server(Network *network, Packet *packet)
{
    if (SDLNet_TCP_Send(network->socket, packet, sizeof(Packet)) < sizeof(Packet))
    {
        LOG_ERROR("サーバーへの送信に失敗しました: " << SDLNet_GetError());
        return false;
    }

    return true;
}

void network_fini(Network *network)
{
    if (network->socket)
    {
        SDLNet_TCP_Close(network->socket);
        network->socket = nullptr;
    }

    if (network->socket_set)
    {
        SDLNet_FreeSocketSet(network->socket_set);
        network->socket_set = nullptr;
    }

    SDLNet_Quit();
}