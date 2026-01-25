#pragma once

#include "common/GamePhase.h"
#include "common/GameScore.h"
#include "common/ability.h"
#include "common/ball.h"
#include "common/player.h"

struct NetworkDataSet
{
    GamePhase game_phase;
    Ball ball;
    Player players[2];
    GameScore game_score;
    AbilityState ability_states[2];  // プレイヤーごとの能力状態
    int match_winner;                // 試合勝者（-1: 未確定、0: P1、1: P2）
};