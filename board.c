#include "headers/board.h"

#include <stdio.h>

void board_init(Board *board) {
    for (int i = 0; i < BOARD_ROWS; i++) {
        for (int j = 0; j < BOARD_COLS; j++) {
            board->cells[i][j] = '*';
        }
    }
}

void board_display(const Board *board) {
    putchar('\n');
    for (int i = 0; i < BOARD_ROWS; i++) {
        for (int j = 0; j < BOARD_COLS; j++) {
            printf("%c ", board->cells[i][j]);
        }
        putchar('\n');
    }
    printf("1 2 3 4 5 6 7\n\n");
}

int board_place_coin(Board *board, int column, char player) {
    if (column < 1 || column > BOARD_COLS) {
        return -1;
    }

    int col_index = column - 1;
    for (int row = BOARD_ROWS - 1; row >= 0; row--) {
        if (board->cells[row][col_index] == '*') {
            board->cells[row][col_index] = player;
            board_display(board);
            return row;
        }
    }

    return -1;
}

static char check_sequence(int distance, int required, char player) {
    return (distance >= required) ? player : '0';
}

char board_check_success(const Board *board, int row, int column, char player, int n) {
    if (row < 0 || column < 1 || column > BOARD_COLS) {
        return '0';
    }

    int col_index = column - 1;

    // Vertical
    int distance = 0;
    for (int i = 0; i < BOARD_ROWS; i++) {
        if (board->cells[i][col_index] == player) {
            distance++;
            if (check_sequence(distance, n, player) == player) {
                return player;
            }
        } else {
            distance = 0;
        }
    }

    // Horizontal
    distance = 0;
    for (int j = 0; j < BOARD_COLS; j++) {
        if (board->cells[row][j] == player) {
            distance++;
            if (check_sequence(distance, n, player) == player) {
                return player;
            }
        } else {
            distance = 0;
        }
    }

    // Diagonal top-left to bottom-right
    int i = row;
    int j = col_index;
    while (i > 0 && j > 0) {
        i--;
        j--;
    }

    distance = 0;
    while (i < BOARD_ROWS && j < BOARD_COLS) {
        if (board->cells[i][j] == player) {
            distance++;
            if (check_sequence(distance, n, player) == player) {
                return player;
            }
        } else {
            distance = 0;
        }
        i++;
        j++;
    }

    // Diagonal bottom-left to top-right
    i = row;
    j = col_index;
    while (i < BOARD_ROWS - 1 && j > 0) {
        i++;
        j--;
    }

    distance = 0;
    while (i >= 0 && j < BOARD_COLS) {
        if (board->cells[i][j] == player) {
            distance++;
            if (check_sequence(distance, n, player) == player) {
                return player;
            }
        } else {
            distance = 0;
        }
        i--;
        j++;
    }

    return '0';
}

int board_is_column_available(const Board *board, int column) {
    if (column < 1 || column > BOARD_COLS) {
        return 0;
    }

    return board->cells[0][column - 1] == '*';
}
