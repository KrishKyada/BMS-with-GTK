#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define FILENAME "accounts.dat"
#define TRANSACTION_FILE "transactions.dat"
#define ADMIN_PASSWORD "iitj1234"
#define HASH_MULTIPLIER 31


typedef struct
{
    int accountNumber;
    char name[50];
    float balance;
    char password[20];
} Account;


typedef struct
{
    int accountNumber;
    char type[10];
    float amount;
} Transaction;


typedef struct
{
    GtkWidget *window;
    GtkWidget *main_box;
    GtkWidget *content_area;
    GtkWidget *result_label;
    GtkWidget *active_form;
} BankingApp;

BankingApp app;


void show_message(const char *message);
void show_form(GtkWidget *form);
void return_to_main_menu(GtkWidget *widget, gpointer data);
void transfer_money_submit(GtkWidget *widget, gpointer data);
GtkWidget* transfer_money_form(void);
GtkWidget* create_account_form(void);
GtkWidget* deposit_money_form(void);
GtkWidget* withdraw_money_form(void);
GtkWidget* check_balance_form(void);
GtkWidget* view_transactions_form(void);
GtkWidget* view_all_accounts_form(void);


void hashPassword(const char *password, char *hashed)
{
    unsigned int hash = 0;
    while (*password)
    {
        hash = hash * HASH_MULTIPLIER + *password++;
    }
    sprintf(hashed, "%u", hash); 
}


int accountExists(int accNum)
{
    Account acc;
    FILE *fp = fopen(FILENAME, "rb");
    if (!fp)
    {
        return 0; 
    }

    while (fread(&acc, sizeof(Account), 1, fp))
    {
        if (acc.accountNumber == accNum)
        {
            fclose(fp);
            return 1; 
        }
    }
    fclose(fp);
    return 0; 
}


void recordTransaction(int accNum, const char *type, float amount)
{
    Transaction trans;
    trans.accountNumber = accNum;
    strcpy(trans.type, type); 
    trans.amount = amount;

    FILE *fp = fopen(TRANSACTION_FILE, "ab"); 
    if (!fp)
    {
        show_message("Error opening transaction file!");
        return;
    }

    fwrite(&trans, sizeof(Transaction), 1, fp); 
    fclose(fp);                                 
}


void show_message(const char *message)
{
    gtk_label_set_text(GTK_LABEL(app.result_label), message);
}


void create_account_submit(GtkWidget *widget, gpointer data)
{
    GtkWidget **entry_fields = (GtkWidget **)data;

    int accNum = atoi(gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[0]))));
    const char *name = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[1])));
    float balance = atof(gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[2]))));
    const char *password = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[3])));

    
    if (strlen(name) == 0)
    {
        show_message("Name cannot be empty!");
        return;
    }

    if (balance <= 0)
    {
        show_message("Initial deposit must be greater than zero!");
        return;
    }

    if (strlen(password) < 8)
    {
        show_message("Password must be at least 8 characters!");
        return;
    }

    
    if (accountExists(accNum))
    {
        show_message("Account number already exists! Try another.");
        return;
    }

    
    Account acc;
    char hashedPassword[20];

    acc.accountNumber = accNum;
    strncpy(acc.name, name, sizeof(acc.name) - 1);
    acc.name[sizeof(acc.name) - 1] = '\0'; 
    acc.balance = balance;

    
    hashPassword(password, hashedPassword);
    strcpy(acc.password, hashedPassword);

    FILE *fp = fopen(FILENAME, "ab");
    if (!fp)
    {
        show_message("Error opening file!");
        return;
    }

    fwrite(&acc, sizeof(Account), 1, fp);
    fclose(fp);

    
    recordTransaction(accNum, "Deposit", balance);

    char result[100];
    snprintf(result, sizeof(result), "Account %d created successfully!", accNum);
    show_message(result);

    
    gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[0])), "", 0);
    gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[1])), "", 0);
    gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[2])), "", 0);
    gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[3])), "", 0);
}


void transfer_money_submit(GtkWidget *widget, gpointer data)
{
    GtkWidget **entry_fields = (GtkWidget **)data;

    int fromAcc = atoi(gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[0]))));
    const char *password = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[1])));
    int toAcc = atoi(gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[2]))));
    float amount = atof(gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[3]))));

    if (amount <= 0)
    {
        show_message("Deposit amount must be greater than zero!");
        return;
    }

    if (fromAcc == toAcc)
    {
        show_message("Cannot transfer to the same account!");
        return;
    }

    if (amount <= 0)
    {
        show_message("Transfer amount must be greater than zero!");
        return;
    }

    if (strlen(password) < 8)
    {
        show_message("Password must be at least 8 characters!");
        return;
    }

    Account sender, receiver;
    FILE *fp = fopen(FILENAME, "rb+");
    if (!fp)
    {
        show_message("Error accessing account file!");
        return;
    }

    int senderFound = 0, receiverFound = 0;
    long senderPos = -1, receiverPos = -1;
    long pos = 0;

    char hashedPassword[20];
    hashPassword(password, hashedPassword);

    while (fread(&sender, sizeof(Account), 1, fp))
    {
        if (!senderFound && sender.accountNumber == fromAcc)
        {
            if (strcmp(sender.password, hashedPassword) != 0)
            {
                fclose(fp);
                show_message("Invalid password!");
                return;
            }
            senderFound = 1;
            senderPos = pos;
        }
        else if (!receiverFound && sender.accountNumber == toAcc)
        {
            receiver = sender;
            receiverFound = 1;
            receiverPos = pos;
        }

        if (senderFound && receiverFound)
            break;

        pos++;
    }

    if (!senderFound)
    {
        fclose(fp);
        show_message("Sender account not found!");
        return;
    }

    if (!receiverFound)
    {
        fclose(fp);
        show_message("Recipient account not found!");
        return;
    }

    fseek(fp, senderPos * sizeof(Account), SEEK_SET);
    fread(&sender, sizeof(Account), 1, fp);

    if (sender.balance < amount)
    {
        fclose(fp);
        show_message("Insufficient balance for transfer!");
        return;
    }

    sender.balance -= amount;
    receiver.balance += amount;

    fseek(fp, senderPos * sizeof(Account), SEEK_SET);
    fwrite(&sender, sizeof(Account), 1, fp);

    fseek(fp, receiverPos * sizeof(Account), SEEK_SET);
    fwrite(&receiver, sizeof(Account), 1, fp);

    fclose(fp);

    recordTransaction(fromAcc, "TransOut", amount);
    recordTransaction(toAcc, "TransIn", amount);

    char result[100];
    snprintf(result, sizeof(result), "Transfer successful! %.2f transferred to account #%d", amount, toAcc);
    show_message(result);

    gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[0])), "", 0);
    gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[1])), "", 0);
    gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[2])), "", 0);
    gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[3])), "", 0);
}



void deposit_money_submit(GtkWidget *widget, gpointer data)
{
    GtkWidget **entry_fields = (GtkWidget **)data;

    int accNum = atoi(gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[0]))));
    float amount = atof(gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[1]))));

    
    if (amount <= 0)
    {
        show_message("Deposit amount must be greater than zero!");
        return;
    }

    Account acc;
    FILE *fp = fopen(FILENAME, "rb+");
    if (!fp)
    {
        show_message("Error accessing file!");
        return;
    }

    int found = 0;
    while (fread(&acc, sizeof(Account), 1, fp))
    {
        if (acc.accountNumber == accNum)
        {
            found = 1;
            acc.balance += amount;
            fseek(fp, -sizeof(Account), SEEK_CUR);
            fwrite(&acc, sizeof(Account), 1, fp);
            break;
        }
    }

    fclose(fp);

    if (!found)
    {
        show_message("Account not found!");
        return;
    }

    
    recordTransaction(accNum, "Deposit", amount);

    char result[100];
    snprintf(result, sizeof(result), "Deposit successful! New balance: %.2f", acc.balance);
    show_message(result);

    
    gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[0])), "", 0);
    gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[1])), "", 0);
}


void withdraw_money_submit(GtkWidget *widget, gpointer data)
{
    GtkWidget **entry_fields = (GtkWidget **)data;

    int accNum = atoi(gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[0]))));
    const char *password = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[1])));
    float amount = atof(gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[2]))));

    
    if (accNum <= 0)
    {
        show_message("Invalid account number!");
        return;
    }

    if (strlen(password) < 1)
    {
        show_message("Password cannot be empty!");
        return;
    }

    if (amount <= 0)
    {
        show_message("Withdrawal amount must be greater than zero!");
        return;
    }

    Account acc;
    FILE *fp = fopen(FILENAME, "rb+");
    if (!fp)
    {
        show_message("Error accessing file!");
        return;
    }

    int found = 0;
    char hashedPassword[20];
    hashPassword(password, hashedPassword);
    
    while (fread(&acc, sizeof(Account), 1, fp))
    {
        if (acc.accountNumber == accNum)
        {
            found = 1;
            
            
            if (strcmp(acc.password, hashedPassword) != 0)
            {
                fclose(fp);
                show_message("Invalid password!");
                return;
            }
            
            if (amount > acc.balance)
            {
                fclose(fp);
                show_message("Insufficient balance!");
                return;
            }

            acc.balance -= amount;
            fseek(fp, -sizeof(Account), SEEK_CUR);
            fwrite(&acc, sizeof(Account), 1, fp);
            break;
        }
    }

    fclose(fp);

    if (!found)
    {
        show_message("Account not found!");
        return;
    }

    
    recordTransaction(accNum, "Withdraw", amount);

    char result[100];
    snprintf(result, sizeof(result), "Withdrawal successful! New balance: %.2f", acc.balance);
    show_message(result);

    
    gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[0])), "", 0);
    gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[1])), "", 0);
    gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[2])), "", 0);
}

void check_balance_submit(GtkWidget *widget, gpointer data)
{
    GtkWidget **entry_fields = (GtkWidget **)data;

    int accNum = atoi(gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[0]))));
    const char *password = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[1])));
    GtkWidget *result_label = entry_fields[2];  

    
    if (accNum <= 0)
    {
        gtk_label_set_text(GTK_LABEL(result_label), "Invalid account number!");  
        return;
    }

    Account acc;
    FILE *fp = fopen(FILENAME, "rb");
    if (!fp)
    {
        gtk_label_set_text(GTK_LABEL(result_label), "Error accessing file!");  
        return;
    }

    int found = 0;
    char hashedPassword[20];
    hashPassword(password, hashedPassword);

    while (fread(&acc, sizeof(Account), 1, fp))
    {
        if (acc.accountNumber == accNum)
        {
            found = 1;
            if (strcmp(acc.password, hashedPassword) != 0)
            {
                fclose(fp);
                gtk_label_set_text(GTK_LABEL(result_label), "Invalid password!");  
                return;
            }

            char result[100];
            snprintf(result, sizeof(result), "Account Balance: %.2f", acc.balance);
            gtk_label_set_text(GTK_LABEL(result_label), result);  
            break;
        }
    }

    fclose(fp);

    if (!found)
    {
        gtk_label_set_text(GTK_LABEL(result_label), "Account not found!");  
    }

    
    gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[0])), "", 0);
    gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(entry_fields[1])), "", 0);
}


void view_transactions_submit(GtkWidget *widget, gpointer data)
{
    GtkWidget *entry = GTK_WIDGET(data);

    int accNum = atoi(gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry))));

    
    if (accNum <= 0)
    {
        show_message("Invalid account number!");
        return;
    }

    Transaction trans;
    FILE *fp = fopen(TRANSACTION_FILE, "rb");
    if (!fp)
    {
        show_message("No transactions found!");
        return;
    }

    
    GtkTextBuffer *buffer = gtk_text_buffer_new(NULL);
    GtkTextIter iter;
    gtk_text_buffer_get_start_iter(buffer, &iter);

    char header[100];
    snprintf(header, sizeof(header), "Transaction History for Account %d:\n\n", accNum);
    gtk_text_buffer_insert(buffer, &iter, header, -1);
    gtk_text_buffer_insert(buffer, &iter, "Type       Amount\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "---------------------\n", -1);

    int found = 0;
    while (fread(&trans, sizeof(Transaction), 1, fp))
    {
        if (trans.accountNumber == accNum)
        {
            found = 1;
            char line[50];
            snprintf(line, sizeof(line), "%-10s %.2f\n", trans.type, trans.amount);
            gtk_text_buffer_insert(buffer, &iter, line, -1);
        }
    }

    fclose(fp);

    if (!found)
    {
        gtk_text_buffer_insert(buffer, &iter, "No transactions found for this account.", -1);
    }

    
    GtkWidget *text_view = gtk_text_view_new_with_buffer(buffer);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);

    
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), text_view);
    gtk_widget_set_size_request(scrolled_window, 300, 200);

    
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Transaction History",
                                                    GTK_WINDOW(app.window),
                                                    GTK_DIALOG_MODAL,
                                                    "Close", GTK_RESPONSE_CLOSE,
                                                    NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_box_append(GTK_BOX(content_area), scrolled_window);

    gtk_widget_set_size_request(dialog, 350, 300);
    gtk_widget_show(dialog);

    g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), NULL);

    
    gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(entry)), "", 0);
}


void view_all_accounts_submit(GtkWidget *widget, gpointer data)
{
    GtkWidget *entry = GTK_WIDGET(data);

    const char *password = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry)));

    
    if (strcmp(password, ADMIN_PASSWORD) != 0)
    {
        show_message("Invalid admin password!");
        return;
    }

    Account acc;
    FILE *fp = fopen(FILENAME, "rb");
    if (!fp)
    {
        show_message("No accounts found!");
        return;
    }

    
    GtkTextBuffer *buffer = gtk_text_buffer_new(NULL);
    GtkTextIter iter;
    gtk_text_buffer_get_start_iter(buffer, &iter);

    gtk_text_buffer_insert(buffer, &iter, "All Accounts:\n\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "Account Number | Name        | Balance\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "----------------------------------------\n", -1);

    int found = 0;
    while (fread(&acc, sizeof(Account), 1, fp))
    {
        found = 1;
        char line[100];
        snprintf(line, sizeof(line), "%-14d | %-10s | %.2f\n", acc.accountNumber, acc.name, acc.balance);
        gtk_text_buffer_insert(buffer, &iter, line, -1);
    }

    fclose(fp);

    if (!found)
    {
        gtk_text_buffer_insert(buffer, &iter, "No accounts found in the system.", -1);
    }

    
    GtkWidget *text_view = gtk_text_view_new_with_buffer(buffer);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);

    
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), text_view);
    gtk_widget_set_size_request(scrolled_window, 400, 300);

    
    GtkWidget *dialog = gtk_dialog_new_with_buttons("All Accounts",
                                                    GTK_WINDOW(app.window),
                                                    GTK_DIALOG_MODAL,
                                                    "Close", GTK_RESPONSE_CLOSE,
                                                    NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_box_append(GTK_BOX(content_area), scrolled_window);

    gtk_widget_set_size_request(dialog, 450, 400);
    gtk_widget_show(dialog);

    g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), NULL);

    
    gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(entry)), "", 0);
}


void return_to_main_menu(GtkWidget *widget, gpointer data)
{
    if (app.active_form)
    {
        gtk_widget_set_visible(app.active_form, FALSE);
        app.active_form = NULL;
    }

    gtk_widget_set_visible(app.main_box, TRUE);
    show_message("Welcome to the IITJ Bank");
}


void show_form(GtkWidget *form)
{
    gtk_widget_set_visible(app.main_box, FALSE);

    if (app.active_form)
    {
        gtk_widget_set_visible(app.active_form, FALSE);
    }

    app.active_form = form;
    gtk_widget_set_visible(form, TRUE);
}


GtkWidget* create_account_form()
{
    GtkWidget *form = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_append(GTK_BOX(app.content_area), form);

    GtkWidget *label = gtk_label_new("Create New Account");
    GtkWidget *acc_num_label = gtk_label_new("Account Number:");
    GtkWidget *name_label = gtk_label_new("Name:");
    GtkWidget *balance_label = gtk_label_new("Initial Deposit:");
    GtkWidget *password_label = gtk_label_new("Password:");

    GtkWidget *acc_num_entry = gtk_entry_new();
    GtkWidget *name_entry = gtk_entry_new();
    GtkWidget *balance_entry = gtk_entry_new();
    GtkWidget *password_entry = gtk_entry_new();

    
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *submit_button = gtk_button_new_with_label("Create Account");
    GtkWidget *back_button = gtk_button_new_with_label("Back to Menu");

    
    gtk_widget_add_css_class(label, "title");
    gtk_widget_add_css_class(submit_button, "suggested-action");

    
    GtkWidget **entry_fields = g_malloc(4 * sizeof(GtkWidget *));
    entry_fields[0] = acc_num_entry;
    entry_fields[1] = name_entry;
    entry_fields[2] = balance_entry;
    entry_fields[3] = password_entry;

    
    g_signal_connect(submit_button, "clicked", G_CALLBACK(create_account_submit), entry_fields);
    g_signal_connect(back_button, "clicked", G_CALLBACK(return_to_main_menu), NULL);
    g_signal_connect(form, "unrealize", G_CALLBACK(g_free), entry_fields);

    
    gtk_box_append(GTK_BOX(form), label);

    
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    gtk_grid_attach(GTK_GRID(grid), acc_num_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), acc_num_entry, 1, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), name_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), name_entry, 1, 1, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), balance_label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), balance_entry, 1, 2, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), password_label, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), password_entry, 1, 3, 1, 1);

    gtk_box_append(GTK_BOX(form), grid);

    gtk_box_append(GTK_BOX(button_box), back_button);
    gtk_box_append(GTK_BOX(button_box), submit_button);
    gtk_box_append(GTK_BOX(form), button_box);

    
    gtk_widget_set_margin_start(form, 20);
    gtk_widget_set_margin_end(form, 20);
    gtk_widget_set_margin_top(form, 20);
    gtk_widget_set_margin_bottom(form, 20);

    
    gtk_widget_set_visible(form, FALSE);

    return form;
}


GtkWidget* transfer_money_form()
{
    GtkWidget *form = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_append(GTK_BOX(app.content_area), form);

    GtkWidget *label = gtk_label_new("Transfer Money");
    GtkWidget *from_acc_label = gtk_label_new("Your Account Number:");
    GtkWidget *password_label = gtk_label_new("Your Password:");
    GtkWidget *to_acc_label = gtk_label_new("Recipient Account Number:");
    GtkWidget *amount_label = gtk_label_new("Amount to Transfer:");

    GtkWidget *from_acc_entry = gtk_entry_new();
    GtkWidget *password_entry = gtk_entry_new();
    GtkWidget *to_acc_entry = gtk_entry_new();
    GtkWidget *amount_entry = gtk_entry_new();

    
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *submit_button = gtk_button_new_with_label("Transfer");
    GtkWidget *back_button = gtk_button_new_with_label("Back to Menu");

    
    gtk_widget_add_css_class(label, "title");
    gtk_widget_add_css_class(submit_button, "suggested-action");

    
    GtkWidget **entry_fields = g_malloc(4 * sizeof(GtkWidget *));
    entry_fields[0] = from_acc_entry;
    entry_fields[1] = password_entry;
    entry_fields[2] = to_acc_entry;
    entry_fields[3] = amount_entry;

    
    g_signal_connect(submit_button, "clicked", G_CALLBACK(transfer_money_submit), entry_fields);
    g_signal_connect(back_button, "clicked", G_CALLBACK(return_to_main_menu), NULL);
    g_signal_connect(form, "unrealize", G_CALLBACK(g_free), entry_fields);

    
    gtk_box_append(GTK_BOX(form), label);

    
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    gtk_grid_attach(GTK_GRID(grid), from_acc_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), from_acc_entry, 1, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), password_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), password_entry, 1, 1, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), to_acc_label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), to_acc_entry, 1, 2, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), amount_label, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), amount_entry, 1, 3, 1, 1);

    gtk_box_append(GTK_BOX(form), grid);

    gtk_box_append(GTK_BOX(button_box), back_button);
    gtk_box_append(GTK_BOX(button_box), submit_button);
    gtk_box_append(GTK_BOX(form), button_box);

    
    gtk_widget_set_margin_start(form, 20);
    gtk_widget_set_margin_end(form, 20);
    gtk_widget_set_margin_top(form, 20);
    gtk_widget_set_margin_bottom(form, 20);

    
    gtk_widget_set_visible(form, FALSE);

    return form;
}


GtkWidget* deposit_money_form()
{
    GtkWidget *form = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_append(GTK_BOX(app.content_area), form);

    GtkWidget *label = gtk_label_new("Deposit Money");
    GtkWidget *acc_num_label = gtk_label_new("Account Number:");
    GtkWidget *amount_label = gtk_label_new("Amount to Deposit:");

    GtkWidget *acc_num_entry = gtk_entry_new();
    GtkWidget *amount_entry = gtk_entry_new();

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *submit_button = gtk_button_new_with_label("Deposit");
    GtkWidget *back_button = gtk_button_new_with_label("Back to Menu");

    
    gtk_widget_add_css_class(label, "title");
    gtk_widget_add_css_class(submit_button, "suggested-action");

    
    GtkWidget **entry_fields = g_malloc(2 * sizeof(GtkWidget *));
    entry_fields[0] = acc_num_entry;
    entry_fields[1] = amount_entry;

    
    g_signal_connect(submit_button, "clicked", G_CALLBACK(deposit_money_submit), entry_fields);
    g_signal_connect(back_button, "clicked", G_CALLBACK(return_to_main_menu), NULL);
    g_signal_connect(form, "unrealize", G_CALLBACK(g_free), entry_fields);

    
    gtk_box_append(GTK_BOX(form), label);

    
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    gtk_grid_attach(GTK_GRID(grid), acc_num_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), acc_num_entry, 1, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), amount_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), amount_entry, 1, 1, 1, 1);

    gtk_box_append(GTK_BOX(form), grid);

    gtk_box_append(GTK_BOX(button_box), back_button);
    gtk_box_append(GTK_BOX(button_box), submit_button);
    gtk_box_append(GTK_BOX(form), button_box);

    
    gtk_widget_set_margin_start(form, 20);
    gtk_widget_set_margin_end(form, 20);
    gtk_widget_set_margin_top(form, 20);
    gtk_widget_set_margin_bottom(form, 20);

    
    gtk_widget_set_visible(form, FALSE);

    return form;
}


GtkWidget* withdraw_money_form()
{
    GtkWidget *form = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_append(GTK_BOX(app.content_area), form);

    GtkWidget *label = gtk_label_new("Withdraw Money");
    GtkWidget *acc_num_label = gtk_label_new("Account Number:");
    GtkWidget *password_label = gtk_label_new("Password:");  
    GtkWidget *amount_label = gtk_label_new("Amount to Withdraw:");

    GtkWidget *acc_num_entry = gtk_entry_new();
    GtkWidget *password_entry = gtk_entry_new();  
    GtkWidget *amount_entry = gtk_entry_new();

    
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *submit_button = gtk_button_new_with_label("Withdraw");
    GtkWidget *back_button = gtk_button_new_with_label("Back to Menu");

    
    gtk_widget_add_css_class(label, "title");
    gtk_widget_add_css_class(submit_button, "suggested-action");

    
    GtkWidget **entry_fields = g_malloc(3 * sizeof(GtkWidget *));  
    entry_fields[0] = acc_num_entry;
    entry_fields[1] = password_entry;  
    entry_fields[2] = amount_entry;    

    
    g_signal_connect(submit_button, "clicked", G_CALLBACK(withdraw_money_submit), entry_fields);
    g_signal_connect(back_button, "clicked", G_CALLBACK(return_to_main_menu), NULL);
    g_signal_connect(form, "unrealize", G_CALLBACK(g_free), entry_fields);

    
    gtk_box_append(GTK_BOX(form), label);

    
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    gtk_grid_attach(GTK_GRID(grid), acc_num_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), acc_num_entry, 1, 0, 1, 1);
    
    
    gtk_grid_attach(GTK_GRID(grid), password_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), password_entry, 1, 1, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), amount_label, 0, 2, 1, 1);  
    gtk_grid_attach(GTK_GRID(grid), amount_entry, 1, 2, 1, 1);  

    gtk_box_append(GTK_BOX(form), grid);

    gtk_box_append(GTK_BOX(button_box), back_button);
    gtk_box_append(GTK_BOX(button_box), submit_button);
    gtk_box_append(GTK_BOX(form), button_box);

    
    gtk_widget_set_margin_start(form, 20);
    gtk_widget_set_margin_end(form, 20);
    gtk_widget_set_margin_top(form, 20);
    gtk_widget_set_margin_bottom(form, 20);

    
    gtk_widget_set_visible(form, FALSE);

    return form;
}

GtkWidget* check_balance_form()
{
    GtkWidget *form = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_append(GTK_BOX(app.content_area), form);

    GtkWidget *label = gtk_label_new("Check Balance");
    GtkWidget *acc_num_label = gtk_label_new("Account Number:");
    GtkWidget *password_label = gtk_label_new("Password:");

    GtkWidget *acc_num_entry = gtk_entry_new();
    GtkWidget *password_entry = gtk_entry_new();

    
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);

    
    GtkWidget *balance_result_label = gtk_label_new("");  
    
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *submit_button = gtk_button_new_with_label("Check Balance");
    GtkWidget *back_button = gtk_button_new_with_label("Back to Menu");

    
    gtk_widget_add_css_class(label, "title");
    gtk_widget_add_css_class(submit_button, "suggested-action");

    
    GtkWidget **entry_fields = g_malloc(3 * sizeof(GtkWidget *));  
    entry_fields[0] = acc_num_entry;
    entry_fields[1] = password_entry;
    entry_fields[2] = balance_result_label;  

    
    g_signal_connect(submit_button, "clicked", G_CALLBACK(check_balance_submit), entry_fields);
    g_signal_connect(back_button, "clicked", G_CALLBACK(return_to_main_menu), NULL);
    g_signal_connect(form, "unrealize", G_CALLBACK(g_free), entry_fields);

    
    gtk_box_append(GTK_BOX(form), label);

    
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    gtk_grid_attach(GTK_GRID(grid), acc_num_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), acc_num_entry, 1, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), password_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), password_entry, 1, 1, 1, 1);

    gtk_box_append(GTK_BOX(form), grid);
    
    
    gtk_box_append(GTK_BOX(form), balance_result_label);  

    gtk_box_append(GTK_BOX(button_box), back_button);
    gtk_box_append(GTK_BOX(button_box), submit_button);
    gtk_box_append(GTK_BOX(form), button_box);

    
    gtk_widget_set_margin_start(form, 20);
    gtk_widget_set_margin_end(form, 20);
    gtk_widget_set_margin_top(form, 20);
    gtk_widget_set_margin_bottom(form, 20);

    
    gtk_widget_set_visible(form, FALSE);

    return form;
}


GtkWidget* view_transactions_form()
{
    GtkWidget *form = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_append(GTK_BOX(app.content_area), form);

    GtkWidget *label = gtk_label_new("View Transactions");
    GtkWidget *acc_num_label = gtk_label_new("Account Number:");

    GtkWidget *acc_num_entry = gtk_entry_new();

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *submit_button = gtk_button_new_with_label("View Transactions");
    GtkWidget *back_button = gtk_button_new_with_label("Back to Menu");

    
    gtk_widget_add_css_class(label, "title");
    gtk_widget_add_css_class(submit_button, "suggested-action");

    
    g_signal_connect(submit_button, "clicked", G_CALLBACK(view_transactions_submit), acc_num_entry);
    g_signal_connect(back_button, "clicked", G_CALLBACK(return_to_main_menu), NULL);

    
    gtk_box_append(GTK_BOX(form), label);

    
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    gtk_grid_attach(GTK_GRID(grid), acc_num_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), acc_num_entry, 1, 0, 1, 1);

    gtk_box_append(GTK_BOX(form), grid);

    gtk_box_append(GTK_BOX(button_box), back_button);
    gtk_box_append(GTK_BOX(button_box), submit_button);
    gtk_box_append(GTK_BOX(form), button_box);

    
    gtk_widget_set_margin_start(form, 20);
    gtk_widget_set_margin_end(form, 20);
    gtk_widget_set_margin_top(form, 20);
    gtk_widget_set_margin_bottom(form, 20);

    
    gtk_widget_set_visible(form, FALSE);

    return form;
}


GtkWidget* view_all_accounts_form()
{
    GtkWidget *form = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_append(GTK_BOX(app.content_area), form);

    GtkWidget *label = gtk_label_new("View All Accounts (Admin)");
    GtkWidget *password_label = gtk_label_new("Admin Password:");

    GtkWidget *password_entry = gtk_entry_new();

    
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *submit_button = gtk_button_new_with_label("View Accounts");
    GtkWidget *back_button = gtk_button_new_with_label("Back to Menu");

    
    gtk_widget_add_css_class(label, "title");
    gtk_widget_add_css_class(submit_button, "suggested-action");

    
    g_signal_connect(submit_button, "clicked", G_CALLBACK(view_all_accounts_submit), password_entry);
    g_signal_connect(back_button, "clicked", G_CALLBACK(return_to_main_menu), NULL);

    
    gtk_box_append(GTK_BOX(form), label);

    
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    gtk_grid_attach(GTK_GRID(grid), password_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), password_entry, 1, 0, 1, 1);

    gtk_box_append(GTK_BOX(form), grid);

    gtk_box_append(GTK_BOX(button_box), back_button);
    gtk_box_append(GTK_BOX(button_box), submit_button);
    gtk_box_append(GTK_BOX(form), button_box);

    
    gtk_widget_set_margin_start(form, 20);
    gtk_widget_set_margin_end(form, 20);
    gtk_widget_set_margin_top(form, 20);
    gtk_widget_set_margin_bottom(form, 20);

    
    gtk_widget_set_visible(form, FALSE);

    return form;
}


void on_button_clicked(GtkWidget *widget, gpointer data)
{
    int option = GPOINTER_TO_INT(data);

    switch (option)
    {
    case 1:
        show_form(create_account_form());
        break;
    case 2:
        show_form(deposit_money_form());
        break;
    case 3:
        show_form(withdraw_money_form());
        break;
    case 4:
        show_form(check_balance_form());
        break;
    case 5:
        show_form(view_transactions_form());
        break;
    case 6:
        show_form(transfer_money_form());  
        break;
    case 7:
        show_form(view_all_accounts_form());
        break;
    case 8:  
        gtk_window_destroy(GTK_WINDOW(app.window));
        break;
    default:
        show_message("Invalid option!");
    }
}

void load_css()
{
    GtkCssProvider *provider = gtk_css_provider_new();

    const char *css =
        "label.title { font-size: 18px; font-weight: bold; margin-bottom: 10px; }"
        "button { padding: 8px 15px; border-radius: 5px; }"
        "button.menu-button { margin: 5px; font-weight: bold; }"
        "button.menu-button:hover{background-color:rgb(243, 74,18);}"
        "entry { min-width: 200px; }"
        "box { background-color:rgb(110, 203, 236); }";

    gtk_css_provider_load_from_data(provider, css, -1);

    gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                                               GTK_STYLE_PROVIDER(provider),
                                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_object_unref(provider);
}



static void activate(GtkApplication *gtk_app, gpointer user_data)
{
    
    app.window = gtk_application_window_new(gtk_app);
    gtk_window_set_title(GTK_WINDOW(app.window), "Banking Management System");
    gtk_window_set_default_size(GTK_WINDOW(app.window), 500, 600);

    
    load_css();

    
    app.content_area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_window_set_child(GTK_WINDOW(app.window), app.content_area);

    
    app.main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_widget_set_margin_start(app.main_box, 30);
    gtk_widget_set_margin_end(app.main_box, 30);
    gtk_widget_set_margin_top(app.main_box, 30);
    gtk_widget_set_margin_bottom(app.main_box, 30);
    gtk_box_append(GTK_BOX(app.content_area), app.main_box);  

    
    GtkWidget *title_label = gtk_label_new("IITJ Netbanking");
    gtk_widget_add_css_class(title_label, "title");
    gtk_box_append(GTK_BOX(app.main_box), title_label);

    
    GtkWidget *subtitle = gtk_label_new("Select an option from the menu below:");
    gtk_box_append(GTK_BOX(app.main_box), subtitle);

    
    const char *button_labels[] = {
        "Create Account",
        "Deposit Money",
        "Withdraw Money",
        "Check Balance",
        "View Transactions",
        "Transfer Money",
        "View All Accounts (Admin)",
        "Exit"
    };

    for (int i = 0; i < 8; i++)
    {
        GtkWidget *button = gtk_button_new_with_label(button_labels[i]);
        gtk_widget_add_css_class(button, "menu-button");
        gtk_widget_set_size_request(button, 100, 50);
        gtk_box_append(GTK_BOX(app.main_box), button);
        g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), GINT_TO_POINTER(i + 1));
    }

    
    app.result_label = gtk_label_new("Welcome to the IITJ Banking System");
    gtk_widget_set_margin_top(app.result_label, 20);
    gtk_widget_set_halign(app.result_label, GTK_ALIGN_CENTER);  
    gtk_box_append(GTK_BOX(app.content_area), app.result_label);

    gtk_widget_show(app.window);
}

int main(int argc, char *argv[])
{
    GtkApplication *app_ptr;
    int status;

    app_ptr = gtk_application_new("com.example.banking", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app_ptr, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app_ptr), argc, argv);
    g_object_unref(app_ptr);

    return status;
}