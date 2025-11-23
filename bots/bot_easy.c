#include "../headers/bot_easy.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../headers/bot_utils.h"

static int bot_easy_decide(const BotRequest *request) {
    if (!request || !request->board) {
        return -1;
    }

    int column = bot_utils_random_valid_column(request->board);
    return column;
}

BotStrategy bot_easy_strategy(void) {
    BotStrategy strategy = {
        .name = "Easy Bot",
        .difficulty = BOT_DIFFICULTY_EASY,
        .decide = bot_easy_decide
    };
    return strategy;
}
