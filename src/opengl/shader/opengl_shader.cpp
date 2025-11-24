#include "opengl_shader.hpp"

#include <SDL2/SDL.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "opengl/glad/glad.h"
#include "util/log.hpp"

bool opengl_shader_init(OpenGLShader *shader, const char *vertex_file, const char *fragment_file)
{
    LOG_DEBUG("シェーダー初期化開始: vertex=" << vertex_file << ", fragment=" << fragment_file);
    std::string vertex_shader_source;
    std::string fragment_shader_source;

    try
    {
        std::ifstream vertex_shader_file(vertex_file);
        if (!vertex_shader_file.is_open())
        {
            throw std::runtime_error("Failed to open vertex file");
        }
        std::stringstream vertex_stream;
        vertex_stream << vertex_shader_file.rdbuf();
        vertex_shader_source = vertex_stream.str();
        vertex_shader_file.close();
    }
    catch (const std::exception &e)
    {
        LOG_ERROR(e.what());
        return false;
    }

    try
    {
        std::ifstream fragment_shader_file(fragment_file);
        if (!fragment_shader_file.is_open())
        {
            throw std::runtime_error("Failed to open fragment file");
        }
        std::stringstream fragment_stream;
        fragment_stream << fragment_shader_file.rdbuf();
        fragment_shader_source = fragment_stream.str();
        fragment_shader_file.close();
    }
    catch (const std::exception &e)
    {
        LOG_ERROR(e.what());
        return false;
    }

    const char *vertex_shader_code = vertex_shader_source.c_str();
    const char *fragment_shader_code = fragment_shader_source.c_str();

    // MARK: Compile

    unsigned int vertex_shader;
    unsigned int fragment_shader;
    int success;
    char infoLog[512];

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_code, NULL);
    glCompileShader(vertex_shader);

    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
        LOG_ERROR("Vertex: " << infoLog);
        return false;
    }

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_code, NULL);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
        LOG_ERROR("Shader: " << infoLog);
        return false;
    }

    unsigned int shader_program;
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);

    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
        LOG_ERROR("Shader Program Linking Failed: " << infoLog);
        return false;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    shader->program = shader_program;
    shader->model_loc = glGetUniformLocation(shader_program, "model");
    shader->view_loc = glGetUniformLocation(shader_program, "view");
    shader->proj_loc = glGetUniformLocation(shader_program, "projection");

    shader->light_loc = glGetUniformLocation(shader_program, "lightPos");
    shader->light_color_loc = glGetUniformLocation(shader_program, "lightColor");
    shader->view_pos_loc = glGetUniformLocation(shader_program, "viewPos");

    if (shader->model_loc == -1 || shader->view_loc == -1 || shader->proj_loc == -1 ||
        shader->light_loc == -1 || shader->light_color_loc == -1 || shader->view_pos_loc == -1)
    {
        LOG_ERROR("uniform variable not found");
        return false;
    }

    LOG_DEBUG("シェーダー初期化完了: program=" << shader->program);
    return true;
}

void opengl_shader_destroy(OpenGLShader *shader)
{
    if (shader->program != 0)
    {
        glDeleteProgram(shader->program);
        shader->program = 0;
    }
}

void opengl_shader_use(OpenGLShader *shader)
{
    glUseProgram(shader->program);
}