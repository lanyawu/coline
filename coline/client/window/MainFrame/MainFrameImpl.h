#ifndef __MAINFRAMEIMPL_H_____
#define __MAINFRAMEIMPL_H_____

#define WIN32_LEAN_AND_MEAN	
#include <ComBase.h>
#include <Core/CoreInterface.h>
#include <xml/tinyxml.h>

#define TRY_LOGON_TIMER_INTERVAL 5000  //�����볢��һ��
#define CHECK_ACTIVE_TIME_INTERVAL 2000 //2����һ���Զ�ת��

class CMainFrameImpl: public CComBase<>, 
	                  public InterfaceImpl<IMainFrame>,
					  public InterfaceImpl<ICoreEvent>,
					  public InterfaceImpl<IProtocolParser>
{
public:
	CMainFrameImpl(void);
	~CMainFrameImpl(void);
	friend void CALLBACK BannerDlCallback(int nErrorCode, int nType,
		              WPARAM wParam, LPARAM lParam, void *pOverlapped);
public:
	//IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppv);

	//ICoreEvent
	STDMETHOD (DoCoreEvent)(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
		                     LPARAM lParam, HRESULT *hResult);
	STDMETHOD (SetCoreFrameWork)(ICoreFrameWork *pCore);
	STDMETHOD (GetSkinXmlString)(IAnsiString *szXmlString);
	//�㲥��Ϣ
	STDMETHOD (DoBroadcastMessage)(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData);
	//
	STDMETHOD (CoreFrameWorkError)(int nErrorNo, const char *szErrorMsg);
	//
	STDMETHOD (DoWindowMessage)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes);

	//IMainFrame
	STDMETHOD (BringToFront)(); 
	//
	STDMETHOD (ShowMainFrame)();
	//
	STDMETHOD (InitMainFrame)();
	//
	STDMETHOD (ShowContacts)();
	//
	STDMETHOD (UpdateUserLabel)(const char *szUserName,  const char *szUTF8Label);
	//
	STDMETHOD (UpdateUserPresence)(const char *szUserName, const char *szPresence, BOOL bSort);
	//
	STDMETHOD (ShowRecentlyUser)(const char *szUserName, const char *szDispName);
	//
	STDMETHOD_(HWND, GetSafeWnd)();
	//
	//
	STDMETHOD (UpdateHotKey)(int nType);
	//

	//
	STDMETHOD (DoRecvProtocol)(const BYTE *pData, const LONG lSize);
	//
	STDMETHOD (DoPresenceChange)(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder);
private:
	void DoSysMenuCommand(WPARAM wParam);
	void DoMainMenuCommand(WPARAM wParam);
	void DoListMenuCommand(WPARAM wParam, LPARAM lParam);
	void DoTreeGroupMenuCommand(WPARAM wParam, LPARAM lParam);
	void DoRecentlyMenuCommand(HWND hWnd, WPARAM wParam, LPARAM lParam); 
	void DoSearchMenuCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void DoOpenChatFrameByRecent();
	BOOL GetUserNameByRecent(std::string &strUserName, BOOL &bGroup);
	HRESULT DoDblClickEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam); 
	void DoUpdateSign();
	void GetBannerImage();
	BOOL ExpandNodeByPid(void *pParentNode, const int nPid);
	BOOL DoStatusOrder(HWND hWnd, void *pNode);
	void UpdateStatusToUI(const char *szPresence, const TCHAR *szMemo);
	BOOL GetDisplayTextById(const char *szUserName, TCHAR *szDisplayText);
	//
	void ModifyShiftColor(COLORREF cr);
	//
	void DoOffline();
	//
	BOOL DoSysProtocol(TiXmlElement *pNode, const char *pType);
	//
	void OnWMPowerBroadcast(WPARAM wParam, LPARAM lParam);
	//ע���ȼ�
	void RegisterGoComHotkey();

	//
	void DeleteHotKey();
	// 
	void RefreshBanner();
private:
	HWND m_hWnd;
	ICoreFrameWork *m_pCore;
	UINT_PTR m_ptrTimer;
	std::string m_strUserName;
	int m_nTryTimes;  //���µ�½���Դ��� 
	
	//�Զ�ת��״̬
	UINT_PTR m_ptrAutoChgTimer;
	BOOL m_bAutoChanged; //�Ƿ��Ѿ�ִ�й��Զ�ת��

	//Hotkey
	int m_dwAtomRecvHotkeyId;  //��ȡ��Ϣhotkeyԭ��
	int m_dwRecvHotkeyShiftState; //��ȡ��Ϣshift ״̬
	int m_dwRecvHotkeyKeyStatue;  //��ȡ��Ϣ����״̬
	//
	int m_dwAtomCutScrHotkeyId; //����hotkeyԭ��
	int m_dwCutScrHotkeyShiftState; //����shift״̬
	int m_dwCutScrHotkeyKeyState; //��������״̬
	//
	int m_nSearchStatus;  //�������״̬
	//
	std::string m_strBannerPicUrl;
	std::string m_strBannerClickUrl;
	std::string m_strBannerLocalFile;
};

#endif
