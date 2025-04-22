#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Encryption key
#define ENCRYPTION_KEY "ATM_SECRET_KEY"

// Function to encrypt or decrypt a string using XOR encryption
void xorEncryptDecrypt(char *data, const char *key) {
    size_t keyLen = strlen(key);
    for (size_t i = 0; i < strlen(data); i++) {
        data[i] ^= key[i % keyLen];
    }
}

// Function to encrypt the credentials file
void encryptCredentialsFile(const char *filePath) {
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        printf("Error: Unable to open %s file.\n", filePath);
        return;
    }

    FILE *tempFile = fopen("data/temp_credentials.txt", "w");
    if (tempFile == NULL) {
        printf("Error: Unable to create temporary file.\n");
        fclose(file);
        return;
    }

    char line[256];
    fgets(line, sizeof(line), file); // Write the header line as-is
    fprintf(tempFile, "%s", line);
    fgets(line, sizeof(line), file); // Write the separator line as-is
    fprintf(tempFile, "%s", line);

    char accountHolderName[50];
    int cardNumber, pin;
    char encryptedPIN[10];

    while (fscanf(file, "%49[^|] | %d | %d\n", accountHolderName, &cardNumber, &pin) == 3) {
        sprintf(encryptedPIN, "%d", pin);
        xorEncryptDecrypt(encryptedPIN, ENCRYPTION_KEY); // Encrypt the PIN
        fprintf(tempFile, "%-20s | %-11d | %-4s\n", accountHolderName, cardNumber, encryptedPIN);
    }

    fclose(file);
    fclose(tempFile);

    // Replace the original file with the encrypted file
    remove(filePath);
    rename("data/temp_credentials.txt", filePath);

    printf("Encrypted %s successfully.\n", filePath);
}

// Function to encrypt the accounting file
void encryptAccountingFile(const char *filePath) {
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        printf("Error: Unable to open %s file.\n", filePath);
        return;
    }

    FILE *tempFile = fopen("data/temp_accounting.txt", "w");
    if (tempFile == NULL) {
        printf("Error: Unable to create temporary file.\n");
        fclose(file);
        return;
    }

    char line[256];
    fgets(line, sizeof(line), file); // Write the header line as-is
    fprintf(tempFile, "%s", line);
    fgets(line, sizeof(line), file); // Write the separator line as-is
    fprintf(tempFile, "%s", line);

    int cardNumber;
    float balance;
    char encryptedBalance[20];

    while (fscanf(file, "%d | %f\n", &cardNumber, &balance) == 2) {
        sprintf(encryptedBalance, "%.2f", balance);
        xorEncryptDecrypt(encryptedBalance, ENCRYPTION_KEY); // Encrypt the balance
        fprintf(tempFile, "%-12d | %-8s\n", cardNumber, encryptedBalance);
    }

    fclose(file);
    fclose(tempFile);

    // Replace the original file with the encrypted file
    remove(filePath);
    rename("data/temp_accounting.txt", filePath);

    printf("Encrypted %s successfully.\n", filePath);
}

// Function to encrypt the admin credentials file
void encryptAdminCredentialsFile(const char *filePath) {
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        printf("Error: Unable to open %s file.\n", filePath);
        return;
    }

    FILE *tempFile = fopen("data/temp_admin_credentials.txt", "w");
    if (tempFile == NULL) {
        printf("Error: Unable to create temporary file.\n");
        fclose(file);
        return;
    }

    char adminId[50], adminPass[50];
    if (fscanf(file, "%s %s", adminId, adminPass) == 2) {
        xorEncryptDecrypt(adminId, ENCRYPTION_KEY); // Encrypt Admin ID
        xorEncryptDecrypt(adminPass, ENCRYPTION_KEY); // Encrypt Admin Password
        fprintf(tempFile, "%s %s\n", adminId, adminPass);
    }

    fclose(file);
    fclose(tempFile);

    // Replace the original file with the encrypted file
    remove(filePath);
    rename("data/temp_admin_credentials.txt", filePath);

    printf("Encrypted %s successfully.\n", filePath);
}

int main() {
    // Encrypt the necessary files
    encryptCredentialsFile("../data/credentials.txt");
    encryptAccountingFile("../data/accounting.txt");
    encryptAdminCredentialsFile("../data/admin_credentials.txt");

    printf("All necessary files have been encrypted successfully.\n");
    return 0;
}