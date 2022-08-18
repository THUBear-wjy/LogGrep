#include "sampler.h"
#include "constant.h"
#include "util.h"
#include<cstdio>
#include<iostream>
#include<random>
#include<cstring>
using namespace std;
Sampler::Sampler(string _input_path){
    FILE* fo;
    long lSize;
    if ((fo = initFile(_input_path, lSize)) == NULL){
        return;
    }
    if (fo == NULL){
        cout << "null" << endl;
    }
    //cout << lSize << endl;
    int nowSize = 0;
    char * buffer = (char*)malloc(sizeof(char) * LINE_LENGTH);
    //cout << "here" << endl;
    totLength = 0;
    while(nowSize < lSize){
        int readSize = 0;
        int badChar = Myreadline(fo, readSize, nowSize, lSize, buffer);
        //cout << buffer << endl;
        totLength++;
    }
    free(buffer);
    stringSample = NULL;
    input_path = _input_path;
}
Sampler::~Sampler(){
    freeSample();
}
void Sampler::freeSample(){
    for (int i = 0; i < sampleSize; i++){
        free(stringSample[i]);
    }
    free(stringSample);
}
int Sampler::sample(double rate){
    sampleSize = min(max((int)(totLength * rate),1000), totLength);
    //cout << "totLength: " << totLength << endl;
    if (stringSample != NULL){
        freeSample();
    }
    stringSample = new char*[sampleSize+2];
    FILE* fo; long lSize;
    if ((fo = initFile(input_path, lSize)) == NULL){
        return -1;
    }
    int nowSize = 0;
    char* buffer = (char*)malloc(sizeof(char) * MAX_LENGTH);
    unsigned select = sampleSize;
    unsigned remaining = totLength;
    int nowNum = 0;
    while(nowSize < lSize){
        //cout << "Now size: " << nowSize << " tot size: " << lSize << endl;
        int readSize = 0;
        int badChar = Myreadline(fo, readSize, nowSize, lSize, buffer);
        if (rand() % remaining < select){
            buffer[readSize] = '\0';
            stringSample[nowNum] = new char[readSize+1];
            memcpy(stringSample[nowNum++], buffer, sizeof(char) * (readSize+1));
            select--;
        }
        remaining--;
    }
    free(buffer);
    //cout << "Now num: " << nowNum << " Target select: " << sampleSize << endl;
    return sampleSize;
}
char** Sampler::getSample(){return stringSample;}
int Sampler::getTot(){return totLength;}
int Sampler::getSampleSize(){return sampleSize;}
void Sampler::printSample(){
    for (int i = 0; i < sampleSize; i++){
        printf("%s\n", stringSample[i]);
    }
}