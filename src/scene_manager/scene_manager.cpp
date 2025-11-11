#include "scene_manager.hpp"

SDL_bool update_scene(scene_type scene)
{
    switch (scene)
    {
        case SCENE_GAME:
            printf("GameScene\n");
            break;

        case SCENE_TITLE:
            printf("TitleScene\n");
            break;
    }

    return SDL_TRUE;
}