#ifndef FILE_UTILS_H
#define FILE_UTILS_H

// Function to read a file and return its contents as a string
char *readFile(const char *filePath);

// Function to write a string to a file
int writeFile(const char *filePath, const char *content);

// Function to append a string to a file
int appendToFile(const char *filePath, const char *content);

#endif // FILE_UTILS_H