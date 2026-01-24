#include "input_manager.hpp"

#include <cstring>

// 入力状態を保持するグローバル構造体
static struct
{
    bool keys[KEY_COUNT];
    bool prev_keys[KEY_COUNT];
} g_input;

void input_manager_init()
{
    memset(&g_input, 0, sizeof(g_input));
}

void input_manager_update()
{
    memcpy(g_input.prev_keys, g_input.keys, sizeof(g_input.keys));
}

void input_manager_handle_event(const SDL_Event &event)
{
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
    {
        bool pressed = (event.type == SDL_KEYDOWN);

        switch (event.key.keysym.sym)
        {
            case SDLK_UP:
                g_input.keys[KEY_UP] = pressed;
                break;
            case SDLK_DOWN:
                g_input.keys[KEY_DOWN] = pressed;
                break;
            case SDLK_LEFT:
                g_input.keys[KEY_LEFT] = pressed;
                break;
            case SDLK_RIGHT:
                g_input.keys[KEY_RIGHT] = pressed;
                break;
            case SDLK_RETURN:
                g_input.keys[KEY_ENTER] = pressed;
                break;
            case SDLK_ESCAPE:
                g_input.keys[KEY_ESCAPE] = pressed;
                break;
            default:
                break;
        }
    }
}

bool input_is_key_pressed(KeyCode key)
{
    if (key < 0 || key >= KEY_COUNT) return false;
    return g_input.keys[key];
}

bool input_is_key_just_pressed(KeyCode key)
{
    if (key < 0 || key >= KEY_COUNT) return false;
    return g_input.keys[key] && !g_input.prev_keys[key];
}

bool input_is_key_just_released(KeyCode key)
{
    if (key < 0 || key >= KEY_COUNT) return false;
    return !g_input.keys[key] && g_input.prev_keys[key];
}
