#include "../headers/bot_hard.h"
#include "../headers/bot_hard_bb.h"

static int bot_hard_decide(const BotRequest *request) {
    return bot_hard_decide_bb(request);
}

BotStrategy bot_hard_strategy(void) {
    BotStrategy strategy = {
        .name = "Hard Bot",
        .difficulty = BOT_DIFFICULTY_HARD,
        .decide = bot_hard_decide
    };
    return strategy;
}
