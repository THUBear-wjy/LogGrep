#ifndef ENCODER_H
#define ENCODER_H
#include<iostream>
#include<string>
#include<cstdlib>
#include<vector>
#include"Coffer.h"
#include"LengthParser.h"
#include"SubPattern.h"
#include"union.h"

using namespace std;
class Encoder{
    private:
        //string Meta;
        vector<Coffer*> data;
        Coffer* merge(int* ids); //TODO: Merge several small coffers
        string compress(); //Build meta string, merge coffers
        string padding(string filename, int Idx, int maxIdx);
        string padding(string filename, string target, int maxLen, int typ);
        void padding(string filename, char* buffer, int startPos, int padSize, int typ);
    public:
        bool _padding;
        bool _meta_out;

        Encoder(string __padding, string zip_mode);

        //Compression 
        void serializeTemplate(string zip_out, LengthParser* parser); 
        void serializeTemplateOutlier(char** failed_log, int failLine);
        
        void serializeVar(string filename,char* globuf, VarArray* varMapping, int maxLen);
        void serializeEntry(string filename, int * entry, int maxEntry, int total);
        void serializeDic(string varName, char* globuf, VarArray* varMapping, Union* root); //Compress each dictioanry
        void serializeSvar(string filename, SubPattern* pit); //Compress subvariable
        void serializeOutlier(string filename, vector<pair<int, string> >outliers);
        
        void serializeSubpattern(string zip_path, string SUBPATTERN, int SUBCOUNT);

        //Output
        void output(string zip_path, int typ);

};
#endif

