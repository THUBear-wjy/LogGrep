#ifndef CONSTANT_H
#define CONSTANT_H

#include <iostream>
#include <cstdlib>
#include <string>
#include <map>

#define POS_TEMPLATE 16
#define POS_VAR 8
#define POS_SUBVAR 4
#define POS_TYPE 0

#define TYPE_DIC 0
#define TYPE_SVAR 1
#define TYPE_VAR 2
#define TYPE_ENTRY 3
#define TYPE_META 4
#define TYPE_TEMPLATE 5
#define TYPE_VARIABLELIST 6
#define TYPE_OUTLIER 7

#define MAXLOG 100000 //The max number of log entry
#define MAX_VALUE_LEN  10000
#define MAX_FILE_NAMELEN   512 
#define TOTLINE 100000 //Total log line

#define MAXTEMPLATE 1000  //The max number of templates
#define MAX_LENGTH 3000000 //Max length of signle log line
#define VAR_LENGTH 1000 //Max length of single variable

#define MAXTOCKEN 2048 //The max number of tocken(including tonks )
#define MAXCOL 256 //The number of variable(int, string)
#define MAXUNION 1024
#define MAXCOMPRESS 16*1024*1024

#define LINE_LENGTH 10000000 //The length of log read buffer
#define MAXSTRLEN 1024 //The max number of char in string parameter
#define BUFSIZE 1024 //String reading buffer
#define TEMPLATE_THRESHOLD 64
#define MAXBUFFER 100000000

#define SysDebug //printf
#define SysWarning printf
using namespace std;
typedef struct VarArray
{
    int tag;
    int * startPos;
    int * len;
    int totsize;
    int nowPos;
    VarArray(int _tag, int initSize)
    {
        tag = _tag;
        startPos = new int[initSize];
        len = new int[initSize];
        totsize = initSize;
        nowPos = 0;
    }
    ~VarArray(){
        free(startPos);
        free(len);
    }
    void Add(int startpos, int length)
    {
        if(nowPos == totsize){ //Full
            int* nstartPos = new int[totsize * 2];
            int* nlen = new int[totsize * 2];
            for(int i = 0; i < totsize; i++){
                nstartPos[i] = startPos[i];
                nlen[i] = len[i];
            }       
            free(startPos);
            free(len);
            startPos = nstartPos;
            len = nlen;    
            totsize = totsize * 2;
        }
        startPos[nowPos] = startpos;
        len[nowPos] = length;
        nowPos++;
    }
}VarArray;


//record read segment attribute
typedef struct SegTag
{
    int startPos;//start position of read segment in original file
    int segLen;//read segment length
    int tag;//read segment tag
}SegTag;


#endif
