#pragma once

#include <SDL2/SDL.h>

// キーコード
enum KeyCode
{
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_ENTER,
    KEY_ESCAPE,
    KEY_COUNT
};

/// @brief 入力マネージャーの初期化
void input_manager_init();

/// @brief 毎フレーム開始時に呼ぶ（前フレームの状態を保存）
void input_manager_update();

/// @brief SDL イベントを処理
void input_manager_handle_event(const SDL_Event &event);

/// @brief キーが押されているか
bool input_is_key_pressed(KeyCode key);

/// @brief キーが今押された瞬間か
bool input_is_key_just_pressed(KeyCode key);

/// @brief キーが今離された瞬間か
bool input_is_key_just_released(KeyCode key);
