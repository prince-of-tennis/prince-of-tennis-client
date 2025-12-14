#pragma once

#include <memory>

struct _EZ_Shader
{
    unsigned int program;          // シェーダープログラムID
    unsigned int model_loc;        // モデル行列のロケーション
    unsigned int view_loc;         // ビュー行列のロケーション
    unsigned int proj_loc;         // 投影行列のロケーション
    unsigned int light_loc;        // ライト位置のロケーション
    unsigned int light_color_loc;  // ライト色のロケーション
    unsigned int view_pos_loc;     // カメラ位置のロケーション

    ~_EZ_Shader();
};

typedef std::shared_ptr<_EZ_Shader> EZ_Shader;

/// @brief デフォルトShaderの作成
/// @return 作成されたShader
EZ_Shader EZ_CreateShader();

/// @brief カスタムShaderの作成
/// @param vertex_file_path .vertファイルのパス
/// @param fragment_file_path .fragファイルのパス
/// @return 作成されたShader
EZ_Shader EZ_CreateCustomShader(const char *vertex_file_path, const char *fragment_file_path);

/// @brief Shaderの初期化
/// @param shader OpenGLShader
/// @param vertex_file .vertファイルのパス
/// @param fragment_file .fragファイルのパス
/// @return 成功したか
bool opengl_shader_init(_EZ_Shader *shader, const char *vertex_file, const char *fragment_file);

/// @brief Shaderの解放
/// @param shader OpenGLShader
void _EZ_DestroyShader(_EZ_Shader *shader);

/// @brief Shaderを使用
/// @param shader OpenGLShader
void _EZ_UseShader(_EZ_Shader *shader);
