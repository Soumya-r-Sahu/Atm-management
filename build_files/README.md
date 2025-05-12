# Core Banking System (CBS) Build Files

This directory contains the build system files for the Core Banking System project.

## Contents

- `Makefile`: The main makefile for building the project on Linux and Windows (with MinGW)
- `build.bat`: Windows batch script for building the project without MinGW Make
- `obj/`: Directory for object files (created during build process)

## Build Requirements

### Windows
- MinGW GCC compiler
- GNU Make for Windows (optional, fallback to manual commands in build.bat)
- MySQL client libraries
- libcurl development libraries

### Linux
- GCC compiler
- GNU Make
- MySQL client libraries (`libmysqlclient-dev`)
- libcurl4-openssl-dev

## Build Instructions

### Windows

1. Using build.bat (easiest method):
   ```
   cd file_system
   BUILD_FILES\build.bat
   ```

2. Using MinGW Make:
   ```
   cd file_system
   mingw32-make -f BUILD_FILES\Makefile
   ```

### Linux

1. Using Make:
   ```
   cd file_system
   make -f BUILD_FILES/Makefile
   ```

## Build Targets

- `cbs_server`: Main server application
- `api_server`: API server for handling web requests
- `atm_client`: ATM client interface
- `admin_dashboard`: Administration dashboard interface
- `unit_tests`: Unit test suite
- `integration_tests`: Integration test suite
- `test_iso8583`: ISO 8583 message format testing tool
- `test_transaction`: Transaction processing testing tool
- `test_api_request`: API request testing tool

## Build Options

You can build specific targets using:

```
make -f BUILD_FILES/Makefile <target>
```

For example, to build only the API server:

```
make -f BUILD_FILES/Makefile api_server
```

To clean all build artifacts:

```
make -f BUILD_FILES/Makefile clean
```

## Customization

You may need to adjust the paths in the Makefile to match your system configuration:

- `MYSQL_CFLAGS`: Include path for MySQL
- `MYSQL_LIBS`: Library path for MySQL
- `CURL_CFLAGS`: Include path for libcurl
- `CURL_LIBS`: Library path for libcurl

## Troubleshooting

If you encounter build issues:

1. Ensure all dependencies are installed
2. Check that include paths are correctly set
3. Verify library paths
4. Make sure all source files exist in their expected locations

For detailed help, run:

```
make -f BUILD_FILES/Makefile help
```
