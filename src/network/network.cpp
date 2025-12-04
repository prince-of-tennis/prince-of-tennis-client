#include "network/network.hpp"

#include "common/Gamestatus.h"
#include "util/log.hpp"

bool network_init(Network *network, Context *context)
{
    if (SDLNet_Init() == -1)
    {
        LOG_DEBUG("SDLNetの初期化に失敗しました: " << SDLNet_GetError());
        return false;
    }

    if (SDLNet_ResolveHost(&network->ip_address, context->network_host.c_str(),
                           context->network_port) == -1)
    {
        LOG_ERROR("名前解決に失敗しました");
        return false;
    }

    LOG_DEBUG("名前解決完了");

    network->socket = SDLNet_TCP_Open(&network->ip_address);
    if (!network->socket)
    {
        LOG_ERROR("サーバに接続できませんでした");
        return false;
    }

    LOG_DEBUG("サーバに接続完了");

    // 監視対象に追加
    network->socket_set = SDLNet_AllocSocketSet(1);
    SDLNet_TCP_AddSocket(network->socket_set, network->socket);

    LOG_DEBUG("ソケット通信の初期化完了");

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
        Gamestatus status;
        int len = SDLNet_TCP_Recv(network->socket, &status, sizeof(Gamestatus));

        if (len > 0)
        {
            LOG_DEBUG("通信完了");
        }
        else
        {
            LOG_ERROR("サーバーとの接続が切れました");
            return false;
        }
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