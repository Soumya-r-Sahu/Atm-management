# ATM Management System

## Overview
This project is a comprehensive ATM Management System with a clear separation between frontend and backend components. The backend is implemented in C and SQL for core ATM operations, while the frontend consists of a CLI interface, web interface using HTML, CSS, and PHP, as well as a mobile application built with Flutter.

## Project Structure
```
Atm-management/
├── backend/
│   ├── c_backend/         # C implementation for core ATM operations
│   │   ├── include/       # Header files
│   │   │   ├── admin/     # Admin-related headers
│   │   │   ├── atm/       # ATM-related headers
│   │   │   ├── common/    # Common utility headers
│   │   │   ├── core/      # Core system headers
│   │   │   └── upi/       # UPI transaction headers
│   │   ├── src/           # Source files
│   │   │   ├── admin/     # Admin implementation
│   │   │   ├── atm/       # ATM implementation
│   │   │   ├── common/    # Common utilities
│   │   │   ├── netbanking/# Netbanking implementation
│   │   │   ├── transaction/# Transaction processing
│   │   │   └── upi_transaction/# UPI implementation
│   │   ├── data/          # Data files
│   │   └── logs/          # Log files
│   ├── database/          # Database models and utilities
│   └── web_backend/       # PHP backend for web interface
│       ├── api/           # RESTful API endpoints
│       ├── config/        # Configuration files
│       ├── includes/      # PHP includes
│       └── models/        # Database models
├── frontend/
│   ├── cli/               # Command-line interface
│   │   ├── include/       # CLI header files
│   │   └── src/           # CLI source files
│   ├── web/               # Web frontend (HTML, CSS, PHP)
│   │   ├── assets/        # Static assets
│   │   ├── pages/         # PHP pages
│   │   └── templates/     # Reusable templates
│   └── mobile/            # Mobile frontend (Flutter)
│       └── flutter_app/   # Flutter application
├── bin/                   # Compiled executables
├── data/                  # Data files
│   └── temp/              # Temporary files
├── docs/                  # Documentation
├── logs/                  # Log files
├── setup/                 # Setup scripts and database schema
├── .gitignore            # Git ignore file
└── Makefile              # Build configuration
```

## Database Schema
The system uses a single MySQL database with the following key tables:
- BranchInformation: Information about bank branches
- Customers: Customer details
- Accounts: Account information
- Cards: Card details
- AdminUsers: Admin user information
- ATMMachines: ATM machine details
- Transactions: Transaction records
- BillPayments: Bill payment records
- SecurityLogs: Security audit logs

## Features
- ATM Operations:
  - Cash withdrawals
  - Cash deposits
  - Fund transfers
  - Balance inquiries
  - Mini statements
  - PIN changes
  - Bill payments
- Admin Management:
  - User management
  - ATM management
  - Transaction monitoring
  - System configuration
- Web Interface:
  - Customer portal
  - Admin dashboard
  - Transaction history
  - Account management
- Mobile Application:
  - Mobile banking
  - UPI transactions
  - Account management
  - Transaction history

## Setup Instructions

### Prerequisites
- MySQL Server (version 5.7 or higher)
- MySQL C Connector (libmysqlclient-dev)
- GCC Compiler
- PHP 7.4 or higher
- Flutter SDK
- Web server (Apache/Nginx)

### Database Setup
1. Create the database:
   ```bash
   mysql -u root -p
   ```
   ```sql
   CREATE DATABASE atm_management;
   USE atm_management;
   ```

2. Run the database schema script:
   ```bash
   mysql -u root -p atm_management < setup/database_schema.sql
   ```

### C Backend Setup
1. Configure database connection:
   Edit `backend/c_backend/include/common/db_config.h` with your MySQL credentials.

2. Compile the C backend:
   ```bash
   make clean
   make
   ```

### Web Backend Setup
1. Configure database connection:
   Edit `backend/web_backend/config/config.php` with your MySQL credentials.

2. Set up the web server:
   Configure your web server to point to the `frontend/web` directory.

### Mobile App Setup
1. Set up Flutter:
   ```bash
   cd frontend/mobile/flutter_app
   flutter pub get
   ```

2. Configure the app:
   Edit `frontend/mobile/flutter_app/lib/config/config.dart` with your API endpoints.

## Running the Application
- C Backend:
  ```bash
  cd bin
  ./atm_system
  ./admin_system
  ./upi_system
  ```

- CLI Frontend:
  ```bash
  cd bin
  ./atm_cli
  ```

- Web Frontend:
  Access through your web server, e.g., http://localhost/atm-management/

- Mobile App:
  ```bash
  cd frontend/mobile/flutter_app
  flutter run
  ```

## Security Considerations
- All passwords are securely hashed
- PIN numbers are encrypted
- Transactions are logged for audit purposes
- Session management for web and mobile interfaces
- Input validation to prevent SQL injection

## Future Enhancements
- Multi-factor authentication
- SMS notifications for transactions
- Advanced reporting and analytics
- Integration with other banking systems
- Biometric authentication

## Contributors
- [Your Name]

## License
[Your License]
