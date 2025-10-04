

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include "game_logic.c"

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_USERS 10
#define MAX_GAMES 5
#define CERT_LEN 32
#define PASSWORD "letmein"

typedef struct 
{
    char username[32];
    char cert[CERT_LEN+1];
    int in_game;
    int game_id;
    char player;
    int logged_in;
} User;

typedef struct 
{
    int id;
    char board[ROWS][COLS];
    char certA[CERT_LEN+1];
    char certB[CERT_LEN+1];
    char turn;
    int ended;
    char winner;
    int moves;
} Game;

User users[MAX_USERS];
Game games[MAX_GAMES];
int user_count = 0;
int game_count = 0;

void gen_cert(char *buf) {
	for (int i = 0; i < CERT_LEN; i++)
		buf[i] = 'A' + rand() % 26;
	buf[CERT_LEN] = 0;
}

User* find_user_by_cert(const char *cert) {
	for (int i = 0; i < user_count; i++)
		if (strcmp(users[i].cert, cert) == 0)
			return &users[i];
	return NULL;
}

Game* find_game_by_id(int id) {
	for (int i = 0; i < game_count; i++)
		if (games[i].id == id)
			return &games[i];
	return NULL;
}

int add_user(const char *username) {
	if (user_count >= MAX_USERS) return -1;
	strcpy(users[user_count].username, username);
	users[user_count].in_game = 0;
	users[user_count].game_id = -1;
	users[user_count].player = 0;
	users[user_count].logged_in = 0;
	gen_cert(users[user_count].cert);
	user_count++;
	return user_count-1;
}

int add_game() {
	if (game_count >= MAX_GAMES) return -1;
	games[game_count].id = game_count+1;
	init_board(games[game_count].board);
	games[game_count].certA[0] = 0;
	games[game_count].certB[0] = 0;
	games[game_count].turn = 'A';
	games[game_count].ended = 0;
	games[game_count].winner = 0;
	games[game_count].moves = 0;
	game_count++;
	return game_count-1;
}


int main() {
	srand(time(NULL));
	int server_fd, new_socket;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[BUFFER_SIZE] = {0};

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
    {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0) 
    {
		perror("listen");
		exit(EXIT_FAILURE);
	}
	printf("Server listening on port %d...\n", PORT);

	for (int i = 0; i < 2; i++) add_game();

	while (1) {
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) 
        {
			perror("accept");
			exit(EXIT_FAILURE);
		}


		int valread = read(new_socket, buffer, BUFFER_SIZE - 1);
		buffer[valread] = '\0';
		printf("Received: %s\n", buffer);

		char response[BUFFER_SIZE*2] = {0};
		char cmd[32], arg1[64], arg2[64], arg3[64];
		cmd[0]=arg1[0]=arg2[0]=arg3[0]=0;
		sscanf(buffer, "%31s %63s %63s %63s", cmd, arg1, arg2, arg3);

		if (strcmp(cmd, "login") == 0) {
			if (strcmp(arg2, PASSWORD) != 0) {
				snprintf(response, sizeof(response), "ERR wrong password\n");
			} else {
				int idx = add_user(arg1);
				if (idx < 0) {
					snprintf(response, sizeof(response), "ERR user limit\n");
				} else {
					users[idx].logged_in = 1;
					snprintf(response, sizeof(response), "OK cert %s\n", users[idx].cert);
				}
			}
		} else if (strcmp(cmd, "join") == 0) {
			User *u = find_user_by_cert(arg1);
			int gid = atoi(arg2);
			Game *g = find_game_by_id(gid);
			if (!u || !u->logged_in) {
				snprintf(response, sizeof(response), "ERR not logged in\n");
			} else if (!g) {

				snprintf(response, sizeof(response), "ERR no such game\n");
			} else if (g->certA[0] == 0) 
            {

    
				strcpy(g->certA, u->cert);
				u->in_game = 1; u->game_id = gid; u->player = 'A';
				snprintf(response, sizeof(response), "OK joined as A\n");

			} else if (g->certB[0] == 0) 
            {
				strcpy(g->certB, u->cert);
				u->in_game = 1; u->game_id = gid; u->player = 'B';
				snprintf(response, sizeof(response), "OK joined as B\n");
			} else {
				snprintf(response, sizeof(response), "ERR game full\n");
			}
		} else if (strcmp(cmd, "move") == 0) {
			User *u = find_user_by_cert(arg1);
			if (!u || !u->in_game) {
				snprintf(response, sizeof(response), "ERR not in game\n");
			} else {
				Game *g = find_game_by_id(u->game_id);
				if (!g || g->ended) {

					snprintf(response, sizeof(response), "ERR game ended\n");
				} else if ((u->player == 'A' && strcmp(g->certA, u->cert) != 0) || (u->player == 'B' && strcmp(g->certB, u->cert) != 0)) {
					
                    snprintf(response, sizeof(response), "ERR not your game\n");
				} else if (g->turn != u->player) {

					snprintf(response, sizeof(response), "ERR not your turn\n");
				
                } else {
					int col = atoi(arg2);
					int row = place_coin(g->board, col, u->player);
					if (row < 0) {
						snprintf(response, sizeof(response), "ERR column full\n");
					} else {
						g->moves++;
						char win = check_win(g->board, row, col, u->player);
						if (win == u->player) {
							g->ended = 1; g->winner = u->player;
							snprintf(response, sizeof(response), "OK win\n");
						} else if (g->moves >= ROWS*COLS) {
							
                            g->ended = 1; g->winner = 0;
							snprintf(response, sizeof(response), "OK draw\n");
						} else {

							g->turn = (u->player == 'A') ? 'B' : 'A';
							snprintf(response, sizeof(response), "OK next\n");
						}
					}
				}
			}
		} else if (strcmp(cmd, "board") == 0) {
			User *u = find_user_by_cert(arg1);
			if (!u || !u->in_game) {
				snprintf(response, sizeof(response), "ERR not in game\n");
			} else {
				Game *g = find_game_by_id(u->game_id);
				if (!g) {
					snprintf(response, sizeof(response), "ERR no such game\n");
				} else {
					char boardstr[256] = {0};
					for (int i = 0; i < ROWS; i++) {
						for (int j = 0; j < COLS; j++)
							boardstr[i*COLS+j] = g->board[i][j];
					}
					snprintf(response, sizeof(response), "OK board %s\n", boardstr);
				}
			}
		} else if (strcmp(cmd, "logout") == 0) {
			User *u = find_user_by_cert(arg1);
			if (!u) 
            {
				snprintf(response, sizeof(response), "ERR not logged in\n");
			} else
            {

				u->logged_in = 0; u->in_game = 0; u->game_id = -1; u->player = 0;
				snprintf(response, sizeof(response), "OK logged out\n");
			}
		} else {
			snprintf(response, sizeof(response), "ERR unknown command\n");
		}

		send(new_socket, response, strlen(response), 0);
		close(new_socket);
	}
	return 0;
}
