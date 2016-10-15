#ifndef __UIMANAGER_H____
#define __UIMANAGER_H____

#define WIN32_LEAN_AND_MEAN	
#include <string>
#include <map>
#include <ComBase.h>
#include <Commonlib/stringutils.h>
#include <Core/CoreInterface.h>

#include "OrderWinMsgList.h"

class CUIManagerImpl: public CComBase<>, 
	                  public InterfaceImpl<IUIManager>,
					  public InterfaceImpl<ICoreEvent>
{
public:
	CUIManagerImpl(void);
	~CUIManagerImpl(void);
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
	//广播消息
	STDMETHOD (DoBroadcastMessage)(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData);
	//
	STDMETHOD (DoWindowMessage)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes);

	//IUIManager
	STDMETHOD (AddPluginSkin)(const char *szXmlString);
	STDMETHOD (CreateSkinByXmlStream)(const char *szXml, const int nXmlSize, const char *szSkinPath);
	//
	STDMETHOD (InitSkinXmlFile)(const char *szXmlFile);
	STDMETHOD (CreateUIWindow)(HWND hParent, const char *szWindowName, const PRECT lprc, DWORD dwStyle,
		       DWORD dwExStyle, const TCHAR *szCaption, HWND *hWnd);
	STDMETHOD (ShowModalWindow)(HWND hParent, const char *szWindowName, const TCHAR *szCaption, 
		           const int X, const int Y, const int nWidth, const int nHeight, int *nModalResult);
	//订制消息
    STDMETHOD (OrderWindowMessage)(const char *szWndName, HWND hWnd, UINT uMsg, ICoreEvent *pCore);
	STDMETHOD (ClearOrderAllMessage)();
	//删除订制
	STDMETHOD (DeleteWindowMessage)(const char *szWndName, UINT uMsg, ICoreEvent *pCore);
	//
	STDMETHOD (GetWindowHWNDByName)(const char *szWndName, HWND *hWnd);
	//
	STDMETHOD (SendMessageToWindow)(const char *szWndName, UINT uMsg, WPARAM wParam, 
		                            LPARAM lParam, LRESULT *hr);
	STDMETHOD (UIManagerRun)();
	STDMETHOD (BlendSkinStyle)(COLORREF cr);
	STDMETHOD (AlphaBackImage)(const char *szFileName);
	STDMETHOD (GetControlText)(HWND hWnd, const TCHAR *szCtrlName, TCHAR *szText, int *nSize);
	STDMETHOD (SetControlText)(HWND hWnd, const TCHAR *szCtrlName, const TCHAR *szText);
private:
	static BOOL CALLBACK WindowEventCallback(HWND hWnd, LPCTSTR pstrEvent, LPCTSTR pstrWndName, LPCTSTR pstrControlName,
		                     POINT *ptMouse, WPARAM wParam, LPARAM lParam, void *pOverlapped);
	static BOOL CALLBACK WindowMessageCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, 
		                     LRESULT *lRes, void *pOverlapped);
private:
	ICoreFrameWork *m_pCore;
	std::multimap<CAnsiString_, HWND> m_WinList;
	std::map<HWND, COrderWinMsgList *> m_OrderList;
};

#endif
