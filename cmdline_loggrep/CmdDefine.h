/*******************************
 * Write By chen junchao       *
 * 2021-03-03                  *
 * For LogReducer       *
 * Copyright: THU_HPC V1.0     * 
 *******************************/
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define stricmp strcasecmp

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef null
#define null NULL
#endif

//sys define
#define MAX_TAG_COUNT 100000
#define MAX_TAGNAME_LENGTH 128 //do not modify this value
#define MAX_TAG_SHOW_COUNT 500
#define MAX_CMD_ARG_COUNT 50
#define MAX_CMD_ARGSTR_LENGTH 512
#define MAX_CMD_PARAMNAME_LENGTH 256
#define MAX_CMD_PARAMS_COUNT 2000
#define DEF_SHOW_RECORDS_COUNT 30
#define MAX_SHOW_RECORDS_COUNT 300  //too big will be meaningless
#define MAX_ONELINE_STR_LENGTH 1024 //max input char length at one line, can change if needs

#define MAX_HISCMD_COUNT 500 //max history cmd storage size, can change if needed

//for readline to remember keys
#define LEVEL_1_KEY_COUNT 8
#define LEVEL_2_KEY_COUNT 6
#define INPUT_KEY_COUNT 8

//command key words define,ignore case sensitive
#define Thu                 "thulr"
#define Thu_EXIT            "exit"
#define Thu_GET             "select"
#define Thu_FILE            "file"
#define Thu_CONNECT         "con"
#define Thu_DISCONNECT		"discon"
#define Thu_GREP            "grep"
#define Thu_ELASTIC         "esearch"

//command secondary key words define,ignore case sensitive
#define Thu_PATTERNS "patlist"
#define Thu_PATBODY "patbody"
#define Thu_VARS "varlist"
#define Thu_VALUES "vals"
#define Thu_ALL "*"
#define Thu_HELP "--help"

//command params define,ignore case sensitive
#define Thu_INPUT_RFLAG '('
#define Thu_INPUT_LFLAG ')'
#define Thu_INPUT_SPACEFLAG " "
#define Thu_INPUT_SPLIT ","
#define Thu_INPUT_PAT "-p"
#define Thu_INPUT_VAR "-v"
#define Thu_INPUT_MATCH "-m"
#define Thu_INPUT_LOGIC "-l"
#define Thu_INPUT_REG "-reg"
#define Thu_INPUT_COMPARE "-cmp"
#define Thu_INPUT_EXTEND "-ex"
#define Thu_INPUT "-i"
#define Thu_OUTPUT "-o"
#define Thu_OUTPUT_TERM "term" //output to terminal

//Thu prompt and version
#define Thu_PROMPT "Thulr>"
#define Thu_PROMPT_UNCONNECT "Thulr(disconnect)#"
#define Thu_THIS_VER "1.0"
#define Thu_THIS_NAME "Thulr CmdLine Tool"
#define Thu_THIS_COPR "All Right Reserved (c) 2021 THU HPC."

// command and example
#define Thu_GET_TAG_NAMES_BYIDS_CMD "%s %s %s %s (<id1,id2,id3...>)\n"
#define Thu_GET_TAG_NAMES_BYIDS_CMD_EG "eg:%s %s %s %s (11,22,33)\n"
#define Thu_GET_TAG_NAMES_BYIDS_CMD_EX "ex:get tag names by tag ids.\n"

//command key words description

#define Thu_EXIT_DESC "ex:exit 'Thulr Cmd Tool'."
#define Thu_GET_DESC "ex:get information of vars or other items."
#define Thu_FILE_DESC "ex:format input from file."
#define Thu_CON_DESC "ex:connect the logStore, you must exe connection before you operate other commands."
#define Thu_DISCON_DESC "ex:disconnect the logStore."

#define Thu_SYSHELP_DESC "please input 'thulr --help' for more information!\n"
#define Thu_CMDHELP_DESC "please input '<cmd> --help' for the detailed information about the command!\n"
#define Thu_QCMDHELP_DESC "you can input '%s --help' to quickly located the detailded command!\n"
#define Thu_DCMDHELP_DESC "syntactic error: please input '%s --help' for the detailed information about this command!\n"


#define Thu_INPUT_IDS_PROMPT "tag ids,they should be bigger than 0. eg:(1,2,3,4,5,6) ."

//prompt information
#define Thu_PROMPT_SYNTACTIC_ERROR "Syntactic error: can not identify this command!\n"
#define Thu_PROMPT_CONTINUE_SHOW "Continue to show rest records?(y/n):"
#define Thu_PROMPT_MAXRECORDS_SHOW "query %d records.\n"
#define Thu_PROMPT_GETPARAMS_FAILED "Get Input Params Failed,Please Check Your Input Params,Ensure No Space Among.\n"
#define Thu_PROMPT_READ_SUMMARY "Max Value:%f\nMin Value:%f\nAverage Value:%f\nSum:%f\nPercentage Value:%f\n"
#define Thu_PROMPT_PARAMFLAG_ERROR "Invalid Params Left or Right Flag.Valid Flag Mustbe '(' and ')'.\n"
#define Thu_PROMPT_TOINT_FAILED "Convert Param To Int(Long) Failed,Please Check Your Input Params.\n"
#define Thu_PROMPT_TOUINT_FAILED "Convert Param To Unsigned Int(Long) Failed,Please Check Your Input Params.\n"


/*
** reference : split string to array
** return : length of array
** params :
**  source : (<id1,id2,id3>)
**  sep : "," " "
**  array : array of splitted substring, use new inside, so must delete outside
*/
extern int Split_NoDelim(IN char *source, IN char *sep, OUT char *pArray[MAX_CMD_PARAMS_COUNT]);
extern int Split(IN char *source, IN char *sep, OUT char *pArray[MAX_CMD_PARAMS_COUNT]);
extern int Mystrtok(IN char *s, IN char *delim, OUT char* &buf);
/*
** reference : trim '\t' and "space" begin and end 
** return : length of des
*/
extern int Trim(IN char *sou, OUT char *des);
/*
** reference : fflush(stdin) is not standard C roles,it rest with compiler.
** return :
*/
extern int FlushStdin(char *sou, int souLen);
/*
** reference : check param flag is '(' and ')'
** return : true:1 false:0
*/
extern int IsValidParamsFlag(char lFlag, char rFlag);
/*
** reference : check ip address is valid
** return : false:0 true:4
*/
extern int IsValidIpAddress(char *pSou);
/*prompt user,and just show first n records
** if user input "n", stop show the rest and return 0
** if user input "y", show the rest and return 1
*/
extern void PromptAndCtrlShowingRecords(IN int curNum, IN int totalNum, OUT int *flag);
//return: false:0 true:1
extern int StrToInt(IN char *srcStr, OUT int *desInt);
extern int StrToLong(IN char *srcStr, OUT long *desLong);
extern int StrToDouble(IN char *srcStr, OUT double *desDouble);
//print thu prompt
extern void PrintThuPrompt();
extern void PrintBeforeThuPrompt();
//print thu copr
extern void PrintThuCopr();
/*
** reference : get cmd args splitted by space
** return : length of args array
** params :
**  pInputCharArray : user input
**  args : output args array
*/
extern int GetCmdArgs(IN char *pInputCharArray, IN char *sep, OUT char *args[MAX_CMD_ARG_COUNT]);
