#pragma once

#include "common/GamePhase.h"
#include "common/ball.h"
#include "common/player.h"

struct NetworkDataSet
{
    GamePhase game_phase;
    Ball ball;
    Player players[2];
};