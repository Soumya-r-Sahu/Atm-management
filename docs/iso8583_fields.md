# ISO8583 Message Field Documentation

This document provides an overview of the ISO8583 message fields used in the Core Banking System's implementation.

## Message Types

| Code | Description | Usage |
|------|-------------|-------|
| 0100 | Authorization Request | Used for balance inquiries, PIN verification |
| 0110 | Authorization Response | Response to authorization requests |
| 0200 | Financial Presentation Request | Used for withdrawals, deposits, transfers |
| 0210 | Financial Presentation Response | Response to financial transactions |
| 0400 | Reversal Request | Used to reverse a previous transaction |
| 0410 | Reversal Response | Response to reversal requests |
| 0800 | Network Management Request | Used for network management functions |
| 0810 | Network Management Response | Response to network management functions |

## Processing Codes

| Code | Description | Usage |
|------|-------------|-------|
| 000000 | Purchase | Retail purchase transaction |
| 010000 | Cash Withdrawal | ATM or cash withdrawal transaction |
| 200000 | Return/Refund | Refund or return transaction |
| 210000 | Deposit | Cash or check deposit transaction |
| 300000 | Balance Inquiry | Account balance inquiry |
| 400000 | Fund Transfer | Transfer between accounts |
| 920000 | PIN Change | Change PIN request |
| 940000 | PIN Verification | Verify PIN request |

## Response Codes

| Code | Description | Handling |
|------|-------------|---------|
| 00 | Approved | Transaction successful |
| 01 | Refer to card issuer | Contact card issuer |
| 05 | Do not honor | Transaction declined |
| 12 | Invalid transaction | Invalid transaction type |
| 13 | Invalid amount | Amount is incorrect |
| 14 | Invalid card number | Card number is not valid |
| 30 | Format error | Message format error |
| 41 | Lost card | Card reported as lost |
| 51 | Insufficient funds | Not enough funds available |
| 54 | Expired card | Card has expired |
| 55 | Invalid PIN | Incorrect PIN entered |
| 61 | Exceeds withdrawal limit | Transaction exceeds daily withdrawal limit |
| 91 | Issuer unavailable | Card issuer system not available |
| 96 | System malfunction | System error |

## Field Descriptions

| Field | Name | Format | Length | Description |
|-------|------|--------|--------|-------------|
| 2 | Primary Account Number (PAN) | LLVAR | 1-19 | Card number |
| 3 | Processing Code | FIXED | 6 | Transaction type code |
| 4 | Transaction Amount | FIXED | 12 | Amount in cents (no decimal point) |
| 7 | Transmission Date and Time | FIXED | 10 | MMDDhhmmss format |
| 11 | Systems Trace Audit Number | FIXED | 6 | Unique transaction ID |
| 12 | Local Transaction Time | FIXED | 6 | hhmmss format |
| 13 | Local Transaction Date | FIXED | 4 | MMDD format |
| 32 | Acquiring Institution ID | LLVAR | 1-11 | ID of the acquiring institution |
| 37 | Retrieval Reference Number | FIXED | 12 | Reference number for retrievals |
| 38 | Authorization ID Response | FIXED | 6 | Authorization ID |
| 39 | Response Code | FIXED | 2 | Transaction result code |
| 41 | Card Acceptor Terminal ID | FIXED | 8 | Terminal ID |
| 42 | Card Acceptor ID | FIXED | 15 | Merchant ID |
| 43 | Card Acceptor Name/Location | LLVAR | 1-40 | Merchant name and location |
| 44 | Additional Response Data | LLLVAR | 1-25 | Additional data in responses |
| 49 | Transaction Currency Code | FIXED | 3 | ISO currency code |
| 54 | Additional Amounts | LLLVAR | 1-120 | Additional amount information |
| 102 | Account Identification 1 | LLVAR | 1-28 | Source account ID |
| 103 | Account Identification 2 | LLVAR | 1-28 | Destination account ID |
| 128 | Message Authentication Code | FIXED | 8 | MAC for message security |

## Common Transaction Templates

### Balance Inquiry

Required fields: 2, 3, 7, 11, 12, 13, 32, 41, 49

### Cash Withdrawal

Required fields: 2, 3, 4, 7, 11, 12, 13, 32, 41, 49

### Deposit

Required fields: 2, 3, 4, 7, 11, 12, 13, 32, 41, 49

### Funds Transfer

Required fields: 2, 3, 4, 7, 11, 12, 13, 32, 41, 49, 102, 103

## Field Format Types

- **FIXED**: Fixed length fields, padded with spaces or zeros
- **LLVAR**: Variable length fields with 2-digit length indicator
- **LLLVAR**: Variable length fields with 3-digit length indicator

## Data Types

- **n**: Numeric only
- **a**: Alpha only
- **an**: Alphanumeric
- **ans**: Alphanumeric and special characters
- **b**: Binary data

## Message Authentication Code (MAC)

A Message Authentication Code (MAC) is generated and appended to messages to ensure integrity. The MAC is an 8-byte value calculated using a shared secret key.

In our implementation, the MAC is included as field 128 in the message. The MAC is calculated over the entire message excluding the MAC field itself.

## Implementation Details

### Creating an ISO8583 Message

1. Initialize a message structure with `create_iso_message()` and specify the message type.
2. Set fields using `set_iso_field()` for each required field.
3. Build the final message using `build_iso_message()`.

### Parsing an ISO8583 Message

1. Parse the raw message data using `parse_iso_message()`.
2. Validate the message structure with `validate_iso_fields()`.
3. Extract individual fields using `get_iso_field()`.
4. Free the message resources when done using `free_iso_message()`.

### Validating an ISO8583 Message

1. Validate the message against schema rules using `validate_iso8583_schema()`.
2. Validate the message against business rules using `validate_iso8583_business_rules()`.
3. Check for specific error codes and fields in case of validation failures.

## Security Considerations

1. Sensitive fields like PAN should be masked in logs.
2. All messages should include a MAC for integrity.
3. Timestamps should be validated to prevent replay attacks.
4. Fields should be validated for proper format and range.
