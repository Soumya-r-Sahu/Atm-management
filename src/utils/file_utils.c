#include "file_utils.h"
#include "../common/constants.h"
#include "../common/paths.h"  // Added for isTestingMode()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

// Function to check if a file exists
int fileExists(const char *filePath) {
    FILE *file = fopen(filePath, "r");
    if (file != NULL) {
        fclose(file);
        return 1;
    }
    return 0;
}

// Function to create a backup of a file
int backupFile(const char *filePath) {
    char backupPath[256];
    char timestamp[20];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", t);
    sprintf(backupPath, "%s.%s.bak", filePath, timestamp);
    
    char *content = readFile(filePath);
    if (content == NULL) {
        return -1;
    }
    
    int result = writeFile(backupPath, content);
    free(content);
    return result;
}

// Function to write data in the new structured format
// Format: Field1 | Field2 | Field3 | ...
int writeStructuredData(const char *filePath, const char *header, const char **data, int numFields) {
    FILE *file = fopen(filePath, "w");
    if (file == NULL) {
        return -1;
    }
    
    // Write header row
    fprintf(file, "%s\n", header);
    
    // Write separator line
    for (int i = 0; i < strlen(header); i++) {
        fprintf(file, "-");
    }
    fprintf(file, "\n");
    
    // Write data fields with separator
    for (int i = 0; i < numFields; i++) {
        fprintf(file, "%s", data[i]);
        if (i < numFields - 1) {
            fprintf(file, " | ");
        }
    }
    fprintf(file, "\n");
    
    fclose(file);
    return 0;
}

// Function to append data in the new structured format
int appendStructuredData(const char *filePath, const char **data, int numFields) {
    FILE *file = fopen(filePath, "a");
    if (file == NULL) {
        return -1;
    }
    
    // Write data fields with separator
    for (int i = 0; i < numFields; i++) {
        fprintf(file, "%s", data[i]);
        if (i < numFields - 1) {
            fprintf(file, " | ");
        }
    }
    fprintf(file, "\n");
    
    fclose(file);
    return 0;
}

// Function to get the production or test file path based on mode
const char* getFilePath(const char* baseFileName) {
    static char filePath[256];
    if (isTestingMode()) {
        sprintf(filePath, "%s/test_%s", TEST_DATA_DIR, baseFileName);
    } else {
        sprintf(filePath, "%s/%s", PROD_DATA_DIR, baseFileName);
    }
    return filePath;
}