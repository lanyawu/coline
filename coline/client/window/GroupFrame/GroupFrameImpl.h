#ifndef __GROUPFRAME_IMPL_H___
#define __GROUPFRAME_IMPL_H___
#define WIN32_LEAN_AND_MEAN	
#include <ComBase.h>
#include <smartskin/smartskin.h>
#include <Commonlib/stringutils.h>
#include <Core/CoreInterface.h>
#include <Commonlib/SqliteDBOP.h>
#include <Commonlib/GuardLock.h>
#include <xml/tinyxml.h>

#include "../IMCommonLib/InstantUserInfo.h"
#include "../IMCommonLib/InterfaceUserList.h"
#include "../IMCommonLib/TransferFileList.h"
#include "GroupItem.h"

class CGroupFrameImpl: public CComBase<>,
	                   public InterfaceImpl<ICoreEvent>,
				       public InterfaceImpl<IProtocolParser>,
					   public InterfaceImpl<IGroupFrame>  
{
public:
	CGroupFrameImpl(void);
	~CGroupFrameImpl(void);
	friend void CALLBACK HttpUpCallBack(int nErrorCode, int nType, WPARAM wParam, LPARAM lParam, void *pOverlapped);
	friend void CALLBACK HttpDlCallBack(int nErrorCode, int nType, WPARAM wParam, LPARAM lParam, void *pOverlapped);
public:
	//IUnknown
	STDMETHOD (QueryInterface)(REFIID riid, LPVOID *ppv);
	//ICoreEvent
	STDMETHOD (DoCoreEvent)(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
		                     LPARAM lParam, HRESULT *hResult);
	STDMETHOD (SetCoreFrameWork)(ICoreFrameWork *pCore);
	//广播消息
	STDMETHOD (DoBroadcastMessage)(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData);
	STDMETHOD (GetSkinXmlString)(IAnsiString *szXmlString);
	//
	STDMETHOD (CoreFrameWorkError)(int nErrorNo, const char *szErrorMsg);
	//
	STDMETHOD (DoWindowMessage)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes);
	//根据讨论组ID获取名称 
	STDMETHOD (GetGroupNameById)(const char *szGrpId, IAnsiString *strGrpName);
	//IProtocolParser
	STDMETHOD (DoRecvProtocol)(const BYTE *pData, const LONG lSize);
	STDMETHOD (DoPresenceChange)(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder);
	//IGroupFrame
	STDMETHOD (ShowGroupFrame)(const char *szGrpId, const char *szUTF8DspName);
	//
	STDMETHOD (DrawGroupToUI)(HWND hWnd, const char *szGrpId);
	//
	STDMETHOD (ShowGroupMessage)(const char *szGrpId, const char *szMsg);
	//发送文件至群
	STDMETHOD (SendFileToGroup)(const char *szGrpId, const char *szFileName);
	//发送短信至群
	STDMETHOD (SendSMSToGroup)(const char *szGrpId, const char *szSMSText);
	//
	STDMETHOD (SendMailToGroup)(const char *szGrpId);
	STDMETHOD (ShowGroupTipMsg)(HWND hOwner, TCHAR *szMsg);
	STDMETHOD (GetGroupIdByHWND)(HWND hOwner, IAnsiString *strGroupId);
	STDMETHOD (ParserGroupProtocol)(const char *szContent, const int nContentSize, IAnsiString *strDispName,  IAnsiString *strUserList,
	                          IAnsiString *strDspTime, IAnsiString *strDspText, IFontStyle *fs, IAnsiString *strMsgType,
							  BOOL *bSelf);
	//创建分组
	STDMETHOD (CreateGroup)(const char *szGrpId, const char *szGrpName, IUserList *pUser);
	//修改分组名称
	STDMETHOD (UpdateGroupName)(const char *szGrpId, const char *szNewGrpName);
	//退出分组
	STDMETHOD (ExitGroupById)(const char *szGrpId);
	//删除分组
	STDMETHOD (DeleteGroupById)(const char *szGrpId); 
public:
	BOOL RECallBack(HWND hWnd, DWORD dwEvent, char *szFileName, DWORD *dwFileNameSize,
			  char *szFileFlag, DWORD *dwFileFlagSize, DWORD dwFlag);
private:
	BOOL IsGroupManager(const char *szGrp);
	void ClearGroupItems();
	BOOL CheckInputChars(const char *p);
	BOOL GetPendingMsgByName(HWND hWnd, const char *szUserName);
	void DeleteGrpFromMainWindow(const char *szGrpId);
	BOOL OpenGroupFrame(const char *szGrpId, const char *szUTF8DspName);
	void UpdateGroupOnlineInfo(HWND hWnd);
	void RefreshLastOpenFrameRect();
	void RefreshInputChatFont(HWND hWnd, IConfigure *pCfg);
	void LoadGroupUser(HWND hWnd, CGroupItem *pItem);
	void DrawGroupToTree(HWND hWnd, CGroupItem *pItem);
	BOOL DoCustomLink(HWND hWnd, DWORD dwFlag);
	void CancelCustomLink(HWND hWnd, DWORD dwFileId, DWORD dwFlag);
	BOOL AnsycShowTip(HWND hWnd, const TCHAR *szTip);
	BOOL ParserGroupProtocol(const char *pType, TiXmlElement *pNode);
	BOOL SaveGrpMsg(const char *szType, const char *szFromName, const char *szToName,
		                 const char *szTime, const char *szMsg, const char *szBody);
	//
	BOOL ParserGroupProtocol2(const char *pType, TiXmlElement *pNode);
	//
	HRESULT DoMenuCommand(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	//
	HRESULT DoClickEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	//
	HRESULT DoDblClickEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	//
	HRESULT DoItemSelectEvent(HWND hWnd, const char *szCtrlName, WPARAM wParam, LPARAM lParam);
	//
	HRESULT DoInitMenuPopup(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	//
	HRESULT DoLinkEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	void DoCreateGroupMenu();
	//
	BOOL DoGroupMsg(TiXmlElement *pNode);
	//
	BOOL SendMessageToPeer(HWND hWnd);
	//
	BOOL DoSendPicture(HWND hWnd);
	//
	CGroupItem *GetGroupItemByHWND(HWND hWnd);
	//
	CGroupItem *GetGroupItemById(const char *szGrpId);
	//
	const char *GetImagePath();
	//
	BOOL CanClosed(HWND hWnd);
	//
	void InitSelfUserInfo();
	//
	void DoRecvExitGroup(const char *szGrpId, const char *szUid);
	//
	BOOL DoReviseGroup(TiXmlElement *pNode);
	//
	BOOL DoDeleteGroup(const char *szGrpId, const char *szResean, BOOL bSucc);
	//
	BOOL RecvMessageFromGroup(HWND hWnd, TiXmlElement *pNode, const char *szDspName);
	void ParserGroupMessage(TiXmlElement *pNode, std::string strGrpId,  std::string &strFrom, std::string &userList, std::string &strTime,
		                   std::string &strDspName,  std::string &strBody, CCharFontStyle &fs);
	//
	BOOL DoSysProtocol(const char *pType, TiXmlElement *pNode);
	//
	void InitGroupFrame(HWND hWnd);
	//
	void DoGroupCustomPic(TiXmlElement *pNode);
	//
	void DoRecvOfflineFile(TiXmlElement *pNode);
	//
	void DoCutScreen(HWND hWnd, BOOL bHide);
	//common function
	void OpenChatFrame(HWND hWnd);
	//
	void OpenGroupFrameAction(HWND hWnd); 
	//退出分组
	void ExitGroupAction(HWND hWnd);
	//解散分组
	void DeleteGroupAction(HWND hWnd);
	//
	void ShowMiniCard(HWND hWnd);
	//
	void ModifyGroupName(HWND hWnd);
	//
	BOOL InputGroupName(HWND hWnd, const TCHAR *szTitle, std::string &strNewGrpName);
	//
	void DoGroupUserSet(HWND hWnd);
	//
	void DoCreateGroup(HWND hWnd);
	HWND GetMainFrameHWND();
	//
	void RemoveUserByHWND(HWND hWnd, const char *szUserName);
	//
	void OnWMDropFiles(HWND hWnd, WPARAM wParam, LPARAM lParam);
	//
	BOOL UploadCustomPicToServer(HWND hWnd, const char *szFlag);
	BOOL SendOleResourceToPeer(HWND hWnd);
	BOOL DoEmotionPanelClick(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL SendFileToGroup(HWND hWnd, const char *szFileName);
	void RemoveTransFile(int nFileId, const char *szTip);
	void RemoveTransFileProgre(int nFileId, const char *szTip, BOOL bPost);
	//
	void PlayTipSound(const char *szType, const char *szUserName, BOOL bLoop);
	//
	void Invitation(HWND hWnd, IUserList *ulSrcUsers);
	//
	void ShowFileLink(HWND hWnd, const char *szTip, const char *szFileName);
	//
	void GetUserListDisplayNameById(IAnsiString *strDspNames, std::string &userList, std::string &strGuid);
	//
	void GetUserListByItem(CGroupItem *pItem, std::string &strUserList);
	//
	BOOL FileRecvEvent(HWND hOwner, const char *szFileFlag);
	BOOL FileSendOfflineEvent(HWND hOwner, const char *szFileFlag);
	BOOL FileCancelEvent(HWND hOwner, const char *szFileFlag);
	BOOL FileSaveAsEvent(HWND hOwner, const char *szFileFlag);
	BOOL FileRecvToLocal(HWND hOwner, CTransferFileInfo &FileInfo, const char *szLocalFileName);
private:
	std::map<CAnsiString_, CGroupItem *> m_FrameList;
	std::string m_strFileTransSkinXml;
	HWND m_hMainWnd;
	CCustomPicItemList m_CustomPics;
	CTransferFileList m_TransFileList;
	std::string m_strImagePath;
	std::string m_strUserName;
	std::string m_strRealName;
	ICoreFrameWork *m_pCore; 
	RECT m_rcLastOpen;
	BOOL m_bInitFrame;
	BOOL m_bEnterSend;
	//
	std::string m_strNewGrpName;
	//讨论组根节点
	void *m_pGrpRoot;
};

#endif
