#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <map>
#include <cmath>
#include <string>
#include <future> // for std::async
#include <cstdlib> // For generating random codes
#include <ctime>   // For seeding random number generator

using namespace std;

// Forward declarations
class Account;
class Customer;

class Account {
private:
    int id;
    double balance;
    static int next_id;

public:
    Account(double initial_balance = 0.0) : balance(initial_balance) {
        id = next_id++;
    }

    int getId() const {
        return id;
    }

    double getBalance() const {
        return balance;
    }

    void deposit(double amount) {
        balance += amount;
        cout << "Deposit of $" << amount << " successful. New balance: $" << fixed << setprecision(2) << balance << endl;
    }

    void withdraw(double amount) {
        if (balance >= amount) {
            balance -= amount;
            cout << "Withdrawal of $" << amount << " successful. New balance: $" << fixed << setprecision(2) << balance << endl;
        } else {
            cout << "Insufficient funds. Withdrawal failed." << endl;
        }
    }

    void transfer(Account& recipient, double amount) {
        if (balance >= amount) {
            balance -= amount;
            recipient.deposit(amount);
            cout << "Transfer of $" << amount << " successful." << endl;
            cout << "Sender's new balance: $" << fixed << setprecision(2) << balance << endl;
        } else {
            cout << "Insufficient funds. Transfer failed." << endl;
        }
    }

    void displayInfo() const {
        cout << "Account ID: " << id << ", Balance: $" << fixed << setprecision(2) << balance << endl;
    }
};

int Account::next_id = 1;

class Customer {
private:
    string name;
    string address;
    string email; // Add email field for 2FA
    vector<Account*> accounts;

public:
    Customer(const string& name, const string& address, const string& email) : name(name), address(address), email(email) {}

    const string& getName() const {
        return name;
    }

    const string& getAddress() const {
        return address;
    }

    const string& getEmail() const {
        return email;
    }

    void addAccount(Account* account) {
        accounts.push_back(account);
    }

    const vector<Account*>& getAccounts() const {
        return accounts;
    }

    void displayInfo() const {
        cout << "Customer Name: " << name << endl;
        cout << "Address: " << address << endl;
        cout << "Email: " << email << endl; // Display email in customer info
        cout << "Number of Accounts: " << accounts.size() << endl;
        cout << "Accounts:" << endl;
        for (const auto& acc : accounts) {
            acc->displayInfo();
        }
    }
};

class Bank {
private:
    string name;
    mutable map<string, pair<string, string>> users; // username -> (password, customer name)
    mutable map<string, Customer*> customers; // customer name -> Customer object
    map<int, Account*> accounts; // account ID -> Account object
    double interest_rate;
    int compounding_period; // in months

public:
    Bank(const string& name, double interest_rate, int compounding_period)
        : name(name), interest_rate(interest_rate), compounding_period(compounding_period) {}

    void addUser(const string& username, const string& password, const string& customer_name) {
        users[username] = make_pair(password, customer_name);
    }

    bool authenticate(const string& username, const string& password) const {
        auto it = users.find(username);
        if (it != users.end() && it->second.first == password) {
            cout << "Authentication successful." << endl;
            return true;
        }
        cout << "Authentication failed. Invalid username or password." << endl;
        return false;
    }

    // Generate a random 4-digit code
    string generateRandomCode() const {
        srand(time(nullptr)); // Seed random number generator
        string code;
        for (int i = 0; i < 4; ++i) {
            code.push_back('0' + rand() % 10); // Generate a random digit
        }
        return code;
    }

    // Send 2FA code to the customer's email (in a real system, this would send an email)
    void sendTwoFactorCode(const string& email, const string& code) const {
        cout << "Sending 2FA code " << code << " to " << email << "..." << endl;
    }

    // Perform two-factor authentication
    bool performTwoFactorAuth(const string& username) const {
        string code = generateRandomCode();
        auto it = users.find(username);
        if (it != users.end()) {
            string email = customers[it->second.second]->getEmail();
            sendTwoFactorCode(email, code);
            string user_code;
            cout << "Enter the 2FA code sent to your email: ";
            cin >> user_code;
            if (user_code == code) {
                cout << "Two-factor authentication successful." << endl;
                return true;
            } else {
                cout << "Two-factor authentication failed. Incorrect code." << endl;
            }
        } else {
            cout << "User not found." << endl;
        }
        return false;
    }

    void addCustomer(Customer* customer) {
        customers[customer->getName()] = customer;
        cout << "Customer added successfully." << endl;
    }

    void createAccount(const string& customer_name, double initial_balance) {
        auto it = customers.find(customer_name);
        if (it != customers.end()) {
            Account* account = new Account(initial_balance);
            it->second->addAccount(account);
            accounts[account->getId()] = account;
            cout << "Account created successfully." << endl;
        } else {
            cout << "Customer not found. Cannot create account." << endl;
        }
    }

    void deposit(int account_id, double amount) {
        auto it = accounts.find(account_id);
        if (it != accounts.end()) {
            it->second->deposit(amount);
        } else {
            cout << "Account not found. Deposit failed." << endl;
        }
    }

    void withdraw(int account_id, double amount) {
        auto it = accounts.find(account_id);
        if (it != accounts.end()) {
            it->second->withdraw(amount);
        } else {
            cout << "Account not found. Withdrawal failed." << endl;
        }
    }

    void transfer(int sender_account_id, int recipient_account_id, double amount) {
        auto sender_it = accounts.find(sender_account_id);
        auto recipient_it = accounts.find(recipient_account_id);
        if (sender_it != accounts.end() && recipient_it != accounts.end()) {
            sender_it->second->transfer(*(recipient_it->second), amount);
        } else {
            cout << "One or both accounts not found. Transfer failed." << endl;
        }
    }

    void displayCustomerInfo(const     string& customer_name) const {
        auto it = customers.find(customer_name);
        if (it != customers.end()) {
            it->second->displayInfo();
        } else {
            cout << "Customer not found." << endl;
        }
    }

    void displayBankInfo() const {
        cout << "Bank Name: " << name << endl;
        cout << "Number of Customers: " << customers.size() << endl;
        cout << "Customers:" << endl;
        for (const auto& cust : customers) {
            cust.second->displayInfo();
        }
    }

    // Function for parallel deposit
    void depositParallel(int account_id, double amount) {
        deposit(account_id, amount);
    }

    // Function for parallel withdraw
    void withdrawParallel(int account_id, double amount) {
        withdraw(account_id, amount);
    }
};

int main() {
    Bank bank("MyBank", 0.5, 12); // 0.5% interest rate, compounded monthly

    // Adding users for authentication
    bank.addUser("Yash_323", "yash323$", "Yash");
    bank.addUser("Arpit_17", "Arpit323$", "Arpit");

    // User input for adding customers
    string customerName, customerAddress, customerEmail;
    cout << "Enter customer name: ";
    getline(cin, customerName);
    cout << "Enter customer address: ";
    getline(cin, customerAddress);
    cout << "Enter customer email: ";
    getline(cin, customerEmail);

    // Adding customer from user input
    Customer newCustomer(customerName, customerAddress, customerEmail);
    bank.addCustomer(&newCustomer);
    cout << "Customer added successfully." << endl;

    // Creating accounts for the newly added customer
    cout << "Enter initial balance for the first account: ";
    double initialBalance1;
    cin >> initialBalance1;
    bank.createAccount(customerName, initialBalance1);

    cout << "Enter initial balance for the second account: ";
    double initialBalance2;
    cin >> initialBalance2;
    bank.createAccount(customerName, initialBalance2);

    // User input for authentication
    string username, password;
    cout << "Enter username: ";
    cin >> username;
    cout << "Enter password: ";
    cin >> password;

    // Authenticate user
    if (bank.authenticate(username, password)) {
        // Perform two-factor authentication
        if (bank.performTwoFactorAuth(username)) {
            // Actions after authentication
            int choice;
            do {
                cout << "\nActions Menu:\n";
                cout << "1. Deposit\n";
                cout << "2. Withdraw\n";
                cout << "3. Transfer\n";
                cout << "4. Display Customer Information\n";
                cout << "5. Display Bank Information\n";
                cout << "6. Generate Visualization\n";
                cout << "7. Exit\n";
                cout << "Enter your choice: ";
                cin >> choice;

                switch (choice) {
                    case 1: {
                        int accountId;
                        double amount;
                        cout << "Enter account ID: ";
                        cin >> accountId;
                        cout << "Enter amount to deposit: ";
                        cin >> amount;
                        // Call deposit in a separate thread
                        bank.deposit(accountId, amount);
                        break;
                    }
                    case 2: {
                        int accountId;
                        double amount;
                        cout << "Enter account ID: ";
                        cin >> accountId;
                        cout << "Enter amount to withdraw: ";
                        cin >> amount;
                        // Call withdraw in a separate thread
                        bank.withdraw(accountId, amount);
                        break;
                    }
                    case 3: {
                        int senderAccountId, recipientAccountId;
                        double amount;
                        cout << "Enter sender account ID: ";
                        cin >> senderAccountId;
                        cout << "Enter recipient account ID: ";
                        cin >> recipientAccountId;
                        cout << "Enter amount to transfer: ";
                        cin >> amount;
                        bank.transfer(senderAccountId, recipientAccountId, amount);
                        break;
                    }
                    case 4: {
                        cout << "Enter customer name: ";
                        string customerName;
                        cin >> customerName;
                        bank.displayCustomerInfo(customerName);
                        break;
                    }
                    case 5: {
                        bank.displayBankInfo();
                        break;
                    }
                    case 6: {
                        // Generate visualization
                        cout << "Visualization generated." << endl;
                        break;
                    }
                    case 7: {
                        cout << "Exiting..." << endl;
                        break;
                    }
                    default: {
                        cout << "Invalid choice. Please enter a number between 1 and 7." << endl;
                        break;
                    }
                }
            } while (choice != 7);
        }
    }

    return 0;
}

