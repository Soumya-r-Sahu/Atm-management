#!/bin/bash
# File-to-MySQL Migration Script for ATM Project
# Date: May 10, 2025

# Check for mysql command
if ! command -v mysql &> /dev/null; then
    echo "MySQL client not found. Please install MySQL client."
    exit 1
fi

echo "ATM Project File to MySQL Migration Tool"
echo "========================================"
echo

# Load configuration
DB_HOST="localhost"
DB_PORT="3306"
DB_NAME="atm_management"
DB_USER="atm_user"
DB_PASS="secure_password_here"

# Prompt for database credentials
read -p "MySQL Host [$DB_HOST]: " input
DB_HOST=${input:-$DB_HOST}

read -p "MySQL Port [$DB_PORT]: " input
DB_PORT=${input:-$DB_PORT}

read -p "Database Name [$DB_NAME]: " input
DB_NAME=${input:-$DB_NAME}

read -p "Username [$DB_USER]: " input
DB_USER=${input:-$DB_USER}

read -s -p "Password: " input
echo
DB_PASS=${input:-$DB_PASS}

# Test connection
echo "Testing database connection..."
if ! mysql -h "$DB_HOST" -P "$DB_PORT" -u "$DB_USER" -p"$DB_PASS" -e "USE $DB_NAME;" &> /dev/null; then
    echo "Failed to connect to database. Please check credentials."
    exit 1
fi

echo "Connected successfully!"
echo

# Source directories for data files
DATA_DIR="data"
ATM_DATA_DIR="Atm-management/data"

# Check if data directories exist
if [ ! -d "$DATA_DIR" ] && [ ! -d "$ATM_DATA_DIR" ]; then
    echo "Error: Data directories not found."
    exit 1
fi

# Use first available data directory
if [ -d "$DATA_DIR" ]; then
    ACTIVE_DATA_DIR="$DATA_DIR"
else
    ACTIVE_DATA_DIR="$ATM_DATA_DIR"
fi

echo "Using data from: $ACTIVE_DATA_DIR"
echo

# Migrate customer data
echo "Migrating customer data..."
if [ -f "$ACTIVE_DATA_DIR/customer.txt" ]; then
    # Create temporary SQL file
    TEMP_SQL_FILE="temp_customer_import.sql"
    echo "START TRANSACTION;" > "$TEMP_SQL_FILE"
    
    # Skip header lines and process each customer
    tail -n +3 "$ACTIVE_DATA_DIR/customer.txt" | while IFS='|' read -r customer_id account_id name account_type status balance rest; do
        # Clean whitespace
        customer_id=$(echo "$customer_id" | xargs)
        account_id=$(echo "$account_id" | xargs)
        name=$(echo "$name" | xargs)
        account_type=$(echo "$account_type" | xargs)
        status=$(echo "$status" | xargs)
        balance=$(echo "$balance" | xargs)
        
        # Insert customer if not exists
        echo "INSERT IGNORE INTO cbs_customers (customer_id, name, status) 
              VALUES ('$customer_id', '$name', '$status');" >> "$TEMP_SQL_FILE"
        
        # Insert account
        echo "INSERT IGNORE INTO cbs_accounts (account_number, customer_id, account_type, status, balance) 
              VALUES ('$account_id', '$customer_id', '$account_type', '$status', $balance);" >> "$TEMP_SQL_FILE"
    done
    
    echo "COMMIT;" >> "$TEMP_SQL_FILE"
    
    # Import the SQL
    mysql -h "$DB_HOST" -P "$DB_PORT" -u "$DB_USER" -p"$DB_PASS" "$DB_NAME" < "$TEMP_SQL_FILE"
    rm "$TEMP_SQL_FILE"
    
    echo "Customer data migration completed."
else
    echo "Customer data file not found."
fi

# Migrate card data
echo "Migrating card data..."
if [ -f "$ACTIVE_DATA_DIR/card.txt" ]; then
    # Create temporary SQL file
    TEMP_SQL_FILE="temp_card_import.sql"
    echo "START TRANSACTION;" > "$TEMP_SQL_FILE"
    
    # Skip header lines and process each card
    tail -n +3 "$ACTIVE_DATA_DIR/card.txt" | while IFS='|' read -r card_id account_id card_number card_type expiry_date status pin_hash; do
        # Clean whitespace
        card_id=$(echo "$card_id" | xargs)
        account_id=$(echo "$account_id" | xargs)
        card_number=$(echo "$card_number" | xargs)
        card_type=$(echo "$card_type" | xargs)
        expiry_date=$(echo "$expiry_date" | xargs)
        status=$(echo "$status" | xargs)
        pin_hash=$(echo "$pin_hash" | xargs)
        
        # Convert status
        if [[ "$status" == "Active" ]]; then
            status="ACTIVE"
        elif [[ "$status" == "Blocked" ]]; then
            status="BLOCKED"
        fi
        
        # Insert card
        echo "INSERT IGNORE INTO cbs_cards (card_id, account_id, card_number, card_type, expiry_date, status, pin_hash) 
              VALUES ('$card_id', '$account_id', '$card_number', '$card_type', '$expiry_date', '$status', '$pin_hash');" >> "$TEMP_SQL_FILE"
    done
    
    echo "COMMIT;" >> "$TEMP_SQL_FILE"
    
    # Import the SQL
    mysql -h "$DB_HOST" -P "$DB_PORT" -u "$DB_USER" -p"$DB_PASS" "$DB_NAME" < "$TEMP_SQL_FILE"
    rm "$TEMP_SQL_FILE"
    
    echo "Card data migration completed."
else
    echo "Card data file not found."
fi

# Migrate transaction data
echo "Migrating transaction data..."
if [ -f "$ACTIVE_DATA_DIR/atm_transactions.txt" ]; then
    # Create temporary SQL file
    TEMP_SQL_FILE="temp_transaction_import.sql"
    echo "START TRANSACTION;" > "$TEMP_SQL_FILE"
    
    # Skip header lines and process each transaction
    tail -n +3 "$ACTIVE_DATA_DIR/atm_transactions.txt" | while IFS='|' read -r transaction_id timestamp card_number type amount status account_id; do
        # Clean whitespace
        transaction_id=$(echo "$transaction_id" | xargs)
        timestamp=$(echo "$timestamp" | xargs)
        card_number=$(echo "$card_number" | xargs)
        type=$(echo "$type" | xargs)
        amount=$(echo "$amount" | xargs)
        status=$(echo "$status" | xargs)
        account_id=$(echo "$account_id" | xargs)
        
        # Insert transaction
        echo "INSERT IGNORE INTO cbs_transactions (transaction_id, card_number, account_number, transaction_type, amount, status, transaction_date) 
              VALUES ('$transaction_id', '$card_number', '$account_id', '$type', $amount, '$status', '$timestamp');" >> "$TEMP_SQL_FILE"
    done
    
    echo "COMMIT;" >> "$TEMP_SQL_FILE"
    
    # Import the SQL
    mysql -h "$DB_HOST" -P "$DB_PORT" -u "$DB_USER" -p"$DB_PASS" "$DB_NAME" < "$TEMP_SQL_FILE"
    rm "$TEMP_SQL_FILE"
    
    echo "Transaction data migration completed."
else
    echo "Transaction data file not found."
fi

echo
echo "Migration completed successfully."
echo "You can now switch to USE_MYSQL=1 in the database configuration."
