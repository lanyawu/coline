#ifndef __WIDGETBOX_IMPL_H_____
#define __WIDGETBOX_IMPL_H_____

#include <map>
#include <ComBase.h>
#include <Core/CoreInterface.h>
#include <Commonlib/stringutils.h> 

interface __declspec(uuid("0E377446-8772-49DA-B39A-A08F02CE459B")) IWidgetBox :public IUnknown
{

};

class CWidgetBoxImpl:  public CComBase<>, 
					   public InterfaceImpl<ICoreEvent>,
					   public InterfaceImpl<IWidgetBox>,
					   public InterfaceImpl<IProtocolParser>
{
public:
	CWidgetBoxImpl(void);
	~CWidgetBoxImpl(void);
	friend BOOL CALLBACK RichEditCallBack(HWND hWnd, DWORD dwEvent, char *szFileName, DWORD *dwFileNameSize,
			  char *szFileFlag, DWORD *dwFileFlagSize, DWORD dwFlag, void *pOverlapped);
public:
	//IUnknown
	STDMETHOD (QueryInterface)(REFIID riid, LPVOID *ppv);

  
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
	//
	//IProtocolParser
	STDMETHOD (DoRecvProtocol)(const BYTE *pData, const LONG lSize);
	STDMETHOD (DoPresenceChange)(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder);
private:
	HRESULT DoClickEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	HRESULT DoMenuCommand(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	void ShowWidgetBox(HWND hParent);
	//
	void OnWMDropFiles(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void ParserProtocol(char *pData, LONG lSize);
	void RefreshShow2Dock();
	const char *GetImagePath();
	BOOL RECallBack(HWND hWnd, DWORD dwEvent, char *szFileName, DWORD *dwFileNameSize,
			  char *szFileFlag, DWORD *dwFileFlagSize, DWORD dwFlag);
	void InitWidget(HWND hWnd);
	BOOL AddWidgetToUI(HWND hWnd, const char *szTabId, const char *szCaption, const char *szUrl, 
		const char *szTip, const int nImageId);
	BOOL AddWidgetTab(HWND hWnd, const char *szTabId, const char *szTabName);
	BOOL AddNewTab(HWND hWnd);
private:
	ICoreFrameWork *m_pCore;
	BOOL m_bDocked;
	std::string m_strImagePath;
	BOOL m_bShow2Dock;
	std::map<std::string, CStdString_> m_Plugins;
	HWND m_hWnd;
};
 
#endif
