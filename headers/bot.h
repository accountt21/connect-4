#ifndef BOT_H
#define BOT_H

#include "board.h"

typedef enum {
    BOT_DIFFICULTY_EASY = 0,
    BOT_DIFFICULTY_MEDIUM,
    BOT_DIFFICULTY_HARD
} BotDifficulty;

typedef struct {
    const Board *board;
    char bot_token;
    char opponent_token;
    int last_move_row;
    int last_move_column; // zero-based column index, -1 when unknown
} BotRequest;

typedef int (*BotDecisionFn)(const BotRequest *request);

typedef struct {
    const char *name;
    BotDifficulty difficulty;
    BotDecisionFn decide;
} BotStrategy;

#endif // BOT_H
