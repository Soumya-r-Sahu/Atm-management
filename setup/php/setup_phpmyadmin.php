<?php
/**
 * ATM Project Database Setup Helper for phpMyAdmin
 * This script helps with setting up the database through phpMyAdmin
 */

// Configuration - Update these values to match your phpMyAdmin setup
$config = [
    'db_host' => 'localhost',
    'db_user' => 'atm_app',
    'db_pass' => 'secure_password',
    'db_name' => 'atm_management',
    'admin_user' => 'admin@example.com',
    'admin_pass' => password_hash('admin123', PASSWORD_DEFAULT)
];

// HTML output for setup instructions
?>
<!DOCTYPE html>
<html>
<head>
    <title>ATM Project Database Setup</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 800px; margin: 0 auto; padding: 20px; }
        pre { background: #f5f5f5; padding: 15px; border-radius: 5px; overflow-x: auto; }
        .step { margin-bottom: 20px; border-left: 4px solid #0066cc; padding-left: 15px; }
        h2 { color: #0066cc; }
        .code { font-family: monospace; background: #f0f0f0; padding: 2px 4px; }
    </style>
</head>
<body>
    <h1>ATM Project Database Setup Guide for phpMyAdmin</h1>
    
    <div class="step">
        <h2>Step 1: Create Database</h2>
        <p>Open phpMyAdmin and create a new database called <span class="code"><?php echo $config['db_name']; ?></span></p>
        <ol>
            <li>Click on "Databases" in the top menu</li>
            <li>Enter "<?php echo $config['db_name']; ?>" in the "Create database" field</li>
            <li>Select "utf8mb4_unicode_ci" as the collation</li>
            <li>Click "Create"</li>
        </ol>
    </div>
    
    <div class="step">
        <h2>Step 2: Create User</h2>
        <p>Create a database user with the proper privileges</p>
        <ol>
            <li>Click on "User accounts" in the top menu</li>
            <li>Click "Add user account"</li>
            <li>Enter username: <span class="code"><?php echo $config['db_user']; ?></span></li>
            <li>Set password method to "Generate" or enter your own password</li>
            <li>Under "Database for user account", select "Grant all privileges on database <?php echo $config['db_name']; ?>"</li>
            <li>Click "Go" at the bottom</li>
        </ol>
    </div>
    
    <div class="step">
        <h2>Step 3: Import Schema</h2>
        <p>Import the database schema</p>
        <ol>
            <li>Select the <span class="code"><?php echo $config['db_name']; ?></span> database from the left panel</li>
            <li>Click on "Import" in the top menu</li>
            <li>Click "Browse your computer" and select the <span class="code">Database_schema.sql</span> file from the <span class="code">setup/sql</span> directory</li>
            <li>Click "Go" at the bottom</li>
        </ol>
    </div>
    
    <div class="step">
        <h2>Step 4: Update Configuration</h2>
        <p>Update the ATM project database configuration in <span class="code">include/common/database/db_config.h</span> with your phpMyAdmin settings:</p>
        <pre>
// Database connection settings
#define DB_HOST "<?php echo $config['db_host']; ?>"
#define DB_USER "<?php echo $config['db_user']; ?>"
#define DB_PASS "your_actual_password_here"
#define DB_NAME "<?php echo $config['db_name']; ?>"
        </pre>
        <p>And update the PHP connection settings in <span class="code">backend/web_backend/config/database.php</span> if it exists.</p>
    </div>
    
    <div class="step">
        <h2>Step 5: Verify Connection</h2>
        <p>Run the connection test script:</p>
        <pre>bin/test_db_interface</pre>
        <p>This should connect to your database and verify that the connection is working properly.</p>
    </div>
    
    <div class="step">
        <h2>Database Config Check</h2>
        <p>Your current configuration is:</p>
        <ul>
            <li>Host: <?php echo $config['db_host']; ?></li>
            <li>Database Name: <?php echo $config['db_name']; ?></li>
            <li>Username: <?php echo $config['db_user']; ?></li>
        </ul>
        <p>Make sure these match your phpMyAdmin settings.</p>
    </div>
</body>
</html>
