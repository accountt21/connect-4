#include "../headers/bot_hard_bb_dp.h"

#include <stdint.h>
#include <limits.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

#include "../headers/board.h"

#define DP_MAX_DEPTH 12
#define DP_TABLE_SIZE (1u << 20)
#define DP_PROBE_LIMIT 8

typedef enum {
    DP_FLAG_EMPTY = 0,
    DP_FLAG_EXACT,
    DP_FLAG_LOWER,
    DP_FLAG_UPPER
} DPFlag;

typedef struct {
    uint64_t me;
    uint64_t opp;
    int depth;
    int value;
    DPFlag flag;
    int used;
} DPEntry;

static DPEntry dp[DP_TABLE_SIZE];
static size_t dp_entries_used = 0;
static pthread_mutex_t dp_mutex = PTHREAD_MUTEX_INITIALIZER;
static const char *DP_EXPORT_PATH = "dp_export.csv"; // actually we discovered 60ish megabytes is enough. no need to store in the repo.

static inline uint64_t dp_hash(uint64_t me, uint64_t opp) {
    const uint64_t mix1 = 0x9e3779b185ebca87ULL;
    const uint64_t mix2 = 0xc2b2ae3d27d4eb4FULL;
    uint64_t h = me * mix1;
    h ^= opp * mix2;
    h ^= (h >> 33);
    return h;
}

static int dp_probe(uint64_t me, uint64_t opp, int depth, int alpha, int beta, int *value) {
    if (depth > DP_MAX_DEPTH) return 0;
    uint64_t key = dp_hash(me, opp);
    uint32_t idx = (uint32_t)(key & (DP_TABLE_SIZE - 1));
    pthread_mutex_lock(&dp_mutex);
    for (int i = 0; i < DP_PROBE_LIMIT; i++) {
        DPEntry *entry = &dp[(idx + i) & (DP_TABLE_SIZE - 1)];
        if (!entry->used) break;
        if (entry->me != me || entry->opp != opp) continue;
        if (entry->depth < depth) continue;
        int stored = entry->value;
        if (entry->flag == DP_FLAG_EXACT ||
            (entry->flag == DP_FLAG_LOWER && stored >= beta) ||
            (entry->flag == DP_FLAG_UPPER && stored <= alpha)) {
            *value = stored;
            pthread_mutex_unlock(&dp_mutex);
            return 1;
        }
    }
    pthread_mutex_unlock(&dp_mutex);
    return 0;
}

static void dp_store(uint64_t me, uint64_t opp, int depth, int value, DPFlag flag) {
    if (depth > DP_MAX_DEPTH) return;
    uint64_t key = dp_hash(me, opp);
    uint32_t idx = (uint32_t)(key & (DP_TABLE_SIZE - 1));
    pthread_mutex_lock(&dp_mutex);
    for (int i = 0; i < DP_PROBE_LIMIT; i++) {
        DPEntry *entry = &dp[(idx + i) & (DP_TABLE_SIZE - 1)];
        if (!entry->used) {
            dp_entries_used++;
            entry->used = 1;
            entry->me = me;
            entry->opp = opp;
            entry->depth = depth;
            entry->value = value;
            entry->flag = flag;
            pthread_mutex_unlock(&dp_mutex);
            return;
        }
        if (entry->me == me && entry->opp == opp) {
            if (entry->depth > depth) {
                pthread_mutex_unlock(&dp_mutex);
                return;
            }
            entry->depth = depth;
            entry->value = value;
            entry->flag = flag;
            pthread_mutex_unlock(&dp_mutex);
            return;
        }
    }
    DPEntry *entry = &dp[idx];
    if (!entry->used) dp_entries_used++;
    entry->used = 1;
    entry->me = me;
    entry->opp = opp;
    entry->depth = depth;
    entry->value = value;
    entry->flag = flag;
    pthread_mutex_unlock(&dp_mutex);
}

static void dp_export_table(const char *path) {
    pthread_mutex_lock(&dp_mutex);
    FILE *fp = fopen(path, "w");
    if (!fp) {
        pthread_mutex_unlock(&dp_mutex);
        return;
    }
    fprintf(fp, "#entries=%zu\n", dp_entries_used);
    fprintf(fp, "me_bitboard,opp_bitboard,depth,value,flag\n");
    for (size_t i = 0; i < DP_TABLE_SIZE; i++) {
        DPEntry *entry = &dp[i];
        if (!entry->used) continue;
        fprintf(fp, "%llu,%llu,%d,%d,%d\n",
                (unsigned long long)entry->me,
                (unsigned long long)entry->opp,
                entry->depth,
                entry->value,
                entry->flag);
    }
    fclose(fp);
    pthread_mutex_unlock(&dp_mutex);
}

/*

We wrote a complex-er version of minimax with alpha-beta pruning using bitboards for Connect Four (with 12-ply opening-move book ).

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

    int alpha_orig = alpha;
    int beta_orig = beta;
    int cached_value;
    if (dp_probe(me, opp, depth, alpha, beta, &cached_value)) return cached_value;

    uint64_t mask = bb_mask(me, opp);
    // move ordering optimization - this SIGNIFICATLY speeds up pruning. source: https://escholarship.mcgill.ca/concern/theses/j96022066
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
    int result = (best == INT_MIN) ? 0 : best;
    DPFlag flag = DP_FLAG_EXACT;
    if (result <= alpha_orig) flag = DP_FLAG_UPPER;
    else if (result >= beta_orig) flag = DP_FLAG_LOWER;
    dp_store(me, opp, depth, result, flag);
    return result;
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

int bot_hard_decide_bb_dp(const BotRequest *request) {
    if (!request || !request->board) return -1;

    int depth = 12;
    uint64_t ai_bb = 0, op_bb = 0;
    bb_from_board(request->board, request->bot_token, &ai_bb, &op_bb);
    int col = find_best_move_bb(ai_bb, op_bb, depth);
    if (col < 1 || col > BOARD_COLS) {
        for (int c = 1; c <= BOARD_COLS; c++) {
            if (board_is_column_available(request->board, c)) {
                col = c;
                break;
            }
        }
    }
    if (col < 1 || col > BOARD_COLS) col = -1;
    dp_export_table(DP_EXPORT_PATH);
    return col;
}
