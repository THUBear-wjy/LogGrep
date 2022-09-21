#include <fstream>
#include <sys/types.h>
#include <dirent.h>
#include <algorithm>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include "LogStore_API.h"

using namespace std;

double materTime = 0;
////////////////////////////////////////////init & private//////////////////////////////////////////////////////////
LogStoreApi::LogStoreApi()
{
	m_nServerHandle =0;
	m_fd = -1;
	m_fptr = NULL;
	m_glbMetaHeadLen =0;
	m_maxBitmapSize =0;
	m_outliers = NULL;
}
LogStoreApi::~LogStoreApi()
{
	if(m_nServerHandle != 0)
	{
		DisConnect();
	}
	delete m_glbExchgLogicmap;
	delete m_glbExchgPatmap;
	delete m_glbExchgBitmap;
	delete m_glbExchgSubBitmap;
	delete m_glbExchgSubTempBitmap;
}

//return success(>0) or failed(<0) flag  
int LogStoreApi::BootLoader(char* path, char* filename)
{
	unsigned char prop[5];
	size_t destLen, srcLen;
	char cFileTmp[MAX_FILE_NAMELEN] = {'\0'};
	sprintf(cFileTmp, "%s/%s", path, filename);
	//load meta header: 5+4+4
	int ret = LoadGlbMetaHeader(cFileTmp, destLen, srcLen, prop);
	if(ret <= 0) return -1;
	//load meta data
	ret = LoadGlbMetadata(cFileTmp, destLen, srcLen, prop);
	if(ret <= 0) return -2; //printf("%d\n",m_glbMeta.size());
	//load templates(pattern)
	Coffer* coffer = NULL;
	ret = DeCompressPatterns(MAIN_PAT_NAME, coffer);
	if(ret <= 0) return -3;
	ret = LoadMainPatternToGlbMap(coffer ->data, coffer->srcLen);
	ClearCoffer(coffer);
	if(ret <= 0) return -4;
	//load variables(subPattern)
	ret = DeCompressPatterns(SUBV_PAT_NAME, coffer);
	if(ret <= 0) return -5;
	//SyslogDebug("subpat: %s\n", coffer ->data);
	ret = LoadSubPatternToGlbMap(coffer ->data, coffer->srcLen);
	ClearCoffer(coffer);
	if(ret <= 0) return -6;
	//load templates.outlier
	ret = DeCompressPatterns(OUTL_PAT_NAME, coffer);
	if(ret <= 0) return -7;
	if(coffer->lines > 0)
	{
		ret = LoadOutliers(coffer ->data, coffer->lines);
		ClearCoffer(coffer);
		if(ret <= 0) return -8;
	}
	// init global bitmap for caching
	m_glbExchgLogicmap = new BitMap(m_maxBitmapSize);
	m_glbExchgPatmap = new BitMap(m_maxBitmapSize);
	m_glbExchgBitmap = new BitMap(m_maxBitmapSize);
	m_glbExchgSubBitmap = new BitMap(m_maxBitmapSize);
	m_glbExchgSubTempBitmap = new BitMap(m_maxBitmapSize);
	return ret;
}

int LogStoreApi::LoadGlbMetaHeader(char* filename, size_t& destLen, size_t& srcLen, unsigned char prop[5])
{
	m_fptr = fopen(filename, "rb");
	if(m_fptr == NULL) return -1;
	//m_lzmaMethod + prop[4] + destLen + srcLen;
	fread(prop, sizeof(char), 5, m_fptr);
    fread(&destLen, sizeof(size_t), 1, m_fptr);
    fread(&srcLen, sizeof(size_t), 1, m_fptr);
	if(destLen <= 0 || srcLen <=0) return -2;
	m_glbMetaHeadLen = 5 + sizeof(size_t) + sizeof(size_t) + destLen;
	m_lzmaMethod = prop[0];
	return 1;
}

//return entries of meta file
int LogStoreApi::LoadGlbMetadata(char* filename, size_t destLen, size_t srcLen, unsigned char prop[5])
{
	//decompression
	unsigned char* pLzma = new unsigned char[destLen + 5];
	char* meta = new char[srcLen + 5]{'\0'};
    fread(pLzma, sizeof(char), destLen, m_fptr);
	int res = LzmaUncompress((unsigned char*)meta, &srcLen, pLzma, &destLen, prop, 5);
	if(res != SZ_OK) return -1;
	int offset =0, index =0;
	char* meta_buffer = new char[128];
	for (char *p= meta; *p && p-meta< srcLen + 5; p++)
	{
        if (*p == '\n')
		{
			meta_buffer[offset] = '\0';
			Coffer* newCoffer = new Coffer(string(meta_buffer));
			int iname = atoi(newCoffer->filenames.c_str());
			m_glbMeta[iname] = newCoffer;
			if(newCoffer->eleLen == -3 && newCoffer ->lines > 0)//var outliers
			{
				LoadVarOutliers(iname, newCoffer ->lines, newCoffer ->srcLen);
			}
			
			//for stat
			if(newCoffer ->lines > 0)
			{
				Statistic.total_capsule_cnt ++;
			}
			

			offset=0;
			index++;

			if(iname <=0) 
			{
				SyslogError("ErrorMeta:%d filename:%s %s\n", iname, FileName.c_str(), meta_buffer);
			}
			SyslogDebug("coffer filenames: %d (%s), srcLen:%d lines: %d eleLen:%d\n",iname, FormatVarName(iname), newCoffer ->srcLen, newCoffer ->lines, newCoffer ->eleLen);
		}
        else
		{
			meta_buffer[offset++] = *p;
		}
	}
	return index;
}

//E18_V2.outlier: srcLen:87573 lines: 5253 eleLen:-1
int LogStoreApi::LoadVarOutliers(int filename, int lines, int srcLen)
{
	if(lines == 0)
	{
		m_varouts[filename] = NULL;
	}
	else
	{
		Coffer* coffer = NULL;
		int ret = DeCompressPatterns(filename, coffer);
		if(ret <= 0) return -1;
		VarOutliers* outs = new VarOutliers(lines);
		char* buf = coffer->data;
		int slim =0;
		int patStartPos = 0;
		int patEndPos = 0;
		int index=0;
		for (int i=0; i< srcLen; i++)
		{
			if (buf[i] == '\n')//a new line
			{
				patEndPos = i-1;
				int outLen = patEndPos - patStartPos + 1;
				outs->Outliers[index] = new char[outLen + 1];
				memcpy(outs->Outliers[index], buf + patStartPos, outLen);
				outs->Outliers[index][outLen] = '\0';
				//SyslogDebug("%s %d %s\n", filename.c_str(), index, outs->Outliers[index]);
				index = 0;
				slim = 0;
			}
			else if(buf[i] == ' ')
			{
				slim++;
				if(slim == 1) patStartPos = i+1;
			}
			else
			{
				if(slim == 0)
				{
					if((buf[i] - '0') >= 0 && (buf[i] - '0') <= 9)
					{
						index = index * 10 + (buf[i] - '0');
					}
					else
					{
						SyslogError("error read outlier file: %d\n", filename);
					}
				}
			}
		}
		m_varouts[filename] = outs;
	}
}

int LogStoreApi::LoadOutliers(IN char *buf, int lineCount)
{
	m_outliers = new CELL[lineCount];
	int index =0;
	int offset =0;
	int len = strlen(buf);
	for (char *p= buf;*p && p-buf< len; p++)
	{
		if (*p == '\n')//a new line
		{
			m_outliers[index]= new char[offset];
			memcpy(m_outliers[index], p-offset, offset);
			//offset can reach to 18960, can you believe it!
			//SyslogDebug("offset:%d\n", offset);
			index ++;
			offset =0;
		}
		else
		{
			offset++;
		}
	}
	//get the max count
	if(m_maxBitmapSize < lineCount)
		m_maxBitmapSize = lineCount;
	return index;
}

int LogStoreApi::LoadMainPatternToGlbMap(IN char *buf, IN int srcLen)
{
	char etag = ' '; // should be 'E', otherwise is wrong
	int eid =0;//convert eid to int
	int cnt =0;
	char content[MAX_LINE_SIZE]={'\0'};
	int index=0;
	int slim=0;
	for (char *p= buf;*p && p-buf< srcLen; p++)
	{
		if (*p == '\n')//a new line
		{
			content[index] = '\0';
			int rt = AddMainPatternToMap(etag, eid, cnt, content);
			//if(rt < 0)  SyslogError("srcLen:%d buflen: %d\n ", srcLen, strlen(buf));	
			slim =0;
			index =0;
			cnt =0;
			content[0] = '\0';
		}
        else if(*p == ' ')// a new slim
		{
			if(slim == 2)
			{
				content[index++] = *p;
			}
			else if(slim == 1)
			{
				slim++;
			}
			else if(slim == 0)
			{
				index =0;
				slim++;
			}
		}
		else
		{
			if(slim == 2)//content
			{
				content[index++] = *p;
			}
			if(slim == 1)//count
			{
				cnt = cnt * 10 + (*p - '0');
			}
			else if(slim == 0)//E8
			{
				if(index == 0)
				{
					etag = *p;
					eid =0;
					index++;
				}
				else
				{
					if((*p - '0') >= 0 && (*p - '0') <= 9)
					{
						eid = eid * 10 + (*p - '0');
					}
					else
					{
						SyslogError("error read eid: %d\n", eid);
					}
				}
			}
		}
	}

	return m_patterns.size();
}

int LogStoreApi::LoadSubPatternToGlbMap(IN char *buf, IN int srcLen)
{
	char content[3][MAX_VALUE_LEN]={'\0'};
	int index =0;
	int slim =0;
	for (char *p= buf;*p && p-buf< srcLen; p++)
	{
		if (*p == '\n')//a new line
		{
			content[slim][index] = '\0';
			AddSubPatternToMap(atoi(content[0]), content[1][0], content[2]);
			slim =0;
			index =0;
			content[0][0] = '\0';
			content[1][0] = '\0';
			content[2][0] = '\0';
		}
        else if(*p == ' ' && slim < 2)// a new slim
		{
			content[slim][index] = '\0';
			index =0;
			slim++;
		}
		else
		{
			content[slim][index++] = *p;
		}
	}
	return m_subpatterns.size();
}

//add to map, or can use insert-ordered method if neccessary
int LogStoreApi::AddMainPatternToMap(char etag, int eid, int count, char* content)
{
	if(content == NULL || strlen(content) == 0) return -1;
	SyslogDebug("-----------%d: %d %s\n",eid, count, content);
	//if(etag != 'E' || eid <=0)
	if(eid <=0)
	{
		//SyslogError("error eid: %c%d %s : %d %s\n", etag, eid, FileName.c_str(), count, content);
		return -1;
	}
	eid = (eid <<16);
	LogPattern* logP = new LogPattern();
	logP->Count = count;
	logP->SetContent(content);
	int segValue = 0;
	for(int i=0; i< logP->SegSize;i++)
	{	
		logP->SegAttr[i] = GetSegmentType(logP->Segment[i], segValue);
		//SyslogDebug("%s ",logP->Segment[i], logP->SegAttr[i]);
		if(logP->SegAttr[i] == SEG_TYPE_VAR)//store var names
		{
			logP->VarNames[i] = eid | (segValue<<8);
		}
		else
		{
			logP->VarNames[i] = NULL;
		}
	}
	//get the max count
	if(m_maxBitmapSize < count)
		m_maxBitmapSize = count;
	//SyslogDebug("\n");
	m_patterns[eid] =logP;
	return 0;
}

//add to map
int LogStoreApi::AddSubPatternToMap(int vid, char type, char* content)
{
	SubPattern* logP = new SubPattern();
	if(type =='S')
	{
		logP->Type = VAR_TYPE_SUB;
		logP->SetContent(content);
		logP->SetSegments(vid, content);
		SyslogDebug("%d (%s) %c %s \n", vid, FormatVarName(vid), type, content);
		// for(int i=0;i< logP->SegSize; i++)
		// {
		// 	if(logP->SegAttr[i] == SEG_TYPE_CONST)
		// 		printf("--const--%s\n", logP->Segment[i]);
		// 	else
		// 	{
		// 		printf("--%s--%c %d %d\n", logP->Segment[i], logP->SubVars[i]->mark, logP->SubVars[i]->tag, logP->SubVars[i]->len);
		// 	}
		// }
	}
	else if(type =='D')
	{
		logP->Type = VAR_TYPE_DIC;
		int slim =0;
		int patStartPos = 0;
		int patEndPos =0;
		int contLen = strlen(content);
		int len =0;//padding length
    	int lineCnt =0;//totol entries count
		int index=0;
		//get DicCnt
		int dicCnt =0;
		for (int i=0; i< contLen; i++)
		{
			if(content[i] == ' ')// DicCnt
			{
				logP->DicCnt = dicCnt;
				content += i+1; 
				break;
			}
			else
			{
				dicCnt= dicCnt * 10 + (content[i] - '0');
			}
		}
		for (int i=0; i< contLen; i++)
		{
        	if(content[i] == ' ' || content[i] == '\n')// a new slim
			{
				slim++;
				if(slim == 1)
				{
					patEndPos = i-1;
				}
				else if(slim == 3)
				{
					if(index == 0)
					{
						logP->DicVars[index] = new SubPattern::DicVarInfo(len, lineCnt, -1);
					}
					else
					{
						logP->DicVars[index] = new SubPattern::DicVarInfo(len, lineCnt, logP->DicVars[index-1]->lineEno);
					}
					logP->DicVars[index]->SetContent(content + patStartPos, patEndPos - patStartPos +1);
					slim =0;
					len =0;
					lineCnt =0;
					patStartPos = i+1;
					index++;
				}
			}
			else
			{
				if(slim == 1)
				{
					len = len * 10 + (content[i] - '0');
				}
				else if(slim ==2)
				{
					lineCnt = lineCnt * 10 + (content[i] - '0');
				}
				
			}
		}
		SyslogDebug("%d (%s) %c %s\n", vid, FormatVarName(vid), type, content);
		// for(int i=0; i<index; i++)
		// {
		// 	printf("---%d--- lineLen:%d lineSno:%d lineCnt:%d %s \n", index, logP->DicVars[i] ->len, logP->DicVars[i] ->lineSno, logP->DicVars[i] ->lineCnt, logP->DicVars[i] ->pat);
		// 	for(int j=0;j< logP->DicVars[i] ->segSize;j++)
		// 	{
		// 		if(logP->DicVars[i] ->segTag[j] == SEG_TYPE_CONST)
		// 			printf("------------- %s \n", logP->DicVars[i] ->segCont[j]);
		// 		else
		// 			printf("------------- %d \n", logP->DicVars[i] ->segTag[j]);
		// 	}
		// }
	}
	else if(type =='V')
	{
		logP->Type = VAR_TYPE_VAR;
		int slim =0;
		int contLen = strlen(content);
		int tag=0;
		int len=0;
		for (int i=0; i< contLen; i++)
		{
        	if(content[i] == ' ')// a new slim
			{
				slim++;
				if(slim == 1)
				{
					logP->Tag = tag;
				}
			}
			else
			{
				if(slim == 0)
				{
					tag = tag * 10 + (content[i] - '0');
				}
				else if(slim == 1)
				{
					len = len * 10 + (content[i] - '0');
				}
				
			}
		}
		logP->ContSize = len;
		SyslogDebug("%d(%s) %c %d %d\n", vid, FormatVarName(vid), type, logP->Tag, logP->ContSize);
	}
	m_subpatterns[vid] =logP;
	return 0;
}

//decompress patterns
int LogStoreApi::DeCompressPatterns(int patName, OUT Coffer* &coffer)
{
	coffer = m_glbMeta[patName];
	if(coffer == NULL || m_fptr == NULL || coffer->destLen <0 || coffer->srcLen <0) return -1; //error patName
	if(coffer->data == NULL)//data not be cached, need decompress from file
	{
		int res = coffer->readFile(m_fptr, m_glbMetaHeadLen);
		if(res < 0) return -2;
		res = coffer -> decompress(m_lzmaMethod);
		if(res < 0) return -3;
	}

	//for stat
	Statistic.total_decom_capsule_cnt++;
	return 1;
}

//load file segment to mem using mmap, abandoned
int LogStoreApi::LoadFileToMem(const char *varname, int startPos, int bufLen, OUT char *mbuf)
{
	if(m_fd == -1)
	{
		m_fd = open(varname,O_RDONLY);
	}
	if(m_fd != -1)
	{
		lseek(m_fd,startPos,SEEK_SET);
		mbuf = (char *)mmap(NULL,bufLen,PROT_READ,MAP_PRIVATE,m_fd,0);
	}
	return m_fd;
}

//load file segment to mem using mmap, abandoned
unsigned char* LogStoreApi::LoadFileToMem(const char *varname, int startPos, int bufLen)
{
	if(m_fd == -1)
	{
		m_fd = open(varname, O_RDONLY);
	}
	if(m_fd != -1)
	{
		int len = lseek(m_fd,0,SEEK_END);
		if(len > 0) 
		{
			return (unsigned char *)mmap(NULL,len,PROT_READ,MAP_PRIVATE,m_fd,0);
		}
	}
	return NULL;
}

//return 1: success, <0: open file failed 
int LogStoreApi::LoadBuffToMemWithDecomp(int varfname, OUT Coffer* &coffer)
{
	//timeval tt1 = ___StatTime_Start();
	
	int ret =1;
	coffer = m_glbMeta[varfname];
	if(coffer == NULL) 
	{
		SyslogError("Error: not find varname:%s in meta:%s!\n",FormatVarName(varfname), FileName.c_str());
		return -1;
	}
	//if find in cache, then fetch directly
	if(coffer->data == NULL)
	{
		//SyslogDebug("DeCompressPatterns: %s\n", varfname.c_str());
		ret = DeCompressPatterns(varfname, coffer);
	}

	//RunStatus.LoadDeComLogTime += ___StatTime_End(tt1);
	return ret;
}

int LogStoreApi::QueryInMmapByKMP(int varname, const char* queryStr, BitMap* bitmap)
{
	Coffer* meta;
	int len = LoadBuffToMemWithDecomp(varname, meta);
	if(len <=0)
	{
		return 0;
	}
	return KMP(meta->data, queryStr, bitmap, QTYPE_ALIGN_ANY);
}

//load vals by Boyer-Moore,  loading with calculating, may accelerate query
int LogStoreApi::QueryByBM_Union(int varname, const char* queryStr, int queryType, BitMap* bitmap)
{
	Coffer* meta;
	int len = LoadBuffToMemWithDecomp(varname, meta);
	if(len <=0)
	{
		return 0;
	}
	//SyslogDebug("%s: meta: Len:%d line:%d ele: %d\n", FormatVarName(varname), meta->srcLen, meta->lines, meta->eleLen);
	//SyslogDebug("------------%s\n", meta->data);
	int queryLen = strlen(queryStr);
	int sLen = strlen(meta->data);
	if(INC_TEST_FIXED && meta->eleLen > 0)//same length of each line
	{
		if(queryLen == meta->eleLen)
		{
			return BM_Fixed_Align(meta->data, 0, sLen, queryStr, bitmap, meta->eleLen);
		}
		else if(queryType == QTYPE_ALIGN_ANY)
		{
			return BM_Fixed_Anypos(meta->data, 0, sLen, queryStr, bitmap, meta->eleLen);
		}
		else
		{
			return BM_Fixed_Align(meta->data, 0, sLen, queryStr, bitmap, meta->eleLen, queryType);
		}
	}
	else
	{
		SyslogDebug("-----------------using kmp\n");
		int ret = KMP(meta ->data, queryStr, bitmap, queryType);
		return ret;
	}
}

int LogStoreApi::QueryByBM_Union_RefRange(int varname, const char* queryStr, int queryType, BitMap* bitmap, RegMatrix* refRange)
{
	Coffer* meta;
	int varfname = varname +  VAR_TYPE_DIC;
	int len = LoadBuffToMemWithDecomp(varfname, meta);
	if(len <=0)
	{
		return 0;
	}
	int queryLen = strlen(queryStr);
	int offset=0;
	for(int i= 0; i< refRange->Count; i++)
	{
		int sLen = refRange->Matches[i].Match[0].Eo * refRange->Matches[i].Match[0].Index;
		int dicLen;
		int offset = GetDicOffsetByEntry(m_subpatterns[varname], refRange->Matches[i].Match[0].So, dicLen);
		if(queryLen == refRange->Matches[i].Match[0].Index)
		{
			BM_Fixed_Align(meta->data + offset, refRange->Matches[i].Match[0].So, sLen, queryStr, bitmap, refRange->Matches[i].Match[0].Index);
		}
		else if(queryType == QTYPE_ALIGN_ANY)
		{
			BM_Fixed_Anypos(meta->data + offset, refRange->Matches[i].Match[0].So, sLen, queryStr, bitmap, refRange->Matches[i].Match[0].Index);
		}
		else
		{
			BM_Fixed_Align(meta->data + offset, refRange->Matches[i].Match[0].So, sLen, queryStr, bitmap, refRange->Matches[i].Match[0].Index, queryType);
		}
	}
	return bitmap->GetSize();
}

int LogStoreApi::QueryByBM_AxB_Union(int varname, const char* queryStrA, const char* queryStrB, BitMap* bitmap)
{
	Coffer* meta;
	int len = LoadBuffToMemWithDecomp(varname, meta);
	if(len <=0)
	{
		return 0;
	}
	//SyslogDebug("%s: meta: Len:%d line:%d ele: %d\n", FormatVarName(varname), meta->srcLen, meta->lines, meta->eleLen);
	//SyslogDebug("------------%s\n", meta->data);
	int aLen = strlen(queryStrA);
	int bLen = strlen(queryStrB);
	if(meta->eleLen >= (aLen + bLen))//same length of each line
	{
		return BMwildcard_AxB(meta->data, meta->lines, meta->eleLen, queryStrA, queryStrB, bitmap);
	}
	else
	{
		return 0;
	}
}

int LogStoreApi::QueryByBM_Union_ForDic(int varname, const char* querySegs, int querySegCnt, BitMap* bitmap)
{
	Coffer* meta;
	int len = LoadBuffToMemWithDecomp(varname, meta);
	if(len <=0)
	{
		return 0;
	}
	int ret =0;
	if(INC_TEST_FIXED && meta->eleLen > 0)//same length of each line
	{
		if(querySegCnt == 1)
		{
			ret = BM_Fixed_Align(meta->data, 0, strlen(meta->data), querySegs, bitmap, meta->eleLen);
		}
		else
		{
			ret = BM_Fixed_MutiFul(meta->data, querySegs, querySegCnt, bitmap, meta->eleLen);
		}
	}
	else
	{
		SyslogDebug("-----------------using kmp3 %d\n", querySegCnt);
		if(querySegCnt == 1)
		{
			ret = KMP(meta ->data, querySegs, bitmap, QTYPE_ALIGN_FULL);
		}
		else
		{
			for(int i=0;i<querySegCnt;i++)
			{
				ret = KMP(meta ->data, querySegs + i*MAX_DICENTY_LEN, bitmap, QTYPE_ALIGN_FULL);
			}
		}
	}
	return ret;
}

//load vals by Boyer-Moore,  loading with calculating, may accelerate query
//type: 0: full len matched   1: alignleft  2 alignright  3 anypos
int LogStoreApi::QueryByBM_Pushdown(int varname, const char* queryStr, BitMap* bitmap, int type)
{
	Coffer* meta;
	int len = LoadBuffToMemWithDecomp(varname, meta);
	if(len <=0)
	{
		bitmap->Reset();
		return bitmap->GetSize();
	}
	//SyslogDebug("%s: meta: Len:%d line:%d ele: %d\n", FormatVarName(varname), meta->srcLen, meta->lines, meta->eleLen);
	
	if(INC_TEST_FIXED && meta->eleLen > 0)//same length of each line
	{
		if(INC_TEST_PUSHDOWN)
		{
			return BM_Fixed_Pushdown(meta->data, queryStr, bitmap, meta->eleLen, type);
		}
	}
	else
	{
		SyslogDebug("-----------------using kmp4\n");
		if(INC_TEST_PUSHDOWN)
		{
			return BM_Diff_Pushdown(meta ->data, queryStr, bitmap, type);
		}
	}
	return bitmap->GetSize();
}

int LogStoreApi::QueryByBM_Pushdown_RefMap(int varname, const char* queryStr, BitMap* bitmap, BitMap* refBitmap, int type)
{
	Coffer* meta;
	int len = LoadBuffToMemWithDecomp(varname, meta);
	if(len <=0)
	{
		return 0;
	}
	SyslogDebug("%s: meta: Len:%d line:%d ele: %d\n", FormatVarName(varname), meta->srcLen, meta->lines, meta->eleLen);
	//SyslogDebug("------------%s\n", meta->data);
	if(INC_TEST_FIXED && meta->eleLen > 0)//same length of each line
	{
		if(INC_TEST_PUSHDOWN)
		{
			int rst = BM_Fixed_Pushdown_RefMap(meta->data, queryStr, bitmap, refBitmap, meta->eleLen, type);
			SyslogDebug("QueryByBM_Pushdown_RefMap %d\n", rst);
			return rst;
		}
	}
	else
	{
		SyslogDebug("-----------------using kmp5\n");
		if(INC_TEST_PUSHDOWN)
		{
			int rst = BM_Diff_Pushdown_RefMap(meta ->data, queryStr, bitmap, refBitmap, type);
			return rst;
		}
	}
	return bitmap->GetSize();
}

int LogStoreApi::QueryByBM_Pushdown_ForDic(int varname, const char* querySegs, int querySegCnt, BitMap* bitmap)
{
	Coffer* meta;
	int len = LoadBuffToMemWithDecomp(varname, meta);
	if(len <=0)
	{
		return 0;
	}
	int ret =0;
	//SyslogDebug("%s: meta: Len:%d line:%d ele: %d\n", FormatVarName(varname), meta->srcLen, meta->lines, meta->eleLen);
	//SyslogDebug("------------%s %d\n", meta->data, bitmap->GetSize());
	if(INC_TEST_FIXED && meta->eleLen > 0)//same length of each line
	{
		if(querySegCnt == 1)
		{
			if(INC_TEST_PUSHDOWN)
			{
				ret = BM_Fixed_Pushdown(meta->data, querySegs, bitmap, meta->eleLen, 0);
			}
		}
		else
		{
			if(INC_TEST_PUSHDOWN)
			{
				ret = BM_Fixed_Pushdown_MutiFul(meta->data, querySegs, querySegCnt, bitmap, meta->eleLen);
			}
		}
	}
	else
	{
		SyslogDebug("--------cnt: %d----%s type: %d--bitmap: %d---using kmp6\n", querySegCnt, querySegs, 0, bitmap->GetSize());
		if(querySegCnt == 1)
		{
			if(INC_TEST_PUSHDOWN)
			{
				ret = BM_Diff_Pushdown(meta->data, querySegs, bitmap, -1);
			}
		}
		else
		{
			if(INC_TEST_PUSHDOWN)
			{
				m_glbExchgSubTempBitmap->Reset();
				for(int i=0;i<querySegCnt-1;i++)
				{
					BM_Diff_Pushdown_RefMap(meta ->data, querySegs + i*MAX_DICENTY_LEN, m_glbExchgSubTempBitmap, bitmap, QTYPE_ALIGN_ANY);
				}
				BM_Diff_Pushdown(meta->data, querySegs + (querySegCnt-1) *MAX_DICENTY_LEN, bitmap, QTYPE_ALIGN_ANY);
				bitmap->Union(m_glbExchgSubTempBitmap);
			}
		}
	}
	return bitmap->GetSize();
}

int LogStoreApi::QueryByBM_Pushdown_ForDic_RefMap(int varname, const char* querySegs, int querySegCnt, BitMap* bitmap, BitMap* refBitmap)
{
	Coffer* meta;
	int len = LoadBuffToMemWithDecomp(varname, meta);
	if(len <=0)
	{
		return 0;
	}
	int ret =0;
	//SyslogDebug("%s: meta: Len:%d line:%d ele: %d\n", FormatVarName(varname), meta->srcLen, meta->lines, meta->eleLen);
	//SyslogDebug("------------%s %d\n", meta->data, bitmap->GetSize());
	if(INC_TEST_FIXED && meta->eleLen > 0)//same length of each line
	{
		if(querySegCnt == 1)
		{
			if(INC_TEST_PUSHDOWN)
			{
				ret = BM_Fixed_Pushdown_RefMap(meta->data, querySegs, bitmap, refBitmap, meta->eleLen, 0);
			}
		}
		else
		{
			if(INC_TEST_PUSHDOWN)
			{

				ret = BM_Fixed_Pushdown_MutiFul_RefMap(meta->data, querySegs, querySegCnt, bitmap, refBitmap, meta->eleLen);
			}
		}
	}
	else
	{
		SyslogDebug("------------cnt: %d  %s-----using ref kmp6\n", querySegCnt, querySegs);
		if(querySegCnt == 1)
		{
			if(INC_TEST_PUSHDOWN)
			{
				ret = BM_Diff_Pushdown_RefMap(meta->data, querySegs, bitmap, refBitmap, -1);
			}
		}
		else
		{
			if(INC_TEST_PUSHDOWN)
			{
				m_glbExchgSubTempBitmap->Reset();
				for(int i=0;i<querySegCnt-1;i++)
				{
					BM_Diff_Pushdown_RefMap(meta ->data, querySegs + i*MAX_DICENTY_LEN, m_glbExchgSubTempBitmap, refBitmap, QTYPE_ALIGN_ANY);
				}
				BM_Diff_Pushdown_RefMap(meta->data, querySegs + (querySegCnt-1) *MAX_DICENTY_LEN, bitmap, refBitmap, QTYPE_ALIGN_ANY);
				bitmap->Union(m_glbExchgSubTempBitmap);
			}
		}
	}
	return bitmap->GetSize();
}

//load vals by bitmap, achieve efficient skip
//return  0: false    1:true
int LogStoreApi::LoadcVarsByBitmap(int varname, BitMap* bitmap, OUT char *vars, int entryCnt, int varsLineLen, int flag)
{
	Coffer* meta;
	int ret = LoadBuffToMemWithDecomp(varname, meta);
	if(ret <=0) return 0;
	if(INC_TEST_FIXED && meta->eleLen > 0)
	{
		ret = GetCvarsByBitmap_Fixed(meta->data, meta->eleLen, bitmap, vars, entryCnt, varsLineLen, flag);
	}
	else
	{
		ret = GetCvarsByBitmap_Diff(meta->data, 0, bitmap, vars, entryCnt, varsLineLen, flag);
	}
	return ret;
}

//return  0: false    1:true
int LogStoreApi::LoadcVars(int varname, int entryCnt, OUT char *vars, int varsLineLen, int flag)
{
	Coffer* meta;
	int ret = LoadBuffToMemWithDecomp(varname, meta);
	if(ret <=0) return 0;
	if(INC_TEST_FIXED && meta->eleLen > 0)
	{
		ret = GetCvars_Fixed(meta->data, meta->eleLen, entryCnt, vars, varsLineLen, flag);
	}
	else
	{
		ret = GetCvars_Diff(meta->data, vars, entryCnt, varsLineLen, flag);
	}
	return ret;
}

int LogStoreApi::ClearCoffer(Coffer* &coffer)
{
	if(coffer && coffer ->data)
	{
		delete coffer ->data;
		coffer ->data = NULL;
	} 
	if(coffer && coffer ->cdata && strlen((char*)coffer->cdata) >6) 
	{
		delete coffer ->cdata;
		coffer ->cdata = NULL;
	}
	return 1;
}
int LogStoreApi::ClearVarFromCache()
{
	LISTMETAS::iterator it = m_glbMeta.begin();
	for (; it != m_glbMeta.end();it++)
	{
		ClearCoffer(it->second);
		it->second = NULL;
	}
	if(m_outliers)
	{
		delete[] m_outliers;
		m_outliers = NULL;
	}
	return 0;
}

int LogStoreApi::DeepCloneMap(LISTBITMAPS source, LISTBITMAPS& des)
{
	des.clear();
	LISTBITMAPS::iterator itor = source.begin();
	for (; itor != source.end();itor++)
	{
		if(itor->second != NULL)
		{
			BitMap* temp = new BitMap(itor ->second ->TotalSize);
			temp->CloneFrom(itor ->second);
			des[itor->first] = temp;
		}
		else
		{
			des[itor->first] = NULL;
		}
	}
}

///////////////////dic & subpat//////////////////////////
//return: length of bitmap, DEF_BITMAP_FULL means matched in sub-pattern, return all
int LogStoreApi::GetVals_Subpat(int varName, const char* regPattern, int queryType, BitMap* bitmap)
{
	m_glbExchgBitmap->Reset();
	m_glbExchgBitmap->SetSize();
	int bitmapSize = GetVals_Subpat_Pushdown(varName, regPattern, queryType, m_glbExchgBitmap);
	//union
	if(bitmapSize !=0)
	{
		bitmap->Union(m_glbExchgBitmap);
	}
	return bitmap->GetSize();
}

int LogStoreApi::GetVals_AxB_Subpat(int varName, const char* queryA, const char* queryB, BitMap* bitmap)
{
	m_glbExchgBitmap->Reset();
	m_glbExchgBitmap->SetSize();
	int bitmapSize = 0;
	//union
	if(bitmapSize !=0)
	{
		bitmap->Union(m_glbExchgBitmap);
	}
	return bitmap->GetSize();
}


int LogStoreApi::GetVals_Subpat_Pushdown(int varName, const char* regPattern, int queryType, BitMap* bitmap)
{
	//first check outliers
	int varFullName = varName + VAR_TYPE_OUTLIER;
	BitMap* tempBitmap = NULL;
	if(m_varouts[varFullName] != NULL)
	{
		tempBitmap = new BitMap(bitmap->TotalSize);
		GetVarOutliers_BM(varFullName, regPattern, tempBitmap, bitmap);
	}
	//search in subpattern
	RegMatrix* regResult = new RegMatrix();
	int subPatRst = SubPatternMatch(m_subpatterns[varName], regPattern, queryType, regResult);
	if(subPatRst == MATCH_ONPAT)//match on pat
	{
		SyslogDebug("----------matched only on subpat: %s\n", FormatVarName(varName));
		return bitmap->GetSize();// keep bitmap unchanged
	}
	else if(subPatRst <= MATCH_MISS)//no matched in subpat
	{
		SyslogDebug("-----------no matched at subpat: %s\n", FormatVarName(varName));
		bitmap->Reset();
		return 0;
	}
	int realSetNum =regResult->ValidCnt;
	int setX = regResult->Count;
	int bitmapSize =0;
	if(realSetNum == 1)//can pushdown directly
	{
		bitmapSize = GetSubVals_Pushdown(regResult->Matches[0], regPattern, queryType, bitmap);
	}
	else if(realSetNum > 1)//must union multi records
	{
		int setY;
		int validCnt =0;
		for(int i= 0; i< setX; i++)
		{
			if(regResult->Matches[i].BeValid == false) continue;
			validCnt++;
			if(validCnt == 1)//the first pushdown merges into m_glbExchgSubBitmap
			{
				m_glbExchgSubBitmap->Reset();
				GetSubVals_Pushdown(regResult->Matches[i], regPattern, queryType, m_glbExchgSubBitmap, bitmap);
			}
			else if(validCnt == realSetNum)//the last directly merges into bitmap
			{
				GetSubVals_Pushdown(regResult->Matches[i], regPattern, queryType, bitmap, NULL);
				bitmap->Union(m_glbExchgSubBitmap);
				break;
			}
			else//the middle, if only one, then merges into m_glbExchgSubBitmap，else into m_glbExchgSubTempBitmap for buffering
			{
				int setY=regResult->Matches[i].Count;
				if(setY == 1)
				{
					GetSubVals_Pushdown(regResult->Matches[i], regPattern, queryType, m_glbExchgSubBitmap, bitmap);
				}
				else
				{
					m_glbExchgSubTempBitmap->Reset();
					GetSubVals_Pushdown(regResult->Matches[i], regPattern, queryType, m_glbExchgSubTempBitmap, bitmap);
					m_glbExchgSubBitmap->Union(m_glbExchgSubTempBitmap);
				}
			}
		}
	}
	else
	{
		SyslogError("Exception occured at matching subpat: %s\n", FormatVarName(varName));
	}
	//union result
	if(tempBitmap != NULL && tempBitmap->GetSize() > 0)
	{
		bitmap->Union(tempBitmap);
		delete tempBitmap;
	}
	delete regResult;
	return bitmap->GetSize();
}

int LogStoreApi::GetVals_Subpat_Pushdown_RefMap(int varName, const char* regPattern, int queryType, BitMap* bitmap, BitMap* refBitmap)
{
	//first check outliers
	int varFullName = varName + VAR_TYPE_OUTLIER;
	BitMap* tempBitmap = NULL;
	if(m_varouts[varFullName] != NULL)
	{
		tempBitmap = new BitMap(refBitmap->TotalSize);
		GetVarOutliers_BM(varFullName, regPattern, tempBitmap, refBitmap);
	}
	//search in subpattern
	RegMatrix* regResult = new RegMatrix();
	int subPatRst = SubPatternMatch(m_subpatterns[varName], regPattern, queryType, regResult);
	if(subPatRst == MATCH_ONPAT)//match on pat
	{
		SyslogDebug("----------matched only on subpat: %d\n", varName);
		return bitmap->GetSize();// keep bitmap unchanged
	}
	else if(subPatRst <= MATCH_MISS)//no matched in subpat
	{
		SyslogDebug("-----------no matched at subpat: %d\n", varName);
		bitmap->Reset();
		return 0;
	}
	int realSetNum =regResult->ValidCnt;
	int setX = regResult->Count;
	int bitmapSize =0;
	if(realSetNum == 1)//can pushdown directly
	{
		bitmapSize = GetSubVals_Pushdown(regResult->Matches[0], regPattern, queryType, bitmap, refBitmap);
	}
	else if(realSetNum > 1)//must union multi records
	{
		int setY;
		int validCnt =0;
		for(int i= 0; i< setX; i++)
		{
			if(regResult->Matches[i].BeValid == false) continue;
			validCnt++;
			if(validCnt == 1)
			{
				m_glbExchgSubBitmap->Reset();
				GetSubVals_Pushdown(regResult->Matches[i], regPattern, queryType, m_glbExchgSubBitmap, refBitmap);
			}
			else if(validCnt == realSetNum)
			{
				GetSubVals_Pushdown(regResult->Matches[i], regPattern, queryType, bitmap, refBitmap);
				bitmap->Union(m_glbExchgSubBitmap);
				break;
			}
			else
			{
				int setY=regResult->Matches[i].Count;
				if(setY == 1)
				{
					GetSubVals_Pushdown(regResult->Matches[i], regPattern, queryType, m_glbExchgSubBitmap, refBitmap);
				}
				else
				{
					m_glbExchgSubTempBitmap->Reset();
					GetSubVals_Pushdown(regResult->Matches[i], regPattern, queryType, m_glbExchgSubTempBitmap, refBitmap);
					m_glbExchgSubBitmap->Union(m_glbExchgSubTempBitmap);
				}
			}
		}
	}
	else
	{
		SyslogError("Exception occured at matching subpat: %d\n", varName);
	}
	//union result
	if(tempBitmap != NULL && tempBitmap->GetSize() > 0)
	{
		bitmap->Union(tempBitmap);
		delete tempBitmap;
	}
	delete regResult;
	return bitmap->GetSize();
}

int LogStoreApi::GetSubVals_Pushdown(RegMatches regmatches, const char* regPattern, int queryType, BitMap* bitmap, BitMap* refBitmap)
{
	int setY=regmatches.Count;
	int type;
	char queryStr[MAX_VALUE_LEN];
	int bitmapSize=0;
	for(int j=0; j< setY;j++)
	{
		memset(queryStr,'\0', MAX_VALUE_LEN);
		if(regmatches.Match[j].Eo == 0)//allow empty space in vars
			queryStr[0] = ' ';
		else
			strncpy(queryStr, regPattern + regmatches.Match[j].So, regmatches.Match[j].Eo);
		if(regmatches.Match[j].So == 0)//matched from the start pos
		{
			if(setY == 1 && regmatches.Match[j].Eo == strlen(regPattern))//can matched at anypos
			{
				type = queryType;
			}
			else//align right
			{
				type = QTYPE_ALIGN_RIGHT;
			}
		}
		else//the left match, align left, including full match
		{
			type = QTYPE_ALIGN_LEFT;
		}
		SyslogDebug("queryStr %s %d/%d varname: %s type:%d\n", queryStr, j, setY, FormatVarName(regmatches.Match[j].Vname), type);
		if(refBitmap == NULL || j >0)
		{
			bitmapSize = QueryByBM_Pushdown(regmatches.Match[j].Vname, queryStr, bitmap, type);
		}
		else
		{
			bitmapSize = QueryByBM_Pushdown_RefMap(regmatches.Match[j].Vname, queryStr, bitmap, refBitmap, type);
		}
		if(bitmapSize == 0)
		{
			break;//no need to continue
		}		
	}	
	return bitmapSize;
}

int LogStoreApi::GetDicIndexs(int varName, const char* regPattern, int queryType, OUT char* &dicQuerySegs)
{
	int num = 0;
	m_glbExchgBitmap->Reset();
	//search in .dic
	RegMatrix* regResult = new RegMatrix();
	int subPatRst = DicPatternMatch(m_subpatterns[varName], regPattern, queryType, regResult);
	if(subPatRst > 0)
	{
		for(int i= 0; i< regResult->Count; i++)
		{
			SyslogDebug("---%d---DicPatternMatch: %d %d %d\n", varName, regResult->Matches[i].Match[0].Index, regResult->Matches[i].Match[0].So, regResult->Matches[i].Match[0].Eo);
		}
		num = QueryByBM_Union_RefRange(varName, regPattern, queryType, m_glbExchgBitmap, regResult);
		SyslogDebug("m_glbExchgBitmap: %d %d\n", m_glbExchgBitmap->GetSize(), m_glbExchgBitmap->Index[0]);
	}
	//if matched in .dic, then get bitmap in .entry
	if(num > 0)
	{
		SyslogDebug("----in dic query index: %d %d\n", varName, num);
		int varfname = varName + VAR_TYPE_ENTRY;
		int entryLen = m_glbMeta[varfname]->eleLen;
		//dic search result may be bigger than 1
		dicQuerySegs = new char[MAX_DICENTY_LEN * num];
		memset(dicQuerySegs, '\0', MAX_DICENTY_LEN * num);
		for(int i=0; i< num; i++)
		{
			if(INC_TEST_FIXED)
			{
				if(!IntPadding(m_glbExchgBitmap->GetIndex(i), entryLen, dicQuerySegs + MAX_DICENTY_LEN * i))
				{
					SyslogDebug("dic entry padding error. %d\n",varName);
				}
				//printf("----%s\n", dicQuerySegs + MAX_DICENTY_LEN * i);
			}
			else
			{
				;
			}
		}
		// for(int i=0; i< num; i++)
		// {
		// 	printf("dicQuerySegs %s %d\n", dicQuerySegs, strlen(dicQuerySegs));
		// }
	}
	delete regResult;
	return num;
}

//queryType: 0: aligned left   1:regular matched
int LogStoreApi::GetVals_Dic(int varName, const char* regPattern, int queryType, OUT BitMap* bitmap)
{
	char* dicQuerySegs = NULL;
	int num = GetDicIndexs(varName, regPattern, queryType, dicQuerySegs);
	//if matched in .dic, then get bitmap in .entry
	if(num > 0)
	{
		int varfname = varName +  VAR_TYPE_ENTRY;
		QueryByBM_Union_ForDic(varfname, dicQuerySegs, num, bitmap);
		delete dicQuerySegs;
	}
	return bitmap->GetSize();
}

int LogStoreApi::GetVals_AxB_Dic(int varName, const char* queryA, const char* queryB, OUT BitMap* bitmap)
{
	char* dicQuerySegs = NULL;
	int num = 0;
	m_glbExchgBitmap->Reset();
	int varfname = varName +  VAR_TYPE_DIC;
	//search in .dic
	num = QueryByBM_AxB_Union(varfname, queryA, queryB, m_glbExchgBitmap);
	//if matched in .dic, then get bitmap in .entry
	if(num > 0)
	{
		varfname = varName +  VAR_TYPE_ENTRY;
		int entryLen = m_glbMeta[varfname]->eleLen;
		//dic search result may be bigger than 1
		char* paddingStr = new char[MAX_DICENTY_LEN * num]{'\0'};
		for(int i=0; i< num; i++)
		{
			//index start from 1
			if(!IntPadding(m_glbExchgBitmap->GetIndex(i), entryLen, paddingStr + MAX_DICENTY_LEN * i))
			{
				SyslogError("dic entry padding error. %d\n",varName);
			}
			//SyslogDebug("---------%d--------%s\n", i, paddingStr + MAX_DICENTY_LEN * i);
		}
		QueryByBM_Union_ForDic(varfname, paddingStr, num, bitmap);
		delete paddingStr;
	}
	return bitmap->GetSize();
}

//queryType: aligned type
int LogStoreApi::GetVals_Dic_Pushdown(int varName, const char* regPattern, int queryType, OUT BitMap* bitmap)
{
	char* dicQuerySegs = NULL;
	int num = GetDicIndexs(varName, regPattern, queryType, dicQuerySegs);
	//if matched in .dic, then get bitmap in .entry
	if(num > 0)
	{
		int varfname = varName +  VAR_TYPE_ENTRY;
		QueryByBM_Pushdown_ForDic(varfname, dicQuerySegs, num, bitmap);
		delete dicQuerySegs;
	}
	else//no matched in dic
	{
		bitmap->Reset();
	}
	return bitmap->GetSize();
}

int LogStoreApi::GetVals_Dic_Pushdown_RefMap(int varName, const char* regPattern, int queryType, OUT BitMap* bitmap, BitMap* refBitmap)
{
	char* dicQuerySegs = NULL;
	int num = GetDicIndexs(varName, regPattern, queryType, dicQuerySegs);
	//if matched in .dic, then get bitmap in .entry
	if(num > 0)
	{
		int varfname = varName +  VAR_TYPE_ENTRY;
		QueryByBM_Pushdown_ForDic_RefMap(varfname, dicQuerySegs, num, bitmap, refBitmap);
		delete dicQuerySegs;
	}
	else//no matched in dic
	{
		bitmap->Reset();
	}
	return bitmap->GetSize();
}

int LogStoreApi::GetDicOffsetByEntry(SubPattern* subpat, int dicNo, int& dicLen)
{
	int offset =0;
	for(int i=0; i< subpat->DicCnt; i++)
	{
		if(dicNo > subpat->DicVars[i]->lineEno)
		{
			offset += subpat->DicVars[i]->lineCnt * subpat->DicVars[i]->len;
		}
		else
		{
			dicNo = dicNo - subpat->DicVars[i]->lineSno;
			offset += dicNo * subpat->DicVars[i]->len;
			dicLen = subpat->DicVars[i]->len;
			break;
		}
	}
	return offset;
}

int LogStoreApi::GetVarOutliers_BM(int varName, const char *queryStr, BitMap* bitmap, BitMap* refBitmap)
{
	int* badc;
	int* goods;
	InitBM(queryStr, badc, goods);
	int matchResult;
	int bitmapIndex =0;
	VarOutliers* outliers = m_varouts[varName];
	if(refBitmap->GetSize() == DEF_BITMAP_FULL)
	{
		for (map<int, char*>::iterator itor = outliers->Outliers.begin(); itor != outliers->Outliers.end();itor++)
		{
			matchResult = BM_Once(itor->second, queryStr, strlen(itor->second), badc, goods);
			if(matchResult >= 0)
			{
				bitmap->Union(itor->first);
			}
		}
	}
	else
	{
		int bitmapSize = refBitmap->GetSize();
		for(int i=0;i< bitmapSize;i++)
		{
			map<int, char*>::iterator itor = outliers->Outliers.find(refBitmap->GetIndex(i));
			if(itor == outliers->Outliers.end()) continue;//not find in outliers
			matchResult = BM_Once(itor->second, queryStr, strlen(itor->second), badc, goods);
			if(matchResult >= 0)
			{
				bitmap->Union(itor->first);
			}
		}
	}
	return bitmap->GetSize();
}

//A:B C
int LogStoreApi::GetOutliers_MultiToken(char *args[MAX_CMD_ARG_COUNT], int argCountS, int argCountE, BitMap* bitmap, bool beReverse)
{
	char queryStr[MAX_PATTERN_SIZE]={'\0'};
	RecombineString(args, argCountS, argCountE, queryStr);
	int lineCount = m_glbMeta[OUTL_PAT_NAME] ->lines;
	if(beReverse)
	{
		return QueryInStrArray_BM_Reverse(m_outliers, lineCount, queryStr, bitmap);
	}
	return QueryInStrArray_BM(m_outliers, lineCount, queryStr, bitmap);
}

int LogStoreApi::GetOutliers_MultiToken_RefMap(char *args[MAX_CMD_ARG_COUNT], int argCountS, int argCountE, BitMap* bitmap, BitMap* refbitmap, bool beReverse)
{
	char queryStr[MAX_PATTERN_SIZE]={'\0'};
	RecombineString(args, argCountS, argCountE, queryStr);
	int lineCount = m_glbMeta[OUTL_PAT_NAME] ->lines;
	if(beReverse)
	{
		return QueryInStrArray_BM_Reverse_RefMap(m_outliers, lineCount, queryStr, bitmap, refbitmap);
	}
	return QueryInStrArray_BM_RefMap(m_outliers, lineCount, queryStr, bitmap, refbitmap);
}


int LogStoreApi::GetOutliers_SinglToken(char *arg, BitMap* bitmap, bool beReverse)
{
	char *wArray[MAX_CMD_PARAMS_COUNT];//split by wildcard
	//the style maybe:  	abcd, ab*, a*d, *cd, *bc*.
	//spit seq with '*':	abcd, ab,  [a,b], cd, bc.
	int mCount = Split_NoDelim(arg, WILDCARD, wArray);
	int lineCount = m_glbMeta[OUTL_PAT_NAME] ->lines;
	int bitmapSize =0;
	if(mCount == 1)
	{
		if(beReverse)
		{
			bitmapSize = QueryInStrArray_BM_Reverse(m_outliers, lineCount, wArray[0], bitmap);
		}
		else
		{
			bitmapSize = QueryInStrArray_BM(m_outliers, lineCount, wArray[0], bitmap);
		}
	}
	else if(mCount == 2)//a*b
	{
		string queryAxB(wArray[0]);
		queryAxB += ".*";
		queryAxB += wArray[1];
		if(beReverse)
		{
			bitmapSize = QueryInStrArray_CReg_Reverse(m_outliers, lineCount, queryAxB.c_str(), bitmap);
		}
		else
		{
			bitmapSize = QueryInStrArray_CReg(m_outliers, lineCount, queryAxB.c_str(), bitmap);
		}
	}
	return bitmapSize;
}

int LogStoreApi::GetOutliers_SinglToken_RefMap(char *arg, BitMap* bitmap, BitMap* refbitmap, bool beReverse)
{
	char *wArray[MAX_CMD_PARAMS_COUNT];//split by wildcard
	//the style maybe:  	abcd, ab*, a*d, *cd, *bc*.
	//spit seq with '*':	abcd, ab,  [a,b], cd, bc.
	int mCount = Split_NoDelim(arg, WILDCARD, wArray);
	int lineCount = m_glbMeta[OUTL_PAT_NAME] ->lines;
	int bitmapSize =0;
	if(mCount == 1)
	{
		if(beReverse)
		{
			bitmapSize = QueryInStrArray_BM_Reverse_RefMap(m_outliers, lineCount, wArray[0], bitmap, refbitmap);
		}
		else
		{
			bitmapSize = QueryInStrArray_BM_RefMap(m_outliers, lineCount, wArray[0], bitmap, refbitmap);
		}
	}
	else if(mCount == 2)//a*b
	{
		string queryAxB(wArray[0]);
		queryAxB += ".*";
		queryAxB += wArray[1];
		if(beReverse)
		{
			bitmapSize = QueryInStrArray_CReg_Reverse_RefMap(m_outliers, lineCount, queryAxB.c_str(), bitmap, refbitmap);
		}
		else
		{
			bitmapSize = QueryInStrArray_CReg_RefMap(m_outliers, lineCount, queryAxB.c_str(), bitmap, refbitmap);
		}
	}
	return bitmapSize;
}

///////////////////Materializ///////////////
//return true or false
int LogStoreApi::Materializ_Dic(int varname, BitMap* bitmap, int entryCnt, OUT char* vars)
{
	int dicname = varname + VAR_TYPE_DIC;
	int entryname = varname + VAR_TYPE_ENTRY;
	//load entries and dic
	Coffer* entryMeta; Coffer* dicMeta;
	int ret = LoadBuffToMemWithDecomp(entryname, entryMeta);
	if(ret <=0) return 0;
	ret = LoadBuffToMemWithDecomp(dicname, dicMeta);
	if(ret <=0) return 0;
	char* entryBuf = entryMeta->data;
	char* dicBuf = dicMeta->data;
	int entryLen = entryMeta->eleLen;
	int dicOffset=0;
	int dicLen =0;
	if(!bitmap->BeSizeFul())
	{
		for(int i=0;i< entryCnt;i++)
		{
			//calc dic offset
			if(bitmap->GetIndex(i) < entryMeta->lines)
			{
				dicOffset = atoi(entryBuf + bitmap->GetIndex(i) * entryLen, entryLen);
				if(dicOffset < dicMeta->lines)
				{
					int offset = GetDicOffsetByEntry(m_subpatterns[varname], dicOffset, dicLen);
					RemovePadding(dicBuf + offset, dicLen, vars + i * MAX_VALUE_LEN);
				}
			}
			else//may be bug
			{
				SyslogError("Materializ_Dic: %s , out of range.\n", FormatVarName(varname));
			}
		}
	}
	else
	{
		for(int i=0;i< entryCnt;i++)
		{
			if(i < entryMeta->lines)
			{
				dicOffset = atoi(entryBuf, entryLen);
				if(dicOffset < dicMeta->lines)
				{
					int offset = GetDicOffsetByEntry(m_subpatterns[varname], dicOffset, dicLen);
					RemovePadding(dicBuf + offset, dicLen, vars + i * MAX_VALUE_LEN);
				}
				entryBuf += entryLen;
			}
			else
			{
				SyslogError("Materializ_Dic: %s , out of range.\n", FormatVarName(varname));
			}
		}
	}
	return 1;
}

int LogStoreApi::Materializ_Dic_Kmp(int varname, BitMap* bitmap, int entryCnt, OUT char* vars)
{
	int dicname = varname + VAR_TYPE_DIC;
	int entryname = varname + VAR_TYPE_ENTRY;
	//load entries and dic
	Coffer* entryMeta; Coffer* dicMeta;
	int ret = LoadBuffToMemWithDecomp(entryname, entryMeta);
	if(ret <=0) return 0;
	ret = LoadBuffToMemWithDecomp(dicname, dicMeta);
	if(ret <=0) return 0;
	char* entryBuf = entryMeta->data;
	char* dicBuf = dicMeta->data;
	int entryLen = entryMeta->eleLen;
	int dicOffset=0;
	int dicLen =0;

	char* entryVars = new char[MAX_DICENTY_LEN * entryMeta->lines];
	memset(entryVars, '\0', MAX_DICENTY_LEN * entryMeta->lines);
	char* dicVars = new char[MAX_VALUE_LEN * dicMeta->lines];
	memset(dicVars, '\0', MAX_VALUE_LEN * dicMeta->lines);
	if(entryCnt != DEF_BITMAP_FULL)
	{
		GetCvarsByBitmap_Diff(entryBuf, 0, bitmap, entryVars, entryCnt, MAX_DICENTY_LEN);
		BitMap* newbitmap = new BitMap(m_maxBitmapSize);
		for(int ii=0; ii< entryCnt; ii++)
		{
			dicOffset = atoi(entryVars + ii * MAX_DICENTY_LEN, MAX_DICENTY_LEN);
			newbitmap->Union(dicOffset);
		}
		GetCvarsByBitmap_Diff(dicBuf, 0, newbitmap, dicVars, entryCnt, MAX_VALUE_LEN);
		for(int i=0;i< entryCnt;i++)
		{
			memcpy(vars + i * MAX_VALUE_LEN, dicVars + i * MAX_VALUE_LEN, strlen(dicVars + i * MAX_VALUE_LEN));
		}
	}
	else
	{
		GetCvars_Diff(entryBuf, entryVars, entryCnt, MAX_DICENTY_LEN);
		GetCvars_Diff(dicBuf, dicVars, entryCnt, MAX_VALUE_LEN);
		for(int i=0;i< entryCnt;i++)
		{
			if(i < entryMeta->lines)
			{
				dicOffset = atoi(entryVars + i * MAX_DICENTY_LEN, MAX_DICENTY_LEN);
				if(dicOffset < dicMeta->lines)
				{
					memcpy(vars + i * MAX_VALUE_LEN, dicVars + dicOffset * MAX_VALUE_LEN, strlen(dicVars + dicOffset * MAX_VALUE_LEN));
				}
			}
			else
			{
				SyslogError("Materializ_Dic: %s, out of range. \n", FormatVarName(varname));
			}
		}
	}
	if(entryVars)
	{
		delete entryVars;
		entryVars = NULL;
	}
	delete dicVars;
	return 1;
}

int LogStoreApi::Materializ_Subpat(SubPattern* subpat, int varname, BitMap* bitmap, int entryCnt, OUT char* vars)
{
	string constStr;
	int varIndex =0;
	int offsetT, offsetV, constStrLen =0;
	int subVarName = 0;
	int outfilename = varname + VAR_TYPE_OUTLIER; 
	for(int i=0;i< subpat->SegSize; i++)
	{
		if (subpat->SubSegAttr[i] == SEG_TYPE_CONST)//const
		{
			constStr = subpat->SubSegment[i];
			constStrLen = strlen(constStr.c_str());
		}
		else//sub-attr
		{
			subVarName = varname | (varIndex<<4) | VAR_TYPE_SUB;
			Coffer* entryMeta;
			int ret = LoadBuffToMemWithDecomp(subVarName, entryMeta);
			if(ret <=0)
			{
				SyslogError("Materializ_Subpat: load subpat failed. %d\n", subVarName);
				return 0;
			} 
			int entryLen = entryMeta->eleLen;
			if(!bitmap->BeSizeFul())
			{
				if(INC_TEST_FIXED && entryLen > 0)
				{
					for(int i=0;i< entryCnt;i++)
					{
						offsetT = bitmap->GetIndex(i) * entryLen;
						offsetV = i * MAX_VALUE_LEN;
						offsetV += strlen(vars + offsetV);
						memcpy(vars + offsetV, constStr.c_str(), constStrLen);
						offsetV += constStrLen;
						char* content = entryMeta->data + offsetT;
						if(m_varouts[outfilename] != NULL && content[entryLen-1] == ' ')//get from outliers
						{
							if(varIndex==0)
							{
								memcpy(vars + i * MAX_VALUE_LEN, m_varouts[outfilename]->Outliers[bitmap->GetIndex(i)], strlen(m_varouts[outfilename]->Outliers[bitmap->GetIndex(i)]));
							}
						}
						else
						{
							RemovePadding(entryMeta->data + offsetT, entryLen, vars + offsetV);
						}
					}
				}
				else
				{
					char* entryVars = new char[MAX_VALUE_LEN * entryMeta->lines];
					memset(entryVars, '\0', MAX_VALUE_LEN * entryMeta->lines);
					GetCvarsByBitmap_Diff(entryMeta->data, 0, bitmap, entryVars, entryCnt, MAX_VALUE_LEN);
					for(int i=0;i< entryCnt;i++)
					{
						offsetT = i * MAX_VALUE_LEN;
						offsetV = i * MAX_VALUE_LEN;
						offsetV += strlen(vars + offsetV);
						memcpy(vars + offsetV, constStr.c_str(), constStrLen);
						offsetV += constStrLen;
						if(m_varouts[outfilename] != NULL && (entryVars + offsetT)[0] == ' ')//get from outliers
						{
							if(varIndex==0)
							{
								//fine the line
								map<int,char*>::iterator itor = m_varouts[outfilename]->Outliers.find(bitmap->GetIndex(i));
								if(itor != m_varouts[outfilename]->Outliers.end())
								{
									//printf("find in var outliers: %d %d\n", outfilename, bitmap->GetIndex(i));
									memcpy(vars + i * MAX_VALUE_LEN, m_varouts[outfilename]->Outliers[bitmap->GetIndex(i)], strlen(m_varouts[outfilename]->Outliers[bitmap->GetIndex(i)]));
								}
							}
						}
						else
						{
							memcpy(vars + offsetV, entryVars + offsetT, strlen(entryVars + offsetT));
						}
					}
					if(entryVars)
					{
						delete[] entryVars;
						entryVars = NULL;
					} 
				}
			}
			else
			{
				if(INC_TEST_FIXED && entryLen > 0)
				{
					for(int i=0;i< entryCnt;i++)
					{
						offsetT = i * entryLen;
						offsetV = i * MAX_VALUE_LEN;
						offsetV += strlen(vars + offsetV);
						memcpy(vars + offsetV, constStr.c_str(), constStrLen);
						offsetV += constStrLen;
						char* content = entryMeta->data + offsetT;
						//if the last is space, then get from the outlier
						if(m_varouts[outfilename] != NULL && content[entryLen-1] == ' ')
						{
							if(varIndex==0)
							{
								//printf("Get from var outliers. %d--%d\n", outfilename, i);
								memcpy(vars + i * MAX_VALUE_LEN, m_varouts[outfilename]->Outliers[i], strlen(m_varouts[outfilename]->Outliers[i]));
							}
						}
						else//get from outliers
						{
							RemovePadding(content, entryLen, vars + offsetV);
						}
					}
				}
				else
				{
					char* entryVars = new char[MAX_VALUE_LEN * entryMeta->lines];
					memset(entryVars, '\0', MAX_VALUE_LEN * entryMeta->lines);
					GetCvars_Diff(entryMeta->data, entryVars, entryCnt, MAX_VALUE_LEN);
					for(int i=0;i< entryCnt;i++)
					{
						offsetT = i * MAX_VALUE_LEN;
						offsetV = i * MAX_VALUE_LEN;
						offsetV += strlen(vars + offsetV);
						memcpy(vars + offsetV, constStr.c_str(), constStrLen);
						offsetV += constStrLen;
						if(m_varouts[outfilename] != NULL && (entryVars + offsetT)[0] == ' ')//get from outliers
						{
							if(varIndex==0)
							{
								map<int,char*>::iterator itor = m_varouts[outfilename]->Outliers.find(i);
								if(itor != m_varouts[outfilename]->Outliers.end())
								{
									//printf("find in var outliers: %d %d\n", outfilename, i);
									memcpy(vars + i * MAX_VALUE_LEN, m_varouts[outfilename]->Outliers[i], strlen(m_varouts[outfilename]->Outliers[i]));
								}
							}
						}
						else
						{
							memcpy(vars + offsetV, entryVars + offsetT, strlen(entryVars + offsetT));
						}
					}
					if(entryVars)
					{
						delete[] entryVars;
						entryVars = NULL;
					}
				}
			}
			varIndex++;
			constStr = "";
		}
	}
	if(constStr !="")// add the last const if exist
	{
		for(int i=0; i< entryCnt; i++)
		{
			offsetV = i * MAX_VALUE_LEN;
			offsetV += strlen(vars + offsetV);
			memcpy(vars + offsetV, constStr.c_str(), constStrLen);
		}
	}
}

//return:  count of records, -1: not find subpat
int LogStoreApi::Materializ_Pats(int varname, BitMap* bitmap, int entryCnt, OUT char* vars)
{
	int ret = 0;
	LISTSUBPATS::iterator itor = m_subpatterns.find(varname);
	if(itor != m_subpatterns.end())//subpattern being found
	{
		if(itor->second->Type == VAR_TYPE_DIC) //dictionary
		{
			if(INC_TEST_FIXED)
			{
				ret = Materializ_Dic(varname, bitmap, entryCnt, vars);
			}
			else
			{
				;
			}
		}
		else if(itor->second->Type == VAR_TYPE_SUB && itor->second->Content != NULL)// subpattern
		{
			ret = Materializ_Subpat(itor->second, varname, bitmap, entryCnt, vars);
		}
		else//.var
		{
			ret = Materializ_Var(varname, bitmap, entryCnt, vars);
		}
	}
	else
	{
		SyslogDebug("do not find in subpat. varname: %s\n", FormatVarName(varname));
	}
	return ret;
}

//return true or false
int LogStoreApi::Materializ_Var(int varname, BitMap* bitmap, int entryCnt, OUT char* vars)
{
	int ret = 0;
	varname += VAR_TYPE_VAR;
	if(bitmap->BeSizeFul())
	{
		ret = LoadcVars(varname, entryCnt, vars, MAX_VALUE_LEN);
	}
	else
	{
		ret = LoadcVarsByBitmap(varname, bitmap, vars, entryCnt, MAX_VALUE_LEN);
	}
	return ret;
}

int LogStoreApi::Materialization(int pid, BitMap* bitmap, int bitmapSize, int matSize)
{
	int entryCnt = bitmapSize >= matSize ? matSize : bitmapSize;
	if(entryCnt <= 0) return entryCnt;

	LogPattern* pat = m_patterns[pid];
	CELL* output = new CELL[pat->SegSize];
	for(int i=0;i< pat->SegSize;i++)
	{
		if(pat->SegAttr[i] == SEG_TYPE_CONST || pat->SegAttr[i] == SEG_TYPE_DELIM)//const string
		{
			output[i] = pat->Segment[i];
		}
		else
		{
			output[i] = new char[MAX_VALUE_LEN * entryCnt];
			memset(output[i], '\0', MAX_VALUE_LEN * entryCnt);
			
			timeval tt1 = ___StatTime_Start();
			Materializ_Pats(pat->VarNames[i], bitmap, entryCnt, output[i]);
			double  time2 = ___StatTime_End(tt1);
			materTime += time2;
			SyslogDebug("----varname:(%d)%s, %lf sec.\n", pat->VarNames[i], FormatVarName(pat->VarNames[i]), time2);
		}
	}
	//print
	for(int k=0; k< entryCnt; k++)
	{
		for(int i=0;i< pat->SegSize;i++)
		{
			if(pat->SegAttr[i] == SEG_TYPE_CONST || pat->SegAttr[i] == SEG_TYPE_DELIM)
			{
				SyslogOut("%s",output[i]);
			}
			else
			{
				SyslogOut("%s",output[i] + MAX_VALUE_LEN * k);
			}
		}
		SyslogOut("\n");
	}
	if(output)
	{
		for(int i=0;i< pat->SegSize;i++)
		{
			if(pat->SegAttr[i] != SEG_TYPE_CONST && pat->SegAttr[i] != SEG_TYPE_DELIM)
			{
				delete[] output[i];
				output[i] = NULL;
			}
		}
	}
	return entryCnt;
}

int LogStoreApi::MaterializOutlier(BitMap* bitmap, int cnt, int refNum)
{
	int doCnt = refNum > cnt ? cnt : refNum;
	for(int i=0; i< doCnt; i++)
	{
		SyslogOut("%s\n", m_outliers[bitmap->GetIndex(i)]);
	}
}
///////////////////Connect & Disconnect///////////////

int LogStoreApi::IsConnect()
{
	return m_nServerHandle;
}

int LogStoreApi::Connect(char *logStorePath, char* fileName)
{
	if(m_nServerHandle == 1) return m_nServerHandle;
	if(logStorePath == NULL || strlen(logStorePath) <=3)
	{
		logStorePath = DIR_PATH_DEFAULT;
	}
	if(fileName == NULL || strlen(fileName) <=3)
	{
		fileName = FILE_NAME_DEFAULT;
	}
	FileName = string(fileName);
	
	struct timeval t1 = ___StatTime_Start();

	int loadNum = BootLoader(logStorePath, fileName);

	double  time2 = ___StatTime_End(t1);
	SyslogDebug("BootLoader : %lfs %d\n", time2, loadNum);
	RunStatus.LogMetaTime = time2;

	if(loadNum > 0)
	{
		memset(m_filePath,'\0',MAX_DIR_PATH);
		strncpy(m_filePath,logStorePath,strlen(logStorePath));
		m_nServerHandle = 1;
	}
	else
	{
		loadNum = 0;
		m_nServerHandle = 0;
	}
	return loadNum;
}

int LogStoreApi::DisConnect()
{
	//delete old patterns
	for (LISTPATS::iterator itor = m_patterns.begin(); itor != m_patterns.end();itor++)
	{
		if(itor->second->Content)
		{
			delete itor->second->Content;
		}
		for(int i=0; i< itor->second->SegSize;i++)
		{
			if (itor->second->Segment[i])
			{
				delete (itor->second->Segment[i]);
			}
		}
	}
	m_patterns.clear();
	//delete old subpatterns
	for (LISTSUBPATS::iterator itor = m_subpatterns.begin(); itor != m_subpatterns.end();itor++)
	{
		if(itor->second->Content)
		{
			delete itor->second->Content;
		}
		for(int i=0; i< itor->second->SegSize;i++)
		{
			if (itor->second->SubSegment[i])
			{
				delete (itor->second->SubSegment[i]);
			}
			if(itor->second->SubVars[i])
			{
				delete (itor->second->SubVars[i]);
			}
		}
		for(int i=0; i< itor->second->DicCnt;i++)
		{
			if(itor->second->DicVars[i])
			{
				delete[] (itor->second->DicVars[i]);
			}
		}
	}
	m_subpatterns.clear();
	memset(m_filePath,'\0',MAX_DIR_PATH);
	if(m_fd > 0)
	{
		close(m_fd);//release file handle
		m_fd = -1;
	}
	if(m_fptr)
	{
		fclose(m_fptr);//release file handle
		m_fptr = NULL;
	}
	ClearVarFromCache();//clear cached vars to release mem
	Release_SearchTemp();
	m_nServerHandle = 0;
	return m_nServerHandle;
}

///////////////////////SELECT///////////////////////////

int LogStoreApi::GetPatterns(OUT vector< pair<string, LogPattern> > &patterns)
{
	// vector< pair<string, LogPattern> > name_score_vec(m_patterns.begin(), m_patterns.end()); 
	// sort(name_score_vec.begin(), name_score_vec.end(), CmpLogPatternByValue()); 
	// patterns = name_score_vec;
	// int vecSize = name_score_vec.size();
	// for (int i=0; i< vecSize;i++)
	// {
	// 	patterns[i] = name_score_vec[i];
	// }
	return 0;
}

int LogStoreApi::GetPatternById(int patId, OUT char** patBody)
{
	LISTPATS::iterator itor = m_patterns.find(patId);
	if(itor != m_patterns.end())
	{
		int len = itor->second->ContSize;
		if(*patBody == NULL)
		{
			*patBody = new char[len + 1]{'\0'};
		}
		strncpy(*patBody, itor->second->Content, len);
		return len;
	}
	return 0;
}

int LogStoreApi::GetVariablesByPatId(int patId, RegMatch *regResult)
{
	LISTPATS::iterator itor = m_patterns.find(patId);
	if(itor != m_patterns.end())
	{
		return 0;//RegMultiQueryAll(itor->second.Content,"<([^<>]*)>", regResult);
	}
	return 0;
}

//select vals -P E10 -V V3~0 -reg 1582675736   3
//select vals -P E10 -V V5~0 -reg 5699476115821218558   2
int LogStoreApi::GetValuesByVarName_Reg(int varName, const char* regPattern, OUT char* vars, OUT BitMap* bitmap)
{
	int num = QueryByBM_Union(varName, regPattern, QTYPE_ALIGN_ANY, bitmap);
	SyslogDebug("BM count:%d\n", num);
	bitmap->Reset();
	num = QueryInMmapByKMP(varName, regPattern, bitmap);
	for(int i=0;i<num;i++)
	{
		//SyslogDebug("KMP bitmap:%d\n", bitmap->GetIndex(i));
	}
	return num;
}

int LogStoreApi::GetValuesByPatId_VarId_Reg(char *args[MAX_CMD_ARG_COUNT], int argCount, OUT char* vars, BitMap* bitmap)
{
	int varname = (atoi(args[3]) <<16) | (atoi(args[5])<<8);
	return GetValuesByVarName_Reg(varname, args[7], vars, bitmap);
}

int LogStoreApi::GetValuesByPatId_VarId(int patId, int varId, OUT char* vars)
{
	return 0;
}

///////////////////////wildcard///////////////////////////////////

int LogStoreApi::SearchInVar_Union(int varName, char *querySeg, short querySegTag, OUT BitMap* bitmap)
{
	int bitmapLen = bitmap->GetSize();//matched or missmatched only at this proc
	LISTSUBPATS::iterator itorsub = m_subpatterns.find(varName);
	if(itorsub != m_subpatterns.end())//subpattern being found
	{
		if(itorsub->second->Type == VAR_TYPE_DIC) //.dic
		{
			bitmapLen = GetVals_Dic(varName, querySeg, QTYPE_ALIGN_ANY, bitmap);
			if(bitmapLen > 0 || bitmapLen == DEF_BITMAP_FULL) 
			{
				SyslogDebug("dic: %s query num: %d\n", FormatVarName(varName), bitmapLen);
			}
		}
		else if(itorsub->second->Type == VAR_TYPE_SUB && itorsub->second->Content != NULL)//.svar
		{
			bitmapLen = GetVals_Subpat(varName, querySeg, QTYPE_ALIGN_ANY, bitmap);
			if(bitmapLen > 0  || bitmapLen == DEF_BITMAP_FULL) 
			{
				SyslogDebug("subattr: %s query num: %d\n", FormatVarName(varName), bitmapLen);
			}
		}
		else if(itorsub->second->Type == VAR_TYPE_VAR) //.var
		{
			Statistic.total_queried_cap_cnt++;
			//filter length and tag
			//bool beFilterFailed = (itorsub->second->Tag & querySegTag != querySegTag) || strlen(querySeg) > itorsub->second->ContSize;
			bool beLenFilterFailed = strlen(querySeg) > itorsub->second->ContSize;
			bool beTagFilterFailed = itorsub->second->Tag & querySegTag != querySegTag;
			if(INC_TEST_JUDGETAG && beLenFilterFailed)
			{
				Statistic.length_filtered_cap_cnt++;
				return bitmapLen;
			}
			if(INC_TEST_JUDGETAG && beTagFilterFailed)
			{
				Statistic.tag_filtered_cap_cnt++;
				return bitmapLen;
			}

			int varfname = varName + VAR_TYPE_VAR;
			bitmapLen = QueryByBM_Union(varfname, querySeg, QTYPE_ALIGN_ANY, bitmap);
			if(bitmapLen > 0 || bitmapLen == DEF_BITMAP_FULL)
			{
				SyslogDebug(".var:%s, query num: %d\n", FormatVarName(varName), bitmapLen);
			}
		}
		else//unknown type, may skip
		{
			SyslogError("Error: Name= %s\n", FormatVarName(varName));
		}

	}
	else//not find, output error
	{
		SyslogError("Error: not find var in variables.txt(SearchInVar_Union), varname: %s(%d), filename:%s\n", FormatVarName(varName), varName, FileName.c_str());
	}
	return bitmapLen;
}

int LogStoreApi::SearchInVar_AxB_Union(int varName, char *queryA, char *queryB, short qATag, short qBTag, OUT BitMap* bitmap)
{
	int bitmapLen = bitmap->GetSize();//matched or missmatched only at this proc
	int abLen = strlen(queryA) + strlen(queryB);
	LISTSUBPATS::iterator itorsub = m_subpatterns.find(varName);
	if(itorsub != m_subpatterns.end())//subpattern being found
	{
		if(itorsub->second->Type == VAR_TYPE_DIC) //.dic
		{
			bitmapLen = GetVals_AxB_Dic(varName, queryA, queryB, bitmap);
			if(bitmapLen > 0 || bitmapLen == DEF_BITMAP_FULL) 
			{
				SyslogDebug("dic: %s query num: %d\n", FormatVarName(varName), bitmapLen);
			}
		}
		else if(itorsub->second->Type == VAR_TYPE_SUB && itorsub->second->Content != NULL)//.svar
		{
			bitmapLen = GetVals_AxB_Subpat(varName, queryA, queryB, bitmap);
			if(bitmapLen > 0  || bitmapLen == DEF_BITMAP_FULL) 
			{
				SyslogDebug("subattr: %s query num: %d\n", FormatVarName(varName), bitmapLen);
			}
		}
		else if(itorsub->second->Type == VAR_TYPE_VAR) //.var
		{
			bool beOk = (itorsub->second->Tag & qATag != qATag) || strlen(queryA) > itorsub->second->ContSize
				|| (itorsub->second->Tag & qBTag != qBTag) || strlen(queryB) > itorsub->second->ContSize;
			if(INC_TEST_JUDGETAG && beOk)
			{
				return bitmapLen;
			}
			//because no predict filtering, query will slow
			int varfname = varName + VAR_TYPE_VAR;
			bitmapLen = QueryByBM_AxB_Union(varfname, queryA, queryB, bitmap);
			if(bitmapLen > 0 || bitmapLen == DEF_BITMAP_FULL)
			{
				SyslogDebug(".var:%s, query num: %d\n", FormatVarName(varName), bitmapLen);
			}
		}
		else//unknown type, may skip
		{
			SyslogError("Error: name= %s\n", FormatVarName(varName));
		}

	}
	else//not find, output error
	{
		SyslogError("Error: not find var in variables.txt, varname: %s(%d), filename:%s\n", FormatVarName(varName), varName, FileName.c_str());
	}
	return bitmapLen;
}

//queryType: 0: align left   1: align right
int LogStoreApi::SearchInVar_Pushdown(int varName, char *querySeg, short querySegTag, int queryType, OUT BitMap* bitmap)
{
	if(bitmap->GetSize() == 0) return 0;
	//match subpatterns
	int bitmapLen =0;
	LISTSUBPATS::iterator itorsub = m_subpatterns.find(varName);
	if(itorsub != m_subpatterns.end())//subpattern being found
	{
		if(itorsub->second->Type == VAR_TYPE_DIC) //it is dictionary
		{
			bitmapLen = GetVals_Dic_Pushdown(varName, querySeg, queryType, bitmap);
		}
		else if(itorsub->second->Type == VAR_TYPE_SUB && itorsub->second->Content != NULL)//it is subpattern
		{
			bitmapLen = GetVals_Subpat_Pushdown(varName, querySeg, queryType, bitmap);
		}
		else if(itorsub->second->Type == VAR_TYPE_VAR) //.var
		{
			bool beOk = (itorsub->second->Tag & querySegTag != querySegTag) || strlen(querySeg) > itorsub->second->ContSize;
			if(INC_TEST_JUDGETAG && beOk)
			{
				bitmap->Reset();
				return 0;
			}
			//because no predict filtering, query will slow
			int varfname = varName + VAR_TYPE_VAR;
			bitmapLen = QueryByBM_Pushdown(varfname, querySeg, bitmap, queryType);
		}
		else//unknown type, may skip
		{
			SyslogError("Error: Type= %s\n", FormatVarName(varName));
		}

	}
	else//no subpattern
	{
		SyslogError("Error: not find var in variables.txt, varname: %s(%d), filename:%s\n", FormatVarName(varName), varName, FileName.c_str());
	}
	SyslogDebug("----%s : %d\n", FormatVarName(varName), bitmapLen);
	return bitmapLen;
}

int LogStoreApi::SearchInVar_Pushdown_RefMap(int varName, char *querySeg, short querySegTag, int queryType, OUT BitMap* bitmap, BitMap* refBitmap)
{
	//match subpatterns
	int bitmapLen =0;
	LISTSUBPATS::iterator itorsub = m_subpatterns.find(varName);
	if(itorsub != m_subpatterns.end())//subpattern being found
	{
		if(itorsub->second->Type == VAR_TYPE_DIC) //it is dictionary
		{
			bitmapLen = GetVals_Dic_Pushdown_RefMap(varName, querySeg, queryType, bitmap, refBitmap);
		}
		else if(itorsub->second->Type == VAR_TYPE_SUB && itorsub->second->Content != NULL)//it is subpattern
		{
			bitmapLen = GetVals_Subpat_Pushdown_RefMap(varName, querySeg, queryType, bitmap, refBitmap);
		}
		else if(itorsub->second->Type == VAR_TYPE_VAR) //.var
		{
			bool beOk = (itorsub->second->Tag & querySegTag != querySegTag) || strlen(querySeg) > itorsub->second->ContSize;
			if(INC_TEST_JUDGETAG && beOk)
			{
				bitmap->Reset();
				return 0;
			}
			//because no predict filtering, query will slow
			int varfname = varName + VAR_TYPE_VAR;
			bitmapLen = QueryByBM_Pushdown_RefMap(varfname, querySeg, bitmap, refBitmap, queryType);
		}
		else//unknown type, may skip
		{
			SyslogError("Error: name= %s\n", FormatVarName(varName));
		}

	}
	else//no subpattern
	{
		SyslogError("Error: not find var in variables.txt, varname: %s(%d), filename:%s\n", FormatVarName(varName), varName, FileName.c_str());
	}
	SyslogDebug("----%s : %d\n", FormatVarName(varName), bitmapLen);
	return bitmapLen;
}

//querySegTag  0: int  1: str
int LogStoreApi::SearchSingleInPattern(LogPattern* logPat, char *queryStr, short queryStrTag, BitMap* bitmap)
{
	int* badc;
	int* goods;
	InitBM(queryStr, badc, goods);
	//assume: first find in main pattern, if matched, then return
	int matchedPos = BM_Once(logPat->Content, queryStr, strlen(logPat->Content), badc, goods);
	if (matchedPos >=0)
	{
		SyslogDebug("---matched on logPat!----------------\n");
		bitmap->SetSize();
	}
	else
	{
		//match with each segment
		for(int i=0; i< logPat->SegSize;i++)
		{
			if(logPat->SegAttr[i] == SEG_TYPE_VAR)//this segment is var, must pushdown to query in vars
			{
				//each var query is isolate, so should union each query result
				int num = SearchInVar_Union(logPat->VarNames[i], queryStr, queryStrTag, bitmap);
				if(num == DEF_BITMAP_FULL)
				{
					//!!! matched in sub-pattern, means matched all
					bitmap->SetSize();
					break;
				}
			}
		}
	}
	return bitmap->GetSize();
}

int LogStoreApi::SearchSingleInPattern_RefMap(LogPattern* logPat, char *queryStr, short queryStrTag, BitMap* bitmap, BitMap* refBitmap)
{
	int* badc;
	int* goods;
	InitBM(queryStr, badc, goods);
	//assume: first find in main pattern, if matched, then return
	int matchedPos = BM_Once(logPat->Content, queryStr, strlen(logPat->Content), badc, goods);
	if (matchedPos >=0)
	{
		bitmap->Union(refBitmap);
		SyslogDebug("---matched on logPat!-----\n");
	}
	else
	{
		//match with each segment
		for(int i=0; i< logPat->SegSize;i++)
		{
			if(logPat->SegAttr[i] == SEG_TYPE_VAR)//this segment is var, must pushdown to query in vars
			{
				m_glbExchgPatmap->Reset();
				//each var query is isolate, so should union each query result
				int num = SearchInVar_Pushdown_RefMap(logPat->VarNames[i], queryStr, queryStrTag, QTYPE_ALIGN_ANY, m_glbExchgPatmap, refBitmap);
				if(num > 0)
				{
					bitmap->Union(m_glbExchgPatmap);
				}
			}
		}
	}
	return bitmap->GetSize();
}

int LogStoreApi::SearchMultiInPattern(LogPattern* logPat, char **querySegs, int argCountS, int argCountE, short* querySegTags, int* querySegLens, BitMap* bitmap)
{
	int isMatched;
	int iPos, j, maxI, segLen;
	bitset<MAX_INPUT_TOKENSIZE> flag;//identity that if matched const segment, at least matched one (flag = 1)
	int segSize = argCountE - argCountS + 1;
	maxI = logPat->SegSize - segSize;// get the max comparing pos
	for(int index=0; index<= maxI;index++)
	{
		iPos = index;
		for(j=0; j< segSize; j++)
		{
			//matched by delim
			if(logPat->SegAttr[iPos] == SEG_TYPE_DELIM)//delim
			{
				if(querySegTags[j] == TAG_DELIM && querySegs[j + argCountS][0] == logPat->Segment[iPos][0])//delim
				{
					flag.set(j);
					iPos++;
				}
				else
				{
					break;
				}
			}
			else if(logPat->SegAttr[iPos] == SEG_TYPE_VAR)//var:<V1>...
			{
				if(querySegTags[j] == TAG_DELIM)
				{
					break;
				}
				else
				{
					iPos++;
				}
			}
			else//const
			{
				if(querySegTags[j] == TAG_DELIM)
				{
					break;
				}
				else
				{
					//degraded to string matching 
					segLen = strlen(logPat->Segment[iPos]);
					isMatched = SeqMatching_BothSide(logPat->Segment[iPos], segLen, querySegs[j+ argCountS], querySegLens[j]);
					if(isMatched > 0)//matched!
					{
						//queue from right to left, the first is the rightest
						flag.set(j);
						iPos++;
					}
					else//match failed!
					{
						break;
					}
				}
			}
		}
		//total matched, record every pos, may matched multi results
		if(j == segSize)
		{
			SyslogDebug("multiquery: i: %d\n", index);
			if(flag.count() == segSize)
			{//all matched in main pattern,return full matched
				bitmap->SetSize();
				break;
			}
			m_glbExchgPatmap->Reset();
			m_glbExchgPatmap->SetSize();
			int queryType;
			for(int flagIndex = 0; flagIndex < segSize; flagIndex++)
			{
				if(flag[flagIndex] == 0 && m_glbExchgPatmap->GetSize() != 0)//pushdown to query vars
				{
					if(flagIndex == 0)//first seg matches the variable, then the remainings are right alighed
					{
						queryType = QTYPE_ALIGN_RIGHT;
					}
					else if(flagIndex == segSize-1)//last seg in var, then left aligned
					{
						queryType = QTYPE_ALIGN_LEFT;
					}
					else//the middles are must fully matched
					{
						queryType = QTYPE_ALIGN_FULL;
					}
					SearchInVar_Pushdown(logPat->VarNames[index + flagIndex], querySegs[flagIndex + argCountS], querySegTags[flagIndex], queryType, m_glbExchgPatmap);
				}
			}
			bitmap->Union(m_glbExchgPatmap);
		}
		flag.reset();
	}
	return bitmap->GetSize();
}

int LogStoreApi::SearchMultiInPattern_RefMap(LogPattern* logPat, char **querySegs, int argCountS, int argCountE, short* querySegTags, int* querySegLens, BitMap* bitmap, BitMap* refBitmap)
{
	int isMatched;
	int iPos, j, maxI, segLen;
	bitset<MAX_INPUT_TOKENSIZE> flag;//identity that if matched const segment, at least matched one (flag = 1)
	int segSize = argCountE - argCountS + 1;
	maxI = logPat->SegSize - segSize;// get the max comparing pos
	for(int index=0; index<= maxI;index++)
	{
		iPos = index;
		for(j=0; j< segSize; j++)
		{
			//matched by delim
			if(logPat->SegAttr[iPos] == SEG_TYPE_DELIM)//delim
			{
				if(querySegTags[j] == TAG_DELIM && querySegs[j + argCountS][0] == logPat->Segment[iPos][0])//delim
				{
					flag.set(j);
					iPos++;
				}
				else
				{
					break;
				}
			}
			else if(logPat->SegAttr[iPos] == SEG_TYPE_VAR)//var:<V1>...
			{
				if(querySegTags[j] == TAG_DELIM)
				{
					break;
				}
				else
				{
					iPos++;
				}
			}
			else//const
			{
				if(querySegTags[j] == TAG_DELIM)
				{
					break;
				}
				else
				{
					segLen = strlen(logPat->Segment[iPos]);
					isMatched = SeqMatching_BothSide(logPat->Segment[iPos], segLen, querySegs[j+ argCountS], querySegLens[j]);
					if(isMatched > 0)//matched!
					{
						//queue from right to left, the first is the rightest
						flag.set(j);
						iPos++;
					}
					else//match failed!
					{
						break;
					}
				}
			}
		}
		//total matched, record every pos, may matched multi results
		if(j == segSize)
		{
			SyslogDebug("multiquery: i: %d\n", index);
			if(flag.count() == segSize)
			{//all matched in main pattern,return full matched
				bitmap->Union(refBitmap);
				break;
			}
			m_glbExchgPatmap->Reset();
			int queryType;
			int count=0;
			for(int flagIndex = 0; flagIndex < segSize; flagIndex++)
			{
				if(flag[flagIndex] == 0)//pushdown to query vars
				{
					if(flagIndex == 0)
					{
						queryType = QTYPE_ALIGN_RIGHT;
					}
					else if(flagIndex == segSize-1)
					{
						queryType = QTYPE_ALIGN_LEFT;
					}
					else
					{
						queryType = QTYPE_ALIGN_FULL;
					}
					if(count == 0)
					{
						SearchInVar_Pushdown_RefMap(logPat->VarNames[index + flagIndex], querySegs[flagIndex + argCountS], querySegTags[flagIndex], queryType, m_glbExchgPatmap, refBitmap);
					}
					else
					{
						SearchInVar_Pushdown(logPat->VarNames[index + flagIndex], querySegs[flagIndex + argCountS], querySegTags[flagIndex], queryType, m_glbExchgPatmap);
					}
					count++;
				}
			}
			bitmap->Union(m_glbExchgPatmap);
		}
		flag.reset();
	}
	return bitmap->GetSize();
}

//query A*B in one token
int LogStoreApi::Search_AxB_InPattern(LogPattern* logPat, char* queryStrA, char* queryStrB, short qATag, short qBTag, BitMap* bitmap)
{
	return bitmap->GetSize();
}
int LogStoreApi::Search_AxB_InPattern_Logic(LogPattern* logPat, char* queryStrA, char* queryStrB, BitMap* bitmap, BitMap* refBitmap)
{
	;
}

int LogStoreApi::Search_SingleSegment(char *querySeg, OUT LISTBITMAPS &bitmaps)
{
	char *wArray[MAX_CMD_PARAMS_COUNT];//split by wildcard
	//the style maybe:  	abcd, ab*, a*d, *cd, *bc*.
	//spit seq with '*':	abcd, ab,  [a,b], cd, bc.
	int mCount = Split_NoDelim(querySeg, WILDCARD, wArray);
	int num = 0;
	if(mCount == 1)//abcd, ab*, *cd, *bc*.
	{
		short queryStrTag = GetStrTag(wArray[0], strlen(wArray[0]));
		//match with each main pattern
		LISTPATS::iterator itor = m_patterns.begin();
		for (; itor != m_patterns.end();itor++)
		{
			BitMap* bitmap = new BitMap(itor->second->Count);
			num += SearchSingleInPattern(itor->second, wArray[0], queryStrTag, bitmap);
			if(bitmap->GetSize() > 0 || bitmap->BeSizeFul())
			{
				bitmaps[itor->first] = bitmap;
			}
			else
			{
				bitmaps[itor->first] = NULL;
			}
		}
	}
	else if(mCount == 2)//a*b
	{
		short queryATag = GetStrTag(wArray[0], strlen(wArray[0]));
		short queryBTag = GetStrTag(wArray[1], strlen(wArray[1]));
		//match with each main pattern
		LISTPATS::iterator itor = m_patterns.begin();
		for (; itor != m_patterns.end();itor++)
		{
			BitMap* bitmap = new BitMap(itor->second->Count);
			num += Search_AxB_InPattern(itor->second, wArray[0], wArray[1], queryATag, queryBTag, bitmap);
			if(bitmap->GetSize() > 0 || bitmap->BeSizeFul())
			{
				bitmaps[itor->first] = bitmap;
			}
			else
			{
				bitmaps[itor->first] = NULL;
			}
		}
	}
	//delete
	for(int i=0;i<mCount;i++)
	{
		if (wArray[i])
		{
			delete (wArray[i]);
		}
	}
	return num;
}

//select * -m token:1576667788536595
int LogStoreApi::Search_MultiSegments(char **querySegs, int segSize, OUT LISTBITMAPS& bitmaps)
{
	short* querySegTags = new short[segSize];
	int* querySegLens = new int[segSize];
	for(int i=0; i< segSize; i++)
	{
		querySegLens[i] = strlen(querySegs[i]);
		querySegTags[i] = GetStrTag(querySegs[i], querySegLens[i]);
		//SyslogDebug("%s %d %d\n", querySegs[i], querySegTags[i], querySegLens[i]);
	}
	//match with each main pattern
	LISTPATS::iterator itor = m_patterns.begin();
	for (; itor != m_patterns.end();itor++)
	{
		BitMap* bitmap = new BitMap(itor->second->Count);
		SearchMultiInPattern(itor->second, querySegs, 0, segSize-1, querySegTags, querySegLens, bitmap);
		if(bitmap->GetSize() > 0 || bitmap->BeSizeFul())
		{
			bitmaps[itor->first] = bitmap;
		}
		else
		{
			bitmaps[itor->first] = NULL;
		}
	}
	delete[] querySegTags;
	delete[] querySegLens;
	return 0;
}

int LogStoreApi::IsSearchWithLogic(char *args[MAX_CMD_ARG_COUNT], int argCount)
{
	for(int i=0; i< argCount; i++)
	{
		if(stricmp(args[i], LOGIC_AND) == 0 || stricmp(args[i], LOGIC_OR) == 0 || stricmp(args[i], LOGIC_NOT) == 0)
		{
			return 1;
		}
	}
	return 0;
}

int LogStoreApi::SearchByLogic_not(char *args[MAX_CMD_ARG_COUNT], int argCountS, int argCountE, OUT LISTBITMAPS& bitmaps)
{
	char *wArray[MAX_CMD_PARAMS_COUNT];
	int mCount = 0;
	short queryStrTag = 0;short queryStr2Tag = 0;
	short* querySegTags = NULL;
	int* querySegLens = NULL;
	int segSize = argCountE - argCountS + 1;
	if(segSize == 1)//single
	{
		mCount = Split_NoDelim(args[argCountS], WILDCARD, wArray);
		queryStrTag = GetStrTag(wArray[0], strlen(wArray[0]));
		if(mCount >1)//for A*B
		{
			queryStr2Tag = GetStrTag(wArray[1], strlen(wArray[1]));
		}
	}
	else//multi
	{
		mCount = 0;
		querySegTags = new short[segSize];
		querySegLens = new int[segSize];
		for(int i=0; i< segSize; i++)
		{
			querySegLens[i] = strlen(args[i + argCountS]);
			querySegTags[i] = GetStrTag(args[i + argCountS], querySegLens[i]);
		}
	}
	//match with each main pattern
	LISTPATS::iterator itor = m_patterns.begin();
	for (; itor != m_patterns.end();itor++)
	{
		if(bitmaps[itor->first] == NULL)// query end
		{
			continue;
		}
		else if(bitmaps[itor->first]->BeSizeFul())
		{
			bitmaps[itor->first] ->Reset();
			if(mCount == 1)//abcd, ab*, *cd, *bc*.
			{
				SearchSingleInPattern(itor->second, wArray[0], queryStrTag, bitmaps[itor->first]);
			}
			else if(mCount == 0)//A:B
			{
				SearchMultiInPattern(itor->second, args, argCountS, argCountE, querySegTags, querySegLens, bitmaps[itor->first]);
			}
			else
			{
				Search_AxB_InPattern(itor->second, wArray[0], wArray[1], queryStrTag, queryStr2Tag, bitmaps[itor->first]);
			}
			if(bitmaps[itor->first]->BeSizeFul())
			{
				delete bitmaps[itor->first];
				bitmaps[itor->first] = NULL;
			}
			else
			{
				bitmaps[itor->first]->Reverse();
			}		
		}
		else if(bitmaps[itor->first]->GetSize() > 0)
		{
			if(INC_TEST_PUSHDOWN)
			{
				m_glbExchgLogicmap ->Reset();
				if(mCount == 1)
				{
					SearchSingleInPattern_RefMap(itor->second, wArray[0], queryStrTag, m_glbExchgLogicmap, bitmaps[itor->first]);
				}
				else if(mCount == 0)//A:B
				{
					SearchMultiInPattern_RefMap(itor->second, args, argCountS, argCountE, querySegTags, querySegLens, m_glbExchgLogicmap, bitmaps[itor->first]);
				}
				else
				{
					//Search_AxB_InPattern_Logic(itor->second, wArray[0], wArray[1], rangeSize, range, bitmaps[itor->first]);
				}
				bitmaps[itor->first]->Complement(m_glbExchgLogicmap);
				if(bitmaps[itor->first]->GetSize() == 0)
				{
					delete bitmaps[itor->first];
					bitmaps[itor->first] = NULL;
				}
			}
		}
	}
	//search in outliers
	if(bitmaps[OUTL_PAT_NAME] == NULL || bitmaps[OUTL_PAT_NAME]->GetSize() == 0)
	{
		;
	}
	else if(bitmaps[OUTL_PAT_NAME]->GetSize() > 0)
	{
		if(INC_TEST_PUSHDOWN)
		{
			if(mCount == 1)
			{
				GetOutliers_SinglToken(wArray[0], bitmaps[OUTL_PAT_NAME], true);
			}
			else if(mCount == 0)
			{
				GetOutliers_MultiToken(args, argCountS, argCountE, bitmaps[OUTL_PAT_NAME], true);
			}
			else//A*B
			{
				;
			}
			if(bitmaps[OUTL_PAT_NAME]->GetSize() == 0)
			{
				delete bitmaps[OUTL_PAT_NAME];
				bitmaps[OUTL_PAT_NAME] = NULL;
			}
		}
	}
	//delete
	for(int i=0;i<mCount;i++)
	{
		if (wArray[i])
		{
			delete (wArray[i]);
		}
	}
	if(querySegTags) delete[] querySegTags;
	if(querySegLens) delete[] querySegLens;
	return 0;
}

int LogStoreApi::SearchByLogic_norm(char *args[MAX_CMD_ARG_COUNT], int argCountS, int argCountE, OUT LISTBITMAPS& bitmaps)
{
	char *wArray[MAX_CMD_PARAMS_COUNT];
	int mCount = 0;
	short queryStrTag = 0;short queryStr2Tag = 0;
	short* querySegTags = NULL;
	int* querySegLens = NULL;
	int segSize = argCountE - argCountS + 1;
	if(segSize == 1)//single
	{
		mCount = Split_NoDelim(args[argCountS], WILDCARD, wArray);
		queryStrTag = GetStrTag(wArray[0], strlen(wArray[0]));
		if(mCount >1)//for A*B
		{
			queryStr2Tag = GetStrTag(wArray[1], strlen(wArray[1]));
		}
	}
	else//multi
	{
		mCount = 0;
		querySegTags = new short[segSize];
		querySegLens = new int[segSize];
		for(int i=0; i< segSize; i++)
		{
			querySegLens[i] = strlen(args[i + argCountS]);
			querySegTags[i] = GetStrTag(args[i + argCountS], querySegLens[i]);
		}
	}
	//match with each main pattern
	LISTPATS::iterator itor = m_patterns.begin();
	LISTBITMAPS::iterator ifind;
	for (; itor != m_patterns.end();itor++)
	{
		ifind = bitmaps.find(itor->first);
		//not find, means first step to search
		if(ifind == bitmaps.end())
		{
			BitMap* bitmap = new BitMap(itor->second->Count);
			if(mCount == 1)//abcd, ab*, *cd, *bc*.
			{
				SearchSingleInPattern(itor->second, wArray[0], queryStrTag, bitmap);
			}
			else if(mCount == 0)//A:B
			{
				SearchMultiInPattern(itor->second, args, argCountS, argCountE, querySegTags, querySegLens, bitmap);
			}
			else
			{
				Search_AxB_InPattern(itor->second, wArray[0], wArray[1], queryStrTag, queryStr2Tag, bitmap);
			}
			if(bitmap->GetSize() == 0)
			{
				delete bitmap;
				bitmaps[itor->first] = NULL;
			}
			else
			{
				bitmaps[itor->first] = bitmap;
			}
		}
		else if(bitmaps[itor->first] == NULL)// query end
		{
			continue;
		}
		else if(bitmaps[itor->first]->BeSizeFul())
		{
			bitmaps[itor->first] ->Reset();
			if(mCount == 1)//abcd, ab*, *cd, *bc*.
			{
				SearchSingleInPattern(itor->second, wArray[0], queryStrTag, bitmaps[itor->first]);
			}
			else if(mCount == 0)//A:B
			{
				SearchMultiInPattern(itor->second, args, argCountS, argCountE, querySegTags, querySegLens, bitmaps[itor->first]);
			}
			else
			{
				Search_AxB_InPattern(itor->second, wArray[0], wArray[1], queryStrTag, queryStr2Tag, bitmaps[itor->first]);
			}
			if(bitmaps[itor->first]->GetSize() == 0)
			{
				delete bitmaps[itor->first];
				bitmaps[itor->first] = NULL;
			}			
		}
		else if(bitmaps[itor->first]->GetSize() > 0)
		{
			BitMap* bitmap = new BitMap(itor->second->Count);
			if(INC_TEST_PUSHDOWN)
			{
				if(mCount == 1)
				{
					SearchSingleInPattern_RefMap(itor->second, wArray[0], queryStrTag, bitmap, bitmaps[itor->first]);
				}
				else if(mCount == 0)//A:B
				{
					SearchMultiInPattern_RefMap(itor->second, args, argCountS, argCountE, querySegTags, querySegLens, bitmap, bitmaps[itor->first]);
				}
				else
				{
					//Search_AxB_InPattern_Logic(itor->second, wArray[0], wArray[1], rangeSize, range, bitmaps[itor->first]);
				}
				if(bitmap->GetSize() == 0)
				{
					delete bitmaps[itor->first];
					bitmaps[itor->first] = NULL;
				}
				else
				{
					delete bitmaps[itor->first];
					bitmaps[itor->first] = bitmap;
				}
			}
		}
		else
		{
			if(bitmaps[itor->first])
			{
				delete bitmaps[itor->first];
				bitmaps[itor->first] = NULL;
			}
		}
	}
	//search in outliers
	ifind = bitmaps.find(OUTL_PAT_NAME);
	if(ifind == bitmaps.end())
	{
		BitMap* bitmap_outlier = new BitMap(m_glbMeta[OUTL_PAT_NAME]->lines);
		bitmap_outlier->SetSize();
		if(mCount == 1)
		{
			GetOutliers_SinglToken(wArray[0], bitmap_outlier);
		}
		else if(mCount == 0)
		{
			GetOutliers_MultiToken(args, argCountS, argCountE, bitmap_outlier);
		}
		else//A*B
		{
			;
		}
		if(bitmap_outlier->GetSize() == 0)
		{
			delete bitmap_outlier;
			bitmaps[OUTL_PAT_NAME] = NULL;
		}
		else
		{
			bitmaps[OUTL_PAT_NAME] = bitmap_outlier;
		}
	}
	else if(bitmaps[OUTL_PAT_NAME] == NULL || bitmaps[OUTL_PAT_NAME]->GetSize() == 0)
	{
		;//query end
	}
	else if(bitmaps[OUTL_PAT_NAME]->GetSize() > 0)
	{
		if(INC_TEST_PUSHDOWN)
		{
			if(mCount == 1)
			{
				GetOutliers_SinglToken(wArray[0], bitmaps[OUTL_PAT_NAME]);
			}
			else if(mCount == 0)
			{
				GetOutliers_MultiToken(args, argCountS, argCountE, bitmaps[OUTL_PAT_NAME]);
			}
			else//A*B
			{
				;
			}
			if(bitmaps[OUTL_PAT_NAME]->GetSize() == 0)
			{
				delete bitmaps[OUTL_PAT_NAME];
				bitmaps[OUTL_PAT_NAME] = NULL;
			}
		}
	}
	else
	{
		SyslogError("bitmaps[OUTL_PAT_NAME]->GetSize() is -99!");
	}

	//delete
	for(int i=0;i<mCount;i++)
	{
		if (wArray[i])
		{
			delete (wArray[i]);
		}
	}
	if(querySegTags) delete[] querySegTags;
	if(querySegLens) delete[] querySegLens;
	return 0;
}

int LogStoreApi::SearchByLogic_norm_RefMap(char *args[MAX_CMD_ARG_COUNT], int argCountS, int argCountE, OUT LISTBITMAPS& bitmaps, LISTBITMAPS refbitmaps)
{
	char *wArray[MAX_CMD_PARAMS_COUNT];
	int mCount = 0;
	short queryStrTag = 0;short queryStr2Tag = 0;
	short* querySegTags = NULL;
	int* querySegLens = NULL;
	int segSize = argCountE - argCountS + 1;
	if(segSize == 1)//single
	{
		mCount = Split_NoDelim(args[argCountS], WILDCARD, wArray);
		queryStrTag = GetStrTag(wArray[0], strlen(wArray[0]));
		if(mCount >1)//for A*B
		{
			queryStr2Tag = GetStrTag(wArray[1], strlen(wArray[1]));
		}
	}
	else//multi
	{
		mCount = 0;
		querySegTags = new short[segSize];
		querySegLens = new int[segSize];
		for(int i=0; i< segSize; i++)
		{
			querySegLens[i] = strlen(args[i + argCountS]);
			querySegTags[i] = GetStrTag(args[i + argCountS], querySegLens[i]);
		}
	}
	//match with each main pattern
	LISTPATS::iterator itor = m_patterns.begin();
	LISTBITMAPS::iterator ifind;
	for (; itor != m_patterns.end();itor++)
	{
		ifind = refbitmaps.find(itor->first);
		//not find, means first step to search
		if(ifind == refbitmaps.end() || (refbitmaps[itor->first] != NULL && refbitmaps[itor->first]->BeSizeFul()))
		{
			BitMap* bitmap = new BitMap(itor->second->Count);
			if(mCount == 1)//abcd, ab*, *cd, *bc*.
			{
				SearchSingleInPattern(itor->second, wArray[0], queryStrTag, bitmap);
			}
			else if(mCount == 0)//A:B
			{
				SearchMultiInPattern(itor->second, args, argCountS, argCountE, querySegTags, querySegLens, bitmap);
			}
			else
			{
				Search_AxB_InPattern(itor->second, wArray[0], wArray[1], queryStrTag, queryStr2Tag, bitmap);
			}
			if(bitmap->GetSize() == 0)
			{
				delete bitmap;
				LISTBITMAPS::iterator ifind = bitmaps.find(itor->first);
				if(ifind != bitmaps.end())
				{
					;
				}
				else
				{
					bitmaps[itor->first] = NULL;
				}
			}
			else
			{
				LISTBITMAPS::iterator ifind = bitmaps.find(itor->first);
				if(ifind != bitmaps.end() && bitmaps[itor->first] != NULL)
				{
					bitmaps[itor->first]->Union(bitmap);
					delete bitmap;
				}
				else
				{
					bitmaps[itor->first] = bitmap;
				}
			}
		}
		else if(refbitmaps[itor->first] == NULL)// query end
		{
			continue;
		}
		else if(refbitmaps[itor->first]->GetSize() > 0)
		{
			BitMap* bitmap = new BitMap(itor->second->Count);
			if(INC_TEST_PUSHDOWN)
			{
				if(mCount == 1)
				{
					SearchSingleInPattern_RefMap(itor->second, wArray[0], queryStrTag, bitmap, refbitmaps[itor->first]);
				}
				else if(mCount == 0)//A:B
				{
					SearchMultiInPattern_RefMap(itor->second, args, argCountS, argCountE, querySegTags, querySegLens, bitmap, refbitmaps[itor->first]);
				}
				else
				{
					//Search_AxB_InPattern_Logic(itor->second, wArray[0], wArray[1], rangeSize, range, bitmaps[itor->first]);
				}
				if(bitmap->GetSize() == 0)
				{
					LISTBITMAPS::iterator ifind = bitmaps.find(itor->first);
					if(ifind != bitmaps.end())
					{
						;
					}
					else
					{
						bitmaps[itor->first] = NULL;
					}
				}
				else
				{
					LISTBITMAPS::iterator ifind = bitmaps.find(itor->first);
					if(ifind != bitmaps.end() && bitmaps[itor->first] != NULL)
					{
						bitmaps[itor->first]->Union(bitmap);
						delete bitmap;
					}
					else
					{
						bitmaps[itor->first] = bitmap;
					}
				}
			}
		}
	}
	//search in outliers
	ifind = refbitmaps.find(OUTL_PAT_NAME);
	if(ifind == refbitmaps.end())
	{
		BitMap* bitmap_outlier = new BitMap(m_glbMeta[OUTL_PAT_NAME]->lines);
		bitmap_outlier->SetSize();
		if(mCount == 1)
		{
			GetOutliers_SinglToken(wArray[0], bitmap_outlier);
		}
		else if(mCount == 0)
		{
			GetOutliers_MultiToken(args, argCountS, argCountE, bitmap_outlier);
		}
		else//A*B
		{
			;
		}
		if(bitmap_outlier->GetSize() == 0)
		{
			delete bitmap_outlier;
			LISTBITMAPS::iterator ifind = bitmaps.find(OUTL_PAT_NAME);
			if(ifind != bitmaps.end())
			{
				;
			}
			else
			{
				bitmaps[OUTL_PAT_NAME] = NULL;
			}
		}
		else
		{
			LISTBITMAPS::iterator ifind = bitmaps.find(OUTL_PAT_NAME);
			if(ifind != bitmaps.end() && bitmaps[itor->first] != NULL)
			{
				bitmaps[OUTL_PAT_NAME]->Union(bitmap_outlier);
				delete bitmap_outlier;
			}
			else
			{
				bitmaps[OUTL_PAT_NAME] = bitmap_outlier;
			}
		}
	}
	else if(refbitmaps[OUTL_PAT_NAME] == NULL || refbitmaps[OUTL_PAT_NAME]->GetSize() == 0)
	{
		;//query end
	}
	else if(refbitmaps[OUTL_PAT_NAME]->GetSize() > 0)
	{
		if(INC_TEST_PUSHDOWN)
		{
			LISTBITMAPS::iterator ifind = bitmaps.find(OUTL_PAT_NAME);
			if(ifind == bitmaps.end() || bitmaps[OUTL_PAT_NAME] != NULL)
			{
				bitmaps[OUTL_PAT_NAME] = new BitMap(m_glbMeta[OUTL_PAT_NAME]->lines);
			}
			if(mCount == 1)
			{
				GetOutliers_SinglToken_RefMap(wArray[0], bitmaps[OUTL_PAT_NAME], refbitmaps[OUTL_PAT_NAME]);
			}
			else if(mCount == 0)
			{
				GetOutliers_MultiToken_RefMap(args, argCountS, argCountE, bitmaps[OUTL_PAT_NAME], refbitmaps[OUTL_PAT_NAME]);
			}
			else//A*B
			{
				;
			}
			if(bitmaps[OUTL_PAT_NAME]->GetSize() == 0)
			{
				delete bitmaps[OUTL_PAT_NAME];
				bitmaps[OUTL_PAT_NAME] = NULL;
			}
		}
	}

	//delete
	for(int i=0;i<mCount;i++)
	{
		if (wArray[i])
		{
			delete (wArray[i]);
		}
	}
	if(querySegTags) delete[] querySegTags;
	if(querySegLens) delete[] querySegLens;
	return 0;
}

int LogStoreApi::SearchByLogic_norm_or(char *args[MAX_CMD_ARG_COUNT], int argCountS, int argCountE, OUT LISTBITMAPS& bitmaps)
{
	int temp =argCountS;
	int orLen = strlen(LOGIC_or);
	LISTBITMAPS tempbitmaps;
	bool union_flag = false;
	for(int i=argCountS; i<= argCountE; i++)
	{
		if(strlen(args[i]) == orLen && stricmp(args[i], LOGIC_or) == 0)
		{
			union_flag = true;
			SearchByLogic_norm_RefMap(args, temp, i-2, tempbitmaps, bitmaps);
			temp = i+2;
			i++;
		}
	}
	SearchByLogic_norm(args, temp, argCountE, bitmaps);
	if(union_flag)//union
	{
		LISTBITMAPS::iterator itor = tempbitmaps.begin();
		LISTBITMAPS::iterator ifind;
		for (; itor != tempbitmaps.end();itor++)
		{
			ifind = bitmaps.find(itor->first);
			if(ifind != bitmaps.end() && bitmaps[itor->first]!= NULL)
			{
				bitmaps[itor->first]->Union(itor->second);
			}
			else		
				bitmaps[itor->first] = itor->second;
		}
	}
	return 0;
}

int LogStoreApi::SearchByLogic_and(char *args[MAX_CMD_ARG_COUNT], int argCountS, int argCountE, OUT LISTBITMAPS& bitmaps)
{
	int flag =0;//0: normal  1: with not
	int temp =argCountS;
	int notLen = strlen(LOGIC_NOT);
	for(int i=argCountS; i<= argCountE; i++)
	{
		if(strlen(args[i]) == notLen && stricmp(args[i], LOGIC_NOT) == 0)
		{
			char queryStr[MAX_PATTERN_SIZE]={'\0'};
			RecombineString(args, 0, i-2, queryStr);
			LISTSESSIONS::iterator ifind = m_sessions.find(queryStr);
			if(ifind == m_sessions.end())
			{
				if(flag == 1)
				{
					SearchByLogic_not(args, temp, i-2, bitmaps);
				}
				else if(temp <= i-2)
				{
					SearchByLogic_norm_or(args, temp, i-2, bitmaps);
				}
				if(INC_TEST_SESSION)//add to cache
				{
					DeepCloneMap(bitmaps, m_sessions[queryStr]);
				}
			}
			else//enable cache 
			{
				DeepCloneMap(m_sessions[queryStr], bitmaps);
			}
			flag =1;
			temp =i+2;
			i++;
		}
	}
	char queryChars[MAX_PATTERN_SIZE]={'\0'};
	RecombineString(args, 0, argCountE, queryChars);
	string queryStr(queryChars);
	LISTSESSIONS::iterator ifind = m_sessions.find(queryStr);
	if(ifind == m_sessions.end())
	{
		//COW (undo)
		if(flag == 1)
		{
			SearchByLogic_not(args, temp, argCountE, bitmaps);
		}
		else
		{
			SearchByLogic_norm_or(args, temp, argCountE, bitmaps);
		}
		if(INC_TEST_SESSION)//add to cache
		{
			DeepCloneMap(bitmaps, m_sessions[queryStr]);
		}
	}
	else
	{
		DeepCloneMap(m_sessions[queryStr], bitmaps);
	}
	return 0;
}

int LogStoreApi::SearchByLogic_OR(char *args[MAX_CMD_ARG_COUNT], int argCountS, int argCountE, OUT LISTBITMAPS& bitmaps)
{
	int temp =argCountS;
	int andLen = strlen(LOGIC_AND);
	for(int i=argCountS; i<= argCountE; i++)
	{
		if(strlen(args[i]) == andLen && stricmp(args[i], LOGIC_AND) == 0)
		{
			SearchByLogic_and(args, temp, i-2, bitmaps);
			temp = i+2;
			i++;
		}
	}
	SearchByLogic_and(args, temp, argCountE, bitmaps);
	return 0;
}

int LogStoreApi::SearchByLogic(char *args[MAX_CMD_ARG_COUNT], int argCount, OUT LISTBITMAPS& bitmaps)
{
	SearchByLogic_OR(args, 0, argCount-1, bitmaps);
	/*
	int temp =0;
	int orLen = strlen(LOGIC_OR);
	LISTBITMAPS::iterator itor;
	LISTBITMAPS::iterator ifind;
	for(int i=0; i< argCount; i++)
	{
		if(strlen(args[i]) == orLen && stricmp(args[i], LOGIC_OR) == 0)
		{
			LISTBITMAPS tempbitmaps;
			SearchByLogic_OR(args, temp, i-2, tempbitmaps);
			itor = tempbitmaps.begin();
			for (; itor != tempbitmaps.end();itor++)
			{
				ifind = bitmaps.find(itor->first);
				if(ifind != bitmaps.end())
					bitmaps[itor->first]->Union(itor->second);
				else		
					bitmaps[itor->first] = itor->second;
			}
			temp = i+2;
			i++;
		}
	}
	LISTBITMAPS tempbitmaps;
	SearchByLogic_OR(args, temp, argCount-1, tempbitmaps);
	itor = tempbitmaps.begin();
	for (; itor != tempbitmaps.end();itor++)
	{
		ifind = bitmaps.find(itor->first);
		if(ifind != bitmaps.end())
			bitmaps[itor->first]->Union(itor->second);
		else		
			bitmaps[itor->first] = itor->second;
	}*/
	return 0;
}

int LogStoreApi::SearchByWildcard_Token(char *args[MAX_CMD_ARG_COUNT], int argCount, int matNum)
{
	LISTBITMAPS bitmaps;
	//only one segment
	if(argCount == 1)
	{
		LISTSESSIONS::iterator ifind = m_sessions.find(args[0]);
		if(ifind == m_sessions.end())
		{
			timeval tt1 = ___StatTime_Start();
			Search_SingleSegment(args[0], bitmaps);
			RunStatus.SearchPatternTime = ___StatTime_End(tt1);
			timeval tt2 = ___StatTime_Start();
			BitMap* bitmap_outlier = new BitMap(m_glbMeta[OUTL_PAT_NAME]->lines);
			bitmap_outlier->SetSize();
			GetOutliers_SinglToken(args[0], bitmap_outlier);
			bitmaps[OUTL_PAT_NAME] = bitmap_outlier;
			RunStatus.SearchOutlierTime = ___StatTime_End(tt2);
			RunStatus.SearchTotalTime = RunStatus.SearchPatternTime + RunStatus.SearchOutlierTime;
			SyslogPerf("It takes %lfs to single query.\n",RunStatus.SearchPatternTime);
			SyslogPerf("It takes %lfs to single outliers query.\n",RunStatus.SearchOutlierTime);
			//session
			if(INC_TEST_SESSION)
			{
				m_sessions[args[0]] = bitmaps;
			}
		}
		else
		{
			timeval tt1 = ___StatTime_Start();
			bitmaps = m_sessions[args[0]];
			RunStatus.SearchPatternTime = ___StatTime_End(tt1);
			RunStatus.SearchOutlierTime = 0;
			RunStatus.SearchTotalTime = RunStatus.SearchPatternTime + RunStatus.SearchOutlierTime;
			SyslogPerf("It takes %lfs to single query with session cache(cur: %d items).\n",RunStatus.SearchTotalTime, m_sessions.size());
		}
	}
	else//multi segs, must align from the second segment
	{
		int flag = IsSearchWithLogic(args, argCount);
		if(flag == 0)
		{
			//rebuild querystring
			char queryChars[MAX_PATTERN_SIZE]={'\0'};
			RecombineString(args, 0, argCount-1, queryChars);
			string queryStr(queryChars);
			//searching
			LISTSESSIONS::iterator ifind = m_sessions.find(queryStr);
			if(ifind == m_sessions.end())
			{
				timeval tt1 = ___StatTime_Start();
				Search_MultiSegments(args, argCount, bitmaps);
				RunStatus.SearchPatternTime = ___StatTime_End(tt1);
				timeval tt2 = ___StatTime_Start();
				BitMap* bitmap_outlier = new BitMap(m_glbMeta[OUTL_PAT_NAME]->lines);
				bitmap_outlier->SetSize();
				GetOutliers_MultiToken(args, 0, argCount-1, bitmap_outlier);
				bitmaps[OUTL_PAT_NAME] = bitmap_outlier;
				RunStatus.SearchOutlierTime = ___StatTime_End(tt2);
				RunStatus.SearchTotalTime = RunStatus.SearchPatternTime + RunStatus.SearchOutlierTime;
				SyslogPerf("It takes %lfs to multi query.\n",RunStatus.SearchPatternTime);
				SyslogPerf("It takes %lfs to multi outliers query.\n",RunStatus.SearchOutlierTime);
				//session
				if(INC_TEST_SESSION)
				{
					m_sessions[queryStr] = bitmaps;
				}
			}
			else
			{
				timeval tt1 = ___StatTime_Start();
				bitmaps = m_sessions[queryStr];
				RunStatus.SearchPatternTime = ___StatTime_End(tt1);
				RunStatus.SearchOutlierTime = 0;
				RunStatus.SearchTotalTime = RunStatus.SearchPatternTime + RunStatus.SearchOutlierTime;
				SyslogPerf("It takes %lfs to multi query with session cache(cur: %d items).\n",RunStatus.SearchTotalTime, m_sessions.size());
			}
		}
		else
		{
			timeval tt1 = ___StatTime_Start();
			SearchByLogic(args, argCount, bitmaps);
			RunStatus.SearchTotalTime = ___StatTime_End(tt1);
			SyslogPerf("It takes %lfs to logic query.\n",RunStatus.SearchTotalTime);
		}
		
	}
	
	SysCodeRead("--------- Materialization --------------\n");
	materTime = 0;
	LISTBITMAPS::iterator itor = bitmaps.begin();//match with each main pattern
	int num = 0;
	int matnum = 0;
	timeval tt= ___StatTime_Start();
	//first check outliers, then check pats
	if(bitmaps[OUTL_PAT_NAME] != NULL)
	{
		int entryCnt = bitmaps[OUTL_PAT_NAME]->GetSize();
		RunStatus.SearchOutliersNum = entryCnt;
		SysCodeRead("%s: entryCnt: %d.\n", FormatVarName(OUTL_PAT_NAME), entryCnt);
		matnum = entryCnt;
		num += entryCnt;
	}
	for (; itor != bitmaps.end();itor++)
	{
		if(itor ->second != NULL && itor->first != OUTL_PAT_NAME)
		{
			int entryCnt =0;
			if(itor->second->BeSizeFul())
			{
				entryCnt = m_patterns[itor->first]->Count;
			}
			else
			{
				entryCnt = itor->second->GetSize();
			}
			SysCodeRead("%s: entryCnt: %d.\n", FormatVarName(itor->first), entryCnt);
			if(matNum - matnum > 0)
			{
				matnum += Materialization(itor->first, itor->second, entryCnt, matNum - matnum);
			}
			num += entryCnt;
		}
	}
	double  timem = ___StatTime_End(tt);
	RunStatus.SearchTotalEntriesNum = num;
	if(num > 0)
	{
		SysCodeRead("%s: Total query num: %d\n",FileName.c_str(), num);
		SyslogPerf("It takes %lfs (%lfs) to Materialization(%d).\n",timem, materTime, matnum);
	}
	RunStatus.MaterializFulTime = timem;
	RunStatus.MaterializAlgTime = materTime;
	//output outliers
	if(bitmaps[OUTL_PAT_NAME] != NULL)
	{
		MaterializOutlier(bitmaps[OUTL_PAT_NAME], bitmaps[OUTL_PAT_NAME]->GetSize(), matNum);
	}

	return matnum;
}


//https://blog.csdn.net/yangbingzhou/article/details/51352648
int LogStoreApi::SearchByReg(const char *regPattern)
{
	
	return 0;
}

char* LogStoreApi::FormatVarName(int varName)
{
	char sName[128]={'\0'};
	if(varName <= 15)
		sprintf(sName, "%d",varName);
	else
	{
		int e = varName >>16;
		int v = (varName >>8) & 0xFF;
		int s = (varName >>4) & 0x0F;
		int t = varName & 0x0F;
		if(t == VAR_TYPE_SUB)
			sprintf(sName, "%d_%d~%d.%d",e,v,s,t);
		else
			sprintf(sName, "%d_%d.%d",e,v,t);
	}
	return sName;
}