#ifndef COFFER_H
#define COFFER_H
#include<iostream>
#include<cstdlib>
#include<cstdio>
#include<vector>
using namespace std;
class Coffer{
    public:
        string filenames;
        char* data;
        int srcLen; //original total size
        int destLen; //compressed total size
        int eleLen; //size of each elements
        int type; 
        int lines; //# of lines

        unsigned char* cdata;
        unsigned char dictSize[4];
        
        int compressed;
        int offset;
        Coffer();
        Coffer(string filename, char* srcData, int srcL, int line, int typ, int _ele);
        Coffer(string filename, string srcData, int srcL, int line, int typ, int _ele); 
        ~Coffer();
        Coffer(string metaFile);
        int readFile(FILE* zipFile, int fstart); //Read to cdata
        int readFile(unsigned char* cData);

        int compress(); //compress data to cdata
        int decompress(unsigned char method); //decompress cdata to data

        void output(FILE* zipFile, int typ); //output compressed cdata
        void printFile(string rootPath); //output to root Path

        string print();


};
#endif

