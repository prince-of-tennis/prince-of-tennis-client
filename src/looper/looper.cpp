#include "looper.hpp"

#include "util/log.hpp"

void init_looper(Looper *looper, eSceneType default_scene)
{
    looper->scene_manager = make_unique<SceneManager>();
    init_scene_manager(looper->scene_manager, default_scene);
}

void loop(Looper *looper)
{
    SDL_bool is_running = SDL_TRUE;
    while (is_running)
    {
        is_running = update_scene(looper->scene_manager);
    }
}

void finalize_looper(Looper *looper)
{
    fini_scene_manager(looper->scene_manager);
}