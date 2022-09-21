#ifndef CMD_MANAGER_H
#define CMD_MANAGER_H

#include <stdio.h>
#include "CmdInvoker.h"

// global var, defined in main
extern int main_while_flag;
extern int  thread_while_flag;

class CmdManager
{
public:
	CmdManager();
	~CmdManager();

public:
    int DoCmdAnalysis();
    int DoCmdAnalysis(char* path, char *m_lineRead);

private:
    bool ICanWork(void);

    int AnalysisCmd(char *inputCharArray, char *args[MAX_CMD_ARG_COUNT]);
    int AnalysisCmd_ForTest(char* path, char *inputCharArray, char *args[MAX_CMD_ARG_COUNT]);

    char *ReadCmdLine();

    /*
    ** reference : Prompt according to thu connected status
    ** return : connected:Thu_PROMPT    disconnect:Thu_PROMPT_UNCONNECT
    */
    char *GetThuPrompt();

private:
	CmdInvoker *m_invoker;
    // user input char*
    char *m_lineRead;
	// store cmd args
	char *m_args[MAX_CMD_ARG_COUNT];
};

#endif 
