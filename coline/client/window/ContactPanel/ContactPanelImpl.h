#ifndef __CONTACTPANEL_IMPL_H____
#define __CONTACTPANEL_IMPL_H____
#define WIN32_LEAN_AND_MEAN	
#include <ComBase.h>
#include <smartskin/smartskin.h>
#include <Core/CoreInterface.h>
#include <Commonlib/SqliteDBOP.h>
#include <Commonlib/GuardLock.h>
#include <xml/tinyxml.h>
#include <map>
#include "../IMCommonLib/InstantUserInfo.h"
#include "../IMCommonLib/InterfaceUserList.h"

class CContactPanelImpl: public CComBase<>,
	                     public InterfaceImpl<ICoreEvent>,
				         public InterfaceImpl<IProtocolParser>,
					     public InterfaceImpl<IContactPanel>
 
{
public:
	CContactPanelImpl(void);
	~CContactPanelImpl(void);
public:
	//IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppv);
	//ICoreEvent
	STDMETHOD (DoCoreEvent)(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
		                     LPARAM lParam, HRESULT *hResult);
	STDMETHOD (SetCoreFrameWork)(ICoreFrameWork *pCore);
	STDMETHOD (GetSkinXmlString)(IAnsiString *szXmlString);
	//
	STDMETHOD (CoreFrameWorkError)(int nErrorNo, const char *szErrorMsg);
		//¹ã²¥ÏûÏ¢
	STDMETHOD (DoBroadcastMessage)(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData);
	//
	STDMETHOD (DoWindowMessage)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes);
	//IProtocolParser
	STDMETHOD (DoRecvProtocol)(const BYTE *pData, const LONG lSize);
	//
	STDMETHOD (DoPresenceChange)(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder);
	//IContactPanel
	STDMETHOD (ShowPanel)(HWND hParent, const TCHAR *szCaption, LPRECT lprc, BOOL bModal, IUserList *pList);
 
	STDMETHOD (GetSelContact)(IUserList *pList);
private:
	HRESULT DoClickEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	HRESULT DoMenuCommandEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	HRESULT DoStatusChanged(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	BOOL InitContactPanel(HWND hParent, LPRECT lprc, const TCHAR *szCaption, HWND *hWnd, IUserList *pList);
	void DoAdjustSelNode(IContacts *pContact, HWND hWnd, const char *szUserName, BOOL bAdd);
	void DelTreeSelectedUsers(HWND hWnd);
private:
	HWND m_hCurrHwnd;
	CInterfaceUserList m_SelList;
	ICoreFrameWork *m_pCore; 
	HWND m_hWnd;
};

#endif
