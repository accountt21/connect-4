#include <stdio.h>
#include <stdlib.h>

void display(char table[6][7]);
void placeCoin(char table[6][7], int column, char player);
int checkSuccess(char table[6][7], int row, int column, char player);

int main(){
    //initial print statements
    printf("Welcome to Connect Four!");

    char playerA;
    char playerB;

    printf("\nEnter Player A: ");
    scanf(" %c", &playerA);
    printf("Enter Player B: ");
    scanf(" %c", &playerB);

    //initialize and fill the 2D array
    char table[6][7];

    for(int i = 0; i < 6; i++){
        for(int j = 0; j < 7; j++){
            table[i][j] = '*';
        }
    }

    //methods 
    display(table);
    placeCoin(table, 1, playerA);
    placeCoin(table, 2, playerB);
    placeCoin(table, 3, playerA);
    placeCoin(table, 4, playerB);
    placeCoin(table, 2, playerA);
    placeCoin(table, 3, playerB);
    placeCoin(table, 4, playerA);
    placeCoin(table, 3, playerA);
    placeCoin(table, 4, playerB);
    placeCoin(table, 1, playerA);

    int n = checkSuccess(table, 2, 3, playerA);
    printf("%d", n);
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

void placeCoin(char table[6][7], int column, char player){
    int row = 5;
    column--;
    char* p = &table[row][column];
     
    while (*p != '*')
    {
        row--;
        p = &table[row][column];
    }

    *p = player;

    display(table);
}

int checkSuccess(char table[6][7], int row, int column, char player){
    int distance = 0;
    
    //check vertically 

    for(int i = 0; i < 6; i++){
        if(table[i][column] == player){
            distance++;
            if(distance == 4){
                return 1;
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
                return 1;
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
                return 1;
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
                return 1;
            }
        }
        else{
            distance = 0;
        }
    }

    return 0;
    
}

