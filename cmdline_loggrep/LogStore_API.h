#include "CmdDefine.h"
#include "LogStructure.h"
#include "SearchAlgorithm.h"

#ifndef CMD_LOGSTOREAPI_H
#define CMD_LOGSTOREAPI_H

#include "/apsarapangu/disk2/compression_clove/Coffer.h"
#include "/apsarapangu/disk2/compression_clove/LZMA/LzmaLib.h"
//use char* as a key will cause unknown error
typedef list<char*> LISTCHARS;
typedef map<int, LogPattern*> LISTPATS;
typedef map<int, SubPattern*> LISTSUBPATS;
typedef map<int, Coffer*> LISTMETAS;
typedef map<int, VarOutliers*> LISTOUTS;
typedef map<int, BitMap*> LISTBITMAPS;
typedef map<string, LISTBITMAPS> LISTSESSIONS;
//typedef map<string, QueryProc*> LISTSESSIONS;// to do, replace algrithm(LRU + access freq), and COW

typedef struct QueryProc
{
	int No;
	int Freq;//access frequency
	LISTBITMAPS Bitmaps;
	QueryProc()
	{
		No = 0;
		Freq=0;
	}

	LISTBITMAPS GetBitmaps()
	{
		No ++;
		Freq++;
		return Bitmaps;
	}
}QueryProc;

typedef char* CELL;


typedef int(*pLoadPatCallback)(char*);

class LogStoreApi
{
public:
	LogStoreApi();
	~LogStoreApi();

private:
	int m_nServerHandle;
	int m_fd;
	FILE* m_fptr;
	int m_glbMetaHeadLen;
	unsigned char m_lzmaMethod;
	int m_maxBitmapSize;

	LISTMETAS m_glbMeta;
	LISTPATS m_patterns;
	LISTSUBPATS m_subpatterns;
	LISTSESSIONS m_sessions;
	LISTOUTS m_varouts;
	char m_filePath[MAX_DIR_PATH];
	CELL* m_outliers;
	BitMap* m_glbExchgLogicmap;//to cache bitmap on logics
	BitMap* m_glbExchgPatmap;//to cache bitmap on pats
	BitMap* m_glbExchgBitmap;//to cache bitmap on vars
	BitMap* m_glbExchgSubBitmap;//to cache bitmap on subvars
	BitMap* m_glbExchgSubTempBitmap;//to cache bitmap on subvars while multi-pushdown

	//int maxCnt;
	

public:
	RunningStatus RunStatus;
	Statistics Statistic;
	string FileName;

private:
	int LoadFileToMem(const char *varname, int startPos, int bufLen, OUT char *mbuf);
	unsigned char* LoadFileToMem(const char *varname, int startPos, int bufLen);
	int LoadBuffToMemWithDecomp(int varname, OUT Coffer* &meta);
	int DeCompressPatterns(int patName, OUT Coffer* &coffer);
	int LzmaDeCompression(IN char* inBuf, OUT char* outBuf);
	int DeepCloneMap(LISTBITMAPS source, LISTBITMAPS& des);

	int BootLoader(char* path, char* file);
	int LoadGlbMetaHeader(char* filename, size_t& desLen, size_t& srcLen, unsigned char prop[5]);
	int LoadGlbMetadata(char* filename, size_t desLen, size_t srcLen, unsigned char prop[5]);
	int LoadMainPatternToGlbMap(IN char* deCompressedBuf, IN int srcLen);
	int LoadSubPatternToGlbMap(IN char* deCompressedBuf, IN int srcLen);
	int AddMainPatternToMap(char etag, int eid, int count, char* content);
	int AddSubPatternToMap(int vid, char type, char* content);
	int LoadOutliers(IN char* deCompressedBuf, int lineCount);
	int LoadVarOutliers(int filename, int lines, int srcLen);

	int LoadcVars(int varname, int lineCnt, OUT char* vars, int varsLineLen, int flag=true);
	int LoadcVarsByBitmap(int varname, BitMap* bitmap, OUT char* vars, int entryCnt, int varsLineLen, int flag=true);
	int ClearVarFromCache();
	int ClearCoffer(Coffer* &coffer);
	char* FormatVarName(int varName);
	
	int QueryInMmapByKMP(int varname, const char* queryStr, OUT BitMap* bitmap);
	int QueryByBM_Union(int varname, const char* queryStr, int queryType, BitMap* bitmap);
	int QueryByBM_AxB_Union(int varname, const char* queryStrA, const char* queryStrB, BitMap* bitmap);
	int QueryByBM_Union_ForDic(int varname, const char* querySegs, int querySegCnt, BitMap* bitmap);
	int QueryByBM_Union_RefRange(int varname, const char* queryStr, int queryType, BitMap* bitmap, RegMatrix* refRange);
	int QueryByBM_Pushdown(int varname, const char* queryStr, BitMap* bitmap, int type);
	int QueryByBM_Pushdown_RefMap(int varname, const char* queryStr, BitMap* bitmap, BitMap* refBitmap, int type);
	int QueryByBM_Pushdown_ForDic(int varname, const char* querySegs, int querySegCnt, BitMap* bitmap);
	int QueryByBM_Pushdown_ForDic_RefMap(int varname, const char* querySegs, int querySegCnt, BitMap* bitmap, BitMap* refBitmap);

	int GetValuesByVarName_Reg(int varName, const char* regPattern, OUT char* vars, OUT BitMap* bitmap);
	int GetVals_Subpat(int varName, const char* regPattern, int queryType, OUT BitMap* bitmap);
	int GetVals_AxB_Subpat(int varName, const char* queryA, const char* queryB, BitMap* bitmap);
	int GetVals_Subpat_Pushdown(int varName, const char* regPattern, int queryType, BitMap* bitmap);
	int GetVals_Subpat_Pushdown_RefMap(int varName, const char* regPattern, int queryType, BitMap* bitmap, BitMap* refBitmap);
	int GetVals_Dic(int varName, const char* regPattern, int queryType, OUT BitMap* bitmap);
	int GetVals_AxB_Dic(int varName, const char* queryA, const char* queryB, OUT BitMap* bitmap);
	int GetVals_Dic_Pushdown(int varName, const char* regPattern, int queryType, OUT BitMap* bitmap);
	int GetVals_Dic_Pushdown_RefMap(int varName, const char* regPattern, int queryType, OUT BitMap* bitmap, BitMap* refBitmap);
	int GetSubVals_Pushdown(RegMatches regmatches, const char* regPattern, int queryType, BitMap* bitmap, BitMap* refBitmap=NULL);
	int GetDicIndexs(int varName, const char* regPattern, int queryType, OUT char* &dicQuerySegs);
	int GetDicOffsetByEntry(SubPattern* subpat, int dicNo, int& dicLen);
	int GetVarOutliers_BM(int varName, const char *queryStr, BitMap* bitmap, BitMap* refBitmap);
	int GetOutliers_MultiToken(char *args[MAX_CMD_ARG_COUNT], int argCountS, int argCountE, BitMap* bitmap, bool beReverse=false);
	int GetOutliers_SinglToken(char *arg, BitMap* bitmap, bool beReverse=false);
	int GetOutliers_MultiToken_RefMap(char *args[MAX_CMD_ARG_COUNT], int argCountS, int argCountE, BitMap* bitmap, BitMap* refbitmap, bool beReverse=false);
	int GetOutliers_SinglToken_RefMap(char *arg, BitMap* bitmap, BitMap* refbitmap, bool beReverse=false);

	int Search_SingleSegment(char *querySeg, OUT LISTBITMAPS& bitmaps);
	int Search_MultiSegments(char **querySegs, int segSize, OUT LISTBITMAPS& bitmaps);
	int SearchSingleInPattern(LogPattern* logPat, char *querySeg, short querySegTag, BitMap* bitmap);
	int SearchMultiInPattern(LogPattern* logPat, char **querySegs, int argCountS, int argCountE, short* querySegTags, int* querySegLens, BitMap* bitmap);
	int Search_AxB_InPattern(LogPattern* logPat, char* queryStrA, char* queryStrB, short qATag, short qBTag, BitMap* bitmap);
	int SearchInVar_Union(int varName, char *querySeg, short querySegTag, OUT BitMap* bitmap);
	int SearchInVar_AxB_Union(int varName, char *queryA, char *queryB, short qATag, short qBTag, OUT BitMap* bitmap);
	int SearchInVar_Pushdown(int varName, char *querySeg, short querySegTag, int queryType, OUT BitMap* bitmap);
	int SearchInVar_Pushdown_RefMap(int varName, char *querySeg, short querySegTag, int queryType, OUT BitMap* bitmap, BitMap* refBitmap);
	
	int IsSearchWithLogic(char *args[MAX_CMD_ARG_COUNT], int argCount);
	int SearchSingleInPattern_RefMap(LogPattern* logPat, char *querySeg, short querySegTag, BitMap* bitmap, BitMap* refBitmap);
	int Search_AxB_InPattern_Logic(LogPattern* logPat, char* queryStrA, char* queryStrB, BitMap* bitmap, BitMap* refBitmap);
	int SearchMultiInPattern_RefMap(LogPattern* logPat, char **querySegs, int argCountS, int argCountE, short* querySegTags, int* querySegLens, BitMap* bitmap, BitMap* refBitmap);
	int SearchByLogic(char *args[MAX_CMD_ARG_COUNT], int argCount, OUT LISTBITMAPS& bitmaps);
	int SearchByLogic_OR(char *args[MAX_CMD_ARG_COUNT], int argCountS, int argCountE, OUT LISTBITMAPS& bitmap);
	int SearchByLogic_and(char *args[MAX_CMD_ARG_COUNT], int argCountS, int argCountE, OUT LISTBITMAPS& bitmap);
	int SearchByLogic_norm(char *args[MAX_CMD_ARG_COUNT], int argCountS, int argCountE, OUT LISTBITMAPS& bitmap);
	int SearchByLogic_norm_RefMap(char *args[MAX_CMD_ARG_COUNT], int argCountS, int argCountE, OUT LISTBITMAPS& bitmaps, LISTBITMAPS refbitmaps);
	int SearchByLogic_norm_or(char *args[MAX_CMD_ARG_COUNT], int argCountS, int argCountE, OUT LISTBITMAPS& bitmaps);
	int SearchByLogic_not(char *args[MAX_CMD_ARG_COUNT], int argCountS, int argCountE, OUT LISTBITMAPS& bitmap);

	int Materialization(int pid, BitMap* bitmap, int bitmapSize, int matSize);
	int Materializ_Pats(int varname, BitMap* bitmap, int entryCnt, OUT char* vars);
	int Materializ_Subpat(SubPattern* subpat, int varname, BitMap* bitmap, int entryCnt, OUT char* vars);
	int Materializ_Dic(int varname, BitMap* bitmap, int entryCnt, OUT char* vars);
	int Materializ_Var(int varname, BitMap* bitmap, int entryCnt, OUT char* vars);
	int MaterializOutlier(BitMap* bitmap, int cnt, int refNum);
	int Materializ_Dic_Kmp(int varname, BitMap* bitmap, int entryCnt, OUT char* vars);

public:
	int IsConnect();

	/*
	** reference : establish access to logStore
	** return : patterns count 0: disconnected
	*/
	int Connect(char *logStorePath, char* fileName);

	/*
	** reference : close a connection access to logStore
	** return : 1 connected 0: disconnected
	*/
	int DisConnect();

	int GetPatterns(OUT vector< pair<string, LogPattern> > &patterns);
	int GetPatternById(IN int patId, OUT char** patBody);
	int GetVariablesByPatId(int patId, RegMatch *regResult);
	int GetValuesByPatId_VarId(int patId, int varId, OUT char* vars);
	int GetValuesByPatId_VarId_Reg(char *args[MAX_CMD_ARG_COUNT], int argCount, OUT char* vars, BitMap* bitmap);
	int SearchByReg(const char *regPattern);
	int SearchByWildcard_Token(char *args[MAX_CMD_ARG_COUNT], int argCount, int matNum);

};

#endif
