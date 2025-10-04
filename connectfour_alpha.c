#include <stdio.h>
#include <stdlib.h>

void display(char** table);
void placeCoin(char** table, int column, char player);
/* int checkSuccess(char table[6][7], int row, int column, char player); */

void main(){
    printf("Welcome to Connect Four!");
    char playerA;
    char playerB;

    printf("\nPlayer A: ");
    scanf(" %c", &playerA);
    printf("Player B: ");
    scanf(" %c", &playerB);
    printf("\n");

    char** table = (char**) malloc(sizeof(char*)*6);
    for(int l = 0; l < 7; l++){
        *(table + l) = (char*) malloc(sizeof(char)*7);
    }

    for(int i = 0; i < 6; i++){
        for(int j = 0; j < 7; j++){
            table[i][j] = '*';
        }
    }

    display(table);
    placeCoin(table, 2, playerA);
    placeCoin(table, 2, playerB);
    placeCoin(table, 3, playerA);
    placeCoin(table, 1, playerB);

    /* 
    int n = checkSuccess(table, 2, 1, playerA);
    printf("\n%d", n); */
}

void display(char** table){
    for(int i = 0; i < 6; i++){
        for(int j = 0; j < 7; j++){
            printf("%c ", table[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void placeCoin(char** table, int column, char player){
    int row = 6;
    printf("test1");
    char* p = &(table[row][column]);
    printf("test2, *p = %c and p = %p", *p, p);
    while (*p != '*');
    {
        printf("test");
        row--;
        p = &(table[row][column]);
    }
    *p = player;

    display(table);
}

/* int checkSuccess(char table[6][7], int row, int column, char player){
    int distance = 0;
    
    //check vertically 

    for(int i = 0; i < 6; i++){
        if(table[column][i] == player){
            distance++;
        }
    }

    if(distance > 3){
        return 1;
    }
    else{
        distance = 0;
    }

    //check horizontal
    
    for(int j = 0; j < 7; j++){
        if(table[row][j] == player){
            distance++;
        }
    }

    if(distance > 3){
        return 1;
    }
    else{
        distance = 0;
    }

    //check diagonally 

    for(int k = 0; k < 7-column; k++){
        if(table[row + k][column + k] == player){
            distance++;
        }
    }

    for(int l = 1; l < column; l++){
        if(table[row - l][column - l] == player){
            distance++;
        }
    }

    if(distance > 3){
        return 1;
    }
    else{
        distance = 0;
    }
} */
