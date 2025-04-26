#include "file_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to read a file and return its contents as a string
char *readFile(const char *filePath) {
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        printf("Error: Unable to open file %s\n", filePath);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    char *content = (char *)malloc((fileSize + 1) * sizeof(char));
    if (content == NULL) {
        printf("Error: Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    fread(content, sizeof(char), fileSize, file);
    content[fileSize] = '\0';

    fclose(file);
    return content;
}

// Function to write a string to a file
int writeFile(const char *filePath, const char *content) {
    FILE *file = fopen(filePath, "w");
    if (file == NULL) {
        printf("Error: Unable to open file %s\n", filePath);
        return -1;
    }

    fprintf(file, "%s", content);
    fclose(file);
    return 0;
}

// Function to append a string to a file
int appendToFile(const char *filePath, const char *content) {
    FILE *file = fopen(filePath, "a");
    if (file == NULL) {
        printf("Error: Unable to open file %s\n", filePath);
        return -1;
    }

    fprintf(file, "%s", content);
    fclose(file);
    return 0;
}