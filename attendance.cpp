#include "attendance.h"
#include "filehandler.h"
#include "course_ops.h"   // to get the list of enrolled students
#include <iostream>
using namespace std;

string ATT_FILE = "attendance_log.txt";
string ATT_HEADER = "roll,course_code,date,status";

// We keep a backup of the file rows BEFORE marking, so undo can restore it.
// (A simple global variable acts as our "snapshot".)
vector<vector<string> > backupRows;
bool hasBackup = false;

// ---- Mark attendance for all enrolled students on a given date ----
void markAttendance(string courseCode, string semester, string date) {
    // 1. Save a backup of the current file (snapshot before changes)
    backupRows = readTXT(ATT_FILE);
    hasBackup = true;

    // 2. Get the list of enrolled students for this course
    vector<string> rolls = listEnrolledStudents(courseCode, semester);

    // 3. Ask for P / A / L for each student and append the row
    for (int i = 0; i < (int)rolls.size(); i++) {
        string status;
        cout << "Attendance for " << rolls[i] << " (P/A/L): ";
        cin >> status;

        // only accept P, A, or L; default to A if something else
        if (status != "P" && status != "A" && status != "L") {
            status = "A";
        }

        vector<string> row;
        row.push_back(rolls[i]);
        row.push_back(courseCode);
        row.push_back(date);
        row.push_back(status);
        appendTXT(ATT_FILE, row);
    }
    cout << "Attendance saved for " << date << "\n";
}

// ---- Compute attendance %: (present + 0.5*late) / totalSessions * 100 ----
double getAttendancePct(string roll, string courseCode) {
    vector<vector<string> > rows = readTXT(ATT_FILE);

    double present = 0;
    double late = 0;
    double total = 0;

    // accumulator loop: count this student's sessions in this course
    for (int i = 0; i < (int)rows.size(); i++) {
        if (rows[i][0] == roll && rows[i][1] == courseCode) {
            total = total + 1;
            if (rows[i][3] == "P") present = present + 1;
            if (rows[i][3] == "L") late = late + 1;
        }
    }

    if (total == 0) return 0.0;   // avoid divide-by-zero

    return (present + 0.5 * late) / total * 100.0;
}

// ---- List rolls of students with attendance below 75% in a course ----
vector<string> getShortageList(string courseCode, string semester) {
    vector<string> rolls = listEnrolledStudents(courseCode, semester);
    vector<string> shortage;

    for (int i = 0; i < (int)rolls.size(); i++) {
        double pct = getAttendancePct(rolls[i], courseCode);
        if (pct < 75.0) {
            shortage.push_back(rolls[i]);
        }
    }
    return shortage;
}

// ---- Undo the last session: restore the file from the backup snapshot ----
bool undoLastSession() {
    if (!hasBackup) {
        return false;   // nothing to undo
    }
    writeTXT(ATT_FILE, ATT_HEADER, backupRows);   // rewrite from snapshot
    hasBackup = false;  // backup used up
    return true;
}

// ---- Print a formatted table of enrolled students and status for a date ----
void printDailySheet(string courseCode, string semester, string date) {
    vector<string> rolls = listEnrolledStudents(courseCode, semester);
    vector<vector<string> > rows = readTXT(ATT_FILE);

    cout << "\n--- Daily Sheet for " << courseCode << " on " << date << " ---\n";
    cout << "Roll\t\tStatus\n";

    for (int i = 0; i < (int)rolls.size(); i++) {
        string status = "-";   // default if no record for that date
        for (int j = 0; j < (int)rows.size(); j++) {
            if (rows[j][0] == rolls[i] &&
                rows[j][1] == courseCode &&
                rows[j][2] == date) {
                status = rows[j][3];
            }
        }
        cout << rolls[i] << "\t" << status << "\n";
    }
}