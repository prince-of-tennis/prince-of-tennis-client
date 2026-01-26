#include "game_scene_camera.hpp"

#include "game_scene.hpp"
#include "opengl/EasyGL.hpp"

void game_scene_update_camera(GameScene *scene)
{
    int my_player_id = scene->context->player_id;
    if (my_player_id < 0 || my_player_id >= PLAYER_MAX)
    {
        return;
    }

    Player &my_player = scene->player_data[my_player_id];

    // カメラはプレイヤーの後方に配置
    // player_id == 0: Z正方向（コート奥側から見る）
    // player_id == 1: Z負方向（コート手前側から見る）
    float camera_z_direction = (my_player_id == 0) ? CAMERA_POS_Z : -CAMERA_POS_Z;

    // カメラ位置 = プレイヤー位置 + オフセット
    float camera_x = my_player.point.x + CAMERA_POS_X;
    float camera_y = CAMERA_POS_Y;
    float camera_z = my_player.point.z + camera_z_direction;

    EZ_CameraSetPosition(scene->camera, camera_x, camera_y, camera_z);

    // 注視点: プレイヤーの位置（Y軸は固定高さ）
    float target_x = my_player.point.x;
    float target_y = CAMERA_TARGET_Y;
    float target_z = my_player.point.z;

    EZ_CameraSetTargetPosition(scene->camera, target_x, target_y, target_z);
}

void game_scene_init_camera_position(GameScene *scene)
{
    // プレイヤーIDに応じてカメラの初期Z位置を決定
    // player_id == 0: コート奥側から見る（Z正方向）
    // player_id == 1: コート手前側から見る（Z負方向）
    float camera_z = (scene->context->player_id == 0) ? CAMERA_POS_Z : -CAMERA_POS_Z;
    EZ_CameraSetPosition(scene->camera, CAMERA_POS_X, CAMERA_POS_Y, camera_z);
    EZ_CameraSetTargetPosition(scene->camera, CAMERA_TARGET_X, CAMERA_TARGET_Y, CAMERA_TARGET_Z);
}
