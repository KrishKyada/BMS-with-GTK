# Bank Management System (GTK GUI Version)

A modern graphical user interface (GUI) based bank management application built with GTK 4 and C. This application provides a user-friendly way to manage bank accounts, perform transactions, and access banking services securely.

## Overview

The GTK GUI version of the Banking Management System offers the same core functionality as the command-line version but with an intuitive graphical interface. Users can create accounts, deposit and withdraw money, check balances, transfer funds, view transaction history, and access admin features—all through an easy-to-navigate GUI.

## Features

- **Modern GUI Interface**: Built with GTK 4 for a responsive and modern user experience
- **Account Management**: Create new bank accounts with secure password protection
- **Deposit & Withdrawal**: Perform deposit and withdrawal operations with real-time balance updates
- **Balance Inquiry**: Check account balance securely with password authentication
- **Transaction History**: View complete transaction history in a formatted dialog window
- **Money Transfer**: Transfer money between accounts with password verification
- **Admin Panel**: View all accounts and balances with admin authentication
- **Secure Password Storage**: Passwords are hashed using a hash function for security
- **Data Persistence**: All account and transaction data stored in binary files
- **Form Validation**: Input validation for all banking operations
- **Styled Interface**: Custom CSS styling for an attractive appearance

## Project Structure

```
BMS-GTK/
├── accounts.dat          (Binary file storing account data)
├── transactions.dat      (Binary file storing transaction records)
└── bms_gtk.c             (Main source code with GTK implementation)
```

## Data Structures

### Account Structure
```c
typedef struct {
    int accountNumber;     // Unique account identifier
    char name[50];         // Account holder's name
    float balance;         // Current account balance
    char password[20];     // Hashed password
} Account;
```

### Transaction Structure
```c
typedef struct {
    int accountNumber;     // Associated account number
    char type[10];         // Transaction type (Deposit/Withdraw/TransIn/TransOut)
    float amount;          // Transaction amount
} Transaction;
```

### GUI Application Structure
```c
typedef struct {
    GtkWidget *window;      // Main application window
    GtkWidget *main_box;    // Main menu container
    GtkWidget *content_area; // Content area for forms
    GtkWidget *result_label; // Result/message display label
    GtkWidget *active_form; // Currently active form
} BankingApp;
```

## Installation & Setup

### Prerequisites
- GTK 4.0 or higher
- GCC compiler
- pkg-config
- Development libraries for GTK 4

### Installing Dependencies

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install libgtk-4-dev build-essential pkg-config
```

#### Fedora
```bash
sudo dnf install gtk4-devel gcc pkg-config
```

#### macOS (using Homebrew)
```bash
brew install gtk4 pkg-config
```

#### Windows
Download and install GTK 4 from the [official GTK website](https://www.gtk.org/docs/installations/windows/)

### Compilation

```bash
gcc -o bms_gtk bms_gtk.c `pkg-config --cflags --libs gtk4`
```

### Running the Application

```bash
./bms_gtk
```

On Windows:
```bash
bms_gtk.exe
```

## User Interface Guide

### Main Menu
The application opens with a main menu displaying 8 options:
1. Create Account
2. Deposit Money
3. Withdraw Money
4. Check Balance
5. View Transactions
6. Transfer Money
7. View All Accounts (Admin)
8. Exit

### Using Each Feature

#### 1. Create Account
- Click the "Create Account" button
- Fill in the required fields:
  - Account Number (unique identifier)
  - Name (account holder's name)
  - Initial Deposit (opening balance)
  - Password (minimum 8 characters)
- Click "Create Account" to complete
- Success message displayed at bottom
- Click "Back to Menu" to return to main menu

#### 2. Deposit Money
- Click the "Deposit Money" button
- Enter your account number
- Enter the deposit amount
- Click "Deposit" to proceed
- New balance displayed in result message
- Click "Back to Menu" to return

#### 3. Withdraw Money
- Click the "Withdraw Money" button
- Enter your account number
- Enter your password for authentication
- Enter the withdrawal amount
- Click "Withdraw" to proceed
- System validates password and sufficient balance
- New balance displayed in result message

#### 4. Check Balance
- Click the "Check Balance" button
- Enter your account number
- Enter your password
- Click "Check Balance"
- Current balance displayed in the form
- Results shown in real-time within the form

#### 5. View Transactions
- Click the "View Transactions" button
- Enter your account number
- Click "View Transactions"
- A dialog window opens displaying:
  - Transaction type (Deposit, Withdraw, TransIn, TransOut)
  - Transaction amount
  - Complete history for the account
- Close the dialog to return to main menu

#### 6. Transfer Money
- Click the "Transfer Money" button
- Enter your account number
- Enter your password for authentication
- Enter recipient's account number
- Enter transfer amount
- Click "Transfer"
- System validates credentials and sufficient balance
- Confirmation message displayed

#### 7. View All Accounts (Admin)
- Click the "View All Accounts (Admin)" button
- Enter the admin password (default: iitj1234)
- Click "View Accounts"
- Dialog window displays all accounts with:
  - Account numbers
  - Account holder names
  - Current balances
- Close the dialog to return to main menu

#### 8. Exit
- Click the "Exit" button
- Application closes safely

## Core Functions

| Function | Purpose |
|----------|---------|
| `activate()` | GTK application activation callback, initializes main window |
| `load_css()` | Loads and applies custom CSS styling to the application |
| `show_message()` | Displays messages in the result label |
| `show_form()` | Displays a form and hides the main menu |
| `return_to_main_menu()` | Returns to main menu from any form |
| `hashPassword()` | Hashes passwords using a simple hash algorithm |
| `accountExists()` | Checks if an account exists in the database |
| `recordTransaction()` | Records all transactions in transaction file |
| `create_account_form()` | Creates the account creation form widget |
| `deposit_money_form()` | Creates the deposit form widget |
| `withdraw_money_form()` | Creates the withdrawal form widget |
| `check_balance_form()` | Creates the balance check form widget |
| `view_transactions_form()` | Creates the transaction viewing form widget |
| `transfer_money_form()` | Creates the money transfer form widget |
| `view_all_accounts_form()` | Creates the admin accounts viewing form widget |
| `create_account_submit()` | Handles account creation submission |
| `deposit_money_submit()` | Handles deposit submission |
| `withdraw_money_submit()` | Handles withdrawal submission |
| `check_balance_submit()` | Handles balance check submission |
| `view_transactions_submit()` | Handles transaction viewing submission |
| `transfer_money_submit()` | Handles money transfer submission |
| `view_all_accounts_submit()` | Handles admin accounts viewing submission |
| `on_button_clicked()` | Main menu button click handler |

## Security Features

- **Password Hashing**: All passwords are hashed before storage using a hash function
- **Password Masking**: Password entry fields hide input characters
- **Authentication**: Balance checks and transfers require password verification
- **Admin Protection**: Admin functions are protected with a master password (iitj1234)
- **Data Validation**: Comprehensive input validation for all transactions
- **Balance Verification**: System validates sufficient balance before withdrawals

## File Format

### accounts.dat
Binary file containing Account structures with:
- Account number (int - 4 bytes)
- Name (char[50] - 50 bytes)
- Balance (float - 4 bytes)
- Hashed password (char[20] - 20 bytes)
- Total: 78 bytes per record

### transactions.dat
Binary file containing Transaction structures with:
- Account number (int - 4 bytes)
- Transaction type (char[10] - 10 bytes)
- Amount (float - 4 bytes)
- Total: 18 bytes per record

## Configuration

The following constants can be modified in the source code:

```c
#define FILENAME "accounts.dat"          // Account data file path
#define TRANSACTION_FILE "transactions.dat"  // Transaction data file path
#define ADMIN_PASSWORD "iitj1234"        // Admin password for viewing all accounts
#define HASH_MULTIPLIER 31               // Multiplier for hash function
```

## CSS Styling

The application uses custom CSS for styling. Key style definitions:

```css
label.title          /* Title labels - large, bold text */
button               /* General button styling */
button.menu-button   /* Main menu buttons with hover effects */
button.menu-button:hover  /* Hover effect for menu buttons */
entry                /* Text entry fields */
box                  /* Container backgrounds */
```

Colors used:
- Background: RGB(110, 203, 236) - Light blue
- Hover effect: RGB(243, 74, 18) - Orange

## Input Validation

The application validates all inputs:
- Account numbers must be valid integers
- Names cannot be empty
- Initial deposits must be greater than zero
- Passwords must be at least 8 characters
- Withdrawal/transfer amounts must be positive
- Passwords are verified with hashing
- Sufficient balance checked before withdrawals
- Account existence verified before operations

## Dialog Windows

The application uses GTK dialog windows for:
- Transaction History: Displays transactions in a scrollable text view
- View All Accounts: Displays all accounts in a formatted table within a scrolled window

These dialogs are modal and can be closed by clicking the "Close" button.

## Limitations & Future Improvements

### Current Limitations
- Simple hash function (not cryptographically secure)
- No data encryption for stored files
- Single-threaded operation
- No multi-user simultaneous access
- Password minimum length: 8 characters
- Limited transaction type categories

### Suggested Improvements
- Implement SHA-256 or bcrypt for password hashing
- Add file encryption (AES)
- Implement database backend (SQLite)
- Add date/time stamps to transactions
- Transaction fees calculation
- Interest calculation for savings accounts
- Account freezing/blocking features
- Transaction search and filtering
- Account statement export to PDF
- Dark mode theme support
- Multi-language support
- Transaction limits and alerts
- Backup and recovery mechanisms
- Two-factor authentication

## Troubleshooting

### GTK Libraries Not Found
**Error**: "gtk4 not found"
**Solution**: Install GTK 4 development libraries and pkg-config

### Compilation Error
**Error**: "undefined reference to gtk functions"
**Solution**: Ensure compilation includes proper GTK flags:
```bash
gcc -o bms_gtk bms_gtk.c `pkg-config --cflags --libs gtk4`
```

### Files Not Being Created
**Error**: "Error opening file"
**Solution**: Run the program from a directory with write permissions

### Window Not Displaying
**Error**: Window appears but is blank
**Solution**: Ensure GTK 4 is properly installed and DISPLAY is set correctly (on Linux)

### Password Not Working
**Error**: "Invalid password" message
**Solution**: Passwords are case-sensitive; ensure caps lock is not engaged

### Account Not Found
**Error**: "Account not found" message
**Solution**: Verify the account number is correct and the account exists

## System Requirements

- OS: Linux, Windows, or macOS
- Processor: Any modern CPU
- RAM: 128 MB minimum
- Disk Space: 5 MB for application and data files
- GTK: Version 4.0 or higher

## Building on Different Platforms

### Linux (Ubuntu/Debian)
```bash
sudo apt-get install libgtk-4-dev
gcc -o bms_gtk bms_gtk.c `pkg-config --cflags --libs gtk4`
./bms_gtk
```

### Linux (Fedora/RHEL)
```bash
sudo dnf install gtk4-devel
gcc -o bms_gtk bms_gtk.c `pkg-config --cflags --libs gtk4`
./bms_gtk
```

### macOS
```bash
brew install gtk4
gcc -o bms_gtk bms_gtk.c `pkg-config --cflags --libs gtk4`
./bms_gtk
```

## License

This is an educational project created for learning GTK 4, C programming, and GUI application development. Use for educational purposes only.

## Author

Krish Kyada

## Support

For bugs, issues, or suggestions:
1. Check the troubleshooting section
2. Review the code for logic errors
3. Verify GTK installation and dependencies
4. Check file permissions and directory access

## Notes

- The application creates `accounts.dat` and `transactions.dat` files in the current working directory
- Ensure the working directory has write permissions for the application to function properly
- The GUI is responsive and handles all operations in the main thread
- All forms support "Back to Menu" button to return without saving changes
- Result messages are displayed at the bottom of the application window

---

**Important**: This system is for educational purposes only and should not be used in production environments without significant security enhancements, proper encryption, and additional safety features.
