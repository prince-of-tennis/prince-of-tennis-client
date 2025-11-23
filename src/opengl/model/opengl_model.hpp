#pragma once

#include <string>

using namespace std;

struct OpenGLModel
{
    const char *model_file;
    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;
    unsigned int index_count;
};

bool opengl_model_init(OpenGLModel *model, string model_file);

/// @brief モデルの解放
void opengl_model_destroy(OpenGLModel *model);

/// @brief モデルの描画
void opengl_model_draw(OpenGLModel *model);