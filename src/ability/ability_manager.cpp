#include "ability_manager.hpp"

#include <cstring>

#include "joycon/joycon.hpp"
#include "util/log.hpp"

// JoyconButtonからAbilityButtonへの変換テーブル
static int ability_button_to_joycon(AbilityButton btn)
{
    switch (btn)
    {
        case ABILITY_BTN_A:
            return JOYCON_BTN_A;
        case ABILITY_BTN_B:
            return JOYCON_BTN_B;
        case ABILITY_BTN_X:
            return JOYCON_BTN_X;
        case ABILITY_BTN_Y:
            return JOYCON_BTN_Y;
        case ABILITY_BTN_ZR:
            return JOYCON_BTN_ZR;
        case ABILITY_BTN_ZL:
            return JOYCON_BTN_ZL;
        default:
            return -1;
    }
}

void ability_manager_init(AbilityManager* mgr)
{
    std::memset(mgr, 0, sizeof(AbilityManager));
    mgr->has_pending_request = false;
}

void ability_manager_update(AbilityManager* mgr, const Joycon* joycon)
{
    if (joycon == nullptr)
    {
        return;
    }

    // 各能力のボタン状態を更新
    for (size_t i = 0; i < ABILITY_CONFIG_COUNT; ++i)
    {
        const AbilityConfig& config = ABILITY_CONFIGS[i];
        int joycon_btn = ability_button_to_joycon(config.button);
        if (joycon_btn >= 0)
        {
            mgr->button_held[config.type] = joycon_is_pressed(joycon, static_cast<JoyconButton>(joycon_btn));
        }
    }
}

void ability_manager_tick(AbilityManager* mgr)
{
    // ローカル能力の残りフレームを減らす
    for (int i = 0; i < ABILITY_MAX; ++i)
    {
        if (mgr->local_states[i].remaining_frames > 0)
        {
            mgr->local_states[i].remaining_frames--;
        }
    }
}

// 共通の発動チェックロジック
static AbilityActivateRequest* ability_try_activate(AbilityManager* mgr, int player_id,
                                                    AbilityTrigger trigger)
{
    for (size_t i = 0; i < ABILITY_CONFIG_COUNT; ++i)
    {
        const AbilityConfig& config = ABILITY_CONFIGS[i];

        // トリガーが一致しない場合はスキップ
        if (config.trigger != trigger)
        {
            continue;
        }

        // ボタンが押されていない場合はスキップ
        if (!mgr->button_held[config.type])
        {
            continue;
        }

        // サーバー不要の能力はローカルで即発動
        if (!config.requires_server)
        {
            ability_activate_local(mgr, config.type);
        }

        // リクエストを生成
        mgr->pending_request.player_id = player_id;
        mgr->pending_request.ability_type = config.type;
        mgr->pending_request.trigger = trigger;
        mgr->has_pending_request = true;

        LOG_DEBUG("能力発動: type=" << static_cast<int>(config.type)
                                    << " trigger=" << static_cast<int>(trigger)
                                    << " player=" << player_id);

        return &mgr->pending_request;
    }

    return nullptr;
}

AbilityActivateRequest* ability_check_instant(AbilityManager* mgr, int player_id)
{
    return ability_try_activate(mgr, player_id, TRIGGER_INSTANT);
}

AbilityActivateRequest* ability_check_on_swing(AbilityManager* mgr, int player_id)
{
    // スイング時のボタン状態をバッファリング（打撃検出用）
    // 打撃検出はサーバーからの遅延があるため、スイング時の状態を保持
    for (int i = 0; i < ABILITY_MAX; ++i)
    {
        mgr->swing_button_held[i] = mgr->button_held[i];
    }

    return ability_try_activate(mgr, player_id, TRIGGER_ON_SWING);
}

AbilityActivateRequest* ability_check_on_hit(AbilityManager* mgr, int player_id)
{
    // 打撃時発動能力のチェック（スイング時にバッファしたボタン状態を使用）
    for (size_t i = 0; i < ABILITY_CONFIG_COUNT; ++i)
    {
        const AbilityConfig& config = ABILITY_CONFIGS[i];

        // トリガーが一致しない場合はスキップ
        if (config.trigger != TRIGGER_ON_HIT)
        {
            continue;
        }

        // スイング時にボタンが押されていなかった場合はスキップ
        if (!mgr->swing_button_held[config.type])
        {
            continue;
        }

        // バッファをクリア（連続発動防止）
        mgr->swing_button_held[config.type] = false;

        // サーバー不要の能力はローカルで即発動
        if (!config.requires_server)
        {
            ability_activate_local(mgr, config.type);
        }

        // リクエストを生成
        mgr->pending_request.player_id = player_id;
        mgr->pending_request.ability_type = config.type;
        mgr->pending_request.trigger = TRIGGER_ON_HIT;
        mgr->has_pending_request = true;

        LOG_DEBUG("能力発動(on_hit): type=" << static_cast<int>(config.type)
                                            << " player=" << player_id);

        return &mgr->pending_request;
    }

    return nullptr;
}

bool ability_is_active(const AbilityManager* mgr, int player_id, AbilityType type)
{
    // サーバー管理の能力の場合
    if (player_id >= 0 && player_id < ABILITY_PLAYER_MAX)
    {
        if (mgr->server_states[player_id].active_ability == type &&
            mgr->server_states[player_id].remaining_frames > 0)
        {
            return true;
        }
    }

    // ローカル管理の能力の場合
    return ability_is_local_active(mgr, type);
}

bool ability_is_local_active(const AbilityManager* mgr, AbilityType type)
{
    if (type <= ABILITY_NONE || type >= ABILITY_MAX)
    {
        return false;
    }
    return mgr->local_states[type].remaining_frames > 0;
}

void ability_set_server_state(AbilityManager* mgr, const AbilityState* state)
{
    if (state == nullptr)
    {
        return;
    }

    int player_id = state->player_id;
    if (player_id < 0 || player_id >= ABILITY_PLAYER_MAX)
    {
        LOG_ERROR("不正なplayer_id: " << player_id);
        return;
    }

    mgr->server_states[player_id] = *state;
}

void ability_activate_local(AbilityManager* mgr, AbilityType type)
{
    const AbilityConfig* config = ability_get_config(type);
    if (config == nullptr)
    {
        return;
    }

    mgr->local_states[type].active_ability = type;
    mgr->local_states[type].remaining_frames = config->duration_frames;

    LOG_DEBUG("ローカル能力発動: type=" << static_cast<int>(type)
                                        << " duration=" << config->duration_frames);
}
