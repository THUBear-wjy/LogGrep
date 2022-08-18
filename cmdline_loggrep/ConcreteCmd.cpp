#include "ConcreteCmd.h"

static int initLevel[3] = {1,0,0};
void ThuCmd::Execute(char *args[MAX_CMD_ARG_COUNT], int argCount)
{
	Initial(args, argCount);
    if (Cmd_HelpCallback(Help))
	{
		return;
	}
	else
	{
		//syntactic error
		Cmd_SyntacticError();
	}
}

void ThuCmd::Help()
{
	printf("%s\n", Thu_CMDHELP_DESC);
	printf("%s\t\t\t%s\n", Thu_EXIT, Thu_EXIT_DESC);
	printf("%s\t\t\t%s\n", Thu_GET, Thu_GET_DESC);
	printf("%s\t\t\t%s\n", Thu_FILE, Thu_FILE_DESC);
	printf("%s\t\t\t%s\n", Thu_CONNECT, Thu_CON_DESC);
	printf("%s\t\t\t%s\n", Thu_DISCONNECT, Thu_DISCON_DESC);
}

////////////////////////////////NoSQLt_CONNECT//////////////////////////////////////////////////////
void ThuConnectCmd::Execute(char *args[MAX_CMD_ARG_COUNT], int argCount)
{
	Initial(args, argCount);
    if (Cmd_HelpCallback(Help))
	{
		return;
	}
	if (m_argCount >= 2)
	{
		ConnectLogStore();
	}
	else
	{
		//syntactic error
		Cmd_SyntacticError();
	}
}

void ThuConnectCmd::ConnectLogStore()
{
	;
}

void ThuConnectCmd::Help()
{
    printf("%s <file-in-path>\n", Thu_CONNECT);
}

void ThuConnectCmd::Cmd_SyntacticError()
{
	printf(Thu_DCMDHELP_DESC,Thu_CONNECT);
}
