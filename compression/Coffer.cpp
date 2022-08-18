#include "Coffer.h"
#include"LZMA/LzmaLib.h"
#include<cstring>
#include<cstdio>
#include<cstdlib>
using namespace std;
Coffer::Coffer(string filename, char* srcData, int srcL, int line, int typ, int ele){
    data = srcData;
    filenames = filename;
    cdata = NULL;
    srcLen = srcL;
    lines = line;
    type = typ;
    eleLen = ele;
    compressed = 0;
}

Coffer::Coffer(string filename, string srcData, int srcL, int line, int typ, int ele){
//cout << "Build coffer" << filename << " " << line << endl;
    data = new char[srcL + 5];
    memcpy(data, srcData.c_str(), sizeof(char)*srcL);
    data[srcL] = '\0';
    filenames = filename;
    cdata = NULL;
    srcLen = srcL;
    lines = line;
    eleLen = ele;
    type = typ;
    compressed = 0;
}

Coffer::Coffer(string metaStr){
    data = NULL;
    cdata = NULL;
    type = -1;
   // cout << "Build based: " << metaStr << endl;
    char filename[128];
    int _compressed, _offset, _destLen, _srcLen, _lines, _eleLen;
    sscanf(metaStr.c_str(), "%s %d %d %d %d %d %d", filename, &_compressed, &_offset, &_destLen, &_srcLen, &_lines, &_eleLen);
    //cout << filename << endl;
    //cout << _compressed << endl;
    //cout << _compressed << _offset << _destLen << _srcLen << _lines << _eleLen << endl;
    filenames = filename;
    compressed = _compressed;
    offset = _offset;
    destLen = _destLen;
    srcLen = _srcLen;
    lines = _lines;
    eleLen = _eleLen;
}

Coffer::~Coffer()
{
    if(data) free(data);
    if(cdata) free(cdata);
}

int Coffer::compress(){
    //cout << "Here1 size: " << this -> lines << endl;
    int tempLines = this -> lines;
    size_t sizeProp = 5;
    //unsigned char*buffer = new unsigned char[srcLen*2];
    cdata = new unsigned char[srcLen * 2];
    unsigned char prop[5]; 
    size_t tempD;
    int res = LzmaCompress(cdata, (size_t*)&tempD, (unsigned char*)data, srcLen, prop, &sizeProp, 5, (1 << 24), 3, 0, 2, 32, 2);
    destLen = tempD;
    if (res != SZ_OK){
        //memcpy(buffer, data, sizeof(char) * srcLen);
        cout << "Coffer: " << filenames << " compress failed" << endl;
        memcpy(cdata, data, sizeof(char)*srcLen);
        destLen = srcLen;
        this -> lines = tempLines;
        return destLen;
    }
    for(int i = 1; i < 5; i ++) dictSize[i - 1] = prop[i];
    compressed = 1;
   // this -> lines = tempLines;
    //cout << "Here: " << this -> print() << endl;
    return destLen + 4;
}

int Coffer::readFile(FILE* zipFile, int fstart){
    int totOffset = fstart + this->offset;
    fseek(zipFile, totOffset, SEEK_SET);
    //dictSize = new unsigned char[4];
    fread(dictSize, sizeof(char), 4, zipFile);
    cdata = new unsigned char[destLen + 5];
    return fread(cdata, sizeof(char), destLen, zipFile);
}

int Coffer::readFile(unsigned char* cData){
    memcpy(dictSize, cData, 4);
    cdata = cData + 4;
    return 1;
}

int Coffer::decompress(unsigned char method){
    if(compressed == 0){
        if(srcLen == 0) return 0;
        data = new char[srcLen + 5];
        memcpy(data ,cdata , sizeof(char)*srcLen);
        return srcLen;
    }
    unsigned char prop[5];
    prop[0] = method;
    for(int i = 1; i < 5; i++) prop[i] = dictSize[i - 1];
    data = new char[srcLen + 5];
    size_t _srcLen = srcLen;
    size_t _destLen = destLen;
    int res = LzmaUncompress((unsigned char*)data, &_srcLen, cdata, &_destLen, prop, 5);
    if(res != SZ_OK){
        cout << "Coffer decompressed failed with: " << res << endl;
        return -1;
    }
    return srcLen;
}

void Coffer::output(FILE* zipFile, int typ){
    if(cdata == NULL || zipFile == NULL){
        cout << "coffer: " + filenames + " output failed" << endl;
        return;
    }
    fwrite(dictSize, sizeof(char), 4, zipFile);
    fwrite(cdata, sizeof(char), destLen, zipFile);
}

string Coffer::print(){
    string name = filenames;
    // int outputSize = (srcLen > 20) ? 20: srcLen;
    // if(data != NULL) name += " data: " + string(data, outputSize);
    // name += "\ntype: " + to_string(this -> type);
    // name += " srcLen: " + to_string(this -> srcLen);
    // name += " destLen: " + to_string(this -> destLen);
    // name += " lines: " + to_string(this -> lines);
    // name += " eleLen: " + to_string(this -> eleLen);
    // name += "\n";
    return name;
}

void Coffer::printFile(string output_path){
    FILE* pFile = fopen((output_path + filenames).c_str(), "w");
    fwrite(data, sizeof(char), srcLen, pFile);
    fclose(pFile);
}

