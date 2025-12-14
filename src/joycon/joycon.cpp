#include "joycon.hpp"

// #include <joyconlib.h>
#include <math.h>
#include <stdio.h>

#include "common/player_input.h"
#include "util/log.hpp"
bool joycon_init(Joycon *joycon)
{
    joycon_err err = joycon_open(&joycon->joycon, JOYCON_R);
    if (JOYCON_ERR_NONE != err)
    {
        LOG_ERROR("SDLの初期化に失敗しました: ");
        return false;
    }

    LOG_SUCCESS("Joyconの初期化に成功しました");

    return true;
}

PlayerInput get_joycon(Joycon *joycon)
{
    PlayerInput player_input = {false, false, false, false};

    joycon_get_state(&joycon->joycon);

    if (joycon->joycon.stick.x < -0.7)
    {
        player_input.left = true;
    }

    if (joycon->joycon.stick.x > 0.7)
    {
        player_input.right = true;
    }

    if (joycon->joycon.stick.y < -0.7)
    {
        player_input.back = true;
    }

    if (joycon->joycon.stick.y > 0.7)
    {
        player_input.front = true;
    }

    if (joycon->joycon.axis[0].acc_z > 20)
    {
        player_input.swing = true;
    }

    return player_input;
}

void joycon_fini(Joycon *joycon)
{
    joycon_close(&joycon->joycon);
}