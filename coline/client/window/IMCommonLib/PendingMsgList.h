#ifndef __PENDINGMSGLIST_H___
#define __PENDINGMSGLIST_H___

#include <string>
#include<vector>
#include <list>
#include <ComBase.h>
#include <Commonlib/GuardLock.h>
#include <Core/CoreInterface.h>

class CPendingMsgItem
{
public:
	std::string m_strFromName;
	std::string m_strType;
	std::string m_strProtocol;
};

class CUserMessageTip: public CComBase<>, 
	                   public InterfaceImpl<IUserPendMessageTip>
{
public:
	CUserMessageTip();
	~CUserMessageTip();
public:
	//IUnknown
	STDMETHOD (QueryInterface)(REFIID riid, LPVOID *ppv);
	//IUserPendMessageTip
    //get
	STDMETHOD (GetUserName)(IAnsiString *strUserName);
	STDMETHOD_ (int, GetMsgCount)() ;
	STDMETHOD (GetMessageTip)(IAnsiString *strMsg);
	//set
	STDMETHOD (SetUserName)(const char *szUserName);
	STDMETHOD (SetMsgCount)(const int nCount);
	STDMETHOD (SetMessageTip)(const char *szMsg);
	//
	void IncCount();
    const char *GetUserName();
private:
	std::string m_strUserName;
	int m_nMsgCount;
	std::string m_strMsgTip;
};

class CUserMessageTipList :public CComBase<>,
	                       public InterfaceImpl<IUserPendMessageTipList>
{
public:
	CUserMessageTipList();
	~CUserMessageTipList();
public:
	//IUnknown
	STDMETHOD (QueryInterface)(REFIID riid, LPVOID *ppv);
	//	
	STDMETHOD_ (int, GetPendMsgTipCount)();
	STDMETHOD (GetFrontMessage)(IUserPendMessageTip *pTip);
	//
	STDMETHOD (AddPendMsgTip)(IUserPendMessageTip *pTip, BOOL bCopy);
private:
	std::list<IUserPendMessageTip *> m_TipList;
};

class CPendingMsgList
{
public:
	CPendingMsgList(void);
	~CPendingMsgList(void);
public:
	BOOL AddItem(const char *szFromName, const char *szType, const char *szBuf);
	BOOL GetFrontProtocolByName(const char *szFromName, const char *szType,
		                   IAnsiString *strProtocl, BOOL bPop = TRUE);
	BOOL GetLastProtocol(IAnsiString *strFromName, IAnsiString *strType);
	//
	BOOL GetMessageTipList(IUserPendMessageTipList *pList);
private:
	std::vector<CPendingMsgItem *> m_PendingList;
	CGuardLock m_Lock;
};

#endif
