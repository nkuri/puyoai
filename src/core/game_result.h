#ifndef CORE_GAME_RESULT_H_
#define CORE_GAME_RESULT_H_

enum class GameResult {
    PLAYING,
    DRAW,
    P1_WIN,
    P2_WIN,
    P1_WIN_WITH_CONNECTION_ERROR,
    P2_WIN_WITH_CONNECTION_ERROR,
    GAME_HAS_STOPPED,
};

GameResult toOppositeResult(GameResult);

#endif
