# CBS Project Initialization Script
# This script installs and configures all necessary dependencies for the CBS project
# Run this script with administrator privileges

Write-Host "CBS Project Initialization Script" -ForegroundColor Cyan
Write-Host "=================================" -ForegroundColor Cyan

# Check if running as administrator
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")
if (-not $isAdmin) {
    Write-Host "This script requires administrator privileges. Please run as administrator." -ForegroundColor Red
    Exit 1
}

# Create function to check if a command exists
function Test-CommandExists {
    param ($command)
    $oldPreference = $ErrorActionPreference
    $ErrorActionPreference = 'stop'
    try {
        if (Get-Command $command) { return $true }
    } catch {
        return $false
    } finally {
        $ErrorActionPreference = $oldPreference
    }
}

# Function to create directory if it doesn't exist
function Ensure-Directory {
    param ($path)
    if (-not (Test-Path $path)) {
        Write-Host "Creating directory: $path" -ForegroundColor Yellow
        New-Item -ItemType Directory -Path $path -Force | Out-Null
    }
}

# Function to download a file
function Download-File {
    param ($url, $outputFile)
    Write-Host "Downloading $url to $outputFile..." -ForegroundColor Yellow
    $webClient = New-Object System.Net.WebClient
    $webClient.DownloadFile($url, $outputFile)
}

# Check and install Chocolatey if needed
Write-Host "`n[1/5] Checking for Chocolatey package manager..." -ForegroundColor Green
if (-not (Test-CommandExists choco)) {
    Write-Host "Chocolatey not found. Installing..." -ForegroundColor Yellow
    Set-ExecutionPolicy Bypass -Scope Process -Force
    [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
    Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
    
    # Refresh environment variables
    $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
} else {
    Write-Host "Chocolatey is already installed." -ForegroundColor Green
}

# Check and install MinGW/GCC
Write-Host "`n[2/5] Checking for C/C++ compiler (MinGW)..." -ForegroundColor Green
if (-not (Test-CommandExists gcc)) {
    Write-Host "GCC not found. Installing MinGW..." -ForegroundColor Yellow
    choco install mingw -y
    
    # Refresh environment variables
    $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
    
    if (Test-CommandExists gcc) {
        Write-Host "MinGW/GCC installed successfully." -ForegroundColor Green
    } else {
        Write-Host "Failed to install MinGW/GCC. Please install manually." -ForegroundColor Red
    }
} else {
    $gccVersion = (gcc --version | Select-Object -First 1)
    Write-Host "GCC is already installed: $gccVersion" -ForegroundColor Green
}

# Check and install make
Write-Host "`n[3/5] Checking for Make build tools..." -ForegroundColor Green
if (-not (Test-CommandExists mingw32-make)) {
    Write-Host "mingw32-make not found. Ensuring it's installed with MinGW..." -ForegroundColor Yellow
    choco install make -y
    
    # Refresh environment variables
    $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
    
    if (Test-CommandExists mingw32-make) {
        Write-Host "mingw32-make installed successfully." -ForegroundColor Green
    } else {
        Write-Host "Failed to install mingw32-make. Please install manually." -ForegroundColor Red
    }
} else {
    $makeVersion = (mingw32-make --version | Select-Object -First 1)
    Write-Host "mingw32-make is already installed: $makeVersion" -ForegroundColor Green
}

# Check and install MySQL
Write-Host "`n[4/5] Checking for MySQL..." -ForegroundColor Green
$mysqlInstalled = $false

# Check if MySQL service exists
$mysqlService = Get-Service -Name "MySQL*" -ErrorAction SilentlyContinue
if ($mysqlService) {
    Write-Host "MySQL service found: $($mysqlService.DisplayName)" -ForegroundColor Green
    $mysqlInstalled = $true
} 
# Check if MySQL is in PATH
elseif (Test-CommandExists mysql) {
    $mysqlVersion = (mysql --version | Select-Object -First 1)
    Write-Host "MySQL client found: $mysqlVersion" -ForegroundColor Green
    $mysqlInstalled = $true
} 
# Check if XAMPP MySQL exists
elseif (Test-Path "C:\xampp\mysql\bin\mysql.exe") {
    Write-Host "XAMPP MySQL installation found." -ForegroundColor Green
    $mysqlInstalled = $true
    
    # Add XAMPP MySQL to PATH if not already there
    $xamppMysqlPath = "C:\xampp\mysql\bin"
    if ($env:Path -notlike "*$xamppMysqlPath*") {
        Write-Host "Adding XAMPP MySQL to PATH..." -ForegroundColor Yellow
        [Environment]::SetEnvironmentVariable("Path", $env:Path + ";$xamppMysqlPath", "User")
        $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
    }
}

# Install MySQL if not found
if (-not $mysqlInstalled) {
    Write-Host "MySQL not found. Would you like to install it? (Y/N)" -ForegroundColor Yellow
    $choice = Read-Host
    if ($choice -eq "Y" -or $choice -eq "y") {
        Write-Host "Installing MySQL via Chocolatey..." -ForegroundColor Yellow
        choco install mysql -y
        
        # Refresh environment variables
        $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
        
        Write-Host "MySQL installation completed. You may need to set up users and databases manually." -ForegroundColor Green
        Write-Host "You can run the setup_mysql.bat script to configure the database for this project." -ForegroundColor Green
    } else {
        Write-Host "MySQL installation skipped. You will need to install MySQL manually or use the NO_MYSQL build option." -ForegroundColor Yellow
    }
}

# Ensure project directories exist
Write-Host "`n[5/5] Ensuring project directories exist..." -ForegroundColor Green
$projectRoot = Split-Path -Path $MyInvocation.MyCommand.Path -Parent
$directories = @(
    "$projectRoot\bin", 
    "$projectRoot\data", 
    "$projectRoot\data\temp", 
    "$projectRoot\logs"
)

foreach ($dir in $directories) {
    Ensure-Directory $dir
}

# Set up project configuration
Write-Host "`nSetting up project configuration..." -ForegroundColor Green
if (-not (Test-Path "$projectRoot\data\system_config.json")) {
    Write-Host "Creating default system_config.json..." -ForegroundColor Yellow
    $defaultConfig = @"
{
    "database": {
        "type": "mysql",
        "host": "localhost",
        "port": 3306,
        "username": "root",
        "password": "",
        "database": "cbs_banking"
    },
    "logging": {
        "level": "info",
        "file": "../logs/system.log",
        "console": true
    },
    "security": {
        "encryption_key": "DEFAULT_KEY_CHANGE_ME",
        "session_timeout": 1800
    },
    "system": {
        "language": "en",
        "timezone": "UTC",
        "debug_mode": false
    }
}
"@
    $defaultConfig | Out-File -FilePath "$projectRoot\data\system_config.json" -Encoding utf8
}

# Attempt to run the setup_mysql.bat script if MySQL is installed
if ($mysqlInstalled) {
    Write-Host "`nWould you like to run the MySQL setup script now? (Y/N)" -ForegroundColor Yellow
    $choice = Read-Host
    if ($choice -eq "Y" -or $choice -eq "y") {
        Write-Host "Running MySQL setup script..." -ForegroundColor Green
        Start-Process -FilePath "$projectRoot\setup_mysql.bat" -Wait -NoNewWindow
    }
}

# Build the project
Write-Host "`nWould you like to build the project now? (Y/N)" -ForegroundColor Yellow
$choice = Read-Host
if ($choice -eq "Y" -or $choice -eq "y") {
    Write-Host "Building the project..." -ForegroundColor Green
    Push-Location $projectRoot
    mingw32-make all
    Pop-Location
}

Write-Host "`nCBS Project initialization complete!" -ForegroundColor Cyan
Write-Host "You can now build the project with 'mingw32-make all'" -ForegroundColor Green
Write-Host "To build without MySQL support, use 'mingw32-make all CFLAGS+=`"-DNO_MYSQL`"'" -ForegroundColor Green
Write-Host "To run tests, use 'mingw32-make test'" -ForegroundColor Green
