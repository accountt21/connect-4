#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void display(char table[6][7]);
int placeCoin(char table[6][7], int column, char player);
char checkSuccess(char table[6][7], int row, int column, char player, int n);
int chooseColumn(char table[6][7], char difficulty, int row, int column, char playerA, char playerB);
int checkLevel(char table[6][7], int row, int column);
int checkColumn(char table[6][7], int row, int column);

//stores the last move and the next move of the bot
int nextMove = -1;
int oldMove[2] = {-1, -1};

int main(){
    //initial print statements
    printf("Welcome to Connect Four!");

    char playerType;
    char difficulty;
    printf("\nPress H to play against a human and B to play against a bot: ");
    scanf(" %c", &playerType);

    char playerA;
    char playerB;

    printf("\nEnter Player A: ");
    scanf(" %c", &playerA);

    if(playerType == 'H'){
        printf("Enter Player B: ");
        scanf(" %c", &playerB);
    }
    else{
        playerB = 'B';
        printf("\nEnter the bot difficulty (E for easy, M for medium, H for hard): ");
        scanf(" %c", &difficulty);
    }

    //initialize and fill the 2D array
    char table[6][7];

    for(int i = 0; i < 6; i++){
        for(int j = 0; j < 7; j++){
            table[i][j] = '*';
        }
    }

    //game

    display(table);

    int attempts = 0;
    int chosen_column;
    int curr_row;
    char success = '0';
    
    while (success == '0' && attempts < 43)
    {
        printf("\nPlayer A, choose a column (1-7): ");
        scanf(" %d", &chosen_column);
        curr_row = placeCoin(table, chosen_column, playerA);
        success = checkSuccess(table, curr_row, chosen_column, playerA, 4);
        if(success != '0'){
            break;
        }
        attempts++;

        if(playerType == 'H'){
            printf("\nPlayer B, choose a column (1-7): ");
            scanf(" %d", &chosen_column);
        }
        else{
            chosen_column = chooseColumn(table, difficulty, curr_row, chosen_column, playerA, playerB);
            printf("\nBot chose %d\n", chosen_column);
        }
        curr_row = placeCoin(table, chosen_column, playerB);
        success = checkSuccess(table, curr_row, chosen_column, playerB, 4);
        if(success != '0'){
            break;
        }
        attempts++;
    }
    
    if(success == 0){
        printf("\nNo player wins\n\n");
    }
    else{
        printf("\nPlayer %c wins!\n\n", success);
    }
}

void display(char table[6][7]){
    printf("\n");
    for(int i = 0; i < 6; i++){
        for(int j = 0; j < 7; j++){
            printf("%c ", table[i][j]);
        }
        printf("\n");
    }
    printf("1 2 3 4 5 6 7");
    printf("\n");
}

int placeCoin(char table[6][7], int column, char player){
    int row = 5;
    column--;
    char* p = &table[row][column];
     
    while (*p != '*' && row > -1)
    {
        row--;
        p = &table[row][column];
    }

    if(row == -1){
        printf("\nThe column %d is full, choose another one (1-7): ", column+1);
        scanf(" %d", &column);
        return placeCoin(table, column, player);
    }
    else{
        *p = player;
        display(table);
        //updates the row of the bot's last move
        if(player == 'B'){
            oldMove[0] = row;
        }
        return row;
    }

    return row;
}

char checkSuccess(char table[6][7], int row, int column, char player, int n){
    int distance = 0;
    column--;

    //check vertically 

    for(int i = 0; i < 6; i++){
        if(table[i][column] == player){
            distance++;
            if(distance == n){
                //checks if placing coin on column is legal 
                if(i > 0 && table[i-n][column] == '*' && (player != 'B' || checkColumn(table, i-n, column) == 1)){
                    nextMove = column;
                }
                else{
                    nextMove = -1;
                }
                return player;
            }
        }
        else{
            distance = 0;
        }
    }

    distance = 0;

    //check horizontal
    
    for(int j = 0; j < 7; j++){
        if(table[row][j] == player){
            distance++;
            if(distance == n){
                if(j > 3 && table[row][j-1] != 'B' && table[row][j-1] != '*' && table[row][j-4] != 'B' && table[row][j-3] != '*'){
                    nextMove = j-2;
                }
                else if(j < 7 && table[row][j+1] == '*' && checkLevel(table, row, j+1) == 1){
                    nextMove = j+1;
                }
                else if(j > n-1 && table[row][j-n] == '*' && checkLevel(table, row, j-n) == 1){
                    nextMove = j-n;
                }
                else{
                    nextMove = -1;
                }
                return player;
            }
        }
        else{
            distance = 0;
        }
    }

    distance = 0;

    //check diagonal

    //find the index of the start of the top-left bottom-right diagonal
    int i = 0;
    int j = 0;

    if (row <= column) {
        i = 0;
        j = column-row;
    } 
    else {
        i = row-column;
        j = 0;
    }

    //iterate
    
    while (i < 6 && j < 7)
    {
        if(table[i++][j++] == player){
            distance++;
            if(distance == n){
                nextMove = -1;
                return player;
            }
        }
        else{
            distance = 0;
        }
    }

    //find the index of the start of the bottom-left top-right diagonal

    if (row + column < 7) {
        i = 0;
        j = row + column;
    } else {
        i = row + column - 6;
        j = 6;
    }

    //iterate

    while (i < 6 && j >= 0){
        if(table[i++][j--] == player){
            distance++;
            if(distance == n){
                nextMove = -1;
                return player;
            }
        }
        else{
            distance = 0;
        }
    }

    return '0';
    
}

int chooseColumn(char table[6][7], char difficulty, int row, int column, char playerA, char playerB){
    if(difficulty == 'E'){
        int r;
        srand(time(NULL));
        while (1)
        {
            r = rand() % 7 + 1;
            if(table[0][r-1] == '*'){
                return r;
            }
        }
    }
    else if(difficulty == 'M'){
        nextMove = -1;
        int i = 3;
        while (i > 0)
        {  
            if(oldMove[0] != -1 && checkSuccess(table, oldMove[0], oldMove[1], playerB, i) == playerB && nextMove != -1){
                return nextMove + 1;
            }
            else if(checkSuccess(table, row, column, playerA, i) == playerA && nextMove != -1){
                oldMove[1] = nextMove + 1;
                return nextMove + 1;
            }
            i--;
        }
    }
    return 0;
}

int checkLevel(char table[6][7], int row, int column){
    int i = 0;
    row++;
    while(row < 7){
        if(table[row][column] == '*'){
            return 0;
        }
        row++;
    }
    return 1;
}

int checkColumn(char table[6][7], int row, int column){
    int emptyspace = 0;
    int wonspace = 0;
    int row_temp = row;
    row--;
    while(row > 0 && table[row][column] == '*'){
        emptyspace++;
        row--;
    }
    row = row_temp;
    while(row < 7 && table[row][column] == 'B'){
        wonspace++;
        row++;
    }
    if(emptyspace + wonspace > 3){
        return 1;
    }
    else{
        return 0;
    }
}

