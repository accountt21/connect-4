#include "../headers/bot_hard.h"

#include "../headers/bot_utils.h"

static int find_immediate_play(const BotRequest *request, char token) {
    if (!request || !request->board) {
        return -1;
    }

    Board snapshot;
    for (int column = 1; column <= BOARD_COLS; column++) {
        if (!board_is_column_available(request->board, column)) {
            continue;
        }

        int row = bot_utils_simulate_move(request->board, column, token, &snapshot);
        if (row != -1 && board_check_success(&snapshot, row, column, token, 4) == token) {
            return column;
        }
    }

    return -1;
}

static int prefer_center(const Board *board) {
    static const int ordering[BOARD_COLS] = {4, 3, 5, 2, 6, 1, 7};

    for (int i = 0; i < BOARD_COLS; i++) {
        int column = ordering[i];
        if (board_is_column_available(board, column)) {
            return column;
        }
    }

    return -1;
}

static int bot_hard_decide(const BotRequest *request) {
    if (!request || !request->board) {
        return -1;
    }

    int column = find_immediate_play(request, request->bot_token);
    if (column != -1) {
        return column;
    }

    column = find_immediate_play(request, request->opponent_token);
    if (column != -1) {
        return column;
    }

    column = prefer_center(request->board);
    if (column != -1) {
        return column;
    }

    return bot_utils_random_valid_column(request->board);
}

BotStrategy bot_hard_strategy(void) {
    BotStrategy strategy = {
        .name = "Hard Bot",
        .difficulty = BOT_DIFFICULTY_HARD,
        .decide = bot_hard_decide
    };
    return strategy;
}
