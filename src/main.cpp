#include <stdio.h>

#include "looper/looper.hpp"

int main(void)
{
    Looper looper;
    init_looper(&looper, SCENE_TITLE);
    loop(&looper);
    finalize_looper(&looper);

    return 0;
}