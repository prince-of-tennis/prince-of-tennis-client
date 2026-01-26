#pragma once

#include <SDL2/SDL.h>
#include <atomic>

#include "joycon/joycon.hpp"

// 接続状態
enum class ConnectionState
{
    DISCONNECTED,  // 未接続
    CONNECTING,    // 接続中
    CONNECTED,     // 接続済み
    FAILED         // 接続失敗
};

// ジョイコン接続を別スレッドで管理するクラス
struct ConnectionManager
{
    // ジョイコン関連
    Joycon *joycon;                              // ジョイコンへのポインタ（外部所有）
    std::atomic<ConnectionState> joycon_state;   // ジョイコン接続状態
    SDL_Thread *joycon_thread;                   // ジョイコン接続スレッド
    std::atomic<bool> joycon_thread_running;     // スレッド実行フラグ

    // 再接続用
    std::atomic<bool> reconnect_requested;       // 再接続リクエスト
};

// ConnectionManager初期化
void connection_manager_init(ConnectionManager *mgr, Joycon *joycon);

// ConnectionManager終了処理
void connection_manager_fini(ConnectionManager *mgr);

// ジョイコン接続を別スレッドで開始
void connection_manager_start_joycon_connect(ConnectionManager *mgr);

// ジョイコン接続スレッドを停止
void connection_manager_stop_joycon_connect(ConnectionManager *mgr);

// ジョイコンの接続状態を取得
ConnectionState connection_manager_get_joycon_state(const ConnectionManager *mgr);

// ジョイコンが接続済みかどうか
bool connection_manager_is_joycon_connected(const ConnectionManager *mgr);

// ジョイコンの再接続をリクエスト
void connection_manager_request_joycon_reconnect(ConnectionManager *mgr);

// ジョイコン切断を検出（呼び出し側で適宜チェック）
bool connection_manager_check_joycon_disconnected(ConnectionManager *mgr);
