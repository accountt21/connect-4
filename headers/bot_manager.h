#ifndef BOT_MANAGER_H
#define BOT_MANAGER_H

#include <stddef.h>

#include "bot.h"

typedef struct {
    const BotStrategy *strategies;
    size_t count;
} BotManager;

void bot_manager_init(BotManager *manager);
int bot_manager_choose_column(const BotManager *manager, BotDifficulty difficulty, const BotRequest *request);
BotDifficulty bot_manager_difficulty_from_char(char symbol);
const char *bot_manager_difficulty_label(BotDifficulty difficulty);

#endif // BOT_MANAGER_H
