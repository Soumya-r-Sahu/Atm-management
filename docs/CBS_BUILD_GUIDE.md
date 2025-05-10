# CBS Project - Build and Setup Guide

This guide provides step-by-step instructions for setting up, building, and running the Core Banking System project.

## Prerequisites

### Required Software

- **C Compiler**: GCC 7.0+ or equivalent
- **Build Tools**: Make (GNU Make 3.8+)
- **Database**: MySQL 5.7+ or MariaDB 10.3+
- **Libraries**:
  - MySQL client libraries
  - OpenSSL 1.1.1+ development libraries
  - pthread library (not required for Windows simplified builds)

### Windows-Specific Requirements

- MinGW or MinGW-w64 with GCC
- MySQL for Windows
- Windows build batch files included in the project

## Directory Setup

The project uses the following directory structure:

```
CBS-project/
├── application/        - Application-specific code
├── backend/            - Backend implementations
│   ├── c_backend/      - C implementation of backend
│   ├── database/       - Database abstraction layer
│   └── web_backend/    - Web service backend
├── bin/                - Compiled binaries
├── data/               - Data files for file-based storage
├── docs/               - Documentation
├── include/            - Header files
│   ├── admin/          - Admin module headers
│   ├── atm/            - ATM module headers
│   ├── common/         - Common utility headers
│   └── tests/          - Test framework headers
├── logs/               - Log files
├── setup/              - Setup scripts
│   └── sql/           - Database setup SQL
└── tests/              - Test files
```

## Database Setup

### MySQL/MariaDB Setup

1. **Install MySQL/MariaDB** for your platform

2. **Create the database and user**:

   ```sql
   CREATE DATABASE atm_management;
   CREATE USER 'atm_user'@'localhost' IDENTIFIED BY 'securepassword';
   GRANT ALL PRIVILEGES ON atm_management.* TO 'atm_user'@'localhost';
   FLUSH PRIVILEGES;
   ```

3. **Run the setup script**:

   ```bash
   # Linux/macOS
   ./setup_mysql.sh
   
   # Windows
   setup_mysql.bat
   ```

   Or manually:

   ```bash
   mysql -u atm_user -p atm_management < setup/sql/create_tables.sql
   mysql -u atm_user -p atm_management < setup/sql/initial_data.sql
   ```

### File-based Database Setup (alternative)

For testing without MySQL:

```bash
# Linux/macOS
make init_data_files

# Windows
bin\init_data_files.exe
```

## Building the Project

### Standard Build (Linux/macOS/Unix)

1. **Full build**:

   ```bash
   make all
   ```

2. **Specific components**:

   ```bash
   make atm_system
   make admin_system
   make upi_system
   ```

3. **Clean and rebuild**:

   ```bash
   make clean
   make all
   ```

### Windows Build

1. **Using the provided batch file**:

   ```powershell
   .\build.bat
   ```

2. **For specific components**:

   ```powershell
   .\build.bat atm_system
   .\build.bat admin_system
   ```

### Test Build

```bash
# Linux/macOS
make test

# Windows
.\build.bat test
```

## Configuration

### Database Configuration

Edit the database configuration in `include/common/database/db_unified_config.h`:

```c
#define DB_HOST "localhost"
#define DB_USER "atm_user"
#define DB_PASS "securepassword"
#define DB_NAME "atm_management"
#define DB_PORT 3306
```

Alternatively, use the configuration tools:

```bash
# Linux/macOS
./setup/sync_db_config.sh

# Windows
setup\sync_db_config.bat
```

### System Configuration

Edit the system configuration in `data/system_config.txt`:

```
# ATM Configuration
MAX_WITHDRAWAL_AMOUNT=10000
DAILY_WITHDRAWAL_LIMIT=20000
SESSION_TIMEOUT_SECONDS=120

# Logging Configuration
LOG_LEVEL=INFO
LOG_RETENTION_DAYS=30
```

## Running the Applications

### ATM System

```bash
# Linux/macOS
bin/atm_system

# Windows
bin\atm_system.exe
```

### Admin System

```bash
# Linux/macOS
bin/admin_system

# Windows
bin\admin_system.exe
```

Default admin credentials:
- Username: `admin`
- Password: `admin123`

### UPI System

```bash
# Linux/macOS
bin/upi_system

# Windows
bin\upi_system.exe
```

## Testing

### Running Tests

```bash
# All tests
make test

# Specific test
bin/test_atm_db
bin/test_admin_operations_dao
```

### Test Data

Test card data is provided in `data/test_cards.txt`:

```
Card Number: 1234567890
PIN: 1234
Balance: 1000.00
```

## Troubleshooting

### Common Build Issues

1. **Missing Libraries**:
   
   ```
   error: mysql.h: No such file or directory
   ```
   
   Solution: Install MySQL development libraries:
   
   ```bash
   # Debian/Ubuntu
   sudo apt-get install libmysqlclient-dev
   
   # Red Hat/Fedora
   sudo dnf install mysql-devel
   
   # Windows
   # Download and install MySQL C API from dev.mysql.com
   ```

2. **Compiler Version Issues**:
   
   ```
   error: unknown type name 'alignas'
   ```
   
   Solution: Ensure you're using a C11-compatible compiler:
   
   ```bash
   gcc --version  # Should be 7.0+
   ```

3. **Failed Database Connection**:

   Check connection parameters in `db_unified_config.h` and verify the MySQL service is running:
   
   ```bash
   # Linux/macOS
   service mysql status
   
   # Windows
   sc query mysql
   ```

### Log File Issues

If logs aren't appearing:

1. Check that `logs` directory exists and is writable
2. Verify logger initialization in code
3. Check log level settings

## Security Notes

1. **Change Default Passwords**:
   - Admin user: Use the admin interface to change the default password
   - Database user: Update MySQL user password and configuration file

2. **Log File Security**:
   - Ensure log directory permissions are restricted
   - Consider encrypting sensitive logs
   - Set up log rotation to prevent disk space issues

## Performance Optimization

1. **Database Connection Pool**:
   
   Adjust connection pool settings in `db_connection_pool.h`:
   
   ```c
   .min_connections = 5,
   .max_connections = 20,
   .connection_timeout = 30
   ```

2. **Log Level in Production**:
   
   Set appropriate log level for production:
   
   ```c
   cbs_setLogLevel(CBS_LOG_LEVEL_INFO);  // Debug off in production
   ```

## Additional Resources

- Complete documentation is available in `docs/`
- Example code is provided in `examples/`
- Test cases demonstrate proper usage patterns in `tests/`

---

## Navigation

- [Documentation Index](./README.md)
- [Project Documentation](./CBS_PROJECT_DOCUMENTATION.md)
- [Function Reference](./CBS_FUNCTION_REFERENCE.md)
- [Logging Guide](./CBS_LOGGING_GUIDE.md)
- [Testing Plan](./CBS_TESTING_PLAN.md)
