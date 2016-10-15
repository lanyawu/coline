#ifndef __MSGMGRUIIMPL_H___
#define __MSGMGRUIIMPL_H___
#include <ComBase.h>
#include <Core/CoreInterface.h>
#include <Commonlib/stringutils.h>
#include <Commonlib/SqliteDBOP.h>
#include <xml/tinyxml.h>


class CMsgMgrUIImpl :public CComBase<>,
	                 public InterfaceImpl<ICoreEvent>,
					 public InterfaceImpl<IMsgMgrUI>
{
public:
	CMsgMgrUIImpl(void);
	~CMsgMgrUIImpl(void);
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
	//广播消息
	STDMETHOD (DoBroadcastMessage)(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData);
	//
	STDMETHOD (DoWindowMessage)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes);

	//IMsgMgrUI
	STDMETHOD (ShowMsgMgrFrame)(const char *szType, const char *szInitUserName, LPRECT lprc);
private:
	void GetLogonUserName(std::string &strUserName); //获取已经登陆的用户名
	void Navigate2(const char *szType, const char *szUserName);
	void LoadContacts(void *pNode, const char *szType, const char *szUserName);
	BOOL ExpandNodeByPid(void *pParentNode, const int nPid);
	BOOL DataListDrawToUI(IUserList  *pDataList, void *pParentNode, 
	                                  int nSaveNodeId, void **pSaveNode);
	void ShowHistoryMsg(const char *szType, const char *szUserName);
	void SearchHistoryMsg(const char *szType, const char *szUserName, const char *szKey);
	void DisplayHistoryMsg(const int nPage);
	void RefreshWhoTip();
	void DisplayMessageList(IMessageList *pMsgList); 
	BOOL DoMenuCommand(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	//depts 38,12,0 format
	void LoadContactsByDepts(void *pParentNode, const char *szDepts);
	//
	void ShowMgrFrameByHWND(HWND hWnd);
	//
	void ShowMgrFrameByTree(HWND hWnd);
	//
	void RefreshByTreeNode(HWND hWnd);
	//
	void DeleteMsgByTreeNode(HWND hWnd);
	//
	BOOL RECallBack(HWND hWnd, DWORD dwEvent, char *szFileName, DWORD *dwFileNameSize,
			  char *szFileFlag, DWORD *dwFileFlagSize, DWORD dwFlag);
	//
	const char *GetImagePath();
	//
	void ExpandGroupTree(void *pParentNode);
	//
	void ExpandSmsTree(void *pParentNode);
	//
	
	BOOL ExportP2PMsgToFile(const char *szUserName, int nFileType, const char *szFileName);
	//
	BOOL InsertMsgNode(TiXmlElement *pNode, const int Id, IFontStyle *pfs, const char *szFromId, 
		const char *szFrom, const char *szToId, const char *szTo, const char *szTime, const char *szText); 
	//
	void ExportCurrentNodeToFile(int nFileType);
	//
	void DeleteCurrentMsg(HWND hWnd);
	BOOL ImportMsgFile(const char *szFileName);
	BOOL DoImportMsg(IMsgMgr *pMgr, const char *szType, TiXmlElement *pNode);
private:
	ICoreFrameWork *m_pCore;
	HWND m_hWnd;
	CStdString_ m_strTipName;
	int m_nTotalPage;
	int m_nCurrPage;
	std::string m_strImagePath;
	std::string m_strCurrType;
	std::string m_strUserName;
	std::string m_strKey;
};

#endif
