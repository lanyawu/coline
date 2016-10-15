#ifndef __MESSAGELIST_H____
#define __MESSAGELIST_H____
#include <string>
#include <vector>
#include <ComBase.h>
#include <Core/CoreInterface.h>


class CMessageItem
{
public:
	int nMsgId;
	std::string strType;
	std::string strFromName;
	std::string strToName;
	std::string strTime;
	std::string strMsg;
};

class CMessageList: public CComBase<>, 
	                public InterfaceImpl<IMessageList>
{
public:
	CMessageList(void);
	~CMessageList(void);
public:
	static BOOL StringToTimeStamp(const char *szTime, int &nStamp);
	static BOOL TimeStampToString(const int nStamp, std::string &strTime);
	STDMETHOD (Clear)();
	//IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppv);

	//IMessageList
	//
	STDMETHOD (AddMsg)(const int nMsgId, const char *szType, const char *szFromName,
		               const char *szToName, const char *szTime, const char *szMsg);
	STDMETHOD (AddRawMsg)(const int nMsgId, const char *szRawMsg);
	//
	STDMETHOD_(DWORD, GetCount)();
	//
	STDMETHOD (GetMsg)(const int nIdx, int *nMsgId, IAnsiString *szType, IAnsiString *strFromName,
		               IAnsiString *strToName, IAnsiString *strTime, IAnsiString *strMsg);
	STDMETHOD (GetRawMsg)(const int nIdx, int *nMsgId, IAnsiString *strRawMsg);
private:
	std::vector<CMessageItem *> m_List;
};

#endif

