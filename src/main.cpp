#include <stdio.h>

#include "looper/looper.hpp"

int main(void)
{
    Looper looper;
    if (looper_init(&looper, SCENE_TITLE))
    {
        SDL_Quit();
        return -1;
    }

    loop(&looper);
    looper_fini(&looper);

    return 0;
}