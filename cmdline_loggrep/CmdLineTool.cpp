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
    if(argc < 3)
    { 
        main_while_flag = false;
        printf("please input like: ./thulr_cmdline <full filepath> <query string>.\n");
        printf("for example: ./thulr_cmdline home/disk3/Hadoop/ \"job_12345_0001\"");
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

