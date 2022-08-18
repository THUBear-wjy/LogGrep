#ifndef CMD_OPERATOR_H
#define CMD_OPERATOR_H

#include <fstream>
#include <cstring> 
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <algorithm>
#include <vector>
//#include <boost/regex.hpp>
#include "CmdDefine.h"
#include "LogStructure.h"
#include "SearchAlgorithm.h"
#include "LogStore_API.h"
#include "LogDispatcher.h"

using namespace std;
extern int  thread_while_flag;

class CmdOperator
{
public:
	CmdOperator();
	~CmdOperator();

private:
	char m_filename[MAX_FILE_NUM][MAX_FILE_NAMELEN];
	char m_filePath[MAX_DIR_PATH];
	LISTCHARS m_fileList;
	LogStoreApi* m_logStore;

private:
	int TraveDir(char* dirPath);
	int ReadFileToBuffByFread(FILE *pFile);
	char* GetSubstr(const char*str, int start, int end);

public:

	int IsThulrConnect();
	int Connect(char *logStore, char* logFileName);
	void DisConnect();

	void ShowPatterns(int flag = 0);
	void ShowPatternByPatId(const char *patId);
	void ShowVariablesByPatId(const char *patId);
	void ShowValuesByPatId_VarId(const char *patId, const char* varId);
	void ShowValuesByPatId_VarId_Reg(char *args[MAX_CMD_ARG_COUNT], int argCount, int flag = 0);
	int SearchByReg(char *args[MAX_CMD_ARG_COUNT], int argCount);
	int SearchByWildcard(char *args[MAX_CMD_ARG_COUNT], int argCount);

	int SearchByExact_LCS(const char *queryStr);
	int SearchByExact_Token(char *queryStr);

};

#endif
