#ifndef __BCFRAME_IMPL_H___
#define __BCFRAME_IMPL_H___
#define WIN32_LEAN_AND_MEAN	
#include <ComBase.h>
#include <map>
#include <string>
#include <smartskin/smartskin.h>
#include <Core/CoreInterface.h>
#include <Commonlib/SqliteDBOP.h>
#include <Commonlib/GuardLock.h>
#include <xml/tinyxml.h>

#include "../IMCommonLib/InstantUserInfo.h"
#include "../IMCommonLib/InterfaceUserList.h"
#include "../IMCommonLib/TransferFileList.h"

class CBCFrameImpl: public CComBase<>,
	                 public InterfaceImpl<ICoreEvent>,
				     public InterfaceImpl<IProtocolParser>,
					 public InterfaceImpl<IBCFrame> 
 
{
public:
	CBCFrameImpl(void);
	~CBCFrameImpl(void);
	friend void CALLBACK HttpUpCallBack(int nErrorCode, int nType, WPARAM wParam, LPARAM lParam, void *pOverlapped);
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
	//
	STDMETHOD (DoBroadcastMessage)(const char *szFromWndName, HWND hWnd, const char *szType, 
		                           const char *szContent, void *pData);
	//
	STDMETHOD (DoWindowMessage)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes);
	//IProtocolParser
	STDMETHOD (DoRecvProtocol)(const BYTE *pData, const LONG lSize);
	STDMETHOD (DoPresenceChange)(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder);
	//IBCFrame
	STDMETHOD (ShowBCFrame)(const TCHAR *szCaption, LPRECT lprc, int nStyle, IUserList *pUsers);
private:
	HRESULT DoMenuCommand(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	HRESULT DoClickEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	HRESULT DoStatusChanged(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	HRESULT DoItemSelectEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	LRESULT OnWMDropFiles(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void BroadcastFile(const char *szFileName);
	void DoContactTreeChanged(HWND hWnd);
	BOOL DoAdjustSelNode(IContacts *pContact, HWND hWnd, const char *szUserName, const TCHAR *szDspName, BOOL bAdd);
	void DoEmotionClick(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void RefreshInputChatFont(HWND hWnd, IConfigure *pCfg);
	void BroadCastMessage();
	BOOL SendMessageToPeer(HWND hWnd);
	BOOL SendSms(HWND hWnd);
	BOOL SendFilesToUsers(HWND hWnd);
	void BroadcastFileMsg(const char *szFileMsg);
	void SaveMessage(const char *szType, TiXmlElement *pNode);
	void DoCreateGroupMenu(const TCHAR *szCaption, int nStyle);

	//
	BOOL SendOleResourceToPeer(HWND hWnd);
	BOOL UploadCustomPicToServer(HWND hWnd, const char *szFlag);
	BOOL UploadLocalFileToServer(HWND hWnd, const char *szLocalFileName);
	const char *GetImagePath();
	void DelTreeSelectedUsers(HWND hWnd);
	void DeleteAllSelFile();
	BOOL DoSendPicture(HWND hWnd);
	BOOL CutScreen(HWND hWnd, BOOL bHide);

	//
	BOOL RECallBack(HWND hWnd, DWORD dwEvent, char *szFileName, DWORD *dwFileNameSize,
			  char *szFileFlag, DWORD *dwFileFlagSize, DWORD dwFlag);
private:
	int m_nStyle;
	std::map<std::string, std::string> m_PhoneTable;
	CTransferFileList m_TransFileList;
	std::string m_strUserName;
	ICoreFrameWork *m_pCore;
	std::string m_strFileTransSkinXml;
	std::string m_strImagePath;
	CInterfaceUserList m_SelList; 
	CCustomPicItemList m_CustomPics;
	CGuardLock m_PendLock;
	BOOL m_bEnterSend;
	BOOL m_bInitFrame;
	HWND m_hWnd;
};

#endif

