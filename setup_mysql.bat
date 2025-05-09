@echo off
REM MySQL Configuration Helper for ATM Project
REM This script helps setup and verify MySQL for the ATM project

echo ATM Project MySQL Configuration Helper
echo =====================================

REM Check if MySQL service is running
echo Checking if MySQL service is running...
sc query mysql > nul 2>&1
if %ERRORLEVEL% EQU 0 (
  echo MySQL service is running.
) else (
  echo MySQL service is not running or not installed.
  echo Please install MySQL Server and make sure the service is running.
  echo You can download MySQL from: https://dev.mysql.com/downloads/installer/
  pause
  exit /b 1
)

REM Check for MySQL client
echo.
echo Checking for MySQL client...

REM Check in PATH first
where mysql > nul 2>&1
if %ERRORLEVEL% EQU 0 (
  echo MySQL client found in PATH.
  set MYSQL_CMD=mysql
) else (
  echo MySQL client not found in PATH.
  echo Checking common locations...
  
  REM Check XAMPP location
  if exist "C:\xampp\mysql\bin\mysql.exe" (
    echo MySQL client found in XAMPP.
    set MYSQL_CMD=C:\xampp\mysql\bin\mysql
  ) else if exist "C:\Program Files\MySQL\MySQL Server 8.0\bin\mysql.exe" (
    echo MySQL client found in Program Files.
    set MYSQL_CMD=C:\Program Files\MySQL\MySQL Server 8.0\bin\mysql
  ) else (
    echo MySQL client not found in common locations.
    echo.
    echo Please make sure MySQL is properly installed.
    echo Common locations:
    echo - C:\Program Files\MySQL\MySQL Server 8.0\bin
    echo - C:\xampp\mysql\bin
    echo.
    echo You can continue with NO_MYSQL build option if MySQL is not available.
    pause
    set MYSQL_CMD=
  )
)

REM Configure database for ATM project
echo.
echo Would you like to configure the ATM project database? (Y/N)
set /p setup_db=

if /i "%setup_db%"=="Y" (
  echo.
  echo Creating ATM project database...
  
  REM Check if mysql client is available
  where mysql > nul 2>&1
  if %ERRORLEVEL% EQU 0 (
    REM Prompt for MySQL root password
    echo Please enter your MySQL root password:
    set /p root_password=
    
    REM Create database and user
    echo Creating database and user...
    echo CREATE DATABASE IF NOT EXISTS atm_management; > temp_setup.sql
    echo CREATE USER IF NOT EXISTS 'atm_user'@'localhost' IDENTIFIED BY 'securepassword'; >> temp_setup.sql
    echo GRANT ALL PRIVILEGES ON atm_management.* TO 'atm_user'@'localhost'; >> temp_setup.sql
    echo FLUSH PRIVILEGES; >> temp_setup.sql
    
    mysql -u root -p%root_password% < temp_setup.sql
    if %ERRORLEVEL% EQU 0 (
      echo Database setup successful!
      
      REM Import schema if available
      if exist setup\sql\schema.sql (
        echo Importing database schema...
        mysql -u root -p%root_password% atm_management < setup\sql\schema.sql
        echo Schema import complete!
      ) else (
        echo Schema file not found at setup\sql\schema.sql
      )
    ) else (
      echo Failed to set up database. Please check your MySQL credentials.
    )
    
    REM Clean up
    del temp_setup.sql
  ) else (
    echo MySQL client not found in PATH. Cannot set up database.
  )
) else (
  echo Skipping database setup.
)

REM Build options
echo.
echo ATM Project Build Options:
echo 1. Build with MySQL support
echo 2. Build without MySQL (use stub implementation)
echo.
echo Choose build option (1/2):
set /p build_option=

if "%build_option%"=="1" (
  echo.
  echo Building with MySQL support...
  mingw32-make all
) else if "%build_option%"=="2" (
  echo.
  echo Building without MySQL (using stub implementation)...
  mingw32-make all CFLAGS+=-DNO_MYSQL
) else (
  echo Invalid option selected.
)

echo.
echo Configuration complete!
pause
