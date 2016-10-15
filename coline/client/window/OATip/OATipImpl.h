#ifndef __OATIP_IMPL_H___
#define __OATIP_IMPL_H___

#include <map>
#include <ComBase.h>
#include <Core/CoreInterface.h>
#include <Commonlib/stringutils.h>   
#include <xml/tinyxml.h>


class COATipImpl:  public CComBase<>, 
			       public InterfaceImpl<ICoreEvent>,
				   public InterfaceImpl<IProtocolParser>,
				   public InterfaceImpl<IOATip>
{
public:
	COATipImpl(void);
	~COATipImpl(void);
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
	
	//IProtocolParser
	STDMETHOD (DoRecvProtocol)(const BYTE *pData, const LONG lSize);
	STDMETHOD (DoPresenceChange)(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder);
	//
	STDMETHOD (ShowOATipPanel)(const TCHAR *szSender, const TCHAR *szFrom, const TCHAR *szTime,
		                     const TCHAR *szCatalog, const TCHAR *szId, const TCHAR *szTip, const TCHAR *szBody);
private:
	void DoOATipProtcol(const char *szType, TiXmlElement *pNode); 
	void OpenMail();
private:
	ICoreFrameWork *m_pCore;
	HWND m_hMainWindow;
};
 
#endif
 

