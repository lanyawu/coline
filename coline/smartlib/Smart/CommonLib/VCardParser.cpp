#include <fstream>
#include <Commonlib/VCardParser.h>

#pragma warning(disable:4996)

CCardItem::CCardItem():
           m_bIsBegin(FALSE),
		   m_bIsEnd(FALSE),
		   m_bStartData(FALSE)
{
	//
}

extern "C" void VCARD_PropHandler(void *userData, const CARD_Char *propName, const CARD_Char **params)
{
	if (!userData)
		return;
	VCardList *pList = (VCardList *)userData;

	if (strcmpi(propName, "BEGIN") == 0)
	{
		// begin: vcard/vcal/whatever
		pList->push(CCardItem()); 
        return;
	};

	if (pList->empty())
    {
        
        return;
    };

	if (strcmpi(propName, "END") == 0)
	{
		// end: vcard/vcal/whatever
		pList->top().m_bIsEnd = TRUE; 
	}
	else
	{ 
		pList->top().m_strCurrType = propName; 
	};
}

extern "C" void VCARD_DataHandler(void *userData, const CARD_Char *data, int len)
{
	if (!userData)
		return ;
	VCardList *pList = (VCardList *)userData;

    if (pList->empty())
    { 
        return;
    };

	if (pList->top().m_bIsBegin)
	{
		// accumulate begin data 
		if (len > 0)
		{
			char *pTmp = new char[len + 1];
			memset(pTmp, 0, len + 1);
			strncpy(pTmp, data, len); 
			pList->top().m_strType += pTmp;
		}
		else
		{
			 
			pList->top().m_bIsBegin = FALSE;
		};
	}
	else if (pList->top().m_bIsEnd)
	{
		if (len == 0)
        {
			pList->top().m_strType.clear();
        };
	}
	else
	{ 
		pList->top().m_bStartData = FALSE;

		if (len == 0)
        { 
            pList->top().m_bStartData = TRUE;
        }
		else
		{
			// output printable data
            // do some arbitary wrapping
			if (!pList->top().m_strCurrType.empty())
			{
				if (::stricmp(pList->top().m_strCurrType.c_str(), "FN") == 0)
				{
					//ÐÕÃû
					char szSrc[256] = {0};
					char szTmp[256] = {0};
					strncpy(szSrc, data, len);
					CStringConversion::UTF8ToString(szSrc, szTmp, 255);
					pList->top().m_strName = szTmp;
				} else if (::stricmp(pList->top().m_strCurrType.c_str(), "TEL") == 0)
				{
					char szSrc[256] = {0}; 
					strncpy(szSrc, data, len);
					pList->top().m_strTel = szSrc;
				} else if (::stricmp(pList->top().m_strCurrType.c_str(), "EMAIL") == 0)
				{
					char szSrc[256] = {0}; 
					strncpy(szSrc, data, len);
					pList->top().m_strEmail = szSrc;
				}
			} 
            // debug
            // write to file
            /* 
            ofstream os("e:\\test\\test.jpg", ios::binary | ios::ate);
            os.write(data, len);
            os.close();
            */
		};
	};
};

CVCardParser::CVCardParser(void)
{
}


CVCardParser::~CVCardParser(void)
{
}

BOOL CVCardParser::LoadFromStream(const char *pBuf, const int nBufSize, VCardList &vcList)
{
	// allocate parser
	std::locale::global(std::locale(""));
	CARD_Parser vp = CARD_ParserCreate(NULL);
	BOOL bSucc = TRUE;
	// initialise
	CARD_SetPropHandler(vp, VCARD_PropHandler);
	CARD_SetDataHandler(vp, VCARD_DataHandler);
	CARD_SetUserData(vp, &vcList);
	if (CARD_Parse(vp, pBuf, nBufSize, false) != 0)
	{
		CARD_Parse(vp, NULL, 0, true);
	} else
		bSucc = FALSE;
	// free parser
	CARD_ParserFree(vp);

	return bSucc; 
}

BOOL CVCardParser::LoadFromFile(const char *szFileName, VCardList &vcList)
{
	BOOL bSucc = FALSE;
	int nSize = 0;
	char *pBuf = NULL;
	TCHAR szwFileName[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szFileName, szwFileName, MAX_PATH - 1);
	ifstream ifs(szwFileName, std::ios::binary);
	if (ifs.is_open())
	{
		ifs.seekg(0, std::ios::end);
		nSize = (int) ifs.tellg(); 
		if (nSize > 0)
		{
			pBuf = new char[nSize];
			ifs.seekg(0, std::ios::beg);
			ifs.read(pBuf, nSize);
			bSucc = (ifs.gcount() == nSize);
		}
		ifs.close();  
	}
	if (bSucc && pBuf)
	{
		bSucc = LoadFromStream(pBuf, nSize, vcList);
	}
	if (pBuf)
		delete []pBuf;
	return bSucc;
}

#pragma warning(default:4996)
