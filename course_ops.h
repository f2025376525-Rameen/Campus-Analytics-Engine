#ifndef COURSE_OPS_H
#define COURSE_OPS_H

#include <string>
#include <vector>
using namespace std;

// Result of trying to enroll. Tells us success or the exact failure reason.
enum EnrollResult {
    ENROLL_SUCCESS,
    ERR_STUDENT_INACTIVE,
    ERR_COURSE_NOT_FOUND,
    ERR_NO_SEATS,
    ERR_ALREADY_ENROLLED,
    ERR_CREDIT_OVERLOAD,
    ERR_PREREQ_NOT_MET
};

// Sums credit hours of all active (enrolled) courses for a student in a semester.
int getCreditLoad(string roll, string semester);

// Checks the course prerequisite. Returns true if met (or prereq is NONE).
bool checkPrerequisite(string roll, string courseCode);

// Tries to enroll a student in a course. Returns an EnrollResult code.
EnrollResult enrollStudent(string roll, string courseCode, string semester);

// Drops a course (only if no attendance exists for it). Returns true if dropped.
bool dropCourse(string roll, string courseCode, string semester);

// Returns rolls of all students actively enrolled in a course.
vector<string> listEnrolledStudents(string courseCode, string semester);

// Helper: converts an EnrollResult code into a readable message.
string enrollResultToText(EnrollResult r);

#endif