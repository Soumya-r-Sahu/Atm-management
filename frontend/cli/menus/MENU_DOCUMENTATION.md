# Core Banking System Menu Structure Documentation

## Overview

This document describes the reorganization of the ATM Management System menu structure based on Core Banking System (CBS) directory organization principles. The menu system has been redesigned to provide a clear separation of concerns, better organization, and improved maintainability.

## Directory Structure

The menu system has been reorganized into the following directory structure:

```
frontend/cli/menus/
  ├── menu_types.h        # User type and menu type definitions
  ├── menu_system.h       # Function prototypes for all menus
  ├── menu_utils.h        # Utility function declarations
  ├── menu_utils.c        # Utility function implementations
  ├── main_menu.c         # Main menu implementation
  ├── customer_menu.c     # Customer menu implementation
  ├── account_menu.c      # Account management submenu
  ├── transaction_menu.c  # Transaction management submenu
  ├── beneficiary_menu.c  # Beneficiary management submenu
  ├── bill_payment_menu.c # Bill payment submenu
  ├── atm_operations_menu.c # ATM operations menu
  ├── admin_menu.c        # Admin menu implementation
  └── cbs_admin_menu.c    # CBS admin menu implementation
```

## User Types

The system defines the following user types:

- `USER_CUSTOMER` (1): Regular bank customer
- `USER_ADMIN` (2): Bank administrator
- `USER_ATM_OPERATOR` (3): ATM maintenance operator
- `USER_ADMIN_SUPER` (4): Super admin with elevated privileges

## Menu Structure

### Main Menu

The main menu serves as the entry point to the application and provides access to:

1. Customer Login
2. ATM Services
3. Admin Login
4. CBS Administration
5. Help
6. Exit

### Customer Menu Hierarchy

Customer menu is organized into logical submenus:

- Customer Menu
  - Account Menu
    - View Account Details
    - Change PIN
    - View Transaction History
    - Request Statement
  - Transaction Menu
    - Transfer Funds
    - Pay Bill
    - Check Balance Enquiry
  - Beneficiary Menu
    - View Beneficiaries
    - Add Beneficiary
    - Delete Beneficiary
  - Bill Payment Menu
    - Pay Utility Bill
    - Manage Bill Schedules

### ATM Operations Menu Hierarchy

ATM operations menu is designed for ATM maintenance staff:

- ATM Operations Menu
  - Cash Management Menu
    - Check Cash Levels
    - Cash Refill
    - Cash Reconciliation
  - Maintenance Menu
    - Perform Maintenance
    - Perform Software Update
  - Device Status Menu
    - Show Device Status
    - Show Error Log
    - Generate Transaction Report

### Admin Menu Hierarchy

Admin menu is for bank administrators:

- Admin Menu
  - User Management Menu
    - View Users
    - Add User
    - Edit User
    - Disable User
    - Reset User Password
  - System Configuration Menu
    - General Settings
    - Transaction Limits
    - Fee Configuration
    - ATM Configuration
    - System Parameters
  - Reports Menu
    - Transaction Reports
    - User Activity Reports
    - ATM Performance Reports
    - System Usage Reports
  - Security Management Menu
    - Access Control
    - Security Policies
    - System Logs
  - Audit Menu
    - User Audit Logs
    - Transaction Audit Logs
    - System Audit Logs
    - Export Audit Logs

### CBS Admin Menu Hierarchy

CBS admin menu is for system-level administration:

- CBS Admin Menu
  - Database Administration
  - System Management
  - Security Operations
  - Backup & Recovery
  - Server Management
  - Update Management
  - Audit & Compliance
  - Performance Monitoring

## Function Naming Conventions

The function naming follows these conventions:

- `run<MenuName>`: Functions that display and handle a menu (e.g., `runCustomerMenu`)
- `display<MenuName>`: Functions that only display a menu (e.g., `displayMainMenu`)
- `handle<MenuName>Choice`: Functions that handle menu choices (e.g., `handleMainMenuChoice`)

Specific action functions use descriptive verbs:
- `view<Entity>`: Display entity details (e.g., `viewAccountDetails`)
- `add<Entity>`: Add a new entity (e.g., `addBeneficiary`)
- `edit<Entity>`: Modify an existing entity (e.g., `editUser`)
- `delete<Entity>`: Remove an entity (e.g., `deleteBeneficiary`)
- `check<Action>`: Verify or query information (e.g., `checkBalanceEnquiry`)
- `manage<Entity>`: Access a management submenu (e.g., `manageBillSchedules`)

## Utility Functions

The menu system includes a comprehensive set of utility functions for:

- Screen handling (clear screen, print headers, etc.)
- User input (strings, integers, passwords, etc.)
- Formatting (currency, dates, time)
- Notification (success, error, info, warning)
- Navigation (pausing, confirming)

## Key Design Improvements

1. **Separation of Concerns**: Each menu is isolated in its own file
2. **Consistent Function Signatures**: Parameters are standardized across menus
3. **Type Safety**: Proper enums instead of defines
4. **Improved Readability**: Consistent naming conventions 
5. **Enhanced Documentation**: All functions have proper documentation
6. **Better Modularity**: Menu hierarchies are clearly defined

## Usage Examples

### Running the Main Menu
```c
#include "menus/menu_system.h"

int main() {
    runMainMenu();
    return 0;
}
```

### Accessing a Specific Menu Directly (for testing)
```c
#include "menus/menu_system.h"

int main() {
    // Access customer menu directly
    runCustomerMenu("testuser", USER_CUSTOMER);
    return 0;
}
```

## Implementation Notes

1. The menu system uses proper function prototypes in header files and implementations in C files
2. Common utility functions are shared across all menus for consistency
3. User authentication is simulated in the demo implementation
4. The menu system supports future expansions through its modular design
