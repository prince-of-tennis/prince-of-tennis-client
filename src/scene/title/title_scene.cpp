#include "title_scene.hpp"

#include "glad/glad.h"
#include "input/input_manager.hpp"
#include "util/log.hpp"

constexpr float TENNIS_COURT_SCALE = 0.2f;
constexpr float STICK_THRESHOLD = 0.5f;

bool title_scene_init(TitleScene *scene)
{
    scene->font = EZ_2D_CreateFont("fonts/font.otf", 48);
    if (!scene->font) return false;

    scene->shader = EZ_CreateShader();
    if (!scene->shader) return false;

    float aspect = static_cast<float>(scene->context->window_width) /
                   static_cast<float>(scene->context->window_height);
    scene->camera = EZ_CreateCamera(aspect);
    EZ_CameraSetPosition(scene->camera, 0.0f, 15.0f, 25.0f);
    EZ_CameraSetTargetPosition(scene->camera, 0.0f, 0.0f, 0.0f);
    EZ_CameraSetClipPlanes(scene->camera, 0.1f, 200.0f);

    scene->light = EZ_CreateLight();
    EZ_LightSetPosition(scene->light, 10.0f, 20.0f, 10.0f);

    scene->court_object = EZ_CreateObject("obj/tennis_court.obj", "img/container.jpeg");
    if (!scene->court_object) return false;
    EZ_ObjectSetPosition(scene->court_object, 0.0f, 0.0f, 0.0f);
    EZ_ObjectSetScale(scene->court_object, TENNIS_COURT_SCALE, TENNIS_COURT_SCALE, TENNIS_COURT_SCALE);

    if (!audio_init(&scene->audio)) return false;

    scene->se_cursor_move = audio_load_se(&scene->audio, "audio/se/move_cursor.mp3");
    scene->se_decide = audio_load_se(&scene->audio, "audio/se/select.mp3");

    scene->blink_counter = 0;
    scene->show_start_message = true;
    scene->court_rotation = 0.0f;
    scene->frame_counter = 0;
    scene->title_alpha = 0.0f;
    scene->fade_in_complete = false;
    scene->selected_menu = MENU_START;
    scene->prev_stick_up = false;
    scene->prev_stick_down = false;

    return true;
}

static void update_joycon_connection(TitleScene *scene)
{
    if (!scene->connection_manager || scene->joycon) return;

    ConnectionState state = connection_manager_get_joycon_state(scene->connection_manager);

    if (state == ConnectionState::CONNECTED)
    {
        scene->joycon = scene->connection_manager->joycon;
        if (scene->joycon_initialized_ptr)
            *scene->joycon_initialized_ptr = true;
        LOG_SUCCESS("ジョイコン接続完了");
    }
    else if (state == ConnectionState::DISCONNECTED || state == ConnectionState::FAILED)
    {
        connection_manager_request_joycon_reconnect(scene->connection_manager);
    }
}

static void update_animations(TitleScene *scene)
{
    scene->frame_counter++;

    if (!scene->fade_in_complete)
    {
        scene->title_alpha = static_cast<float>(scene->frame_counter) / static_cast<float>(FADE_IN_FRAMES);
        if (scene->title_alpha >= 1.0f)
        {
            scene->title_alpha = 1.0f;
            scene->fade_in_complete = true;
        }
    }

    scene->court_rotation += COURT_ROTATION_SPEED;
    if (scene->court_rotation >= 360.0f)
        scene->court_rotation -= 360.0f;
    EZ_ObjectSetRotation(scene->court_object, 0.0f, scene->court_rotation, 0.0f);

    scene->blink_counter++;
    if (scene->blink_counter >= BLINK_INTERVAL)
    {
        scene->blink_counter = 0;
        scene->show_start_message = !scene->show_start_message;
    }
}

static void get_menu_input(TitleScene *scene, bool *up, bool *down, bool *select)
{
    *up = *down = *select = false;

    if (scene->joycon)
    {
        bool stick_up = scene->joycon->joycon.stick.y > STICK_THRESHOLD;
        bool stick_down = scene->joycon->joycon.stick.y < -STICK_THRESHOLD;

        if (stick_up && !scene->prev_stick_up) *up = true;
        if (stick_down && !scene->prev_stick_down) *down = true;

        scene->prev_stick_up = stick_up;
        scene->prev_stick_down = stick_down;

        if (joycon_is_just_pressed(scene->joycon, JOYCON_BTN_A))
            *select = true;
    }

    if (input_is_key_just_pressed(KEY_UP)) *up = true;
    if (input_is_key_just_pressed(KEY_DOWN)) *down = true;
    if (input_is_key_just_pressed(KEY_ENTER)) *select = true;
}

TitleSceneResult title_scene_update(TitleScene *scene)
{
    update_joycon_connection(scene);
    update_animations(scene);

    if (!scene->fade_in_complete)
        return TITLE_RESULT_NONE;

    bool up, down, select;
    get_menu_input(scene, &up, &down, &select);

    if (up)
    {
        scene->selected_menu = (scene->selected_menu - 1 + MENU_ITEM_COUNT) % MENU_ITEM_COUNT;
        if (scene->se_cursor_move >= 0)
            audio_play_se(&scene->audio, scene->se_cursor_move);
    }

    if (down)
    {
        scene->selected_menu = (scene->selected_menu + 1) % MENU_ITEM_COUNT;
        if (scene->se_cursor_move >= 0)
            audio_play_se(&scene->audio, scene->se_cursor_move);
    }

    if (select)
    {
        if (scene->se_decide >= 0)
            audio_play_se(&scene->audio, scene->se_decide);

        if (scene->selected_menu == MENU_START)
            return TITLE_RESULT_START;
        if (scene->selected_menu == MENU_EXIT)
            return TITLE_RESULT_EXIT;
    }

    return TITLE_RESULT_NONE;
}

void title_scene_draw(TitleScene *scene)
{
    glClearColor(0.85f, 0.78f, 0.65f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    EZ_DrawObject(scene->court_object, scene->shader, scene->camera, scene->light);

    float w = static_cast<float>(scene->context->window_width);
    float h = static_cast<float>(scene->context->window_height);

    EZ_2D_DrawText(scene->font, w / 2.0f - 220.0f, h * 0.25f, "超次元テニス",
                   TITLE_FONT_SIZE, 0.0f, 0.0f, 0.0f, scene->title_alpha);

    if (!scene->fade_in_complete) return;

    const char *items[] = {"はじめる", "おわる"};
    float menu_x = w / 2.0f - 80.0f;

    for (int i = 0; i < MENU_ITEM_COUNT; i++)
    {
        float y = MENU_START_Y + i * MENU_ITEM_SPACING;
        bool selected = (i == scene->selected_menu);
        float r = selected ? 0.4f : 0.5f;
        float g = selected ? 0.25f : 0.45f;
        float b = selected ? 0.1f : 0.4f;

        if (selected)
            EZ_2D_DrawText(scene->font, menu_x - 50.0f, y, "▶", MENU_FONT_SIZE, r, g, b, 1.0f);

        EZ_2D_DrawText(scene->font, menu_x, y, items[i], MENU_FONT_SIZE, r, g, b, 1.0f);
    }
}

void title_scene_fini(TitleScene *scene)
{
    audio_fini(&scene->audio);
}
