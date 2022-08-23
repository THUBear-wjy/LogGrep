#ifndef CMD_BASE_H
#define CMD_BASE_H

#include "CmdOperator.h"

class  BaseCmd
{
public:
	BaseCmd(CmdOperator* cmdOperator);
	~BaseCmd();

protected:
	//cmd args splitted by space
	char *m_args[MAX_CMD_ARG_COUNT];
	//cmd args count
	int m_argCount;
	//CmdOperator Object
	CmdOperator* m_cmdOperator;

protected:
	/*get params from string splitted by ','
	** if ids is not int, then take default value: 0
	** format string:(<id1,id2,id3>)
	** out params: array
	** return: count of params
	*/
	int GetIntParams(IN char *formatedString, OUT int params[MAX_CMD_PARAMS_COUNT]);
	
	/*get params from string splitted by ','
	** if ids is not unsigned int, then take default value: 0
	** format string:(<id1,id2,id3>)
	** out params: array
	** return: count of params
	*/
	int GetUIntParams(IN char *formatedString, OUT unsigned int params[MAX_CMD_PARAMS_COUNT]);
	int GetStrParams(IN char *formatedString, OUT char *params,int paramsLen);
	
	//Help information callback
    bool Cmd_HelpCallback(IN void(*pHelpCallback)());
    bool Cmd_HelpCallback(IN void(*pHelpCallback)(int[]), IN int level, IN int levelValue[]);
    bool CheckSyntactic(IN void(*pHelpCallback)(int[]), IN int level, IN int levelValue[], IN char* helpDesc);
	//init, true:1 false:0
    int Initial(char *args[MAX_CMD_ARG_COUNT], int argCount);

public:
    //Syntactic error
    virtual void Cmd_SyntacticError();
	virtual void Execute(char *args[MAX_CMD_ARG_COUNT], int argCount);
};

#endif
