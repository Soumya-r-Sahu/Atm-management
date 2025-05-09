#!/bin/bash
# MySQL Configuration Helper for ATM Project
# This script helps setup and verify MySQL for the ATM project

echo "ATM Project MySQL Configuration Helper"
echo "====================================="

# Check if MySQL service is running
echo "Checking if MySQL service is running..."
if systemctl is-active --quiet mysql || systemctl is-active --quiet mysqld; then
  echo "MySQL service is running."
else
  echo "MySQL service is not running or not installed."
  echo "Please install MySQL Server and make sure the service is running."
  echo "You can install MySQL with: sudo apt-get install mysql-server (Ubuntu/Debian)"
  echo "Or: sudo yum install mysql-server (CentOS/RHEL)"
  read -p "Press Enter to continue..."
  exit 1
fi

# Check for MySQL client
echo
echo "Checking for MySQL client..."
if command -v mysql &> /dev/null; then
  echo "MySQL client found in PATH."
else
  echo "MySQL client not found in PATH."
  echo
  echo "Please make sure MySQL is properly installed and its bin directory is in your PATH."
  echo "You can install the MySQL client with: sudo apt-get install mysql-client (Ubuntu/Debian)"
  echo "Or: sudo yum install mysql (CentOS/RHEL)"
  echo
  echo "You can continue with NO_MYSQL build option if MySQL is not available."
  read -p "Press Enter to continue..."
fi

# Configure database for ATM project
echo
read -p "Would you like to configure the ATM project database? (Y/N): " setup_db

if [[ $setup_db == "Y" || $setup_db == "y" ]]; then
  echo
  echo "Creating ATM project database..."
  
  # Check if mysql client is available
  if command -v mysql &> /dev/null; then
    # Prompt for MySQL root password
    echo "Please enter your MySQL root password:"
    read -s root_password
    
    # Create database and user
    echo "Creating database and user..."
    mysql -u root -p"$root_password" <<EOF
CREATE DATABASE IF NOT EXISTS atm_management;
CREATE USER IF NOT EXISTS 'atm_user'@'localhost' IDENTIFIED BY 'securepassword';
GRANT ALL PRIVILEGES ON atm_management.* TO 'atm_user'@'localhost';
FLUSH PRIVILEGES;
EOF
    
    if [ $? -eq 0 ]; then
      echo "Database setup successful!"
      
      # Import schema if available
      if [ -f setup/sql/schema.sql ]; then
        echo "Importing database schema..."
        mysql -u root -p"$root_password" atm_management < setup/sql/schema.sql
        echo "Schema import complete!"
      else
        echo "Schema file not found at setup/sql/schema.sql"
      fi
    else
      echo "Failed to set up database. Please check your MySQL credentials."
    fi
  else
    echo "MySQL client not found in PATH. Cannot set up database."
  fi
else
  echo "Skipping database setup."
fi

# Build options
echo
echo "ATM Project Build Options:"
echo "1. Build with MySQL support"
echo "2. Build without MySQL (use stub implementation)"
echo
read -p "Choose build option (1/2): " build_option

if [ "$build_option" == "1" ]; then
  echo
  echo "Building with MySQL support..."
  make all
elif [ "$build_option" == "2" ]; then
  echo
  echo "Building without MySQL (using stub implementation)..."
  make all CFLAGS+=-DNO_MYSQL
else
  echo "Invalid option selected."
fi

echo
echo "Configuration complete!"
read -p "Press Enter to continue..."
