#include "opengl_model.hpp"

#include <SDL2/SDL.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>
#include <vector>

#include "glad/glad.h"
#include "util/log.hpp"

using namespace std;

bool opengl_model_init(OpenGLModel *model, string model_file)
{
    LOG_DEBUG("モデル読み込み開始: " << model_file);
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    Assimp::Importer importer;
    model->model_file = model_file;
    const aiScene *scene =
        importer.ReadFile(model->model_file.c_str(),
                          aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        LOG_ERROR("Assimpモデル読み込み失敗: " << importer.GetErrorString());
        return false;
    }
    LOG_DEBUG("Assimpモデル読み込み成功: meshes=" << scene->mNumMeshes);

    aiMesh *mesh = scene->mMeshes[0];

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        vertices.push_back(mesh->mVertices[i].x);
        vertices.push_back(mesh->mVertices[i].y);
        vertices.push_back(mesh->mVertices[i].z);

        if (mesh->mTextureCoords[0])
        {
            vertices.push_back(mesh->mTextureCoords[0][i].x);
            vertices.push_back(mesh->mTextureCoords[0][i].y);
        }
        else
        {
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
        }

        vertices.push_back(mesh->mNormals[i].x);
        vertices.push_back(mesh->mNormals[i].y);
        vertices.push_back(mesh->mNormals[i].z);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    model->index_count = indices.size();
    LOG_DEBUG("モデルデータ: vertices=" << vertices.size() / 8 << ", indices=" << indices.size());

    glGenVertexArrays(1, &model->vao);
    glGenBuffers(1, &model->vbo);
    glGenBuffers(1, &model->ebo);
    LOG_DEBUG("OpenGL バッファ生成: VAO=" << model->vao << ", VBO=" << model->vbo
                                          << ", EBO=" << model->ebo);

    glBindVertexArray(model->vao);

    glBindBuffer(GL_ARRAY_BUFFER, model->vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    LOG_DEBUG("モデル初期化完了: index_count=" << model->index_count);
    return true;
}

void opengl_model_destroy(OpenGLModel *model)
{
    if (model->vao != 0)
    {
        glDeleteVertexArrays(1, &model->vao);
        model->vao = 0;
    }

    if (model->vbo != 0)
    {
        glDeleteBuffers(1, &model->vbo);
        model->vbo = 0;
    }

    if (model->ebo != 0)
    {
        glDeleteBuffers(1, &model->ebo);
        model->ebo = 0;
    }
}

void opengl_model_draw(OpenGLModel *model)
{
    glBindVertexArray(model->vao);
    glDrawElements(GL_TRIANGLES, model->index_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}