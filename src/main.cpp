#include <stdio.h>

#include "looper/looper.hpp"

int main(void)
{
    Looper looper;
    looper_init(&looper, SCENE_TITLE);
    loop(&looper);
    looper_fini(&looper);

    return 0;
}