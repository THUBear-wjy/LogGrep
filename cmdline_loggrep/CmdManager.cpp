#include "CmdManager.h"
#include <iostream>
#include <pthread.h>
#include <signal.h>




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

int CmdManager::DoCmdAnalysis(char* path, char *m_lineRead)
{
	if (!ICanWork()) return -1;
    SetStopSignal();
    AnalysisCmd_ForTest(path, m_lineRead, m_args);
	return 0;
}



