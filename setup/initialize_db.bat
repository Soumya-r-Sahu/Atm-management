@echo off

REM ATM Management System - Database Initialization Script
REM Created May 9, 2025

set DB_HOST=localhost
set DB_USER=root
set DB_PASS=Admin6074
set DB_NAME=atm_management

echo ATM Management System - Fresh Database Setup
echo ===================================================

REM Check if mysql client is installed
where mysql >nul 2>nul
if %errorlevel% neq 0 (
    echo MySQL client is not installed. Please install it first.
    exit /b 1
)

REM Create fresh database and tables
echo Creating fresh database schema...
mysql -h %DB_HOST% -u %DB_USER% -p%DB_PASS% < setup\sql\fresh_schema.sql

if %errorlevel% neq 0 (
    echo Error: Failed to create database schema.
    exit /b 1
)

echo Database schema created successfully.
echo ===================================================
echo Database initialization completed.
echo ===================================================