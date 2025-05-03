/**
 * @file file_utils.c
 * @brief Implementation of file utility functions for the ATM Management System
 * @date May 2, 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <time.h>
#include <common/utils/file_utils.h>

// Function to read a file and return its contents as a string
char *readFile(const char *filePath) {
    FILE *file = fopen(filePath, "r");
    if (!file) {
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Allocate memory for the content
    char *content = (char *)malloc(fileSize + 1);
    if (!content) {
        fclose(file);
        return NULL;
    }
    
    // Read the file content
    size_t bytesRead = fread(content, 1, fileSize, file);
    content[bytesRead] = '\0';
    
    fclose(file);
    return content;
}

// Function to write a string to a file
int writeFile(const char *filePath, const char *content) {
    FILE *file = fopen(filePath, "w");
    if (!file) {
        return 0;
    }
    
    int result = fputs(content, file) != EOF;
    fclose(file);
    return result;
}

// Function to append a string to a file
int appendToFile(const char *filePath, const char *content) {
    FILE *file = fopen(filePath, "a");
    if (!file) {
        return 0;
    }
    
    int result = fputs(content, file) != EOF;
    fclose(file);
    return result;
}

// Function to check if a file exists
int fileExists(const char *filePath) {
    struct stat buffer;
    return (stat(filePath, &buffer) == 0);
}

// Function to create a backup of a file
int backupFile(const char *filePath) {
    if (!fileExists(filePath)) {
        return 0;
    }
    
    // Generate backup filename with timestamp
    char backupPath[256];
    time_t now = time(NULL);
    struct tm *timeInfo = localtime(&now);
    
    sprintf(backupPath, "%s.backup.%04d%02d%02d%02d%02d%02d", 
            filePath,
            timeInfo->tm_year + 1900, 
            timeInfo->tm_mon + 1, 
            timeInfo->tm_mday,
            timeInfo->tm_hour, 
            timeInfo->tm_min, 
            timeInfo->tm_sec);
    
    // Read source file
    char *content = readFile(filePath);
    if (!content) {
        return 0;
    }
    
    // Write to backup file
    int result = writeFile(backupPath, content);
    free(content);
    
    return result;
}

// Function to write data in the structured format
int writeStructuredData(const char *filePath, const char *header, const char **data, int numFields) {
    FILE *file = fopen(filePath, "w");
    if (!file) {
        return 0;
    }
    
    // Write header if provided
    if (header) {
        fprintf(file, "%s\n", header);
    }
    
    // Write data with pipe separator
    for (int i = 0; i < numFields; i++) {
        fprintf(file, "%s", data[i]);
        if (i < numFields - 1) {
            fprintf(file, "|");
        }
    }
    fprintf(file, "\n");
    
    fclose(file);
    return 1;
}

// Function to append data in the structured format
int appendStructuredData(const char *filePath, const char **data, int numFields) {
    FILE *file = fopen(filePath, "a");
    if (!file) {
        return 0;
    }
    
    // Write data with pipe separator
    for (int i = 0; i < numFields; i++) {
        fprintf(file, "%s", data[i]);
        if (i < numFields - 1) {
            fprintf(file, "|");
        }
    }
    fprintf(file, "\n");
    
    fclose(file);
    return 1;
}

// Function to get the production or test file path based on mode
const char* getFilePath(const char* baseFileName) {
    // Check if we're in test mode - this would be implemented based on project requirements
    bool testMode = false;
    
    // In a real implementation, this would check environment variables or config
    // For now, we just return the base file name
    return baseFileName;
}
