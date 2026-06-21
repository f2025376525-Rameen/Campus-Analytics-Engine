#include "reports.h"
#include "filehandler.h"
#include "student_ops.h"
#include "grades.h"
#include "attendance.h"
#include "fee_tracker.h"
#include <iostream>
#include <iomanip>
#include <fstream>
using namespace std;

// ---- Helper: numeric string to double ----
double toDouble(string s) {
    double result = 0;
    int i = 0;
    while (i < (int)s.length() && s[i] != '.') {
        if (s[i] >= '0' && s[i] <= '9') result = result * 10 + (s[i] - '0');
        i++;
    }
    if (i < (int)s.length() && s[i] == '.') {
        i++;
        double place = 0.1;
        while (i < (int)s.length()) {
            if (s[i] >= '0' && s[i] <= '9') { result = result + (s[i]-'0')*place; place /= 10; }
            i++;
        }
    }
    return result;
}

// ---- Merit list: active students sorted by CGPA, highest first ----
void printMeritList() {
    vector<vector<string> > rows = readTXT("students.txt");
    vector<vector<string> > active;

    // keep only active students
    for (int i = 0; i < (int)rows.size(); i++) {
        if (rows[i][4] == "active") active.push_back(rows[i]);
    }

    // bubble sort by CGPA (column 3), descending
    for (int i = 0; i < (int)active.size() - 1; i++) {
        for (int j = 0; j < (int)active.size() - 1 - i; j++) {
            if (toDouble(active[j][3]) < toDouble(active[j + 1][3])) {
                vector<string> temp = active[j];
                active[j] = active[j + 1];
                active[j + 1] = temp;
            }
        }
    }

    cout << "\n========== MERIT LIST ==========\n";
    cout << left << setw(6) << "Rank" << setw(15) << "Roll"
         << setw(20) << "Name" << "CGPA\n";
    cout << "--------------------------------------------------\n";
    for (int i = 0; i < (int)active.size(); i++) {
        cout << left << setw(6) << (i + 1)
             << setw(15) << active[i][0]
             << setw(20) << active[i][1]
             << active[i][3] << "\n";
    }
}

// ---- Attendance defaulters: any course attendance below 75% ----
void printAttendanceDefaulters() {
    vector<vector<string> > att = readTXT("attendance_log.txt");
    vector<vector<string> > students = readTXT("students.txt");

    cout << "\n===== ATTENDANCE DEFAULTERS (<75%) =====\n";
    cout << left << setw(15) << "Roll" << setw(20) << "Name"
         << setw(12) << "Course" << "Pct\n";
    cout << "--------------------------------------------------\n";

    // We check each unique roll+course pair once.
    vector<string> seen;   // track pairs we've already printed

    for (int i = 0; i < (int)att.size(); i++) {
        string roll = att[i][0];
        string course = att[i][1];
        string pairKey = roll + "|" + course;

        // skip if we've already handled this roll+course
        bool already = false;
        for (int k = 0; k < (int)seen.size(); k++) {
            if (seen[k] == pairKey) already = true;
        }
        if (already) continue;
        seen.push_back(pairKey);

        double pct = getAttendancePct(roll, course);
        if (pct < 75.0) {
            // find the student's name
            string name = "Unknown";
            for (int s = 0; s < (int)students.size(); s++) {
                if (students[s][0] == roll) name = students[s][1];
            }
            cout << left << setw(15) << roll << setw(20) << name
                 << setw(12) << course << fixed << setprecision(1) << pct << "%\n";
        }
    }
}

// ---- Fee defaulters: outstanding amount + weeks overdue ----
void printFeeDefaulters() {
    vector<string> rolls = getDefaulters();   // already sorted by amount
    vector<vector<string> > fees = readTXT("fees.txt");

    cout << "\n========== FEE DEFAULTERS ==========\n";
    cout << left << setw(15) << "Roll" << setw(15) << "Outstanding" << "\n";
    cout << "-----------------------------------\n";

    for (int i = 0; i < (int)rolls.size(); i++) {
        // find this student's outstanding amount
        for (int j = 0; j < (int)fees.size(); j++) {
            if (fees[j][0] == rolls[i]) {
                double bal = toDouble(fees[j][2]) - toDouble(fees[j][3]);
                cout << left << setw(15) << rolls[i] << setw(15) << bal << "\n";
            }
        }
    }
}

// ---- Full semester result sheet with borders ----
void printSemesterResult(string semester) {
    vector<vector<string> > students = readTXT("students.txt");

    cout << "\n";
    cout << setfill('*') << setw(50) << "" << "\n" << setfill(' ');
    cout << "        SEMESTER RESULT - " << semester << "\n";
    cout << setfill('*') << setw(50) << "" << "\n" << setfill(' ');
    cout << left << setw(15) << "Roll" << setw(20) << "Name"
         << setw(8) << "GPA" << "Status\n";
    cout << "--------------------------------------------------\n";

    for (int i = 0; i < (int)students.size(); i++) {
        if (students[i][4] != "active") continue;

        string roll = students[i][0];
        double gpa = computeGPA(roll, semester);
        string status = (gpa >= 1.0) ? "PASS" : "FAIL";

        cout << left << setw(15) << roll << setw(20) << students[i][1]
             << setw(8) << fixed << setprecision(2) << gpa << status << "\n";
    }
    cout << setfill('*') << setw(50) << "" << "\n" << setfill(' ');
}

// ---- Department summary: count, avg CGPA, pass rate per dept ----
void printDepartmentSummary() {
    vector<vector<string> > students = readTXT("students.txt");

    // parallel arrays: one entry per unique department
    vector<string> deptNames;
    vector<int> deptCount;
    vector<double> deptCgpaSum;
    vector<int> deptPass;

    for (int i = 0; i < (int)students.size(); i++) {
        if (students[i][4] != "active") continue;

        string dept = students[i][2];
        double cgpa = toDouble(students[i][3]);

        // find if this dept is already in our arrays
        int idx = -1;
        for (int d = 0; d < (int)deptNames.size(); d++) {
            if (deptNames[d] == dept) idx = d;
        }
        // new dept -> add it
        if (idx == -1) {
            deptNames.push_back(dept);
            deptCount.push_back(0);
            deptCgpaSum.push_back(0);
            deptPass.push_back(0);
            idx = deptNames.size() - 1;
        }

        deptCount[idx] = deptCount[idx] + 1;
        deptCgpaSum[idx] = deptCgpaSum[idx] + cgpa;
        if (cgpa >= 2.0) deptPass[idx] = deptPass[idx] + 1;  // pass threshold
    }

    cout << "\n========== DEPARTMENT SUMMARY ==========\n";
    cout << left << setw(10) << "Dept" << setw(8) << "Count"
         << setw(12) << "Avg CGPA" << "Pass Rate\n";
    cout << "--------------------------------------------------\n";
    for (int d = 0; d < (int)deptNames.size(); d++) {
        double avg = deptCgpaSum[d] / deptCount[d];
        double passRate = (double)deptPass[d] / deptCount[d] * 100.0;
        cout << left << setw(10) << deptNames[d]
             << setw(8) << deptCount[d]
             << setw(12) << fixed << setprecision(2) << avg
             << setprecision(0) << passRate << "%\n";
    }
}

// ---- Export a report to a .txt file (redirect cout to a file) ----
void exportReportToFile(int reportChoice, string semester) {
    ofstream outFile("report_output.txt");

    // Save the current cout destination, then point cout at our file.
    streambuf* originalCout = cout.rdbuf();
    cout.rdbuf(outFile.rdbuf());

    // Now anything we "cout" goes into the file instead of the screen.
    if (reportChoice == 1) printMeritList();
    else if (reportChoice == 2) printAttendanceDefaulters();
    else if (reportChoice == 3) printFeeDefaulters();
    else if (reportChoice == 4) printSemesterResult(semester);
    else if (reportChoice == 5) printDepartmentSummary();

    // Restore cout back to the screen.
    cout.rdbuf(originalCout);
    outFile.close();

    cout << "Report saved to report_output.txt\n";
}