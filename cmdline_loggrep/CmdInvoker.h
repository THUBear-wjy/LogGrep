#ifndef CMD_INVOKER_H
#define CMD_INVOKER_H

#include "CmdOperator.h"
#include "ConcreteCmd.h"

typedef struct ParamTree
{
	string NodeValue;
	vector<ParamTree*> Children;
	int Flag; //-1 invalid 0: keywords  1：input flag    2: inputdata   10+: execute flag
	ParamTree()
	{
		NodeValue="";
		Children.clear();
		Flag = -1;
	}
	void Set(string nodeV, int flag)
	{
		NodeValue= nodeV;
		Flag = flag;
	}
	ParamTree* AddChild(string nodeV, int flag)
	{
		ParamTree* child = new ParamTree();
		child->Set(nodeV, flag);
		Children.push_back(child);
		return child;
	}
	
}ParamTree;

class CmdInvoker
{
public:
	CmdInvoker();
	~CmdInvoker();

public:
	//Classify commands by the keyword
	void DoInvokeCmd(char *args[MAX_CMD_ARG_COUNT], int argCount);
	void DoInvokeCmd_ForTest(char* path, char *args[MAX_CMD_ARG_COUNT], int argCount);
	bool IsThulrConnect();

private:
	//CmdOperator Object
	CmdOperator* m_cmdOperator;
	LogDispatcher* m_logDisp;
	ThuCmd* m_pThuCmd;

	ParamTree* m_paramTree;

private:
	void AddChildlineToTree(ParamTree* root, string* childline, int* flags, int len);
	void InitParamTree();
	void ReleaseParamTree(ParamTree* root);
	void PreOrder_FullPath(ParamTree* root, string *p, int icur);
	int SearchParamTree(ParamTree* root, char *args[MAX_CMD_ARG_COUNT], int argCount, int icur);
	int ExeFunc(int num, char *args[MAX_CMD_ARG_COUNT], int argCount);
	void GetCmdFromFile(char *arg);
};

#endif
