#ifndef __USEREVENTLIST_H____
#define __USEREVENTLIST_H____

#include <string>
#include <map>
#include <vector>
#include <Commonlib/GuardLock.h>
#include <Commonlib/stringutils.h>
#include <Core/CoreInterface.h>

class COrderEventList
{
public:
	COrderEventList();
	~COrderEventList();
public:
	BOOL DoEvent(HWND hWnd, WPARAM wParam, LPARAM lParam, HRESULT *hResult);
	BOOL DoEvent(HWND hWnd, const char *szCtrlName, const char *szEventType, WPARAM wParam, 
		           LPARAM lParam, HRESULT *hResult);
	BOOL AddOrderEvent(ICoreEvent *pOrder);
	BOOL DeleteOrderEvent(ICoreEvent *pOrder);
	void SetEventName(const char *szEventName);
	void SetEventType(const char *szEventType);
private:
	void Clear();
private:
	std::vector<ICoreEvent *> m_OrderList;
	std::string m_strEventName;
	std::string m_strEventType;
};

class CUserEventList
{
public:
	CUserEventList(void);
	~CUserEventList(void);
public:
	BOOL AddEvent(const char *szWndName, const char *szCtrlName, const char *szEventType, ICoreEvent *pCore);
	BOOL DeleteEvent(const char *szWndName, const char *szCtrlName, const char *szEventType, ICoreEvent *pCore);
	BOOL DoEvent(HWND hWnd, const char *szWndName, const char *szCtrlName, const char *szEventType, WPARAM wParam, 
		           LPARAM lParam, HRESULT *hResult);
	void Clear();
private:
	BOOL OrderAllWndEvent(ICoreEvent *pCore, const char *szWndName);
	BOOL OrderNormalEvent(ICoreEvent *pCore, const char *szWndName, const char *szCtrlName, const char *szEventType);
	BOOL DeleteAllWndEvent(ICoreEvent *pCore, const char *szWndName);
	BOOL DeleteNormalEvent(ICoreEvent *pCore, const char *szWndName, const char *szCtrlName, const char *szEventType);
private:
	std::map<CAnsiString_, COrderEventList *> m_EventList;
	std::map<CAnsiString_, COrderEventList *> m_WndEventList;
};

//protocol 
class COrderProtocolList
{
public:
	COrderProtocolList();
	~COrderProtocolList();
public:
	BOOL AddOrderProtocol(IProtocolParser *pOrder);
	BOOL DeleteOrderProtocol(IProtocolParser *pOrder);
	BOOL DoProtocol(const char *pBuf, const int nBufSize);
	BOOL DoPresenceChanged(const char *szUserName, const char *szPresence, const char *szMemo, BOOL bOrder);
private:
	std::vector<IProtocolParser *> m_OrderList;
};

class COrderUserProtocol
{
public:
	COrderUserProtocol();
	~COrderUserProtocol();
public:
	BOOL AddOrderProto(IProtocolParser *pOrder, const char *szProtoName, const char *szProtoType);
	BOOL DeleteOrderProto(IProtocolParser *pOrder, const char *szProtoName, const char *szProtoType);
	BOOL DoProtocol(const char *szProtoName, const char *szType, const char *pBuf, const int nBufSize);
	BOOL DoPresenceChanged(const char *szUserName, const char *szPresence, const char *szMemo, BOOL bOrder);
	void Clear();
private:
	BOOL OrderAllName(IProtocolParser *pOrder, const char *szProtoName);
	BOOL OrderNameType(IProtocolParser *pOrder, const char *szProtoName, const char *szProtoType);
	//delete
	BOOL DeleteAllName(IProtocolParser *pOrder, const char *szProtoName);
	BOOL DeleteNameType(IProtocolParser *pOrder, const char *szProtoName, const char *szProtoType);
private:
	std::map<CAnsiString_, COrderProtocolList *> m_OrderList; //one protocol, name and type
	std::map<CAnsiString_, COrderProtocolList *> m_OrderName; //All Name Protocol
	CGuardLock m_Lock;
};

#endif
