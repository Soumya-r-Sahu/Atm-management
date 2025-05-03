#!/bin/bash

# ATM Management System - Database Initialization Script
# Created May 3, 2025

DB_HOST="localhost"
DB_USER="root"
DB_PASS="Admin6074"
DB_NAME="atm_management"

echo "ATM Management System - Fresh Database Setup"
echo "==================================================="

# Check if mysql client is installed
if ! command -v mysql &> /dev/null
then
    echo "MySQL client is not installed. Please install it first."
    exit 1
fi

# Create fresh database and tables
echo "Creating fresh database schema..."
mysql -h "$DB_HOST" -u "$DB_USER" -p"$DB_PASS" < setup/sql/fresh_schema.sql

if [ $? -ne 0 ]; then
    echo "Error: Failed to create database schema."
    exit 1
fi

echo "Database schema created successfully."
echo "==================================================="
echo "Database initialization completed."
echo "==================================================="