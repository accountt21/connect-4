#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void display(char table[6][7]);
int placeCoin(char table[6][7], int column, char player);
char checkSuccess(char table[6][7], int row, int column, char player);
int chooseColumn(char table[6][7], char difficulty);

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
        success = checkSuccess(table, curr_row, chosen_column, playerA);
        if(success != '0'){
            break;
        }
        attempts++;

        if(playerType == 'H'){
            printf("\nPlayer B, choose a column (1-7): ");
            scanf(" %d", &chosen_column);
        }
        else{
            chosen_column = chooseColumn(table, difficulty);
            printf("\nBot chose %d\n", chosen_column);
        }
        curr_row = placeCoin(table, chosen_column, playerB);
        success = checkSuccess(table, curr_row, chosen_column, playerB);
        if(success != '0'){
            break;
        }
        attempts++;
    }
    
    if(success == 0){
        printf("\nNo player wins");
    }
    else{
        printf("\nPlayer %c wins!", success);
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
        return row;
    }
   
    return row;
}

char checkSuccess(char table[6][7], int row, int column, char player){
    int distance = 0;
    column--;

    //check vertically 

    for(int i = 0; i < 6; i++){
        if(table[i][column] == player){
            distance++;
            if(distance == 4){
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
            if(distance == 4){
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
            if(distance == 4){
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
            if(distance == 4){
                return player;
            }
        }
        else{
            distance = 0;
        }
    }

    return '0';
    
}

int chooseColumn(char table[6][7], char difficulty){
    int r;
    if(difficulty == 'E'){
        srand(time(NULL));
        while (1)
        {
            r = rand() % 6 + 1;
            if(table[0][r-1] == '*'){
                break;
            }
        }
    }
    return r;
}
