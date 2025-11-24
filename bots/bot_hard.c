#include "../headers/bot_hard.h"
#include "../headers/bot_hard_mt.h"

static int bot_hard_decide(const BotRequest *request) {
    return bot_hard_decide_parallel(request);
}

BotStrategy bot_hard_strategy(void) {
    BotStrategy strategy = {
        .name = "Hard Bot",
        .difficulty = BOT_DIFFICULTY_HARD,
        .decide = bot_hard_decide
    };
    return strategy;
}
