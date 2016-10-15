#ifndef __GROUPFRAME_IMPL_H___
#define __GROUPFRAME_IMPL_H___
#define WIN32_LEAN_AND_MEAN	
#include <ComBase.h>
#include <smartskin/smartskin.h>
#include <Core/CoreInterface.h>
#include <Commonlib/SqliteDBOP.h>
#include <Commonlib/GuardLock.h>
#include <xml/tinyxml.h>

#include "../IMCommonLib/InstantUserInfo.h"
#include "../IMCommonLib/InterfaceUserList.h"

class CBMCFrameImpl  : public CComBase<>,
	                   public InterfaceImpl<ICoreEvent>,
				       public InterfaceImpl<IProtocolParser>,
					   public InterfaceImpl<IBMCFrame>   
{
public:
	CBMCFrameImpl(void);
	~CBMCFrameImpl(void);
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
	//
	STDMETHOD (DoBroadcastMessage)(const char *szFromWndName, HWND hWnd, const char *szType, 
		                           const char *szContent, void *pData);
	//
	STDMETHOD (DoWindowMessage)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes);
	//IProtocolParser
	STDMETHOD (DoRecvProtocol)(const BYTE *pData, const LONG lSize);
	STDMETHOD (DoPresenceChange)(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder);
	//IBMCFrame
	STDMETHOD (ShowBMCFrame)(LPRECT lprc);
private:
	HRESULT DoMenuCommand(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
private:
	ICoreFrameWork *m_pCore;
	HWND m_hWnd;
};

#endif
