#ifndef __CHAT_FRAME_IMPL_H____
#define __CHAT_FRAME_IMPL_H____
#define WIN32_LEAN_AND_MEAN	
#include <map>
#include <ComBase.h>
#include <SmartSkin/smartskin.h>
#include <Core/CoreInterface.h>
#include <Commonlib/stringutils.h>
#include <xml/tinyxml.h>
#include "UserChatFrame.h"
#include "../IMCommonLib/TransferFileList.h"

const TCHAR UI_COMPANY_TREE_NAME_WCHAR[] = L"colleaguetree";
const char  UI_COMPANY_TREE_NAME_STR[] = "colleaguetree";
const TCHAR UI_CHARFRAME_INPUT_EDIT_NAME[] = L"messageedit";
const TCHAR UI_CHATFRAME_DISPLAY_EDIT_NAME[] = L"messagedisplay";
#define MEDIA_EXTRACT_WIDTH       150

//聊天窗口命令定义
#define CHAT_COMMAND_TAB2RMC_REQUEST   1  //切换至远程协助的请求界面
#define CHAT_COMMAND_TAB2RMC_CONTROL   2  //切换至远程协助的控制界面
#define CHAT_COMMAND_TAB2FILE_PRO      3  //切换至文件传输进度界面
#define CHAT_COMMAND_TAB2INFO          4  //切换至信息显示界面
#define CHAT_COMMAND_CLEAR_RMC_CHL     5  //清除远程协助相关数据
#define CHAT_COMMAND_TAB2RMC_SHOW      6  //切换至远程协助显示界面
#define CHAT_COMMAND_TAB2RMC_SWCTRL    7  //显示申请控制
#define CHAT_COMMAND_TAB2AV            8  //切换到音视频显示界面
class CRmcPeerInfomation
{
public:
	std::string m_strPeerInternetIp;
	std::string m_strPeerIntranetIp;
	std::string m_strKey;
	std::string m_strPeerName;
	WORD m_wPeerInternetPort;
	WORD m_wPeerIntranetPort;
	int m_nWidth;
	int m_nHeight;
	int  m_nChlId;
	DWORD m_dwShowSet;
	RECT m_rcSet;
	HWND m_hOwner;
	HWND m_hFullScreen;
	HWND m_hSetWnd;
	BOOL m_bRequest; //请求方
	BOOL m_bConnected; //已经连接成功
};

class CVideoPeerInfomation
{
public:
	std::string m_strPeerInternetIp;
	std::string m_strPeerIntranetIp;
	std::string m_strKey;
	std::string m_strPeerName;
	WORD m_wPeerInternetPort;
	WORD m_wPeerIntranetPort;
	int  m_nChlId;
	HWND m_hOwner;
};

class CChatFrameImpl: public CComBase<>,
	                  public InterfaceImpl<IChatFrame>,
					  public InterfaceImpl<IProtocolParser>,
					  public InterfaceImpl<ICoreEvent>
{
public:
	CChatFrameImpl(void);
	~CChatFrameImpl(void);
	friend void CALLBACK HttpUpCallBack(int nErrorCode, int nType, WPARAM wParam, LPARAM lParam, void *pOverlapped);
	friend void CALLBACK HttpDlCallBack(int nErrorCode, int nType, WPARAM wParam, LPARAM lParam, void *pOverlapped);
public:
	//IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppv);

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

	//IProtocolParser
	STDMETHOD (DoRecvProtocol)(const BYTE *pData, const LONG lSize);
	STDMETHOD (DoPresenceChange)(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder);

	//IChatFrame
	STDMETHOD_ (HWND, ShowChatFrame)(HWND hWndFrom, const char *szUserName, const char *szUTF8DspName);
	//
	STDMETHOD (ShowChatMessage)(HWND hOwner, const char *szMsg);
	STDMETHOD (SendFileToPeer)(const char *szUserName, const char *szFileName);
	STDMETHOD (SendRmcRequest)(const char *szUserName);
	STDMETHOD (SendVideoRequest)(const char *szUserName);
	STDMETHOD (SendAudioRequest)(const char *szUserName);
	//
	STDMETHOD (ShowChatTipMsg)(HWND hOwner, const TCHAR *szMsg);
	//
	STDMETHOD (VideoConnected)(LPARAM lParam, WPARAM wParam);
	//
	STDMETHOD (GetUserNameByHWND)(HWND hOwner, IAnsiString *strUserName);
	//
	STDMETHOD (ParserP2PProtocol)(const char *szContent, const int nContentSize, IAnsiString *strDispName,
		                          IAnsiString *strDspTime, IAnsiString *strDspText, IFontStyle *fs, IAnsiString *strMsgType,
								  BOOL *bSelf);
public:
	BOOL RECallBack(HWND hWnd, DWORD dwEvent, char *szFileName, DWORD *dwFileNameSize,
			  char *szFileFlag, DWORD *dwFileFlagSize, DWORD dwFlag);
	BOOL GetTransferFileInfoById(const int nFileId, CTransferFileInfo &FileInfo);
	void TransFileProgress(const DWORD dwErrorNo, const DWORD dwFileId, const LPARAM lParam, 
												 const WPARAM wParam);
	void DoUserSignChanged(const char *szUserName);
	void RcmCallback(const DWORD dwErrorNo, const DWORD dwRmcId, const WPARAM wParam, const LPARAM lParam);
	void VideoNotify(const DWORD dwErrorNo, const DWORD dwChlId, const LPARAM lParam, const WPARAM wParam);
private:
	BOOL SendFileToPeer(HWND hWnd, const char *szLocalFileName);
	void CloseRmcFullWindow();
	void ClearRmcChannel();
	BOOL CheckInputChars(const char *p);
	void RemoveTransFile(int nFileId, const char *szTip, BOOL bAnsy, const char *szPeerName = NULL, BOOL bByPeer = FALSE);
	void RemoveTransFileProgre(int nFileId, const char *szTip, BOOL bPost);
	BOOL CanClosed(HWND hWnd);
	void GetChatFrameResource(const char *p, int nSize);
	BOOL AnsycShowTip(HWND hOwner, const TCHAR *szTipMsg);
	BOOL AnsyShowFileLink(HWND hOwner, const char *szTipMsg, const char *szFileName);
	void CancelCustomLink(HWND hWnd, DWORD dwFileId, DWORD dwFlag);
	void ClearChatFrame();
	BOOL DoCustomLink(HWND hWnd, DWORD dwFlag);
	HWND OpenChatFrame(HWND hWndFrom, const char *szUserName, const char *szUTF8DspName);
	BOOL GetPendingMsgByName(HWND hWnd, const char *szUserName, const char *szDspName);
	BOOL SendMessageToPeer(HWND hWnd);
	BOOL SendReceiptToPeer(const char *szUserName, const char *szText, const char *szType);
	BOOL SendOleResourceToPeer(HWND hWnd);
	HWND GetHWNDByUserName(const char *szUserName);
	const char *GetRealNameByHWND(HWND hWnd);
	BOOL RecvMessageFromPeer(HWND hWnd, TiXmlElement *pNode, const char *szDspName);
	void ParserP2PMessage(TiXmlElement *pNode, std::string &strFrom, std::string &strTime,
		                  std::string &strTo, std::string &strReceipt, std::string &strDspName,
						  std::string &strBody, CCharFontStyle &fs);
	void RefreshLastOpenFrameRect();
	HWND GetMainFrameHWND();
	void SaveMessage(const char *szType, TiXmlElement *pNode);
	BOOL SaveP2PMsg(const char *szType, const char *szFromName, const char *szToName,
		                 const char *szTime, const char *szMsg, const char *szBody);
	CUserChatFrame *GetChatFrameByHWND(HWND hChat);
	const char *GetImagePath();
	//click event
	BOOL CutScreen(HWND hWnd, BOOL bHide);
	//do protocol
	BOOL DoMessageProtocol(const char *szType, TiXmlElement *pNode);
	//
	BOOL DoTransferProtocol(const char *szType, TiXmlElement *pNode);
	//
	BOOL DoSystemProtocol(const char *szType, TiXmlElement *pNode);
	//
	//Event
	LRESULT DoClickEvent(HWND hWnd, const char *szCtrlName, WPARAM wParam, LPARAM lParam);
	//
	LRESULT DoDblClkEvent(HWND hWnd, const char *szCtrlName, WPARAM wParam, LPARAM lParam);
	//
	LRESULT DoItemSelectEvent(HWND hWnd, const char *szCtrlName, WPARAM wParam, LPARAM lParam);
	//
	LRESULT DoLayoutEvent(HWND hWnd, const char *szCtrlName, WPARAM wParam, LPARAM);
	//
	LRESULT DoMenuClickEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	//
	LRESULT DoInitMenuPopup(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	//
	LRESULT DoAfterInitMenu(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	//
	LRESULT DoLinkEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	//
	LRESULT DoWindowPosChanged(HWND hWnd, WPARAM wParam, LPARAM lParam);
	//
	LRESULT OnWMDropFiles(HWND hWnd, WPARAM wParam, LPARAM lParam);
 
	//远程协助相关 ==> 接收对方的远程协助请求
	BOOL DoAcceptControl(HWND hOwner);
	//远程协助相关 ==>拒绝对方的远程协助请求
	BOOL DoRefuseControl(HWND hOwner);
	//
	LRESULT DoAfterInitEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	void RefreshInputChatFont(HWND hWnd, IConfigure *pCfg);
	void ExtractWindowWidth(HWND hWnd, const TCHAR *szName, int nExtractW);
	BOOL UploadLocalFileToServer(HWND hWnd, const char *szLocalFileName);
	BOOL UploadCustomPicToServer(HWND hWnd, const char *szFlag);
	BOOL DoCustomPicProtocol(TiXmlElement *pNode);
	HWND OpenChatFrameByCollegeTree(HWND hWnd);
	BOOL SendMailToPeerByTreeMenu(HWND hWnd);
	BOOL SelectFileAndSend(HWND hWnd);
	BOOL SendRtoRequest(HWND hWnd);
	BOOL SendVideoRequest(HWND hWnd);
	BOOL DoSendPicture(HWND hWnd); 
	BOOL DoEmotionClick(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void RmcFullScreen();
	void SetRmcParam(HWND hWnd);
	void PlayTipSound(const char *szType, const char *szUserName, BOOL bLoop);
	//
	BOOL FileRecvEvent(HWND hOwner, const char *szFileFlag);
	BOOL FileSendOfflineEvent(HWND hOwner, const char *szFileFlag);
	BOOL FileCancelEvent(HWND hOwner, const char *szFileFlag);
	BOOL FileSaveAsEvent(HWND hOwner, const char *szFileFlag);
	BOOL FileRecvToLocal(HWND hOwner, CTransferFileInfo &FileInfo, const char *szLocalFileName);
	//
	void AllFileRecv(HWND hWnd);
	void AllFileSaveAs(HWND hWnd);
	void AllFileSaveToPath(HWND hWnd, const char *szPath);
	void AllFileRefuse(HWND hWnd);
	//
	void ChangeAllFileSetVisible(HWND hWnd);

	void RMFileProgress(WPARAM wParam, LPARAM lParam);
	//
	void DoChatCommand(HWND hOwner, int nCommand);
	//
	void TerminatedTransFileByHWND(HWND hOwner, BOOL bAnsy);
	//
	void DoRefuseVideoRequest(HWND hOwner);
	//
	void SendCancelVideoProtocol(const char *szPeerName, const char *szChlId, const char *szReason);
	//
	void SendAuthRequest(const char *szUserName);
	//
	void DoAuthRequestFrom(const char *szUserName, const char *szTime, const char *szText);
	//
	void InitSelfChatFrame();
	//
	void SendSmsByHWND(HWND hWnd);
private:
	std::map<HWND, CUserChatFrame *> m_ChatFrameList;
	CTransferFileList m_TransFileList;
	CCustomPicItemList m_CustomPics;
	DWORD m_dwLastShake;
	BOOL m_bInitFrame;
	std::string m_strImagePath;
	std::string m_strRealName;
	std::string m_strUserName;
	std::string m_strTransferPort;
	std::string m_strTransferIp;
	std::string m_strFileTransSkinXml;
	ICoreFrameWork *m_pCore;
	HWND m_hMainFrame;
	RECT m_rcLastOpen;
	BOOL m_bEnterSend;
	//rmc 
	int m_rmcChlId;
	CRmcPeerInfomation m_rmcInfo; 

	//video
	int m_VideoChlId;
	CVideoPeerInfomation m_VideoInfo; 
};

#endif

