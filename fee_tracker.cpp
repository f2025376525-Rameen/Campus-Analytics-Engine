#include "fee_tracker.h"
#include "filehandler.h"
#include <iostream>
#include <iomanip>   // for setw / setfill (formatted receipt)
using namespace std;

string FEE_FILE = "fees.txt";
// columns: roll,semester,amount_due,amount_paid,due_date,paid_date
string FEE_HEADER = "roll,semester,amount_due,amount_paid,due_date,paid_date";

// ---- Helper: convert numeric string to a double (handles decimals too) ----
double strToDouble(string s) {
    double result = 0;
    int i = 0;
    // integer part
    while (i < (int)s.length() && s[i] != '.') {
        if (s[i] >= '0' && s[i] <= '9') {
            result = result * 10 + (s[i] - '0');
        }
        i++;
    }
    // decimal part
    if (i < (int)s.length() && s[i] == '.') {
        i++;
        double place = 0.1;
        while (i < (int)s.length()) {
            if (s[i] >= '0' && s[i] <= '9') {
                result = result + (s[i] - '0') * place;
                place = place / 10;
            }
            i++;
        }
    }
    return result;
}

// ---- Helper: pull day, month, year out of a "DD-MM-YYYY" string ----
void parseDate(string date, int &day, int &month, int &year) {
    // date format is fixed width: positions 0-1 day, 3-4 month, 6-9 year
    day   = (date[0] - '0') * 10 + (date[1] - '0');
    month = (date[3] - '0') * 10 + (date[4] - '0');
    year  = (date[6] - '0') * 1000 + (date[7] - '0') * 100
          + (date[8] - '0') * 10   + (date[9] - '0');
}

// ---- Helper: is this a leap year? ----
bool isLeapYear(int year) {
    if (year % 400 == 0) return true;
    if (year % 100 == 0) return false;
    if (year % 4 == 0)   return true;
    return false;
}

// ---- Helper: total days from a fixed start point (year 0) to this date ----
// We count: full years' days + full months' days + days. Manual month lengths.
long totalDaysFromStart(int day, int month, int year) {
    // days in each month (index 1 = Jan ... index 12 = Dec)
    int monthDays[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    long total = 0;

    // add days for all complete years before this year
    for (int y = 0; y < year; y++) {
        if (isLeapYear(y)) total = total + 366;
        else total = total + 365;
    }

    // add days for all complete months in THIS year
    for (int m = 1; m < month; m++) {
        total = total + monthDays[m];
        if (m == 2 && isLeapYear(year)) total = total + 1; // Feb in a leap year
    }

    // add the remaining days
    total = total + day;
    return total;
}

// ---- Days between two DD-MM-YYYY dates ----
int daysBetween(string date1, string date2) {
    int d1, m1, y1, d2, m2, y2;
    parseDate(date1, d1, m1, y1);
    parseDate(date2, d2, m2, y2);

    long total1 = totalDaysFromStart(d1, m1, y1);
    long total2 = totalDaysFromStart(d2, m2, y2);

    long diff = total2 - total1;
    if (diff < 0) diff = -diff;   // always return a positive difference
    return (int)diff;
}

// ---- Validate a DD-MM-YYYY date with simple string checks ----
bool isValidDate(string date) {
    if (date.length() != 10) return false;
    if (date[2] != '-' || date[5] != '-') return false;
    // every other position must be a digit
    for (int i = 0; i < 10; i++) {
        if (i == 2 || i == 5) continue;
        if (date[i] < '0' || date[i] > '9') return false;
    }
    return true;
}

// ---- Record a payment: update amount_paid and paid_date in fees.txt ----
bool recordPayment(string roll, string semester, double amount, string paidDate) {
    if (!isValidDate(paidDate)) {
        cout << "ERROR: Date must be in DD-MM-YYYY format\n";
        return false;
    }

    vector<vector<string> > rows = readTXT(FEE_FILE);
    bool found = false;

    for (int i = 0; i < (int)rows.size(); i++) {
        if (rows[i][0] == roll && rows[i][1] == semester) {
            // add the new amount to whatever was already paid
            double alreadyPaid = strToDouble(rows[i][3]);
            double newPaid = alreadyPaid + amount;

            rows[i][3] = to_string((long)newPaid);  // amount_paid
            rows[i][5] = paidDate;                   // paid_date
            found = true;
        }
    }

    if (found) {
        writeTXT(FEE_FILE, FEE_HEADER, rows);
    }
    return found;
}

// ---- Late fine: 2% of amount_due for every COMPLETE week late ----
double computeLateFine(string roll, string semester) {
    vector<string> row = findRow(FEE_FILE, 0, roll);
    if (row.size() == 0) return 0.0;

    // make sure this row is for the right semester
    if (row[1] != semester) return 0.0;

    string dueDate  = row[4];
    string paidDate = row[5];

    if (paidDate == "NONE") return 0.0;   // not paid yet, handle elsewhere

    int lateDays = daysBetween(dueDate, paidDate);

    // only count if paid AFTER the due date
    int dd, dm, dy, pd, pm, py;
    parseDate(dueDate, dd, dm, dy);
    parseDate(paidDate, pd, pm, py);
    if (totalDaysFromStart(pd, pm, py) <= totalDaysFromStart(dd, dm, dy)) {
        return 0.0;   // paid on time or early
    }

    int completeWeeks = lateDays / 7;   // integer division = complete weeks
    double amountDue = strToDouble(row[2]);

    return amountDue * 0.02 * completeWeeks;   // 2% per week
}

// ---- Print a formatted receipt ----
void generateReceipt(string roll, string semester) {
    vector<string> row = findRow(FEE_FILE, 0, roll);
    if (row.size() == 0) {
        cout << "No fee record found for " << roll << "\n";
        return;
    }

    double amountDue  = strToDouble(row[2]);
    double amountPaid = strToDouble(row[3]);
    double fine       = computeLateFine(roll, semester);
    double totalDue   = amountDue + fine;
    double balance    = totalDue - amountPaid;

    cout << "\n";
    cout << setfill('=') << setw(40) << "" << "\n";
    cout << setfill(' ');
    cout << "              FEE RECEIPT\n";
    cout << setfill('=') << setw(40) << "" << "\n";
    cout << setfill(' ');
    cout << left << setw(20) << "Roll:"        << roll << "\n";
    cout << left << setw(20) << "Semester:"    << semester << "\n";
    cout << left << setw(20) << "Tuition Due:" << amountDue << "\n";
    cout << left << setw(20) << "Late Fine:"   << fine << "\n";
    cout << left << setw(20) << "Total Due:"   << totalDue << "\n";
    cout << left << setw(20) << "Amount Paid:" << amountPaid << "\n";
    cout << left << setw(20) << "Balance:"     << balance << "\n";
    cout << setfill('=') << setw(40) << "" << "\n";
    cout << setfill(' ');
}

// ---- Defaulters: balance > 0, sorted by outstanding amount (bubble sort) ----
vector<string> getDefaulters() {
    vector<vector<string> > rows = readTXT(FEE_FILE);

    vector<string> defRolls;
    vector<double> defAmounts;

    // collect students who still owe money
    for (int i = 0; i < (int)rows.size(); i++) {
        double due  = strToDouble(rows[i][2]);
        double paid = strToDouble(rows[i][3]);
        double balance = due - paid;

        if (balance > 0) {
            defRolls.push_back(rows[i][0]);
            defAmounts.push_back(balance);
        }
    }

    // bubble sort by outstanding amount (largest first)
    for (int i = 0; i < (int)defRolls.size() - 1; i++) {
        for (int j = 0; j < (int)defRolls.size() - 1 - i; j++) {
            if (defAmounts[j] < defAmounts[j + 1]) {
                // swap amounts
                double tA = defAmounts[j];
                defAmounts[j] = defAmounts[j + 1];
                defAmounts[j + 1] = tA;
                // swap rolls (keep them in sync)
                string tR = defRolls[j];
                defRolls[j] = defRolls[j + 1];
                defRolls[j + 1] = tR;
            }
        }
    }

    return defRolls;
}