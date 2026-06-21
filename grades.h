#ifndef GRADES_H
#define GRADES_H

#include <string>
#include <vector>
using namespace std;

// Struct to hold class statistics. (We use struct, NOT class — OOP not allowed.)
struct Stats {
    double highest;
    double lowest;
    double mean;
    double median;
};

// Finds the average of the best 3 out of up to 5 quiz marks.
// (Excludes the two lowest. Handles the case of fewer than 3 quizzes.)
double bestThreeOfFive(vector<double> quizzes);

// Applies weights: quiz*0.10 + asgn*0.10 + mid*0.30 + final*0.50. Out of 100.
double computeWeightedTotal(double quiz, double asgn, double mid, double final_);

// Maps a total (0-100) to a letter grade string.
string getLetterGrade(double total);

// Converts a letter grade into GPA points (A=4.0, B+=3.5, etc.).
double gradeToPoints(string grade);

// Credit-weighted GPA across all courses of a student in a semester.
double computeGPA(string roll, string semester);

// Computes highest, lowest, mean, median from a list of marks.
Stats computeClassStats(vector<double> marks);

// If attendance < 75%, the grade becomes F regardless of marks.
string applyAttendancePenalty(string roll, string courseCode, string grade);

#endif