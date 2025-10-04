#include <stdio.h>
#include <stdlib.h>

void display(char table[6][7]);

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
    display(table[6][7]);
    
}

void display(char table[6][7]){
    for(int i = 0; i < 6; i++){
        for(int j = 0; j < 7; j++){
            printf("%c ", table[i][j]);
        }
        printf("\n");
    }
}