#ifndef __CORE_LOGIN_IMPL_H____
#define __CORE_LOGIN_IMPL_H____

#define WIN32_LEAN_AND_MEAN	
#include <ComBase.h>
#include <Core/CoreInterface.h>
#include <xml/tinyxml.h>
#include "../IMCommonLib/ProtoTcpSocket.h"

class CLoginImpl: public CComBase<>, 
	              public InterfaceImpl<ICoreLogin>,
				  public InterfaceImpl<ICoreEvent>,
				  public InterfaceImpl<IProtocolParser>,
				  public IProtoSocketNotify
{
public:
	CLoginImpl(void);
	~CLoginImpl(void);
public:
	//IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppv);
	//ICoreEvent
	STDMETHOD (DoCoreEvent)(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
		                     LPARAM lParam, HRESULT *hResult);
	STDMETHOD (SetCoreFrameWork)(ICoreFrameWork *pCore);
	STDMETHOD (GetSkinXmlString)(IAnsiString *szXmlString);
	STDMETHOD (CoreFrameWorkError)(int nErrorNo, const char *szErrorMsg);
	//广播消息
	STDMETHOD (DoBroadcastMessage)(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData);
	STDMETHOD (DoWindowMessage)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes);
	//ICoreLogin
	STDMETHOD (ShowLogonWindow)(const char *szInitUserName, const char *szInitUserPwd, BOOL bAutoLogin);
	STDMETHOD (LogonSvr)(const char *szUserName, const char *szUserPwd, const char *szDomain, 
		                 const char *szPresence, const char *szPresenceMemo);
	STDMETHOD (CancelLogon)();
	//IProtocolParser
	STDMETHOD (DoRecvProtocol)(const BYTE *pData, const LONG lSize);
	STDMETHOD (DoPresenceChange)(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder);
	//
	virtual BOOL OnRecvProtocol(const char *szBuf, const int nBufSize);
	virtual void OnSocketClose(CAsyncNetIoSocket *pSocket, const int nErrorNo);
	virtual void OnSocketConnect(CAsyncNetIoSocket *pSocket, const int nErrorNo);
	void UpdateClientVersion(const char *szHttpIp, const char *szPort, const char *szUrl, const char *szCurVer);
private:
	//EVENT
	HRESULT ClickEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
		                     LPARAM lParam);
	HRESULT LinkEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, LPARAM lParam);

	HRESULT ChangeEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, LPARAM lParam);

	HRESULT ItemSelectEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, LPARAM lParam);
	//
	HRESULT MenuCommandEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, LPARAM lParam);
	//
	HRESULT WindowAfterInit(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, LPARAM lParam);
	//
	void SaveUserInfo();
	void SaveNetSetting(HWND hWnd);
	void SetEnabled(BOOL bEnabled);
	void InitLogonWindow();
	void AddUserInfoToUI(TiXmlElement *pNode, BOOL bShowRealName);
	void StartLogin();
	void GetClientVersioin();
	//
	void ChangeLoginUserName(HWND hWnd, const char *szUserName, BOOL bShowRealName);
	//
	void InitDefaultSvrParam(HWND hWnd);
	//
	void DeleteCurrUser(HWND hWnd);
private:
	std::string m_strClientVer;
	ICoreFrameWork *m_pCore;
	CProtoInterfaceSocket *m_pBalancerSocket;
	HWND m_hWnd;
	std::string m_strPresence;
	std::string m_strPresenceMemo;
	//当前登陆的服务器地址信息
	BOOL m_bLoginSucc;
	std::string m_strLoginHost;
	WORD  m_wLoginPort;
	BOOL  m_bFromBal;
	BOOL  m_bShowRealName;
};

#endif

