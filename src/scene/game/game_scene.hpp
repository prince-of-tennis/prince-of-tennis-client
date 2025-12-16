#pragma once

#include "common/ball.h"
#include "common/player.h"
#include "common/player_input.h"
#include "core/context.hpp"
#include "network/network.hpp"
#include "opengl/2d/EZ_2d.h"
#include "opengl/camera/EZ_Camera.hpp"
#include "opengl/light/EZ_Light.hpp"
#include "opengl/object/EZ_Object.hpp"
#include "opengl/shader/EZ_Shader.hpp"
#include "util/helper.hpp"

#define PLAYER_MAX 2

struct GameScene
{
    EZ_Shader shader;
    EZ_Camera camera;
    EZ_Light light;
    Context *context;

    UniquePtr<Network> network;

    // ボール
    EZ_Object ball_object;
    Ball ball_data;

    // プレイヤー
    EZ_Object player_objects[PLAYER_MAX];
    Player player_data[PLAYER_MAX];

    // 2D描画用
    EZ_2D_Font font;
    EZ_2D_Image test_image;
};

bool game_scene_init(GameScene *scene);

bool game_scene_update(GameScene *scene, PlayerInput *player_input);

void game_scene_draw(GameScene *scene);
