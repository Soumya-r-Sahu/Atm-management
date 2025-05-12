@echo off
REM Core Banking System build script for Windows

echo ======================================================
echo       Core Banking System Build Script (Windows)
echo ======================================================
echo.

REM Check for MinGW/GCC
where gcc >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: GCC not found! Please install MinGW and add it to your PATH.
    echo You can download MinGW from: https://sourceforge.net/projects/mingw/
    goto :error
)

REM Check for GNU Make
where mingw32-make >nul 2>&1
if %errorLevel% neq 0 (
    echo WARNING: mingw32-make not found. Using fallback commands.
    goto :manual_build
) else (
    goto :make_build
)

:make_build
echo Building with mingw32-make...
mkdir BUILD_FILES\obj 2>nul
mkdir BIN 2>nul

echo Running make...
mingw32-make -f BUILD_FILES\Makefile

if %errorLevel% neq 0 (
    echo ERROR: Build failed with make.
    goto :error
)

goto :success

:manual_build
echo Building with manual commands...
mkdir BUILD_FILES\obj 2>nul
mkdir BIN 2>nul

echo Compiling main application...
gcc -Wall -Wextra -g -IINCLUDE -IBACKEND/c_backend/include -c APPLICATION\main.c -o BUILD_FILES\obj\main.o

echo Compiling backend modules...
gcc -Wall -Wextra -g -IINCLUDE -IBACKEND/c_backend/include -c BACKEND\c_backend\src\account.c -o BUILD_FILES\obj\backend_account.o
gcc -Wall -Wextra -g -IINCLUDE -IBACKEND/c_backend/include -c BACKEND\c_backend\src\transaction.c -o BUILD_FILES\obj\backend_transaction.o
gcc -Wall -Wextra -g -IINCLUDE -IBACKEND/c_backend/include -c BACKEND\c_backend\src\ledger.c -o BUILD_FILES\obj\backend_ledger.o
gcc -Wall -Wextra -g -IINCLUDE -IBACKEND/c_backend/include -c BACKEND\c_backend\src\audit.c -o BUILD_FILES\obj\backend_audit.o

echo Compiling database modules...
gcc -Wall -Wextra -g -IINCLUDE -IBACKEND/c_backend/include -c BACKEND\database\dao.c -o BUILD_FILES\obj\db_dao.o
gcc -Wall -Wextra -g -IINCLUDE -IBACKEND/c_backend/include -c BACKEND\database\mysql_connector.c -o BUILD_FILES\obj\db_mysql_connector.o
gcc -Wall -Wextra -g -IINCLUDE -IBACKEND/c_backend/include -c BACKEND\database\file_storage.c -o BUILD_FILES\obj\db_file_storage.o

echo Compiling web backend modules...
gcc -Wall -Wextra -g -IINCLUDE -IBACKEND/c_backend/include -c BACKEND\web_backend\rest_api.c -o BUILD_FILES\obj\web_rest_api.o
gcc -Wall -Wextra -g -IINCLUDE -IBACKEND/c_backend/include -c BACKEND\web_backend\soap_service.c -o BUILD_FILES\obj\web_soap_service.o
gcc -Wall -Wextra -g -IINCLUDE -IBACKEND/c_backend/include -c BACKEND\web_backend\api_utils.c -o BUILD_FILES\obj\web_api_utils.o

echo Compiling application modules...
gcc -Wall -Wextra -g -IINCLUDE -IBACKEND/c_backend/include -c APPLICATION\config.c -o BUILD_FILES\obj\config.o
gcc -Wall -Wextra -g -IINCLUDE -IBACKEND/c_backend/include -c APPLICATION\iso8583_parser.c -o BUILD_FILES\obj\iso8583_parser.o
gcc -Wall -Wextra -g -IINCLUDE -IBACKEND/c_backend/include -c APPLICATION\iso8583_builder.c -o BUILD_FILES\obj\iso8583_builder.o

echo Building main server executable...
gcc -o BIN\cbs_server.exe BUILD_FILES\obj\main.o BUILD_FILES\obj\config.o BUILD_FILES\obj\iso8583_parser.o BUILD_FILES\obj\iso8583_builder.o BUILD_FILES\obj\backend_account.o BUILD_FILES\obj\backend_transaction.o BUILD_FILES\obj\backend_ledger.o BUILD_FILES\obj\backend_audit.o BUILD_FILES\obj\db_dao.o BUILD_FILES\obj\db_mysql_connector.o BUILD_FILES\obj\db_file_storage.o BUILD_FILES\obj\web_rest_api.o BUILD_FILES\obj\web_soap_service.o BUILD_FILES\obj\web_api_utils.o -lm -lws2_32

if %errorLevel% neq 0 (
    echo ERROR: Build failed during manual compilation.
    goto :error
)

echo Building unit tests...
gcc -o BIN\unit_tests.exe TESTS\unit_tests.c BUILD_FILES\obj\config.o BUILD_FILES\obj\iso8583_parser.o BUILD_FILES\obj\iso8583_builder.o BUILD_FILES\obj\backend_account.o BUILD_FILES\obj\backend_transaction.o BUILD_FILES\obj\backend_ledger.o BUILD_FILES\obj\backend_audit.o BUILD_FILES\obj\db_dao.o BUILD_FILES\obj\db_mysql_connector.o BUILD_FILES\obj\db_file_storage.o BUILD_FILES\obj\web_rest_api.o BUILD_FILES\obj\web_soap_service.o BUILD_FILES\obj\web_api_utils.o -lm -lws2_32

if %errorLevel% neq 0 (
    echo WARNING: Unit tests build failed. Continuing...
)

goto :success

:error
echo.
echo Build failed. Please check the errors above.
exit /b 1

:success
echo.
echo Build completed successfully.
echo The binaries are located in the BIN directory.
exit /b 0
