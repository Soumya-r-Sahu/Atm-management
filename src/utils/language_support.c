#include "language_support.h"
#include "file_utils.h"
#include "../utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Current language setting
static Language currentLanguage = LANG_ENGLISH;

// Structure to store key-value pairs for localization
typedef struct {
    char key[50];
    char value[200];
} LocalizedString;

// Arrays to store localized strings for each language
static LocalizedString* languageData[LANG_COUNT] = {NULL};
static int stringCounts[LANG_COUNT] = {0};

// Helper function to parse a language file and populate the language data
static int loadLanguageFile(Language lang, const char* filePath) {
    // First, free any existing language data
    if (languageData[lang] != NULL) {
        free(languageData[lang]);
        languageData[lang] = NULL;
        stringCounts[lang] = 0;
    }
    
    // Read the file content
    char* content = readFile(filePath);
    if (content == NULL) {
        writeErrorLog("Failed to read language file");
        return 0;
    }
    
    // Count the number of lines in the file (to allocate memory)
    int lineCount = 0;
    char* ptr = content;
    while (*ptr) {
        if (*ptr == '\n') lineCount++;
        ptr++;
    }
    
    // Allocate memory for the language data
    languageData[lang] = (LocalizedString*)malloc(lineCount * sizeof(LocalizedString));
    if (languageData[lang] == NULL) {
        free(content);
        writeErrorLog("Failed to allocate memory for language data");
        return 0;
    }
    
    // Parse the file line by line
    char* line = strtok(content, "\n");
    int index = 0;
    
    while (line != NULL && index < lineCount) {
        // Skip empty lines and comments
        if (line[0] == '\0' || line[0] == '#') {
            line = strtok(NULL, "\n");
            continue;
        }
        
        // Split the line into key and value
        char* separator = strchr(line, '=');
        if (separator != NULL) {
            *separator = '\0';  // Split the string at the '=' character
            
            // Trim spaces and copy key
            char* key = line;
            while (*key == ' ' || *key == '\t') key++;
            strncpy(languageData[lang][index].key, key, sizeof(languageData[lang][index].key) - 1);
            languageData[lang][index].key[sizeof(languageData[lang][index].key) - 1] = '\0';
            
            // Trim trailing spaces from key
            char* end = languageData[lang][index].key + strlen(languageData[lang][index].key) - 1;
            while (end > languageData[lang][index].key && (*end == ' ' || *end == '\t')) *end-- = '\0';
            
            // Trim spaces and copy value
            char* value = separator + 1;
            while (*value == ' ' || *value == '\t') value++;
            strncpy(languageData[lang][index].value, value, sizeof(languageData[lang][index].value) - 1);
            languageData[lang][index].value[sizeof(languageData[lang][index].value) - 1] = '\0';
            
            // Trim trailing spaces from value
            end = languageData[lang][index].value + strlen(languageData[lang][index].value) - 1;
            while (end > languageData[lang][index].value && (*end == ' ' || *end == '\t')) *end-- = '\0';
            
            index++;
        }
        
        line = strtok(NULL, "\n");
    }
    
    stringCounts[lang] = index;
    free(content);
    
    return index > 0;
}

// Initialize language support by loading language files
int initLanguageSupport() {
    int success = 1;
    
    // Load English (default) language
    if (!loadLanguageFile(LANG_ENGLISH, "../data/languages/english.txt")) {
        writeErrorLog("Failed to load English language file");
        success = 0;
    }
    
    // Load Hindi language
    if (!loadLanguageFile(LANG_HINDI, "../data/languages/hindi.txt")) {
        writeErrorLog("Failed to load Hindi language file");
        success = 0;
    }
    
    // Load Odia language
    if (!loadLanguageFile(LANG_ODIA, "../data/languages/odia.txt")) {
        writeErrorLog("Failed to load Odia language file");
        success = 0;
    }
    
    return success;
}

// Set the current language
void setLanguage(Language lang) {
    if (lang >= 0 && lang < LANG_COUNT) {
        currentLanguage = lang;
    }
}

// Get the localized text for a given key
const char* getLocalizedText(const char* key) {
    // Look for the key in the current language
    for (int i = 0; i < stringCounts[currentLanguage]; i++) {
        if (strcmp(languageData[currentLanguage][i].key, key) == 0) {
            return languageData[currentLanguage][i].value;
        }
    }
    
    // If not found in current language, fall back to English
    if (currentLanguage != LANG_ENGLISH) {
        for (int i = 0; i < stringCounts[LANG_ENGLISH]; i++) {
            if (strcmp(languageData[LANG_ENGLISH][i].key, key) == 0) {
                return languageData[LANG_ENGLISH][i].value;
            }
        }
    }
    
    // If still not found, return the key as is (for debugging)
    return key;
}

// Get the current language
Language getCurrentLanguage() {
    return currentLanguage;
}