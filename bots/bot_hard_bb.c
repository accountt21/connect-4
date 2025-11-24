#include "../headers/bot_hard_bb.h"

#include <stdint.h>
#include <limits.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>

#include "../headers/board.h"

/*

We wrote a complex-er version of minimax with alpha-beta pruning using bitboards for Connect Four.

This allowed up to reasonably run at depth 11 (with the help of multithreading.)

Depth 12 works as well but it's quite slow (1 second+ per move on a decent machine.)

*/

static inline int idx_from_rowcol(int row /*0 bottom..5 top*/, int col) {
    return col * 7 + row;
}

static inline uint64_t bb_set(uint64_t bb, int row, int col) {
    return bb | (1ULL << idx_from_rowcol(row, col));
}

static inline int bb_has_won(uint64_t bb) {
    uint64_t m;
    m = bb & (bb >> 1);  if (m & (m >> 2))  return 1; // vertical
    m = bb & (bb >> 7);  if (m & (m >> 14)) return 1; // horizontal
    m = bb & (bb >> 6);  if (m & (m >> 12)) return 1; // diag (/)
    m = bb & (bb >> 8);  if (m & (m >> 16)) return 1; // diag (\)
    return 0;
}

static void bb_from_board(const Board *board, char ai_token, uint64_t *ai_bb, uint64_t *op_bb) {
    uint64_t a = 0, b = 0;
    for (int r_top = 0; r_top < BOARD_ROWS; r_top++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            char cell = board->cells[r_top][c];
            if (cell == '*') continue;
            int r = (BOARD_ROWS - 1) - r_top; // convert to bottom-based row
            if (cell == ai_token) a = bb_set(a, r, c);
            else b = bb_set(b, r, c);
        }
    }
    *ai_bb = a; *op_bb = b;
}

static inline uint64_t bb_mask(uint64_t p, uint64_t o) { return p | o; }

static inline int col_is_full(uint64_t mask, int col) {
    return (mask >> idx_from_rowcol(5, col)) & 1ULL; 
}

static inline int next_free_row(uint64_t mask, int col) {
    int base = col * 7;
    for (int r = 0; r < BOARD_ROWS; r++) {
        if (((mask >> (base + r)) & 1ULL) == 0ULL) return r;
    }
    return -1;
}

static inline void bb_play(uint64_t p, uint64_t o, int col, uint64_t *p_out, uint64_t *o_out) {
    uint64_t mask = bb_mask(p, o);
    int r = next_free_row(mask, col);
    uint64_t bit = (r >= 0) ? (1ULL << idx_from_rowcol(r, col)) : 0ULL;
    *p_out = p | bit; 
    *o_out = o;
}

static inline int count_bits(uint64_t x) {

    int c = 0; while (x) { x &= (x - 1); c++; } return c;
}

static int evaluate_heuristic(uint64_t me, uint64_t opp) {


    uint64_t center_mask = 0ULL;
    for (int r = 0; r < BOARD_ROWS; r++) center_mask |= (1ULL << idx_from_rowcol(r, 3));
    int center_score = 3 * (count_bits(me & center_mask) - count_bits(opp & center_mask));
    int piece_score = count_bits(me) - count_bits(opp);
    return 10 * center_score + piece_score;
}

static int negamax(uint64_t me, uint64_t opp, int depth, int alpha, int beta) {
    if (bb_has_won(opp)) return -1000000; // previous move by opp just won
    if (depth == 0) return evaluate_heuristic(me, opp);

    uint64_t mask = bb_mask(me, opp);
    // move ordering optimization
    static const int order[BOARD_COLS] = {3, 2, 4, 1, 5, 0, 6};
    int best = INT_MIN;

    for (int i = 0; i < BOARD_COLS; i++) {
        int col = order[i];
        if (col_is_full(mask, col)) continue;
        
        uint64_t me2, opp2;
        bb_play(me, opp, col, &me2, &opp2);
        if (bb_has_won(me2)) return 1000000 - (7 - depth); // immediate win; prefer quicker win

        int score = -negamax(opp2, me2, depth - 1, -beta, -alpha);
        if (score > best) best = score;
        if (best > alpha) alpha = best;
        if (alpha >= beta) break; 
    }
    return (best == INT_MIN) ? 0 : best;
}

typedef struct {
    uint64_t me2;
    uint64_t opp2;
    int depth;
    int col; // NOTE: 0-based
    int score;
} RootTask;

static void* root_worker(void* arg) {
    RootTask* t = (RootTask*)arg;
    int alpha = INT_MIN / 2;
    int beta  = INT_MAX / 2;
    t->score = -negamax(t->opp2, t->me2, t->depth - 1, -beta, -alpha);
    return NULL;
}

static int find_best_move_bb(uint64_t ai_bb, uint64_t op_bb, int depth) {
    uint64_t mask = bb_mask(ai_bb, op_bb);
    static const int order[BOARD_COLS] = {3, 2, 4, 1, 5, 0, 6};
    int best_col = -1;
    int best_score = INT_MIN;

    for (int i = 0; i < BOARD_COLS; i++) {
        int col = order[i];
        if (col_is_full(mask, col)) continue;
        uint64_t me2, opp2;
        bb_play(ai_bb, op_bb, col, &me2, &opp2);
        if (bb_has_won(me2)) return col + 1;
    }

    pthread_t threads[BOARD_COLS];
    RootTask tasks[BOARD_COLS];
    int tcount = 0;
    for (int i = 0; i < BOARD_COLS; i++) {
        int col = order[i];
        if (col_is_full(mask, col)) continue;
        uint64_t me2, opp2;
        bb_play(ai_bb, op_bb, col, &me2, &opp2);
        tasks[tcount].me2 = me2;
        tasks[tcount].opp2 = opp2;
        tasks[tcount].depth = depth;
        tasks[tcount].col = col;
        tasks[tcount].score = INT_MIN;
        pthread_create(&threads[tcount], NULL, root_worker, &tasks[tcount]);
        tcount++;
    }
    for (int i = 0; i < tcount; i++) pthread_join(threads[i], NULL);

    for (int i = 0; i < tcount; i++) {
        int score = tasks[i].score;
        int col = tasks[i].col;
        if (score > best_score) { best_score = score; best_col = col; }
    }
    return (best_col >= 0) ? (best_col + 1) : -1;
}

int bot_hard_decide_bb(const BotRequest *request) {
    if (!request || !request->board) return -1;

    int depth = 11;
    uint64_t ai_bb = 0, op_bb = 0;
    bb_from_board(request->board, request->bot_token, &ai_bb, &op_bb);
    int col = find_best_move_bb(ai_bb, op_bb, depth);
    if (col < 1 || col > BOARD_COLS) {
        // fall back if neccesary
        for (int c = 1; c <= BOARD_COLS; c++) if (board_is_column_available(request->board, c)) return c;
        return -1;
    }
    return col;
}
