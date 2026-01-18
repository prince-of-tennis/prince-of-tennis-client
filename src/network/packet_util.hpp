#pragma once

#include <cstddef>
#include <cstring>
#include <type_traits>

#include "common/packet.h"

/**
 * @brief パケットデータのシリアライズ/デシリアライズ用ユーティリティ
 *
 * memcpy を直接使用する代わりに、サイズチェック付きの型安全な関数を提供
 */

/**
 * @brief データをパケットにシリアライズ
 * @tparam T シリアライズするデータ型（POD型である必要がある）
 * @param packet 書き込み先パケット
 * @param type パケットタイプ
 * @param data シリアライズするデータ
 * @return 成功時 true、失敗時 false
 */
template <typename T>
bool packet_serialize(Packet *packet, PacketType type, const T &data)
{
    static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");

    if (packet == nullptr)
    {
        return false;
    }

    // バッファオーバーフローチェック
    if (sizeof(T) > PACKET_MAX_SIZE)
    {
        return false;
    }

    packet->type = type;
    packet->size = static_cast<uint32_t>(sizeof(T));
    std::memcpy(packet->data, &data, sizeof(T));

    return true;
}

/**
 * @brief パケットからデータをデシリアライズ
 * @tparam T デシリアライズするデータ型（POD型である必要がある）
 * @param packet 読み取り元パケット
 * @param data デシリアライズ先
 * @return 成功時 true、失敗時 false
 */
template <typename T>
bool packet_deserialize(const Packet &packet, T &data)
{
    static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");

    // サイズ検証: パケットサイズが期待する型のサイズと一致するか
    if (packet.size != sizeof(T))
    {
        return false;
    }

    // バッファオーバーフローチェック
    if (packet.size > PACKET_MAX_SIZE)
    {
        return false;
    }

    std::memcpy(&data, packet.data, sizeof(T));

    return true;
}
