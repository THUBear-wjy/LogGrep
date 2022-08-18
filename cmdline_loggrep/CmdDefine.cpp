#include "CmdDefine.h"

int Split_NoDelim(IN char *source, IN char *sep, OUT char *pArray[MAX_CMD_PARAMS_COUNT])
{
	int nParamCount = 0;
	char *pParam = strtok(source, sep);
	while (pParam && nParamCount < MAX_CMD_PARAMS_COUNT)
	{
		int nTempLen = strlen(pParam);
		pArray[nParamCount] = new char[nTempLen + 1]{'\0'};
		//trim space and tab at both begin and end
		memcpy(pArray[nParamCount], pParam, nTempLen);
		nParamCount++;
		pParam = strtok(NULL, sep);
	}
	return nParamCount;
}

int Split(IN char *source, IN char *sep, OUT char *pArray[MAX_CMD_PARAMS_COUNT])
{
	int nParamCount = 0;
	char *pParam;
	int return_value = Mystrtok(source, sep, pParam);
	while ((pParam || return_value != 0) && nParamCount < MAX_CMD_PARAMS_COUNT)
	{
		if(pParam)
		{
			int nTempLen = strlen(pParam);
			pArray[nParamCount] = new char[nTempLen + 1]{'\0'};
			//trim space and tab at both begin and end
			//int nDesLen = Trim(pParam, pArray[nParamCount]);
			memcpy(pArray[nParamCount], pParam, nTempLen);
			nParamCount++;
		}
		if(return_value != 0)
		{
			pArray[nParamCount] = new char[2]{'\0'};
			pArray[nParamCount][0] = char(return_value);
			nParamCount++;
		}
		return_value = Mystrtok(NULL, sep, pParam);
	}
	return nParamCount;
}

//by wjy
int Mystrtok(IN char *s, IN char *delim, char* &buf)
{

    const char *spanp;
    int c, sc;
    char *tok;
    static char *last;
    int dcount = 0;
    buf = NULL;
    if (s == NULL && (s = last) == NULL)
        return 0;
    
    c = *s++;
    for (spanp = delim; (sc = *spanp++) != 0;) {
        if (c == sc){           
            last = s;
            return c;
        }
    }
    
    if (c == 0) {                 
        last = NULL;
        return c;
    }
    
    tok = s - 1;
    for(;;){
        c = *s++;
        spanp = delim;
        do {
            if ((sc = *spanp++) == c) {
                int return_value = 0;
                if (c == 0){
                    s = NULL;
                    return_value = 0;
                }
                else{
                    return_value = s[-1];
                    s[-1] = 0;
                }   
                last = s;
                (buf) = (tok);
                return return_value;
            }
        } while (sc != 0);
    }
//    return strtok(s, delim);
}

int Trim(IN char *sou, OUT char *des)
{
	int nDesLen = 0;
	int nBeginPos = 0;
	int nEndPos = strlen(sou) - 1;
	while (sou[nBeginPos] == ' ' || sou[nBeginPos] == '\t' || sou[nBeginPos] == '\n')
	{
		++nBeginPos;
	}
	while (sou[nEndPos] == ' ' || sou[nEndPos] == '\t' || sou[nEndPos] == '\n')
	{
		--nEndPos;
	}
	if(des == NULL)
	{
		des = new char[strlen(sou) + 1]{'\0'};
	}
	nDesLen = (nEndPos - nBeginPos + 1);
	strncpy(des, sou + nBeginPos, nDesLen + 1);
	return nDesLen < 0 ? 0 : nDesLen;
}

int IsValidParamsFlag(char lFlag, char rFlag)
{
	if (lFlag == Thu_INPUT_RFLAG && rFlag == Thu_INPUT_LFLAG)
	{
		return 1;
	}
	printf(Thu_PROMPT_PARAMFLAG_ERROR);
	return 0;
}

int IsValidIpAddress(char *pSou)
{
	int nRet = 0;
	int nIp[4] = {0};
	nRet = sscanf(pSou, "%d.%d.%d.%d", &nIp[0], &nIp[1], &nIp[2], &nIp[3]);
	if (nRet == 4)
	{
		for (int i = 0; i < 4; i++)
		{
			if (0 > nIp[i] || 255 < nIp[i])
			{
				nRet = 0;
				break;
			}
		}
	}
	else
	{
		nRet = 0;
	}
	return nRet;
}

void PromptAndCtrlShowingRecords(int curNum, int totalNum, int *flag)
{
	if (curNum < DEF_SHOW_RECORDS_COUNT)
	{
		*flag = 1;
	}
	else if (curNum == DEF_SHOW_RECORDS_COUNT)
	{
		printf("Total query items num:%d, %s",totalNum, Thu_PROMPT_CONTINUE_SHOW);
        char cTmp = getchar();
        *flag = (cTmp == 'y') ? 1 : 0;
		while ( (cTmp = getchar()) != '\n' && cTmp != EOF ) ;//empty stdin
	}
}

int StrToInt(IN char *srcStr, OUT int *desInt)
{
	long desLong = 0;
	int nRet = StrToLong(srcStr, &desLong);
	//convert forced
	*desInt = (int)desLong;
	return nRet;
}
int StrToLong(IN char *srcStr, OUT long* desLong)
{
	int nRet = 0;
	if (srcStr == NULL)
	{
		*desLong = 0;
		nRet = 0;
	}
	else if (stricmp(srcStr, "0") == 0)
	{
		*desLong = 0;
		nRet = 1;
	}
	else
	{
		*desLong = atol(srcStr);
		nRet = *desLong != 0 ? 1 : 0;
	}
	return nRet;
}
int StrToDouble(IN char *srcStr, OUT double* desDouble)
{
	int nRet = 0;
	if (srcStr == NULL)
	{
		*desDouble = 0.0;
		nRet = 0;
	}
	else
	{
		*desDouble = atof(srcStr);
		nRet = 1;
	}
	return nRet;
}

void PrintThuPrompt()
{
    printf("%s", Thu_PROMPT);
}

void PrintBeforeThuPrompt()
{
    printf("\n");
}

void PrintThuCopr()
{
	printf("%s [version %s]\n", Thu_THIS_NAME, Thu_THIS_VER);
	printf("%s\n", Thu_THIS_COPR);
}

int GetCmdArgs(IN char *pInputCharArray, IN char *sep, OUT char *args[MAX_CMD_ARG_COUNT])
{
	int nSouLen = strlen(pInputCharArray);
    char *pStrArray[MAX_CMD_PARAMS_COUNT] = { 0 };
	int nCount = Split(pInputCharArray, sep, pStrArray);
	// for(int i=0;i< nCount;i++)
    // {
    //     printf("%d: %s\n", i, pStrArray[i]);
    // }
	for (int i = 0; i < nCount; i++)
	{
        //i must be smaller than MAX_CMD_ARG_COUNT
        if(i < MAX_CMD_ARG_COUNT)
        {
            strncpy(args[i], pStrArray[i], strlen(pStrArray[i]) + 1);
        }
		//delete
		if (pStrArray[i])
		{
			delete(pStrArray[i]);
		}
	}
	return nCount;
}

int FlushStdin(char *sou, int souLen)
{
    return fflush(stdin);
}
