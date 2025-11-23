#include "../headers/bot_utils.h"

#include <stdlib.h>

int bot_utils_simulate_move(const Board *board, int column, char player, Board *snapshot) {
    if (!board || !snapshot || column < 1 || column > BOARD_COLS)
        return -1;


    *snapshot = *board;
    int col_index = column - 1;
    for (int row = BOARD_ROWS - 1; row >= 0; row--) {
        if (snapshot->cells[row][col_index] == '*') {
            snapshot->cells[row][col_index] = player;
            return row;
        }
    }

    return -1;
}

int bot_utils_random_valid_column(const Board *board) 
{
    if (!board) 
        return -1;


    int available_columns[BOARD_COLS];
    int count = 0;

    for (int column = 1; column <= BOARD_COLS; column++) 
        if (board_is_column_available(board, column)) 
            available_columns[count++] = column;
 
    if (count == 0)
        return -1;


    int index = rand() % count;
    return available_columns[index];
}
