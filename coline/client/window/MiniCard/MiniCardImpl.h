#ifndef __ECONTRACTSIMPL_H___
#define __ECONTRACTSIMPL_H___
#define WIN32_LEAN_AND_MEAN	
#include <ComBase.h>
#include <smartskin/smartskin.h>
#include <Core/CoreInterface.h> 
#include <xml/tinyxml.h>

#include "../IMCommonLib/InstantUserInfo.h"


class CMiniCardImpl:  public CComBase<>,
	                  public InterfaceImpl<ICoreEvent>,
				      public InterfaceImpl<IProtocolParser>,
					  public InterfaceImpl<IMiniCard>
{
public:
	CMiniCardImpl(void);
	~CMiniCardImpl(void);
public:
	//IUnknown
	STDMETHOD (QueryInterface)(REFIID riid, LPVOID *ppv);

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

	//IProtocolParser
	STDMETHOD (DoRecvProtocol)(const BYTE *pData, const LONG lSize);
	STDMETHOD (DoPresenceChange)(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder);
	//IMiniCard
	STDMETHOD (ShowMiniCard)(const char *szUserName, int x, int y, int nAlign);
private:
	HRESULT DoTreeImageClick(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	//
	HRESULT DoMenuCommand(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	//
	HRESULT DoClickEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	//
	void ShowCurrContactCard(HWND hWnd, const TCHAR *szCtrlName);
	BOOL UpdateFromServer(const char *szUserName);
	BOOL DoSysProtocol(const char *pType, TiXmlElement *pNode);
private:
	ICoreFrameWork *m_pCore;
	HWND m_hWnd;
	std::string m_strCurrName;
	UINT_PTR m_ptrTimer;
};

#endif

