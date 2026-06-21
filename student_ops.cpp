#include "student_ops.h"
#include "filehandler.h"   // we reuse readTXT, writeTXT, appendTXT, findRow
#include <iostream>
using namespace std;

// File name we work with (column order: roll,name,dept,cgpa,status)
string STUDENT_FILE = "students.txt";
string STUDENT_HEADER = "roll,name,dept,cgpa,status";

// ---- Helper: check roll format BSAI-YY-XXX (e.g. BSAI-23-001) ----
// Rules: starts with "BSAI-", then 2 digits, then "-", then 3 digits.
bool isValidRoll(string roll) {
    if (roll.length() != 11) return false;          // total length must be 11

    // first 5 characters must be "BSAI-"
    if (roll.substr(0, 5) != "BSAI-") return false;

    // characters 5 and 6 must be digits (the YY part)
    if (roll[5] < '0' || roll[5] > '9') return false;
    if (roll[6] < '0' || roll[6] > '9') return false;

    // character 7 must be '-'
    if (roll[7] != '-') return false;

    // characters 8, 9, 10 must be digits (the XXX part)
    for (int i = 8; i <= 10; i++) {
        if (roll[i] < '0' || roll[i] > '9') return false;
    }
    return true;
}

// ---- Helper: check a name has no digits ----
bool nameHasNoDigits(string name) {
    for (int i = 0; i < (int)name.length(); i++) {
        if (name[i] >= '0' && name[i] <= '9') {
            return false;   // found a digit -> invalid
        }
    }
    return true;
}

// ---- Add a new student ----
string addStudent(string roll, string name, string dept, double cgpa) {
    // 1. Roll format check
    if (!isValidRoll(roll)) {
        return "ERROR: Roll must be in format BSAI-YY-XXX";
    }
    // 2. No duplicate roll allowed
    if (rowExists(STUDENT_FILE, 0, roll)) {
        return "ERROR: This roll already exists";
    }
    // 3. Name must not contain digits
    if (!nameHasNoDigits(name)) {
        return "ERROR: Name cannot contain digits";
    }
    // 4. CGPA must be between 0.0 and 4.0
    if (cgpa < 0.0 || cgpa > 4.0) {
        return "ERROR: CGPA must be between 0.0 and 4.0";
    }

    // Build the new row and append it. New students start as 'active'.
    vector<string> row;
    row.push_back(roll);
    row.push_back(name);
    row.push_back(dept);

    // convert cgpa (double) to string manually using to_string
    row.push_back(to_string(cgpa));
    row.push_back("active");

    appendTXT(STUDENT_FILE, row);
    return "SUCCESS: Student added";
}

// ---- Search by roll number ----
vector<string> searchByRoll(string roll) {
    return findRow(STUDENT_FILE, 0, roll);   // column 0 is roll
}

// ---- Helper: does 'text' contain 'part' as a substring? ----
bool containsSubstring(string text, string part) {
    if (part.length() > text.length()) return false;

    // try every starting position in text
    for (int i = 0; i + (int)part.length() <= (int)text.length(); i++) {
        if (text.substr(i, part.length()) == part) {
            return true;
        }
    }
    return false;
}

// ---- Search by (partial) name ----
vector<vector<string> > searchByName(string partialName) {
    vector<vector<string> > allRows = readTXT(STUDENT_FILE);
    vector<vector<string> > matches;

    for (int i = 0; i < (int)allRows.size(); i++) {
        // column 1 is the name
        if (containsSubstring(allRows[i][1], partialName)) {
            matches.push_back(allRows[i]);
        }
    }
    return matches;
}

// ---- Update one field of a student (roll cannot change) ----
bool updateStudent(string roll, int fieldIndex, string newValue) {
    if (fieldIndex == 0) return false;   // do not allow changing the roll

    vector<vector<string> > rows = readTXT(STUDENT_FILE);
    bool found = false;

    for (int i = 0; i < (int)rows.size(); i++) {
        if (rows[i][0] == roll) {
            rows[i][fieldIndex] = newValue;
            found = true;
        }
    }

    if (found) {
        writeTXT(STUDENT_FILE, STUDENT_HEADER, rows);   // rewrite whole file
    }
    return found;
}

// ---- Soft delete: set status (column 4) to 'inactive' ----
bool softDelete(string roll) {
    return updateStudent(roll, 4, "inactive");
}

// ---- List active students, sorted by roll using SELECTION SORT ----
vector<vector<string> > listActiveStudents() {
    vector<vector<string> > rows = readTXT(STUDENT_FILE);
    vector<vector<string> > active;

    // keep only active students
    for (int i = 0; i < (int)rows.size(); i++) {
        if (rows[i][4] == "active") {
            active.push_back(rows[i]);
        }
    }

    // selection sort by roll number (column 0)
    for (int i = 0; i < (int)active.size(); i++) {
        int smallest = i;
        for (int j = i + 1; j < (int)active.size(); j++) {
            if (active[j][0] < active[smallest][0]) {
                smallest = j;
            }
        }
        // swap active[i] and active[smallest]
        vector<string> temp = active[i];
        active[i] = active[smallest];
        active[smallest] = temp;
    }

    return active;
}