#include "headers/bot_manager.h"

#include <ctype.h>

#include "headers/bot_easy.h"
#include "headers/bot_medium.h"
#include "headers/bot_hard.h"

// this is a bot orchestrator that selects the appropriate bot strategy based on difficulty level.
// we decided to write this for better modularity and separation of concerns.

void bot_manager_init(BotManager *manager) {
    if (!manager) {
        return;
    }

    static int initialized = 0;
    static BotStrategy strategies[3];

    if (!initialized) {
        strategies[0] = bot_easy_strategy();
        strategies[1] = bot_medium_strategy();
        strategies[2] = bot_hard_strategy();
        initialized = 1;
    }

    manager->strategies = strategies;
    manager->count = sizeof(strategies) / sizeof(strategies[0]);
}

int bot_manager_choose_column(const BotManager *manager, BotDifficulty difficulty, const BotRequest *request) {
    if (!manager || !manager->strategies || manager->count == 0) {
        return -1;
    }

    for (size_t i = 0; i < manager->count; i++) {
        const BotStrategy *strategy = &manager->strategies[i];
        if (strategy->difficulty == difficulty && strategy->decide) {
            return strategy->decide(request);
        }
    }

    return -1;
}

BotDifficulty bot_manager_difficulty_from_char(char symbol) {
    switch (toupper((unsigned char)symbol)) {
        case 'H':
            return BOT_DIFFICULTY_HARD;
        case 'M':
            return BOT_DIFFICULTY_MEDIUM;
        case 'E':
        default:
            return BOT_DIFFICULTY_EASY;
    }
}

const char *bot_manager_difficulty_label(BotDifficulty difficulty) {
    switch (difficulty) {
        case BOT_DIFFICULTY_HARD:
            return "Hard";
        case BOT_DIFFICULTY_MEDIUM:
            return "Medium";
        case BOT_DIFFICULTY_EASY:
        default:
            return "Easy";
    }
}
