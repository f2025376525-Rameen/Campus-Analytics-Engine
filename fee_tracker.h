#ifndef FEE_TRACKER_H
#define FEE_TRACKER_H

#include <string>
#include <vector>
using namespace std;

// Records a payment: validates the date, updates amount_paid and paid_date.
bool recordPayment(string roll, string semester, double amount, string paidDate);

// Counts days between two DD-MM-YYYY dates (manual, no ctime).
int daysBetween(string date1, string date2);

// Late fine = 2% of amount due for each COMPLETE week the payment is late.
double computeLateFine(string roll, string semester);

// Prints a formatted receipt (tuition, fine, total, paid, balance).
void generateReceipt(string roll, string semester);

// Returns rolls of students with balance > 0 past their due date,
// sorted by outstanding amount using bubble sort.
vector<string> getDefaulters();

#endif