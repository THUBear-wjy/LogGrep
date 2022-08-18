#ifndef CMD_CONCRETE_H
#define CMD_CONCRETE_H

#include "BaseCmd.h"

class ThuCmd : public BaseCmd
{
public:
	ThuCmd(CmdOperator* cmdOperator)
        :BaseCmd(cmdOperator){};
    ~ThuCmd(){};

public:
    void Execute(char *args[MAX_CMD_ARG_COUNT], int argCount);

private:

    static void Help();
};

class ThuConnectCmd : public BaseCmd
{
public:
	ThuConnectCmd(CmdOperator* cmdOperator)
		:BaseCmd(cmdOperator){};
	~ThuConnectCmd(){};

public:
    void Cmd_SyntacticError();
	void Execute(char *args[MAX_CMD_ARG_COUNT], int argCount);

private:
	void ConnectLogStore();
    static void Help();
};
#endif
