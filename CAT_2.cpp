#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <set>

using namespace std;

const string ACCOUNTS_FILE = "accounts_data.txt";

struct Account {
    string id;
    string bankName;
    int    balance;
};

vector<Account> accounts;
int counter = 1;

string currentTimestamp() {
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ltm);
    return string(buf);
}

string generateTxnID() {
    stringstream ss;
    ss << "TXN" << time(0) << "_" << counter++;
    return ss.str();
}

void loadAccounts() {
    accounts.clear();
    ifstream fin(ACCOUNTS_FILE);
    string id, bank;
    int bal;
    while (fin >> id >> bank >> bal) {
        accounts.push_back({id, bank, bal});
    }
}

void saveAccounts() {
    ofstream fout(ACCOUNTS_FILE, ios::trunc);
    for (auto &a : accounts) {
        fout << a.id << " " << a.bankName << " " << a.balance << "\n";
    }
}

int findAccount(const string &id) {
    for (int i = 0; i < (int)accounts.size(); i++) {
        if (accounts[i].id == id)
            return i;
    }
    return -1;
}

void writeRequestLog(const string &bank, const string &line) {
    ofstream log(bank + "_request.csv", ios::app);
    log << line << "\n";
}

void writeRespondLog(const string &bank, const string &line) {
    ofstream log(bank + "_respond.csv", ios::app);
    log << line << "\n";
}

void simulateCrash(int code) {
    srand(time(0));
    string reason;
    if (code == 2) reason = "Unexpected power outage!";
    else if (code == 3) reason = "Hard disk failure!";
    else if (code == 4) {
        const string reasons[] = {
            "Power surge detected!", "Unstable voltage input!",
            "Overheating CPU!", "Memory (RAM) access violation!",
            "Corrupted BIOS!", "Electromagnetic interference!",
            "Loose motherboard connection!", "Fan failure - System overheated!"
        };
        reason = reasons[rand() % (sizeof(reasons) / sizeof(reasons[0]))];
    } else {
        reason = "Unknown error!";
    }
    cout << "\n--- SYSTEM CRASH ---\n"
         << "Reason: " << reason << "\n"
         << "Emergency shutdown initiated.\n";
    exit(1);
}

void crashMenu(const string &phase) {
    cout << "\n*** Crash options after " << phase << " ***\n"
         << "1. No crash (continue)\n"
         << "2. Power outage\n"
         << "3. Hard disk failure\n"
         << "4. Random reason\n"
         << "Choice: ";
    int choice;
    cin >> choice;
    if (!cin || choice < 1 || choice > 4) {
        cin.clear();
        cin.ignore(1e4, '\n');
        cout << "Invalid choice, continuing.\n";
        return;
    }
    if (choice != 1)
        simulateCrash(choice);
}

void doTransaction(const string &fromID, const string &toID, int amount) {
    string txn = generateTxnID();
    string ts  = currentTimestamp();

    int fi = (fromID != "-") ? findAccount(fromID) : -1;
    int ti = (toID   != "-") ? findAccount(toID)   : -1;
    string bankFrom = (fi >= 0) ? accounts[fi].bankName : "";
    string bankTo   = (ti >= 0) ? accounts[ti].bankName : "";

    // Phase 1: Debit
    if (fromID != "-") {
        if (fi < 0) { cout << "Invalid sender: " << fromID << "\n"; return; }
        if (accounts[fi].balance < amount) { cout << "Insufficient funds for " << fromID << "\n"; return; }
        int before = accounts[fi].balance;
        int after  = before - amount;
        accounts[fi].balance = after;
        saveAccounts();
        writeRespondLog(bankFrom, txn + ",UPDATE," + fromID + ",DEBIT," + to_string(before) + "," + to_string(after) + "," + ts);
    }
    crashMenu("debit phase");

    // Phase 2: Request (cross-bank)
    if (fromID != "-" && toID != "-" && bankFrom != bankTo) {
        writeRequestLog(bankTo, txn + ",REQUEST," + fromID + "," + toID + "," + to_string(amount) + "," + ts);
        crashMenu("request phase");
    }

    // Phase 3: Credit
    if (toID != "-") {
        if (ti < 0) { cout << "Invalid receiver: " << toID << "\n"; return; }
        int before = accounts[ti].balance;
        int after  = before + amount;
        accounts[ti].balance = after;
        saveAccounts();
        writeRespondLog(bankTo, txn + ",UPDATE," + toID + ",CREDIT," + to_string(before) + "," + to_string(after) + "," + currentTimestamp());
    }
    crashMenu("credit phase");

    cout << "Transaction " << txn << " completed successfully.\n";
}

void showAccounts() {
    cout << "\n--- Accounts ---\n";
    for (auto &a : accounts)
        cout << a.id << " (" << a.bankName << "): " << a.balance << "\n";
    cout << "---------------\n";
}

void recover() {
    cout << "Recovering from logs...\n";
    loadAccounts();
    set<string> banks;
    for (auto &a : accounts) banks.insert(a.bankName);

    // 1) Replay all respond updates
    for (auto &bank : banks) {
        ifstream logIn(bank + "_respond.csv");
        if (!logIn) continue;
        string line;
        while (getline(logIn, line)) {
            vector<string> f;
            stringstream ss(line);
            string tok;
            while (getline(ss, tok, ',')) f.push_back(tok);
            if (f.size() >= 7 && f[1] == "UPDATE") {
                int ai = findAccount(f[2]);
                if (ai < 0) continue;
                int after = stoi(f[5]);
                accounts[ai].balance = after;
            }
        }
    }

    // 2) Undo incomplete cross-bank transfers
    int undoCount = 0;
    for (auto &bankTo : banks) {
        // collect responded TXNs for this bank
        set<string> respondedTxns;
        ifstream respIn(bankTo + "_respond.csv");
        if (respIn) {
            string rline;
            while (getline(respIn, rline)) {
                vector<string> r;
                stringstream ss2(rline);
                string t2;
                while (getline(ss2, t2, ',')) r.push_back(t2);
                if (!r.empty()) respondedTxns.insert(r[0]);
            }
        }
        // scan request log for missing responds
        ifstream reqIn(bankTo + "_request.csv");
        if (reqIn) {
            string reqLine;
            while (getline(reqIn, reqLine)) {
                vector<string> req;
                stringstream ss3(reqLine);
                string t3;
                while (getline(ss3, t3, ',')) req.push_back(t3);
                if (req.size() >= 6 && req[1] == "REQUEST") {
                    string txn  = req[0];
                    string from = req[2];
                    int amt      = stoi(req[4]);
                    if (respondedTxns.count(txn) == 0) {
                        int fi = findAccount(from);
                        if (fi >= 0) {
                            accounts[fi].balance += amt;
                            cout << "Rolled back incomplete TXN " << txn << " for account " << from << " by +" << amt << "\n";
                            undoCount++;
                        }
                    }
                }
            }
        }
    }
    if (undoCount > 0) {
        saveAccounts();
        cout << "Recovery complete. Rolled back " << undoCount << " incomplete transaction(s).\n";
    } else {
        cout << "Recovery complete. No incomplete transactions to roll back.\n";
    }
}

void createAccount() {
    string id, bank;
    int bal;
    cout << "New Account ID: ";
    cin >> id;
    if (findAccount(id) != -1) { cout << "Exists already.\n"; return; }
    cout << "Bank Name: ";
    cin >> bank;
    cout << "Initial Balance: ";
    cin >> bal;
    if (bal < 0) { cout << "Must be non-negative.\n"; return; }
    accounts.push_back({id, bank, bal});
    saveAccounts();
    cout << "Account created.\n";
}

int main() {
    loadAccounts();
    while (true) {
        cout << R"(
1. Show Accounts
2. Transfer
3. Credit
4. Debit
5. Recover
6. Create Account
7. Crash
8. Exit
Choice: )";
        int c;
        cin >> c;
        if (!cin) {
            cin.clear();
            cin.ignore(1e4, '\n');
            continue;
        }
        switch (c) {
            case 1:
                showAccounts();
                break;
            case 2: {
                string f, t;
                int amt;
                cout << "From: "; cin >> f;
                cout << "To:   "; cin >> t;
                cout << "Amt:  "; cin >> amt;
                doTransaction(f, t, amt);
                break;
            }
            case 3: {
                string t;
                int amt;
                cout << "Credit To: "; cin >> t;
                cout << "Amt:       "; cin >> amt;
                doTransaction("-", t, amt);
                break;
            }
            case 4: {
                string f;
                int amt;
                cout << "Debit From: "; cin >> f;
                cout << "Amt:        "; cin >> amt;
                doTransaction(f, "-", amt);
                break;
            }
            case 5:
                recover();
                break;
            case 6:
                createAccount();
                break;
            case 7:
                crashMenu("manual crash");
                break;
            case 8:
                cout << "Bye.\n";
                return 0;
            default:
                cout << "Invalid choice.\n";
        }
    }
}