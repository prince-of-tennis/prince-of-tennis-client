#include "looper.hpp"

#include "util/log.hpp"

void looper_init(Looper *looper, eSceneType default_scene)
{
    looper->scene_manager = make_unique<SceneManager>();
    scene_manager_init(looper->scene_manager, default_scene);
}

void loop(Looper *looper)
{
    SDL_bool is_running = SDL_TRUE;
    while (is_running)
    {
        is_running = scene_update(looper->scene_manager);
    }
}

void looper_fini(Looper *looper)
{
    scene_manager_fini(looper->scene_manager);
}