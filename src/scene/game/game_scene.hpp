#pragma once

#include "core/context.hpp"
#include "network/network.hpp"
#include "opengl/camera/opengl_camera.hpp"
#include "opengl/light/opengl_light.hpp"
#include "opengl/object/opengl_object.hpp"
#include "opengl/shader/opengl_shader.hpp"
#include "util/helper.hpp"

struct GameScene
{
    UniquePtr<OpenGLObject> obj;
    OpenGLShader shader;
    OpenGLCamera camera;
    OpenGLLight light;
    Context *context;

    UniquePtr<Network> network;
};

bool game_scene_init(GameScene *scene);

bool game_scene_update(GameScene *scene);

void game_scene_draw(GameScene *scene);
