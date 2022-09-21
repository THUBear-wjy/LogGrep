#ifndef SEARCH_ALGORITHM_H
#define SEARCH_ALGORITHM_H

#include "LogStructure.h"
#include <regex.h>
#include <vector>

//#define max(a,b) (((a)>(b))?(a):(b))

int BinarySearch(int a[], int value, int n);
LcsMatch GetLCS_DPoptc(const char* str1, int size1, const char* str2, int size2, bool beReplace= false);
short GetCharTag(const char queryChar);
extern short GetStrTag(const char* queryStr, int pLen);
extern int GetSegmentType(IN char* segment, int& segValue);
extern int IntPadding(int target, int maxLen, char* retStr);
extern int RemovePadding(IN char* paddingedData, int len, OUT char* oriData);
extern int atoi(char* buf, int len);
extern int StrIsDelim(const char* queryStr, int segLen);
extern int RecombineString(char* args[], int argCountS, int argCountE, char* queryStr);
extern void Release_SearchTemp();
/*
** reference : all reg match result position can be marked
** return : match count , error: less than 0
*/
extern int QueryInStrArray_CReg(char** targetStr, int lineCount, const char *queryStr, BitMap* bitmap);
extern int QueryInStrArray_CReg_Reverse(char** targetStr, int lineCount, const char *queryStr, BitMap* bitmap);
extern int QueryInStrArray_CReg_RefMap(char** targetStr, int lineCount, const char *queryStr, BitMap* bitmap, BitMap* refBitmap);
extern int QueryInStrArray_CReg_Reverse_RefMap(char** targetStr, int lineCount, const char *queryStr, BitMap* bitmap, BitMap* refBitmap);
extern int QueryInStr_CReg(const char* text, const char *regPattern);
extern int QueryInStrArray_BM(char** targetStr, int lineCount, char *queryStr, BitMap* bitmap);
extern int QueryInStrArray_BM_Reverse(char** targetStr, int lineCount, char *queryStr, BitMap* bitmap);
extern int QueryInStrArray_BM_RefMap(char** targetStr, int lineCount, char *queryStr, BitMap* bitmap, BitMap* refBitmap);
extern int QueryInStrArray_BM_Reverse_RefMap(char** targetStr, int lineCount, char *queryStr, BitMap* bitmap, BitMap* refBitmap);

/*
** reference : KMP
** return : the index of matched start pos
*/
extern void InitKmpNext(const char *T, int* &next);
extern int KMP(char *S,const char *T, int next[MAX_VALUE_LEN], int queryType);
extern int KMP(char *S,const char *T, OUT BitMap* bitmap, int queryType);

/*
** reference : Boyer-Moore
** return : the index of matched start pos
*/
extern void InitBM(const char* pattern, int* &badc, int* &goods);
void BuildBadC(const char* pattern, int* &badc);
void BuildGoodS(const char *pattern, int* &goods);

int BM_Fixed_AlignR(char* text, int sIdx, int tLen, const char* pattern, BitMap* bitmap, int lineLen, bool enableUnionSkip=false);
int BM_Fixed_AlignL(char* text, int sIdx, int tLen, const char* pattern, BitMap* bitmap, int lineLen, bool enableUnionSkip=false);
int Fixed_AlignL_For_Empty(char* text, int sIdx, int tLen, BitMap* bitmap, int lineLen, bool enableUnionSkip=false);

int SeqMatching_AlignLeft(const char *S, int sLen, const char *T, int tLen);
int SeqMatching_AlignLeft(const char * S, int sLen, const char * T, int tLen, int* badc, int* goods);
int SeqMatching_AlignRight(const char * S, int sLen, const char * T, int tLen);
int SeqMatching_MultiFul(const char *S, int sLen, const char *T, int tCnt);
int SeqMatching_BothSide(const char * S, int sLen, const char * T, int tLen);

extern int BM_Once(char* text, const char* pattern, int textLen, int* badc, int* goods);
extern int BM_Fixed_Align(char* text, int sIdx, int tLen, const char* pattern, BitMap* bitmap, int lineLen, int flag=0);
extern int BM_Fixed_MutiFul(char* text, const char* pattern, int patCnt, BitMap* bitmap, int lineLen);
extern int BM_Fixed_Anypos(char* text, int sIdx, int tLen, const char* pattern, BitMap* bitmap, int lineLen);
extern int BM_Fixed_Pushdown_MutiFul(char* text, const char* pattern, int patCnt, BitMap* bitmap, int lineLen);
extern int BM_Fixed_Pushdown_MutiFul_RefMap(char* text, const char* pattern, int patCnt, BitMap* bitmap, BitMap* refBitmap, int lineLen);
extern int BM_Diff(char* text, int sIdx, int tLen, const char* pattern, BitMap* bitmap, int minLineLen, int maxLineLen);
extern int BM_Fixed_Pushdown(char* text, const char* pattern, BitMap* bitmap, int lineLen, int type);
extern int BM_Fixed_Pushdown_RefMap(char* text, const char* pattern, BitMap* bitmap, BitMap* refBitmap, int lineLen, int type);
extern int BM_Diff_Pushdown(char* text, const char* pattern, BitMap* bitmap, int type);
extern int BM_Diff_Pushdown_RefMap(char* text, const char* pattern, BitMap* bitmap, BitMap* refBitmap, int type);
extern int BMwildcard_AxB(char *text, int lineCnt, int lineLen, const char *A, const char *B, BitMap* bitmap);

extern int GetCvarsByBitmap_Fixed(char* text, int lineLen, BitMap* bitmap, OUT char *vars, int entryCnt, int varsLineLen, bool flag=true);
extern int GetCvars_Fixed(char* text, int lineLen, int lineCnt, OUT char *vars, int varsLineLen, bool flag=true);
extern int GetCvarsByBitmap_Diff(char* text, int minLineLen, BitMap* bitmap, OUT char *vars, int entryCnt, int varsLineLen, bool flag=true);
extern int GetCvars_Diff(char* text, OUT char *vars, int entryCnt, int varsLineLen, bool flag=true);

int MatchInSubpatVar_Forward(int strTag_mark, int maxlen_mark, const char* source, int souLen, int& souIndex);
int MatchInSubpatVar_Backward(int strTag_mark, int maxlen_mark, const char* source, int& souIndex);
int MatchInSubPatConst_Forward(const char* patConst, int patLen, const char* source, int souLen, int& souIndex);
int MatchInSubPatConst_Backward(const char* patConst, int patLen, const char* source, int& souIndex);
int MatchInSubpatVvarWithTail(int strTag_mark, int maxlen_mark, const char* patConst, const char* source, int souLen, int& souIndex);
int MatchInSubpatVvarWithHead(int strTag_mark, int maxlen_mark, const char* patConst, const char* source, int& souIndex);
int MatchInSubPatConst_WithVar(SubPattern* pat, int curIndex, const char* source, int souLen, OUT RegMatrix* regResult);
int MatchInDicPatConst_WithVar(SubPattern::DicVarInfo* pat, int curIndex, const char* source, int souLen);
int SubPatMatch_0_J(SubPattern* pat, int sPos, int ePos, const char* source, int souLen, OUT RegMatrix* regResult, int deftSouIdx=0);
int SubPatMatch_N_I(SubPattern* pat, int sPos, int ePos, const char* source, int souLen, OUT RegMatrix* regResult);
int SubPatMatch_I_J(SubPattern* pat, const char* source, int souLen, OUT RegMatrix* regResult);
int DicPatMatch_0_J(SubPattern::DicVarInfo* pat, int sPos, int ePos, const char* source, int souLen, int deftSouIdx=0);
int DicPatMatch_N_I(SubPattern::DicVarInfo* pat, int sPos, int ePos, const char* source, int souLen);
int DicPatMatch_I_J(SubPattern::DicVarInfo* pat, const char* source, int souLen);
bool UnionMatchResult(RegMatches& obj, RegMatches& refobj);
extern int SubPatternMatch(SubPattern* pat, const char* source, int matchType, OUT RegMatrix* regResult);
extern int DicPatternMatch(SubPattern* pat, const char* source, int matchType, OUT RegMatrix* regResult);

extern int Alg_Test();

#endif
