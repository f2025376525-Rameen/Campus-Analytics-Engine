#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <string>
#include <vector>
using namespace std;

// ---- File reading/writing functions ----

// Reads a txt file. Skips the header (first line).
// Returns all data rows; each row is a vector of its comma-separated fields.
vector<vector<string> > readTXT(string filename);

// Overwrites the file: writes the header line first, then all rows.
// If a field contains a comma, it gets wrapped in quotes.
void writeTXT(string filename, string header, vector<vector<string> > rows);

// Adds ONE new row at the end of the file (without loading whole file).
void appendTXT(string filename, vector<string> row);

// Searches the file: returns the first row whose field at keyCol == keyValue.
// Returns an empty vector if nothing matches.
vector<string> findRow(string filename, int keyCol, string keyValue);

// Returns true if ANY row has the given value at column colIndex.
bool rowExists(string filename, int colIndex, string value);

#endif