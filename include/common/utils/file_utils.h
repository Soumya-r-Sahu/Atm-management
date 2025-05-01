#ifndef FILE_UTILS_H
#define FILE_UTILS_H

// Function to read a file and return its contents as a string
char *readFile(const char *filePath);

// Function to write a string to a file
int writeFile(const char *filePath, const char *content);

// Function to append a string to a file
int appendToFile(const char *filePath, const char *content);

// Function to check if a file exists
int fileExists(const char *filePath);

// Function to create a backup of a file
int backupFile(const char *filePath);

// Function to write data in the structured format
int writeStructuredData(const char *filePath, const char *header, const char **data, int numFields);

// Function to append data in the structured format
int appendStructuredData(const char *filePath, const char **data, int numFields);

// Function to get the production or test file path based on mode
const char* getFilePath(const char* baseFileName);

#endif // FILE_UTILS_H