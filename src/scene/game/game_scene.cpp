#include "game_scene.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glad/glad.h"
#include "network/network.hpp"
#include "opengl/2d/EZ_2d.h"
#include "util/log.hpp"

bool game_scene_init(GameScene *scene)
{
    LOG_DEBUG("GameScene初期化開始");

    // シェーダー初期化
    scene->shader = EZ_CreateShader();
    if (scene->shader == nullptr)
    {
        LOG_ERROR("シェーダーの作成に失敗しました");
        return false;
    }
    LOG_SUCCESS("シェーダー初期化完了");

    EZ_Model model = EZ_CreateModel("obj/tennis_court.obj");
    EZ_Texture texture = EZ_CreateTexture("img/container.jpeg");

    // オブジェクト作成
    scene->obj = EZ_CreateObjectFromModelTexture(model, texture);
    if (scene->obj == nullptr)
    {
        LOG_ERROR("オブジェクトの作成に失敗しました");
        return false;
    }
    // オブジェクトの位置を設定
    EZ_ObjectSetPosition(scene->obj, 0.0f, -1.0f, 0.0f);

    // カメラ初期化
    scene->camera = EZ_CreateCamera(static_cast<float>(scene->context->window_width) /
                                    static_cast<float>(scene->context->window_height));

    // ライト初期化
    scene->light = EZ_CreateLight();

    // Network
    scene->network.reset(new Network);
    if (!network_init(scene->network.get(), scene->context))
    {
        return false;
    }

    // フォントの読み込み（オプション）
    scene->font = EZ_2D_CreateFont("assets/fonts/font.otf", 48);
    if (!scene->font)
    {
        LOG_WARN("フォントの読み込みに失敗しました。テキスト描画は無効化されます。");
    }

    // 画像の読み込み（オプション）
    scene->test_image = EZ_2D_CreateImage("img/screen.jpg");
    if (!scene->test_image)
    {
        LOG_WARN("画像の読み込みに失敗しました。");
    }

    LOG_SUCCESS("GameScene初期化完了");
    return true;
}

bool game_scene_update(GameScene *scene)
{
    if (!network_listen_to_server(scene->network.get()))
    {
        return false;
    }

    return true;
}

void game_scene_draw(GameScene *scene)
{
    // 画面クリア
    glClearColor(scene->context->background_r, scene->context->background_g,
                 scene->context->background_b, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (scene->obj)
    {
        EZ_DrawObject(scene->obj, scene->shader, scene->camera, scene->light);
    }
    else
    {
        LOG_ERROR("描画対象のオブジェクトがnullです");
    }

    // 2D描画テスト
    EZ_2D_DrawRect(50, 50, 200, 100, 0.0f, 1.0f, 0.0f, 0.8f);  // 緑の半透明矩形
    EZ_2D_DrawCircle(400, 300, 50, 1.0f, 0.0f, 0.0f, 1.0f);    // 赤い円
    EZ_2D_DrawText(scene->font, 100, 200, "Hello, EZ_2D!", 32, 1.0f, 1.0f, 1.0f,
                   1.0f);                                                        // 白いテキスト
    EZ_2D_DrawImage(scene->test_image, 300, 150, 0, 0, 1.0f, 1.0f, 1.0f, 1.0f);  // 元のサイズで描画
    EZ_2D_DrawImage(scene->test_image, 500, 150, 100, 100, 1.0f, 1.0f, 1.0f,
                    0.5f);  // 100x100にリサイズ、半透明
}