#ifndef UNION_H
#define UNION_H
#include<iostream>
#include<vector>
#include<map>
#include<set>
#include"constant.h"
using namespace std;

class Union{
public:
    int num; //The # of union
    int type; //Empty -> -3, Unchecked -> -2, Splited -> -1, Constant -> 0, Fixed int -> 1, Fixed str -> 2, var int-> 3, var str-> 4
    int tot; //The total size of union
    int hex; //is hex?
    //int outlierNum; //Size of outliers
    //int* outlier; // record outlier index
    unsigned int * HashValue;
    unsigned int * UniquePos;

private:
    set<int> outlier;
    char** data;


    static bool pairCmp(pair<unsigned int, int> t1, pair<unsigned int, int> t2);
    static bool formatCmp(pair<string, int> t1, pair<string, int> t2);
    static bool rpairCmp(pair<unsigned int, int> t1, pair<unsigned int, int> t2);
   // int getHex();

    //char* getFormat(char* target);
    char* getLCS(char* t1, char* t2);
    int testSpliter(char tester); 
    //int testType(char* format);
    int testLCS(char* tester, int size);
    

    int** matchSpliter(char spliter);
    //int** matchType(char* format);
    int** matchLCS(char* LCS);

    int split(Union** tree, int nowEnd);
    int equeue(Union** tree, int nowEnd, int** delimer, int parts);
    
    unsigned int _stringHash_(const char* start, int len);
public:
    char* globalMem;
    int _tot;

    const double threashold = 0.9;
    const double uniqueRate = 0.5;
    const int maxDicPat = 4;
    string nowFormat[5];
    int nowPaddingSize[5];
    int nowCounter[5];
    //static bool isHex(string temp);
    //static int getType(char c);
    //static bool isInt(char* temp);
    
    static bool unionCmp(Union& u1, Union& u2);
    
    vector<pair<unsigned int, int> >* getContainer();

    //Dictioanry 
    map<unsigned int, int> dictionary;
    map<unsigned int, int> posDictionary;
    map<string, int> format_counter;

    int dicMax;
    int patCount;
    string outputDic(VarArray* varMapping);
    //bool buildDic(VarArray varMapping);
    void buildMapping(VarArray* varMapping);
    string getFormat(const char* start, int len);
    
    //Sub-pattern
    string constant;
    int length;

    Union(char* globuf, VarArray* varMapping, double _rate); // Sample first union
    Union(char** _data, int _num, int _type, int _tot, set<int> _outlier); // Build union during spliting
    ~Union();
    
    //bool operator< (Union& uni);
    double getUniqueRate();
    int getNum();
    
    bool execute(Union** tree, int nowStart, int& nowEnd); // 1) try to split, 2) if can not split, fix type
    void output();
};
#endif 
