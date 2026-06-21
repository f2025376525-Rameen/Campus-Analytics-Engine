#include "grades.h"
#include "filehandler.h"
#include "attendance.h"   // to check attendance % for the penalty
#include <iostream>
using namespace std;

// ---- Best 3 out of 5 quizzes ----
// We find and exclude the TWO LOWEST values, then average the rest.
double bestThreeOfFive(vector<double> quizzes) {
    int n = quizzes.size();

    // Edge case: fewer than 3 quizzes -> just average whatever we have.
    if (n < 3) {
        if (n == 0) return 0.0;
        double sum = 0;
        for (int i = 0; i < n; i++) sum = sum + quizzes[i];
        return sum / n;
    }

    // Make a copy so we can "remove" the two lowest by marking them used.
    vector<double> temp = quizzes;

    // Remove the lowest value TWICE (no sort allowed, so we search each time).
    for (int round = 0; round < 2; round++) {
        int lowIndex = 0;
        for (int i = 1; i < (int)temp.size(); i++) {
            if (temp[i] < temp[lowIndex]) {
                lowIndex = i;
            }
        }
        // erase the lowest by shifting the rest left
        for (int i = lowIndex; i < (int)temp.size() - 1; i++) {
            temp[i] = temp[i + 1];
        }
        temp.pop_back();   // remove last (now duplicate) element
    }

    // average the remaining values
    double sum = 0;
    for (int i = 0; i < (int)temp.size(); i++) sum = sum + temp[i];
    return sum / temp.size();
}

// ---- Weighted total out of 100 ----
double computeWeightedTotal(double quiz, double asgn, double mid, double final_) {
    return quiz * 0.10 + asgn * 0.10 + mid * 0.30 + final_ * 0.50;
}

// ---- Map total to a letter grade ----
string getLetterGrade(double total) {
    if (total >= 85) return "A";
    if (total >= 80) return "B+";
    if (total >= 70) return "B";
    if (total >= 65) return "C+";
    if (total >= 60) return "C";
    if (total >= 50) return "D";
    return "F";
}

// ---- Convert a letter grade to GPA points ----
double gradeToPoints(string grade) {
    if (grade == "A")  return 4.0;
    if (grade == "B+") return 3.5;
    if (grade == "B")  return 3.0;
    if (grade == "C+") return 2.5;
    if (grade == "C")  return 2.0;
    if (grade == "D")  return 1.0;
    return 0.0;   // F
}

// ---- Helper: convert numeric string to int (for credits) ----
int strToInt(string s) {
    int result = 0;
    for (int i = 0; i < (int)s.length(); i++) {
        if (s[i] >= '0' && s[i] <= '9') {
            result = result * 10 + (s[i] - '0');
        }
    }
    return result;
}

// ---- Credit-weighted GPA across a student's courses in a semester ----
// grades.txt assumed columns: roll,course_code,grade,semester
double computeGPA(string roll, string semester) {
    vector<vector<string> > grades = readTXT("grades.txt");
    vector<vector<string> > courses = readTXT("courses.txt");

    double totalPoints = 0;   // sum of (gpa_points * credits)
    double totalCredits = 0;

    for (int i = 0; i < (int)grades.size(); i++) {
        if (grades[i].size() >= 4 &&
            grades[i][0] == roll &&
            grades[i][3] == semester) {

            string code  = grades[i][1];
            string grade = grades[i][2];
            double points = gradeToPoints(grade);

            // find this course's credits
            for (int j = 0; j < (int)courses.size(); j++) {
                if (courses[j][0] == code) {
                    int credits = strToInt(courses[j][2]);
                    totalPoints = totalPoints + points * credits;
                    totalCredits = totalCredits + credits;
                }
            }
        }
    }

    if (totalCredits == 0) return 0.0;
    return totalPoints / totalCredits;
}

// ---- Class statistics: highest, lowest, mean, median ----
Stats computeClassStats(vector<double> marks) {
    Stats s;
    s.highest = 0; s.lowest = 0; s.mean = 0; s.median = 0;

    int n = marks.size();
    if (n == 0) return s;   // empty -> all zero

    // highest, lowest, and sum for mean
    s.highest = marks[0];
    s.lowest = marks[0];
    double sum = 0;
    for (int i = 0; i < n; i++) {
        if (marks[i] > s.highest) s.highest = marks[i];
        if (marks[i] < s.lowest)  s.lowest = marks[i];
        sum = sum + marks[i];
    }
    s.mean = sum / n;

    // median needs sorted data -> bubble sort a copy (no library sort)
    vector<double> sorted = marks;
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            if (sorted[j] > sorted[j + 1]) {
                double t = sorted[j];
                sorted[j] = sorted[j + 1];
                sorted[j + 1] = t;
            }
        }
    }
    // middle value (or average of two middle values if even count)
    if (n % 2 == 1) {
        s.median = sorted[n / 2];
    } else {
        s.median = (sorted[n / 2 - 1] + sorted[n / 2]) / 2.0;
    }

    return s;
}

// ---- Attendance penalty: below 75% -> grade becomes F ----
string applyAttendancePenalty(string roll, string courseCode, string grade) {
    double pct = getAttendancePct(roll, courseCode);
    if (pct < 75.0) {
        return "F";
    }
    return grade;
}