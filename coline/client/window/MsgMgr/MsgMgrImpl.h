#ifndef __MSGMGR_IMPL_H____
#define __MSGMGR_IMPL_H____
#include <map>
#include <ComBase.h>
#include <Commonlib/GuardLock.h>
#include <Core/CoreInterface.h>
#include <Commonlib/stringutils.h>
#include <Commonlib/SqliteDBOP.h> 

class CMsgMgrImpl: public CComBase<>,
	               public InterfaceImpl<IMsgMgr>
{
public:
	CMsgMgrImpl(void);
	~CMsgMgrImpl(void);
public:
	//IUnknown
	STDMETHOD (QueryInterface)(REFIID riid, LPVOID *ppv);
	
	//IMsgMgr
	//init 
	STDMETHOD (InitMsgMgr)(const char *szFileName, const char *szUserName);
	//
	STDMETHOD (SaveMsg)(const char *szType, const char *szFromName, const char *szToName,
		                 const char *szTime, const char *szMsg, const char *szBody);
	//get message 
	STDMETHOD (GetMsg)(const char *szType, const char *szFromName, const int nPage, 
		                const int nPageCount, IMessageList *pList);
	//
	STDMETHOD (GetMsgById)(const char *szType, const int nMsgId, IMessageList *pList);
	STDMETHOD (GetRawMsg)(const char *szType, const char *szFromName, const int nPage, 
		                const int nPageCount, IMessageList *pList);
	STDMETHOD_ (int, GetMsgCount)(const char *szType, const char *szFromName);
	//nMsg != 0 delete one message
	//nMsg == 0 delete all message from == szFromName and type == szType
	STDMETHOD (ClearMsg)(const int nMsgId, const char *szType, const char *szFromName);
	//
	STDMETHOD_ (int, GetSearchMsgCount)(const char *szType, const char *szKey, const char *szFromName);
	//
	STDMETHOD (SearchMsg)(const char *szKey, const char *szType, const char *szFromName,
		                   const int nPage, const int nPageCount, IMessageList *pList);
	//
	STDMETHOD (SearchRawMsg)(const char *szKey, const char *szType, const char *szFromName,
		                   const int nPage, const int nPageCount, IMessageList *pList); 
	//
	STDMETHOD (SaveGroupInfo)(const char *szGrpId, const char *szGrpDspName, const char *szCreator);
	//
	STDMETHOD (GetGroups)(IUserList *pList);
	//
	STDMETHOD (GetSmsUserList)(IUserList *pList);
private:
	BOOL CheckDBOPValid();
	void InitChatContactList();
	void CheckChatContactIsExists(const char *szFromName);
private:
	CSqliteDBOP *m_pDBOP;
	CGuardLock m_DbLock;
	std::map<CAnsiString_, int> m_ContactList;
};

#endif
