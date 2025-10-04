#include <stdio.h>
#include <stdlib.h>

void display(char table[6][7]);
void placeCoin(char table[6][7], int column, char player);


void main(){
    printf("Welcome to Connect Four!");
    char playerA;
    char playerB;

    printf("\nPlayer A: ");
    scanf(" %c", &playerA);
    printf("Player B: ");
    scanf(" %c", &playerB);
    printf("\n");

    char table[6][7];

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

}

void display(char table[6][7]){
    for(int i = 0; i < 6; i++){
        for(int j = 0; j < 7; j++){
            printf("%c ", table[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void placeCoin(char table[6][7], int column, char player){
    int row = 6;
    char *p = &(table[row][column]);

    while (start != '*' && row > -1);
    {
        row--;
        start = table[row][column];
    }
    
    if(row > -1){
        table[row][column] = player;
    }

    display(table);
}