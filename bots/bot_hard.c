#include "../headers/bot_hard.h"
#include <stdint.h>
#include "../headers/bot_utils.h"

// Bitboard

typedef struct {
	uint64_t position;
	uint64_t mask;
	int moves_played;
} BitboardState;

enum {
	BB_WIDTH = 7,
	BB_HEIGHT = 6,
	BB_MIN_SCORE = -10000,
	BB_MAX_SCORE = 10000
};

static inline uint64_t bb_bottom_mask(int col) {
	return (uint64_t)1 << (col * (BB_HEIGHT + 1));
}

static inline uint64_t bb_column_mask(int col) {
	return ((uint64_t)0x7F) << (col * (BB_HEIGHT + 1));
}

// Bitwise operations

static inline int bb_can_play(uint64_t mask, int col) {
	uint64_t top_bit = bb_bottom_mask(col) << BB_HEIGHT;
	return (mask & top_bit) == 0;
}

static inline void bb_make_move(uint64_t *position, uint64_t *mask, int col) {
	uint64_t next = (*mask + bb_bottom_mask(col)) & bb_column_mask(col);
	*position ^= next;
	*mask |= next;
}

static inline int bb_check_win(uint64_t pos) {
	uint64_t m;

	// vertical scan
	m = pos & (pos >> 1);
	if (m & (m >> 2)) {
		return 1;
	}

	// horizontal scan
	m = pos & (pos >> (BB_HEIGHT + 1));
	if (m & (m >> (2 * (BB_HEIGHT + 1)))) {
		return 1;
	}

	// diagonal / (up-right) scan
	m = pos & (pos >> BB_HEIGHT);
	if (m & (m >> (2 * BB_HEIGHT))) {
		return 1;
	}

	// Diagonal \ (down-right)
	m = pos & (pos >> (BB_HEIGHT + 2));
	if (m & (m >> (2 * (BB_HEIGHT + 2)))) {
		return 1;
	}

	return 0;
}

// Evaluation

static int bb_heuristic(uint64_t my_pos, uint64_t opp_pos) {
	
    // Center column is column 3 (0-based index 3)

	int center_col = 3;
	uint64_t col_mask = bb_column_mask(center_col);
	uint64_t my_center = my_pos & col_mask;
	uint64_t opp_center = opp_pos & col_mask;

	int my_count = 0;
	int opp_count = 0;

	while (my_center) {
		my_count += (int)(my_center & 1ULL);
		my_center >>= 1;
	}
	while (opp_center) {
		opp_count += (int)(opp_center & 1ULL);
		opp_center >>= 1;
	}

	return (my_count - opp_count) * 10;
}

// Move ordering: source why this is good: https://stackoverflow.com/questions/7033165/what-is-a-good-move-ordering-for-connect-four
static const int bb_move_order[BB_WIDTH] = {3, 2, 4, 1, 5, 0, 6};

// Negamax + Alpha-Beta solver. Inspiration: https://github.com/majikoushik/MiniMaxAlphaBeta

static int bb_is_draw(uint64_t mask) {
	const uint64_t full_mask =
		((uint64_t)1 << (BB_WIDTH * (BB_HEIGHT + 1))) - 1ULL;
	return (mask & full_mask) == full_mask;
}

static int bb_negamax(uint64_t position, uint64_t mask,
					   uint64_t opp_position,
					   int alpha, int beta, int depth) {

	if (bb_check_win(opp_position))
		return -(BB_MAX_SCORE - depth);

	if (bb_is_draw(mask) || depth == 0) 
		return bb_heuristic(position, opp_position);

	int best = BB_MIN_SCORE - 1;

	for (int i = 0; i < BB_WIDTH; i++) {
		int col = bb_move_order[i];
		if (!bb_can_play(mask, col))
			continue;

		uint64_t next_pos = position;
		uint64_t next_mask = mask;
		bb_make_move(&next_pos, &next_mask, col);

		int score = -bb_negamax(opp_position, next_mask, next_pos,
								-beta, -alpha, depth - 1);

		if (score > best)
			best = score;
		if (score > alpha)
			alpha = score;
		if (alpha >= beta)
			break;
	}

	return best;
}

// Integration
static void bb_from_board(const Board *board,
						  char bot_token,
						  char opp_token,
						  BitboardState *bot_state,
						  BitboardState *opp_state) {
	bot_state->position = 0ULL;
	bot_state->mask = 0ULL;
	bot_state->moves_played = 0;

	opp_state->position = 0ULL;
	opp_state->mask = 0ULL;
	opp_state->moves_played = 0;
	for (int col = 0; col < BB_WIDTH; col++) {
		uint64_t bit = bb_bottom_mask(col);
		for (int row = 0; row < BB_HEIGHT; row++) {
			char cell = board->cells[BB_HEIGHT - 1 - row][col];
			if (cell != '*') {
				bot_state->mask |= bit;
				opp_state->mask |= bit;
				bot_state->moves_played++;

				if (cell == bot_token) {
					bot_state->position |= bit;
				} else if (cell == opp_token) {
					opp_state->position |= bit;
				}
			}
			bit <<= 1;
		}
	}
}

static int bot_hard_decide(const BotRequest *request) {
	if (!request || !request->board) {
		return -1;
	}

	BitboardState bot_state;
	BitboardState opp_state;
	bb_from_board(request->board, request->bot_token, request->opponent_token,
				  &bot_state, &opp_state);

	if (bot_state.mask == 0ULL) {
		return 4;
	}

	int best_col = -1;
	int best_score = BB_MIN_SCORE - 1;

	const int MAX_DEPTH = 7;

	for (int depth = 1; depth <= MAX_DEPTH; depth++) {
		int current_best_col = best_col;
		int current_best_score = best_score;

		int alpha = BB_MIN_SCORE;
		int beta = BB_MAX_SCORE;

		for (int i = 0; i < BB_WIDTH; i++) {
			int col = bb_move_order[i];
			if (!bb_can_play(bot_state.mask, col)) {
				continue;
			}

			uint64_t next_pos = bot_state.position;
			uint64_t next_mask = bot_state.mask;
			bb_make_move(&next_pos, &next_mask, col);

			int score = -bb_negamax(opp_state.position, next_mask, next_pos,
									-beta, -alpha, depth - 1);

			if (score > current_best_score || current_best_col == -1) {
				current_best_score = score;
				current_best_col = col;
			}
			if (score > alpha) {
				alpha = score;
			}
			if (alpha >= beta) {
				break;
			}
		}

		if (current_best_col != -1) {
			best_col = current_best_col;
			best_score = current_best_score;
		}
	}

	if (best_col == -1) {
		int fallback = bot_utils_random_valid_column(request->board);
		return fallback;
	}

	return best_col + 1;
}

BotStrategy bot_hard_strategy(void) {
	BotStrategy strategy = {
		.name = "Hard Bot",
		.difficulty = BOT_DIFFICULTY_HARD,
		.decide = bot_hard_decide
	};
	return strategy;
}