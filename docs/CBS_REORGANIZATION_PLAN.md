# CBS Project Reorganization Plan

This document outlines the steps needed to reorganize the Core Banking System (CBS) project file structure for better maintainability and organization.

## 1. Database Layer Reorganization

The current structure has database-related files directly in `/backend/database/`. We should organize these by implementation type:

```
/backend/database/
├── mysql/             # MySQL-specific implementation
│   ├── include/       # MySQL DAO header files
│   └── src/           # MySQL DAO implementation files
├── file_based/        # File-based implementation
│   ├── include/       # File-based DAO header files
│   └── src/           # File-based implementation files
└── common/            # Shared database utilities
    ├── include/       # Common DAO interfaces
    └── src/           # Common DAO utilities
```

## 2. Log File Organization

Create a better structure for log files:

```
/logs/
├── transactions/      # All transaction-related logs
├── security/          # Security and authentication logs
├── errors/            # Error logs
└── audit/             # Audit trail logs
```

## 3. Binary Organization

Organize the binary directory:

```
/bin/
├── production/        # Production executables
│   ├── atm_system
│   ├── admin_system
│   └── upi_system
├── testing/           # Test binaries
│   └── [all test executables]
└── debug/             # Debug builds
```

## 4. Build Scripts Organization

Create a dedicated build folder:

```
/build/
├── makefiles/         # Specific module makefiles
├── scripts/           # Build automation scripts
└── config/            # Build configuration files
```

## 5. Implementation Plan

1. Create the new directory structure
2. Move files to their appropriate locations
3. Update include paths in source files
4. Update build scripts to reflect the new structure
5. Test the system after reorganization

## 6. Benefits

- Better separation of concerns
- Improved maintainability
- Clearer organization for new team members
- Better scalability for adding new features
