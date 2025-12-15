#pragma once

#include "common/GamePhase.h"
#include "common/ball.h"

struct NetworkDataSet
{
    GamePhase game_phase;
    Ball ball;
};