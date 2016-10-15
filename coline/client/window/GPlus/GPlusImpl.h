#ifndef __GPLUSIMPL_H____
#define __GPLUSIMPL_H____
#include <map>
#include <string>
#include <ComBase.h>
#include <Core/CoreInterface.h>
#include <Commonlib/stringutils.h> 

class CGPlusImpl:  public CComBase<>, 
				   public InterfaceImpl<ICoreEvent>,
				   public InterfaceImpl<IGPlus>
{
public:
	CGPlusImpl(void);
	~CGPlusImpl(void);
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

	//IGPlus
	STDMETHOD (ShowGPlusToolBar)();
private:
	void ShowDesktop();
	void ShowAppDesktop();
	void LoadUserApplications();
	void LoadRecentlyUsers();
	void HideAllDesktop();
	BOOL AddWidgetToUI(HWND hWnd, const char *szTabId, const char *szCaption, const char *szUrl, 
		const char *szTip, const int nImageId, const char *szImageFile);
private:
	ICoreFrameWork *m_pCore;
	HWND m_hMainToolBar;
	HWND m_hSearchBar;

	//
	HWND m_hDesktop;
	HWND m_hAppDesktop;
	std::map<std::string, std::string> m_Plugins;
};

#endif
