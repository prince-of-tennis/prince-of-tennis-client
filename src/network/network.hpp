#pragma once

#include <SDL2/SDL_net.h>

#include "common/packet.h"
#include "core/context.hpp"
#include "network/network_data_set.h"
struct Network
{
    IPaddress ip_address;
    TCPsocket socket;
    SDLNet_SocketSet socket_set;

    NetworkDataSet network_data_set;
};

/// @brief ネットワーク関連の初期化
/// @param network 構造体
/// @param context Context
/// @return 成功したか
bool network_init(Network *network, Context *context);

/// @brief サーバにListen(毎フレーム実行)
bool network_listen_to_server(Network *network);

/// @brief サーバにパケットを送信
/// @param network 構造体
/// @param packet 送信するパケット
/// @return 成功したか
bool network_send_to_server(Network *network, Packet packet);

/// @brief 受信したパケットの処理
/// @param network 構造体
/// @param packet 受信したパケット
bool network_handle_packet(Network *network, Packet packet);

/// @brief ネットワーク関連の終了処理
/// @param network 構造体
void network_fini(Network *network);
