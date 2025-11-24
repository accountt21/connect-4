#include "../headers/bot_hard_mt.h"

#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <pthread.h>

#include "../headers/bot_utils.h"
#include "../headers/board.h"

static const int WIN_SCORE = 1000000;
static const int THREE_SCORE = 100;
static const int TWO_SCORE = 10;

static int detect_win(const Board *board, char player) {
    if (!board) return 0;
    for (int r = 0; r < BOARD_ROWS; r++)
        for (int c = 0; c < BOARD_COLS; c++)
            if (board->cells[r][c] == player)
                if (board_check_success(board, r, c + 1, player, 4) == player)
                    return 1;
    return 0;
}

static int valid_locations(const Board *board, int out_cols[BOARD_COLS]) {
    if (!board) return 0;
    int count = 0;
    for (int col = 1; col <= BOARD_COLS; col++) {
        if (board_is_column_available(board, col)) {
            out_cols[count++] = col;
        }
    }
    return count;
}

static int evaluate_adjacents(char a, char b, char c, char d, char player) {
    int player_pieces = 0;
    int empty_spaces = 0;
    char vals[4] = {a,b,c,d};
    for (int i = 0; i < 4; i++) {
        if (vals[i] == player) player_pieces++;
        else if (vals[i] == '*') empty_spaces++;
    }

    if (player_pieces == 4) return WIN_SCORE;
    if (player_pieces == 3 && empty_spaces == 1) return THREE_SCORE;
    if (player_pieces == 2 && empty_spaces == 2) return TWO_SCORE;

    return 0;
}

static int score_board(const Board *board, char player) {
    if (!board) return 0;
    int score = 0;

    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 2; c <= 4; c++) {
            if (board->cells[r][c] == player) {
                if (c == 3) score += 3;
                else score += 2;
            }
        }
    }
    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c <= BOARD_COLS - 4; c++) {
            score += evaluate_adjacents(
                board->cells[r][c], board->cells[r][c+1],
                board->cells[r][c+2], board->cells[r][c+3], player);
        }
    }

    for (int c = 0; c < BOARD_COLS; c++) {
        for (int r = 0; r <= BOARD_ROWS - 4; r++) {
            score += evaluate_adjacents(board->cells[r][c], board->cells[r+1][c],
                board->cells[r+2][c], board->cells[r+3][c], player);
        }
    }

    for (int r = 0; r <= BOARD_ROWS - 4; r++) {
        for (int c = 0; c <= BOARD_COLS - 4; c++) {
            score += evaluate_adjacents(board->cells[r][c], board->cells[r+1][c+1],
                board->cells[r+2][c+2], board->cells[r+3][c+3], player);
        }
    }

    for (int r = 3; r < BOARD_ROWS; r++) {
        for (int c = 0; c <= BOARD_COLS - 4; c++) {
            score += evaluate_adjacents(board->cells[r][c], board->cells[r-1][c+1],
                board->cells[r-2][c+2], board->cells[r-3][c+3], player);
        }
    }

    return score;
}

static int minimax(const Board *board, int depth, int alpha, int beta, int maximizingPlayer,
                   char ai_token, char human_token, int *out_col) {
    int valid[BOARD_COLS];
    int n_valid = valid_locations(board, valid);
    // reorder valid moves by proximity to center
    int center = BOARD_COLS / 2;
    for (int i = 0; i < n_valid - 1; i++) {
        for (int j = i + 1; j < n_valid; j++) {
            if (abs(valid[j] - center) < abs(valid[i] - center)) {
                int tmp = valid[i];
                valid[i] = valid[j];
                valid[j] = tmp;
            }
        }
    }
    int is_terminal = detect_win(board, ai_token) || detect_win(board, human_token) || (n_valid == 0);

    if (depth == 0 || is_terminal) {
        if (is_terminal)
            if (detect_win(board, human_token))
                return -WIN_SCORE;
            else if (detect_win(board, ai_token))
                return WIN_SCORE;
            else
                return 0;
        else
            return score_board(board, ai_token);
    }

    if (maximizingPlayer) {
        int value = INT_MIN;
        int chosen = -1;
        if (n_valid > 0) chosen = valid[0];

        for (int i = 0; i < n_valid; i++) {
            int col = valid[i];
            Board child;
            int row = bot_utils_simulate_move(board, col, ai_token, &child);
            if (row == -1) continue;
            int new_score = minimax(&child, depth - 1, alpha, beta, 0, ai_token, human_token, NULL);
            if (new_score > value) {
                value = new_score;
                chosen = col;
            }
            if (value > alpha) alpha = value;
            if (beta <= alpha) break;
        }
        if (out_col) *out_col = chosen;
        return value;
    } else {
        int value = INT_MAX;
        int chosen = -1;
        if (n_valid > 0) chosen = valid[0];

        for (int i = 0; i < n_valid; i++) {
            int col = valid[i];
            Board child;
            int row = bot_utils_simulate_move(board, col, human_token, &child);
            if (row == -1) continue;
            int new_score = minimax(&child, depth - 1, alpha, beta, 1, ai_token, human_token, NULL);
            if (new_score < value) {
                value = new_score;
                chosen = col;
            }
            if (value < beta) beta = value;
            if (beta <= alpha) break;
        }
        if (out_col) *out_col = chosen;
        return value;
    }
}

typedef struct {
    Board child;
    int depth_minus_one;
    char ai_token;
    char human_token;
    int root_col;
    int score;
} RootTask;

static void *root_worker(void *arg) {
    RootTask *t = (RootTask *)arg;
    int s = minimax(&t->child, t->depth_minus_one, INT_MIN, INT_MAX, 0, t->ai_token, t->human_token, NULL);
    t->score = s;
    return NULL;
}

int bot_hard_decide_parallel(const BotRequest *request) {
    if (!request || !request->board) return -1;

    static int seeded = 0;
    if (!seeded) { srand((unsigned int)time(NULL)); seeded = 1; }

    Board snapshot = *request->board;

    int depth = 5;
    int chosen = -1;

    int valid[BOARD_COLS];
    int n_valid = 0;
    for (int col = 1; col <= BOARD_COLS; col++) {
        if (board_is_column_available(&snapshot, col)) {
            valid[n_valid++] = col;
        }
    }

    if (n_valid <= 1) {
        if (n_valid == 1) {
            chosen = valid[0];
        } else {
            minimax(&snapshot, depth, INT_MIN, INT_MAX, 1, request->bot_token, request->opponent_token, &chosen);
        }
        return chosen;
    }

    // immediate win/block check
    for (int i = 0; i < n_valid; i++) {
        int col = valid[i];
        Board child;
        int row = bot_utils_simulate_move(&snapshot, col, request->bot_token, &child);
        if (row != -1 && detect_win(&child, request->bot_token)) {
            return col; // instant win
        }
    }
    for (int i = 0; i < n_valid; i++) {
        int col = valid[i];
        Board child;
        int row = bot_utils_simulate_move(&snapshot, col, request->opponent_token, &child);
        if (row != -1 && detect_win(&child, request->opponent_token)) {
            return col; // block opponent
        }
    }


    RootTask tasks[BOARD_COLS];
    pthread_t threads[BOARD_COLS];
    int task_count = 0;

    for (int i = 0; i < n_valid; i++) {
        int col = valid[i];
        Board child;
        int row = bot_utils_simulate_move(&snapshot, col, request->bot_token, &child);
        if (row == -1) continue;
        tasks[task_count].child = child;
        tasks[task_count].depth_minus_one = depth - 1;
        tasks[task_count].ai_token = request->bot_token;
        tasks[task_count].human_token = request->opponent_token;
        tasks[task_count].root_col = col;
        tasks[task_count].score = INT_MIN;
        pthread_create(&threads[task_count], NULL, root_worker, &tasks[task_count]);
        task_count++;
    }

    int best_score = INT_MIN;
    int best_col = -1;

    // joining threads and collecting results
    for (int i = 0; i < task_count; i++) {
        pthread_join(threads[i], NULL);
        if (tasks[i].score > best_score) {
            best_score = tasks[i].score;
            best_col = tasks[i].root_col;
        }
    }

    if (best_col == -1) {
        minimax(&snapshot, depth, INT_MIN, INT_MAX, 1, request->bot_token, request->opponent_token, &best_col);
    }
    return best_col;
}
