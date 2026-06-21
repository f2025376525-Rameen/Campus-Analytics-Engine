#ifndef REPORTS_H
#define REPORTS_H

#include <string>
using namespace std;

// All active students sorted by CGPA (descending), with a rank column.
void printMeritList();

// Students with any course attendance below 75%. Shows roll, name, course, %.
void printAttendanceDefaulters();

// Fee defaulters with outstanding amount and weeks overdue.
void printFeeDefaulters();

// Full result sheet with borders: grade, GPA, attendance status per student.
void printSemesterResult(string semester);

// Groups students by department: count, average CGPA, pass rate per dept.
void printDepartmentSummary();

// Redirects cout to a file, prints a chosen report, then restores cout.
void exportReportToFile(int reportChoice, string semester);

#endif