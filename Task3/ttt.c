#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_NUMS 9

int validate_strategy(const char *strategy) {
    if (strlen(strategy) != MAX_NUMS) {
        return 1;
    }

    for (int i = 0; i < MAX_NUMS; i++) {
        if (strategy[i] < '1' || strategy[i] > '9') {
            return 1;
        }
        
        for (int j = i - 1; j >= 0; j--) {
            if (strategy[i] == strategy[j]) {
                return 1;
            }
        }
    }

    return 0;
}

int validate_move(int player_move, const char *board){
    return(board[player_move - 1] == 'O' || board[player_move - 1] == 'X'||
            player_move < 1 || player_move > MAX_NUMS);
    }

char next_move(const char *strategy, const char *board) {
    
    for(int i = 0; i < MAX_NUMS; i++){
        if(!validate_move((strategy[i]- '0'), board)){
            return strategy[i];
        }
    }
    
    return '\0';
}

int check_winner(const char *board, char player) {
    const int winning_combinations[8][3] = {
        {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, // rows
        {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, // columns
        {0, 4, 8}, {2, 4, 6}             // diagonals
    };
    for (int i = 0; i < 8; ++i) {
        if (board[winning_combinations[i][0]] == player &&
            board[winning_combinations[i][1]] == player &&
            board[winning_combinations[i][2]] == player) {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char *argv[]) 
{
    char board[10];
    if (argc != 2) {
        printf("Error\n");
        return 1;
    }

    const char *strategy = argv[1];
    if (validate_strategy(strategy)) {
        printf("Error\n");
        return 1;
    }
    
    int round = 0;
    while (round < 11) {
        char move = next_move(strategy, board);
        if (move == '\0') {
            printf("Error\n");
            return 1;
        }
        fprintf(stdout,"%c\n", move);

        board[move - '1'] = 'X';
        if (check_winner(board, 'X')) {
            printf("I win\n");
            return 0;
        }

        // Read player's move
        int player_move;
        if (scanf("%d", &player_move) != 1 || validate_move(player_move, board)) {
            printf("Error4\n");
            return 1;
        }
        board[player_move - 1] = 'O';

        if (check_winner(board, 'O')) {
            printf("I lose\n");
            return 0;
        }
        round++;
    }
    printf("Draw");
    return 0;
}
