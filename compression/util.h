#ifndef UTIL
#define UTIL
#define NUM_TY 1
#define AF_TY 2
#define af_TY 4
#define GZ_TY 8
#define gz_TY 16
#define symbol_TY 32
#include<cstdlib>
#include<cstdio>
#include<ctype.h>
#include<iostream>
using namespace std;

FILE* initFile(string input_path, long& lSize);
int Myreadline(FILE* in, int& readSize, int& nowSize, long totSize, char *buffer);
int Mystrtok(char *s, const char *delim, char* &buf);
int Myatoi(char *nptr, int max_length, int &readLength);
int getTypeC(char c);
int getType(const char* temp, int len);

#endif
