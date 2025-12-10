#pragma once

#include <memory>
#include <string>
#include <vector>

#include "opengl/mesh/EZ_Mesh.hpp"

using namespace std;

struct _EZ_Model
{
    string model_file_path;
    vector<EZ_Mesh> meshes;

    ~_EZ_Model();
};

typedef shared_ptr<_EZ_Model> EZ_Model;

/// @brief モデルの作成
/// @param model_file_path モデルファイルパス
/// @return 作成されたモデル
EZ_Model EZ_CreateModel(string model_file_path);

/// @brief モデルの解放
void _EZ_DestroyModel(_EZ_Model *model);

/// @brief モデルの描画
void _EZ_DrawModel(_EZ_Model *model);
