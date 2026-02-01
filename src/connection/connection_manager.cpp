#include "connection_manager.hpp"

#include <string.h>

#include "util/log.hpp"

// ジョイコン接続スレッド関数
static int joycon_connect_thread_func(void *data)
{
    ConnectionManager *mgr = static_cast<ConnectionManager *>(data);
    constexpr int RETRY_DELAY_MS = 1000;  // リトライ間隔

    while (mgr->joycon_thread_running.load())
    {
        ConnectionState current_state = mgr->joycon_state.load();

        // 接続中、再接続リクエスト、または失敗状態の場合に接続を試行
        bool should_connect = (current_state == ConnectionState::CONNECTING) ||
                              (current_state == ConnectionState::FAILED) ||
                              mgr->reconnect_requested.load();

        if (should_connect)
        {
            mgr->reconnect_requested.store(false);
            mgr->joycon_state.store(ConnectionState::CONNECTING);

            LOG_DEBUG("ジョイコン接続を試行中...");

            // joyconlib_tをクリーンな状態にしてから接続を試行
            memset(&mgr->joycon->joycon, 0, sizeof(joyconlib_t));

            if (joycon_init(mgr->joycon))
            {
                mgr->joycon_state.store(ConnectionState::CONNECTED);
                LOG_SUCCESS("ジョイコン接続成功");
            }
            else
            {
                mgr->joycon_state.store(ConnectionState::FAILED);
                LOG_WARN("ジョイコン接続失敗、再試行します...");
                SDL_Delay(RETRY_DELAY_MS);
                continue;
            }
        }

        // 少し待機してCPU負荷を下げる
        SDL_Delay(100);
    }

    return 0;
}

void connection_manager_init(ConnectionManager *mgr, Joycon *joycon)
{
    mgr->joycon = joycon;
    mgr->joycon_state.store(ConnectionState::DISCONNECTED);
    mgr->joycon_thread = nullptr;
    mgr->joycon_thread_running.store(false);
    mgr->reconnect_requested.store(false);
}

void connection_manager_fini(ConnectionManager *mgr)
{
    connection_manager_stop_joycon_connect(mgr);
}

void connection_manager_start_joycon_connect(ConnectionManager *mgr)
{
    // 既にスレッドが動いている場合は何もしない
    if (mgr->joycon_thread_running.load())
    {
        return;
    }

    // 既に接続済みの場合は何もしない
    if (mgr->joycon_state.load() == ConnectionState::CONNECTED)
    {
        return;
    }

    mgr->joycon_state.store(ConnectionState::CONNECTING);
    mgr->joycon_thread_running.store(true);
    mgr->joycon_thread = SDL_CreateThread(joycon_connect_thread_func, "JoyconConnect", mgr);

    if (mgr->joycon_thread == nullptr)
    {
        LOG_ERROR("ジョイコン接続スレッド作成失敗: " << SDL_GetError());
        mgr->joycon_thread_running.store(false);
        mgr->joycon_state.store(ConnectionState::FAILED);
    }
}

void connection_manager_stop_joycon_connect(ConnectionManager *mgr)
{
    if (mgr->joycon_thread != nullptr)
    {
        mgr->joycon_thread_running.store(false);
        int status;
        SDL_WaitThread(mgr->joycon_thread, &status);
        mgr->joycon_thread = nullptr;
    }
}

ConnectionState connection_manager_get_joycon_state(const ConnectionManager *mgr)
{
    return mgr->joycon_state.load();
}

bool connection_manager_is_joycon_connected(const ConnectionManager *mgr)
{
    return mgr->joycon_state.load() == ConnectionState::CONNECTED;
}

void connection_manager_request_joycon_reconnect(ConnectionManager *mgr)
{
    // 既に接続中なら何もしない
    if (mgr->joycon_state.load() == ConnectionState::CONNECTING)
    {
        return;
    }

    mgr->reconnect_requested.store(true);

    // スレッドが動いていなければ開始
    if (!mgr->joycon_thread_running.load())
    {
        connection_manager_start_joycon_connect(mgr);
    }
}

bool connection_manager_check_joycon_disconnected(ConnectionManager *mgr)
{
    // 接続済み状態でジョイコンが切断されたかチェック
    if (mgr->joycon_state.load() == ConnectionState::CONNECTED && mgr->joycon != nullptr)
    {
        // joycon_check_connectedで実際の接続状態を確認
        if (!joycon_check_connected(mgr->joycon))
        {
            // 切断を検出
            mgr->joycon_state.store(ConnectionState::DISCONNECTED);
            LOG_WARN("ConnectionManager: ジョイコン切断を検出");
            return true;
        }
    }
    return false;
}
