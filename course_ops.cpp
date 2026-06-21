#include "course_ops.h"
#include "filehandler.h"
#include <iostream>
using namespace std;

// ---- File names and their column orders (for reference) ----
// students.txt    -> roll,name,dept,cgpa,status              (status at col 4)
// courses.txt     -> code,title,credits,seats,prereq         (prereq at col 4)
// enrollments.txt -> roll,course_code,semester,status        (status at col 3)
// attendance_log  -> roll,course_code,date,status
string COURSE_FILE = "courses.txt";
string ENROLL_FILE = "enrollments.txt";
string ENROLL_HEADER = "roll,course_code,semester,status";

// ---- Helper: convert a numeric string like "3" to an int (manual) ----
int stringToInt(string s) {
    int result = 0;
    for (int i = 0; i < (int)s.length(); i++) {
        if (s[i] >= '0' && s[i] <= '9') {
            result = result * 10 + (s[i] - '0');
        }
    }
    return result;
}

// ---- Sum credit hours of all enrolled courses in a semester ----
// Uses a NESTED LOOP over enrollments and courses files.
int getCreditLoad(string roll, string semester) {
    vector<vector<string> > enrolls = readTXT(ENROLL_FILE);
    vector<vector<string> > courses = readTXT(COURSE_FILE);
    int total = 0;

    for (int i = 0; i < (int)enrolls.size(); i++) {
        // match roll, semester, and status == enrolled
        if (enrolls[i][0] == roll &&
            enrolls[i][2] == semester &&
            enrolls[i][3] == "enrolled") {

            string code = enrolls[i][1];
            // find this course's credits in courses.txt
            for (int j = 0; j < (int)courses.size(); j++) {
                if (courses[j][0] == code) {
                    total = total + stringToInt(courses[j][2]); // credits at col 2
                }
            }
        }
    }
    return total;
}

// ---- Check the prerequisite for a course ----
// If prereq is NONE -> OK. Otherwise student must have a non-F grade in it.
bool checkPrerequisite(string roll, string courseCode) {
    vector<string> course = findRow(COURSE_FILE, 0, courseCode);
    if (course.size() == 0) return false;   // course not found

    string prereq = course[4];   // prereq at column 4
    if (prereq == "NONE") {
        return true;             // no prerequisite needed
    }

    // Check grades.txt for a non-F grade in the prereq course.
    // grades.txt assumed columns: roll,course_code,grade  (we read if it exists)
    vector<vector<string> > grades = readTXT("grades.txt");
    for (int i = 0; i < (int)grades.size(); i++) {
        if (grades[i].size() >= 3 &&
            grades[i][0] == roll &&
            grades[i][1] == prereq &&
            grades[i][2] != "F") {
            return true;          // passed the prerequisite
        }
    }
    return false;                 // prereq not passed
}

// ---- Count how many students are actively enrolled in a course ----
int countEnrolled(string courseCode, string semester) {
    vector<vector<string> > enrolls = readTXT(ENROLL_FILE);
    int count = 0;
    for (int i = 0; i < (int)enrolls.size(); i++) {
        if (enrolls[i][1] == courseCode &&
            enrolls[i][2] == semester &&
            enrolls[i][3] == "enrolled") {
            count++;
        }
    }
    return count;
}

// ---- Try to enroll a student. Runs ALL the checks in order. ----
EnrollResult enrollStudent(string roll, string courseCode, string semester) {
    // 1. Student must exist and be active
    vector<string> student = findRow("students.txt", 0, roll);
    if (student.size() == 0 || student[4] != "active") {
        return ERR_STUDENT_INACTIVE;
    }

    // 2. Course must exist
    vector<string> course = findRow(COURSE_FILE, 0, courseCode);
    if (course.size() == 0) {
        return ERR_COURSE_NOT_FOUND;
    }

    // 3. Seats available? (seats at col 3)
    int seats = stringToInt(course[3]);
    if (countEnrolled(courseCode, semester) >= seats) {
        return ERR_NO_SEATS;
    }

    // 4. Not already enrolled in the same course+semester
    vector<vector<string> > enrolls = readTXT(ENROLL_FILE);
    for (int i = 0; i < (int)enrolls.size(); i++) {
        if (enrolls[i][0] == roll &&
            enrolls[i][1] == courseCode &&
            enrolls[i][2] == semester &&
            enrolls[i][3] == "enrolled") {
            return ERR_ALREADY_ENROLLED;
        }
    }

    // 5. Credit load + this course's credits must stay <= 21
    int currentLoad = getCreditLoad(roll, semester);
    int thisCredits = stringToInt(course[2]);
    if (currentLoad + thisCredits > 21) {
        return ERR_CREDIT_OVERLOAD;
    }

    // 6. Prerequisite must be met
    if (!checkPrerequisite(roll, courseCode)) {
        return ERR_PREREQ_NOT_MET;
    }

    // All checks passed -> add enrollment row
    vector<string> row;
    row.push_back(roll);
    row.push_back(courseCode);
    row.push_back(semester);
    row.push_back("enrolled");
    appendTXT(ENROLL_FILE, row);

    return ENROLL_SUCCESS;
}

// ---- Drop a course (only if no attendance rows exist for it) ----
bool dropCourse(string roll, string courseCode, string semester) {
    // Check attendance_log: if any session exists, dropping is not allowed.
    vector<vector<string> > att = readTXT("attendance_log.txt");
    for (int i = 0; i < (int)att.size(); i++) {
        if (att[i][0] == roll && att[i][1] == courseCode) {
            return false;   // attendance exists -> cannot drop
        }
    }

    // Find the enrollment row and set status to 'dropped'
    vector<vector<string> > enrolls = readTXT(ENROLL_FILE);
    bool found = false;
    for (int i = 0; i < (int)enrolls.size(); i++) {
        if (enrolls[i][0] == roll &&
            enrolls[i][1] == courseCode &&
            enrolls[i][2] == semester &&
            enrolls[i][3] == "enrolled") {
            enrolls[i][3] = "dropped";
            found = true;
        }
    }

    if (found) {
        writeTXT(ENROLL_FILE, ENROLL_HEADER, enrolls);
    }
    return found;
}

// ---- List rolls of all active enrolled students in a course ----
vector<string> listEnrolledStudents(string courseCode, string semester) {
    vector<vector<string> > enrolls = readTXT(ENROLL_FILE);
    vector<string> rolls;
    for (int i = 0; i < (int)enrolls.size(); i++) {
        if (enrolls[i][1] == courseCode &&
            enrolls[i][2] == semester &&
            enrolls[i][3] == "enrolled") {
            rolls.push_back(enrolls[i][0]);
        }
    }
    return rolls;
}

// ---- Convert an EnrollResult code into a readable message ----
string enrollResultToText(EnrollResult r) {
    if (r == ENROLL_SUCCESS)        return "Enrollment successful";
    if (r == ERR_STUDENT_INACTIVE)  return "Student not found or inactive";
    if (r == ERR_COURSE_NOT_FOUND)  return "Course does not exist";
    if (r == ERR_NO_SEATS)          return "No seats available";
    if (r == ERR_ALREADY_ENROLLED)  return "Already enrolled in this course";
    if (r == ERR_CREDIT_OVERLOAD)   return "Credit load would exceed 21 hours";
    if (r == ERR_PREREQ_NOT_MET)    return "Prerequisite not met";
    return "Unknown result";
}