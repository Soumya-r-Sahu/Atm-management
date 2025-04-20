#ifndef CARD_NUM_VALIDATION_H
#define CARD_NUM_VALIDATION_H

int isCardNumberValid(int cardNumber);
int loadCredentials(int cardNumber, int *pin, char *accountHolderName);

#endif // CARD_NUM_VALIDATION_H