#ifndef __COREFRAMEWORKIMPL_H___
#define __COREFRAMEWORKIMPL_H___

#define WIN32_LEAN_AND_MEAN	
#include <map>
#include <string>
#include <ComBase.h>
#include <Core/CoreInterface.h>
#include <xml/tinyxml.h>
#include <Commonlib/stringutils.h>
#include "../IMCommonLib/ProtoTcpSocket.h"
#include "../IMCommonLib/UserEventList.h"
#include "../IMCommonLib/PluginList.h"
#include "../IMCommonlib/PendingMsgList.h"

class CCoreFrameWorkImpl: public IProtoSocketNotify,
	                      public CComBase<>, 
	                      public InterfaceImpl<ICoreFrameWork>,
				          public InterfaceImpl<IProtocolParser>

{
public:
	CCoreFrameWorkImpl(void);
	~CCoreFrameWorkImpl(void);
public:
	//IProtoSocketNotify
	virtual BOOL OnRecvProtocol(const char *szBuf, const int nBufSize);
	virtual void OnSocketClose(CAsyncNetIoSocket *pSocket, const int nErrorNo);
	virtual void OnSocketConnect(CAsyncNetIoSocket *pSocket, const int nErrorNo);

	//IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppv);
	//ICoreFrameWork
	STDMETHOD (SetAgent)(const char *szType, const char *szAddress, USHORT uPort, 
		                     const char *szUserName, const char *szUserPwd);
	//send raw protocol
	STDMETHOD (SendRawMessage)(const BYTE *pData, const LONG lSize, const LONG lStyle);
	//user information
	STDMETHOD (GetUserName)(IAnsiString *szUserName);
	//
	STDMETHOD (GetUserNickName)(IAnsiString *szUTF8Name);
	//
	STDMETHOD (GetUserDomain)(IAnsiString *szDomain);
	//
	STDMETHOD (GetUserPassword)(IAnsiString *szUserPwd);
	//
	STDMETHOD (GetUserInGroup)(IAnsiString *szGroupId, IAnsiString *szUTF8GrpName);
	//
	STDMETHOD (Offline)();
	//
	STDMETHOD (Logout)();
	//
	STDMETHOD (GetOfflineMsg)();
	//
	STDMETHOD_ (BOOL, GetIsOnline)();
	//
	STDMETHOD (InitPlugins)(HINSTANCE hInstace);
	//
	STDMETHOD (ClearPlugins)();
	//
	STDMETHOD (ChangePresence)(const char *szPresence, const char *szMemo);
	//广播消息
	STDMETHOD (BroadcastMessage)(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData);
	//
	STDMETHOD (GetPresence)(const char *szUserName, IAnsiString *strPresence, IAnsiString *strPresenceMemo);
	//
	STDMETHOD (DoPresenceChanged)(const char *szUserName, const char *szPresence, const char *szMemo, BOOL bOrder);
	//
	STDMETHOD (DoCoreEvent)(HWND hWnd, const char *szWndName, const char *szType, const char *szName, WPARAM wParam, 
		                     LPARAM lParam, HRESULT *hResult);
	//初始化安全连接
	STDMETHOD (InitSafeSocket)();
	//建立安全连接
	STDMETHOD (EstablishSafeSocket)(const char *szSvrHost, USHORT uPort);
	//认证用户
	STDMETHOD (AuthUser)(const char *szUserName, const char *szUserPwd, const char *szPresence, 
		                   const char *szPresenceMemo);
	//
	STDMETHOD (InitConfigure)(const char *szCfgFileName);
	//
	STDMETHOD (GetSvrParams)(const char *szParamName, IAnsiString *szParamValue, BOOL bRealTime);
	//
	STDMETHOD (AddOrderProtocol)(IProtocolParser *pOrder, const char *szProtoName, const char *szProtoType);
	//
	STDMETHOD (DelOrderProtocol)(IProtocolParser *pOrder, const char *szProtoName, const char *szProtoType);
	//
	STDMETHOD (AddOrderEvent)(ICoreEvent *pEvent, const char *szWndName, const char *szCtrlName, 
		                      const char *szEventType);
	STDMETHOD (DeleteOrderEvent)(ICoreEvent *pEvent, const char *szWndName, const char *szCtrlName, 
		                      const char *szEventType);
    //
	STDMETHOD (GetFrontPendingMsg)(const char *szUserName, const char *szType, IAnsiString *strProtocol, BOOL bPop);
	//获取待处理消息简要信息
	STDMETHOD (GetUserPendMsgTipList)(IUserPendMessageTipList *pList);
	//
	STDMETHOD (GetLastPendingMsg)(IAnsiString *strFromName, IAnsiString *strType);
	//
	STDMETHOD (AddTrayMsgTypeIcon)(const char *szMsgType, HICON hIcon);
	//
	STDMETHOD (StartTrayIcon)(const char *szMsgType, const char *szTip, HICON hIcon);
	//
	STDMETHOD (ShowTrayTipInfo)(const TCHAR *szImageFile, const TCHAR *szTip, const TCHAR *szUrl,
		                        const TCHAR *szCaption, ICoreEvent *pEvent);
	STDMETHOD (PickPendingMessage)();
	//权限相关 是否有权限
	STDMETHOD (CanAllowAction)(int nAction);
	//IProtocolParser
	STDMETHOD (DoRecvProtocol)(const BYTE *pData, const LONG lSize);
	STDMETHOD (DoPresenceChange)(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder);
private:
	void Clear();
	BOOL GetInterfaceById(CPluginList &Plugins, int nPluginId, const IID &riid, IUnknown **ppvObject);
	BOOL GetEventListByPlgId(CPluginList &Plugins, int nPluginId);
	BOOL GetSkinXmlInst(ICoreEvent *pEvent);
	BOOL SendAuthData();

	//protocol
	BOOL DoSysProtocol(const char *szType, TiXmlElement *pNode);
	//
	BOOL DoMessageProtocol(const char *szType, TiXmlElement *pNode);
	//
	BOOL DoGroupMessage(TiXmlElement *pNode);
	//
	void NotifyErrorEvent(IUnknown *pUnknown, int nErrorNo, const char *szMsg);

	//通知错误信息所有插件
	void NotifyErrorAllPlugin(int nErrorNode, const char *szMsg);
	//
	BOOL RegisterMutex();
private:
	CUserEventList m_EventList;
	COrderUserProtocol m_ProtoList;
	CPendingMsgList m_PendingList;
	HICON m_hLogoIcon;
	HANDLE m_hMutex;
	BOOL m_OfflineDid; //是否正在处理离线消息
	std::map<CAnsiString_, IUnknown *> m_OtherPlugs;
	std::string m_CfgFileName;
	std::string m_strUserName;
	std::string m_strUserPwd;
	std::string m_strPresence;
	std::string m_strPresenceMemo;
	std::string m_strNickName;
	std::string m_strDomain;
	std::map<CAnsiString_, std::string> m_SvrParams; // 
	std::map<CAnsiString_, HICON > m_hAniIconList;
	//Auth Socket 
	CProtoInterfaceSocket *m_pAuthSocket;
	int m_RoleId; //权限
	std::string m_RoleList; //权限表
	BOOL m_bLogon;
	std::string m_strAuthIp;
	USHORT      m_uAuthPort;

	//status
	BOOL m_bIsOnline;   //是否在线
	BOOL m_bLoginSucc; //

	//interface 
    IUIManager  *m_pUIManager;
	ICoreLogin  *m_pCoreLogin;
	IContacts   *m_pContacts;
	IMsgMgr     *m_pMsgMgr;
	IChatFrame  *m_pChatFrame;
	IGroupFrame *m_pGroupFrame;
	IConfigure  *m_pConfigure;
	ITrayMsg    *m_pTrayMsg;
	IMainFrame  *m_pMainFrame;
};

#endif

