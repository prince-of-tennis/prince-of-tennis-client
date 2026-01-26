#include "matching_scene.hpp"

#include "glad/glad.h"
#include "input/input_manager.hpp"
#include "util/log.hpp"

constexpr float TENNIS_COURT_SCALE = 0.2f;
constexpr float COURT_ROTATION_SPEED = 0.2f;
constexpr float STATUS_FONT_SIZE = 36.0f;

static void update_joycon_phase(MatchingScene *scene)
{
    if (scene->connection_manager)
    {
        ConnectionState state = connection_manager_get_joycon_state(scene->connection_manager);

        if (state == ConnectionState::CONNECTED)
        {
            if (scene->joycon_initialized_ptr)
                *scene->joycon_initialized_ptr = true;
            scene->phase = MATCHING_PHASE_SERVER;
            scene->last_attempt_failed = false;
            scene->retry_count = 0;
            return;
        }

        if (state == ConnectionState::CONNECTING)
        {
            scene->status_message = "Joy-Conに接続中";
            return;
        }

        if (state == ConnectionState::FAILED)
        {
            scene->status_message = "Joy-Conに接続中";
            connection_manager_request_joycon_reconnect(scene->connection_manager);
            return;
        }

        connection_manager_start_joycon_connect(scene->connection_manager);
        scene->status_message = "Joy-Conに接続中";
        return;
    }

    if (scene->last_attempt_failed)
    {
        scene->retry_wait_frames--;
        if (scene->retry_wait_frames <= 0)
        {
            scene->last_attempt_failed = false;
            scene->retry_count++;
        }
        return;
    }

    scene->status_message = "Joy-Conに接続中";

    if (joycon_init(scene->joycon))
    {
        if (scene->joycon_initialized_ptr)
            *scene->joycon_initialized_ptr = true;
        scene->phase = MATCHING_PHASE_SERVER;
        scene->last_attempt_failed = false;
        scene->retry_count = 0;
    }
    else
    {
        scene->last_attempt_failed = true;
        scene->retry_wait_frames = RETRY_WAIT_FRAMES;
    }
}

static void update_server_phase(MatchingScene *scene)
{
    if (scene->last_attempt_failed)
    {
        scene->retry_wait_frames--;
        if (scene->retry_wait_frames <= 0)
        {
            scene->last_attempt_failed = false;
            scene->retry_count++;
        }
        return;
    }

    scene->status_message = "サーバーに接続中";

    if (network_init(scene->network, scene->context))
    {
        if (scene->network_initialized_ptr)
            *scene->network_initialized_ptr = true;
        scene->phase = MATCHING_PHASE_WAITING;
        scene->status_message = "対戦相手を待っています";
        scene->last_attempt_failed = false;
        scene->retry_count = 0;
    }
    else
    {
        scene->last_attempt_failed = true;
        scene->retry_wait_frames = RETRY_WAIT_FRAMES;
    }
}

static void update_waiting_phase(MatchingScene *scene)
{
    scene->status_message = "対戦相手を待っています";

    if (!network_listen_to_server(scene->network))
        return;

    if (scene->context->player_id != -1)
    {
        scene->phase = MATCHING_PHASE_COMPLETE;
        scene->complete_counter = COMPLETE_DISPLAY_FRAMES;

        if (scene->se_decide >= 0)
            audio_play_se(&scene->audio, scene->se_decide);
    }
}

static MatchingResult update_complete_phase(MatchingScene *scene)
{
    scene->status_message = "マッチング完了！";
    scene->complete_counter--;
    return (scene->complete_counter <= 0) ? MATCHING_RESULT_SUCCESS : MATCHING_RESULT_NONE;
}

bool matching_scene_init(MatchingScene *scene)
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

    if (!audio_init(&scene->audio)) return false;

    scene->se_decide = audio_load_se(&scene->audio, "audio/se/select.mp3");

    scene->retry_count = 0;
    scene->retry_wait_frames = 0;
    scene->last_attempt_failed = false;
    scene->status_message = "";
    scene->dot_animation_counter = 0;
    scene->complete_counter = 0;

    scene->phase = (scene->joycon_initialized_ptr && *scene->joycon_initialized_ptr)
                       ? MATCHING_PHASE_SERVER
                       : MATCHING_PHASE_JOYCON;

    return true;
}

MatchingResult matching_scene_update(MatchingScene *scene)
{
    scene->dot_animation_counter++;
    if (scene->dot_animation_counter >= DOT_ANIMATION_INTERVAL * 4)
        scene->dot_animation_counter = 0;

    if (input_is_key_just_pressed(KEY_ESCAPE))
        return MATCHING_RESULT_CANCEL;

    switch (scene->phase)
    {
        case MATCHING_PHASE_JOYCON:
            update_joycon_phase(scene);
            break;
        case MATCHING_PHASE_SERVER:
            update_server_phase(scene);
            break;
        case MATCHING_PHASE_WAITING:
            update_waiting_phase(scene);
            break;
        case MATCHING_PHASE_COMPLETE:
            return update_complete_phase(scene);
    }

    return MATCHING_RESULT_NONE;
}

void matching_scene_draw(MatchingScene *scene)
{
    glClearColor(0.85f, 0.78f, 0.65f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float w = static_cast<float>(scene->context->window_width);
    float h = static_cast<float>(scene->context->window_height);

    if (scene->status_message && scene->status_message[0] != '\0')
    {
        int dots = scene->dot_animation_counter / DOT_ANIMATION_INTERVAL;
        char msg[256];
        snprintf(msg, sizeof(msg), "%s%.*s", scene->status_message, dots, "...");

        float x = w / 2.0f - 200.0f;
        float y = h / 2.0f;
        EZ_2D_DrawText(scene->font, x, y, msg, STATUS_FONT_SIZE, 0.3f, 0.2f, 0.1f, 1.0f);
    }
}

void matching_scene_fini(MatchingScene *scene)
{
    scene->font.reset();
    audio_fini(&scene->audio);
}
