#ifndef SAMPLE_H
#define SAMPLE_H
#include<string>
#include<cstdlib>
#include<cstdio>
#include<iostream>
using namespace std;
class Sampler{
private:
    char** stringSample;
    int totLength;
    int sampleSize;
    string input_path;
    void freeSample();
public:
    Sampler();
    Sampler(string intput_path);
    int sample(double sample_rate);
    int getTot();
    int getSampleSize();
    char** getSample();
    void printSample();
    ~Sampler();
};
#endif