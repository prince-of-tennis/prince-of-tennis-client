#pragma once

struct OpenGLShader
{
    unsigned int program;
    unsigned int model_loc;
    unsigned int view_loc;
    unsigned int proj_loc;
    unsigned int light_loc;
    unsigned int light_color_loc;
    unsigned int view_pos_loc;
};

/// @brief Shaderの初期化
/// @param shader OpenGLShader
/// @param vertex_file .vertファイルのパス
/// @param fragment_file .fragファイルのパス
/// @return 成功したか
bool opengl_shader_init(OpenGLShader *shader, const char *vertex_file, const char *fragment_file);

/// @brief Shaderの解放
/// @param shader OpenGLShader
void opengl_shader_destroy(OpenGLShader *shader);

/// @brief Shaderを使用
/// @param shader OpenGLShader
void opengl_shader_use(OpenGLShader *shader);