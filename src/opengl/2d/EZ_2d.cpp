#include "EZ_2d.h"

#include <cmath>

#include "font/EZ_2d_font.h"
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "image/EZ_2d_image.h"
#include "util/log.hpp"

using namespace std;

// フォント用の外部関数（font/EZ_2d_font.cppで定義）
extern bool _EZ_2D_InitFreeType();
extern void _EZ_2D_DestroyFreeType();

// 頂点シェーダー: スクリーン座標 → NDC変換
static const char *VERTEX_SHADER_SOURCE = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 projection;
uniform mat4 model;

void main()
{
    gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)";

// フラグメントシェーダー: 色付け
static const char *FRAGMENT_SHADER_SOURCE = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform vec4 color;
uniform bool useTexture;
uniform bool isText;
uniform sampler2D tex;

void main()
{
    if (useTexture)
    {
        if (isText)
        {
            // テキスト描画の場合: アルファチャンネルのみ使用
            vec4 sampled = vec4(1.0, 1.0, 1.0, texture(tex, TexCoord).r);
            FragColor = color * sampled;
        }
        else
        {
            // 画像描画の場合: フルカラー
            FragColor = texture(tex, TexCoord) * color;
        }
    }
    else
    {
        FragColor = color;
    }
}
)";

GLuint g_shader_program = 0;
GLint g_uniform_projection = -1;
GLint g_uniform_model = -1;
GLint g_uniform_color = -1;
GLint g_uniform_use_texture = -1;
GLint g_uniform_is_text = -1;

GLuint g_rect_vao = 0;
static GLuint g_rect_vbo = 0;

static GLuint g_circle_vao = 0;
static GLuint g_circle_vbo = 0;

static int g_screen_width = 0;
static int g_screen_height = 0;

glm::mat4 g_projection_matrix;

// テキスト描画用のVAO/VBO
GLuint g_text_vao = 0;
GLuint g_text_vbo = 0;

static GLuint _EZ_2D_CompileShader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        LOG_ERROR("シェーダーのコンパイルに失敗: " << info_log);
        return 0;
    }

    return shader;
}

static bool _EZ_2D_InitShader()
{
    GLuint vertex_shader = _EZ_2D_CompileShader(GL_VERTEX_SHADER, VERTEX_SHADER_SOURCE);
    if (vertex_shader == 0) return false;

    GLuint fragment_shader = _EZ_2D_CompileShader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER_SOURCE);
    if (fragment_shader == 0)
    {
        glDeleteShader(vertex_shader);
        return false;
    }

    g_shader_program = glCreateProgram();
    glAttachShader(g_shader_program, vertex_shader);
    glAttachShader(g_shader_program, fragment_shader);
    glLinkProgram(g_shader_program);

    GLint success;
    glGetProgramiv(g_shader_program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char info_log[512];
        glGetProgramInfoLog(g_shader_program, 512, NULL, info_log);
        LOG_ERROR("シェーダープログラムのリンクに失敗: " << info_log);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return false;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    g_uniform_projection = glGetUniformLocation(g_shader_program, "projection");
    g_uniform_model = glGetUniformLocation(g_shader_program, "model");
    g_uniform_color = glGetUniformLocation(g_shader_program, "color");
    g_uniform_use_texture = glGetUniformLocation(g_shader_program, "useTexture");
    g_uniform_is_text = glGetUniformLocation(g_shader_program, "isText");

    LOG_SUCCESS("2Dシェーダー初期化完了");
    return true;
}

static bool _EZ_2D_InitRectMesh()
{
    float vertices[] = {
        // 位置       // テクスチャ座標
        0.0f, 0.0f, 0.0f, 0.0f,  // 左上
        1.0f, 0.0f, 1.0f, 0.0f,  // 右上
        1.0f, 1.0f, 1.0f, 1.0f,  // 右下

        1.0f, 1.0f, 1.0f, 1.0f,  // 右下
        0.0f, 1.0f, 0.0f, 1.0f,  // 左下
        0.0f, 0.0f, 0.0f, 0.0f   // 左上
    };

    glGenVertexArrays(1, &g_rect_vao);
    glGenBuffers(1, &g_rect_vbo);

    glBindVertexArray(g_rect_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_rect_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    LOG_SUCCESS("矩形メッシュ初期化完了");
    return true;
}

static bool _EZ_2D_InitCircleMesh()
{
    const int segments = 32;
    const float radius = 1.0f;

    // 円を三角形ファンで構成
    vector<float> vertices;
    // 中心点
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(0.5f);
    vertices.push_back(0.5f);

    // 円周上の点
    for (int i = 0; i <= segments; i++)
    {
        float angle = 2.0f * glm::pi<float>() * i / segments;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back((x + 1.0f) * 0.5f);
        vertices.push_back((y + 1.0f) * 0.5f);
    }

    glGenVertexArrays(1, &g_circle_vao);
    glGenBuffers(1, &g_circle_vbo);

    glBindVertexArray(g_circle_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_circle_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    LOG_SUCCESS("円メッシュ初期化完了");
    return true;
}

static bool _EZ_2D_InitTextVAO()
{
    // テキスト描画用のVAO/VBO
    glGenVertexArrays(1, &g_text_vao);
    glGenBuffers(1, &g_text_vbo);

    glBindVertexArray(g_text_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_text_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    LOG_SUCCESS("テキストVAO初期化完了");
    return true;
}

bool EZ_2D_Init(int screen_width, int screen_height)
{
    LOG_DEBUG("2D描画システム初期化開始");

    g_screen_width = screen_width;
    g_screen_height = screen_height;

    // 正射影行列を作成 (左上原点、Y軸下向き)
    g_projection_matrix =
        glm::ortho(0.0f, (float)screen_width, (float)screen_height, 0.0f, -1.0f, 1.0f);

    // アルファブレンディングを有効化
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (!_EZ_2D_InitShader()) return false;
    if (!_EZ_2D_InitRectMesh()) return false;
    if (!_EZ_2D_InitCircleMesh()) return false;
    if (!_EZ_2D_InitTextVAO()) return false;

    // FreeTypeライブラリの初期化（フォントモジュール）
    if (!_EZ_2D_InitFreeType()) return false;

    LOG_SUCCESS("2D描画システム初期化完了");
    return true;
}

void _EZ_2D_Destroy()
{
    // シェーダーの解放
    if (g_shader_program != 0)
    {
        glDeleteProgram(g_shader_program);
        g_shader_program = 0;
    }

    // メッシュの解放
    if (g_rect_vao != 0) glDeleteVertexArrays(1, &g_rect_vao);
    if (g_rect_vbo != 0) glDeleteBuffers(1, &g_rect_vbo);
    if (g_circle_vao != 0) glDeleteVertexArrays(1, &g_circle_vao);
    if (g_circle_vbo != 0) glDeleteBuffers(1, &g_circle_vbo);
    if (g_text_vao != 0) glDeleteVertexArrays(1, &g_text_vao);
    if (g_text_vbo != 0) glDeleteBuffers(1, &g_text_vbo);

    // FreeTypeの解放（フォントモジュール）
    _EZ_2D_DestroyFreeType();

    LOG_SUCCESS("2D描画システム解放完了");
}

void EZ_2D_DrawRect(float x, float y, float width, float height, float r, float g, float b, float a)
{
    glUseProgram(g_shader_program);

    // モデル行列: 位置とスケール
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, 0.0f));
    model = glm::scale(model, glm::vec3(width, height, 1.0f));

    glUniformMatrix4fv(g_uniform_projection, 1, GL_FALSE, glm::value_ptr(g_projection_matrix));
    glUniformMatrix4fv(g_uniform_model, 1, GL_FALSE, glm::value_ptr(model));
    glUniform4f(g_uniform_color, r, g, b, a);
    glUniform1i(g_uniform_use_texture, 0);
    glUniform1i(g_uniform_is_text, 0);  // 矩形描画（テキストではない）

    glBindVertexArray(g_rect_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void EZ_2D_DrawCircle(float x, float y, float radius, float r, float g, float b, float a)
{
    glUseProgram(g_shader_program);

    // モデル行列: 中心位置とスケール
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, 0.0f));
    model = glm::scale(model, glm::vec3(radius, radius, 1.0f));

    glUniformMatrix4fv(g_uniform_projection, 1, GL_FALSE, glm::value_ptr(g_projection_matrix));
    glUniformMatrix4fv(g_uniform_model, 1, GL_FALSE, glm::value_ptr(model));
    glUniform4f(g_uniform_color, r, g, b, a);
    glUniform1i(g_uniform_use_texture, 0);
    glUniform1i(g_uniform_is_text, 0);  // 円描画（テキストではない）

    glBindVertexArray(g_circle_vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 34);  // 中心点 + 33点
    glBindVertexArray(0);
}
