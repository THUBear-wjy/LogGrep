#ifndef CMD_LOGSTRUCTURE_H
#define CMD_LOGSTRUCTURE_H


#include <iostream>
#include <list>
#include <cstring> 
#include <string.h>
#include <map>
#include <bitset>
#include "CmdDefine.h"
#include <sys/time.h>

using namespace std;

#define Syslog				printf
#define SyslogError			//printf
#define SyslogWarnning		//printf
#define SyslogDebug       //printf
#define SysCodeRead       //printf
#define SyslogPerf        //printf
#define SyslogOut       printf
#define MAX_FILE_LEN		    	1000000
#define MAX_LINE_SIZE		     	122400
#define MAX_VALUE_LEN         1024
#define MAX_FILE_NUM		     	16
#define MAX_FILE_NAMELEN	  	512
#define MAX_DIR_PATH		      128
#define MAX_PATTERN_SIZE      300
#define MAX_PATNAME_SIZE      30
#define MAX_SUBATTR_CNT       50
#define MAX_SUBATTR_CLS       50
#define MAX_INPUT_TOKENSIZE   50
#define MAX_BITMAP_CELL       32
#define MAX_BITMAP_SIZE       32000 //MAX_FILE_LEN/ MAX_BITMAP_CELL
#define DEF_BITMAP_FULL       -99
#define MAX_UNION_LINELEN     100
#define MAX_UNION_BITMAPS     50
#define MAX_DICENTY_LEN       10
#define MAX_MATERIAL_SIZE     100
#define MAX_SESSION_SIZE      10  

//multi thread ctrl
#define MAX_THREAD_PARALLEL		1
#define MAX_FILE_CNT			    4000

#define DIR_PATH_DEFAULT		"../example_zip/Apache"
#define FILE_NAME_DEFAULT   "0.log.zip"

#define TOKEN             " \t:=,"
#define WILDCARD          "*"
#define TOKEN_VAR_LEFT    '<'
#define TOKEN_VAR_RIGHT   '>'
#define TOKEN_VAR_FLAG    'V'

#define FIXED_LEN_FLAG      'F'
#define VARIED_LEN_FLAG     'V'

#define LOGIC_AND         "and"
#define LOGIC_OR          "OR"
#define LOGIC_NOT         "not"
#define LOGIC_or          "or"

#define TAG_DELIM         0   //000000
#define TAG_0_9           1   //000001
#define TAG_A_F           2   //000010
#define TAG_a_f           4   //000100
#define TAG_G_Z           8   //001000
#define TAG_g_z           16  //010000
#define TAG_SYMBOL        32  //100000

//the name struct of a file
//Old: E3_V4~1.svar
//New: (3<<16) + (4<<8) + (1<<4) + 1
#define VAR_TYPE_DIC       0  //.dic
#define VAR_TYPE_SUB       1  //.svar
#define VAR_TYPE_VAR       2  //.var
#define VAR_TYPE_ENTRY     3  //.entry
#define VAR_TYPE_META      4  //.meta
#define VAR_TYPE_TMPLS     5  //.templates
#define VAR_TYPE_VARLIST   6  //.variables
#define VAR_TYPE_OUTLIER   7  //.outlier

#define MAIN_PAT_NAME      VAR_TYPE_TMPLS//"templates.txt"
#define SUBV_PAT_NAME      VAR_TYPE_VARLIST//"variables.txt"
#define OUTL_PAT_NAME      VAR_TYPE_OUTLIER//"templates.outlier"

#define QTYPE_ALIGN_FULL   0
#define QTYPE_ALIGN_LEFT   1
#define QTYPE_ALIGN_RIGHT  2
#define QTYPE_ALIGN_ANY    3

#define SEG_TYPE_CONST     0
#define SEG_TYPE_VAR       1
#define SEG_TYPE_DELIM     2
#define SEG_TYPE_SUBVAR    99

#define MATCH_MISS         0
#define MATCH_FULL         1
#define MATCH_PART         2
#define MATCH_ONPAT        9

#define setbit(x,y) x|=(1<<y) //set the y pos of x as 1
#define clrbit(x,y) x&=~(1<<y) //set the y pos of x as 0
#define getbit(x,y) (x>>y)&1  //get the y pos value of x

#define INC_TEST_JUDGELEN        1      //whether enble length filters
#define INC_TEST_JUDGETAG        1      //whether enble stamp filters (1~63)
#define INC_TEST_FIXED           1      //whether enble fixed length
#define INC_TEST_PUSHDOWN        1      //whether enble pruning
#define INC_TEST_INDEXMAP               //whether enble optimized bitmap
#define INC_TEST_SESSION         1      //whether enble session-level optimization
#define ENABLE_CACHE_REPLACE     0      //whether enble cache replacement strategy (FIFO)


typedef struct GlbMeta
{
  int StartPos;
  int LzmaSize;
  int OrigSize;
  int LineCount;
  int LineSize;

  GlbMeta()
  {
    StartPos = 0;
    LzmaSize = 0;
    OrigSize = 0;
    LineCount = 0;
    LineSize = 0;
  }
  int Set(int startPos, int lzmaSize, int origSize, int lineCount, int lineSize)
  {
    StartPos = startPos;
    LzmaSize = lzmaSize;
    OrigSize = origSize;
    LineCount = lineCount;
    LineSize = lineSize;
  }
}GlbMeta;

typedef struct LogPattern
{
	int Count;
	char* Content;
  int ContSize;
  int SegSize;
  char* Segment[MAX_CMD_PARAMS_COUNT];
  int SegAttr[MAX_CMD_PARAMS_COUNT];//0:const 2:delim >2^16:value  
  int VarNames[MAX_CMD_PARAMS_COUNT];//E1 :<V0>= E1_V0 
  LogPattern()
  {
    Count =0;
    Content = NULL;
    ContSize = 0;
    SegSize = 0;
  }

  int SetContent(char* content)
  {
    ContSize = strlen(content);
	  Content = new char[ContSize + 1]{'\0'};
	  strncpy(Content, content, ContSize);
    //enable segments
    SegSize = Split(content, TOKEN, Segment);
  }
}LogPattern;

typedef struct SubPattern
{
	int Type;//0:dictionary 1:subpattern 2:var
	char* Content;
  int ContSize;
  int VarNum;//the count of sub-attrs
  int SegSize;
  //add for .dic and .var
  short Tag;// see def upper
  int DicCnt;

  typedef struct SubVarInfo
  {
    char mark;
    int tag;
    int len;
    SubVarInfo(char m, int t, int l)
    {
      mark = m;
      tag = t;
      len = l;
    }
  }SubVarInfo;

  typedef struct DicVarInfo
  {
    char* pat;
    int patLen;
    int len;//padding length
    int lineCnt;//totol entries count
    int lineSno;
    int lineEno;
    int segSize;
    char* segCont[MAX_CMD_PARAMS_COUNT];
    int segTag[MAX_CMD_PARAMS_COUNT];//0:const 1~31
    DicVarInfo(int l, int lcnt, int preleno)
    {
      pat = NULL;
      patLen = 0;
      len = l;
      lineCnt = lcnt;
      lineSno = preleno + 1;
      lineEno = lineSno + lineCnt -1;
      segSize =0;
    }
    void SetContent(char* content, int clen)
    {
      patLen = clen;
	    pat = new char[patLen + 1];
	    strncpy(pat, content, patLen);
      pat[patLen] = '\0';

      int lPos =0;
      int rPos =0;
      int lastPos =0;
      int tag =0;
      for(int i=0;i<patLen;i++)
      {
        if(pat[i] == TOKEN_VAR_LEFT)
        {
          lPos =i;
        }
        if(pat[i] == TOKEN_VAR_RIGHT)
        {
          rPos =i;
        }
        if(lPos >= 0 && rPos > lPos)
        {
          if(lPos !=0 && lPos != lastPos)
          {
            segCont[segSize] = new char[lPos - lastPos +1]{'\0'};
            strncpy(segCont[segSize], pat + lastPos, lPos - lastPos);
            segTag[segSize] = SEG_TYPE_CONST;
            segSize++;
          }
          sscanf(pat + lPos, "<%d>", &tag);
          // printf("%d-%d-%d-%s-%d\n", lPos, rPos, lastPos, pat + lPos, tag);
          segTag[segSize] = INC_TEST_JUDGETAG == 1? tag : 63;
          segCont[segSize] = NULL;
          segSize++;
          lastPos = rPos + 1;
          lPos = -1;
        }
      }
      if(lastPos < patLen)
      {
        segCont[segSize] = new char[patLen - lastPos +1]{'\0'};
        strncpy(segCont[segSize], pat + lastPos, patLen - lastPos);
        segTag[segSize] = SEG_TYPE_CONST;
        segSize++;
      }
    }
  }DicVarInfo;

  char* SubSegment[MAX_CMD_PARAMS_COUNT];
  int SubSegAttr[MAX_CMD_PARAMS_COUNT];//0:const >2^16 (real value):value
  SubVarInfo* SubVars[MAX_CMD_PARAMS_COUNT];
  DicVarInfo* DicVars[MAX_CMD_PARAMS_COUNT];

  SubPattern()
  {
    Type = -1;
    Content = NULL;
    ContSize = 0;
    VarNum = 0;
    SegSize =0;
    Tag =0;
    DicCnt =0;
  }

  void SetContent(char* content)
  {
    ContSize = strlen(content);
	  Content = new char[ContSize + 1];
	  strncpy(Content, content, ContSize);
    Content[ContSize] = '\0';
  }
  void SetSegments(int vname, char* content)
  {//perhaps:  a<10b<*>_c<*>_d>20e
    int lPos =0;
    int rPos =0;
    int lastPos =0;
    for(int i=0;i<ContSize;i++)
    {
      if(content[i] == TOKEN_VAR_LEFT)
      {
        lPos =i;
      }
      if(content[i] == TOKEN_VAR_RIGHT)
      {
        rPos =i;
      }
      if(lPos >= 0 && rPos > lPos)
      {
        if(lPos !=0 && lPos != lastPos)
        {
          SubSegment[SegSize] = new char[lPos - lastPos +1]{'\0'};
          strncpy(SubSegment[SegSize], content + lastPos, lPos - lastPos);
          SubSegAttr[SegSize] = SEG_TYPE_CONST;
          SubVars[SegSize] = NULL;
          SegSize++;
        }
        SubSegment[SegSize] = NULL;
        //Segment[SegSize] = new char[MAX_PATNAME_SIZE]{'\0'};
        //sprintf(Segment[SegSize],"%s~%d.svar",vname, VarNum);
        SubSegAttr[SegSize] = vname | (VarNum<<4) | VAR_TYPE_SUB;
        char fixlen_mark;
	      int strTag_mark;
	      int maxlen_mark;
	      int scanRst = sscanf(content + lPos, "<%c,%d,%d>", &fixlen_mark, &strTag_mark, &maxlen_mark);
        if(scanRst == 3)
        {
          if(INC_TEST_JUDGETAG)
          {
            SubVars[SegSize] = new SubVarInfo(fixlen_mark, strTag_mark, maxlen_mark);
          }
          else
          {
            SubVars[SegSize] = new SubVarInfo(fixlen_mark, 63, maxlen_mark);
          }
        }
        else
        {
          ;
        }
        SegSize++;
        VarNum++;
        lastPos = rPos + 1;
        lPos = -1;
      }
    }
    if(lastPos < ContSize)
    {
      SubSegment[SegSize] = new char[ContSize - lastPos +1]{'\0'};
      strncpy(SubSegment[SegSize], content + lastPos, ContSize - lastPos);
      SubSegAttr[SegSize] = SEG_TYPE_CONST;
      SubVars[SegSize] = NULL;
      SegSize++;
    }
  }
}SubPattern;

typedef struct VarOutliers
{
  int Count;
  map<int, char*> Outliers;
  VarOutliers(int cnt)
  {
    Count = cnt;
  }
}VarOutliers;

typedef struct RegMatch
{
  int Vname;
  int Index;//the regmatch count
  int So;//match start pos
  int Eo;//match length
  RegMatch()
  {
    Vname = 0;
    Index = -1;
    So = 0;
    Eo = 0;
  }
}RegMatch;

typedef struct RegMatches
{
  RegMatch Match[MAX_SUBATTR_CNT];
  int Count;
  bool BeValid;
  RegMatches()
  {
    Count =0;
    BeValid = true;
  }
  void Add(int index, int so, int eo, int vname)
  {
    Match[Count].Index = index;
    Match[Count].So = so;
    Match[Count].Eo = eo;
    Match[Count].Vname = vname;
    Count++;
  }
}RegMatches;

typedef struct RegMatrix
{
  RegMatches Matches[MAX_SUBATTR_CLS];
  int Count;
  int ValidCnt;
  RegMatrix()
  {
    Count =0;
    ValidCnt =0;
  }
  void Addx(int index, int so, int eo, int vname)
  {
    Matches[Count-1].Add(index, so, eo, vname);
  }
  void Addy()
  {
    Count ++; 
    Matches[Count-1].Count =0;
  }
  void Suby()
  {
    Count --; 
  }
  void Reset()
  {
    Count =0;
  }
}RegMatrix;

#ifdef INC_TEST_INDEXMAP
//record bitmap and matched positions
class BitMap
{
  public:
    bitset<MAX_FILE_LEN> Bitmap;
    int* Index;
    int Size; //DEF_BITMAP_FULL: all
  public:
    int TotalSize;
    
  //int so[MAX_FILE_LEN];
  //int eo[MAX_FILE_LEN];
  public:
    BitMap(int tolsize)
    {
      TotalSize = tolsize;
      Index = new int[tolsize];
      Reset();
    }
    ~BitMap()
    {
      if(Index) 
      {
        delete Index;
        Index = NULL;
      }
    }
    void Reset()
    {
      Bitmap.reset();
      Size =0;
    }
  
    void ResetSize()
    {
      Size =0;
    }

    void SetSize()
    {
      Size = DEF_BITMAP_FULL;//means all 1
    }

    //set pos as 0
    void Reset(int pos)
    {
      Bitmap.reset(pos);
    }
    //intersection
    void Inset(int pos)
    {
      Index[Size++] = pos;
    }
    //union
    void Union(int pos)
    {
      if(Bitmap[pos] == 0)
      {
        Bitmap.set(pos);
        Index[Size++] = pos;
      }
    }
    void Inset(BitMap* target)
    {
      if(target->Size == DEF_BITMAP_FULL)
      {
        return;
      }
      if(target->Size == 0)
      {
        Reset();
        return;
      }
      int tempSize = Size;
      ResetSize();
      for(int i=0; i< tempSize; i++)
      {
        if(target->Bitmap[Index[i]] == 0)
        {
          Reset(Index[i]);
        }
        else
        {
          Inset(Index[i]);
        }
      }
    }
    void Complement(BitMap* target)
    {
      if(target->Size == DEF_BITMAP_FULL || target->Size == TotalSize)
      {
        Reset();
        return;
      }
      if(target->Size == 0)
      {
        return;
      }
      int tempSize = Size;
      ResetSize();
      for(int i=0; i< tempSize; i++)
      {
        if(target->Bitmap[Index[i]] == 0)
        {
          Inset(Index[i]);
        }
        else
        {
          Reset(Index[i]);
        }
      }
    }
    void Reverse()
    {
      if(Size == DEF_BITMAP_FULL)
      {
        Reset();
        return;
      }
      if(Size == 0)
      {
        SetSize();
        return;
      }
      ResetSize();
      for(int i=0; i< TotalSize; i++)
      {
        if(Bitmap[i] == 0)
        {
          Bitmap.set(i);
          Inset(i);
        }
        else
        {
          Reset(i);
        }
      }
    }
    void Union(BitMap* target)
    {
      if(target == NULL) return;
      if(target->Size == DEF_BITMAP_FULL)
      {
        Size = DEF_BITMAP_FULL;//universal set
      }
      else
      {
        for(int i=0; i< target->Size; i++)
        {
          Union(target->Index[i]);
        }
      }
    }

    int GetValue(int pos)
    {
      return Bitmap[pos];
    }

    int GetSize()
    {
      return Size;
    }

    int GetIndex(int index)
    {
      return Index[index];
    }

    int BeSizeFul()
    {
      return (Size == DEF_BITMAP_FULL || Size == TotalSize);
    }

    int CloneFrom(BitMap* target)
    {
      TotalSize = target->TotalSize;
      Size = target->Size;
      for(int i=0;i< Size; i++)
      {
        Index[i] = target->Index[i];
      }
      Bitmap |= target->Bitmap;
    }
};
#endif

typedef struct RunningStatus
{
  double LogMetaTime;
  double LoadDeComLogTime;
  double SearchTotalTime;
  double SearchPatternTime;
  double SearchOutlierTime;
  double MaterializFulTime;
  double MaterializAlgTime;
  int SearchTotalEntriesNum;
  int SearchOutliersNum;
  RunningStatus()
  {
    LogMetaTime = 0;
    LoadDeComLogTime =0;
    SearchTotalTime =0;
    SearchPatternTime = 0;
    SearchOutlierTime = 0;
    MaterializFulTime = 0;
    MaterializAlgTime = 0;
    SearchTotalEntriesNum =0;
    SearchOutliersNum =0;
  }
}RunningStatus;

typedef struct Statistics
{
  //columns
	int total_capsule_cnt; 
	int total_decom_capsule_cnt; //(query+materialization)
  //total_filtered_cap_cnt + valid_cap_filter_cnt <= total_queried_cap_cnt
	int total_queried_cap_cnt;
	int total_filtered_cap_cnt;//= length_filtered_cap_cnt + tag_filtered_cap_cnt
	int length_filtered_cap_cnt;
	int tag_filtered_cap_cnt;
	int valid_cap_filter_cnt;//The examined capsules indeed contain valid data 
  int tag_cap_mix_cnt;//Number of mixed data types included in the tag 

  //rows，bitmap pruning+ Horizontal division +local metadata design

	Statistics()
	{
		total_capsule_cnt =0;
		total_decom_capsule_cnt =0;

		total_queried_cap_cnt=0;
		total_filtered_cap_cnt=0;
		length_filtered_cap_cnt=0;
		tag_filtered_cap_cnt=0;
    valid_cap_filter_cnt=0;
    tag_cap_mix_cnt=0;
	}
}Statistics;


typedef struct LcsMatch
{
  int Len;//matched length
  int Pos1;//the pos of the first seq
  int Pos2;//the pos of the second seq
  LcsMatch()
  {
    Len = 0;
    Pos1 = 0;
    Pos2 = 0;
  }
  void Get(int len, int pos1, int pos2)
  {
    Len = len;
    Pos1 = pos1;
    Pos2 = pos2;
  }
}LcsMatch;

//order by value
struct CmpLogPatternByValue 
{
  bool operator()(const pair<string, LogPattern>& lhs, const pair<string, LogPattern>& rhs) {  
    return lhs.second.Count > rhs.second.Count;  
  }  
};

struct ptrCmp
{
    bool operator() (const char* s1, const char* s2) const
    {
        return strcmp(s1, s2) < 0;
    }
};




extern timeval ___StatTime_Start();
extern double  ___StatTime_End(timeval t1);
#endif
