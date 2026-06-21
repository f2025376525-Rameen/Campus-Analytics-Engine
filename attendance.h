#ifndef ATTENDANCE_H
#define ATTENDANCE_H

#include <string>
#include <vector>
using namespace std;

// Marks attendance for all enrolled students in a course on a given date.
// Saves a backup of the file first (so we can undo). Appends rows.
void markAttendance(string courseCode, string semester, string date);

// Computes attendance %: (present + 0.5*late) / totalSessions * 100.
double getAttendancePct(string roll, string courseCode);

// Returns rolls of students whose attendance in a course is below 75%.
vector<string> getShortageList(string courseCode, string semester);

// Undoes the last marked session by restoring from backup. False if no backup.
bool undoLastSession();

// Prints a formatted table of all enrolled students and status for a date.
void printDailySheet(string courseCode, string semester, string date);

#endif