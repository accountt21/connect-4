#ifndef BOARD_H
#define BOARD_H

#define BOARD_ROWS 6
#define BOARD_COLS 7

typedef struct {
    char cells[BOARD_ROWS][BOARD_COLS];
} Board;

void board_init(Board *board);
void board_display(const Board *board);
int board_place_coin(Board *board, int column, char player);
char board_check_success(const Board *board, int row, int column, char player, int n);
int board_is_column_available(const Board *board, int column);

#endif
