#pragma once
#include "common/ability.h"
#include "common/ability_config.h"

// 前方宣言
struct Joycon;

// プレイヤー数
constexpr int ABILITY_PLAYER_MAX = 2;

// 能力マネージャー
struct AbilityManager
{
    // ボタン押下状態（能力ごと）
    bool button_held[ABILITY_MAX];

    // スイング時のボタン押下状態（打撃検出用にバッファリング）
    // 打撃検出はサーバーからの遅延があるため、スイング時の状態を保持
    bool swing_button_held[ABILITY_MAX];

    // ローカル能力状態（クライアント側で管理する能力用）
    AbilityState local_states[ABILITY_MAX];

    // サーバーから受信した能力状態（プレイヤーごと）
    AbilityState server_states[ABILITY_PLAYER_MAX];

    // 発動リクエストバッファ
    AbilityActivateRequest pending_request;
    bool has_pending_request;
};

// 初期化
void ability_manager_init(AbilityManager* mgr);

// ボタン状態更新（毎フレーム呼ぶ）
void ability_manager_update(AbilityManager* mgr, const Joycon* joycon);

// フレーム進行（クールダウン・持続時間の減算）
void ability_manager_tick(AbilityManager* mgr);

// 即時発動（ボタンのみ）チェック
// 戻り値: 発動リクエストがある場合はそのポインタ、なければnullptr
AbilityActivateRequest* ability_check_instant(AbilityManager* mgr, int player_id);

// スイング時発動チェック（外部からスイング検出時に呼ぶ）
AbilityActivateRequest* ability_check_on_swing(AbilityManager* mgr, int player_id);

// 打撃時発動チェック（外部から打撃検出時に呼ぶ）
AbilityActivateRequest* ability_check_on_hit(AbilityManager* mgr, int player_id);

// 能力がアクティブかチェック
bool ability_is_active(const AbilityManager* mgr, int player_id, AbilityType type);

// ローカル能力がアクティブかチェック（クライアント完結の能力用）
bool ability_is_local_active(const AbilityManager* mgr, AbilityType type);

// サーバーからの能力状態を設定
void ability_set_server_state(AbilityManager* mgr, const AbilityState* state);

// ローカル能力を発動（クライアント完結の能力用）
void ability_activate_local(AbilityManager* mgr, AbilityType type);
