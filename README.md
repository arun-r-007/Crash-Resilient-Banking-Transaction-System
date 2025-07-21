# TxLogger - Crash-Resilient Banking Transaction System

## Overview

This project simulates a **banking system** with a **crash-resilient transaction mechanism** using a file-based data store. It handles:

-  Account management  
-  Inter-bank transfers  
-  Crash simulation  
-  Recovery from incomplete transactions using request/response logs  

---

# Features

-  Create and manage bank accounts  
-  Perform intra-bank and inter-bank transactions (transfer, credit, debit)  
-  Simulate system crashes at various transaction phases  
-  Recover safely from crashes  
-  File-based logging and data persistence  
-  Timestamped transaction records with unique transaction IDs  

---

# Technologies Used

### **Language**:  
- C++

### **Data Storage**:  
- Text files  
  - `accounts_data.txt`  
  - `<bank>_request.csv`  
  - `<bank>_respond.csv`

### **Core Concepts**:  
- File I/O operations  
- Log-based crash recovery  
- Structured transaction phases (debit → request → credit)  
- Crash simulation and recovery  
- Use of STL containers (`vector`, `set`, `stringstream`)  

---

## File Structure

| File Name            | Purpose                                 |
|---------------------|-----------------------------------------|
| `main.cpp`          | Main source code                        |
| `accounts_data.txt` | Stores account ID, bank name, and balance |
| `<bank>_request.csv`| Logs cross-bank transaction requests     |
| `<bank>_respond.csv`| Logs updates (debit/credit) per bank     |

---

## Transaction Flow

1. **Debit Phase**  
   Sender's balance is reduced and logged.

2. **Request Phase (Cross-Bank Only)**  
   A request is logged in the receiver bank's `*_request.csv`.

3. **Credit Phase**  
   Receiver's balance is increased and logged.

Each phase offers optional **crash simulation**, halting the system mid-transaction.

---

## Crash Recovery

The recovery process:
- Replays all valid responses (`*_respond.csv`) to restore correct balances.
- Checks `*_request.csv` for transactions that didn't reach the credit phase.
- Rolls back such incomplete transactions by **re-crediting the sender's account**.

---

## Menu Options

```

1. Show Accounts      - Display all accounts
2. Transfer           - Send money from one account to another
3. Credit             - Add money to an account
4. Debit              - Withdraw money from an account
5. Recover            - Perform crash recovery from logs
6. Create Account     - Create a new bank account
7. Crash              - Simulate crash (Power Outage, Disk Failure, etc.)
8. Exit               - Exit the program

```

---

## Sample Account Format

Stored in `accounts_data.txt`:
```

A001 SBI 10000
A002 ICICI 5000
A003 HDFC 7500

```

Each line: `AccountID BankName Balance`

---

## Crash Simulation Options

During critical phases, you can simulate a crash:
```

1. No crash (continue)
2. Power outage
3. Hard disk failure
4. Random reason

````

These are used to test the system's durability and correctness of recovery logic.

---

## How to Run

1. **Compile:**
```bash
g++ main.cpp -o banking
````

2. **Run:**

```bash
./banking
```

3. **Create Accounts:**
   Use option 6 to create new accounts before performing transactions.

---

## Example Use Case

* User transfers ₹1000 from A001 (SBI) to A002 (ICICI).
* System crashes after debit phase.
* Recovery rolls back debit from A001 since credit to A002 didn’t happen.


## Concepts Demonstrated

* Transaction logging
* Crash recovery mechanisms
* Atomicity in distributed transactions
* File-based data consistency
* Simulation of real-world failures

---

## Future Enhancements

* Add password-protected accounts
* GUI for better user interaction
* Integration with databases (SQLite/MySQL)
* Transaction history per account
* Support for concurrent transactions (multi-threading)
