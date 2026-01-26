#include "game_scene.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "connection/connection_manager.hpp"
#include "game_scene_camera.hpp"
#include "game_scene_network.hpp"
#include "glad/glad.h"
#include "joycon/joycon.hpp"
#include "network/network.hpp"
#include "opengl/2d/EZ_2d.h"
#include "opengl/EasyGL.hpp"
#include "util/log.hpp"

constexpr int NETWORK_ERROR_DISPLAY_FRAMES = 180;

static int get_player_point(const GameScore &score, int player_id)
{
    return (player_id == 0) ? score.point_p1 : score.point_p2;
}

static int get_player_sets(const GameScore &score, int player_id)
{
    return (player_id == 0) ? score.sets_p1 : score.sets_p2;
}

static bool init_objects(GameScene *scene)
{
    scene->shader = EZ_CreateShader();
    if (!scene->shader) return false;

    scene->ball_object = EZ_CreateObject("obj/ball.obj", "img/container.jpeg");
    if (!scene->ball_object) return false;

    scene->ball_data.point = {0.0f, 2.0f, 0.0f};
    EZ_ObjectSetPosition(scene->ball_object, 0.0f, 2.0f, 0.0f);
    EZ_ObjectSetScale(scene->ball_object, 0.5, 0.5, 0.5);

    for (int i = 0; i < PLAYER_MAX; i++)
    {
        scene->player_objects[i] = EZ_CreateObject("obj/player.obj", "img/container.jpeg");
        if (!scene->player_objects[i]) return false;

        scene->player_data[i].point.x = PLAYER_INITIAL_X[i];
        scene->player_data[i].point.y = PLAYER_INITIAL_Y;
        scene->player_data[i].point.z = PLAYER_INITIAL_Z;
        EZ_ObjectSetPosition(scene->player_objects[i], PLAYER_INITIAL_X[i], PLAYER_INITIAL_Y, PLAYER_INITIAL_Z);
        EZ_ObjectSetScale(scene->player_objects[i], 1.0, 1.0, 1.0);
    }

    scene->court_object = EZ_CreateObject("obj/tennis_court.obj", "img/container.jpeg");
    if (!scene->court_object) return false;
    EZ_ObjectSetPosition(scene->court_object, 0.0f, 0.0f, 0.0f);
    EZ_ObjectSetScale(scene->court_object, TENNIS_COURT_SCALE, TENNIS_COURT_SCALE, TENNIS_COURT_SCALE);

    scene->ground_object = EZ_CreateObject("obj/ground.obj", "img/ground.png");
    if (!scene->ground_object) return false;
    EZ_ObjectSetPosition(scene->ground_object, 0.0f, 0.0f, 0.0f);
    EZ_ObjectSetScale(scene->ground_object, 1.0f, 1.0f, 1.0f);

    return true;
}

bool game_scene_init(GameScene *scene, Network *network)
{
    scene->network.reset(network);

    if (!init_objects(scene))
    {
        LOG_ERROR("オブジェクト初期化失敗");
        return false;
    }

    scene->light = EZ_CreateLight();
    scene->camera = EZ_CreateCamera(static_cast<float>(scene->context->window_width) /
                                    static_cast<float>(scene->context->window_height));
    game_scene_init_camera_position(scene);
    EZ_CameraSetClipPlanes(scene->camera, 0.1f, 200.0f);

    scene->server_sync_counter = 0;

    scene->font = EZ_2D_CreateFont("fonts/font.otf", 48);
    if (!scene->font)
    {
        LOG_ERROR("フォント初期化失敗");
        return false;
    }

    memset(&scene->game_score, 0, sizeof(GameScore));
    memset(&scene->prev_game_score, 0, sizeof(GameScore));

    if (!audio_init(&scene->audio))
    {
        LOG_ERROR("オーディオ初期化失敗");
        return false;
    }

    scene->se_hit_ball = audio_load_se(&scene->audio, "audio/se/hit_ball.mp3");
    scene->se_yatta = audio_load_se(&scene->audio, "audio/se/yatta.mp3");
    scene->prev_hit_count = 0;

    ability_manager_init(&scene->ability_manager);

    scene->is_game_finished = false;
    scene->winner_id = -1;
    scene->current_phase = GAME_PHASE_START_GAME;
    scene->network->network_data_set.match_winner = -1;
    scene->network_error = false;
    scene->network_error_counter = 0;

    LOG_SUCCESS("GameScene初期化完了");
    return true;
}

void game_scene_fini(GameScene *scene)
{
    audio_fini(&scene->audio);
    scene->network.release();
    scene->context->player_id = -1;
}

static void update_joycon_connection(GameScene *scene)
{
    if (!scene->connection_manager) return;

    if (scene->joycon && connection_manager_check_joycon_disconnected(scene->connection_manager))
    {
        LOG_WARN("ジョイコン切断、再接続開始");
        scene->joycon = nullptr;
        connection_manager_request_joycon_reconnect(scene->connection_manager);
    }

    if (!scene->joycon && connection_manager_is_joycon_connected(scene->connection_manager))
    {
        scene->joycon = scene->connection_manager->joycon;
        LOG_SUCCESS("ジョイコン再接続完了");
    }
}

static void update_player_scale(GameScene *scene, int i)
{
    const AbilityState &state = scene->network->network_data_set.ability_states[i];
    bool is_giant = (state.active_ability == ABILITY_GIANT && state.remaining_frames > 0);
    float scale = is_giant ? 10.0f : 1.0f;
    EZ_ObjectSetScale(scene->player_objects[i], scale, scale, scale);
}

static void handle_ability_buttons(GameScene *scene, int my_id)
{
    static bool prev_giant = false;
    static bool prev_clone = false;

    bool curr_giant = scene->ability_manager.button_held[ABILITY_GIANT];
    bool curr_clone = scene->ability_manager.button_held[ABILITY_CLONE];

    if (curr_giant != prev_giant)
    {
        AbilityActivateRequest req = {my_id, ABILITY_GIANT, curr_giant ? TRIGGER_INSTANT : TRIGGER_ON_HIT};
        game_scene_send_ability_request(scene, &req);
        prev_giant = curr_giant;
    }

    if (curr_clone != prev_clone)
    {
        AbilityActivateRequest req = {my_id, ABILITY_CLONE, curr_clone ? TRIGGER_INSTANT : TRIGGER_ON_HIT};
        game_scene_send_ability_request(scene, &req);
        prev_clone = curr_clone;
    }
}

GameSceneResult game_scene_update(GameScene *scene, PlayerInput *player_input, PlayerSwing *player_swing)
{
    if (scene->network_error)
    {
        scene->network_error_counter--;
        const Uint8 *key = SDL_GetKeyboardState(nullptr);
        if (scene->network_error_counter <= 0 || key[SDL_SCANCODE_RETURN])
            return GAME_RESULT_NETWORK_ERROR;
        return GAME_RESULT_CONTINUE;
    }

    if (scene->is_game_finished)
    {
        const Uint8 *key = SDL_GetKeyboardState(nullptr);
        if (key[SDL_SCANCODE_RETURN])
            return GAME_RESULT_RETURN_TITLE;
        return GAME_RESULT_CONTINUE;
    }

    update_joycon_connection(scene);

    if (!network_listen_to_server(scene->network.get()))
    {
        LOG_ERROR("サーバー接続切断");
        scene->network_error = true;
        scene->network_error_counter = NETWORK_ERROR_DISPLAY_FRAMES;
        return GAME_RESULT_CONTINUE;
    }

    scene->ball_data = scene->network->network_data_set.ball;
    EZ_ObjectSetPosition(scene->ball_object, scene->ball_data.point.x, scene->ball_data.point.y, scene->ball_data.point.z);

    bool ball_hit = (scene->ball_data.hit_count > scene->prev_hit_count);
    if (ball_hit)
        audio_play_se(&scene->audio, scene->se_hit_ball);

    scene->game_score = scene->network->network_data_set.game_score;

    int my_id = scene->context->player_id;
    int my_prev = get_player_point(scene->prev_game_score, my_id);
    int my_curr = get_player_point(scene->game_score, my_id);
    if (my_curr > my_prev)
        audio_play_se(&scene->audio, scene->se_yatta);
    scene->prev_game_score = scene->game_score;

    scene->server_sync_counter++;
    if (scene->server_sync_counter >= SERVER_SYNC_INTERVAL)
        scene->server_sync_counter = 0;

    for (int i = 0; i < PLAYER_MAX; i++)
    {
        scene->player_data[i] = scene->network->network_data_set.players[i];
        EZ_ObjectSetPosition(scene->player_objects[i], scene->player_data[i].point.x,
                             scene->player_data[i].point.y, scene->player_data[i].point.z);
        update_player_scale(scene, i);
    }

    game_scene_update_camera(scene);
    game_scene_send_player_input(scene, player_input);
    ability_manager_update(&scene->ability_manager, scene->joycon);

    AbilityActivateRequest *instant_req = ability_check_instant(&scene->ability_manager, my_id);
    if (instant_req)
        game_scene_send_ability_request(scene, instant_req);

    handle_ability_buttons(scene, my_id);

    if (player_swing)
    {
        AbilityActivateRequest *swing_req = ability_check_on_swing(&scene->ability_manager, my_id);
        if (swing_req)
            game_scene_send_ability_request(scene, swing_req);
    }

    game_scene_send_player_swing(scene, player_swing);

    if (ball_hit)
    {
        AbilityActivateRequest *hit_req = ability_check_on_hit(&scene->ability_manager, my_id);
        if (hit_req)
            game_scene_send_ability_request(scene, hit_req);
    }

    ability_manager_tick(&scene->ability_manager);
    scene->prev_hit_count = scene->ball_data.hit_count;
    scene->current_phase = scene->network->network_data_set.game_phase;

    int winner = scene->network->network_data_set.match_winner;
    if (winner >= 0 && !scene->is_game_finished)
    {
        scene->is_game_finished = true;
        scene->winner_id = winner;
        LOG_SUCCESS("試合終了 勝者: Player" << (winner + 1));
    }

    return GAME_RESULT_CONTINUE;
}

static void draw_clone(GameScene *scene, int player_idx, float offset_x)
{
    float x = scene->player_data[player_idx].point.x;
    float y = scene->player_data[player_idx].point.y;
    float z = scene->player_data[player_idx].point.z;

    EZ_ObjectSetPosition(scene->player_objects[player_idx], x - offset_x, y, z);
    EZ_DrawObject(scene->player_objects[player_idx], scene->shader, scene->camera, scene->light);

    EZ_ObjectSetPosition(scene->player_objects[player_idx], x + offset_x, y, z);
    EZ_DrawObject(scene->player_objects[player_idx], scene->shader, scene->camera, scene->light);

    EZ_ObjectSetPosition(scene->player_objects[player_idx], x, y, z);
}

static void draw_centered_text(GameScene *scene, const char *text, float size, float y, float r, float g, float b)
{
    float w = static_cast<float>(scene->context->window_width);
    float tw = EZ_2D_GetTextWidth(scene->font, text, size);
    EZ_2D_DrawText(scene->font, (w - tw) / 2.0f, y, text, size, r, g, b, 1.0f);
}

void game_scene_draw(GameScene *scene)
{
    EZ_BackgroundClear(scene->context->background_r, scene->context->background_g, scene->context->background_b);

    if (scene->ground_object)
        EZ_DrawObject(scene->ground_object, scene->shader, scene->camera, scene->light);

    if (scene->court_object)
        EZ_DrawObject(scene->court_object, scene->shader, scene->camera, scene->light);

    bool ball_invisible = ability_is_local_active(&scene->ability_manager, ABILITY_INVISIBLE_BALL);
    if (scene->ball_object && !ball_invisible)
        EZ_DrawObject(scene->ball_object, scene->shader, scene->camera, scene->light);

    constexpr float CLONE_OFFSET = 3.0f;
    for (int i = 0; i < PLAYER_MAX; i++)
    {
        if (!scene->player_objects[i]) continue;

        EZ_DrawObject(scene->player_objects[i], scene->shader, scene->camera, scene->light);

        const AbilityState &state = scene->network->network_data_set.ability_states[i];
        if (state.active_ability == ABILITY_CLONE && state.remaining_frames > 0)
            draw_clone(scene, i, CLONE_OFFSET);
    }

    if (scene->is_game_finished)
    {
        EZ_LightSetColor(scene->light, 0.0f, 0.0f, 0.0f);
        EZ_BackgroundClear(0, 0, 0, 1);

        float h = static_cast<float>(scene->context->window_height);
        bool won = (scene->winner_id == scene->context->player_id);
        const char *text = won ? "Win!!" : "Loose...";
        float r = won ? 1.0f : 0.6f, g = won ? 0.9f : 0.6f, b = won ? 0.0f : 0.6f;

        draw_centered_text(scene, text, 120.0f, h / 2.0f, r, g, b);
        draw_centered_text(scene, "Press Enter", 30.0f, h / 2.0f + 150.0f, 1.0f, 1.0f, 1.0f);
    }

    if (scene->network_error)
    {
        EZ_LightSetColor(scene->light, 0.0f, 0.0f, 0.0f);
        EZ_BackgroundClear(0, 0, 0, 1);

        float h = static_cast<float>(scene->context->window_height);
        draw_centered_text(scene, "サーバーとの接続が切れました", 40.0f, h / 2.0f - 20.0f, 1.0f, 0.3f, 0.3f);
        draw_centered_text(scene, "タイトルに戻ります...", 28.0f, h / 2.0f + 50.0f, 0.8f, 0.8f, 0.8f);
    }

    draw_score(scene);
}

void draw_score(GameScene *scene)
{
    if (!scene->font) return;

    int my_id = scene->context->player_id;
    int opp_id = (my_id == 0) ? 1 : 0;
    const GameScore &score = scene->network->network_data_set.game_score;

    char point_text[64];
    snprintf(point_text, sizeof(point_text), "%d : %d",
             get_player_point(score, my_id), get_player_point(score, opp_id));

    char set_text[64];
    snprintf(set_text, sizeof(set_text), "%d - %d",
             get_player_sets(score, my_id), get_player_sets(score, opp_id));

    float w = static_cast<float>(scene->context->window_width);

    float pw = EZ_2D_GetTextWidth(scene->font, point_text, SCORE_FONT_SIZE);
    EZ_2D_DrawText(scene->font, (w - pw) / 2.0f, SCORE_POS_Y, point_text, SCORE_FONT_SIZE, 1.0f, 1.0f, 1.0f, 1.0f);

    float sw = EZ_2D_GetTextWidth(scene->font, set_text, SET_SCORE_FONT_SIZE);
    EZ_2D_DrawText(scene->font, (w - sw) / 2.0f, SCORE_POS_Y + SET_SCORE_OFFSET_Y, set_text, SET_SCORE_FONT_SIZE, 1.0f, 1.0f, 1.0f, 1.0f);
}
