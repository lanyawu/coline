#ifndef __CONFIGUREUIIMPL_H____
#define __CONFIGUREUIIMPL_H____

#include <ComBase.h>
#include <Core/CoreInterface.h>
#include <Commonlib/SqliteDBOP.h>
#include <xml/tinyxml.h>
 
class CConfigureUIImpl: public CComBase<>, 
	                    public InterfaceImpl<IConfigureUI>,
						public InterfaceImpl<IProtocolParser>,
						public InterfaceImpl<ICoreEvent>
{
public:
	CConfigureUIImpl(void);
	~CConfigureUIImpl(void);
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
	//
	STDMETHOD (DoRecvProtocol)(const BYTE *pData, const LONG lSize);
	STDMETHOD (DoPresenceChange)(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder);
	//
	STDMETHOD (Navegate2Frame)(const LPRECT lprc, const char *szItem);
	//
	STDMETHOD (ViewUserInfo)(const char *szUserName);
private:
	HRESULT DoMenuCommand(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	void CheckStatusMapUIByCfg(IConfigure *pCfg, BOOL bCommon, const char *szSection, 
		      const char *szParam, const TCHAR *szCtrlName);
	void CheckStatusMapUIToCfg(IConfigure *pCfg, BOOL bCommon, const char *szSection, 
		      const char *szParam, const TCHAR *szCtrlName);
	//
	void EditMapUIByCfg(IConfigure *pCfg, BOOL bCommon, const char *szSection,
		      const char *szParam, const TCHAR *szCtrlName);
	void EditMapUIToCfg(IConfigure *pCfg, BOOL bCommon, const char *szSection,
		      const char *szParam, const TCHAR *szCtrlName);
	//
	void RadioMapUIByCfg(IConfigure *pCfg, BOOL bCommon, const char *szSection,
		       const char *szParam, const TCHAR *szCtrlName);
	void RadioMapUIToCfg(IConfigure *pCfg, BOOL bCommon, const char *szSection,
		       const char *szParam, const TCHAR *szCtrlName);
	//
	void HotKeyMapUIByCfg(IConfigure *pCfg, BOOL bCommon, const char *szSection,
		       const char *szParam, const char *szDefault, const TCHAR *szCtrlName, 
			   const TCHAR *szRadioDefault, const TCHAR *szRadioCustom);
	void HotKeyMapUIToCfg(IConfigure *pCfg, BOOL bCommon, const char *szSection,
		       const char *szParam, const char *szDefault, const TCHAR *szCtrlName, 
			   const TCHAR *szRadioDefault, const TCHAR *szRadioCustom);
	//
	void InitUI();
	//
	void SaveUI();
	//
	void SavePersonUserSetting(IConfigure *pCfg);
	//
	void WriteWindowRun(BOOL bIsRun);
	//
	BOOL IsWindowsRun();
	//
	void InitReplys(IConfigure *pCfg);
	//初始化网络设置
	void InitNetSetting(IConfigure *pCfg);
	//
	void InitSoundFile(IConfigure *pCfg);
	//
	void InitExceptContactTip(IConfigure *pCfg);
	//
	void SelectSoundListChanged();
	//
	void AddContactTip();
	//
	void DelContactTip();
	//
	void InitPersonInfo(const char *szUserName);
	//
	void SetPersonEnable(BOOL bEnable);
	//
	BOOL DoSysProtocol(const char *pType, TiXmlElement *pNode);
	//
	BOOL UpdateFromServer(const char *szUserName);
	void SavePersonCfg();
	void ShowInfoToUI(const char *szText, int nSize);
	void MapToUI(const char *szText, const TCHAR *szUIName);
	void UIMapToXml(TiXmlElement *pNode, const char *szAttrName, const TCHAR *szUIName);
	void ShowCurrContactCard(HWND hWnd, const TCHAR *szCtrlName);
	//
	void UploadPictureToSvr(HWND hWnd);
	//
	void SetChanged(BOOL bChanged);
private:
	ICoreFrameWork *m_pCore;
	std::string m_strHeaderFileName;
	BOOL m_bChanged;
	HWND m_hWnd;
	HWND m_hWndInfo;
	HWND m_hWndMain; //主窗口HANDLE
};

#endif
