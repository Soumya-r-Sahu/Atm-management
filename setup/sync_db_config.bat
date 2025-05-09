@echo off
REM Database configuration synchronizer for the ATM Project (Windows version)
REM This script ensures DB settings are the same across C and PHP backends

echo ATM Project Database Configuration Synchronizer
echo ====================================================

REM Define paths
set C_CONFIG_PATH=include\common\database\db_config.h
set PHP_CONFIG_PATH=backend\web_backend\config\database.php

if not exist "%C_CONFIG_PATH%" (
    echo Error: C configuration file not found at %C_CONFIG_PATH%
    exit /b 1
)

if not exist "%PHP_CONFIG_PATH%" (
    echo Error: PHP configuration file not found at %PHP_CONFIG_PATH%
    exit /b 1
)

echo Reading database configurations...

REM Use PowerShell to extract the settings
powershell -Command "& {
    # Read C configuration
    $cConfig = Get-Content '%C_CONFIG_PATH%'
    $cHostMatch = $cConfig | Select-String -Pattern '#define\s+DB_HOST\s+""([^""]+)""'
    $cUserMatch = $cConfig | Select-String -Pattern '#define\s+DB_USER\s+""([^""]+)""'
    $cPassMatch = $cConfig | Select-String -Pattern '#define\s+DB_PASS\s+""([^""]+)""'
    $cNameMatch = $cConfig | Select-String -Pattern '#define\s+DB_NAME\s+""([^""]+)""'
    
    $cHost = if ($cHostMatch) { $cHostMatch.Matches.Groups[1].Value } else { 'unknown' }
    $cUser = if ($cUserMatch) { $cUserMatch.Matches.Groups[1].Value } else { 'unknown' }
    $cPass = if ($cPassMatch) { $cPassMatch.Matches.Groups[1].Value } else { 'unknown' }
    $cName = if ($cNameMatch) { $cNameMatch.Matches.Groups[1].Value } else { 'unknown' }
    
    # Read PHP configuration
    $phpConfig = Get-Content '%PHP_CONFIG_PATH%'
    $phpHostMatch = $phpConfig | Select-String -Pattern ""define\('DB_HOST', '([^']+)'\);""
    $phpUserMatch = $phpConfig | Select-String -Pattern ""define\('DB_USER', '([^']+)'\);""
    $phpPassMatch = $phpConfig | Select-String -Pattern ""define\('DB_PASS', '([^']+)'\);""
    $phpNameMatch = $phpConfig | Select-String -Pattern ""define\('DB_NAME', '([^']+)'\);""
    
    $phpHost = if ($phpHostMatch) { $phpHostMatch.Matches.Groups[1].Value } else { 'unknown' }
    $phpUser = if ($phpUserMatch) { $phpUserMatch.Matches.Groups[1].Value } else { 'unknown' }
    $phpPass = if ($phpPassMatch) { $phpPassMatch.Matches.Groups[1].Value } else { 'unknown' }
    $phpName = if ($phpNameMatch) { $phpNameMatch.Matches.Groups[1].Value } else { 'unknown' }
    
    # Output to temporary file for batch to read
    @{
        CHost = $cHost
        CUser = $cUser
        CPass = $cPass
        CName = $cName
        PHPHost = $phpHost
        PHPUser = $phpUser
        PHPPass = $phpPass
        PHPName = $phpName
    } | ConvertTo-Json > 'temp_db_config.json'
}"

REM Read the JSON into environment variables
for /f "usebackq delims=" %%a in (`powershell -Command "& { Get-Content 'temp_db_config.json' | ConvertFrom-Json | ForEach-Object { $_.CHost } }"`) do set C_DB_HOST=%%a
for /f "usebackq delims=" %%a in (`powershell -Command "& { Get-Content 'temp_db_config.json' | ConvertFrom-Json | ForEach-Object { $_.CUser } }"`) do set C_DB_USER=%%a
for /f "usebackq delims=" %%a in (`powershell -Command "& { Get-Content 'temp_db_config.json' | ConvertFrom-Json | ForEach-Object { $_.CName } }"`) do set C_DB_NAME=%%a
for /f "usebackq delims=" %%a in (`powershell -Command "& { Get-Content 'temp_db_config.json' | ConvertFrom-Json | ForEach-Object { $_.PHPHost } }"`) do set PHP_DB_HOST=%%a
for /f "usebackq delims=" %%a in (`powershell -Command "& { Get-Content 'temp_db_config.json' | ConvertFrom-Json | ForEach-Object { $_.PHPUser } }"`) do set PHP_DB_USER=%%a
for /f "usebackq delims=" %%a in (`powershell -Command "& { Get-Content 'temp_db_config.json' | ConvertFrom-Json | ForEach-Object { $_.PHPName } }"`) do set PHP_DB_NAME=%%a

REM Clean up temp file
del temp_db_config.json

echo Comparing database settings...

set SETTINGS_MATCH=true

if not "%C_DB_HOST%"=="%PHP_DB_HOST%" (
    echo Mismatch: DB_HOST
    echo   C: %C_DB_HOST%
    echo   PHP: %PHP_DB_HOST%
    set SETTINGS_MATCH=false
)

if not "%C_DB_USER%"=="%PHP_DB_USER%" (
    echo Mismatch: DB_USER
    echo   C: %C_DB_USER%
    echo   PHP: %PHP_DB_USER%
    set SETTINGS_MATCH=false
)

if not "%C_DB_NAME%"=="%PHP_DB_NAME%" (
    echo Mismatch: DB_NAME
    echo   C: %C_DB_NAME%
    echo   PHP: %PHP_DB_NAME%
    set SETTINGS_MATCH=false
)

if "%SETTINGS_MATCH%"=="true" (
    echo Success: All database settings match between C and PHP backends!
) else (
    echo Would you like to synchronize the settings? (y/n)
    set /p SYNC_CHOICE=
    
    if /i "%SYNC_CHOICE%"=="y" (
        echo Which configuration should be the source of truth?
        echo 1. C configuration
        echo 2. PHP configuration
        set /p SOURCE_CHOICE=
        
        if "%SOURCE_CHOICE%"=="1" (
            echo Updating PHP configuration based on C settings...
            powershell -Command "& {
                $content = Get-Content '%PHP_CONFIG_PATH%'
                $content = $content -replace ""define\('DB_HOST', '[^']+'\);"", ""define('DB_HOST', '%C_DB_HOST%');""
                $content = $content -replace ""define\('DB_USER', '[^']+'\);"", ""define('DB_USER', '%C_DB_USER%');""
                $content = $content -replace ""define\('DB_NAME', '[^']+'\);"", ""define('DB_NAME', '%C_DB_NAME%');""
                Set-Content '%PHP_CONFIG_PATH%' $content
            }"
            echo PHP configuration updated successfully!
        ) else if "%SOURCE_CHOICE%"=="2" (
            echo Updating C configuration based on PHP settings...
            powershell -Command "& {
                $content = Get-Content '%C_CONFIG_PATH%'
                $content = $content -replace ""#define DB_HOST \""[^\""]+""\"""", ""#define DB_HOST \""%PHP_DB_HOST%\""""
                $content = $content -replace ""#define DB_USER \""[^\""]+""\"""", ""#define DB_USER \""%PHP_DB_USER%\""""
                $content = $content -replace ""#define DB_NAME \""[^\""]+""\"""", ""#define DB_NAME \""%PHP_DB_NAME%\""""
                Set-Content '%C_CONFIG_PATH%' $content
            }"
            echo C configuration updated successfully!
        ) else (
            echo Invalid choice. No changes were made.
        )
    ) else (
        echo No changes were made. Please manually update the configuration files to ensure consistency.
    )
)

echo Database configuration check complete.
