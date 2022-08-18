#ifndef SUBPATTERN_H
#define SUBPATTERN_H

#include "constant.h"
#include "union.h"

using namespace std;

class SubPattern{
public:
    string * data;
    int data_count;

    string const_pattern;
    string nextConstant;
    string readyAdd;

    int type; //0 for constant, 1 for fixed length, 2 for others
    int dataType;
    char nextChar;
    int patternLength;
    int length;
    int hex;
    int maxLen;

    SubPattern(Union* now, string nextConstant, int maxData);
    
    bool extract(string log, int & strIdx, bool & jump);
    void add(bool success);

    void output();
    
    string getPattern();

    bool output_var(string path);
};
#endif // lengthsearch header
