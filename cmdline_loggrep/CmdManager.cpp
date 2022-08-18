#include "CmdManager.h"
#include <iostream>
#include <pthread.h>
#include <signal.h>


const char *Level_1_Keywords[LEVEL_1_KEY_COUNT] = { Thu, Thu_EXIT, Thu_GET, Thu_FILE, Thu_CONNECT, Thu_DISCONNECT, Thu_GREP, Thu_ELASTIC};
const char *Level_2_Keywords[LEVEL_2_KEY_COUNT] = { Thu_PATTERNS, Thu_PATBODY, Thu_VARS, Thu_HELP, Thu_VALUES, Thu_INPUT};
const char *Input_Keywords[INPUT_KEY_COUNT] = { Thu_INPUT_PAT, Thu_INPUT_VAR, Thu_INPUT_MATCH, Thu_INPUT_REG, 
Thu_INPUT_COMPARE, Thu_OUTPUT, Thu_OUTPUT_TERM, Thu_INPUT_EXTEND};

//define the call back func
typedef const char *(CmdArrayCallBack)(unsigned int cmdIndex);

const char *GetMainCmdByIndex(unsigned int cmdIndex)
{
    if(cmdIndex >=  LEVEL_1_KEY_COUNT)
    {
        return NULL;
    }
    return Level_1_Keywords[cmdIndex];
}

const char *GetSecondCmdByIndex(unsigned int cmdIndex)
{
    if(cmdIndex >=  LEVEL_1_KEY_COUNT)
    {
        return NULL;
    }
    return Level_1_Keywords[cmdIndex];
}

const char *GetOtherCmdByIndex(unsigned int cmdIndex)
{
    if(cmdIndex >= LEVEL_2_KEY_COUNT)
    {
        if(cmdIndex >= LEVEL_2_KEY_COUNT + INPUT_KEY_COUNT)
        {
            return NULL;
        }
        return Input_Keywords[cmdIndex - LEVEL_2_KEY_COUNT];
    }
    return Level_2_Keywords[cmdIndex];
}

////////////////////////////////////////////Readline////////////////////////////////////////////////////////////
static char *CmdGenerator(const char *partCmdText, int state, CmdArrayCallBack pCallbackFun)
{
    static int keyWordArrayIndex = 0;
    static int partKeyWordLen = 0;
    if(0 == state)
    {
        keyWordArrayIndex = 0;
        partKeyWordLen = strlen(partCmdText);
    }

    const char *pCmdText = NULL;
    while((pCmdText = pCallbackFun(keyWordArrayIndex)))
    {
        keyWordArrayIndex++;

        if(!strncmp (pCmdText, partCmdText, partKeyWordLen))
        {
            return strdup(pCmdText);
        }
    }

    return NULL;
}

static char *MainCmdGenerator(const char *partCmdText, int state)
{
    return CmdGenerator(partCmdText,state,GetMainCmdByIndex);
}

static char *OtherCmdGenerator(const char *partCmdText, int state)
{
    return CmdGenerator(partCmdText,state,GetOtherCmdByIndex);
}

static char **CmdTabAutoCompletion(const char *partCmdText, int startIndex, int endIndex)
{
    char **pMatches = NULL;
    //the main cmd keys
    if ( 0 == startIndex)
    {
        pMatches = rl_completion_matches(partCmdText,MainCmdGenerator);
    }
    else
    {
        //the other cmd keys
        pMatches = rl_completion_matches(partCmdText,OtherCmdGenerator);
    }
    return pMatches;
}

static void InitReadLine(void)
{
    rl_attempted_completion_function = CmdTabAutoCompletion;
}
/////////////////////////////////////////////Signal/////////////////////////////////////////////////////////////
void SignalInvoker(int sig)
{
    //CTRL + Z
    if(sig == SIGTSTP)
    {
       thread_while_flag = false;
    }
    printf("\n");
}

void SetStopSignal()
{
    signal(SIGTSTP,SignalInvoker);
}
///////////////////////////////////////////CmdManager/////////////////////////////////////////////////////////


CmdManager::CmdManager()
{
	m_invoker = new CmdInvoker();
    m_lineRead = NULL;

	for (int i = 0; i < MAX_CMD_ARG_COUNT; i++)
	{
		m_args[i] = new char[MAX_CMD_ARGSTR_LENGTH];
	}
}

CmdManager::~CmdManager()
{
	if (m_invoker)
	{
		delete(m_invoker);
		m_invoker = NULL;
	}
	for (int i = 0; i < MAX_CMD_ARG_COUNT; i++)
	{
		if (m_args[i])
		{
			delete(m_args[i]);
			m_args[i] = NULL;
		}
	}
    if(m_lineRead)
    {
        free(m_lineRead);
        m_lineRead = NULL;
    }
}

bool CmdManager::ICanWork(void)
{
    if (!m_invoker)
	{
		return false;
	}
	PrintThuCopr();
	return true;
}

char *CmdManager::GetThuPrompt()
{
    if(m_invoker && m_invoker->IsThulrConnect())
    {
        return Thu_PROMPT;
    }
    else
    {
        return Thu_PROMPT_UNCONNECT;
    }
}

int CmdManager::AnalysisCmd(char *inputCharArray, char *args[MAX_CMD_ARG_COUNT])
{
	int argCount = 0;
	//do if inputCharArray is not null
    if (inputCharArray && *inputCharArray)
	{
        argCount = GetCmdArgs(inputCharArray, TOKEN, args);
        if(argCount > 0)
        {
            thread_while_flag = true;
            m_invoker->DoInvokeCmd(args, argCount);
        }
	}
	return 0;
}

int CmdManager::AnalysisCmd_ForTest(char* path, char *inputCharArray, char *args[MAX_CMD_ARG_COUNT])
{
	int argCount = 0;
	//do if inputCharArray is not null
    if (inputCharArray && *inputCharArray)
	{
        argCount = GetCmdArgs(inputCharArray, TOKEN, args);
        if(argCount > 0)
        {
            m_invoker->DoInvokeCmd_ForTest(path, args, argCount);
        }
	}
	return 0;
}

char *CmdManager::ReadCmdLine()
{
    //release buffer if it's malloced
    if(m_lineRead)
    {
        free(m_lineRead);
        m_lineRead = NULL;
    }
    //user readline to read user input
    m_lineRead = readline(GetThuPrompt());
    if(m_lineRead && *m_lineRead)
    {
        //add to history
        add_history(m_lineRead);
    }
    return m_lineRead;
}

int CmdManager::DoCmdAnalysis(char* path, char *m_lineRead)
{
	if (!ICanWork()) return -1;
    SetStopSignal();
    AnalysisCmd_ForTest(path, m_lineRead, m_args);
	return 0;
}

int CmdManager::DoCmdAnalysis()
{
	if (!ICanWork()) return -1;
    SetStopSignal();
    InitReadLine();
    while (main_while_flag)
	{
        PrintBeforeThuPrompt();
        ReadCmdLine();
        if(m_lineRead && *m_lineRead)
        {
            AnalysisCmd(m_lineRead,m_args);
        }
    }

	return 0;
}

