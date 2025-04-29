Collecting workspace information# 🏧 ATM Management System in C Language

Welcome to the **ATM Management System**! This comprehensive C program simulates a real-world ATM with advanced user and administrative functionalities. Below is a detailed guide to help you navigate through this feature-rich application.

---

## ✨ Key Features:

### 1️⃣ **User Authentication**
- 💳 Card number validation with secure PIN verification
- 🔒 Sophisticated hash-based PIN authentication
- 🛡️ Account blocking after multiple failed attempts
- ⏱️ Automatic session timeout for security

### 2️⃣ **Transaction Management**
- 💰 Balance inquiries with detailed account information
- 📥 Cash deposits with instant balance updates
- 📤 Withdrawals with sufficient funds verification
- 💸 Money transfers between accounts
- 📃 Mini statements showing recent transactions

### 3️⃣ **Security Features**
- 🔐 PIN change functionality with verification
- 🚫 Card blocking/unblocking mechanism
- 📝 Comprehensive transaction logging
- 🔍 Fraud detection for suspicious activities

### 4️⃣ **Admin Dashboard**
- 📊 Real-time ATM status monitoring
- 💵 Cash management and reporting
- 👥 User account management
- 🛠️ System configuration and maintenance
- 📈 Transaction analytics and reporting

### 5️⃣ **Multi-Language Support**
- 🌍 Supports English, Hindi, and Odia languages
- 🗣️ Language selection at startup
- 🔤 Localized messages for better user experience

### 6️⃣ **Receipt Generation**
- 🧾 Digital receipts for all transactions
- 📱 Option to send receipts via SMS (simulation)
- 📅 Timestamp and transaction reference included

### 7️⃣ **Error Handling**
- ⚠️ Comprehensive error messages
- 🔄 Graceful recovery from failures
- 📝 Detailed error logging for troubleshooting

---

## 🛠️ Functions Explained:

### 👤 User Functions
| Function | Description | Error Handling |
|----------|-------------|----------------|
| `handleCardAuthentication()` | Validates card number and PIN | Blocks card after 3 failed attempts |
| `displayMainMenu()` | Shows main transaction options | Handles invalid selections |
| `handleBalanceCheck()` | Retrieves and displays account balance | Shows error if database access fails |
| `handleDeposit()` | Processes cash deposits | Validates input amount and updates balance |
| `handleWithdrawal()` | Processes withdrawals with limit checks | Checks for sufficient funds and daily limits |
| `handleMoneyTransfer()` | Transfers funds between accounts | Verifies recipient account and sender's balance |
| `handleMiniStatement()` | Shows recent transaction history | Displays message if no transactions exist |
| `handlePinChange()` | Updates user's PIN securely | Verifies current PIN before allowing changes |

### 👨‍💼 Admin Functions
| Function | Description | Error Handling |
|----------|-------------|----------------|
| `displayDashboard()` | Shows ATM status and statistics | Handles missing data gracefully |
| `manageCash()` | Monitors and manages cash levels | Displays warnings for low cash levels |
| `manageUsers()` | Account creation and management | Validates input and prevents duplicates |
| `manageTransactions()` | Reviews transaction history | Provides filtering options |
| `atmMaintenance()` | ATM status toggling and reporting | Confirms status changes |
| `securityManagement()` | Monitors suspicious activity | Alerts for potential security issues |
| `toggleServiceMode()` | Sets ATM online/offline | Requires confirmation for mode changes |

---

## ⚠️ Common Errors and Solutions:

| Error | Cause | Solution |
|-------|-------|----------|
| "Invalid card number" | Non-existent card number entered | Double-check card number or contact bank |
| "Incorrect PIN" | Wrong PIN entered | Try again or reset PIN through bank |
| "Insufficient funds" | Withdrawal amount exceeds balance | Deposit funds or reduce withdrawal amount |
| "Daily limit exceeded" | Transactions exceed daily limit | Try again tomorrow or request limit increase |
| "Card is blocked" | Multiple failed PIN attempts | Contact bank to unblock card |
| "ATM out of service" | ATM in maintenance mode | Try another ATM or come back later |
| "Session timeout" | Inactivity for extended period | Restart transaction |

---

## 📊 Transaction Flow:

```
┌─────────────────┐
│    Start ATM    │
└────────┬────────┘
         │
┌────────▼────────┐
│  Select Language │
└────────┬────────┘
         │
┌────────▼────────┐
│ Enter Card Number│
└────────┬────────┘
         │
┌────────▼────────┐
│   Enter PIN     │◄──────┐
└────────┬────────┘       │
         │                │
         │ Failed         │
┌────────▼────────┐       │
│Verify Credentials│───────┘
└────────┬────────┘
         │ Success
┌────────▼────────┐
│  ATM Main Menu  │◄─────────┐
└────────┬────────┘          │
         │                   │
         ├───────────────────┘
         │
┌────────▼────────┐
│Select Transaction│
└────────┬────────┘
         │
         ├─────────────┬─────────────┬─────────────┬─────────────┐
         │             │             │             │             │
┌────────▼────┐ ┌──────▼─────┐ ┌─────▼─────┐ ┌─────▼─────┐ ┌─────▼─────┐
│Check Balance│ │  Deposit   │ │ Withdraw  │ │ Change PIN│ │Mini-Statement│
└────────┬────┘ └──────┬─────┘ └─────┬─────┘ └─────┬─────┘ └─────┬─────┘
         │            │             │             │             │
         └────────────┴─────────────┴──────┬──────┴─────────────┘
                                           │
                                  ┌────────▼────────┐
                                  │Generate Receipt │
                                  └────────┬────────┘
                                           │
                                  ┌────────▼────────┐
                                  │  Exit / Logout  │
                                  └─────────────────┘
```

---

## 🖥️ Sample Output:

### 🔹 User Login:
```
 ____________________________________________________
|                                                    |
|              WELCOME TO ATM SYSTEM                 |
|                                                    |
|____________________________________________________|

===== Customer Authentication =====
Please enter your card number: 1234567890

===== PIN Verification =====
Please enter your PIN: ****

PIN verified successfully!
```

### 🔹 Main Menu:
```
 ___________________________________________________
|                                                   |
|              WELCOME John Doe                     |
|___________________________________________________|

 ___________________________________________________
|                                                   |
|                   MAIN MENU                       |
|___________________________________________________|
| 1. Check Balance                                  |
| 2. Deposit                                        |
| 3. Withdraw                                       |
| 4. Money Transfer                                 |
| 5. Mini Statement                                 |
| 6. Change PIN                                     |
| 7. Exit                                           |
|___________________________________________________|

Enter your choice: 1
```

### 🔹 Balance Check Result:
```
Checking your account balance...

===== BALANCE INFORMATION =====
Account Holder: John Doe
Current Balance: $1,250.75
===============================

=========== RECEIPT ==========
Date: 2025-04-27 15:30:45
Card: **** **** **** 7890
Transaction: Balance Check
Current Balance: $1,250.75
============================
Thank you for using our ATM!
```

### 🔹 Admin Dashboard:
```
===== 📊 Dashboard =====
ATM Status: 3 of 4 ATMs Online
Total Cash Available: ₹745,000.00
Number of Transactions Today: 45
Total Transaction Count: 338

--- Alerts ---
- Multiple failed PIN attempts detected (Card: **** 1234)
- Low cash warning for ₹500 notes
- Scheduled maintenance due in 3 days
```

---

## 📝 Admin Configuration Details:

The system supports extensive configuration options managed through the admin interface:

- **Withdrawal Limits**: Configurable daily and per-transaction limits
- **Session Timeout**: Adjustable user session timeout (default: 180 seconds)
- **Language Settings**: Default language selection
- **Security Parameters**: PIN attempt limits and lockout durations
- **Cash Management**: Denomination tracking and alerts
- **Maintenance Schedule**: Planned downtime management

---

## 🚀 Getting Started:

### For Users:
1. Run atm_system.exe
2. Select preferred language
3. Enter card number when prompted
4. Input PIN (default: 1234 for test accounts)
5. Navigate through available options

### For Administrators:
1. Run atm_system.exe
2. Select "Admin Mode" at the initial screen
3. Enter admin credentials (default: admin/admin123)
4. Access the comprehensive admin dashboard

---

## 🔧 System Requirements:
- C Compiler (GCC recommended)
- Windows OS (for full functionality)
- Minimum 50MB disk space
- Terminal with ANSI color support (for enhanced UI)

---

## 🎉 Thank You!

Thank you for choosing our **ATM Management System**! This project demonstrates comprehensive banking operations with robust security features. We welcome feedback and contributions to enhance this system further.

*Developed with ❤️ in India*