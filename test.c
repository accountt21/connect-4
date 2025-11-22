#include <stdio.h>
#include <stdlib.h>

int main(void) {
    FILE *file;

    // open file for writing
    file = fopen("example.txt", "w");
    if (file == NULL) {
        perror("error opening file for writing");
        return 1;
    }

    // write data to file
    char data[] = "hello allie from pona!";
    size_t written = fwrite(data, sizeof(char), sizeof(data), file);
    if (written < sizeof(data)) {
        perror("error writing to file");
    }

    // close file
    if (fclose(file) == EOF) {
        perror("error closing file");
    }

    // open file for reading
    file = fopen("example.txt", "r");
    if (file == NULL) {
        perror("error opening file for reading");
        return 1;
    }

    // read data from file

    char buffer[50];
    size_t read = fread(buffer, sizeof(char), sizeof(data), file);
    buffer[read] = '\0'; // null-terminate the string

    printf("content read from file: %s\n", buffer);

    // close file again
    if (fclose(file) == EOF) {
        perror("error closing file after reading");
    }

    return 0;
}