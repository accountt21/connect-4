#include <stdio.h>
#include <stdlib.h>

void display(char table[6][7]);
void placeCoin(char table[6][7], int column, char player);

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
    placeCoin(table, 2, playerA);
    placeCoin(table, 2, playerB);
    placeCoin(table, 3, playerA);
    placeCoin(table, 1, playerB);
    placeCoin(table, 4, playerA);
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



