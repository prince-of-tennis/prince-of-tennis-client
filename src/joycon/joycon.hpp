#pragma once
#include <joyconlib.h>

#include <SDL2/SDL.h>

#include "common/player_input.h"
#include "common/player_swing.h"

struct Joycon
{
    joyconlib_t joycon;
    PlayerInput cached_input;   // 前回の入力をキャッシュ
    PlayerSwing cached_swing;   // 前回のスイングをキャッシュ
    Uint32 last_swing_time_ms;  // 最後にswingを送信した時刻（ミリ秒）
};

bool joycon_init(Joycon *joycon);

PlayerInput get_joycon(Joycon *joycon, int player_id);

// Joy-Conの入力が大きく変化したかチェック
bool joycon_has_significant_change(const PlayerInput *current, const PlayerInput *cached);

// Joy-Conからスイングデータを取得
PlayerSwing get_joycon_swing(Joycon *joycon, int player_id);

// スイング動作が閾値を超えたかチェック
bool joycon_has_significant_swing(Joycon *joycon, const PlayerSwing *current);

void joycon_fini(Joycon *joycon);