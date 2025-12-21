#include "joycon.hpp"

// #include <joyconlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "common/player_input.h"
#include "util/log.hpp"
bool joycon_init(Joycon *joycon)
{
    joycon_err err = joycon_open(&joycon->joycon, JOYCON_R);
    if (err != JOYCON_ERR_NONE)
    {
        LOG_ERROR("Joy-Conの初期化に失敗しました: " << err);
        return false;
    }

    LOG_SUCCESS("Joy-Conの初期化に成功しました");

    return true;
}

PlayerInput get_joycon(Joycon *joycon, int player_id)
{
    static const float TRIGGER_THRESHOLD = 0.5f;

    PlayerInput player_input;
    memset(&player_input, 0, sizeof(PlayerInput));

    player_input.player_id = player_id;

    joycon_get_state(&joycon->joycon);

    if (joycon->joycon.stick.x < -TRIGGER_THRESHOLD)
    {
        player_input.left = true;
    }

    if (joycon->joycon.stick.x > TRIGGER_THRESHOLD)
    {
        player_input.right = true;
    }

    if (joycon->joycon.stick.y < -TRIGGER_THRESHOLD)
    {
        player_input.front = true;
    }

    if (joycon->joycon.stick.y > TRIGGER_THRESHOLD)
    {
        player_input.back = true;
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