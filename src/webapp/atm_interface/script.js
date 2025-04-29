// SBI ATM Interface Functionality

// Global state variables
let currentScreen = 'insert-card-screen';
let pin = '';
let pinAttempts = 0;
let currentAmount = '';
let transactionType = '';
let sessionData = {
    cardNumber: '**** **** **** 1234',
    accountNumber: '123456789012',
    balance: 25000.00,
    name: 'John Doe'
};

// Initialize when document is loaded
document.addEventListener('DOMContentLoaded', function() {
    // Update date and time
    updateDateTime();
    
    // Set up event listeners
    setupEventListeners();
    
    // Show insert card screen by default (already visible)
    // startCardAnimation();
});

// Update the date and time display
function updateDateTime() {
    const now = new Date();
    const dateOptions = { day: '2-digit', month: '2-digit', year: 'numeric' };
    const timeOptions = { hour: '2-digit', minute: '2-digit', second: '2-digit', hour12: true };
    
    const dateStr = now.toLocaleDateString('en-IN', dateOptions).replace(/\//g, '/');
    const timeStr = now.toLocaleTimeString('en-IN', timeOptions);
    
    document.getElementById('date-time').textContent = `Date: ${dateStr} Time: ${timeStr}`;
    
    // Update every second
    setTimeout(updateDateTime, 1000);
}

// Setup all event listeners
function setupEventListeners() {
    // Insert card button
    document.getElementById('insert-card-btn').addEventListener('click', function() {
        showScreen('pin-screen');
        document.getElementById('pin').value = '';
    });
    
    // Number buttons on keypad
    document.querySelectorAll('.number-btn').forEach(button => {
        button.addEventListener('click', function() {
            const number = this.textContent;
            handleNumberInput(number);
        });
    });
    
    // Function buttons on keypad
    document.querySelector('.cancel-btn').addEventListener('click', handleCancel);
    document.querySelector('.clear-btn').addEventListener('click', handleClear);
    document.querySelector('.enter-btn').addEventListener('click', handleEnter);
    
    // Menu options
    document.querySelectorAll('.menu-item').forEach(item => {
        item.addEventListener('click', function() {
            const action = this.getAttribute('data-action');
            const amount = this.getAttribute('data-amount');
            
            if (action) {
                handleMenuAction(action);
            } else if (amount) {
                handleFastCash(amount);
            }
        });
    });
    
    // Transaction confirmation buttons
    document.querySelectorAll('[data-action="cancel"]').forEach(btn => {
        btn.addEventListener('click', () => showScreen('main-menu'));
    });
    
    document.querySelectorAll('[data-action="confirm"]').forEach(btn => {
        btn.addEventListener('click', processTransaction);
    });
    
    // Receipt options
    document.querySelector('[data-action="no-receipt"]').addEventListener('click', exitATM);
    document.querySelector('[data-action="print-receipt"]').addEventListener('click', printReceipt);
    
    // Back to main menu from balance or mini statement
    document.querySelectorAll('[data-action="back-to-main"]').forEach(btn => {
        btn.addEventListener('click', () => showScreen('main-menu'));
    });
    
    // Try again button on failure screen
    document.querySelector('[data-action="try-again"]').addEventListener('click', function() {
        // Go back to the previous transaction screen
        if (transactionType === 'withdrawal') {
            showScreen('withdrawal');
        } else if (transactionType === 'fast-cash') {
            showScreen('fast-cash');
        } else {
            showScreen('main-menu');
        }
    });
    
    document.querySelector('[data-action="main-menu"]').addEventListener('click', () => showScreen('main-menu'));
    
    // New transaction button on exit screen
    document.getElementById('restart-btn').addEventListener('click', resetATM);
}

// Handle number input for PIN or amount
function handleNumberInput(number) {
    if (currentScreen === 'pin-screen') {
        if (pin.length < 4) {
            pin += number;
            document.getElementById('pin').value = '*'.repeat(pin.length);
        }
    } else if (currentScreen === 'withdrawal') {
        currentAmount += number;
        document.getElementById('amount').value = formatCurrency(currentAmount);
    }
}

// Format number as currency
function formatCurrency(amount) {
    if (!amount) return '';
    
    // Add thousand separators
    return '₹ ' + parseInt(amount).toLocaleString('en-IN');
}

// Handle cancel button press
function handleCancel() {
    if (currentScreen === 'pin-screen') {
        // Return to insert card screen
        resetATM();
    } else {
        // Return to main menu from any other screen
        showScreen('main-menu');
    }
}

// Handle clear button press
function handleClear() {
    if (currentScreen === 'pin-screen') {
        // Clear PIN
        pin = '';
        document.getElementById('pin').value = '';
    } else if (currentScreen === 'withdrawal') {
        // Clear amount
        currentAmount = '';
        document.getElementById('amount').value = '';
    }
}

// Handle enter button press
function handleEnter() {
    if (currentScreen === 'pin-screen') {
        validatePIN();
    } else if (currentScreen === 'withdrawal') {
        processTransaction();
    }
}

// Validate PIN
function validatePIN() {
    // For demo, use 1234 as the PIN
    if (pin === '1234') {
        showScreen('main-menu');
        document.getElementById('pin-message').textContent = '';
        pinAttempts = 0;
    } else {
        pinAttempts++;
        
        if (pinAttempts >= 3) {
            // Block card after 3 attempts
            document.getElementById('pin-message').textContent = 'PIN incorrect. Card blocked.';
            setTimeout(resetATM, 3000);
        } else {
            document.getElementById('pin-message').textContent = 
                `PIN incorrect. ${3 - pinAttempts} attempts remaining.`;
            pin = '';
            document.getElementById('pin').value = '';
        }
    }
}

// Handle menu item selection
function handleMenuAction(action) {
    transactionType = action;
    
    switch(action) {
        case 'balance':
            showScreen('balance-display');
            // Update balance display with session data
            document.getElementById('account-number').textContent = 
                sessionData.cardNumber;
            document.getElementById('available-balance').textContent = 
                `₹ ${sessionData.balance.toLocaleString('en-IN')}.00`;
            document.getElementById('ledger-balance').textContent = 
                `₹ ${sessionData.balance.toLocaleString('en-IN')}.00`;
            break;
            
        case 'withdrawal':
            showScreen('withdrawal');
            currentAmount = '';
            document.getElementById('amount').value = '';
            break;
            
        case 'fast-cash':
            showScreen('fast-cash');
            break;
            
        case 'mini-statement':
            showScreen('mini-statement');
            break;
            
        case 'back':
            showScreen('main-menu');
            break;
            
        default:
            // For demo, just show "not implemented" for other options
            alert('This feature is not implemented in the demo.');
            break;
    }
}

// Handle fast cash selection
function handleFastCash(amount) {
    transactionType = 'fast-cash';
    currentAmount = amount;
    
    // Show processing screen
    showScreen('processing');
    
    // Simulate processing delay
    setTimeout(function() {
        // Check if amount is valid (for demo, allow withdrawals up to 10000)
        if (parseInt(amount) <= 10000) {
            // Show success screen
            document.getElementById('success-details').textContent = 
                `You have withdrawn ${formatCurrency(amount)}.`;
            showScreen('transaction-success');
        } else {
            // Show failure screen for amounts over the demo limit
            document.getElementById('error-details').textContent = 
                'Transaction declined. Withdrawal limit exceeded.';
            showScreen('transaction-failed');
        }
    }, 2000);
}

// Process a transaction
function processTransaction() {
    if (currentScreen === 'withdrawal') {
        // Validate amount
        if (!currentAmount || parseInt(currentAmount) < 100 || parseInt(currentAmount) % 100 !== 0) {
            alert('Please enter a valid amount in multiples of 100.');
            return;
        }
    }
    
    // Show processing screen
    showScreen('processing');
    
    // Simulate processing delay
    setTimeout(function() {
        // For demonstration, approve withdrawals up to 10000
        if (parseInt(currentAmount) <= 10000) {
            // Show success screen
            document.getElementById('success-details').textContent = 
                `You have withdrawn ${formatCurrency(currentAmount)}.`;
            showScreen('transaction-success');
        } else {
            // Show failure screen
            document.getElementById('error-details').textContent = 
                'Transaction declined. Withdrawal limit exceeded.';
            showScreen('transaction-failed');
        }
    }, 2000);
}

// Print a receipt
function printReceipt() {
    // Simulate printing receipt
    showScreen('exit-screen');
}

// Exit the ATM interface
function exitATM() {
    showScreen('exit-screen');
}

// Reset the ATM to initial state
function resetATM() {
    pin = '';
    pinAttempts = 0;
    currentAmount = '';
    transactionType = '';
    showScreen('insert-card-screen');
}

// Show a specific screen and hide others
function showScreen(screenId) {
    // Hide all screens
    document.querySelectorAll('.screen-content').forEach(screen => {
        screen.classList.add('hidden');
    });
    
    // Show the requested screen
    document.getElementById(screenId).classList.remove('hidden');
    
    // Update current screen tracker
    currentScreen = screenId;
}

// Function to simulate card animation
function startCardAnimation() {
    const card = document.querySelector('.card');
    card.style.animation = 'cardInsert 2s ease-in-out infinite';
}

// Function to stop card animation
function stopCardAnimation() {
    const card = document.querySelector('.card');
    card.style.animation = 'none';
}