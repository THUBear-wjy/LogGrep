#ifndef _LZMA_SDK_
#define _LZMA_SDK_
#include<cstdlib>
#include<cstdio>
#include<cstring>
#include<iostream>
#include"constant.h"
#include"LZMA/LzmaLib.h"

using namespace std;

int LzmaCompression(FILE* ZIP, char * srcData, int srcLen, int tag){
    if(srcData == NULL)
    {
        printf("input params error!");
        return -1;
    }
    if(srcLen == 0) return 0;
    //cout << "srcData: " << srcData << " srcLen: " << srcLen << endl;
    
    if (ZIP == NULL)
    {
        return  - 1;
    }

    size_t destLen = srcLen;
	size_t sizeProp = 5;
	unsigned char prop[5];
	unsigned char *pLzma = new unsigned char[srcLen + sizeProp + 10000]; //store lzma data

	int res = LzmaCompress(pLzma,&destLen,(unsigned char*)srcData,srcLen,prop,&sizeProp,5,(1<<24),3,0,2,32,2);
	cout << destLen << endl;
    if (SZ_OK != res)
	{
		printf("res:%d ", res);
		fwrite(srcData, sizeof(char), srcLen, ZIP);
        delete[] pLzma;
        return -1;
	}
    //write compress data into file
    int return_value = (tag == 0) ? destLen:destLen+5;
    if(tag) fwrite(prop, sizeof(char), 5, ZIP);
    fwrite(pLzma, sizeof(char), destLen, ZIP);
    fclose(ZIP);
/*
	if (SZ_OK != LzmaUncompress(punLzma, &destLen2, pLzma, &destLen, prop, sizeProp))
	{
		printf("uncompresss error！");
		return -1;
	}
*/
	delete[] pLzma;

    return return_value;

}

int LzmaUnCompression(const char* zip_name, char* result, int offset, int destLen, int srcLen, int tag){ //0 for read compressed, -1 for read normal
    if(zip_name == NULL || offset < 0 || destLen < 0){
        printf("uncompression params error!");
        return -1;
    }
    if(srcLen == 0) return 0;
    unsigned char prop[5];
    
    FILE* pZipFile = fopen(zip_name, "rb");
    if(pZipFile == NULL){
        printf("uncompression read file error");
        return -1;
    }
    if(tag == -1){
        fseek(pZipFile, offset, 0);
        fread(result, sizeof(char), srcLen, pZipFile);
        fclose(pZipFile);
        return srcLen;
    }
    unsigned char* buffer = new unsigned char[destLen];
    fseek(pZipFile, offset, 0);
    fread(prop, sizeof(char), 5, pZipFile);
    fread(buffer, sizeof(char), destLen - 5, pZipFile);
    fclose(pZipFile);

    size_t outputLen = srcLen;
    size_t inputLen = destLen - 5;
    int res = LzmaUncompress((unsigned char*)result, &outputLen, buffer, &inputLen, prop, 5);
    if (res != SZ_OK){
        printf("res:%d ", res);
        return -1;
    }
    return outputLen;
}
#endif
