

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_PORT 8080
#define SERVER_ADDR "127.0.0.1"
#define BUFFER_SIZE 1024
#define CERT_LEN 32

typedef enum {
    STATE_NEED_LOGIN,
    STATE_NEED_GAME_ID,
    STATE_CONFIRM_IN_GAME,
    STATE_IN_GAME,
    STATE_GAME_ENDED
} ClientState;

void send_recv(const char *msg, char *response) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sockfd);
        exit(1);
    }
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        exit(1);
    }
    if (send(sockfd, msg, strlen(msg), 0) < 0) {
        perror("Send failed");
        close(sockfd);
        exit(1);
    }
    ssize_t n = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
    if (n < 0) {
        perror("Receive failed");
        close(sockfd);
        exit(1);
    }
    buffer[n] = '\0';
    strcpy(response, buffer);
    close(sockfd);
}

int main() {
    ClientState state = STATE_NEED_LOGIN;
    char cert[CERT_LEN+1] = "";
    int game_id = 0;
    char player = 0;
    int game_ended = 0;
    char response[BUFFER_SIZE*2];

    while (1) {
        if (state == STATE_NEED_LOGIN) {
            char username[32], password[32];
            printf("Enter username: ");
            scanf("%31s", username);
            printf("Enter password: ");
            scanf("%31s", password);
            char msg[128];
            snprintf(msg, sizeof(msg), "login %s %s", username, password);
            send_recv(msg, response);
            if (strncmp(response, "OK cert", 7) == 0) {
                sscanf(response, "OK cert %32s", cert);
                printf("Login successful. Certificate: %s\n", cert);
                state = STATE_NEED_GAME_ID;
            } else {
                printf("%s", response);
            }
        } else if (state == STATE_NEED_GAME_ID) {
            printf("Enter game id to join (1 or 2): ");
            scanf("%d", &game_id);
            char msg[128];
            snprintf(msg, sizeof(msg), "join %s %d", cert, game_id);
            send_recv(msg, response);
            if (strncmp(response, "OK joined as", 12) == 0) {
                printf("%s", response);
                state = STATE_CONFIRM_IN_GAME;
            } else {
                printf("%s", response);
            }
        } else if (state == STATE_CONFIRM_IN_GAME) {
            printf("Confirming you are in game %d...\n", game_id);
            // only optionally fetch board for server performance
            // TODO: only recieve new move instead of new board for server performance
            char msg[128];
            snprintf(msg, sizeof(msg), "board %s", cert);
            send_recv(msg, response);
            if (strncmp(response, "OK board", 8) == 0) {
                char *boardstr = response + 9;
                printf("Game board:\n");
                for (int i = 0; i < 6; i++) {
                    for (int j = 0; j < 7; j++) {
                        putchar(boardstr[i*7+j]);
                        putchar(' ');
                    }
                    putchar('\n');
                }
                state = STATE_IN_GAME;
            } else {
                printf("%s", response);
            }
        } else if (state == STATE_IN_GAME) {
            printf("1. Make move\n2. Show board\n");
            int opt;
            scanf("%d", &opt);
            if (opt == 1) {
                int col;
                printf("Enter column (1-7): ");
                scanf("%d", &col);
                char msg[128];
                snprintf(msg, sizeof(msg), "move %s %d", cert, col);
                send_recv(msg, response);
                if (strncmp(response, "OK win", 6) == 0) {
                    printf("Game ended. You win!\n");
                    state = STATE_GAME_ENDED;
                } else if (strncmp(response, "OK draw", 7) == 0) {
                    printf("Game ended. It's a draw.\n");
                    state = STATE_GAME_ENDED;
                } else if (strncmp(response, "OK next", 7) == 0) {
                    printf("Move accepted.\n");
                } else if (strncmp(response, "ERR game ended", 14) == 0) {
                    printf("Game has already ended.\n");
                    state = STATE_GAME_ENDED;
                } else {
                    printf("%s", response);
                }
            } else if (opt == 2) {
                char msg[128];
                snprintf(msg, sizeof(msg), "board %s", cert);
                send_recv(msg, response);
                if (strncmp(response, "OK board", 8) == 0) {
                    char *boardstr = response + 9;
                    printf("Game board:\n");
                    for (int i = 0; i < 6; i++) {
                        for (int j = 0; j < 7; j++) {
                            putchar(boardstr[i*7+j]);
                            putchar(' ');
                        }
                        putchar('\n');
                    }
                } else {
                    printf("%s", response);
                }
            }
        } else if (state == STATE_GAME_ENDED) {
            printf("Game ended. Only logout is allowed.\n");
            printf("1. Logout\n");
            int opt;
            do {
                scanf("%d", &opt);
            } while (opt != 1);
            char msg[128];
            snprintf(msg, sizeof(msg), "logout %s", cert);
            send_recv(msg, response);
            printf("%s", response);
            break;
        }
    }
    return 0;
}