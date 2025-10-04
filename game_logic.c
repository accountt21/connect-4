




// this code logic is only for server. do not edit for assignment.

#include <stdio.h>

#define ROWS 6
#define COLS 7

void init_board(char board[ROWS][COLS]) {
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            board[i][j] = '*';
}

int place_coin(char board[ROWS][COLS], int col, char player) {
    int row = ROWS - 1;
    col--;
    while (row >= 0 && board[row][col] != '*') row--;
    if (row < 0) return -1; // column full !!
    board[row][col] = player;
    return row;
}

char check_win(char board[ROWS][COLS], int row, int col, char player) {
    int count;
    col--;
    // vertical
    count = 0;
    for (int i = 0; i < ROWS; i++)
        count = (board[i][col] == player) ? count + 1 : 0, (count == 4) ? (void)(player) : (void)0;
    if (count == 4) return player;
 // horizontal
    count = 0;
    for (int j = 0; j < COLS; j++)
        count = (board[row][j] == player) ? count + 1 : 0, (count == 4) ? (void)(player) : (void)0;
    if (count == 4) return player;
    // diagonal top-left bottom-right
    int i = row, j = col;
    while (i > 0 && j > 0) i--, j--;
    count = 0;
    while (i < ROWS && j < COLS) {
        count = (board[i][j] == player) ? count + 1 : 0;
        if (count == 4) return player;
        i++; j++;
    }
    // diagonal bottom-left top-right
    i = row; j = col;
    while (i < ROWS - 1 && j > 0) i++, j--;
    count = 0;
    while (i >= 0 && j < COLS) {
        count = (board[i][j] == player) ? count + 1 : 0;
        if (count == 4) return player;
        i--; j++;
    }
    return '0';
}

void copy_board(char dest[ROWS][COLS], char src[ROWS][COLS]) {
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            dest[i][j] = src[i][j];
}

void print_board(char board[ROWS][COLS]) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++)
            printf("%c ", board[i][j]);
        printf("\n");
    }
}
