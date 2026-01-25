#include "title_scene.hpp"

#include "glad/glad.h"
#include "input/input_manager.hpp"
#include "util/log.hpp"

// テニスコートスケール（game_sceneと同じ）
constexpr float TENNIS_COURT_SCALE = 0.2f;

bool title_scene_init(TitleScene *scene)
{
    // フォント読み込み
    scene->font = EZ_2D_CreateFont("fonts/font.otf", 48);
    if (!scene->font)
    {
        LOG_ERROR("タイトルシーン: フォントの読み込みに失敗しました");
        return false;
    }

    // 3Dシェーダー初期化
    scene->shader = EZ_CreateShader();
    if (scene->shader == nullptr)
    {
        LOG_ERROR("タイトルシーン: シェーダーの作成に失敗しました");
        return false;
    }

    // カメラ初期化（俯瞰視点）
    float aspect_ratio = static_cast<float>(scene->context->window_width) /
                         static_cast<float>(scene->context->window_height);
    scene->camera = EZ_CreateCamera(aspect_ratio);
    EZ_CameraSetPosition(scene->camera, 0.0f, 15.0f, 25.0f);
    EZ_CameraSetTargetPosition(scene->camera, 0.0f, 0.0f, 0.0f);
    EZ_CameraSetClipPlanes(scene->camera, 0.1f, 200.0f);

    // ライト初期化
    scene->light = EZ_CreateLight();
    EZ_LightSetPosition(scene->light, 10.0f, 20.0f, 10.0f);

    // テニスコートオブジェクト読み込み
    scene->court_object = EZ_CreateObject("obj/tennis_court.obj", "img/container.jpeg");
    if (scene->court_object == nullptr)
    {
        LOG_ERROR("タイトルシーン: テニスコートオブジェクトの作成に失敗しました");
        return false;
    }
    EZ_ObjectSetPosition(scene->court_object, 0.0f, 0.0f, 0.0f);
    EZ_ObjectSetScale(scene->court_object, TENNIS_COURT_SCALE, TENNIS_COURT_SCALE,
                      TENNIS_COURT_SCALE);

    // オーディオ初期化
    if (!audio_init(&scene->audio))
    {
        LOG_ERROR("タイトルシーン: オーディオの初期化に失敗しました");
        return false;
    }

    // SE読み込み
    scene->se_cursor_move = audio_load_se(&scene->audio, "audio/se/move_cursor.mp3");
    scene->se_decide = audio_load_se(&scene->audio, "audio/se/select.mp3");

    // 変数初期化
    scene->blink_counter = 0;
    scene->show_start_message = true;
    scene->court_rotation = 0.0f;
    scene->frame_counter = 0;
    scene->title_alpha = 0.0f;
    scene->fade_in_complete = false;
    scene->selected_menu = MENU_START;
    scene->prev_stick_up = false;
    scene->prev_stick_down = false;

    LOG_DEBUG("タイトルシーンを初期化しました");
    return true;
}

TitleSceneResult title_scene_update(TitleScene *scene)
{
    // フレームカウンター更新
    scene->frame_counter++;

    // フェードインアニメーション
    if (!scene->fade_in_complete)
    {
        scene->title_alpha =
            static_cast<float>(scene->frame_counter) / static_cast<float>(FADE_IN_FRAMES);
        if (scene->title_alpha >= 1.0f)
        {
            scene->title_alpha = 1.0f;
            scene->fade_in_complete = true;
        }
    }

    // コート回転更新
    scene->court_rotation += COURT_ROTATION_SPEED;
    if (scene->court_rotation >= 360.0f)
    {
        scene->court_rotation -= 360.0f;
    }
    EZ_ObjectSetRotation(scene->court_object, 0.0f, scene->court_rotation, 0.0f);

    // 点滅アニメーション更新
    scene->blink_counter++;
    if (scene->blink_counter >= BLINK_INTERVAL)
    {
        scene->blink_counter = 0;
        scene->show_start_message = !scene->show_start_message;
    }

    // 入力処理（フェードイン完了後のみ）
    if (scene->fade_in_complete)
    {
        bool menu_up = false;
        bool menu_down = false;
        bool menu_select = false;

        // Joy-Con 入力
        if (scene->joycon != nullptr)
        {
            // スティック入力の閾値
            constexpr float STICK_THRESHOLD = 0.5f;

            // 現在のスティック状態を取得
            bool stick_up = scene->joycon->joycon.stick.y > STICK_THRESHOLD;
            bool stick_down = scene->joycon->joycon.stick.y < -STICK_THRESHOLD;

            // 上方向のエッジ検出（今押された瞬間）
            if (stick_up && !scene->prev_stick_up)
            {
                menu_up = true;
            }
            // 下方向のエッジ検出（今押された瞬間）
            if (stick_down && !scene->prev_stick_down)
            {
                menu_down = true;
            }

            // スティック状態を保存
            scene->prev_stick_up = stick_up;
            scene->prev_stick_down = stick_down;

            // A ボタンで決定
            if (joycon_is_just_pressed(scene->joycon, JOYCON_BTN_A))
            {
                menu_select = true;
            }
        }

        // キーボード入力
        if (input_is_key_just_pressed(KEY_UP))
        {
            menu_up = true;
        }
        if (input_is_key_just_pressed(KEY_DOWN))
        {
            menu_down = true;
        }
        if (input_is_key_just_pressed(KEY_ENTER))
        {
            menu_select = true;
        }

        // メニュー上移動
        if (menu_up)
        {
            scene->selected_menu--;
            if (scene->selected_menu < 0)
            {
                scene->selected_menu = MENU_ITEM_COUNT - 1;
            }
            if (scene->se_cursor_move >= 0)
            {
                audio_play_se(&scene->audio, scene->se_cursor_move);
            }
        }

        // メニュー下移動
        if (menu_down)
        {
            scene->selected_menu++;
            if (scene->selected_menu >= MENU_ITEM_COUNT)
            {
                scene->selected_menu = 0;
            }
            if (scene->se_cursor_move >= 0)
            {
                audio_play_se(&scene->audio, scene->se_cursor_move);
            }
        }

        // メニュー決定
        if (menu_select)
        {
            if (scene->se_decide >= 0)
            {
                audio_play_se(&scene->audio, scene->se_decide);
            }

            switch (scene->selected_menu)
            {
                case MENU_START:
                    LOG_DEBUG("タイトルシーン: 「はじめる」が選択されました");
                    return TITLE_RESULT_START;
                case MENU_EXIT:
                    LOG_DEBUG("タイトルシーン: 「おわる」が選択されました");
                    return TITLE_RESULT_EXIT;
                default:
                    break;
            }
        }
    }

    return TITLE_RESULT_NONE;
}

void title_scene_draw(TitleScene *scene)
{
    // 背景をセピア調（古い紙風）でクリア
    glClearColor(0.85f, 0.78f, 0.65f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 3D描画（深度テスト有効）
    glEnable(GL_DEPTH_TEST);

    // コート描画（回転適用済み）
    EZ_DrawObject(scene->court_object, scene->shader, scene->camera, scene->light);

    float screen_width = static_cast<float>(scene->context->window_width);
    float screen_height = static_cast<float>(scene->context->window_height);

    const char *title_text = "超次元テニス";
    float title_x = screen_width / 2.0f - 220.0f;
    float title_y = screen_height * 0.25f;
    EZ_2D_DrawText(scene->font, title_x, title_y, title_text, TITLE_FONT_SIZE, 0.0f, 0.0f, 0.0f,
                   scene->title_alpha);

    // メニュー項目（フェードイン完了後のみ表示）
    if (scene->fade_in_complete)
    {
        const char *menu_items[] = {"はじめる", "おわる"};
        float menu_x = screen_width / 2.0f - 80.0f;

        for (int i = 0; i < MENU_ITEM_COUNT; i++)
        {
            float menu_y = MENU_START_Y + i * MENU_ITEM_SPACING;

            // 選択中のメニューは色を変える（こげ茶 / グレー）
            float r = (i == scene->selected_menu) ? 0.4f : 0.5f;
            float g = (i == scene->selected_menu) ? 0.25f : 0.45f;
            float b = (i == scene->selected_menu) ? 0.1f : 0.4f;

            // カーソル（▶）
            if (i == scene->selected_menu)
            {
                EZ_2D_DrawText(scene->font, menu_x - 50.0f, menu_y, "▶", MENU_FONT_SIZE, r, g, b,
                               1.0f);
            }

            EZ_2D_DrawText(scene->font, menu_x, menu_y, menu_items[i], MENU_FONT_SIZE, r, g, b,
                           1.0f);
        }
    }
}

void title_scene_fini(TitleScene *scene)
{
    // オーディオ終了
    audio_fini(&scene->audio);

    LOG_DEBUG("タイトルシーンを終了しました");
}
