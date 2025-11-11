#include <stdio.h>

#include "scene_manager/scene_manager.hpp"

int main(void)
{
    init_scene_manager(SCENE_TITLE);
    change_scene(SCENE_GAME);
    update_scene();

    return 0;
}