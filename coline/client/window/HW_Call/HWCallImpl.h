#ifndef __HWCALLIMPL_H____
#define __HWCALLIMPL_H____
#include <map>
#include <ComBase.h>
#include <vector>
#include <Core/CoreInterface.h>
#include <Commonlib/GuardLock.h>
#include <Commonlib/stringutils.h> 

//常用联系人
interface __declspec(uuid("8D9ECEAE-E914-4C1B-A439-CAA1A2408911")) IHWCallImpl :public IUnknown
{

};

class CHWCallImpl       :public CComBase<>, 
					     public InterfaceImpl<ICoreEvent>, 
					     public InterfaceImpl<IHWCallImpl>
{
public:
	CHWCallImpl(void);
	~CHWCallImpl(void);

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
	//广播消息
	STDMETHOD (DoBroadcastMessage)(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData);
	//
	STDMETHOD (DoWindowMessage)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes);
private:
	void DoCallPhone(HWND hWnd);
	BOOL GetCurrentUserTel(HWND hWnd, std::string &strTel);
private:
	ICoreFrameWork *m_pCore;
};

#endif