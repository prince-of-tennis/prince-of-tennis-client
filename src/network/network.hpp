#pragma once

#include <SDL2/SDL_net.h>

#include "core/context.hpp"

struct Network
{
    IPaddress ip_address;
    TCPsocket socket;
    SDLNet_SocketSet socket_set;
};

/// @brief ネットワーク関連の初期化
/// @param network 構造体
/// @param context Context
/// @return 成功したか
bool network_init(Network *network, Context *context);

/// @brief サーバにListen(毎フレーム実行)
bool network_listen_to_server(Network *network);

/// @brief ネットワーク関連の終了処理
/// @param network 構造体
void network_fini(Network *network);
