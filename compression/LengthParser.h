#ifndef LENGTHSEARCH_H
#define LENGTHSEARCH_H
#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <bitset>
#include <iostream>
#include "constant.h"
#include "template.h"
using namespace std;

class LengthParser {
private:
    const char* delim; //Fix delimer
    int delimCount;
    char** DELIM; //Used for different delimer
    int now_eid;
    double threashold;
    int headLength;
public:
    map<int, std::vector<templateNode*>* > LengthTemplatePool;
    map<int, int> TC; //template counter
    map<int, int> STC; //sample tempalte counter

    LengthParser(double _threashold);
    ~LengthParser();
    int addTemplate(vector<string> tokens, int length);                 // the function of adding a template
    int parseTemplate(char* log, SegTag segArray[MAXTOCKEN], int token_size);
    int SearchTemplate(char* logs, SegTag segArray[MAXTOCKEN], int segSize, map<int, VarArray*>& variables, bool extract);
    
    void TemplateOutput(string intpu_path);
    int getTemplate(char** longStr);
    void counterReset();
    void TemplatePrint();
    int STCTC(double sampleRate);
    void STCTCOut(string output_path, double sampleRate);

};
#endif // lengthsearch header
