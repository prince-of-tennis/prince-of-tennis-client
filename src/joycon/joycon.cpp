#include "joycon.hpp"

// #include <joyconlib.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>

#include <cmath>

#include "common/player_input.h"
#include "common/player_swing.h"
#include "util/log.hpp"

// スイング判定の閾値定数
static const float SWING_ACC_THRESHOLD = 18.0f;  // 加速度の閾値（スイング検出用）
static const Uint32 SWING_COOLDOWN_MS = 500;     // スイングのクールダウン時間（ミリ秒）

bool joycon_init(Joycon *joycon)
{
    joycon_err err = joycon_open(&joycon->joycon, JOYCON_R);
    if (err != JOYCON_ERR_NONE)
    {
        LOG_ERROR("Joy-Conの初期化に失敗しました: " << err);
        return false;
    }

    // キャッシュを初期化
    memset(&joycon->cached_input, 0, sizeof(PlayerInput));
    joycon->cached_input.player_id = -1;  // 初回判定用

    memset(&joycon->cached_swing, 0, sizeof(PlayerSwing));
    joycon->cached_swing.player_id = -1;  // 初回判定用

    // スイング時刻を初期化
    joycon->last_swing_time_ms = 0;

    // ボタン状態を初期化
    memset(joycon->buttons, 0, sizeof(joycon->buttons));
    memset(joycon->prev_buttons, 0, sizeof(joycon->prev_buttons));

    LOG_SUCCESS("Joy-Conの初期化に成功しました");

    return true;
}

PlayerInput get_joycon(Joycon *joycon, int player_id)
{
    static const float TRIGGER_THRESHOLD = 0.5f;

    PlayerInput player_input;
    memset(&player_input, 0, sizeof(PlayerInput));

    player_input.player_id = player_id;

    joycon_get_state(&joycon->joycon);

    if (joycon->joycon.stick.x < -TRIGGER_THRESHOLD)
    {
        player_input.left = true;
    }

    if (joycon->joycon.stick.x > TRIGGER_THRESHOLD)
    {
        player_input.right = true;
    }

    if (joycon->joycon.stick.y < -TRIGGER_THRESHOLD)
    {
        player_input.front = true;
    }

    if (joycon->joycon.stick.y > TRIGGER_THRESHOLD)
    {
        player_input.back = true;
    }

    return player_input;
}

// Joy-Conの入力が大きく変化したかチェック
bool joycon_has_significant_change(const PlayerInput *current, const PlayerInput *cached)
{
    // 初回は必ず送信
    if (cached->player_id == -1)
    {
        return true;
    }

    // ボタン状態が変化した場合
    if (current->left != cached->left || current->right != cached->right ||
        current->front != cached->front || current->back != cached->back)
    {
        return true;
    }

    return false;
}

// Joy-Conからスイングデータを取得
PlayerSwing get_joycon_swing(Joycon *joycon, int player_id)
{
    PlayerSwing player_swing;
    memset(&player_swing, 0, sizeof(PlayerSwing));

    player_swing.player_id = player_id;

    joycon_get_state(&joycon->joycon);

    // 加速度データを取得
    player_swing.acc_x = joycon->joycon.axis[0].acc_x;
    player_swing.acc_y = joycon->joycon.axis[0].acc_y;
    player_swing.acc_z = joycon->joycon.axis[0].acc_z;

    return player_swing;
}

// スイング動作が閾値を超えたかチェック
bool joycon_has_significant_swing(Joycon *joycon, const PlayerSwing *current)
{
    // 加速度の合成値（ベクトルの大きさ）を計算
    float acc_magnitude =
        std::sqrt(current->acc_x * current->acc_x + current->acc_y * current->acc_y +
                  current->acc_z * current->acc_z);

    // デバッグ：加速度値を定期的に出力（60フレームに1回）
    static int debug_counter = 0;
    if (debug_counter % 60 == 0)
    {
        LOG_DEBUG("加速度: x=" << current->acc_x << " y=" << current->acc_y
                               << " z=" << current->acc_z << " magnitude=" << acc_magnitude
                               << " 閾値=" << SWING_ACC_THRESHOLD);
    }
    debug_counter++;

    // 初回は初期化のみ
    if (joycon->cached_swing.player_id == -1)
    {
        joycon->cached_swing = *current;  // キャッシュを初期化
        return false;
    }

    // クールダウン中かチェック
    Uint32 current_time = SDL_GetTicks();
    Uint32 time_since_last_swing = current_time - joycon->last_swing_time_ms;
    if (time_since_last_swing < SWING_COOLDOWN_MS)
    {
        return false;  // クールダウン中は送信しない
    }

    // 閾値を超えたらスイングと判定
    if (acc_magnitude > SWING_ACC_THRESHOLD)
    {
        LOG_SUCCESS("スイング検出! magnitude=" << acc_magnitude);
        joycon->last_swing_time_ms = current_time;  // 最後のswing時刻を更新
        return true;
    }

    return false;
}

void joycon_update_buttons(Joycon *joycon)
{
    // 前フレームの状態を保存
    memcpy(joycon->prev_buttons, joycon->buttons, sizeof(joycon->buttons));

    // 現在の状態を取得（joycon_get_stateは既に呼ばれている前提）
    const joycon_btn &btn = joycon->joycon.button;

    // joyconlib_tのbtnフィールドから配列へマッピング
    joycon->buttons[JOYCON_BTN_A] = btn.btn.A;
    joycon->buttons[JOYCON_BTN_B] = btn.btn.B;
    joycon->buttons[JOYCON_BTN_X] = btn.btn.X;
    joycon->buttons[JOYCON_BTN_Y] = btn.btn.Y;
    joycon->buttons[JOYCON_BTN_R] = btn.btn.R;
    joycon->buttons[JOYCON_BTN_ZR] = btn.btn.ZR;
    joycon->buttons[JOYCON_BTN_L] = btn.btn.L;
    joycon->buttons[JOYCON_BTN_ZL] = btn.btn.ZL;
    joycon->buttons[JOYCON_BTN_PLUS] = btn.btn.Plus;
    joycon->buttons[JOYCON_BTN_MINUS] = btn.btn.Minus;
    joycon->buttons[JOYCON_BTN_RSTICK] = btn.btn.RStick;
    joycon->buttons[JOYCON_BTN_LSTICK] = btn.btn.LStick;
    joycon->buttons[JOYCON_BTN_UP] = btn.btn.Up;
    joycon->buttons[JOYCON_BTN_DOWN] = btn.btn.Down;
    joycon->buttons[JOYCON_BTN_LEFT] = btn.btn.Left;
    joycon->buttons[JOYCON_BTN_RIGHT] = btn.btn.Right;
    joycon->buttons[JOYCON_BTN_HOME] = btn.btn.Home;
    joycon->buttons[JOYCON_BTN_CAPTURE] = btn.btn.Capture;
    // SR/SLは左右どちらかがあれば有効
    joycon->buttons[JOYCON_BTN_SR] = btn.btn.SR_r || btn.btn.SR_l;
    joycon->buttons[JOYCON_BTN_SL] = btn.btn.SL_r || btn.btn.SL_l;
}

bool joycon_is_pressed(const Joycon *joycon, JoyconButton button)
{
    if (button < 0 || button >= JOYCON_BTN_COUNT)
    {
        return false;
    }
    return joycon->buttons[button];
}

bool joycon_is_just_pressed(const Joycon *joycon, JoyconButton button)
{
    if (button < 0 || button >= JOYCON_BTN_COUNT)
    {
        return false;
    }
    return joycon->buttons[button] && !joycon->prev_buttons[button];
}

bool joycon_is_just_released(const Joycon *joycon, JoyconButton button)
{
    if (button < 0 || button >= JOYCON_BTN_COUNT)
    {
        return false;
    }
    return !joycon->buttons[button] && joycon->prev_buttons[button];
}

void joycon_fini(Joycon *joycon)
{
    joycon_close(&joycon->joycon);
}