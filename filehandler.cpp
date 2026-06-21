#include "filehandler.h"
#include <fstream>   // for ifstream / ofstream (file reading/writing)
#include <iostream>
using namespace std;

// Helper: breaks one line like "BSAI-23-001,Ali,AI" into separate fields.
// Reads the line CHARACTER BY CHARACTER (no getline-split shortcut).
// Also handles fields wrapped in quotes (so commas inside quotes don't split).
vector<string> splitLine(string line) {
    vector<string> fields;
    string current = "";        // builds up the current field
    bool insideQuotes = false;  // are we inside a "..." field?

    for (int i = 0; i < (int)line.length(); i++) {
        char c = line[i];

        if (c == '"') {
            insideQuotes = !insideQuotes;   // flip: entering or leaving quotes
        }
        else if (c == ',' && !insideQuotes) {
            fields.push_back(current);       // comma = end of this field
            current = "";
        }
        else {
            current = current + c;           // normal character, add to field
        }
    }
    fields.push_back(current);  // push the last field after the loop
    return fields;
}

// Reads the whole file. Skips the header line. Returns all data rows.
vector<vector<string> > readTXT(string filename) {
    vector<vector<string> > rows;
    ifstream file(filename.c_str());

    if (!file.is_open()) {       // file missing? return empty
        return rows;
    }

    string line;
    bool isHeader = true;        // first line is the header
    while (getline(file, line)) {
        if (isHeader) {          // skip header row
            isHeader = false;
            continue;
        }
        if (line.length() == 0) continue;   // skip blank lines
        rows.push_back(splitLine(line));
    }
    file.close();
    return rows;
}

// Helper: if a field has a comma, wrap it in quotes before saving.
string protectField(string field) {
    bool hasComma = false;
    for (int i = 0; i < (int)field.length(); i++) {
        if (field[i] == ',') hasComma = true;
    }
    if (hasComma) {
        return "\"" + field + "\"";
    }
    return field;
}

// Overwrites the file: header first, then every row joined by commas.
void writeTXT(string filename, string header, vector<vector<string> > rows) {
    ofstream file(filename.c_str());   // opening with ofstream erases old content
    file << header << "\n";

    for (int i = 0; i < (int)rows.size(); i++) {
        for (int j = 0; j < (int)rows[i].size(); j++) {
            file << protectField(rows[i][j]);
            if (j < (int)rows[i].size() - 1) {
                file << ",";           // comma between fields, not after the last
            }
        }
        file << "\n";
    }
    file.close();
}

// Adds a single row at the end without loading the file. (ios::app = append mode)
void appendTXT(string filename, vector<string> row) {
    ofstream file(filename.c_str(), ios::app);
    for (int j = 0; j < (int)row.size(); j++) {
        file << protectField(row[j]);
        if (j < (int)row.size() - 1) {
            file << ",";
        }
    }
    file << "\n";
    file.close();
}

// Linear search: returns the first row where column keyCol equals keyValue.
vector<string> findRow(string filename, int keyCol, string keyValue) {
    vector<vector<string> > rows = readTXT(filename);
    for (int i = 0; i < (int)rows.size(); i++) {
        if (keyCol < (int)rows[i].size() && rows[i][keyCol] == keyValue) {
            return rows[i];          // found it
        }
    }
    vector<string> empty;            // not found -> empty vector
    return empty;
}

// Returns true if any row has 'value' at column colIndex.
bool rowExists(string filename, int colIndex, string value) {
    vector<vector<string> > rows = readTXT(filename);
    for (int i = 0; i < (int)rows.size(); i++) {
        if (colIndex < (int)rows[i].size() && rows[i][colIndex] == value) {
            return true;
        }
    }
    return false;
}