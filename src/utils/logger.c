#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void logToConsole(const char *message) {
    printf("%s\n", message);
}

void logToFile(const char *message, const char *sourceFile) {
    // Exclude logging activities from `admin.c`
    if (strcmp(sourceFile, "admin.c") == 0) {
        return;
    }

    FILE *file = fopen("data/transactions.log", "a");
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