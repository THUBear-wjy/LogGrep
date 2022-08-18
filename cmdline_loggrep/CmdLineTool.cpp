#include "CmdManager.h"
#include <stdio.h>
#include <pthread.h>

//global var
int main_while_flag;
int thread_while_flag;

int main(int argc, char *argv[])
{ 
    //new
    CmdManager *cmdMan = new CmdManager();
    if(argc == 1)
    { 
        main_while_flag = true;
        cmdMan->DoCmdAnalysis();
    }
    else if(argc == 3)
    {
        main_while_flag = false;
        cmdMan->DoCmdAnalysis(argv[1], argv[2]);
    }

	//delete
    if (cmdMan)
    {
        delete(cmdMan);
    }

	return 0;
}

