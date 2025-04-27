#ifndef ADMIN_MENU_H
#define ADMIN_MENU_H

// Admin menu display function
void displayAdminMenu();

// Admin menu operations
void displayDashboard();
void manageCash();
void manageTransactions();
void manageUsers();
void atmMaintenance();
void securityManagement();
void manageSettings();
void manageSystemConfigurations(); // Add this new function
void manageNotifications();
void viewAuditLogs();

// Helper functions
void clearInputBuffer();
void clearScreen();

#endif // ADMIN_MENU_H