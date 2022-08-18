#include "CmdOperator.h"

////////////////////////////////////////////init & private//////////////////////////////////////////////////////////
CmdOperator::CmdOperator()
{
	m_logStore = new LogStoreApi();
	//Alg_Test();
}
CmdOperator::~CmdOperator()
{
	if (m_logStore)
	{
		delete(m_logStore);
		m_logStore = null;
	}
}

//return: count of files in dir
int CmdOperator::TraveDir(char* dirPath)
{
    DIR *d; 
    struct dirent *file; 
	char cFileTmp[MAX_FILE_NAMELEN] = {'\0'};
	int index = 0;
    if(!(d = opendir(dirPath)))
    {
        return -1;//open failed
    }
    while((file = readdir(d)) != NULL)
    {
		//nowtime only match templates.txt
        if(strncmp(file->d_name, ".", 1) == 0 || file->d_name != "templates.txt")
            continue;
		sprintf(cFileTmp, "%s/%s", dirPath, file->d_name);
        strcpy(m_filename[index++], cFileTmp); 
    }
    closedir(d);
    return index;
}

int CmdOperator::ReadFileToBuffByFread(FILE *pFile)
{
	fseek(pFile,0,SEEK_END);//seek to end
	long nLength=ftell(pFile);
	char* buff = new char[nLength + 1];
	fseek(pFile,0,SEEK_SET);//seek to begin
	int readSize = fread(buff,sizeof(char),nLength,pFile);
	if(readSize != nLength)
	{
		return -1;
	}
	//file dato linked to list
	m_fileList.push_back(buff);
	return readSize;
}

char* CmdOperator::GetSubstr(const char* str, int start, int end)
{
	int len = end - start;
 	char *stbuf = new char[len + 1]{'\0'};
 	strncpy(stbuf, str + start, len);
 	return stbuf;
}

///////////////////Connect & Disconnect///////////////
int CmdOperator::IsThulrConnect()
{
	return m_logStore->IsConnect();
}

int CmdOperator::Connect(char* logStore, char* logFileName)
{
	int loadNum = m_logStore->Connect(logStore, logFileName);
	if(loadNum > 0)
	{
		Syslog("load patterns success,load num:%d, path:%s/%s.\n", loadNum, logStore, logFileName);
	}
	else
	{
		SyslogError("load patterns failed,path:%s/%s.\n",logStore, logFileName);
	}
	return loadNum;
}
void CmdOperator::DisConnect()
{
	int status = m_logStore->DisConnect();
	if(status == 0)
	{
		Syslog("release patterns successful.\n");
	}
	else
	{
		SyslogError("release patterns failed.\n");
	}
}

///////////////////////SELECT///////////////////////////

void CmdOperator::ShowPatterns(int flag)
{
	vector< pair<string, LogPattern> > name_score_vec;
	int vecSize = m_logStore->GetPatterns(name_score_vec);
	int nShowFlag = 0;
	if(vecSize <= 0)
	{
		Syslog("total query patterns num:%d\n",vecSize);
		return;
	}
	for (int i=0; i< vecSize;i++)
	{
		PromptAndCtrlShowingRecords(i, vecSize, &nShowFlag);
		if (0 == nShowFlag || FALSE == thread_while_flag)
		{
			break;
		}
		else
		{
			
			Syslog("%s %d\n",name_score_vec[i].first.c_str(), name_score_vec[i].second.Count);
			if(flag != 0)
			{
				Syslog("---\n");
				
				ShowPatternByPatId(name_score_vec[i].first.c_str());
				Syslog("-------------\n");
			}
			
		}
	}
	if(nShowFlag == 1)
	{
		Syslog("total query patterns num:%d\n",vecSize);
	}
}

void CmdOperator::ShowPatternByPatId(const char *patId)
{
	char* patBody=NULL;
	int patSize = m_logStore->GetPatternById(atoi(patId), &patBody);
	if(patSize > 0)
	{
		Syslog("%s\n",patBody);
	}
	else
	{
		Syslog("no pattern matched.\n");
	}
	if(patBody) delete patBody;
	return;
}

void CmdOperator::ShowVariablesByPatId(const char *patId)
{
	char* patBody = NULL;
	int patSize = m_logStore->GetPatternById(atoi(patId), &patBody);
	if(patSize > 0)
	{
		RegMatch regResult[MAX_FILE_LEN];
		int matchNum = m_logStore->GetVariablesByPatId(atoi(patId), regResult);
		if(matchNum <=0)
		{
			Syslog("no vars found in this pattern.\n");
		}
		else
		{
			for(int i=0;i<matchNum;i++)
			{
				char *temp = GetSubstr(patBody, regResult[i].So, regResult[i].Eo);
				Syslog("%s\n", temp);
				delete[] temp;
			}
			Syslog("total query vars num:%d.\n",matchNum);
		}

	}
	else
	{
		Syslog("no pattern matched.\n");
	}
	if(patBody) delete patBody;
}

void CmdOperator::ShowValuesByPatId_VarId(const char *patId, const char* varId)
{
	char* vars = new char[MAX_VALUE_LEN * MAX_FILE_LEN];
	int nShowFlag = 0;
	int num = m_logStore->GetValuesByPatId_VarId(atoi(patId),atoi(varId),vars);
	if(num > 0)
	{
		for (int i=0; i< num;i++)
		{
			PromptAndCtrlShowingRecords(i, num, &nShowFlag);
			if (0 == nShowFlag || FALSE == thread_while_flag)
			{
				break;
			}
			else
			{
				Syslog("%s\n",vars + i* MAX_VALUE_LEN);
			}
		}
		if(nShowFlag == 1)
		{
			Syslog("total query values num:%d\n",num);
		}
	}
	else
	{
		Syslog("query values failed. error code:%d\n", num);
	}
	if(vars) delete vars;
}
void CmdOperator::ShowValuesByPatId_VarId_Reg(char *args[MAX_CMD_ARG_COUNT], int argCount, int flag)
{
	char* vars = new char[MAX_VALUE_LEN * MAX_FILE_LEN];
	BitMap* regResult = new BitMap(MAX_FILE_LEN);
	int nShowFlag = 0;
	int num = m_logStore->GetValuesByPatId_VarId_Reg(args, argCount, vars,regResult);
	if(num > 0)
	{
		for(int i=0;i<num;i++)
		{
			PromptAndCtrlShowingRecords(i, num, &nShowFlag);
			if (1 == nShowFlag && TRUE == thread_while_flag)
			{
				Syslog("%s\n", vars + regResult->GetIndex(i) * MAX_VALUE_LEN);
			}
			
		}
		// if choose yes  or query num is zero
		if(nShowFlag == 1 || num == 0)
		{
			Syslog("total query values num:%d\n",num);
		}
	}
	else
	{
		Syslog("query values failed. error code:%d\n", num);
	}
	if(vars) delete vars;
}

int CmdOperator::SearchByExact_LCS(const char *queryStr)
{
	return 0;
}

int CmdOperator::SearchByExact_Token(char *queryStr)
{

	return 0;
}

//https://blog.csdn.net/yangbingzhou/article/details/51352648
int CmdOperator::SearchByReg(char *args[MAX_CMD_ARG_COUNT], int argCount)
{
	
	return 0;
}

int CmdOperator::SearchByWildcard(char *args[MAX_CMD_ARG_COUNT], int argCount)
{
	int num = m_logStore->SearchByWildcard_Token(args, argCount, MAX_MATERIAL_SIZE);
	return 0;
}