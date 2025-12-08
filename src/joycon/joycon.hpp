#pragma once
#include <joyconlib.h>

#include "common/player_input.h"

struct Joycon
{
    joyconlib_t joycon;
};

bool joycon_init(Joycon *joycon);

PlayerInput get_joycon(Joycon *joycon);

void joycon_fini(Joycon *joycon);