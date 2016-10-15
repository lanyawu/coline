#ifndef __TEL_QUERYIMPL_H_____
#define __TEL_QUERYIMPL_H_____

#include <map>
#include <ComBase.h>
#include <Core/CoreInterface.h>
#include <Commonlib/stringutils.h> 

interface __declspec(uuid("102915AA-6DA3-4440-85AC-373F6285A204")) ITelQuery :public IUnknown
{
};

class CTelQueryImpl:  public CComBase<>, 
					  public InterfaceImpl<ICoreEvent>,
					  public InterfaceImpl<ITelQuery>
{
public:
	CTelQueryImpl(void);
	~CTelQueryImpl(void);
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


private:
	HRESULT DoSendP2PMsg(HWND hWnd, const char *szContent);
private:
	ICoreFrameWork *m_pCore;
};
 
#endif
