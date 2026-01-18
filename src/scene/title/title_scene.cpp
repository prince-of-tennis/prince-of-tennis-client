#include "title_scene.hpp"

#include "glad/glad.h"
#include "util/log.hpp"

bool title_scene_init(TitleScene *scene)
{
    scene->font = EZ_2D_CreateFont("fonts/font.otf", 48);
    if (!scene->font)
    {
        LOG_ERROR("タイトルシーン: フォントの読み込みに失敗しました");
        return false;
    }

    scene->blink_counter = 0;
    scene->show_start_message = true;

    LOG_DEBUG("タイトルシーンを初期化しました");
    return true;
}

bool title_scene_update(TitleScene *scene)
{
    // 点滅アニメーション更新
    scene->blink_counter++;
    if (scene->blink_counter >= BLINK_INTERVAL)
    {
        scene->blink_counter = 0;
        scene->show_start_message = !scene->show_start_message;
    }

    // Aボタンが押されたらシーン遷移
    if (scene->joycon != nullptr && scene->joycon->joycon.button.btn.A)
    {
        LOG_DEBUG("タイトルシーン: Aボタン入力を検出、ゲームシーンに遷移します");
        return true;
    }

    return false;
}

void title_scene_draw(TitleScene *scene)
{
    // 背景を黒でクリア
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float screen_width = static_cast<float>(scene->context->window_width);
    float screen_height = static_cast<float>(scene->context->window_height);

    // タイトルテキストを中央に描画
    const char *title_text = "超次元テニス";
    float title_x = screen_width / 2.0f - 300.0f;
    float title_y = screen_height * 0.3f;
    EZ_2D_DrawText(scene->font, title_x, title_y, title_text, TITLE_FONT_SIZE, 1.0f, 1.0f, 1.0f,
                   1.0f);

    // 開始メッセージ（点滅）
    if (scene->show_start_message)
    {
        const char *start_text = "Press A";
        float start_x = screen_width / 2.0f - 200.0f;
        float start_y = screen_height * 0.6f;
        EZ_2D_DrawText(scene->font, start_x, start_y, start_text, START_MSG_FONT_SIZE, 0.8f, 0.8f,
                       0.8f, 1.0f);
    }
}

void title_scene_fini(TitleScene *scene)
{
    scene->font.reset();
    LOG_DEBUG("タイトルシーンを終了しました");
}
