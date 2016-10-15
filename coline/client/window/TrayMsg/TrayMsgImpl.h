#ifndef __TRAYMSG_IMPL_H____
#define __TRAYMSG_IMPL_H____

#include <map>
#include <ComBase.h>
#include <Core/CoreInterface.h>
#include <Commonlib/stringutils.h>
#include "TrayMsgSysIcon.h"
#include "../IMCommonLib/PendingMsgList.h"

class CTrayMsgImpl:  public CComBase<>,
	                 public ITrayMsgSysIconApp,
	                 public InterfaceImpl<ITrayMsg>,
					 public InterfaceImpl<ICoreEvent>,
					 public InterfaceImpl<IProtocolParser>
{
public:
	CTrayMsgImpl(void);
	~CTrayMsgImpl(void);
public:
	//IUnknown
	STDMETHOD (QueryInterface)(REFIID riid, LPVOID *ppv);

	//ITrayMsg
	STDMETHOD (InitTrayMsg)(HINSTANCE hInstance, HICON hDefaultIcon, const char *szTip);
	STDMETHOD (RefreshPresence)(const char *szPresence, const char *szMemo);
	STDMETHOD (AddAnimateIcon)(HICON hAnimIcon);
	STDMETHOD (StartAnimate)(const char *szTip);
	STDMETHOD (StopAnimate)();
	STDMETHOD (ShowTrayIcon)();
	STDMETHOD (HideTrayIcon)();
	STDMETHOD (ShowTipInfo)(const TCHAR *szImage, const TCHAR *szTipText, 
		                    const TCHAR *szCaption, const TCHAR *szUrl, BOOL bUserClosed); 
	
	//
	STDMETHOD (ShowTipPanel)(const char *szCaption, const char *szContent);
	//ICoreEvent
	STDMETHOD (DoCoreEvent)(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
		                     LPARAM lParam, HRESULT *hResult);
	STDMETHOD (SetCoreFrameWork)(ICoreFrameWork *pCore);
	STDMETHOD (GetSkinXmlString)(IAnsiString *szXmlString);
	//¹ã²¥ÏûÏ¢
	STDMETHOD (DoBroadcastMessage)(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData);
	//
	STDMETHOD (CoreFrameWorkError)(int nErrorNo, const char *szErrorMsg);
	//
	STDMETHOD (DoWindowMessage)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes);

	//
	STDMETHOD (DoRecvProtocol)(const BYTE *pData, const LONG lSize);
	STDMETHOD (DoPresenceChange)(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder);
	//ITrayMsgSysIconApp
	//×ó¼üµ¥»÷
	virtual void OnLButtonClick(int nShiftState,int nX, int nY);
	//ÓÒ¼üµ¥»÷
	virtual void OnRButtonClick(int nShiftState,int nX, int nY);
	//×ó¼üË«»÷
	virtual void OnLButtonDblClick(int nShiftState,int nX, int nY);
	//ÓÒ¼üË«»÷
	virtual void OnRButtonDblClick(int nShiftState, int nX, int nY);
	//
	virtual void OnBalloonShow(int nShiftState, int nX, int nY);
	//
	virtual void OnBalloonHide(int nShiftState, int nX, int nY);
	//
	virtual void OnToolTipShow(BOOL bActived);
private:
	BOOL GetWindowHWND(const char *szName, HWND *h);
	BOOL InitTrayPanel(const TCHAR *szCaption, const TCHAR *szImage, const TCHAR *szUrl, 
		               const TCHAR *szTipText, HWND *hWnd, BOOL bUserClosed);
	HRESULT DoLinkEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	void InitSelfUserName();
	//
	void ShowHomePage();
	//
	void ShowMsgTipBox();
	//
	void InitMsgTip(HWND hWnd, IUserPendMessageTipList *pList);
private:
	CTrayMsgSysIcon *m_TrayIcon;
	ICoreFrameWork *m_pCore;
	volatile LONG m_lRef;	
	UINT m_uToolBarMsg;
	int  m_nTimerId;
	RECT m_rcScreen;
	UINT_PTR m_ptrTimer;
	std::string m_strUserName;
	std::string m_strRealName;
	std::string m_strHomePage;
	BOOL m_bFlashTray;
	HWND m_hHomePage;
	HWND m_hWnd;
	HWND m_hMsgBox;
	HWND m_hMain;
	HWND m_hLogin;
};

#endif
