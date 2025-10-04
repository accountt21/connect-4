#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_PORT 8080
#define SERVER_ADDR "127.0.0.1"
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char *ping_msg = "ping";
    int option;

    printf("1. Ping server\n");
    printf("Select option: ");
    scanf("%d", &option);

    if (option != 1) {
        printf("Invalid option.\n");
        return 1;
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sockfd);
        return 1;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        return 1;
    }

    if (send(sockfd, ping_msg, strlen(ping_msg), 0) < 0) {
        perror("Send failed");
        close(sockfd);
        return 1;
    }

    ssize_t n = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
    if (n < 0) {
        perror("Receive failed");
        close(sockfd);
        return 1;
    }
    buffer[n] = '\0';

    printf("Server response: %s\n", buffer);

    close(sockfd);
    return 0;
}