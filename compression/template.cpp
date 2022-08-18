#include <iostream>
#include <string>
#include "template.h"
#include <set>
#include "constant.h"
#include <cstring>
#include <vector>
#include <cstdio>
using namespace std;

int templateNode::sum = 0;
// set<string> templateNode::split;
// templateNode** templateNode::allTemplates=NULL;
// int*** templateNode::intData=NULL;
// string*** templateNode::stringData=NULL;
// int* templateNode::topPointer=NULL;

//constructor, need Eid, tokens,and length
templateNode::templateNode(int eid, char* log, SegTag segArray[MAXTOCKEN], int token_size){ //build from sketch
    Eid = eid;
	//allTemplates[eid]=this;
    length = token_size;
    templates = new char* [length];
    templatesTags = new int[length];
	varLength = 0;
    for (int i = 0;i < length;i++){
        int now_segLen = segArray[i].segLen;
        templates[i]= new char[now_segLen+1];
        strncpy(templates[i], log + segArray[i].startPos, now_segLen);
        templates[i][now_segLen] = '\0';
        templatesTags[i] = segArray[i].tag;
    }
}

templateNode::~templateNode(){
	//cout<<"distruction templates:"<<Eid<<endl;
    for(int i = 0; i < length; i++){
        free(templates[i]);
    }
    free(templates);
}

//the match function foe match,need length
//clove +-
int templateNode::matchMatch(char* tockens, SegTag segArray[MAXTOCKEN], int tocken_size, int skip){
    char nowVar[MAX_VALUE_LEN];
    char* temp;
    for (int i=tocken_size-1; i>=0; i--)//from bottom to top
    {
        //if (templates[i] == "<*>") continue;
        if(templatesTags[i] == 2) continue;//变量
        //分隔符
        if(templatesTags[i] > 9 || segArray[i].tag > 9)
        {
            if(segArray[i].tag == templatesTags[i]) 
                continue;
            else
                return -1;
        }  
        //静态字符串的匹配
        temp = tockens + segArray[i].startPos;
        int lastIndex = segArray[i].segLen-1;
        //优先检查长度和首尾两个字符
        if(
        segArray[i].segLen != strlen(templates[i]) ||
        temp[lastIndex] != templates[i][lastIndex] || 
        temp[0] != templates[i][0]
        ) return -1;
        for(int j=1; j< lastIndex; j++)//也可考虑奇数抽样
        {
            if(temp[j] != templates[i][j])
            {
                return -1;
            } 
        }
    }
    return tocken_size;
}
double templateNode::parseMatch(char* log, SegTag segArray[MAXTOCKEN], int token_size){
    if (token_size != length){
        return 0;
    }
    int sim = 0;
    int tot_size = 0;
    for (int i = 0; i < token_size; i++){
        //string tocken = tockens[now_point];
        // if (length < skip && now_point < skip){
        //     now_point++;
        //     continue;
        // }
        if(segArray[i].tag > 9 && templatesTags[i] != segArray[i].tag){ //Not similar delimiter
            return 0;
        }

        if (templatesTags[i] == 2){ //Similarity ++
            tot_size++;
            sim++;
            continue;
        }
        
        if (strlen(templates[i]) != segArray[i].segLen){ //Different length no similarity++
            tot_size++;
            continue;
        }

        if (strncmp(log + segArray[i].startPos, templates[i], segArray[i].segLen) != 0){
          //cout << "Now tocken: " << tocken << " now template tocken: " << templates[now_point] << endl;
            tot_size++;
            continue;
        }
        //Similar
        tot_size++;
        sim++;
    }
    //cout << "now_point: " << now_point << " tot_size: " << tot_size << " sim: " << sim << endl;
    return (double)sim / tot_size;
}
void templateNode::merge(char* log, SegTag segArray[MAXTOCKEN], int token_size){
    for (int i = 0; i < token_size; i++){
        // int len = segArray[i].segLen;
        // char* temps = new char[len+1];
        // strncpy(temps, log + segArray[i].startPos, len);
        // temps[len] = '\0';
        // printf("i: %d Token is: %s", i, temps);
        // printf("i: %d Template token is: %s\n", i, templates[i]);
        if (templatesTags[i] == 2) continue;

        if (strncmp(log + segArray[i].startPos, templates[i], segArray[i].segLen) != 0){
            //printf("Find differenet\n");
            string temp = "<*>";
            strcpy(templates[i], temp.c_str());
            templatesTags[i] = 2;//2代表变量
            varLength++;
            varIndex.push_back(i);
        }
    }
}

void templateNode::reset(){
    sum = 1;//E0,failed match
}

void templateNode::initPara(){
	varLength = 0;
	for (int i=0;i<length;i++){
		if (templates[i] == "<*>"){
		    varLength++;
        	varIndex.push_back(i);
            templatesTags[i]=2;//2代表变量
		}
	}
}

string templateNode::print(){
	string temp = "E" + to_string(Eid) + "\t";
	temp += "varLength: " + to_string(varLength) + " varIndex: ";
    for (int i = 0; i < varLength; i++){
        temp += to_string(varIndex[i]) + " ";
    }
    temp += "|";
    for (int i = 0;i<length;i++){
        temp += string(templates[i]) + " ";
    }
    
    temp += "\n";
    return temp;
}

string templateNode::output(){
    //int nowVar = 0;
    string temp = "";
    for(int i = 0; i < length; i++){
        bool nowGet = false;
        for(int t = 0; t < varLength; t++){
            if (i == varIndex[t]){
                temp += "<V" + to_string(t) + ">";
                nowGet = true;
                break;
            }
        }
        if (nowGet) continue;
        temp += templates[i];
    }
    return temp;
}
