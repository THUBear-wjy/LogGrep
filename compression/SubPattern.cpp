#include "SubPattern.h"
#include <string>
#include <vector>
#include <set>
#include "union.h"
#include "util.h"
using namespace std;
SubPattern::SubPattern(Union* now, string _nextConstant, int maxData){
        type = now ->type;
        dataType = 0;
        data_count = 0;
        data = new string[maxData];
        maxLen = -1;
        length = (type == 2) ? -1 : now ->length;
        hex = now ->hex;
        if (type == 0){
            const_pattern = now -> constant;
        }
        if (type == 2){
            nextConstant = _nextConstant;
        }
}
void SubPattern::add(bool success){
    if(type == 0) return;
    if(success){
        data[data_count++] = readyAdd;
    }else{
        data[data_count++] = "";
    }
}
bool SubPattern::extract(string log, int & strIdx, bool & jump){
        set<int> now_outlier;
        if (type == 0){ //Constant check
            for(int w = 0; w < length; w++){
                if (strIdx + w >= log.size() || const_pattern[w] != log[strIdx + w]){
                    return false;
                }
            }
            strIdx += length;
    
        }
        if (type == 1){ //Fixed length
            if(strIdx >= log.size() || strIdx + length > log.size()){
//    printf("%c", log[strIdx]);
//                cout << "strIdx: " << strIdx << " length: " << length << " type: " << Union::getType(log[strIdx]) << endl; 
                return false;
            }
            string temp = log.substr(strIdx, length);
            dataType |= getType(temp.c_str(), temp.size());
            readyAdd = temp;
            //data[data_count++] = temp;
            strIdx += length;
        }
        if (type == 2){ //Variable length
            if(strIdx >= log.size()) return false;
            int L = 0;
            
            while(strIdx + L + nextConstant.size() < log.size()){
                bool match = true;        
                for(int i = 0; i < nextConstant.size(); i++){
                    if(log[strIdx + L + i] != nextConstant[i]){
                        match = false;
                        break;
                    }
                }
                if(nextConstant.size() > 0 && match) break;
                L++;
            }
            if (strIdx + L > log.size()) return false;
            if(L > maxLen) maxLen = L;
            string temp = log.substr(strIdx, L);
            //cout << "nextConst: " << nextConstant << " temp: " << temp << " strIdx: " << strIdx << " L: " << L << endl;

            dataType |= getType(temp.c_str(), temp.size());
            //data[data_count++] = temp;
            readyAdd = temp;
            strIdx += L;
        }
        return true;
}

string SubPattern::getPattern(){
   if(type == 0) return const_pattern;
   if(type == 1) return "<F," + to_string(dataType) + "," + to_string(length) + ">";
   if(type == 2) return "<V," + to_string(dataType) + "," + to_string(maxLen) + ">";
    return "<NOPATTERN>";
}

void SubPattern::output(){
        cout << "Pattern Like: " << getPattern();
//cout << " length: " << length << " type: " << type << endl;
        cout << endl;
}

bool SubPattern::output_var(string path){
        string suffix = ".unknown";
        if(type == 1) suffix = ".fint";
        if(type == 2) suffix = ".fstr";
        if(type == 3) suffix = ".int";
        if(type == 4) suffix = ".str";
        path = path + suffix;

        FILE* fow = fopen(path.c_str(), "w");
        if(fow == NULL) return false;
        for(int i = 0; i < data_count; i++){
            fprintf(fow, "%s\n", data[i].c_str());
        }
        fclose(fow);
        return true;
    }
