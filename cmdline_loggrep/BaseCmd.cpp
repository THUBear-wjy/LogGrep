#include "BaseCmd.h"

BaseCmd::BaseCmd(CmdOperator *cmdOperator)
{
	m_cmdOperator = cmdOperator;
}

int BaseCmd::Initial(char *args[MAX_CMD_ARG_COUNT], int argCount)
{
	m_argCount = argCount;
	for (int i = 0; i < MAX_CMD_ARG_COUNT; i++)
	{
		if (i < argCount)
		{
			m_args[i] = args[i];
		}
		else
		{
			m_args[i] = 0;
		}
	}
	return 1;
}

BaseCmd::~BaseCmd()
{
	//clear data
	m_argCount = 0;
	for (int i = 0; i < MAX_CMD_ARG_COUNT; i++)
	{
		m_args[i] = 0;
	}
}
//get int params from string splitted by ','
int BaseCmd::GetIntParams(char *formatedString, int params[MAX_CMD_PARAMS_COUNT])
{
	if (formatedString == NULL)
		return 0;
	int nSouLen = strlen(formatedString);
	if (IsValidParamsFlag(formatedString[0], formatedString[nSouLen - 1]))
	{
		char *pStrArray[MAX_CMD_PARAMS_COUNT] = {0};
		int nCount = Split(formatedString, Thu_INPUT_SPLIT, pStrArray);
		for (int i = 0; i < nCount; i++)
		{
			//atoi will ignore "space" and "tab" before the real "int"
			int nValue = 0;
			int nRet = StrToInt(pStrArray[i], &nValue);
			if (nRet == 0)
			{
				//convert failed
				nCount = 0;
				printf(Thu_PROMPT_TOINT_FAILED);
				break;
			}
			//legal judgement
			if (i < MAX_CMD_PARAMS_COUNT)
			{
				params[i] = nValue;
			}
			//delete
			if (pStrArray[i])
			{
				delete (pStrArray[i]);
			}
		}
		return nCount;
	}
	return 0;
}
//get uint params from string splitted by ','
int BaseCmd::GetUIntParams(char *formatedString, unsigned int params[MAX_CMD_PARAMS_COUNT])
{
	int tempParams[MAX_CMD_PARAMS_COUNT] = {0};
	int nCount = GetIntParams(formatedString, tempParams);
	if (nCount > 0)
	{
		//check if params are all > 0
		for (int i = 0; i < nCount; i++)
		{
			if (tempParams[i] >= 0)
			{
				params[i] = tempParams[i];
			}
			else
			{
				//convert failed
				nCount = 0;
				printf(Thu_PROMPT_TOUINT_FAILED);
				break;
			}
		}
	}
	return nCount;
}
//get str params from string splitted by ','
int BaseCmd::GetStrParams(char *formatedString, char *params, int paramsLen)
{
	int nSouLen = strlen(formatedString);
	if (IsValidParamsFlag(formatedString[0], formatedString[nSouLen - 1]))
	{
		char *pStrArray[MAX_CMD_PARAMS_COUNT] = {0};
		int nCount = Split(formatedString, Thu_INPUT_SPLIT, pStrArray);
		for (int i = 0; i < nCount; i++)
		{
			strncpy(params + i * paramsLen, pStrArray[i], strlen(pStrArray[i]) + 1);
			//delete
			if (pStrArray[i])
			{
				delete (pStrArray[i]);
			}
		}
		return nCount;
	}
	return 0;
}
//help information callback
bool BaseCmd::CheckSyntactic(void (*pHelpCallback)(int[]), int level, int levelValue[], char *helpDesc)
{
	if (Cmd_HelpCallback(pHelpCallback, level, levelValue))
	{
		return true;
	}
	printf(Thu_DCMDHELP_DESC, helpDesc);
	return false;
}
//help information callback
bool BaseCmd::Cmd_HelpCallback(void (*pHelpCallback)(int[]), int level, int levelValue[])
{
	if (m_argCount == (level + 1))
	{
		if (stricmp(m_args[level], Thu_HELP) == 0)
		{
			pHelpCallback(levelValue);
			return true;
		}
	}
	return false;
}
//help information callback
bool BaseCmd::Cmd_HelpCallback(void (*pHelpCallback)())
{
	if (m_argCount == 2)
	{
		if (stricmp(m_args[1], Thu_HELP) == 0)
		{
			pHelpCallback();
			return true;
		}
	}
	return false;
}
//Syntactic error
void BaseCmd::Cmd_SyntacticError()
{
	printf(Thu_PROMPT_SYNTACTIC_ERROR);
	printf(Thu_SYSHELP_DESC);
	printf(Thu_CMDHELP_DESC);
}
void BaseCmd::Execute(char *args[MAX_CMD_ARG_COUNT], int argCount)
{
	return;
}
