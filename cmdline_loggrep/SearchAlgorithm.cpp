#include "SearchAlgorithm.h"

#include <sys/time.h>

//cache badc and goods to speedup, shoule release mem after quit
map<string, int*> map_badc; 
map<string, int*> map_goods;
map<string, int*> map_next;
//map<string, int> map_queryTag;

void Release_SearchTemp()
{
	for (map<string, int*>::iterator itor = map_badc.begin(); itor != map_badc.end();itor++)
	{
		if(itor->second)
		{
			delete itor->second;
		}
	}
	map_badc.clear();

	for (map<string, int*>::iterator itor = map_goods.begin(); itor != map_goods.end();itor++)
	{
		if(itor->second)
		{
			delete itor->second;
		}
	}
	map_goods.clear();
}

//return 0: false  1:true
int StrIsDelim(const char* queryStr, int segLen)
{
	if(segLen ==1)
	{
		int tokenLen = strlen(TOKEN);
		for(int i=0; i< tokenLen; i++)
		{
			if(TOKEN[i] == queryStr[0])
			{
				return 1;
			}
		}
	}
	return 0;
}

//return tag, see LogStructure.h
short GetStrTag(const char* queryStr, int pLen)
{
	if(StrIsDelim(queryStr, pLen) == 1)
	{
		return TAG_DELIM;
	}
	short tag=0;
	for(int i = 0; i < pLen; ++i) 
    {  
		tag = tag | GetCharTag(queryStr[i]);
    }
	return tag;
}

//return 0: const   1: value  2 delim
int GetSegmentType(IN char* segment, int& segValue)
{
	if(segment == NULL) return -1;
	int ret = SEG_TYPE_CONST;
	int segLen = strlen(segment);
	segValue =0;
	if(StrIsDelim(segment, segLen) == 1)
	{
		ret = SEG_TYPE_DELIM;
	}
	else if(segLen > 3 && segment[0] == TOKEN_VAR_LEFT && segment[1] == TOKEN_VAR_FLAG && segment[segLen-1] == TOKEN_VAR_RIGHT)
	{
		// if var, length is bigger than 3 ,<V1>
		for(int i=2;i<segLen-1; i++)
			segValue = segValue * 10 + (segment[i] - '0');
		ret = SEG_TYPE_VAR;
	}
	return ret;
}

short GetCharTag(const char queryChar)
{
    if(queryChar >= '0' && queryChar <= '9')
	{
		return TAG_0_9;
	}
	if(queryChar >= 'A' && queryChar <= 'F')
	{
		return TAG_A_F;
	}
	if(queryChar >= 'a' && queryChar <= 'f')
	{
		return TAG_a_f;
	}
	if(queryChar >= 'G' && queryChar <= 'Z')
	{
		return TAG_G_Z;
	}
	if(queryChar >= 'g' && queryChar <= 'z')
	{
		return TAG_g_z;
	}
	return TAG_SYMBOL;
}

int IntPadding(int target, int maxLen, char* retStr)
{
	if(maxLen > MAX_DICENTY_LEN)
	{
		printf("Padding overflow!");
		return 0;
	}
	memset(retStr, ' ', maxLen);
	for(int i=maxLen-1; i>=0; i--)
	{
		retStr[i] = target %10 + '0';
		target = target/10;
		if(target ==0) break;
	}
    return 1;
}

int RemovePadding(IN char* paddingedData, int len, OUT char* oriData)
{
	int i=0;
	while(i < len && paddingedData != NULL && paddingedData[i] == ' ')
	{
		i++;
	}
	int newLen = len - i;
	memcpy(oriData, paddingedData +i, newLen);
	return newLen;
}

int atoi(char* buf, int len)
{
	int ret =0;
	int temp =0;
	for(int i=0;i<len;i++)
	{
		temp = buf[i] - '0';
		if(temp >=0 && temp <=9)
		{
			ret = ret * 10 + (buf[i] - '0'); 
		}
	}
	return ret;
}

int BinarySearch(int a[], int value, int n)
{
    int low, high, mid;
    low = 0;
    high = n-1;
    while(low<=high)
    {
        mid = (low+high)/2;
        if(a[mid] ==value)
            return mid;
        if(a[mid] >value)
            high = mid-1;
        if(a[mid] <value)
            low = mid+1;
    }
    return -1;
}

LcsMatch GetLCS_DPoptc(const char* str1, int size1, const char* str2, int size2, bool beReplace)
{
	LcsMatch res;
    if (size1 == 0 || size2 == 0) return res;

    int indices[] = {0, 0};
    int sizes[] = {size1, size2};
    // shift strings to find the longest common substring
    for (int index = 0; index < 2; ++index)
    {
        indices[0] = 0;
        indices[1] = 0;
        // i is reference to the value in array
        int &i = indices[index];
        int size = sizes[index];
        // this is tricky to skip comparing strings both start with 0 for second loop
        i = index;
        for (; i < size; ++i)
        {
            int m = indices[0];
            int n = indices[1];
            int length = 0;
            // with following check to reduce some more comparisons
			if(!beReplace)
            {
				if (size1-m <= res.Len || size2-n <= res.Len)
                	break;
			}
            while(m < size1 && n < size2)
            {
                if (str1[m] != str2[n])
                {
                    length = 0;
                }
                else
                {
                    ++length;
                    if (res.Len < length)
                    {
                        res.Len = length;
						res.Pos1 = m-res.Len+1;
						res.Pos2 = n-res.Len+1;
                    }
					if(beReplace)
            		{
						if (res.Len == length)
                    	{
							res.Len = length;
							res.Pos1 = m-res.Len+1;
							res.Pos2 = n-res.Len+1;
                    	}
					}
                }
                ++m;
                ++n;
            }
        }
    }
    return res;
}

int RecombineString(char* args[], int argCountS, int argCountE, char* queryStr)
{
	int offset=0;
	for(int i=argCountS; i<= argCountE; i++)
	{
		offset = strlen(queryStr);
		memcpy(queryStr + offset, args[i], strlen(args[i]));
	}
	return 1;
}

///////////////KMP///////////////////////////
void InitKmpNext(const char*T, int* &next)
{
	map<string, int*>::iterator itor = map_next.find(T);
	if(itor != map_next.end())
	{
		next = map_next[T];
		return;
	}
	next = new int[MAX_VALUE_LEN];
    int i=0;
    next[0]= -1;
    int j=-1;
	int tLen = strlen(T);
    while (i< tLen-1) 
	{
        if (j== -1 || T[i]==T[j]) 
		{
            i++;
            j++;
			if (T[i] != T[j])
            	next[i]=j;
			else
				next[i] = next[j];

        }
		else
		{
            j=next[j];
        }
    }
}

int KMP(char * S,const char * T, int* next, int queryType)
{
    int i=0;
    int j=0;
	int sLen = strlen(S);
	int tLen = strlen(T);
    while (i< sLen && j< tLen) {
        if (j==-1 || S[i]==T[j]) {
            i++;
            j++;
        }
        else{
            j=next[j];//if not equal, j turned to the pos of next[]
        }
    }
    if (j == tLen) 
	{//matched
		if(queryType == QTYPE_ALIGN_ANY)
		{
			return i-j;
		}
		else if(queryType == QTYPE_ALIGN_RIGHT)
		{
			if(i == sLen) return i-j;
		}
		else if(queryType == QTYPE_ALIGN_LEFT)
		{
			if(i-j == 0) return i-j;
		}
		else if(queryType == QTYPE_ALIGN_FULL)
		{
			//if(sLen == tLen) 
				return i-j;
		}
		else// for dic
		{
			if(sLen == tLen) 
				return i-j;
		}
		return -1;
    }
    return -1;
}

//for first search in file that has different line length
int KMP(char* S, const char* T, OUT BitMap* bitmap, int queryType)
{	
    int i=0;
    int j=0;
	int sLen = strlen(S);
	int tLen = strlen(T);
	int lineNo = 0;
	int offset = 0;
	int *next;
	InitKmpNext(T, next);
	char vars[MAX_LINE_SIZE]={'\0'};
	while(S[i] && i< sLen)
	{
		while(S[i] != '\n' && i< sLen)
		{
			vars[offset++] = S[i];
			i++;
		}
		vars[offset] = '\0';
    	int matchResult = KMP(vars, T, next, queryType);
		if(matchResult >= 0)
		{
			bitmap->Union(lineNo);
		}
		offset = 0;
		lineNo++;
		i++;
	}
    return bitmap->GetSize();
}

//////////////Boyer-Moore//////////////////
void InitBM(const char* pattern, int* &badc, int* &goods)
{
	BuildBadC(pattern, badc);
	BuildGoodS(pattern, goods);
}

void BuildBadC(const char* pattern, int* &badc)  
{  
	map<string, int*>::iterator itor = map_badc.find(pattern);
	if(itor != map_badc.end())
	{
		badc = map_badc[pattern];
		return;
	}
	
    unsigned int i;
	int pLen = strlen(pattern);
	badc = new int[MAX_PATTERN_SIZE];
    for(i = 0; i < MAX_PATTERN_SIZE; ++i)  
    {  
        badc[i] = pLen;  
    }  
    for(i = 0; i < pLen; ++i)  
    {  
        badc[pattern[i]] = pLen - 1 - i;  
    } 
	map_badc[pattern] = badc;
}  

void BuildGoodS(const char *pattern, int* &goods)
{  
	map<string, int*>::iterator itor = map_goods.find(pattern);
	if(itor != map_goods.end())
	{
		goods = map_goods[pattern];
		return;
	}

	goods = new int[MAX_PATTERN_SIZE];
    unsigned int i, j, c;  
	int pLen = strlen(pattern);
    for(i = 0; i < MAX_PATTERN_SIZE; ++i)  
    {  
        goods[i] = pLen;
    }  
  
    goods[pLen - 1] = 1;   
    for(i = pLen -1, c = 0; i != 0; --i)  
    {  
        for(j = 0; j < i; ++j)  
        {  
            if(memcmp(pattern + i, pattern + j, (pLen - i) * sizeof(char)) == 0)  
            {  
                if(j == 0)  
                {  
                    c = pLen - i;  
                }  
                else  
                {  
                    if(pattern[i - 1] != pattern[j - 1])  
                    {  
                        goods[i - 1] = j - 1;  
                    }  
                }  
            }  
        }  
    }  
      
    for(i = 0; i < pLen - 1; ++i)  
    {  
        if(goods[i] != pLen)  
        {  
            goods[i] = pLen - 1 - goods[i];  
        }  
        else  
        {  
            goods[i] = pLen - 1 - i + goods[i];  
  
            if(c != 0 && pLen - 1 - i >= c)  
            {  
                goods[i] -= c;  
            }  
        }  
    }
	map_goods[pattern] = goods;
}

//bm for the same line length, and lineLen = patLen
//flag: align type
int BM_Fixed_Align(char* text, int sIdx, int tLen, const char* pattern, BitMap* bitmap, int lineLen, int flag)
{
	bool en_union_skip = lineLen > MAX_UNION_LINELEN && bitmap->GetSize() > MAX_UNION_BITMAPS;
	if(flag == QTYPE_ALIGN_FULL || flag == QTYPE_ALIGN_RIGHT)
	{
		return BM_Fixed_AlignR(text, sIdx, tLen, pattern, bitmap, lineLen, en_union_skip);
	}
	else
	{
		if(strlen(pattern) == 1 && pattern[0] == ' ')
		{
			return Fixed_AlignL_For_Empty(text, sIdx, tLen, bitmap, lineLen, en_union_skip);
		}
		return BM_Fixed_AlignL(text, sIdx, tLen, pattern, bitmap, lineLen, en_union_skip);
	}
}

int BM_Fixed_MutiFul(char* text, const char* patterns, int patCnt, BitMap* bitmap, int lineLen)
{
	//if bitmap is already a universal set, return directly
	if(bitmap->BeSizeFul())
	{
		return DEF_BITMAP_FULL;
	}
	int sLen = strlen(text);
	int pLen_1 = lineLen-1;
    int i = pLen_1; 
	int j = pLen_1;
	int k = pLen_1;
    while(j < sLen)
    { 
		for(int patNum =0; patNum < patCnt; patNum++)
		{
    		//inverse
    		while((i != 0) && (patterns[patNum * MAX_DICENTY_LEN + i] == text[j]))
    		{  
        		--i;
        		--j;
    		}
    		//matched  
    		if(i == 0 && patterns[patNum * MAX_DICENTY_LEN + i] == text[j])
    		{
				bitmap->Union(k/lineLen);
				break;
    		}
			j = k;
			i = pLen_1;
		}
		k += lineLen;
  		j = k;
    	i = pLen_1;
    }
	return bitmap->GetSize();  
}

int BM_Fixed_AlignR(char* text, int sIdx, int tLen, const char* pattern, BitMap* bitmap, int lineLen, bool enableUnionSkip)
{
	//if bitmap is already a universal set, return directly
	if(bitmap->BeSizeFul())
	{
		return DEF_BITMAP_FULL;
	}
	int pLen_1 = strlen(pattern)-1;
	if(pLen_1 + 1 > lineLen) return bitmap->GetSize();
    int i = pLen_1; 
	int j = lineLen - 1;
	int k = lineLen - 1;
	int lineNo = sIdx;
	bool enable = true;
    while(j < tLen)
    {  
		enable = enableUnionSkip? bitmap->GetValue(lineNo) == 0 : true;
		if(enable)
		{ 
			//inverse 
			while((i != 0) && (pattern[i] == text[j]))
			{  
				--i;
				--j;
			}
			//matched
			if(i == 0 && pattern[i] == text[j])  
			{  
				bitmap->Union(lineNo);
			}
		}
		//directly jump to the next line end
		k += lineLen;
  		j = k;
    	i = pLen_1;
		lineNo++;
    }
	return bitmap->GetSize();  
}
int BM_Fixed_AlignL(char* text, int sIdx, int tLen, const char* pattern, BitMap* bitmap, int lineLen, bool enableUnionSkip)
{
	if(bitmap->BeSizeFul())
	{
		return DEF_BITMAP_FULL;
	}
	int pLen_1 = strlen(pattern)-1;
	if(pLen_1 + 1 > lineLen) return bitmap->GetSize();
    int i = pLen_1; 
	int j = pLen_1;
	int k = pLen_1;
	int lineNo = sIdx;
	bool enable = true;
	int* badc;
	int* goods;
	InitBM(pattern, badc, goods);
    while(j < tLen)
    {  
		enable = enableUnionSkip? bitmap->GetValue(lineNo) == 0 : true;
		if(enable){
			while(j <tLen && text[j-pLen_1] == ' ')
			{
				j += (goods[i] > badc[text[j]] ? goods[i] : badc[text[j]]);
			}
			if((j-pLen_1)%lineLen == 0 || (j <tLen && text[j-pLen_1-1]== ' '))
			{
				//inverse 
				while((i != 0) && (pattern[i] == text[j]))
				{  
					--i;
					--j;
				}
				//matched  
				if(i == 0 && pattern[i] == text[j])
				{  
					bitmap->Union(lineNo);
				}
			}
		}
		//directly jump to the next line end
		k += lineLen;
		j = k;
		i = pLen_1;
		lineNo++;
    }
	return bitmap->GetSize();  
}
int Fixed_AlignL_For_Empty(char* text, int sIdx, int tLen, BitMap* bitmap, int lineLen, bool enableUnionSkip)
{
	if(bitmap->BeSizeFul())
	{
		return DEF_BITMAP_FULL;
	}
	int pLen_1 = 0;
	int j = pLen_1;
	int lineNo = sIdx;
	bool enable = true;
    while(j < tLen)
    {  
		//matched  
		if(' ' == (text+j)[lineLen-1])
		{  
			bitmap->Union(lineNo);
		}
		//directly jump to the next line end
		j += lineLen;
		lineNo++;
    }
	return bitmap->GetSize();  
}
//bm for the same line length
int BM_Fixed_Anypos(char* text, int sIdx, int tLen, const char* pattern, BitMap* bitmap, int lineLen)
{
	if(bitmap->BeSizeFul())
	{
		return DEF_BITMAP_FULL;
	}
	bool enableUnionSkip = false;
	if(lineLen > MAX_UNION_LINELEN && bitmap->GetSize() >= MAX_UNION_BITMAPS)
	{
		enableUnionSkip = true;
	}

	int pLen_1 = strlen(pattern) - 1; 
	if(pLen_1 + 1 > lineLen) return bitmap->GetSize();
    int i = pLen_1;
	int j = i;
	int k = 0;
	int tempPos = 0;
	int lineNo =sIdx;
	bool enable = true;
	int* badc;
	int* goods;
	InitBM(pattern, badc, goods);
    while(j < tLen)
    {  
		enable = enableUnionSkip? bitmap->GetValue(lineNo) == 0 : true;
		if(enable){
			while(1)
			{ 
				while((i != 0) && (pattern[i] == text[j]))  
				{  
					--i;  
					--j;  
				}
				//if matched, jump to the next line, no need to do: j += g_goods[0]; 
				if(i == 0 && pattern[i] == text[j])  
				{  
					bitmap->Union(lineNo);
					break;
				}  
				else  
				{
					tempPos = goods[i] > badc[text[j]] ? goods[i] : badc[text[j]];  
					if(j + tempPos < k + lineLen)
					{
						j += tempPos;
					}
					else//illegal to cross line for matching, directly jump to the next line
					{
						break;
					}
				}
				i = pLen_1;
			}
		}
		k+= lineLen;
		j = k + pLen_1;
		i = pLen_1;
		lineNo++;
    }

	return bitmap->GetSize();  
}

//bm for the diff line length
int BM_Diff(char* text, int sIdx, int tLen, const char* pattern, BitMap* bitmap, int minLineLen, int maxLineLen)
{
	//if bitmap is already a universal set, return directly
	if(bitmap->BeSizeFul())
	{
		return DEF_BITMAP_FULL;  
	}
	int pLen_1 = strlen(pattern) - 1;  
	int jumpMinLen = pLen_1 < minLineLen -1 ? pLen_1 : minLineLen -1;//Minimize the number of jumps, to prevent from crossing the line
    int i = pLen_1;
	int j = i;
	int k = 0;
	int tempPos = 0;
	int lineNo = sIdx;
	int* badc;
	int* goods;
	InitBM(pattern, badc, goods);
    while(j < tLen)
    {  
        while((i != 0) && (pattern[i] == text[j]))  
        {  
            --i;  
            --j;  
        }
        if(i == 0 && pattern[i] == text[j])  
        {  
			bitmap->Union(lineNo);
			for(int ii= minLineLen-1; ii< maxLineLen;ii++)
			{
				if(text[k+ii]== '\n')//found '\n', lineNo++
				{
					k+= ii+1;
					j = k + jumpMinLen;
					lineNo++;
					break;
				}
			}
        }  
        else  
        {
            tempPos = goods[i] > badc[text[j]] ? goods[i] : badc[text[j]];  
			if(j + tempPos < k + minLineLen-1)//Less than the minimum row length, can be matched normally
			{
				j += tempPos;
			}
			else if(j + tempPos >= k + maxLineLen)//cross lines，illegal!!!
			{
				while(text[j] != '\n')
				{
					j++;
				}
				k = j+1;
				j = k + jumpMinLen;
				lineNo++;
			}
			else//critical region, be careful! bit-by-bit detect '\n'
			{
				j += tempPos;
				for(int ii=0; ii<=tempPos; ii++)
				{
					if(text[j-ii]== '\n')
					{
						k = j-ii + 1;
						j = k + jumpMinLen;
						lineNo++;
						break;
					}
				}
			}
        }
        i = pLen_1;
    }
	return bitmap->GetSize();  
}

//only return the first match
int BM_Once(char* text, const char* pattern, int textLen, int* badc, int* goods)
{
	int pLen_1 = strlen(pattern) -1;
	int i = pLen_1;
    int j = pLen_1;
    while(j < textLen && j >=0)
    {  
        while((i != 0) && (pattern[i] == text[j]))
        {  
            --i;  
            --j;  
        } 
        if(i == 0 && pattern[i] == text[j])  
        {  
			return j;
        }  
        else  
        {
			j += goods[i] > badc[text[j]] ? goods[i] : badc[text[j]]; 
        }
        i = pLen_1;
    }
	return -1;  
}

//bm for the same line length, and lineLen = patLen
//type: align type
int BM_Fixed_Pushdown(char* text, const char* pattern, BitMap* bitmap, int lineLen, int type)
{
	if(bitmap->GetSize() == 0)//if bitmap is empty, return directly
	{
		return 0;
	}
	int sLen = strlen(text);
	if(bitmap->BeSizeFul())//if bitmap is a universal set, then use union
	{
		bitmap->Reset();
		if(type != QTYPE_ALIGN_ANY)
		{
			return BM_Fixed_Align(text, 0, sLen, pattern, bitmap, lineLen, type);
		}
		else
		{	
			return BM_Fixed_Anypos(text, 0, sLen, pattern, bitmap, lineLen);
		}
	}
	int pLen = strlen(pattern);
	if(pLen > lineLen) return bitmap->GetSize();// error length
	int matchResult;
	int bitmapSize = bitmap->GetSize();
	bitmap->ResetSize();//only simple set Index[] size
	if(type == QTYPE_ALIGN_RIGHT || type == QTYPE_ALIGN_FULL)
	{
		for(int i=0;i< bitmapSize;i++)
		{
			matchResult = SeqMatching_AlignRight(text + bitmap->GetIndex(i) * lineLen, lineLen, pattern, pLen);
			if(matchResult > 0)
			{
				bitmap->Inset(bitmap->GetIndex(i));// only set Index[]
			}
			else
			{
				bitmap->Reset(bitmap->GetIndex(i));// only set bitmap as 0
			}
		}
	}
	else if(type == QTYPE_ALIGN_ANY)
	{
		int* badc;
		int* goods;
		InitBM(pattern, badc, goods);
		for(int i=0;i< bitmapSize;i++)
		{
			matchResult = BM_Once(text + bitmap->GetIndex(i) * lineLen, pattern, lineLen, badc, goods);
			if(matchResult >= 0)
			{
				bitmap->Inset(bitmap->GetIndex(i));// only set Index[]
			}
			else
			{
				bitmap->Reset(bitmap->GetIndex(i));// only set bitmap as 0
			}
		}
	}
	else
	{
		//allow <V> contains empty space
		if(pattern[0] == ' ' && strlen(pattern) == 1)
		{
			for(int i=0;i< bitmapSize;i++)
			{
				if((text + bitmap->GetIndex(i+1) * lineLen -1)[0] == ' ')//to the end of the line
				{
					bitmap->Inset(bitmap->GetIndex(i));
				}
				else
				{
					bitmap->Reset(bitmap->GetIndex(i));
				}
			}
		}
		else
		{
			int* badc;
			int* goods;
			InitBM(pattern, badc, goods);
			for(int i=0;i< bitmapSize;i++)
			{
				matchResult = SeqMatching_AlignLeft(text + bitmap->GetIndex(i) * lineLen, lineLen, pattern, pLen, badc, goods);
				if(matchResult > 0)
				{
					bitmap->Inset(bitmap->GetIndex(i));// only set Index[]
				}
				else
				{
					bitmap->Reset(bitmap->GetIndex(i));// only set bitmap as 0
				}
			}
		}
	}
	return bitmap->GetSize();  
}

int BM_Fixed_Pushdown_RefMap(char* text, const char* pattern, BitMap* bitmap, BitMap* refBitmap, int lineLen, int type)
{
	if(refBitmap->GetSize() == 0)//if bitmap is empty, return directly
	{
		return 0;
	}
	int sLen = strlen(text);
	if(refBitmap->BeSizeFul())//if bitmap is a universal set, then use union
	{
		if(type != QTYPE_ALIGN_ANY)
		{
			return BM_Fixed_Align(text, 0, sLen, pattern, bitmap, lineLen, type);
		}
		else
		{	
			return BM_Fixed_Anypos(text, 0, sLen, pattern, bitmap, lineLen);
		}
	}
	else
	{
		int pLen = strlen(pattern);
		if(pLen > lineLen) return 0;// error length
		int matchResult;
		int bitmapSize = refBitmap->GetSize();
		if(type == QTYPE_ALIGN_RIGHT || type == QTYPE_ALIGN_FULL)
		{
			for(int i=0;i< bitmapSize;i++)
			{
				matchResult = SeqMatching_AlignRight(text + refBitmap->GetIndex(i) * lineLen, lineLen, pattern, pLen);
				if(matchResult > 0)
				{
					bitmap->Union(refBitmap->GetIndex(i));
				}
			}
		}
		else if(type == QTYPE_ALIGN_ANY)
		{
			int* badc;
			int* goods;
			InitBM(pattern, badc, goods);
			for(int i=0;i< bitmapSize;i++)
			{
				matchResult = BM_Once(text + refBitmap->GetIndex(i) * lineLen, pattern, lineLen, badc, goods);
				if(matchResult >= 0)
				{
					bitmap->Union(refBitmap->GetIndex(i));
				}
			}
		}
		else
		{
			int* badc;
			int* goods;
			InitBM(pattern, badc, goods);
			for(int i=0;i< bitmapSize;i++)
			{
				matchResult = SeqMatching_AlignLeft(text + refBitmap->GetIndex(i) * lineLen, lineLen, pattern, pLen, badc, goods);
				if(matchResult > 0)
				{
					bitmap->Union(refBitmap->GetIndex(i));
				}
			}
		}
	}
	return bitmap->GetSize();  
}

int BM_Fixed_Pushdown_MutiFul(char* text, const char* pattern, int patCnt, BitMap* bitmap, int lineLen)
{
	if(bitmap->GetSize() == 0)//if bitmap is empty, return directly
	{
		return 0;
	}
	if(bitmap->BeSizeFul())//if bitmap is a universal set, then use union
	{
		bitmap->Reset();
		return BM_Fixed_MutiFul(text, pattern, patCnt, bitmap, lineLen);
	}
	int sLen = strlen(text);
	
	int matchResult;
	int bitmapSize = bitmap->GetSize();
	bitmap->ResetSize();//only simple set size
	for(int i=0;i< bitmapSize;i++)
	{
		matchResult = SeqMatching_MultiFul(text + bitmap->GetIndex(i) * lineLen, lineLen, pattern, patCnt);
		if(matchResult > 0)
		{
			bitmap->Inset(bitmap->GetIndex(i));// only set Index[]
		}
		else
		{
			bitmap->Reset(bitmap->GetIndex(i));// only set bitmap as 0
		}
	}
	return bitmap->GetSize();  
}

int BM_Fixed_Pushdown_MutiFul_RefMap(char* text, const char* pattern, int patCnt, BitMap* bitmap, BitMap* refBitmap, int lineLen)
{
	if(refBitmap->GetSize() == 0)//if bitmap is empty, return directly
	{
		return 0;
	}
	if(refBitmap->BeSizeFul())//if bitmap is a universal set, then use union
	{
		return BM_Fixed_MutiFul(text, pattern, patCnt, bitmap, lineLen);
	}
	int sLen = strlen(text);
	
	int matchResult;
	int bitmapSize = refBitmap->GetSize();
	for(int i=0;i< bitmapSize;i++)
	{
		matchResult = SeqMatching_MultiFul(text + refBitmap->GetIndex(i) * lineLen, lineLen, pattern, patCnt);
		if(matchResult > 0)
		{
			bitmap->Union(refBitmap->GetIndex(i));// only set Index[]
		}
	}
	return bitmap->GetSize();  
}

int BM_Diff_Pushdown(char* text, const char* pattern, BitMap* bitmap, int type)
{
	if(bitmap->GetSize() == 0)//if bitmap is empty, return directly
	{
		return 0;
	}
	if(bitmap->BeSizeFul())//if bitmap is a universal set, then use union
	{
		bitmap->Reset();
		return KMP(text, pattern, bitmap, type);
	}
	int bitmapSize = bitmap->GetSize();
	bitmap->ResetSize();//only simple set Index[] size
	int index=0; int offset=0; int lineIdx = 0;
	int len = strlen(text);
	char* p= text;
	char* vars =new char[MAX_VALUE_LEN];
	int *next;
	InitKmpNext(pattern, next);
	while (*p && p- text< len)
	{
		if(bitmap->GetValue(lineIdx) == 1)
		{
			while(*p != '\n')
			{
				vars[offset++] = *p; 
				p++;
			}
			vars[offset] = '\0';
			//match
			int matchResult = KMP(vars, pattern, next, type);
			if(matchResult >= 0)
			{
				bitmap->Inset(lineIdx);// only set Index[]
			}
			else
			{
				bitmap->Reset(lineIdx);// only set bitmap as 0
			}
			index++;
			offset = 0;
		}
		else
		{
			while(*p && *p != '\n')
			{
				p++;
			}
		}
		lineIdx++;
		p++;
	}
	delete[] vars;
	return bitmap->GetSize();  
}

int BM_Diff_Pushdown_RefMap(char* text, const char* pattern, BitMap* bitmap, BitMap* refBitmap, int type)
{
	if(refBitmap->GetSize() == 0)//if bitmap is empty, return directly
	{
		return 0;
	}
	if(refBitmap->BeSizeFul())//if bitmap is a universal set, then use union
	{
		return KMP(text, pattern, bitmap, type);
	}
	int bitmapSize = refBitmap->GetSize();
	int index=0; int offset=0; int lineIdx = 0;
	int len = strlen(text);
	char* p= text;
	char* vars =new char[MAX_VALUE_LEN];
	int *next;
	InitKmpNext(pattern, next);
	while (*p && (p-text < len))
	{
		if(refBitmap->GetIndex(index) == lineIdx)
		{
			while(*p != '\n' && (p-text < len))
			{
				vars[offset++] = *p; 
				p++;
			}
			vars[offset] = '\0';
			//match
			int matchResult = KMP(vars, pattern, next, type);
			if(matchResult >= 0)
			{
				bitmap->Union(lineIdx);
			}
			index++;
			offset = 0;
		}
		else
		{
			while(*p && *p != '\n')
			{
				p++;
			}
		}
		lineIdx++;
		p++;
	}
	delete[] vars;
	return bitmap->GetSize();  
}

//load vals by bitmap for materializ, achieve efficient skip
//return  0: false    1:true
int GetCvarsByBitmap_Fixed(char* text, int lineLen, BitMap* bitmap, OUT char *vars, int entryCnt, int varsLineLen, bool flag)
{
	int bitmapSize = bitmap->GetSize();
	if(bitmapSize <= 0) return 0;
	int offsetT, offsetV;
	if(flag)
	{
		for(int i=0;i< entryCnt;i++)
		{
			offsetT = bitmap->GetIndex(i) * lineLen;
			offsetV = i * varsLineLen;
			RemovePadding(text + offsetT, lineLen, vars + offsetV);
		}
	}
	else
	{
		for(int i=0;i< entryCnt;i++)
		{
			offsetT = bitmap->GetIndex(i) * lineLen;
			offsetV = i * varsLineLen;
			offsetV += strlen(vars + offsetV);
			RemovePadding(text + offsetT, lineLen, vars + offsetV);
		}
	}
	
	return 1;
}
//return  0: false    1:true
int GetCvars_Fixed(char* text, int lineLen, int lineCnt, OUT char *vars, int varsLineLen, bool flag)
{
	if(lineCnt <= 0) return 0;
	int offsetT, offsetV;
	if(flag)
	{
		for(int i=0;i< lineCnt;i++)
		{
			offsetT = i * lineLen;
			offsetV = i * varsLineLen;
			RemovePadding(text + offsetT, lineLen, vars + offsetV);
		}
	}
	else
	{
		for(int i=0;i< lineCnt;i++)
		{
			offsetT = i * lineLen;
			offsetV = i * varsLineLen;
			offsetV += strlen(vars + offsetV);
			RemovePadding(text + offsetT, lineLen, vars + offsetV);
		}
	}
	return 1;
}

//load vals by bitmap, achieve efficient skip
int GetCvarsByBitmap_Diff(char* text, int minLineLen, BitMap* bitmap, OUT char *vars, int entryCnt, int varsLineLen, bool flag)
{
	int bitmapSize = bitmap->GetSize();
	int len = strlen(text);
	int index=0; int offset=0; int lineIdx = 0;
	char* p= text;int offsetV;
	while (*p && p- text< len)
	{
		if(bitmap->GetValue(lineIdx) == 1)
		{
			offsetV = index * varsLineLen;
			if(!flag) offsetV += strlen(vars + offsetV);
			while(*p && *p != '\n')
			{
				vars[offsetV + offset++] = *p;
				p++;
			}
			index++;
			offset = 0;
			if(index >= entryCnt) break;
		}
		else
		{
			while(*p && *p != '\n')
			{
				p++;
			}
		}
		lineIdx++;
		p++;
	}
	return 0;
}

int GetCvars_Diff(char* text, OUT char *vars, int entryCnt, int varsLineLen, bool flag)
{
	int len = strlen(text);
	int index=0; int offset=0;
	int offsetV = 0;
	if(!flag) offsetV += strlen(vars);
	for (char *p= text;*p && p-text< len; p++)  
	{
        if (*p == '\n')
		{
			offsetV = index * varsLineLen;
			if(!flag) offsetV += strlen(vars + offsetV);
			vars[offsetV + offset] = '\0';
			index++;
			offset = 0;
			if(index >= entryCnt) break;
		}
        else
		{
			vars[offsetV + offset++] = *p; 
		}
	}
	return index;
}

//return  0: false    1:true
//**************
//*******
int SeqMatching_AlignLeft(const char * S, int sLen, const char * T, int tLen, int* badc, int* goods)
{
	if(tLen > sLen) return 0;
	int i, j;
    i = j = tLen -1;  
    while(j < sLen)
    {  
		while(S[j-tLen+1] == ' ')
		{
			j += goods[i] > badc[S[j]] ? goods[i] : badc[S[j]];
		}
		if(j-tLen+1 == 0 || S[j-tLen]== ' ')
		{
       		while((i != 0) && (T[i] == S[j]))
        	{  
            	--i;
            	--j;  
        	}
        	if(i == 0 && T[i] == S[j])
        	{  
				return 1;
        	}
		}
        return 0;
    }
}
int SeqMatching_AlignLeft(const char * S, int sLen, const char * T, int tLen)
{
	if(sLen < tLen)
	{
		return 0;
	}
	//matching from end to start
	for(int i= tLen -1 ; i>= 0; i--)
	{
		if(S[i] != T[i])
		{
			return 0;
		}
	}
	return 1;
}
//**************
//       *******
int SeqMatching_AlignRight(const char * S, int sLen, const char * T, int tLen)
{
	if(sLen < tLen)
	{
		return 0;
	}
	//matching from end to start
	for(int i= 1 ; i<= tLen; i++)
	{
		if(S[sLen-i] != T[tLen-i])
		{
			return 0;
		}
	}
	return 1;
}
//return  0: false    1:true
int SeqMatching_MultiFul(const char *S, int sLen, const char *T, int tCnt)
{
	int i;
	for(int patNum =0; patNum < tCnt; patNum++)
	{
		for(i= sLen-1; i>= 0; i--)
		{
			if(S[i] != (T + patNum * MAX_DICENTY_LEN)[i])
			{
				break;
			}
		}
    	if(i == -1)
    	{  
			return 1;
    	}
	}
	return 0;
}

//for A*B return  0: false    1:true
int BMwildcard_AxB(char *text, int lineCnt, int lineLen, const char *A, const char *B, BitMap* bitmap)
{
	int* badc_a; int* badc_b;
	int* goods_a; int* goods_b;
	InitBM(A, badc_a, goods_a);
	InitBM(B, badc_b, goods_b);
	int aLen = strlen(A);
	int bLen = strlen(B);
	int offset =0;
	if(bitmap->BeSizeFul())
	{
		return DEF_BITMAP_FULL;
	}
	for(int i =0; i < lineCnt; i++)
	{
		int matchResult = BM_Once(text + i * lineLen, A, lineLen, badc_a, goods_a);
		if(matchResult >=0)
		{
			offset = i * lineLen + matchResult + aLen;
			matchResult = BM_Once(text + offset, B, lineLen, badc_b, goods_b);
		}
    	if(matchResult >= 0)
    	{  
			bitmap->Union(i);
    	}
	}
	return bitmap->GetSize();
}

int SeqMatching_BothSide(const char * S, int sLen, const char * T, int tLen)
{
	int ret = SeqMatching_AlignLeft(S, sLen, T, tLen);
	if(ret == 0)
	{
		ret = SeqMatching_AlignRight(S, sLen, T, tLen);
	}
	return ret;
}

//for outlier, using bm
int QueryInStrArray_BM(char** targetStr, int lineCount, char *queryStr, BitMap* bitmap)
{
	if(bitmap == NULL || bitmap->GetSize() == 0) return 0;
	int* badc;
	int* goods;
	InitBM(queryStr, badc, goods);
	int matchResult;
	int bitmapIndex =0;
	if(bitmap->BeSizeFul())
	{
		bitmap->Reset();
		for(int i=0; i< lineCount; i++)
		{
			matchResult = BM_Once(targetStr[i], queryStr, strlen(targetStr[i]), badc, goods);
			if(matchResult >= 0)
			{
				bitmap->Union(i);
			}
		}
	}
	else
	{
		int bitmapSize = bitmap->GetSize();
		bitmap->ResetSize();
		for(int i=0;i< bitmapSize;i++)
		{
			int targetlen = strlen(targetStr[bitmap->GetIndex(i)]);
			matchResult = BM_Once(targetStr[bitmap->GetIndex(i)], queryStr, targetlen, badc, goods);
			if(matchResult >= 0)
			{
				bitmap->Inset(bitmap->GetIndex(i));
			}
			else
			{
				bitmap->Reset(bitmap->GetIndex(i));
			}
		}
	}
	return bitmap->GetSize();
}

int QueryInStrArray_BM_RefMap(char** targetStr, int lineCount, char *queryStr, BitMap* bitmap, BitMap* refBitmap)
{
	if(refBitmap == NULL || refBitmap->GetSize() == 0) return 0;
	int* badc;
	int* goods;
	InitBM(queryStr, badc, goods);
	int matchResult;
	int bitmapIndex =0;
	
	int bitmapSize = refBitmap->GetSize();
	for(int i=0;i< bitmapSize;i++)
	{
		matchResult = BM_Once(targetStr[refBitmap->GetIndex(i)], queryStr, strlen(targetStr[refBitmap->GetIndex(i)]), badc, goods);
		if(matchResult >= 0)
		{
			bitmap->Union(refBitmap->GetIndex(i));
		}
	}
	return bitmap->GetSize();
}

int QueryInStrArray_BM_Reverse(char** targetStr, int lineCount, char *queryStr, BitMap* bitmap)
{
	if(bitmap == NULL || bitmap->GetSize() == 0) return 0;
	int* badc;
	int* goods;
	InitBM(queryStr, badc, goods);
	int matchResult;
	int bitmapIndex =0;
	if(bitmap->BeSizeFul())
	{
		bitmap->Reset();
		for(int i=0; i< lineCount; i++)
		{
			matchResult = BM_Once(targetStr[i], queryStr, strlen(targetStr[i]), badc, goods);
			if(matchResult < 0)
			{
				bitmap->Union(i);
			}
		}
	}
	else
	{
		int bitmapSize = bitmap->GetSize();
		bitmap->ResetSize();
		for(int i=0;i< bitmapSize;i++)
		{
			matchResult = BM_Once(targetStr[bitmap->GetIndex(i)], queryStr, strlen(targetStr[bitmap->GetIndex(i)]), badc, goods);
			if(matchResult < 0)
			{
				bitmap->Inset(bitmap->GetIndex(i));
			}
			else
			{
				bitmap->Reset(bitmap->GetIndex(i));
			}
		}
	}
	return bitmap->GetSize();
}

int QueryInStrArray_BM_Reverse_RefMap(char** targetStr, int lineCount, char *queryStr, BitMap* bitmap, BitMap* refBitmap)
{
	if(refBitmap == NULL || refBitmap->GetSize() == 0) return 0;
	int* badc;
	int* goods;
	InitBM(queryStr, badc, goods);
	int matchResult;
	int bitmapIndex =0;
	
	int bitmapSize = refBitmap->GetSize();
	for(int i=0;i< bitmapSize;i++)
	{
		matchResult = BM_Once(targetStr[refBitmap->GetIndex(i)], queryStr, strlen(targetStr[refBitmap->GetIndex(i)]), badc, goods);
		if(matchResult < 0)
		{
			bitmap->Union(refBitmap->GetIndex(i));
		}
	}
	return bitmap->GetSize();
}

//////////////////////////////////C Regexex////////////////
//for outlier, using wildcard
int QueryInStrArray_CReg(char** targetStr, int lineCount, const char *queryStr, BitMap* bitmap)
{
	if(bitmap == NULL || bitmap->GetSize() == 0) return 0;
	regex_t reg;
	size_t nmatch = 1;
    regmatch_t pm;
	//regcomp(&reg,queryStr,REG_EXTENDED| REG_NEWLINE | REG_NOSUB);
	//need to add REG_NEWLINE
	int initReg = regcomp(&reg,queryStr,REG_EXTENDED | REG_NOSUB);
	if(initReg != 0)
	{
		return -1;
	}
	int matchResult;
	if(bitmap->BeSizeFul())
	{
		bitmap->Reset();
		for(int i=0;i<lineCount;i++)
		{
			matchResult = regexec(&reg, targetStr[i], nmatch, &pm, REG_NOTBOL);
			if(matchResult == REG_NOERROR)
			{
				bitmap->Union(i);
			}
		}
	}
	else
	{
		int bitmapSize = bitmap->GetSize();
		bitmap->ResetSize();
		for(int i=0;i< bitmapSize;i++)
		{
			matchResult = regexec(&reg, targetStr[bitmap->GetIndex(i)], nmatch, &pm, REG_NOTBOL);
			if(matchResult == REG_NOERROR)
			{
				bitmap->Inset(bitmap->GetIndex(i));
			}
			else
			{
				bitmap->Reset(bitmap->GetIndex(i));
			}
		}
	}
	regfree(&reg); 
	return bitmap->GetSize();
}

int QueryInStrArray_CReg_RefMap(char** targetStr, int lineCount, const char *queryStr, BitMap* bitmap, BitMap* refBitmap)
{
	if(refBitmap == NULL || refBitmap->GetSize() == 0) return 0;
	regex_t reg;
	size_t nmatch = 1;
    regmatch_t pm;
	//regcomp(&reg,queryStr,REG_EXTENDED| REG_NEWLINE | REG_NOSUB);
	//need to add REG_NEWLINE
	int initReg = regcomp(&reg,queryStr,REG_EXTENDED | REG_NOSUB);
	if(initReg != 0)
	{
		return -1;
	}
	int matchResult;
	
	int bitmapSize = refBitmap->GetSize();
	for(int i=0;i< bitmapSize;i++)
	{
		matchResult = regexec(&reg, targetStr[refBitmap->GetIndex(i)], nmatch, &pm, REG_NOTBOL);
		if(matchResult == REG_NOERROR)
		{
			bitmap->Union(refBitmap->GetIndex(i));
		}
	}
	regfree(&reg); 
	return bitmap->GetSize();
}

int QueryInStrArray_CReg_Reverse(char** targetStr, int lineCount, const char *queryStr, BitMap* bitmap)
{
	if(bitmap == NULL || bitmap->GetSize() == 0) return 0;
	regex_t reg;
	size_t nmatch = 1;
    regmatch_t pm;
	//regcomp(&reg,queryStr,REG_EXTENDED| REG_NEWLINE | REG_NOSUB);
	//need to add REG_NEWLINE
	int initReg = regcomp(&reg,queryStr,REG_EXTENDED | REG_NOSUB);
	if(initReg != 0)
	{
		return -1;
	}
	int matchResult;
	if(bitmap->BeSizeFul())
	{
		bitmap->Reset();
		for(int i=0;i<lineCount;i++)
		{
			matchResult = regexec(&reg, targetStr[i], nmatch, &pm, REG_NOTBOL);
			if(matchResult != REG_NOERROR)
			{
				bitmap->Union(i);
			}
		}
	}
	else
	{
		int bitmapSize = bitmap->GetSize();
		bitmap->ResetSize();
		for(int i=0;i< bitmapSize;i++)
		{
			matchResult = regexec(&reg, targetStr[bitmap->GetIndex(i)], nmatch, &pm, REG_NOTBOL);
			if(matchResult != REG_NOERROR)
			{
				bitmap->Inset(bitmap->GetIndex(i));
			}
			else
			{
				bitmap->Reset(bitmap->GetIndex(i));
			}
		}
	}
	regfree(&reg); 
	return bitmap->GetSize();
}

int QueryInStrArray_CReg_Reverse_RefMap(char** targetStr, int lineCount, const char *queryStr, BitMap* bitmap, BitMap* refBitmap)
{
	if(refBitmap == NULL || refBitmap->GetSize() == 0) return 0;
	regex_t reg;
	size_t nmatch = 1;
    regmatch_t pm;
	//regcomp(&reg,queryStr,REG_EXTENDED| REG_NEWLINE | REG_NOSUB);
	//need to add REG_NEWLINE
	int initReg = regcomp(&reg,queryStr,REG_EXTENDED | REG_NOSUB);
	if(initReg != 0)
	{
		return -1;
	}
	int matchResult;

	int bitmapSize = refBitmap->GetSize();
	for(int i=0;i< bitmapSize;i++)
	{
		matchResult = regexec(&reg, targetStr[refBitmap->GetIndex(i)], nmatch, &pm, REG_NOTBOL);
		if(matchResult != REG_NOERROR)
		{
			bitmap->Union(refBitmap->GetIndex(i));
		}
	}
	regfree(&reg); 
	return bitmap->GetSize();
}

//return   1:ok
int QueryInStr_CReg(const char* text, const char *regPattern)
{
	regex_t reg;
	size_t nmatch = 1;
    regmatch_t pm;
	int ret =0;
	int initReg = regcomp(&reg,regPattern,REG_EXTENDED | REG_NOSUB);
	if(initReg != 0)
	{
		return -1;
	}
	int	matchResult = regexec(&reg, text, nmatch, &pm, REG_NOTBOL);
	if(matchResult == REG_NOERROR)
	{
		ret = 1;
	}
	regfree(&reg);
	return ret;
}

///////////////////////SubPatternMatch//////////////////////
//return: matched length
int MatchInSubpatVar_Forward(int strTag_mark, int maxlen_mark, const char* source, int souLen, int& souIndex)
{
	int count =0;
	if(souIndex == souLen)//allow empty space in vars
	{
		return count;
	}
	int	thisChar_mark = GetCharTag(source[souIndex]);
	while((thisChar_mark & strTag_mark) == thisChar_mark)
	{
		count++;
		souIndex++;
		if(souIndex == souLen || count == maxlen_mark)//normal end
		{
			break;
		}
		thisChar_mark = GetCharTag(source[souIndex]);
	}
	if(souIndex == souLen || count == maxlen_mark)
	{
		return count;
	}
	return -1;
}

int MatchInSubpatVar_Backward(int strTag_mark, int maxlen_mark, const char* source, int& souIndex)
{
	int count =0;
	int	thisChar_mark = GetCharTag(source[souIndex]);
	while((thisChar_mark & strTag_mark) == thisChar_mark)
	{
		count++;
		souIndex--;
		if(souIndex < 0 || count == maxlen_mark)//normal end
		{
			break;
		}
		thisChar_mark = GetCharTag(source[souIndex]);
	}
	if(souIndex < 0 || count == maxlen_mark)
	{
		return count;
	}
	return -1;
}

//return 0 miss   1 both end   2: only sou end  3 only pat end
int MatchInSubPatConst_Forward(const char* patConst, int patLen, const char* source, int souLen, int& souIndex)
{
	int patIndex = 0;
	while(patIndex < patLen && souIndex < souLen)
	{
		if(patConst[patIndex] == source[souIndex])
		{
			patIndex++;
			souIndex++;
		}
		else
		{
			return 0;
		}
	}
	if(souIndex == souLen)
	{
		if(patIndex == patLen) return 1;
		return 2;
	} 
	return 3;
}
//return 0 miss   1 both end   2: only sou end  3 only pat end
int MatchInSubPatConst_Backward(const char* patConst, int patLen, const char* source, int& souIndex)
{
	int patIndex = patLen-1;
	while(patIndex >= 0 && souIndex >= 0)
	{
		if(patConst[patIndex] == source[souIndex])
		{
			patIndex--;
			souIndex--;
		}
		else
		{
			return 0;
		}
	}
	if(souIndex < 0)
	{
		if(patIndex < 0) return 1;
		return 2;
	} 
	return 3;
}

//return: 0: miss match  1: matched on pat  2: matched on vars
int MatchInSubPatConst_WithVar(SubPattern* pat, int curIndex, const char* source, int souLen, OUT RegMatrix* regResult)
{
	int patLen = strlen(pat->SubSegment[curIndex]);
	LcsMatch lcs= GetLCS_DPoptc(source, souLen, pat->SubSegment[curIndex], patLen);
	if(lcs.Len == 0)// not matched
	{
		return 0;
	}
	else if(lcs.Len == souLen)//whole matched
	{
		return 1;
	}
	int rst=0;
	if(lcs.Pos1 == 0 && (lcs.Pos2 + lcs.Len == patLen))//matched the right
	{
		if(curIndex == pat->SegSize-1) return 0;
		rst = SubPatMatch_0_J(pat, curIndex+1, pat->SegSize-1, source, souLen, regResult, lcs.Len);
		if(rst > 0)
		{
			return 2;
		}
	}
	else if((lcs.Pos1 + lcs.Len == souLen) && lcs.Pos2 == 0)//matched the left
	{
		if(curIndex == 0) return 0;
		rst = SubPatMatch_N_I(pat, curIndex-1, 0, source, souLen - lcs.Len, regResult);
		if(rst > 0)
		{
			return 3;
		}
	}
	else if(lcs.Len == patLen && lcs.Pos1 !=0 && (lcs.Pos1 + lcs.Len != souLen))//matched the both side
	{
		if(curIndex != 0 && curIndex != pat->SegSize-1)
		{
			rst =SubPatMatch_N_I(pat, curIndex-1, 0, source, lcs.Pos1, regResult);
			if(rst > 0)
			{
				rst =SubPatMatch_0_J(pat, curIndex+1, pat->SegSize-1, source, souLen, regResult, lcs.Pos1 + lcs.Len);
				if(rst > 0)
				{
					return 4;
				}
			}
		}
	}
	return 0;
}

//as to varied length vars, return <=0: miss , count: matched
int MatchInSubpatVvarWithTail(int strTag_mark, int maxlen_mark, const char* patConst, const char* source, int souLen, int& souIndex)
{
	int patLen = strlen(patConst);
	LcsMatch lcs= GetLCS_DPoptc(source + souIndex, souLen-souIndex, patConst, patLen);
	if(lcs.Len != patLen) return -1;
	int count = MatchInSubpatVar_Forward(strTag_mark, maxlen_mark, source, souIndex + lcs.Pos1, souIndex);
	if(count < 0) return -2;
	souIndex = souIndex + patLen;
	return count;

}

int MatchInSubpatVvarWithHead(int strTag_mark, int maxlen_mark, const char* patConst, const char* source, int& souIndex)
{
	int patLen = strlen(patConst);
	LcsMatch lcs= GetLCS_DPoptc(source, souIndex, patConst, patLen, true);
	//printf("souIndex2: %d %d\n", souIndex, lcs.Pos1);
	if(lcs.Len != patLen) return -1;
	int souLen = souIndex + 1;
	souIndex = lcs.Pos1 + patLen;
	int count = MatchInSubpatVar_Forward(strTag_mark, maxlen_mark, source, souLen, souIndex);
	//printf("souIndex3: %d souLen: %d\n", souIndex, souLen);
	if(count < 0) return -2;
	souIndex = lcs.Pos1 -1;
	return count;

}

//match from start to end, return: <0: miss, 1: full match, 2: partial match  9: only in pat
int SubPatMatch_0_J(SubPattern* pat, int sPos, int ePos, const char* source, int souLen, OUT RegMatrix* regResult, int deftSouIdx)
{
	int souIndex = deftSouIdx;
	int count=0;
	int i;//Indicates the mismatch location of the variable
	int beEnd= true;//Only valid for fixed variables
	for(i=sPos; i<= ePos; i++)
	{
		if(souIndex >= souLen) break;
		if(pat->SubSegAttr[i] == SEG_TYPE_CONST)
		{
			int rst = MatchInSubPatConst_Forward(pat->SubSegment[i], strlen(pat->SubSegment[i]), source, souLen, souIndex);
			if(rst == 0) return -1;
			if(rst == 2) beEnd= false;
		}
		else if(pat->SubSegAttr[i] > SEG_TYPE_SUBVAR)
		{
			if(pat->SubVars[i]-> mark == FIXED_LEN_FLAG)
			{
				count = MatchInSubpatVar_Forward(pat->SubVars[i]->tag, pat->SubVars[i]->len, source, souLen, souIndex);
				if(count < 0)
				{
					return -2;
				}
				else if(count < pat->SubVars[i]->len)
				{
					beEnd= false;
				}
				regResult->Addx(i, souIndex - count, count, pat->SubSegAttr[i]);
			}
			else//for variable
			{
				//Not the last one and is followed by a static field 
				if(i < ePos && pat->SubSegAttr[i+1] == SEG_TYPE_CONST)
				{
					int tempPos = souIndex;
					count = MatchInSubpatVvarWithTail(pat->SubVars[i]->tag, pat->SubVars[i]->len, pat->SubSegment[i+1], source, souLen, souIndex);
					if(count < 0)
					{
						if(souLen - tempPos <= pat->SubVars[i]->len)
						{
							souIndex = tempPos;
							count = MatchInSubpatVar_Forward(pat->SubVars[i]->tag, pat->SubVars[i]->len, source, souLen, souIndex);
							if(count < 0)
							{
								return -4;
							}
							regResult->Addx(i, souIndex - count, count, pat->SubSegAttr[i]);
						}
						else
						{
							return -3;
						}
					}
					else
					{
						regResult->Addx(i, souIndex - count - strlen(pat->SubSegment[i+1]), count, pat->SubSegAttr[i]);
						i++;
					}				
				}
				else
				{
					count = MatchInSubpatVar_Forward(pat->SubVars[i]->tag, pat->SubVars[i]->len, source, souLen, souIndex);
					if(count < 0)
					{
						return -2;
					}
					regResult->Addx(i, souIndex - count, count, pat->SubSegAttr[i]);
				}
			}
		}
	}
	if(i > ePos && souIndex >= souLen) 
	{
		if(beEnd == true) return MATCH_FULL;
		return MATCH_PART;
	}
	else
	{
		if(souIndex >= souLen)
		{
			if(regResult->Count == 0) return MATCH_ONPAT;
			return MATCH_PART;
		} 
		return -1;
	}
}
int SubPatMatch_N_I(SubPattern* pat, int sPos, int ePos, const char* source, int souLen, OUT RegMatrix* regResult)
{
	int souIndex = souLen -1;
	int i;
	int beEnd= true;
	for(i=sPos; i>=ePos; i--)
	{
		if(souIndex < 0) break;
		if(pat->SubSegAttr[i] == SEG_TYPE_CONST)
		{
			int rst = MatchInSubPatConst_Backward(pat->SubSegment[i], strlen(pat->SubSegment[i]), source, souIndex);
			if(rst == 0) return -1;
			if(rst == 2) beEnd= false;
		}
		else if(pat->SubSegAttr[i] > SEG_TYPE_SUBVAR)
		{
			if(pat->SubVars[i]-> mark == FIXED_LEN_FLAG)
			{
				int count = MatchInSubpatVar_Backward(pat->SubVars[i]->tag, pat->SubVars[i]->len, source, souIndex);
				if(count < 0)
				{
					return -2;
				}
				else if(count < pat->SubVars[i]->len)
				{
					beEnd= false;
				}
				regResult->Addx(i, souIndex +1, count, pat->SubSegAttr[i]);
			}
			else
			{
				if(i > ePos && pat->SubSegAttr[i-1] == SEG_TYPE_CONST)
				{
					int tempPos = souIndex;
					int count = MatchInSubpatVvarWithHead(pat->SubVars[i]->tag, pat->SubVars[i]->len, pat->SubSegment[i-1], source, souIndex);
					if(count < 0)
					{
						if(tempPos < pat->SubVars[i]->len)
						{
							souIndex = tempPos;
							count = MatchInSubpatVar_Backward(pat->SubVars[i]->tag, pat->SubVars[i]->len, source, souIndex);
							if(count < 0)
							{
								return -4;
							}
							regResult->Addx(i, souIndex +1, count, pat->SubSegAttr[i]);
						}
						else
						{
							return -3;
						}
					}
					else
					{
						regResult->Addx(i, souIndex + strlen(pat->SubSegment[i-1]) + 1, count, pat->SubSegAttr[i]);
						i--;
					}
				}
				else
				{
					int count = MatchInSubpatVar_Backward(pat->SubVars[i]->tag, pat->SubVars[i]->len, source, souIndex);
					if(count < 0)
					{
						return -2;
					}
					regResult->Addx(i, souIndex +1, count, pat->SubSegAttr[i]);
				}
			}
		}
	}
	if(i <ePos && souIndex <0) 
	{
		if(beEnd == true) return MATCH_FULL;
		return MATCH_PART;
	}
	else
	{
		if(souIndex <0) 
		{
			if(regResult->Count == 0) return MATCH_ONPAT;
			return MATCH_PART;
		}
		return -1;
	}
}
int SubPatMatch_I_J(SubPattern* pat, const char* source, int souLen, OUT RegMatrix* regResult)
{
	//only check vars
	for(int i=0; i< pat->SegSize; i++)
	{
		if(pat->SubSegAttr[i] == SEG_TYPE_CONST)
		{
			regResult->Addy();
			int rst = MatchInSubPatConst_WithVar(pat, i, source, souLen, regResult);
			if(rst <= 0)//miss
			{
				regResult->Suby();
			}
			if(rst == 1)//matched only on pat
			{
				regResult->Reset();
				return MATCH_ONPAT;
			}
		}
		else if(pat->SubSegAttr[i] > SEG_TYPE_SUBVAR)
		{
			char aaa = pat->SubVars[i]->mark;
			int tttt = pat->SubVars[i]->tag;
			int ii = pat->SubVars[i]->len;
			if(INC_TEST_JUDGELEN && souLen > ii) 
			{
				continue;//length is overflow
			}
			int souIndex = 0;
			int count = MatchInSubpatVar_Forward(pat->SubVars[i]->tag, pat->SubVars[i]->len, source, souLen, souIndex);
			if(count == souLen)
			{
				regResult->Addy();
				regResult->Addx(i, 0, count, pat->SubSegAttr[i]);
			}
		}
	}
	int setNum =regResult->Count;
	regResult->ValidCnt =setNum;
	if(setNum > 1)
	{
		//printf("start union proc, setnum before= %d\n", setNum);
		// for(int i= 0; i< setNum; i++)
		// {
		// 	int y=regResult->Matches[i].Count;
		// 	for(int j=0; j< y;j++)
		// 	{
		// 	printf("---%d %d %d   ", regResult->Matches[i].Match[j].Index, regResult->Matches[i].Match[j].So, regResult->Matches[i].Match[j].Eo);
		// 	}
		// 	printf("\n");
		// }

		for(int i= 1; i< setNum; i++)
		{
			for(int ii=0; ii < i; ii++)
			{
				if(regResult->Matches[ii].BeValid == true)
				{
					bool bOk = UnionMatchResult(regResult->Matches[i], regResult->Matches[ii]);
					if(bOk)
					{
						regResult->ValidCnt--;
						break;
					} 
				}
			}
		}

		//printf("end union proc, setnum after= %d\n", regResult->ValidCnt);
		// for(int i= 0; i< setNum; i++)
		// {
		// 	if(regResult->Matches[i].BeValid == true)
		// 	{
		// 		int y=regResult->Matches[i].Count;
		// 		for(int j=0; j< y;j++)
		// 		{
		// 			printf("---%d %d %d   ", regResult->Matches[i].Match[j].Index, regResult->Matches[i].Match[j].So, regResult->Matches[i].Match[j].Eo);
		// 		}
		// 		printf("\n");
		// 	}
		// }
	}
	
	if(setNum > 0) return MATCH_PART;
	return -1;
}

bool UnionMatchResult(RegMatches& obj, RegMatches& refobj)
{
	int matchCnt = 0;
	if(obj.Count == 0) 
	{
		obj.BeValid = false;
		return true;
	}
	if(obj.Count != refobj.Count) return false;
	//only need to check the first
	for(int j=0; j< refobj.Count; j++)
	{
		if(obj.Match[0].Index == refobj.Match[j].Index && obj.Match[0].So == refobj.Match[j].So && obj.Match[0].Eo == refobj.Match[j].Eo)
		{
			//matched
			obj.BeValid = false;
			return true;
		}
	}
	return false;
}

///////////////////////DicPatternMatch//////////////////////
int MatchInDicPatConst_WithVar(SubPattern::DicVarInfo* pat, int curIndex, const char* source, int souLen)
{
	if(pat->segCont[curIndex] == NULL) return 0;
	int patLen = strlen(pat->segCont[curIndex]);
	LcsMatch lcs= GetLCS_DPoptc(source, souLen, pat->segCont[curIndex], patLen);
	if(lcs.Len == 0)// not matched
	{
		return 0;
	}
	else if(lcs.Len == souLen)//whole matched
	{
		return 1;
	}
	int rst=0;
	if(lcs.Pos1 == 0 && (lcs.Pos2 + lcs.Len == patLen))//matched the right
	{
		if(curIndex == pat->segSize-1) return 0;
		rst = DicPatMatch_0_J(pat, curIndex+1, pat->segSize-1, source, souLen, lcs.Len);
		if(rst > 0)
		{
			return 2;
		}
	}
	else if((lcs.Pos1 + lcs.Len == souLen) && lcs.Pos2 == 0)//matched the left
	{
		if(curIndex == 0) return 0;
		rst = DicPatMatch_N_I(pat, curIndex-1, 0, source, souLen - lcs.Len);
		if(rst > 0)
		{
			return 3;
		}
	}
	else if(lcs.Len == patLen && lcs.Pos1 !=0 && (lcs.Pos1 + lcs.Len != souLen))//matched the both side
	{
		if(curIndex != 0 && curIndex != pat->segSize-1)
		{
			rst =DicPatMatch_N_I(pat, curIndex-1, 0, source, lcs.Pos1);
			if(rst > 0)
			{
				rst =DicPatMatch_0_J(pat, curIndex+1, pat->segSize-1, source, souLen, lcs.Pos1 + lcs.Len);
				if(rst > 0)
				{
					return 4;
				}
			}
		}
	}
	return 0;
}

int DicPatMatch_0_J(SubPattern::DicVarInfo* pat, int sPos, int ePos, const char* source, int souLen, int deftSouIdx)
{
	int souIndex = deftSouIdx;
	int count=0;
	int i;
	int varLen = pat->len - pat->segSize + 1;
	for(i=sPos; i<= ePos; i++)
	{
		if(souIndex >= souLen) break;
		if(pat->segTag[i] == SEG_TYPE_CONST)
		{
			int rst = MatchInSubPatConst_Forward(pat->segCont[i], strlen(pat->segCont[i]), source, souLen, souIndex);
			if(rst == 0) return -1;
		}
		else if(pat->segTag[i] > SEG_TYPE_CONST)
		{
			//Not the last one and is followed by a static field 
			if(i < ePos && pat->segTag[i+1] == SEG_TYPE_CONST)
			{
				int tempPos = souIndex;
				count = MatchInSubpatVvarWithTail(pat->segTag[i], varLen, pat->segCont[i+1], source, souLen, souIndex);
				if(count == -2) return -2;
				if(count == -1)
				{
					souIndex = tempPos;
					if(INC_TEST_JUDGELEN && souLen-souIndex > varLen) return -5;
					count = MatchInSubpatVar_Forward(pat->segTag[i], varLen, source, souLen, souIndex);
					if(count < 0)
					{
						return -3;
					}			
				}
				else
				{
					i++;
				}
			}
			else
			{
				if(INC_TEST_JUDGELEN && souLen-souIndex > varLen) return -5;
				count = MatchInSubpatVar_Forward(pat->segTag[i], varLen, source, souLen, souIndex);
				if(count < 0)
				{
					return -3;
				}
			}
		}
	}
	if(souIndex >= souLen) 
	{
		return MATCH_PART;
	}
	return -1;
}

int DicPatMatch_N_I(SubPattern::DicVarInfo* pat, int sPos, int ePos, const char* source, int souLen)
{
	int souIndex = souLen -1;
	int i;
	int varLen = pat->len - pat->segSize + 1;
	for(i=sPos; i>=ePos; i--)
	{
		if(souIndex < 0) break;
		if(pat->segTag[i] == SEG_TYPE_CONST)
		{
			int rst = MatchInSubPatConst_Backward(pat->segCont[i], strlen(pat->segCont[i]), source, souIndex);
			if(rst == 0) return -1;
		}
		else if(pat->segTag[i] > SEG_TYPE_CONST)
		{
			if(i > ePos && pat->segTag[i-1] == SEG_TYPE_CONST)
			{
				int tempPos = souIndex;
				int count = MatchInSubpatVvarWithHead(pat->segTag[i], varLen, pat->segCont[i-1], source, souIndex);
				if(count == -2) return -2;
				if(count == -1)
				{
					souIndex = tempPos;
					if(INC_TEST_JUDGELEN && souIndex > varLen) return -5;
					count = MatchInSubpatVar_Backward(pat->segTag[i], varLen, source, souIndex);
					if(count < 0)
					{
						return -4;
					}
				}
				else
				{
					i--;
				}
			}
			else
			{
				if(INC_TEST_JUDGELEN && souIndex > varLen) return -5;
				int count = MatchInSubpatVar_Backward(pat->segTag[i], varLen, source, souIndex);
				if(count < 0)
				{
					return -3;
				}
			}
		}
	}
	if(souIndex <0) 
	{
		return MATCH_PART;
	}
	return -1;
}

int DicPatMatch_I_J(SubPattern::DicVarInfo* pat, const char* source, int souLen)
{
	int varLen = pat->len - pat->segSize + 1;
	//only check vars
	for(int i=0; i< pat->segSize; i++)
	{
		if(pat->segTag[i] == SEG_TYPE_CONST)
		{
			int rst = MatchInDicPatConst_WithVar(pat, i, source, souLen);
			if(rst > 1)//matched
			{
				return 2;
			}
		}
		else if(pat->segTag[i] > SEG_TYPE_CONST)
		{
			if(INC_TEST_JUDGELEN && souLen > varLen) continue;//length is overflow
			int souIndex = 0;
			int count = MatchInSubpatVar_Forward(pat->segTag[i], varLen, source, souLen, souIndex);
			if(count == souLen)
			{
				return 1;
			}
		}
	}
	return -1;
}

//matchType  align type
//match from start to end, return: <0: miss, 1: full match, 2: partial match  9: only in pat
int SubPatternMatch(SubPattern* pat, const char* source, int matchType, OUT RegMatrix* regResult)
{
	int matchRst = 0;
	int souLen = strlen(source);
	if(souLen == 0)
	{
		return -1;
	}
	switch(matchType)
	{
		case QTYPE_ALIGN_FULL:
		{
			regResult->Addy();
			matchRst = SubPatMatch_0_J(pat, 0, pat->SegSize-1, source, souLen, regResult);
			if(matchRst < 0)
			{
				regResult->Suby();
			}
			else
			{
				regResult->ValidCnt = 1;
			}
			break;
		}
		case QTYPE_ALIGN_LEFT:
		{
			regResult->Addy();
			matchRst = SubPatMatch_0_J(pat, 0, pat->SegSize-1, source, souLen, regResult);
			if(matchRst < 0)
			{
				regResult->Suby();
			}
			else
			{
				regResult->ValidCnt = regResult->Count;
			}
			break;
		}
		case QTYPE_ALIGN_RIGHT:
		{
			regResult->Addy();
			matchRst = SubPatMatch_N_I(pat, pat->SegSize-1, 0, source, souLen, regResult);
			if(matchRst < 0)
			{
				regResult->Suby();
			}
			else
			{
				regResult->ValidCnt = regResult->Count;
			}
			break;
		}
		case QTYPE_ALIGN_ANY:
		{
			matchRst = SubPatMatch_I_J(pat, source, souLen, regResult);
			break;
		}
		default:
		{
			break;
		}
	}
	return matchRst;
}

int DicPatternMatch(SubPattern* pat, const char* source, int matchType, OUT RegMatrix* regResult)
{
	int matchRst = 0;
	int souLen = strlen(source);
	if(souLen == 0)
	{
		return -1;
	}
	for(int i=0; i< pat->DicCnt; i++)
	{
		if(INC_TEST_JUDGELEN && souLen > pat->DicVars[i]->len) continue;
		switch(matchType)
		{
			case QTYPE_ALIGN_FULL:
			case QTYPE_ALIGN_LEFT:
			{
				matchRst = DicPatMatch_0_J(pat->DicVars[i], 0, pat->DicVars[i]->segSize-1, source, souLen);
				break;
			}
			case QTYPE_ALIGN_RIGHT:
			{
				matchRst = DicPatMatch_N_I(pat->DicVars[i], pat->DicVars[i]->segSize-1, 0, source, souLen);
				break;
			}
			case QTYPE_ALIGN_ANY:
			{
				matchRst = DicPatMatch_I_J(pat->DicVars[i], source, souLen);
				break;
			}
			default:
			{
				break;
			}
		}
		if(matchRst > 0)
		{
			regResult->Addy();
			regResult->Addx(pat->DicVars[i]->len, pat->DicVars[i]->lineSno, pat->DicVars[i]->lineCnt, NULL);
		}
	}
	return regResult->Count;
}

int Alg_Test()
{
	
	// RegMatch regResult[MAX_SUBATTR_CNT];
	// string str1 = "/tmp/oss_front.soc<S>_<FI,3>_<FS,2>_<S>";
	// int rst = PatternMatch(str1.c_str(), "front.socAB_12", 1, regResult);
	// printf("1: %d --- \n", rst);
	// for(int i=0; i< rst; i++)
	// {
	// 	printf("\t %d: %d - %d \n", regResult[i].Index, regResult[i].So, regResult[i].Eo);
	// }
	return 0;
}
