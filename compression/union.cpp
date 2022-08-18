#include "union.h"
#include "util.h"
#include<iostream>
#include<random>
#include<cstdlib>
#include<map>
#include"assert.h"
#include<cstring>
#include<string.h>
#include<ctype.h>
#include<algorithm>
using namespace std;
Union::Union(char* globuf, VarArray* varMapping, double _rate){ //Start construct
    globalMem = globuf;
    _tot = varMapping ->nowPos;
    
    int select = min(max((int)(_tot * _rate),1000), _tot);
    int remaining = _tot;
    int nowSize = 0;
    data = new char* [select];
    HashValue = new unsigned int[_tot];
    UniquePos = new unsigned int[_tot];
    int nowIdx = 0;
    for(int i=0; i< _tot; i++){   
        if(dictionary.size() < _tot * uniqueRate){
            int varLen = varMapping->len[i];
            unsigned int hashValue = _stringHash_(globalMem + varMapping->startPos[i], varLen);
            HashValue[i] = hashValue;
            if(dictionary.find(hashValue) == dictionary.end()){
                UniquePos[nowIdx++] = i;
                dictionary.insert(pair<unsigned int, int>(hashValue, -1));
                posDictionary.insert(pair<unsigned int, int>(hashValue, i));
            }
        }
        //cout << "remaining: " << remaining << " select: " << select << endl;
        if (rand() % remaining < select && nowSize < select){
            int varLen = varMapping ->len[i];
            char* buffer =  new char[varLen + 1];
            strncpy(buffer, globalMem + varMapping ->startPos[i], varLen);
            buffer[varLen] = '\0';
            data[nowSize++] = buffer;
            //printf("buf: %s\t", buffer);
        }
        remaining--;
        // printf("during building\n");
        // for(int i = 0; i < dictionary.size(); i++){
        //     printf("UniquePos[%d]: %d\n",i, UniquePos[i]);
        // }
    }
    this -> dicMax = nowIdx; 
    //cout << "nowSize: " << nowSize << " select: " << select << endl;
    assert(nowSize == select);
    num = 0;
    type = -1;
    tot = nowSize;
    outlier.clear();


}

Union::Union(char** _data, int _num, int _type, int _tot, set<int> _outlier){ //Split construct
    data = _data;
    num = _num;
    type = _type;
    tot = _tot;
    outlier.clear();
    for (set<int>::iterator it = _outlier.begin(); it != _outlier.end(); it++){
        outlier.insert(*it);
    }
}
Union::~Union(){
    for(int i = 0; i < tot; i++){
        if(outlier.find(i) != outlier.end()) continue;
        free(data[i]);
    }
    free(data);
    free(UniquePos);
    free(HashValue);
}
int Union::getNum(){return num;}

unsigned int Union::_stringHash_(const char* start, int len){
    unsigned int seed = 131;
    unsigned int hash = 0;
    for(int i = 0; i < len; i++){
        hash = hash * seed + start[i];
    }
    return (hash & 0x7FFFFFFF);
}

bool Union::execute(Union** tree, int nowStart, int &nowEnd){//Main function excuter each union sequentially
    //First get constant and empty
    map<string, int> counter;
    int int_num = 0;
    map<int, int> len_counter;
   //cout << "Example: " << data[0] << endl;
    for(int i = 0; i < tot; i++){
        if (outlier.find(i) != outlier.end()) continue;
        string temp = data[i];
        //cout << temp << endl;
        if(counter.find(temp) != counter.end()){
            counter[temp]++;
        }else{
            counter[temp] = 1;
        }
        int nowLength = temp.size();
        if(len_counter.find(nowLength) != len_counter.end()){
            len_counter[nowLength]++;
        }else{
            len_counter[nowLength] = 1;
        }
// if (isInt(data[i])) int_num++;
    }

    //Get int rate
//    double intRate = int_num / (double)tot;
    
    //Get most frequent string
    int max = 0;
    string mostFP;
    for(map<string, int>::iterator it = counter.begin(); it != counter.end(); it++){
        if (it ->second > max){
            max = it ->second;
            mostFP = it ->first;
        }
    }
    //Get most frequent length
    max = 0;
    int maxLength = 0;
    for(map<int, int>::iterator it = len_counter.begin(); it != len_counter.end(); it++){
        if (it ->second > max){
            max = it ->second;
            maxLength = it ->first;
        }
    }

    //cout << "Now process Num: #" << num << " most frequent string: " << mostFP << " most frequent len: " << maxLength << " int rate: " << intRate << endl;
    if(counter[mostFP] > tot * threashold){
        if(mostFP.size() == 0){//check whether empty(most frquent string is empty)
            //cout << "Empty union!" << endl;
            type = -3;
            return true;
        }
        //check whether constant(string of largest frequency exceed threashold)
        //cout << "Constant union!" << endl;
        type = 0;
        constant = mostFP;
        length = mostFP.size();
        return true;
    }

    
    //Try to split
    //cout << "split start" << endl;
    int totParts = split(tree, nowEnd);
    //cout << "Split out" << endl;
    if (totParts == 0){ // Split failed -> try close this union
      //  cout << "Num: #" << num << "Spliter failed" << endl;
        //check whether fixed length(most common length exceed threashold)
        if(len_counter[maxLength] > tot * threashold){
            type = 1;
            length = maxLength;
            /*
            if(intRate > threashold){
        //        cout << "Fixed int union" << endl;
                hex = isHex(mostFP);
                type = 1;
            }else{
        //        cout << "Fixed string union" << endl;
                type = 2;
            }

            length = maxLength;
*/
            }else{ //check whether int, string(most common type exceed threashold)
                type = 2;
/*
            if(intRate > threashold){
                hex = isHex(mostFP);
                type = 3;
            }else{
                type = 4;
            }
*/
            }
        
    }else{ //Split success
        //Update other union #(from nowStart to nowEnd, if # > now #, update its #)
        //Update 
        //cout << "Num: #" << num << " Spliter success into " << totParts << endl;
        nowEnd += totParts;
        type = -1;
    }
    return true;
}

string Union::getFormat(const char* start, int len){
    string format = "";
    int nowType = 0;
    if(*start == ' ' && len == 1) format = "<-1>";
    for(int i = 0; i < len; i++){
        if(getTypeC(start[i]) == symbol_TY){
            if(nowType != 0) format += "<" + to_string(nowType) + ">";
            format += start[i];
            nowType = 0;
        }else{
            nowType |= getTypeC(start[i]);
        }
    }
    if(nowType != 0) format += "<" + to_string(nowType) + ">";
    return format;
}

void Union::buildMapping(VarArray* varMapping){
    //printf("start build mapping\n");
    int idx = 0;
    int tot = dictionary.size();
    
    int* Formats = new int[tot];
    
    // for(int i = 0; i < tot; i++){
    //     printf("UniquePos[%d]: %d\n",i, UniquePos[i]);
    // }

    for(int i = 0; i < tot; i++){
        int nowDicPos = UniquePos[i];
        //printf("nowDicPos: %d\n", nowDicPos);
        string format = getFormat(globalMem + varMapping->startPos[nowDicPos], varMapping->len[nowDicPos]);
        //printf(" format: %s idx: %d tot: %d\n", format.c_str(), nowDicPos, tot);
        Formats[idx++] = _stringHash_(format.c_str(), format.size());
        if(format_counter.find(format) != format_counter.end()){
            format_counter[format] += 1;
        }else{
            format_counter[format] = 1;
        }
    }
    //printf("format count finish\n");
   
    vector<pair<string, int> > formatCounter(format_counter.begin(), format_counter.end());
    patCount = min(int(formatCounter.size()), maxDicPat);
    sort(formatCounter.begin(), formatCounter.end(), formatCmp);
    //printf("patCount: %d\n", patCount);
    
    /*
    cout << "format counter: " << endl;
    for(vector<pair<string, int> >::iterator it = formatCounter.begin(); it != formatCounter.end(); it++){
        cout << it -> first << ": " << to_string(it -> second) << endl;
    }
    */
    int selectPat = (patCount == maxDicPat) ? patCount - 1: patCount;
    int dicEntry = 0;
    for(int i = 0; i < selectPat; i++){
        nowFormat[i] = formatCounter[i].first;
        int nowHash = _stringHash_(formatCounter[i].first.c_str(), formatCounter[i].first.size());
        nowCounter[i] = formatCounter[i].second;
        nowPaddingSize[i] = 0;
        for(int t = 0; t < tot; t++){
            int nowUniquePos = UniquePos[t];
            if(Formats[t] == nowHash){
                dictionary[HashValue[nowUniquePos]] = dicEntry++;
                nowPaddingSize[i] = max(nowPaddingSize[i], varMapping->len[nowUniquePos]);
            }
        }
    }
    free(Formats);
    //printf("dictionary entry generate finish\n");
    
    if(patCount != maxDicPat) return;

    int finalType = 0;
    int finalIdx = patCount - 1;
    nowPaddingSize[finalIdx] = 0;
    nowCounter[finalIdx] = 0;
    for(int i = 0; i < tot; i++){
        int nowUniquePos = UniquePos[i];
        if(dictionary[HashValue[nowUniquePos]] == -1){
            finalType |= getType(globalMem + varMapping->startPos[nowUniquePos], varMapping->len[nowUniquePos]);
            nowCounter[finalIdx]++;
            nowPaddingSize[finalIdx] = max(nowPaddingSize[finalIdx], varMapping->len[nowUniquePos]);
            dictionary[HashValue[nowUniquePos]] = dicEntry++;
        }
    }
    
    nowFormat[finalIdx] = "<" + to_string(finalType) + ">";
    //printf("end build mapping\n");
}

vector<pair<unsigned int, int> >* Union::getContainer(){
    vector<pair<unsigned int, int> >* container = new vector<pair<unsigned int, int> >(dictionary.begin(), dictionary.end());
    sort(container -> begin(), container -> end(), rpairCmp);
    return container;
}

bool Union::pairCmp (pair<unsigned int, int> t1, pair<unsigned int, int> t2){
    return (t1.second > t2.second);
}
bool Union::formatCmp (pair<string, int> t1, pair<string, int> t2){
    return (t1.second > t2.second);
}
bool Union::rpairCmp (pair<unsigned int, int> t1, pair<unsigned int, int> t2){
    return (t1.second < t2.second);
}
bool Union::unionCmp(Union& u1, Union& u2){
    if(u1.type < 0 && u2.type >= 0) return true;
    if(u2.type < 0 && u1.type >= 0) return false;
    return (u1.num < u2.num);
}
// bool Union::operator<(Union& uni){
//     return (uni.type < 0) || (this ->num < uni.num);
// }
string Union::outputDic(VarArray* varMapping){
    string temp = "";
    vector<pair<unsigned int, int> > container;
    for(map<unsigned int, int >:: iterator it = dictionary.begin(); it != dictionary.end(); it++){
        container.push_back(make_pair(it ->first, it ->second));
    }
    sort(container.begin(), container.end(), pairCmp);
    char buf[MAX_VALUE_LEN];
    for(vector<pair<unsigned int, int> >::iterator it = container.begin(); it != container.end(); it++){
        int Pos = posDictionary[it -> first];
        strncpy(buf, globalMem + varMapping->startPos[Pos], varMapping->len[Pos]);
        buf[varMapping->len[Pos]] = '\0';
        temp += string(buf) + "\n";
    }
    return temp;
}
void Union::output(){
    cout << "\n------------Union Start------------------" << endl;
    cout << "Union # " << num << " type: " << type << " tot size: " << tot << endl;
    cout << "Union data: " << endl;
    for(int i = 0; i < min(100, tot); i++){
        cout << data[i] << endl;
    }
    cout << "Outlier: " << endl;
    for(set<int>::iterator it = outlier.begin(); it != outlier.end(); it++){
        cout << "Index: " << *it << " Data: " << data[*it] << endl;
//      cout << "Index: " << outlier[i] << " " << data[outlier[i]] << endl;
    }
    cout << "------------Union END------------------\n" << endl;
}
int Union::split (Union** tree, int nowEnd){
    int trial = 3; //Test three times
    while(trial--){
        
        //Select random element
        int pivot_idx = rand() % tot;
        int totrand = 0;
        bool block = false;
        while(outlier.find(pivot_idx) != outlier.end()){
            pivot_idx = rand() % tot;
            if(totrand++ > 10){
                block = true;
                break;
            }
        }
        if(block) continue;

        char* pivot = data[pivot_idx];
        int index = 0;
        
        //test spliter(find a delimer)
        //Success -> matchSpliter -> split success
        //cout << "test spliter"<< endl;
        while(pivot[index] != '\0'){
            char nowChar = pivot[index];
            if (!isalnum(nowChar)){
                int totParts = testSpliter(nowChar); //Fail return 0, else return new union
                //cout << "here" << endl;
                if (totParts){ //split success
                    //cout << "Split with " << nowChar << " " << int(nowChar) << " success" << endl;
                    int** delimer = matchSpliter(nowChar); 
                    equeue(tree, nowEnd, delimer, totParts); //Build new Union and enqueue, update other number
                    
                    for (int i = 0; i < tot; i ++){
                        if(outlier.find(i) != outlier.end()) continue;
                        free(delimer[i]);
                    }
                    free(delimer);
                    return totParts;
                }
            }
            index++;
        }
        
        //cout << "test format" << endl;
        //test type(get type format, a-f,A-F,0-9 -> NUM, Others -> Str)
        //Success -> matchType -> split success
        /*
        char* format = getFormat(pivot);
        //cout << "Test format: " << format << endl;
        int totParts = testType(format);
        if (totParts){ //split success
          //  cout << "Format success: split into " << totParts << endl;
            int ** delimer = matchType(format);
          //  cout << "Get delimer: " << endl;
           // for(int i = 0; i < totParts-1; i++){
           //     cout << "delimer" << i << " is " << delimer[0][i] << endl;
           // }
            equeue(tree, nowEnd, delimer, totParts);
            for(int i = 0; i < tot; i++){
                if(outlier.find(i) != outlier.end()) continue;
                free(delimer[i]);
            }
            free(delimer);
            return totParts;
        }
        */

        //cout << "test LCS" << endl;
        //test LCS(find another string)
        //Success -> matchLCS -> split success
        pivot_idx = rand() % tot;
        
        while(outlier.find(pivot_idx) != outlier.end()) pivot_idx = rand() % tot;
        char* pivot2 = data[pivot_idx];
        char* LCS = getLCS(pivot, pivot2);
        //cout << "Start to test pivot: " << pivot << " pivot2: " << pivot2 << " LCS: " << LCS << endl;
        
        if (strcmp(LCS, pivot) == 0 || strcmp(LCS, pivot2) == 0 || strlen(LCS) < 3){
            continue;
        }
        int nowSize = strlen(LCS);
        int totParts = testLCS(LCS, nowSize);
        while(nowSize-- >= 3 && totParts == 0){
            
            totParts = testLCS(LCS, nowSize);
        }
        
        if (totParts){
            char* newLCS = new char[nowSize+1];
            memcpy(newLCS, LCS, nowSize);
            newLCS[nowSize] = '\0';
            //cout << "start split LCS: " << newLCS << endl;
            int ** delimer = matchLCS(newLCS);
            // cout << "Get delimer: " << endl;
            // for(int i = 0; i < totParts-1; i++){
            //     cout << "delimer" << i << " is " << delimer[0][i] << endl;
            // }
            equeue(tree, nowEnd, delimer, totParts);
            for(int i = 0; i < tot; i ++){
                if(outlier.find(i) != outlier.end()) continue;
                free(delimer[i]);
            }
            free(delimer);
            return totParts;
        }
        
        
    }
    return 0; //Split failed
}
char* Union::getLCS(char* t1, char* t2){
    if (strlen(t1) > strlen(t2)){
        char* temp = t2;
        t2 = t1;
        t1 = temp;
    }
    int m = strlen(t1);
    int n = strlen(t2);
    int maxlen = 0, end = 0;
    int ** DP = new int*[m+1];
    for(int i = 0; i <= m; i++){
        DP[i] = new int[n+1];
    }
    for(int i = 0; i <= m; i++){
        for(int j = 0; j <= n; j++){
            if (i == 0 || j == 0) DP[i][j] = 0;
            else if(t1[i - 1] == t2[j - 1]){
                DP[i][j] = DP[i - 1][j - 1] + 1;
                if(DP[i][j] > maxlen){
                    maxlen = DP[i][j];
                    end = i;
                }
            }else DP[i][j] = 0;
        }
    }
    char* LCS = new char[maxlen+1];
    //cout << "now t1: " << t1 << " now t2: " << t2 << " maxLen: " << maxlen << " end: " << end << endl; 
    memcpy(LCS, &(t1[end - maxlen]), maxlen);
    LCS[maxlen] = '\0';
    for(int i = 0; i <= m; i++){
        free(DP[i]);
    }
    free(DP);
    return LCS;
}
/*
int Union::getType (char c){ //0-9, A-F, a-f -> num(0), others -> str(1)
    if (isdigit(c) || (c >= 65 && c <= 70) || (c >= 97 && c <= 102)){
        return 0;
    }else{
        return 1;
    }
}
*/
/*
int* Union::getFormat (char* target){
    int index = 0;
    int* format = new char[strlen(target) + 1];
    int findex = 0;
    int nowType = getTypeC(target[index]);
    int count = 0;
    while(target[index] != '\0'){
        if (getTypeC(target[index]) != nowType && (nowType == 0 && count < 8)){
            count = 0;
            format[findex++] = nowType;
            nowType = getType(target[index]);
        }
        count++;
        index++;
    }
    
    if(nowType == 0){
        format[findex++] = 'N';
    }else{
        format[findex++] = 'S';
    }
    format[findex] = '\0';
    return format;
}
*/

int Union::equeue (Union** tree, int nowEnd, int** delimer, int totParts){
    //Build new unions
    char*** Dataset = new char**[totParts];
    for(int i = 0; i < totParts; i++){
        Dataset[i] = new char*[tot]; //target data
    }
    for(int i = 0; i < tot; i ++){
        if (outlier.find(i) != outlier.end()){ //outlier
            //cout << "outlier" << endl;
            for(int t = 0; t < totParts; t++){ //Copy outlier for every union
                Dataset[t][i] = NULL; 
                // new char[strlen(data[i]) + 1];
                // memcpy(Dataset[t][t], data[i], strlen(data[i]));
                // Dataset[t][i][strlen(data[i])] = '\0';
            }
            continue;
        }
        //cout << "i: " << i << endl;
        char * temp = data[i];
        for(int t = 0; t < totParts; t++){
            int nowLength = 0;
            char * startPoint;
            if (t == 0){ //First part(For static splie)
                nowLength = delimer[i][0];
                startPoint = temp;
            }else if (t == totParts - 1){ //Last part
                nowLength = strlen(data[i]) - delimer[i][t - 1];
                startPoint = &(temp[delimer[i][t-1]]);
            }else{
                nowLength = delimer[i][t] - delimer[i][t - 1];
                startPoint = &(temp[delimer[i][t - 1]]);
            }
            Dataset[t][i] = new char[nowLength + 1];
            memcpy(Dataset[t][i], startPoint, nowLength * sizeof(char));
            Dataset[t][i][nowLength] = '\0';
//            Dataset[t][i] = 
        }
    }
    
    //Build new Union
    for(int t = 0; t < totParts; t++){
        // cout << "Now process: " << t << endl;
        // cout << Dataset[t][0] << endl;
        Union * nowUnion = new Union(Dataset[t], num+t+1, -2, tot, outlier);
        //cout << "Build new Union" << endl;
        //nowUnion -> output();
        
        tree[nowEnd + t] = nowUnion;
        
    }
    //cout << "here" << endl;
    //Update tree num
    for(int i = 0; i < nowEnd; i++){
        if (tree[i] -> num > num){
            tree[i] -> num += totParts;
        }
    }
    
    return totParts;
}
int Union::testSpliter(char tester){ //Test whether a spliter is common
    int success = 0;
    int index = 0;
    for(int i = 0; i < tot; i++){
        if(outlier.find(i) != outlier.end()) continue;
        char* temp = data[i];
        int index = 0;
        while(temp[index] != '\0'){
            if (temp[index] == tester){
                if (strlen(temp) != 1){
                    success++;
                    break;
                }
            }
            index++;
        }      
    }
    if (success > tot * threashold){
        return 3;
    }else{
        return 0;
    }
}

int Union::testLCS (char* tester, int size){
    //cout << "test size: " << size << endl;
    char temp[size+1];
    memcpy(temp, tester, size);
    temp[size] = '\0';
    string nowTest = temp;
    int out[tot];
    int out_idx = 0;
    int success = 0;
    for(int i = 0; i < tot; i ++){
        //cout << "i: " << i << endl;
        if(outlier.find(i) != outlier.end()) continue;
        //cout << "here1" << endl;
        string nowStr = data[i];
        if(nowStr != nowTest && nowStr.find(nowTest) == -1){ //failed
          //  cout << "here1" << endl;
            out[out_idx++] = i;
          //  cout << "here2" << endl;
        }else{
            success++;
        }
    }
    //cout << "success num: " << success  << " threashold: " << tot * threashold << endl;
    if(success > tot * threashold){
        for(int i = 0; i < out_idx; i++){
            outlier.insert(out[i]);
        }
        return 3;
    }else{
        return 0;
    }
}
int** Union::matchSpliter (char spliter){
    int ** delimer = new int*[tot];
    for (int i = 0; i < tot; i++){
        if(outlier.find(i) != outlier.end()) continue;
        delimer[i] = new int[2];
        char * temp = data[i];
        int index = 0;
        bool out = true;
        while(temp[index] != '\0'){
            if (temp[index] == spliter){
                out = false;
                delimer[i][0] = index; delimer[i][1] = index + 1; //Left close, right open
                break;
            }
            index++;
        }
        if (out){ //Outlier
            outlier.insert(i);
            //outlier[outlierNum++] = i;
            delimer[i][0] = -2; delimer[i][1] = -2;
        }
    }
    return delimer;
}

/*
int** Union::matchType(char * format){
    int** delimer = new int*[tot];
    int totPart = strlen(format);
    for(int i = 0; i < tot; i++){
        if(outlier.find(i) != outlier.end()) continue;
        delimer[i] = new int[totPart - 1];
        char*temp = data[i];
        int index = 0;
        int findex = 0;
        int nowType = getType(temp[0]);
        bool match = ((nowType == 0 && format[0] == 'N') || (nowType == 1 && format[0] == 'S'));
        //cout << temp << " -> " << format << endl;
        //cout << "match: " << match << " findex: " << findex << endl;
        while(temp[index] != '\0'){
            if(!match) break;
            if(getType(temp[index]) != nowType){
                //cout << "Now change @ " << index << " now type: " << getType(temp[index]) << " previous type: " << nowType << endl;
                if (findex == totPart) {
                    match = false;
                    break;
                }
                nowType = getType(temp[index]);
                findex++;
                match = ((nowType == 0 && format[findex] == 'N') || (nowType == 1 && format[findex] == 'S'));
                delimer[i][findex - 1] = index;
            }
            index++;
        }
        //match = ((nowType == 0 && format[findex] == 'N') || (nowType == 1 && format[findex] == 'S'));
        //findex++;
        //cout << "output match: " << match << " findex: " << findex << endl;
        if(!match || findex != totPart - 1){ //Outlier
            outlier.insert(i);
        }
    }
    return delimer;
}
*/

int** Union::matchLCS (char * LCS){
    int** delimer = new int*[tot];
    string testString = LCS;
    for(int i = 0; i < tot; i ++){
        if(outlier.find(i) != outlier.end()) continue;
        delimer[i] = new int[2];
        string nowString = data[i];
        size_t pos = nowString.find(testString);
        //cout << nowString << " " << testString << endl;
        delimer[i][0] = pos;
        delimer[i][1] = pos + testString.size();
       // cout << "i: " << i << " delimer[i][0]: " << delimer[i][0] << " delimer[i][1]: " << delimer[i][1] << endl;
    }
    return delimer;
}


