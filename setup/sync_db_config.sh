#!/bin/bash
# Database configuration synchronizer for the ATM Project
# This script ensures DB settings are the same across C and PHP backends

# ANSI colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}ATM Project Database Configuration Synchronizer${NC}"
echo "===================================================="

# Define paths
C_CONFIG_PATH="include/common/database/db_config.h"
PHP_CONFIG_PATH="backend/web_backend/config/database.php"

if [ ! -f "$C_CONFIG_PATH" ]; then
    echo -e "${RED}Error: C configuration file not found at $C_CONFIG_PATH${NC}"
    exit 1
fi

if [ ! -f "$PHP_CONFIG_PATH" ]; then
    echo -e "${RED}Error: PHP configuration file not found at $PHP_CONFIG_PATH${NC}"
    exit 1
fi

# Extract DB settings from C config
echo -e "${YELLOW}Reading C database configuration...${NC}"
C_DB_HOST=$(grep -E "#define\s+DB_HOST" "$C_CONFIG_PATH" | sed -E 's/#define\s+DB_HOST\s+"(.+)"/\1/')
C_DB_USER=$(grep -E "#define\s+DB_USER" "$C_CONFIG_PATH" | sed -E 's/#define\s+DB_USER\s+"(.+)"/\1/')
C_DB_PASS=$(grep -E "#define\s+DB_PASS" "$C_CONFIG_PATH" | sed -E 's/#define\s+DB_PASS\s+"(.+)"/\1/')
C_DB_NAME=$(grep -E "#define\s+DB_NAME" "$C_CONFIG_PATH" | sed -E 's/#define\s+DB_NAME\s+"(.+)"/\1/')

# Extract DB settings from PHP config
echo -e "${YELLOW}Reading PHP database configuration...${NC}"
PHP_DB_HOST=$(grep -E "define\('DB_HOST'" "$PHP_CONFIG_PATH" | sed -E "s/define\('DB_HOST', '(.+)'\);/\1/")
PHP_DB_USER=$(grep -E "define\('DB_USER'" "$PHP_CONFIG_PATH" | sed -E "s/define\('DB_USER', '(.+)'\);/\1/")
PHP_DB_PASS=$(grep -E "define\('DB_PASS'" "$PHP_CONFIG_PATH" | sed -E "s/define\('DB_PASS', '(.+)'\);/\1/")
PHP_DB_NAME=$(grep -E "define\('DB_NAME'" "$PHP_CONFIG_PATH" | sed -E "s/define\('DB_NAME', '(.+)'\);/\1/")

# Compare settings
echo -e "${BLUE}Comparing database settings...${NC}"
SETTINGS_MATCH=true

if [ "$C_DB_HOST" != "$PHP_DB_HOST" ]; then
    echo -e "${RED}Mismatch: DB_HOST${NC}"
    echo "  C: $C_DB_HOST"
    echo "  PHP: $PHP_DB_HOST"
    SETTINGS_MATCH=false
fi

if [ "$C_DB_USER" != "$PHP_DB_USER" ]; then
    echo -e "${RED}Mismatch: DB_USER${NC}"
    echo "  C: $C_DB_USER"
    echo "  PHP: $PHP_DB_USER"
    SETTINGS_MATCH=false
fi

if [ "$C_DB_NAME" != "$PHP_DB_NAME" ]; then
    echo -e "${RED}Mismatch: DB_NAME${NC}"
    echo "  C: $C_DB_NAME"
    echo "  PHP: $PHP_DB_NAME"
    SETTINGS_MATCH=false
fi

# Password comparison (optional)
if [ "$C_DB_PASS" != "$PHP_DB_PASS" ]; then
    echo -e "${YELLOW}Note: DB_PASS values are different${NC}"
    echo "  This might be intentional for security reasons."
fi

if [ "$SETTINGS_MATCH" = true ]; then
    echo -e "${GREEN}Success: All database settings match between C and PHP backends!${NC}"
else
    echo -e "${YELLOW}Would you like to synchronize the settings? (y/n)${NC}"
    read -r SYNC_CHOICE
    
    if [ "$SYNC_CHOICE" = "y" ] || [ "$SYNC_CHOICE" = "Y" ]; then
        echo -e "${YELLOW}Which configuration should be the source of truth?${NC}"
        echo "1. C configuration"
        echo "2. PHP configuration"
        read -r SOURCE_CHOICE
        
        if [ "$SOURCE_CHOICE" = "1" ]; then
            echo -e "${BLUE}Updating PHP configuration based on C settings...${NC}"
            sed -i "s/define('DB_HOST', '.*');/define('DB_HOST', '$C_DB_HOST');/" "$PHP_CONFIG_PATH"
            sed -i "s/define('DB_USER', '.*');/define('DB_USER', '$C_DB_USER');/" "$PHP_CONFIG_PATH"
            sed -i "s/define('DB_NAME', '.*');/define('DB_NAME', '$C_DB_NAME');/" "$PHP_CONFIG_PATH"
            
            echo -e "${GREEN}PHP configuration updated successfully!${NC}"
        elif [ "$SOURCE_CHOICE" = "2" ]; then
            echo -e "${BLUE}Updating C configuration based on PHP settings...${NC}"
            sed -i "s/#define DB_HOST \".*\"/#define DB_HOST \"$PHP_DB_HOST\"/" "$C_CONFIG_PATH"
            sed -i "s/#define DB_USER \".*\"/#define DB_USER \"$PHP_DB_USER\"/" "$C_CONFIG_PATH"
            sed -i "s/#define DB_NAME \".*\"/#define DB_NAME \"$PHP_DB_NAME\"/" "$C_CONFIG_PATH"
            
            echo -e "${GREEN}C configuration updated successfully!${NC}"
        else
            echo -e "${RED}Invalid choice. No changes were made.${NC}"
        fi
    else
        echo -e "${YELLOW}No changes were made. Please manually update the configuration files to ensure consistency.${NC}"
    fi
fi

echo -e "${BLUE}Database configuration check complete.${NC}"
