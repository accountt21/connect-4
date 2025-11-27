#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "headers/board.h"
#include "headers/bot_manager.h"
#include "headers/bot_utils.h"

typedef enum {
    GAME_MODE_HUMAN_VS_HUMAN = 0,
    GAME_MODE_HUMAN_VS_BOT
} GameMode;

typedef struct {
    GameMode mode;
    char player_a_token;
    char player_b_token;
    BotDifficulty difficulty;
    int human_starts_first;
} GameConfig;

typedef struct {
    int row;
    int column;
} Move;

static void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
    }
}

static GameMode prompt_game_mode(void) {
    while (1) {
        char choice;
        printf("Press H to play against a human and B to play against a bot: ");
        if (scanf(" %c", &choice) != 1) {
            clear_input_buffer();
            puts("Invalid input. Please enter H or B.");
            continue;
        }
        clear_input_buffer();
        choice = (char)toupper((unsigned char)choice);
        if (choice == 'H') {
            return GAME_MODE_HUMAN_VS_HUMAN;
        }
        if (choice == 'B') {
            return GAME_MODE_HUMAN_VS_BOT;
        }
        puts("Please enter H for human vs human or B for human vs bot.");
    }
}

static char prompt_player_token(const char *label) {
    while (1) {
        char token;
        printf("Enter %s token (single character): ", label);
        if (scanf(" %c", &token) == 1) {
            clear_input_buffer();
            return token;
        }
        clear_input_buffer();
        puts("Invalid token. Please enter a single character.");
    }
}

static BotDifficulty prompt_bot_difficulty(void) {
    while (1) {
        char choice;
        printf("Enter the bot difficulty (E for easy, M for medium, H for hard): ");
        if (scanf(" %c", &choice) != 1) {
            clear_input_buffer();
            puts("Invalid input. Please enter E, M, or H.");
            continue;
        }
        clear_input_buffer();
        choice = (char)toupper((unsigned char)choice);
        switch (choice) {
            case 'H':
                return BOT_DIFFICULTY_HARD;
            case 'M':
                return BOT_DIFFICULTY_MEDIUM;
            case 'E':
                return BOT_DIFFICULTY_EASY;
            default:
                puts("Please enter E, M, or H.");
        }
    }
}

static int prompt_human_starts_first(void) {
    while (1) {
        char choice;
        printf("Do you want to go first? (y/n): ");
        if (scanf(" %c", &choice) != 1) {
            clear_input_buffer();
            puts("Invalid input. Please enter Y or N.");
            continue;
        }
        clear_input_buffer();
        choice = (char)toupper((unsigned char)choice);
        if (choice == 'Y') return 1;
        if (choice == 'N') return 0;
        puts("Please enter Y to go first or N to let the bot start.");
    }
}

static GameConfig collect_game_config(void) {
    GameConfig config;
    config.mode = prompt_game_mode();
    config.player_a_token = prompt_player_token("Player A");
    config.human_starts_first = 1;

    if (config.mode == GAME_MODE_HUMAN_VS_HUMAN) {
        config.player_b_token = prompt_player_token("Player B");
        config.difficulty = BOT_DIFFICULTY_EASY;
    } else {
        config.player_b_token = 'B';
        config.difficulty = prompt_bot_difficulty();
        config.human_starts_first = prompt_human_starts_first();
    }

    return config;
}

static int prompt_column_input(const Board *board, const char *label) {
    while (1) {
        int column;
        printf("%s, choose a column (1-%d): ", label, BOARD_COLS);
        if (scanf(" %d", &column) != 1) {
            clear_input_buffer();
            puts("Invalid number. Please enter a value between 1 and 7.");
            continue;
        }
        clear_input_buffer();

        if (column < 1 || column > BOARD_COLS) {
            puts("Column out of range. Try again.");
            continue;
        }
        if (!board_is_column_available(board, column)) {
            puts("That column is full. Try another one.");
            continue;
        }
        return column;
    }
}

static int fallback_column_if_needed(const Board *board) {
    for (int column = 1; column <= BOARD_COLS; column++) {
        if (board_is_column_available(board, column)) {
            return column;
        }
    }
    return -1;
}

static int acquire_bot_column(const Board *board, const BotManager *manager, BotDifficulty difficulty,
                              const BotRequest *request) {
    int column = bot_manager_choose_column(manager, difficulty, request);
    if (column < 1 || column > BOARD_COLS || !board_is_column_available(board, column)) {
        int fallback = bot_utils_random_valid_column(board);
        if (fallback == -1) {
            fallback = fallback_column_if_needed(board);
        }
        column = fallback;
    }
    return column;
}

// Speculation utilities moved to mt_speculation.c

int main(void) {
    srand((unsigned int)time(NULL));

    printf("Welcome to Connect Four!\n\n");

    GameConfig config = collect_game_config();

    Board board;
    board_init(&board);
    board_display(&board);

    BotManager manager;
    bot_manager_init(&manager);

    Move last_moves[2] = { { -1, -1 }, { -1, -1 } };
    char current_player_token = config.player_a_token;
    if (config.mode == GAME_MODE_HUMAN_VS_BOT && !config.human_starts_first) {
        current_player_token = config.player_b_token;
    }
    char winner = '0';
    int move_count = 0;
    const int max_moves = BOARD_ROWS * BOARD_COLS;

    while (winner == '0' && move_count < max_moves) {
        int column = -1;
        int player_index = (current_player_token == config.player_a_token) ? 0 : 1;

        if (config.mode == GAME_MODE_HUMAN_VS_BOT && player_index == 1) {
            BotRequest request = {
                .board = &board,
                .bot_token = config.player_b_token,
                .opponent_token = config.player_a_token,
                .last_move_row = last_moves[0].row,
                .last_move_column = last_moves[0].column
            };
            column = acquire_bot_column(&board, &manager, config.difficulty, &request);
            if (column == -1) {
                puts("No valid moves available for the bot.");
                break;
            }

            printf("Bot (%s) chooses column %d\n", bot_manager_difficulty_label(config.difficulty), column);
        } else {
            const char *label = (player_index == 0) ? "Player A" : "Player B";
            column = prompt_column_input(&board, label);
        }

        int placed_row = board_place_coin(&board, column, current_player_token);
        if (placed_row == -1) {
            puts("Unable to place coin. Please choose again.");
            continue;
        }

        last_moves[player_index].row = placed_row;
        last_moves[player_index].column = column - 1;

        winner = board_check_success(&board, placed_row, column, current_player_token, 4);
        if (winner != '0') {
            break;
        }

        move_count++;
        current_player_token = (current_player_token == config.player_a_token) ? config.player_b_token : config.player_a_token;
    }

    if (winner == '0') {
        puts("\nNo player wins. It's a draw!\n");
    } else {
        printf("\nPlayer %c wins!\n\n", winner);
    }

    return 0;
}
