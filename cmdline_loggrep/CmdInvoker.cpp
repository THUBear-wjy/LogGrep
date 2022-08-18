#include "CmdInvoker.h"

extern int main_while_flag;

CmdInvoker::CmdInvoker()
{
	m_cmdOperator = new CmdOperator();
	m_logDisp = new LogDispatcher();
	m_pThuCmd = new ThuCmd(m_cmdOperator);

	InitParamTree();
}

CmdInvoker::~CmdInvoker()
{
	//delete object
	if (m_pThuCmd)
	{
		delete(m_pThuCmd);
		m_pThuCmd = null;
	}
	if (m_cmdOperator)
	{
		delete(m_cmdOperator);
		m_cmdOperator = null;
	}
	if (m_logDisp)
	{
		delete(m_logDisp);
		m_logDisp = null;
	}

	ReleaseParamTree(m_paramTree);
}

void CmdInvoker::InitParamTree()
{
	m_paramTree = new ParamTree();
	m_paramTree->Set("thulr", 0);
	string params[][MAX_CMD_ARG_COUNT]={
		{Thu_GET,Thu_HELP," "},
		{Thu_GET,Thu_PATTERNS, Thu_INPUT_EXTEND," "},
		{Thu_GET,Thu_PATTERNS," "},
		{Thu_GET,Thu_PATBODY,Thu_INPUT_PAT,"<patternId>"," "},
		{Thu_GET, Thu_VARS, Thu_INPUT_PAT,"<patternId>"," "},
		{Thu_GET, Thu_VALUES, Thu_INPUT_PAT, "<patternId>", Thu_INPUT_VAR, "<variableId>", Thu_INPUT_REG, "<regular>", Thu_INPUT_EXTEND, " "},
		{Thu_GET, Thu_VALUES, Thu_INPUT_PAT, "<patternId>", Thu_INPUT_VAR, "<variableId>", Thu_INPUT_REG, "<regular>"," "},
		{Thu_GET, Thu_VALUES, Thu_INPUT_PAT, "<patternId>", Thu_INPUT_VAR, "<variableId>"," "},
		{Thu_GET, Thu_ALL, Thu_INPUT_MATCH,"<wildcard>"," "},
		{Thu_GET, Thu_ALL, Thu_INPUT_REG,"<regular>"," "},
		{Thu_GET, Thu_ALL, Thu_INPUT_LOGIC,"<logic>"," "},
		{Thu_FILE,Thu_HELP," "},
		{Thu_FILE,Thu_INPUT, "<file-in-path>"," "},
		{Thu_GREP,Thu_INPUT, "<file-in-path>",Thu_INPUT_MATCH, "<wildcard>"," "},
		{Thu_ELASTIC,Thu_INPUT, "<file-in-path>",Thu_INPUT_MATCH, "<wildcard>"," "},
	};
	int flag[][MAX_CMD_ARG_COUNT]={
		{0,0,31},
		{0,0,1,321},
		{0,0,322},
		{0,0,1,2,33},
		{0,0,1,2,34},
		{0,0,1,2,1,2,1,2,1,351},
		{0,0,1,2,1,2,1,2,352},
		{0,0,1,2,1,2,353},
		{0,0,1,2,361},
		{0,0,1,2,362},
		{0,0,1,2,363},
		{0,0,41},
		{0,1,2,42},
		{0,1,2,1,2,51},
		{0,1,2,1,2,52}
	};
	int len[]={3,4,3,5,5,10,9,7,5,5,5,3,4,6,6};
	for(int i=0;i<sizeof(len)/sizeof(len[0]);i++)
	{
		AddChildlineToTree(m_paramTree, params[i],flag[i],len[i]);
	}
}

void CmdInvoker::AddChildlineToTree(ParamTree* root, string* childline, int* flags, int len)
{
	if(root == NULL || len <= 0) return;
	ParamTree* temp =  NULL;
	for(int i=0;i< root->Children.size();i++)
	{
		if (stricmp(root->Children[i]->NodeValue.c_str(), childline[0].c_str()) == 0)
		{
			temp = root->Children[i];
			break;
		}
	}
	if(temp == NULL)//not find
	{
		temp = root->AddChild(childline[0], flags[0]);
	}
	AddChildlineToTree(temp, childline +1, flags +1, len-1);
}


void CmdInvoker::ReleaseParamTree(ParamTree* root)
{
	if(root != NULL)
	{
		for(int i=0; i< root->Children.size();i++)
		{
			ReleaseParamTree(root->Children[i]);
		}
        delete root;
    }
}

void CmdInvoker::PreOrder_FullPath(ParamTree* root, string *p, int icur)
{
	if(root != NULL)
	{
		p[icur] = root->NodeValue;
		int childCount = root->Children.size();
		if(childCount > 0)
		{
			for(int i=0; i< childCount;i++)
			{
				PreOrder_FullPath(root->Children[i], p, icur+1);
			}
		}
		else//leaf node
		{
			for(int i=0;i<=icur;i++)
			{
				printf("%s ",p[i].c_str());
			}
			printf("\n");
		}
    }
}

//return 10+: ok 0: false
int CmdInvoker::SearchParamTree(ParamTree* root, char *args[MAX_CMD_ARG_COUNT], int argCount, int icur)
{
	while(strlen(args[icur]) == 1 && args[icur][0] == ' ') icur++;
	if((icur == argCount+1 || root->Children.size() == 0) && root->Flag >=10)
	{
		//if(root->Flag >=10)
		//{
			return root->Flag;
		//}
		//return 0;
	}
	for(int i=0; i< root->Children.size();i++)
	{
		//skip input value
		if (root->Children[i]->Flag >1 || stricmp(args[icur], root->Children[i]->NodeValue.c_str()) == 0)
		{
			//pruning, no rollback
			return SearchParamTree(root->Children[i], args, argCount, icur +1);
		}
	}
	return 0;
}

bool CmdInvoker::IsThulrConnect()
{
    return m_logDisp->IsConnect();
	//return m_cmdOperator->IsThulrConnect();
}

void CmdInvoker::DoInvokeCmd(char *args[MAX_CMD_ARG_COUNT], int argCount)
{
	if (stricmp(args[0], Thu_CONNECT) == 0)
	{//con...
		if(argCount < 5)
		{
			m_logDisp->Connect(args[2]);
			//m_cmdOperator->Connect(args[2], NULL);
		}
		else
		{
			m_logDisp->Connect(args[2]);
			//m_cmdOperator->Connect(args[2], args[4]);
		}
		return;
	}
	if (stricmp(args[0], Thu_EXIT) == 0)
	{
		m_logDisp->DisConnect();
		//m_cmdOperator->DisConnect();
        main_while_flag = FALSE;
		return;
	}
	if (stricmp(args[0], Thu) == 0)
	{
		m_pThuCmd->Execute(args, argCount);
		return;
	}
	if(!IsThulrConnect())
	{
		printf("please connect the logstore before your operation!\n");
		return;
	}

	if (stricmp(args[0], Thu_GET) == 0 || stricmp(args[0], Thu_FILE) == 0)
	{
		if(argCount <= 6 || args[6][0]==' ') return;
		int funcNum = SearchParamTree(m_paramTree, args, argCount, 0);
		ExeFunc(funcNum, args, argCount);
	}
	else if (stricmp(args[0], Thu_GREP) == 0 || stricmp(args[0], Thu_ELASTIC) == 0)
	{
		int funcNum = SearchParamTree(m_paramTree, args, argCount, 0);
		ExeFunc(funcNum, args, argCount);
	}
	else if (stricmp(args[0], Thu_DISCONNECT) == 0)
	{
		m_logDisp->DisConnect();
		//m_cmdOperator->DisConnect();
	}
	else
	{
		//syntactic error
		m_pThuCmd->Cmd_SyntacticError();
	}
}

void CmdInvoker::DoInvokeCmd_ForTest(char* path, char *args[MAX_CMD_ARG_COUNT], int argCount)
{
	m_logDisp->Connect(path);
	m_logDisp->SearchByWildcard(args, argCount);
}

int CmdInvoker::ExeFunc(int num, char *args[MAX_CMD_ARG_COUNT], int argCount)
{
	string p[20]={""};
	switch(num)
	{
		case 31:
		{
			PreOrder_FullPath(m_paramTree->Children[0], p, 0);
			break;
		}
		case 361:
		case 363:
		{
			char **args_ = new char*[argCount - 6];
			for(int i= 0; i < argCount - 6; i++)
			{
				args_[i] = args[i+6];
			}
			m_logDisp->SearchByWildcard(args_, argCount-6);
			delete[] args_;
			break;
		}
		case 362:
			//m_cmdOperator->SearchByReg(args, argCount);
			break;
		default:
			printf("This is a beta version, and the other functions are not yet available!");
			//printf(Thu_DCMDHELP_DESC,"<cmd>");
			break;
	}
	return 0;
}



