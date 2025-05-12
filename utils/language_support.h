#ifndef LANGUAGE_SUPPORT_H
#define LANGUAGE_SUPPORT_H

// Supported languages
typedef enum {
    LANG_ENGLISH,
    LANG_HINDI,
    LANG_ODIA,
    LANG_COUNT  // Used to determine the number of supported languages
} Language;

// Function prototypes
int initLanguageSupport();
void setLanguage(Language lang);
const char* getLocalizedText(const char* key);
Language getCurrentLanguage();

#endif // LANGUAGE_SUPPORT_H