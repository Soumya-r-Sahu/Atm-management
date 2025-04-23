#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// ============================
// Logging Utilities
// ============================

// Function to log a message to the console
void logToConsole(const char *message) {
    printf("%s\n", message);
}

// Function to log a message to a file
void logToFile(const char *message, const char *sourceFile) {
    // Exclude logging activities from `admin.c`
    if (strcmp(sourceFile, "admin.c") == 0) {
        return;
    }

    FILE *file = fopen("../../data/transactions.log", "a");
    if (file == NULL) {
        logToConsole("Error opening log file.");
        return;
    }

    time_t now = time(NULL);
    char *timestamp = ctime(&now);
    timestamp[strlen(timestamp) - 1] = '\0'; // Remove newline character

    fprintf(file, "[%s] [%s] %s\n", timestamp, sourceFile, message);
    fclose(file);
}

// Centralized implementation of logError
#include <stdio.h>
#include <time.h>

void logError(const char *errorMessage) {
    FILE *file = fopen("../../logs/error.log", "a");
    if (file == NULL) {
        printf("Error: Unable to open error log file.\n");
        return;
    }

    time_t now = time(NULL);
    struct tm *localTime = localtime(&now);
    char timestamp[100];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localTime);

    fprintf(file, "[%s] ERROR: %s\n", timestamp, errorMessage);
    fclose(file);
}

// Function to log admin activity
void logAdminActivity(const char *activity) {
    FILE *file = fopen("../../logs/audit.log", "a");
    if (file == NULL) {
        printf("Error: Unable to open audit log file.\n");
        return;
    }

    time_t now = time(NULL);
    struct tm *localTime = localtime(&now);
    char timestamp[100];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localTime);

    fprintf(file, "[%s] ADMIN: %s\n", timestamp, activity);
    fclose(file);
}