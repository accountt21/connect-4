#ifndef BOT_UTILS_H
#define BOT_UTILS_H

#include "bot.h"

int bot_utils_simulate_move(const Board *board, int column, char player, Board *snapshot);
int bot_utils_random_valid_column(const Board *board);

#endif // BOT_UTILS_H
