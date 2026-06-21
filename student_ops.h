#ifndef STUDENT_OPS_H
#define STUDENT_OPS_H

#include <string>
#include <vector>
using namespace std;

// Adds a new student after validating roll, name, and cgpa.
// Returns a message saying success or what went wrong.
string addStudent(string roll, string name, string dept, double cgpa);

// Finds a student by roll number. Returns their row, or empty if not found.
vector<string> searchByRoll(string roll);

// Returns all students whose NAME contains the given text (substring match).
vector<vector<string> > searchByName(string partialName);

// Updates one field (by column index) of a student. Roll cannot be changed.
// Returns true if the student was found and updated.
bool updateStudent(string roll, int fieldIndex, string newValue);

// Soft-delete: sets the student's status to 'inactive' (row stays in file).
bool softDelete(string roll);

// Returns all active students, sorted by roll number (selection sort).
vector<vector<string> > listActiveStudents();

#endif