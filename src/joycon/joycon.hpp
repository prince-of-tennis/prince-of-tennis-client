#pragma once
#include <joyconlib.h>

#include <SDL2/SDL.h>

#include "common/player_input.h"
#include "common/player_swing.h"

// Joyconボタン定数
enum JoyconButton
{
    JOYCON_BTN_A = 0,
    JOYCON_BTN_B,
    JOYCON_BTN_X,
    JOYCON_BTN_Y,
    JOYCON_BTN_R,
    JOYCON_BTN_ZR,
    JOYCON_BTN_L,
    JOYCON_BTN_ZL,
    JOYCON_BTN_PLUS,
    JOYCON_BTN_MINUS,
    JOYCON_BTN_RSTICK,
    JOYCON_BTN_LSTICK,
    JOYCON_BTN_UP,
    JOYCON_BTN_DOWN,
    JOYCON_BTN_LEFT,
    JOYCON_BTN_RIGHT,
    JOYCON_BTN_HOME,
    JOYCON_BTN_CAPTURE,
    JOYCON_BTN_SR,
    JOYCON_BTN_SL,
    JOYCON_BTN_COUNT  // ボタン数
};

struct Joycon
{
    joyconlib_t joycon;
    PlayerInput cached_input;   // 前回の入力をキャッシュ
    PlayerSwing cached_swing;   // 前回のスイングをキャッシュ
    Uint32 last_swing_time_ms;  // 最後にswingを送信した時刻（ミリ秒）

    // ボタン状態
    bool buttons[JOYCON_BTN_COUNT];       // 現在のボタン状態
    bool prev_buttons[JOYCON_BTN_COUNT];  // 前フレームのボタン状態
};

bool joycon_init(Joycon *joycon);

PlayerInput get_joycon(Joycon *joycon, int player_id);

// Joy-Conの入力が大きく変化したかチェック
bool joycon_has_significant_change(const PlayerInput *current, const PlayerInput *cached);

// Joy-Conからスイングデータを取得
PlayerSwing get_joycon_swing(Joycon *joycon, int player_id);

// スイング動作が閾値を超えたかチェック
bool joycon_has_significant_swing(Joycon *joycon, const PlayerSwing *current);

// ボタン状態を更新（毎フレーム呼ぶ）
void joycon_update_buttons(Joycon *joycon);

// ボタンが押されているか
bool joycon_is_pressed(const Joycon *joycon, JoyconButton button);

// ボタンが今押された瞬間か（エッジ検出）
bool joycon_is_just_pressed(const Joycon *joycon, JoyconButton button);

// ボタンが今離された瞬間か（エッジ検出）
bool joycon_is_just_released(const Joycon *joycon, JoyconButton button);

// ジョイコンが接続されているかチェック（状態取得でエラーが発生したらfalse）
bool joycon_check_connected(Joycon *joycon);

void joycon_fini(Joycon *joycon);