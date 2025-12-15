#include "network/network.hpp"

#include "common/packet.h"
#include "util/log.hpp"

bool network_init(Network *network, Context *context)
{
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
    int num_ready_socket = SDLNet_CheckSockets(network->socket_set, 0);
    if (num_ready_socket == -1)
    {
        LOG_ERROR("ソケットエラー");
        return false;
    }

    if (num_ready_socket > 0)
    {
        Packet packet;
        if (SDLNet_TCP_Recv(network->socket, &packet, sizeof(Packet)) >= sizeof(Packet))
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
        case PACKET_TYPE_GAME_PHASE:
            GamePhase game_phase;
            memcpy(&game_phase, packet.data, sizeof(GamePhase));
            LOG_DEBUG("GamePhase: " << game_phase);
            network->network_data_set.game_phase = game_phase;
            break;

        case PACKET_TYPE_BALL_STATE:
            Ball ball;
            memcpy(&ball, packet.data, sizeof(Ball));
            LOG_DEBUG("Ball受信: (" << ball.point.x << ", " << ball.point.y << ", " << ball.point.z
                                    << ")");
            network->network_data_set.ball = ball;
            break;

        default:
            LOG_ERROR("不正な値が送信されました: " << packet.type);
            return false;
    }

    return true;
}

bool network_send_to_server(Network *network, Packet packet)
{
    if (SDLNet_TCP_Send(network->socket, &packet, sizeof(Packet)) < sizeof(Packet))
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