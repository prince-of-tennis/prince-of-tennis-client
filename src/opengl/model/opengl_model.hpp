#pragma once

#include <string>
#include <vector>

#include "glad/glad.h"
#include "opengl/mesh/opengl_mesh.hpp"

using namespace std;

struct OpenGLModel
{
    string model_file;
    vector<OpenGLMesh> meshes;
};

bool opengl_model_init(OpenGLModel *model, string model_file);

/// @brief モデルの解放
void opengl_model_destroy(OpenGLModel *model);

/// @brief モデルの描画
void opengl_model_draw(OpenGLModel *model);