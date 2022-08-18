#ifndef CMD_LOGDISPATCH_H
#define CMD_LOGDISPATCH_H

#include <iostream>
#include <queue>
#include "LogStore_API.h"

class LogDispatcher
{
public:
	LogDispatcher();
	~LogDispatcher();

private:
	std::queue<char*> m_filequeue;
	LogStoreApi* m_logStores[MAX_FILE_CNT];
	int m_nServerHandle;
	int m_fileCnt;

	char** m_args;
	int m_argCount;
	int m_spid;//pid of each thread
	RunningStatus m_runt;

private:
	int CalRunningTime();
	int ResetRunningTime();
	int TraveDir(char* dirPath);
	void * SearchByWildcard_pthread_exe();

public:
	int Connect(char* dirPath);
	int IsConnect();
	void DisConnect();

	int SearchByWildcard(char *args[MAX_CMD_ARG_COUNT], int argCount);
	int SearchByWildcard_Seq(char *args[MAX_CMD_ARG_COUNT], int argCount);
	int SearchByWildcard_Thread(char *args[MAX_CMD_ARG_COUNT], int argCount);
	static void * SearchByWildcard_pthread(void *ptr);
};

#endif
