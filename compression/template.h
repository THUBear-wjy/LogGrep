#ifndef TEMPLATENODE_H
#define TEMPLATENODE_H

#include <iostream>
#include <string>
#include <set>
#include <string>
#include <vector>
#include "constant.h"

using namespace std;

class templateNode{
public:
    // static set<string> split;//加一个固定的set，split
    static int sum;
    // static templateNode **allTemplates;
	// static int ***intData;
	// static string ***stringData;
	// static int *topPointer;
	int Eid;
    char ** templates;
    int* templatesTags;
    int length;
	int varLength;
	vector<int> varIndex;
    int delimBitmap[128];
    //constructor, need Eid, tokens,and length
    templateNode(int Eid, char* log, SegTag segArray[MAXTOCKEN], int token_size);
    ~templateNode();
    //the match function foe match,need length
    
    int matchMatch(char* tockens, SegTag segArray[MAXTOCKEN], int tocken_size, int logLength);
    double parseMatch(char* tokens, SegTag segArray[MAXTOCKEN], int token_size);
    void merge(char* tokens, SegTag segArray[MAXTOCKEN], int token_size);
    static void reset();
	void initPara();
	string print();
    string output();
};   

#endif
