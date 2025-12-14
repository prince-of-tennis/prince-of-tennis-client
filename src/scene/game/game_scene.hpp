#pragma once

#include "core/context.hpp"
#include "network/network.hpp"
#include "opengl/2d/EZ_2d.h"
#include "opengl/camera/EZ_Camera.hpp"
#include "opengl/light/EZ_Light.hpp"
#include "opengl/object/EZ_Object.hpp"
#include "opengl/shader/EZ_Shader.hpp"
#include "util/helper.hpp"

struct GameScene
{
    EZ_Object obj;
    EZ_Shader shader;
    EZ_Camera camera;
    EZ_Light light;
    Context *context;

    UniquePtr<Network> network;

    // 2D描画用
    EZ_2D_Font font;
    EZ_2D_Image test_image;
};

bool game_scene_init(GameScene *scene);

bool game_scene_update(GameScene *scene);

void game_scene_draw(GameScene *scene);
