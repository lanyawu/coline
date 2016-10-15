#ifndef __VCARDPARSER_H____
#define __VCARDPARSER_H____
#include <commonlib/stringutils.h>
#include <stack>
#include <vcardparser/cardparser.h>

class CCardItem
{
public:
	CCardItem();
	~CCardItem() {};
public:
	BOOL m_bIsBegin;
	BOOL m_bIsEnd;
	BOOL m_bStartData;
	std::string m_strCurrType;
	std::string m_strType;
	std::string m_strName;
	std::string m_strTel;
	std::string m_strEmail;
};

typedef stack<CCardItem> VCardList;

class CVCardParser
{
public:
	CVCardParser(void);
	~CVCardParser(void);
public: 
	BOOL LoadFromStream(const char *pBuf, const int nBufSize, VCardList &vcList);
	BOOL LoadFromFile(const char *szFileName, VCardList &vcList);
};

#endif
