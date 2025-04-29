// ATM Admin Panel Functionality

// Initialize when document is loaded
document.addEventListener('DOMContentLoaded', function() {
    // Set up navigation
    setupNavigation();
    
    // Set up cash management tabs
    setupCashManagement();
    
    // Initialize charts
    initializeCharts();
    
    // Set up form handlers
    setupFormHandlers();
    
    // Set up modal handlers
    setupModalHandlers();
    
    // Initialize date and time
    updateDateTime();
});

// Set up sidebar navigation
function setupNavigation() {
    const navItems = document.querySelectorAll('.nav-item');
    const pageContents = document.querySelectorAll('.page-content');
    
    navItems.forEach(item => {
        if (item.id === 'logout-btn') {
            item.addEventListener('click', handleLogout);
        } else {
            item.addEventListener('click', function() {
                const targetPage = this.getAttribute('data-page');
                
                // Update active nav item
                navItems.forEach(nav => nav.classList.remove('active'));
                this.classList.add('active');
                
                // Show the selected page content
                pageContents.forEach(page => {
                    if (page.id === targetPage) {
                        page.classList.remove('hidden');
                    } else {
                        page.classList.add('hidden');
                    }
                });
                
                // Log navigation action with API
                logAction(`Navigated to ${targetPage.replace('-', ' ')}`);
            });
        }
    });
    
    // Toggle menu button
    const toggleMenu = document.querySelector('.toggle-menu');
    const sidebar = document.querySelector('.sidebar');
    const mainContent = document.querySelector('.main-content');
    
    toggleMenu.addEventListener('click', function() {
        sidebar.classList.toggle('collapsed');
        mainContent.classList.toggle('expanded');
    });
}

// Set up cash management tabs
function setupCashManagement() {
    const viewCashBtn = document.getElementById('view-cash-levels');
    const refillCashBtn = document.getElementById('refill-cash');
    const withdrawalReportsBtn = document.getElementById('withdrawal-reports');
    const virtualWithdrawalsBtn = document.getElementById('virtual-withdrawals');
    
    const cashSections = document.querySelectorAll('.cash-section');
    
    // Show cash levels section by default
    document.getElementById('cash-levels-section').classList.remove('hidden');
    
    // View Cash Levels tab
    viewCashBtn.addEventListener('click', function() {
        showCashSection('cash-levels-section');
    });
    
    // Refill Cash tab
    refillCashBtn.addEventListener('click', function() {
        showCashSection('refill-cash-section');
    });
    
    // Withdrawal Reports tab
    withdrawalReportsBtn.addEventListener('click', function() {
        showCashSection('withdrawal-reports-section');
    });
    
    // Virtual Withdrawals tab
    virtualWithdrawalsBtn.addEventListener('click', function() {
        showCashSection('virtual-withdrawals-section');
    });
    
    // Helper function to show the selected cash section
    function showCashSection(sectionId) {
        cashSections.forEach(section => {
            if (section.id === sectionId) {
                section.classList.remove('hidden');
            } else {
                section.classList.add('hidden');
            }
        });
    }
    
    // Date range filter for reports
    const dateRange = document.getElementById('date-range');
    const customDateRange = document.getElementById('custom-date-range');
    
    if (dateRange) {
        dateRange.addEventListener('change', function() {
            if (this.value === 'custom') {
                customDateRange.style.display = 'flex';
            } else {
                customDateRange.style.display = 'none';
            }
        });
    }
    
    // Handle refill form cancel button
    const cancelRefillBtn = document.getElementById('cancel-refill');
    if (cancelRefillBtn) {
        cancelRefillBtn.addEventListener('click', function() {
            showCashSection('cash-levels-section');
        });
    }
    
    // Handle refill form submission
    const refillForm = document.getElementById('refill-form');
    if (refillForm) {
        refillForm.addEventListener('submit', function(event) {
            event.preventDefault();
            
            // Get form values
            const refillDate = document.getElementById('refill-date').value;
            const refillTime = document.getElementById('refill-time').value;
            const hundredNotes = parseInt(document.getElementById('hundred-notes').value) || 0;
            const twoHundredNotes = parseInt(document.getElementById('two-hundred-notes').value) || 0;
            const fiveHundredNotes = parseInt(document.getElementById('five-hundred-notes').value) || 0;
            const twoThousandNotes = parseInt(document.getElementById('two-thousand-notes').value) || 0;
            
            // Calculate total amount
            const totalAmount = 
                (hundredNotes * 100) + 
                (twoHundredNotes * 200) + 
                (fiveHundredNotes * 500) + 
                (twoThousandNotes * 2000);
            
            // Show success modal with refill details
            document.getElementById('success-message').textContent = 
                `Cash refill request submitted. Total amount: ₹${totalAmount.toLocaleString('en-IN')}`;
            
            showModal('success-modal');
            
            // Log refill action with API
            logAction(`Submitted cash refill request for ₹${totalAmount.toLocaleString('en-IN')}`);
            
            // Reset form
            refillForm.reset();
            
            // Show cash levels section after successful submission
            setTimeout(() => {
                showCashSection('cash-levels-section');
            }, 2000);
        });
    }
}

// Initialize charts for dashboard
function initializeCharts() {
    // Transaction volume chart
    const transactionCtx = document.getElementById('transaction-chart');
    if (transactionCtx) {
        new Chart(transactionCtx, {
            type: 'bar',
            data: {
                labels: ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'],
                datasets: [{
                    label: 'Withdrawals',
                    data: [65, 59, 80, 81, 56, 55, 40],
                    backgroundColor: 'rgba(231, 76, 60, 0.7)',
                }, {
                    label: 'Deposits',
                    data: [28, 48, 40, 19, 86, 27, 90],
                    backgroundColor: 'rgba(39, 174, 96, 0.7)',
                }, {
                    label: 'Balance Inquiries',
                    data: [45, 25, 32, 67, 49, 72, 51],
                    backgroundColor: 'rgba(52, 152, 219, 0.7)',
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    legend: {
                        position: 'top',
                    },
                    title: {
                        display: true,
                        text: 'Weekly Transaction Volume'
                    }
                }
            }
        });
    }
    
    // Hourly chart for virtual withdrawals
    const hourlyCtx = document.getElementById('hourly-chart');
    if (hourlyCtx) {
        new Chart(hourlyCtx, {
            type: 'line',
            data: {
                labels: ['9AM', '10AM', '11AM', '12PM', '1PM', '2PM', '3PM', '4PM', '5PM', '6PM', '7PM', '8PM'],
                datasets: [{
                    label: 'Transactions',
                    data: [3, 5, 6, 8, 4, 6, 5, 7, 9, 12, 8, 4],
                    borderColor: 'rgba(52, 152, 219, 1)',
                    backgroundColor: 'rgba(52, 152, 219, 0.1)',
                    tension: 0.4,
                    fill: true
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    legend: {
                        display: false
                    }
                },
                scales: {
                    y: {
                        beginAtZero: true,
                        ticks: {
                            stepSize: 2
                        }
                    }
                }
            }
        });
    }
}

// Set up form handlers for various sections
function setupFormHandlers() {
    // Generate report button
    const generateReportBtn = document.getElementById('generate-report');
    if (generateReportBtn) {
        generateReportBtn.addEventListener('click', function() {
            const dateRange = document.getElementById('date-range').value;
            let rangeText = '';
            
            switch(dateRange) {
                case 'today':
                    rangeText = 'today';
                    break;
                case 'thisWeek':
                    rangeText = 'this week';
                    break;
                case 'thisMonth':
                    rangeText = 'this month';
                    break;
                case 'custom':
                    const dateFrom = document.getElementById('date-from').value;
                    const dateTo = document.getElementById('date-to').value;
                    rangeText = `from ${dateFrom} to ${dateTo}`;
                    break;
            }
            
            showAlert(`Generated withdrawal report for ${rangeText}`);
            logAction(`Generated withdrawal report for ${rangeText}`);
        });
    }
    
    // Export and print buttons
    const exportButtons = document.querySelectorAll('#export-csv, #export-pdf, #print-report');
    exportButtons.forEach(button => {
        if (button) {
            button.addEventListener('click', function() {
                let action = '';
                
                switch(this.id) {
                    case 'export-csv':
                        action = 'CSV';
                        break;
                    case 'export-pdf':
                        action = 'PDF';
                        break;
                    case 'print-report':
                        action = 'Printed';
                        break;
                }
                
                showAlert(`${action} report generated successfully`);
                logAction(`${action} withdrawal report`);
            });
        }
    });
}

// Set up modal handlers
function setupModalHandlers() {
    const modals = document.querySelectorAll('.modal');
    const closeBtns = document.querySelectorAll('.close-modal');
    
    // Close button functionality
    closeBtns.forEach(btn => {
        btn.addEventListener('click', function() {
            const modal = this.closest('.modal');
            hideModal(modal);
        });
    });
    
    // Close success modal button
    const closeSuccessBtn = document.getElementById('close-success-modal');
    if (closeSuccessBtn) {
        closeSuccessBtn.addEventListener('click', function() {
            hideModal(document.getElementById('success-modal'));
        });
    }
    
    // Confirmation modal buttons
    const cancelActionBtn = document.getElementById('cancel-action');
    const confirmActionBtn = document.getElementById('confirm-action');
    
    if (cancelActionBtn) {
        cancelActionBtn.addEventListener('click', function() {
            hideModal(document.getElementById('confirm-modal'));
        });
    }
    
    if (confirmActionBtn) {
        confirmActionBtn.addEventListener('click', function() {
            // Get the stored callback function and execute it
            if (window.confirmCallback && typeof window.confirmCallback === 'function') {
                window.confirmCallback();
                window.confirmCallback = null;
            }
            
            hideModal(document.getElementById('confirm-modal'));
        });
    }
    
    // Close modals when clicking outside
    modals.forEach(modal => {
        modal.addEventListener('click', function(event) {
            if (event.target === this) {
                hideModal(this);
            }
        });
    });
}

// Show a modal dialog
function showModal(modalId) {
    const modal = document.getElementById(modalId);
    modal.classList.add('active');
}

// Hide a modal dialog
function hideModal(modal) {
    modal.classList.remove('active');
}

// Show confirmation dialog
function showConfirmation(message, callback) {
    const confirmMessage = document.getElementById('confirm-message');
    confirmMessage.textContent = message;
    
    // Store callback function to be executed on confirm
    window.confirmCallback = callback;
    
    showModal('confirm-modal');
}

// Show an alert using the success modal
function showAlert(message) {
    const successMessage = document.getElementById('success-message');
    successMessage.textContent = message;
    
    showModal('success-modal');
}

// Handle logout button click
function handleLogout() {
    showConfirmation('Are you sure you want to logout?', function() {
        // In a real application, this would call an API to logout
        logAction('User logged out');
        
        // Redirect to login page or show login screen
        showAlert('You have been logged out successfully');
        
        // Simulate redirect after logout
        setTimeout(() => {
            window.location.href = '../atm_interface/index.html';
        }, 2000);
    });
}

// Update date and time display (if present)
function updateDateTime() {
    const dateTimeElement = document.getElementById('date-time');
    if (dateTimeElement) {
        const now = new Date();
        dateTimeElement.textContent = now.toLocaleDateString('en-IN', { 
            day: '2-digit', 
            month: '2-digit', 
            year: 'numeric',
            hour: '2-digit',
            minute: '2-digit'
        });
        
        // Update every minute
        setTimeout(updateDateTime, 60000);
    }
}

// Log admin actions to the backend API
function logAction(action) {
    console.log('Admin Action:', action);
    
    // In a real application, this would call an API endpoint
    // to log the admin action for audit purposes
    
    // Example API call:
    // fetch('/api/admin/log', {
    //     method: 'POST',
    //     headers: {
    //         'Content-Type': 'application/json'
    //     },
    //     body: JSON.stringify({
    //         action: action,
    //         timestamp: new Date().toISOString(),
    //         userId: 'admin' // In real app, get from session
    //     })
    // });
}