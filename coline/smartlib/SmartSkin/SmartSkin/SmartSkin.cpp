#include <fstream>
#include <string>
#include <map>
#include <commonlib/stringutils.h>
#include <commonlib/debuglog.h>
#include <commonlib/guardlock.h>
#include <SmartSkin/SmartSkin.h>
#include "SmartUIResource.h"
#include "SmartWindow.h"
#include "SmartTipWnd.h"

static std::map<HWND, CSmartWindow *> m_WinList;

BOOL CPaintManagerUI::m_bApplicationTerminated = FALSE;
CWindowWnd *CPaintManagerUI::m_pMainForm = NULL;

CGuardLock m_WinLock;

#pragma warning(disable:4996)

void CALLBACK WinDestroyEvent(HWND hWnd)
{
	std::map<HWND, CSmartWindow *>::iterator it;
	for (it = m_WinList.begin(); it != m_WinList.end(); it ++)
	{
		if (it->first == hWnd)
		{
			m_WinList.erase(it);
			break;
		}
	}
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
			{	
				::InitCommonControls();
				HMODULE h = ::GetModuleHandle(NULL); 			
				CPaintManagerUI::SetResourceInstance((HINSTANCE)h);  
			}
			 break;
		case DLL_THREAD_ATTACH:
			 break;
		case DLL_THREAD_DETACH:
			 break;
		case DLL_PROCESS_DETACH:
			 SkinDestroyResource();
			 break;
    }
    return TRUE;
}

void UpdateAllWindow(BYTE r, BYTE g, BYTE b)
{
	std::map<HWND, CSmartWindow *>::iterator it;
	for (it = m_WinList.begin(); it != m_WinList.end(); it ++)
	{
		it->second->SetBackGround(r, g, b);
		it->second->Update();
	}
}

CSmartWindow *FindWindowByName(const char *szName)
{
	CSmartWindow *pRes = NULL;
	std::map<HWND, CSmartWindow *>::iterator it;
	for (it = m_WinList.begin(); it != m_WinList.end(); it ++)
	{
		if (::stricmp(it->second->GetWindowName().c_str(), szName) == 0)
		{
			pRes = it->second;
			break;
		}
	}
	return pRes;
}

BOOL  CALLBACK SkinSetCanActived(HWND hWindow, BOOL bCanActived)
{
	std::map<HWND, CSmartWindow *>::iterator it = m_WinList.find(hWindow);
	if (it != m_WinList.end())
	{
		it->second->SetIsActive(bCanActived);
		return TRUE;
	}
	return FALSE;
}

BOOL  CALLBACK SkinCloseWindow(HWND hWindow)
{
	std::map<HWND, CSmartWindow *>::iterator it = m_WinList.find(hWindow);
	if (it != m_WinList.end())
	{
		it->second->Close();
		return TRUE;
	}
	return FALSE;
}
CSmartWindow *FindWindowByHWND(HWND hWnd)
{
	std::map<HWND, CSmartWindow *>::iterator it = m_WinList.find(hWnd);
	if (it != m_WinList.end())
		return it->second;
	return NULL;
}

//检测配环境
BOOL CALLBACK SkinCheckRunOption()
{
	//COM初始化
	HRESULT Hr = ::CoInitialize(NULL);
	if (FAILED(Hr))
		return FALSE;
	//DirectX9检测
	if( ::LoadLibrary(_T("d3d9.dll")) != NULL ) 
		return TRUE;
	return FALSE;
}

//释放皮肤资源
void CALLBACK SkinDestroyResource()
{
	CSmartUIResource::FreeInstance();
	::CoUninitialize();
}

BOOL  CALLBACK SkinReInitApplicationRun()
{
	CPaintManagerUI::ReInitAppRun();
	return 0;
}

//运行
DWORD CALLBACK SkinApplicationRun()
{
	CPaintManagerUI::MessageLoop();
	return 0;
}

//从XML文件初始化皮肤
DWORD CALLBACK SkinCreateFromFile(const char *szXmlFileName)
{
	BOOL dwRes = 0xFFFFFFFF;
	if (szXmlFileName)
	{
		char szSkinPath[MAX_PATH] = {0};
		TCHAR szFileName[MAX_PATH] = {0};
		CSystemUtils::ExtractFilePath(szXmlFileName, szSkinPath, MAX_PATH - 1);
		CStringConversion::StringToWideChar(szXmlFileName, szFileName, MAX_PATH - 1);
		ifstream ifs(szFileName, std::ios_base::in | std::ios_base::binary);
		if (ifs.is_open())
		{
			ifs.seekg(0, std::ios_base::end);
			DWORD dwSize = (DWORD) ifs.tellg();
			if (dwSize > 0)
			{
				char *pData = new char[dwSize + 1];
				ifs.seekg(0, std::ios::beg);
				ifs.read(pData, dwSize);
				DWORD dwReadSize = (DWORD) ifs.gcount();
				pData[dwReadSize] = '\0';
				dwRes = SkinCreateFromStream(pData, dwReadSize, szSkinPath);
				delete []pData;
			} // end if (dwSize > 0)
			ifs.close();
		} // end if (ifs.is_open());
	} // end if (szXmlFileName)
	return dwRes;
}

//从数据流初始化皮肤
BOOL CALLBACK SkinCreateFromStream(const char *szXmlStream, const DWORD dwStreamSize, const char *szSkinPath)
{
	if ((szXmlStream) && (dwStreamSize > 0))
	{
		if (CSmartUIResource::Instance()->LoadSkinDoc(szXmlStream, dwStreamSize))
		{
			CSmartUIResource::Instance()->SetSkinPath(szSkinPath);
			CSmartUIResource::Instance()->LoadImages();
			return TRUE;
		}
	}
	return FALSE;
}

//载入插件皮肤
BOOL  CALLBACK SkinAddPluginXML(const char *szXmlString)
{
	if (szXmlString)
	{
		return CSmartUIResource::Instance()->AddPluginSkin(szXmlString);
	}
	return FALSE;
}

BOOL  CALLBACK SkinBlendSkinStyle(int r, int g, int b)
{
	BOOL bSucc = CSmartUIResource::Instance()->ModifyColorStyle(r, g, b);
	if (bSucc)
	{
		UpdateAllWindow((BYTE)r, (BYTE)g, (BYTE)b);
	}
	return bSucc;
}

//
BOOL  CALLBACK SkinSetWindowTransparent(HWND hWindow, COLORREF crKey, BYTE Alpha, int FLAG)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->SetWindowTransparent(crKey, Alpha, FLAG);
	return FALSE;
}

//
BOOL  CALLBACK SkinSetDockDesktop(HWND hWindow, BOOL bDock, COLORREF crKey, BYTE Alpha)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->SetDockDesktop(bDock, crKey, Alpha);
	return FALSE;
}

//
DWORD CALLBACK SkinAddLinkImage(const char *szFileName, int nSubCount, COLORREF clrParent)
{
	return CSmartUIResource::Instance()->AddLinkGraphic(szFileName, nSubCount, clrParent);
}

//
void  CALLBACK SkinSetLinkImageCallBack(LPSKIN_GET_IMAGE_ID_BY_LINK pCallBack, LPVOID pOverlapped)
{
	return CSmartUIResource::Instance()->SetLinkCallBack(pCallBack, pOverlapped);
}

BOOL  CALLBACK SkinMixSkinBackGround(const char *szImageFile)
{
	BOOL bSucc = CSmartUIResource::Instance()->MixBackGround(szImageFile);
	if (bSucc)
	{
		UpdateAllWindow(0, 0, 0);		
	}
	return bSucc;
}

BOOL CALLBACK SkinShowHintWindow()
{
	CSmartTipWnd *pWnd = new CSmartTipWnd();
	pWnd->Create(NULL, L"HintWindow",  WS_POPUP, WS_EX_TOOLWINDOW | WS_EX_TOPMOST, 0, 0, 1, 1);
	TOOLINFO m_ToolTip;
	RECT rc = {20, 20, 200, 120};
	const TCHAR sToolTip[] = L"HintWindow Tip";
	::ZeroMemory(&m_ToolTip, sizeof(TOOLINFO));
     m_ToolTip.cbSize = sizeof(TOOLINFO);
     m_ToolTip.uFlags = TTF_IDISHWND;  
     m_ToolTip.hwnd = NULL;
     m_ToolTip.uId = 1223;// reinterpret_cast<UINT>(99898989);
     m_ToolTip.hinst = NULL;
     m_ToolTip.lpszText = const_cast<LPTSTR>((LPCTSTR) sToolTip );
     m_ToolTip.rect = rc;
	 ::SendMessage(pWnd->GetHWND(), TTM_TRACKACTIVATE, TRUE, (LPARAM) &m_ToolTip);
	return TRUE;
}

//创建一个窗体
HWND CALLBACK SkinCreateWindowByName(const char *szWindowName, const TCHAR *szCaption, HWND hParent, DWORD dwStyle, 
								 DWORD dwExStyle, const RECT *prc, BOOL bForceCreate, 
								 LPSKIN_WINDOW_EVENT_CALLBACK pCallBack, LPSKIN_WINDOW_MESSAGE_CALLBACK pMsgCallBack, void *pOverlapped)
{
	CSmartWindow *pWin = NULL;
	CSmartWindow *pParent = NULL;
	if (!bForceCreate)
	{
		pWin = FindWindowByName(szWindowName);
	}
    if (hParent)
		pParent = FindWindowByHWND(hParent);
	if (!pWin)
	{
		pWin = new CSmartWindow(WinDestroyEvent, szWindowName, pParent);
		pWin->SetEventCallBack(pCallBack);
		pWin->SetOverlapped(pOverlapped);
		pWin->SetMsgCallBack(pMsgCallBack);
		pWin->Create(hParent, szCaption, dwStyle, dwExStyle, *prc, NULL);
		m_WinList.insert(std::pair<HWND, CSmartWindow *>(pWin->GetHWND(), pWin));
	}
	if (pWin)
		return pWin->GetHWND();
	return NULL;
}

BOOL CALLBACK SkinOrderWindowMessage(HWND hWindow, UINT uMsg)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->OrderWindowMessage_(uMsg);
	return FALSE;
}

BOOL CALLBACK SkinNotifyEvent(HWND hWindow, const TCHAR *szCtrlName, const TCHAR *szEventName,
	                 WPARAM wParam, LPARAM lParam)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->NotifyEvent(szCtrlName, szEventName, wParam, lParam);
	return FALSE;
}

BOOL  CALLBACK SkinSetControlFocus(HWND hWindow, const TCHAR *szCtrlName, BOOL bFocus)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->SetControlFocus(szCtrlName, bFocus);
	return FALSE;
}

BOOL CALLBACK SkinGetControlRect(HWND hWindow, const TCHAR *szCtrlName, RECT *rc)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->GetControlRect_(szCtrlName, rc);
	return FALSE;
}

//弹出一个模态窗口
DWORD CALLBACK SkinMessageBox(HWND hParent, const TCHAR *szContent, TCHAR *szCaption, DWORD dwStyle)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hParent);
	if (pWin)
		return pWin->StdMsgBox(szContent, szCaption, dwStyle);
	return 0;
}
//弹出模态窗口
DWORD CALLBACK SkinShowModal(HWND hWnd)
{
	CSmartWindow *pWin = FindWindowByHWND(hWnd);
	if (pWin)
		return pWin->ShowModal();
	return 0;
}

void CALLBACK SkinSetModalValue(HWND hWnd, int nValue)
{
	CSmartWindow *pWin = FindWindowByHWND(hWnd);
	if (pWin)
	{
		pWin->SetModalValue(nValue);
		pWin->Close();
	}
}

//加入子控件
BOOL  CALLBACK SkinAddChildControl(HWND hWindow, const TCHAR *szParentCtrlName, const char *szSkinXml,
	                       TCHAR *szFlag, int *nFlagSize, const int nIdx)
{
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->AddControlToUI(szParentCtrlName, szSkinXml, szFlag, nFlagSize, nIdx);
	return FALSE;
}

//移除子控件
BOOL  CALLBACK SkinRemoveChildControl(HWND hWindow, const TCHAR *szParentCtrlName, const TCHAR *szFlag)
{
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->RemoveControlFromUI(szParentCtrlName, szFlag);
	return FALSE;
}

//listbox
int  CALLBACK  SkinInsertListItem(HWND hWindow, const TCHAR *szCtrlName, const TCHAR *szDspName, void *pData, int idx)
{
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->InsertListItem(szCtrlName, szDspName, pData, idx);
	return -1;
}

int  CALLBACK  SkinAppendListItem(HWND hWindow, const TCHAR *szCtrlName, const char *szDspText, void *pData)
{
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->AppendListItem(szCtrlName, szDspText, pData);
	return -1;
}

int  CALLBACK  SkinAppendListSubItem(HWND hWindow, const TCHAR *szCtrlName, const int nIdx, 
	                                  const int nSubIdx, const char *szDspText)
{
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->AppendListSubItem(szCtrlName, nIdx, nSubIdx, szDspText);
	return -1;
}

int  CALLBACK  SkinGetListSelItem(HWND hWindow, const TCHAR *szCtrlName)
{
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->GetListSelItem(szCtrlName);
	return -1;
}

BOOL CALLBACK  SkinDeleteListItem(HWND hWindow, const TCHAR *szCtrlName, int idx)
{
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->DeleteListItem(szCtrlName, idx);
	return FALSE;
}

BOOL CALLBACK  SkinGetListItemInfo(HWND hWindow, const TCHAR *szCtrlName, TCHAR *szDspName, void **pData, int idx)
{
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->GetListItemInfo(szCtrlName, szDspName, pData, idx);
	return FALSE;
}

BOOL CALLBACK  SkinListKeyDownEvent(HWND hWindow, const TCHAR *szCtrlName, WORD wKey)
{
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->ListKeyDownEvent(szCtrlName, wKey);
	return FALSE;
}

int  CALLBACK  SkinGetListCount(HWND hWindow, const TCHAR *szCtrlName)
{
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->GetListCount(szCtrlName);
	return 0;
}

BOOL  CALLBACK  SkinRemoveListItem(HWND hWindow, const TCHAR *szCtrlName, int idx)
{
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->RemoveListItem(szCtrlName, idx);
	return FALSE;
}

BOOL CALLBACK  SkinSetListSelItem(HWND hWindow, const TCHAR *szCtrlName, int idx)
{
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->SetListSelItem(szCtrlName, idx);
	return FALSE;
}

//
BOOL  CALLBACK SkinSetWindowMinSize(HWND hWindow, int cx, int cy)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->SetWindowMinSize_(cx, cy);
	return FALSE;
}

//
BOOL  CALLBACK SkinSetWindowMaxSize(HWND hWindow, int cx, int cy)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->SetWindowMaxSize_(cx, cy);
	return FALSE;
}

//获取一个控制指针
void * CALLBACK SkinGetControlByName(HWND hWindow, const char *szControlName)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	void *pControl = NULL;
    CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		pControl = pWin->FindControl(szControlName);
	return pControl;
}

BOOL CALLBACK SkinSetControlAttr(HWND hWindow, const TCHAR *szControlName, const TCHAR *szAttrName,
	                          const TCHAR *szValue)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->SetControlAttribute(szControlName, szAttrName, szValue);
	}
	return FALSE;
}

BOOL CALLBACK SkinGetControlAttr(HWND hWindow, const TCHAR *szControlName, const TCHAR *szAttrName, 
	                          TCHAR *szValue, int *nMaxValueSize)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->GetControlAttribute(szControlName, szAttrName, szValue, nMaxValueSize);
	}
	return FALSE;
}

//update
BOOL CALLBACK SkinUpdateControlUI(HWND hWindow, const TCHAR *szCtrlName)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->UpdateControl(szCtrlName);
	}
	return FALSE;
}

//设置控件Text
BOOL CALLBACK SkinSetControlTextByName(HWND hWindow, const TCHAR *szControlName, const TCHAR *szText)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif 
    CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->SetControlText(szControlName, szText); 
	}
	return FALSE;
}

//获取控件Text
BOOL  CALLBACK SkinGetControlTextByName(HWND hWindow, const TCHAR *szControlName, TCHAR *szText, int *nSize)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->GetControlText(szControlName, szText, nSize);
	return FALSE;
}

//设置控制的可见性
BOOL CALLBACK SkinSetControlVisible(HWND hWindow, const TCHAR *szCtrlName, BOOL bVisible)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif 
    CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->SetCtrlVisible(szCtrlName, bVisible); 
	}
	return FALSE;
}

BOOL  CALLBACK SkinGetControlVisible(HWND hWindow, const TCHAR *szCtrlName)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->GetControlVisible(szCtrlName);
	return FALSE;
}

//设置控制的Enable
BOOL  CALLBACK SkinSetControlEnable(HWND hWindow, const TCHAR *szCtrlName, BOOL bEnable)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif 
    CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->SetCtrlEnable(szCtrlName, bEnable); 
	}
	return FALSE;
}

BOOL  CALLBACK SkinGetControlEnable(HWND hWindow, const TCHAR *szCtrlName)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
    CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->GetControlEnable(szCtrlName);
	}
	return FALSE;
}

//==== RichEdit 相关应用 ===========
//RichEdit 相关应用函数
BOOL CALLBACK SkinAddRichChatText(HWND hWindow, const TCHAR *szControlName, const char *szId, const DWORD dwUserId, const char *szUserName, 
							   const char *szTime, const char *szText, const CCharFontStyle *cfStyle,
							   const int nNickColor, BOOL bIsUTF8, BOOL bAck)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif 
    CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->AddChatText(szControlName, szId, dwUserId, szUserName, szTime, szText, 
			          cfStyle, nNickColor, bIsUTF8, bAck); 
	}
	return FALSE;
}

BOOL  CALLBACK SkinGetRESelectImageFile(HWND hWindow, const TCHAR *szCtrlName, char *szFileName, int *nSize)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif 
    CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->GetRESelectImageFile(szCtrlName, szFileName, nSize);
	return FALSE;
}

int   CALLBACK SkinGetRESelectStyle(HWND hWindow, const TCHAR *szCtrlName)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif 
    CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->GetRESelectStyle(szCtrlName);
	return 0;
}
BOOL  CALLBACK SkinGetREChatId(HWND hWindow, const TCHAR *szCtrlName, char *szId)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif 
    CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->GetREChatId(szCtrlName, szId);
	return FALSE;
}

BOOL  CALLBACK SkinREClearChatMsg(HWND hWindow, const TCHAR *szCtrlName, const char *szId)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif 
    CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->REClearChatMsg(szCtrlName, szId);
	return FALSE;
}


BOOL  CALLBACK SkinGetRichEditOleFlag(HWND hWindow, const TCHAR *szCtrlName, char **pOleFlags)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->GetRichEditOleFlag(szCtrlName, pOleFlags);
	}
	return FALSE;
}

//optionui 相关
BOOL  CALLBACK SkinSetOptionData(HWND hWindow, const TCHAR *szCtrlName, const int nData)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->SetOptionData_(szCtrlName, nData);
	return FALSE;
}

//radio
//radio控件相关
BOOL CALLBACK SkinSetRadioChecked(HWND hWindow, const TCHAR *szCtrlName, BOOL bChecked)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif 
    CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->SetRadioCheck(szCtrlName, bChecked); 
	}
	return FALSE;
}

BOOL  CALLBACK SkinGetRadioChecked(HWND hWindow, const TCHAR *szCtrlName)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->GetRadioCheck(szCtrlName);
	return FALSE;
}

//checkbox 控件相关
BOOL CALLBACK SkinSetCheckBoxStatus(HWND hWindow, const TCHAR *szCtrlName, const int nStatus)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->SetCheckBoxStatus_(szCtrlName, nStatus);
	return FALSE;
}

int CALLBACK SkinGetCheckBoxStatus(HWND hWindow, const TCHAR *szCtrlName)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->GetCheckBoxStatus_(szCtrlName);
	return 0;
}

//Edit相关
//Edit 相关
BOOL CALLBACK SkinSetEditReadOnly(HWND hWindow, const TCHAR *szCtrlName, BOOL bReadOnly)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->SetEditReadOnlyValue(szCtrlName, bReadOnly); 
	}
	return FALSE;
}

BOOL CALLBACK SkinSetMultiEditReadOnly(HWND hWindow, const TCHAR *szCtrlName, BOOL bReadOnly)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->SetMultiEditReadOnlyValue(szCtrlName, bReadOnly); 
	}
	return FALSE;
}

BOOL CALLBACK SkinSetRichEditReadOnly(HWND hWindow, const TCHAR *szCtrlName, BOOL bReadOnly)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->SetRichEditReadOnlyValue(szCtrlName, bReadOnly); 
	}
	return FALSE;
}

BOOL CALLBACK SkinGetRichEditText(HWND hWindow, const TCHAR *szCtrlName, DWORD dwStyle, char **pBuf, int *nSize)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->GetREText(szCtrlName, dwStyle, pBuf, *nSize);
	}
	return FALSE;
}

BOOL  CALLBACK SkinSetRichEditCallBack(HWND hWindow, const TCHAR *szCtrlName, 
	                               LPSKIN_RICHEDIT_EVENT_CALLBACK pCallBack, LPVOID lpOverlapped)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->SetRichEditCallBack_(szCtrlName, pCallBack, lpOverlapped);
	}
	return FALSE;
}

BOOL  CALLBACK SkinRichEditInsertTip(HWND hWindow, const TCHAR *szCtrlName, CCharFontStyle *cfStyle,
	                             DWORD dwOffset, const TCHAR *szText)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->RichEditInsertTip_(szCtrlName, cfStyle, dwOffset, szText);
	}
	return FALSE;
}

BOOL  CALLBACK SkinRichEditAddFileLink(HWND hWindow, const TCHAR *szCtrlName, CCharFontStyle *cfStyle,
	                             DWORD dwOffset, const char *szTip, const char *szFileName)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->RichEditAddFileLink(szCtrlName, cfStyle, dwOffset, szTip, szFileName);
	return FALSE;
}

BOOL  CALLBACK SkinRichEditCommand(HWND hWindow, const TCHAR *szCtrlName, const char *szCommand, LPVOID lpParams)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->RichEditCommand_(szCtrlName, szCommand, lpParams);
	}
	return FALSE;
}

BOOL  CALLBACK SkinInsertImageToRichEdit(HWND hWindow, const TCHAR *szCtrlName, const char *szFileName, 
	                 const char *szTag, const int nPos)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->InsertImageToRichEdit_(szCtrlName, szFileName, szTag, nPos);
	}
	return FALSE;
}

BOOL  CALLBACK SkinCancelCustomLink(HWND hWindow, const TCHAR *szCtrlName, DWORD dwLinkFlag)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->CancelCustomLink(szCtrlName, dwLinkFlag);
	return FALSE;
}

BOOL  CALLBACK SkinReplaceImageInRichEdit(HWND hWindow, const TCHAR *szCtrlName, 
	                const char *szFileName, const char *szTag)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->ReplaceImageInRichEdit_(szCtrlName, szFileName, szTag);
	}
	return FALSE;
}

BOOL  CALLBACK SkinGetCurrentRichEditFont(HWND hWindow, const TCHAR *szCtrlName, CCharFontStyle *cfStyle)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->GetCurrentRichEditFont_(szCtrlName, cfStyle);
	}
	return FALSE;
}

char * CALLBACK SkinGetRichEditOleText(HWND hWindow, const TCHAR *szCtrlName, DWORD dwStyle)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->GetOleText_(szCtrlName, dwStyle);
	}
	return FALSE;
}

BOOL  CALLBACK  SkinSetRichEditText(HWND hWindow, const TCHAR *szCtrlName, const char *szText, DWORD dwStyle)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->SetREText(szCtrlName,szText, dwStyle);
	return FALSE;
}

BOOL  CALLBACK SkinSetRichEditAutoDetectLink(HWND hWindow, const TCHAR *szCtrlName, BOOL bAutoDetect)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->SetEditAutoDetectLink(szCtrlName, bAutoDetect);
	}
	return FALSE;
}

BOOL  CALLBACK SkinREInsertOlePicture(HWND hWindow, const TCHAR *szCtrlName, const char *szPicFileName)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->REInsertOlePicture(szCtrlName, szPicFileName);
	}
	return FALSE;
}

BOOL  CALLBACK SkinGetEditReadOnly(HWND hWindow, const TCHAR *szCtrlName)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->GetEditReadOnlyValue(szCtrlName);
	return FALSE;
}

BOOL CALLBACK SkinGetMultiEditReadOnly(HWND hWindow, const TCHAR *szCtrlName)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->GetMultiEditReadOnlyValue(szCtrlName);
	return FALSE;
}

BOOL  CALLBACK SkinGetRichEditReadOnly(HWND hWindow, const TCHAR *szCtrlName)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->GetRichEditReadOnlyValue(szCtrlName);
	return FALSE;
}

//GifImage 相关
BOOL  CALLBACK SkinLoadGifImage(HWND hWindow, const TCHAR *szCtrlName, const TCHAR *szImageFileName, BOOL bTransParent, 
						   DWORD dwTransClr, BOOL bAnimate)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->SetGifImage(szCtrlName, szImageFileName, bTransParent, dwTransClr, bAnimate);
	return FALSE;
}

//TreeView相关
//设置释放节点内存的回调函数
BOOL CALLBACK SkinSetFreeNodeDataCallBack(HWND hWindow, const TCHAR *szCtrlName, LPSKIN_FREE_NODE_EXDATA pCallback)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->SetFreeNodeDataCallBack_(szCtrlName, pCallback);
	return FALSE;
}

//清除所有树节点
BOOL CALLBACK  SkinTreeViewClear(HWND hWindow, const TCHAR *szCtrlName)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->TreeViewClear(szCtrlName);
	return FALSE;
}

//
int  CALLBACK  SkinGetTreeNodeStatus(void *pNode)
{
	CTreeNodeItem *pTreeNode = reinterpret_cast<CTreeNodeItem *>(pNode);
	if (pTreeNode)
		return pTreeNode->GetCheckStatus();
	return 0;
}

//全选
BOOL CALLBACK  SkinTreeSelectAll(HWND hWindow, const TCHAR *szCtrlName, void *pParentNode, BOOL bRecursive)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->TreeViewSelectAll(szCtrlName, pParentNode, bRecursive);
	return FALSE;
}

//反选
BOOL CALLBACK  SkinTreeUnSelected(HWND hWindow, const TCHAR *szCtrlName, void *pParentNode, BOOL bRecursive)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->TreeViewUnSelected(szCtrlName, pParentNode, bRecursive);
	return FALSE;
}

//删除选择
BOOL CALLBACK  SkinTreeDelSelected(HWND hWindow, const TCHAR *szCtrlName, BOOL bRecursive)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->TreeViewDelSelected(szCtrlName, bRecursive);
	return FALSE;
}

//
BOOL CALLBACK  SkinTreeGetSelectedUsers(HWND hWindow, const TCHAR *szCtrlName, char *szUsers, int *nSize)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->TreeViewGetSelectedUsers(szCtrlName, szUsers, nSize, TRUE);
	return FALSE;
}

//
LPVOID CALLBACK SkinAdjustTreeNode(HWND hWindow, const TCHAR *szCtrlName, void *pParentNode, const TCHAR *szName, 
	                     CTreeNodeType tnType, void *pData, BOOL bAdd, BOOL bRecursive)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->AdjustTreeNode(szCtrlName, pParentNode, szName, tnType, pData, bAdd, bRecursive);
	return NULL;
}

//
BOOL CALLBACK SkinGetTreeNodeById(HWND hWindow, const TCHAR *szCtrlName, const DWORD dwId, 
	CTreeNodeType tnType, void **pNode, void **pData)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->GetTreeNodeById(szCtrlName, dwId, tnType, pNode, pData);
	return FALSE;
}

//展开树节点
BOOL CALLBACK SkinExpandTree(HWND hWindow, const TCHAR *szCtrlName, void *pParentNode, BOOL bExpanded, BOOL bRecursive)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->ExpandTreeView(szCtrlName, pParentNode, bExpanded, bRecursive);
	return FALSE;
}

//设置Icon的类型
BOOL CALLBACK  SkinSetTreeIconType(HWND hWindow, const TCHAR *szCtrlName, BYTE byteIconType)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->SetTreeViewIconType(szCtrlName, byteIconType);
	return FALSE;
}

//设置组节点是否可选
BOOL CALLBACK SkinSetTreeGroupNodeIsSelect(HWND hWindow, const TCHAR *szCtrlName, BOOL bIsSelected)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->SetTreeViewGroupNodeIsSelect(szCtrlName, bIsSelected);
	return FALSE;
}

//获取在线用户统计数据
BOOL CALLBACK  SkinTVGetOnlineCount(HWND hWindow, const TCHAR *szCtrlName, void *pParentNode, DWORD *dwTotalCount, DWORD *dwOnlineCount)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->TVGetOnlineCount(szCtrlName, pParentNode, dwTotalCount, dwOnlineCount);
	return FALSE;
}

//删除节点
BOOL CALLBACK  SkinTVDelNodeByID(HWND hWindow, const TCHAR *szCtrlName, int nId, CTreeNodeType tnType)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->TVDelNodeByID(szCtrlName, nId, tnType);
	return FALSE;
}

//装载默认图标
BOOL CALLBACK SkinLoadTreeDefaultImage(HWND hWindow, const TCHAR *szCtrlName, const char *szImageFileName)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->LoadTreeViewDefaultImage(szCtrlName, szImageFileName);
	return FALSE;
}

//加入一个树节点
LPVOID CALLBACK SkinAddTreeChildNode(HWND hWindow, const TCHAR *szCtrlName, const DWORD dwId, void *pParentNode, const TCHAR *szText,  CTreeNodeType tnType,
							   void *pData, const TCHAR *szLabel, const TCHAR *szImageFileName, const TCHAR *szExtraData)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->AddTreeChildNode_(szCtrlName, dwId, pParentNode, szText, tnType, pData, 
		                               szLabel, szImageFileName, szExtraData);
	return NULL;
}

//
BOOL CALLBACK  SkinTreeScrollToNodeByKey(HWND hWindow, const TCHAR *szCtrlName, const char *szKey)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->TreeScrollToNodeByKey(szCtrlName, szKey);
	return FALSE;
}

//获取节点展开状态
BOOL CALLBACK SkinGetNodeIsExpanded(HWND hWindow, void *pNode, BOOL *bExpanded)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->GetNodeIsExpanded_(pNode, bExpanded);
	return FALSE;
}

//
BOOL CALLBACK SkinGetNodeByKey(HWND hWindow, const TCHAR *szCtrlName, void *pParentNode, const char *szKey, TCHAR *szName, int *nNameLen,
	                           void **pSelNode, CTreeNodeType *tnType, void **pData)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->GetNodeByKey(szCtrlName, pParentNode, szKey, szName, nNameLen, pSelNode, tnType, pData);
	return FALSE;
}

//获取节点下用户列表
BOOL CALLBACK SkinGetNodeChildUserList(HWND hWindow, void *pNode, char *szUserList, int *nSize, BOOL bRecursive)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->GetNodeChildUserList_(pNode, szUserList, nSize, bRecursive);
	return FALSE;
}

//
BOOL CALLBACK SkinSetGetKeyFun(HWND hWindow, const TCHAR *szCtrlName, LPSKIN_GET_TREE_NODE_KEY pCallBack)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->SetGetKeyFun(szCtrlName, pCallBack);
	return FALSE;
}

//update status
LPVOID CALLBACK SkinUpdateUserStatusToNode(HWND hWindow, const TCHAR *szCtrlName, const char *szUserName, 
	                                 const char *szStatus, BOOL bMulti)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->UpdateUserStatusToNode_(szCtrlName, szUserName, szStatus, bMulti);
	return NULL;
}

//
BOOL CALLBACK SkinSetTreeViewStatusOffline(HWND hWindow, const TCHAR *szCtrlName)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->SetTreeViewStatusOffline(szCtrlName);
	return NULL;
}

BOOL CALLBACK  SkinSortTreeNode(HWND hWindow, const TCHAR *szCtrlName,  
	                    void *pNode, LPSKIN_COMPARENODE pCompare, BOOL bRecursive, BOOL bParent)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->SortTreeNode(szCtrlName, pNode, pCompare, bRecursive, bParent);
	}
	return FALSE;
}

//
BOOL CALLBACK  SkinShowTreeExtraData(HWND hWindow, const TCHAR *szCtrlName, BOOL bShow)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->ShowTreeExtraData(szCtrlName, bShow);
	}
	return FALSE;
}

//
LPVOID CALLBACK SkinUpdateTreeNodeExtraData(HWND hWindow, const TCHAR *szCtrlName, const char *szKey, 
	                    const TCHAR *szExtraData, BOOL bMulti)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->UpdateTreeNodeExtraData(szCtrlName, szKey, szExtraData, bMulti);
	}
	return FALSE;
}

//
LPVOID CALLBACK SkinUpdateTreeNodeImageFile(HWND hWindow, const TCHAR *szCtrlName, const char *szKey,
	                     const char *szImageFile, BOOL bMulti)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		return pWin->UpdateTreeNodeImageFile(szCtrlName, szKey, szImageFile, bMulti);
	}
	return FALSE;
}

//
LPVOID CALLBACK SkinUpdateTreeNodeExtraImageFile(HWND hWindow, const TCHAR *szCtrlName, const char *szKey, 
	                           const int nImageId, BOOL bMulti)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
	{
		pWin->UpdateTreeNodeExtraImageFile(szCtrlName, szKey, nImageId, bMulti);
	}
	return FALSE;
}

//update label
LPVOID CALLBACK SkinUpdateUserLabelToNode(HWND hWindow, const TCHAR *szCtrlName, const char *szUserName,
	                                 const char *szUTF8Label,  BOOL bMulti)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->UpdateUserLabelToNode_(szCtrlName, szUserName, szUTF8Label, bMulti);
	return NULL;
}

//获取当前选择的树节点
BOOL CALLBACK SkinGetSelectTreeNode(HWND hWindow, const TCHAR *szCtrlName, TCHAR *szName, int *nNameLen, 
								void **pSelNode, CTreeNodeType *tnType, void **pData)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->GetSelectTreeNode_(szCtrlName, szName, nNameLen, pSelNode, tnType, pData);
	return FALSE;
}

//导航相关
BOOL CALLBACK  SkinNavigate2URL(HWND hWindow, const TCHAR *szCtrlName, const char *szUrl)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->LoadTreeViewDefaultImage(szCtrlName, szUrl);
	return FALSE;
}

//Tab Control 相关
BOOL CALLBACK  SkinTabNavigate(HWND hWindow, const TCHAR *szCtrlName, const int nIdx)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->TabNavigate2(szCtrlName, nIdx);
	return FALSE;
}

BOOL CALLBACK  SkinTabSelectItem(HWND hWindow, const TCHAR *szCtrlName, const TCHAR *szPageName)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->TabSelectItem(szCtrlName, szPageName);
	return FALSE;
}

BOOL CALLBACK  SkinTabGetSelItemName(HWND hWindow, const TCHAR *szCtrlName, TCHAR *szSelTabName, int *nSize)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->TabGetSelItemName(szCtrlName, szSelTabName, nSize);
	return FALSE;
}

BOOL CALLBACK  SkinTabGetChildByClass(HWND hWindow, const TCHAR *szCtrlName, const TCHAR *szClassName,
	TCHAR *szTabName, int *nSize)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->TabGetChildItemByClass(szCtrlName, szClassName, szTabName, nSize);
	return FALSE;
}

//DropDown 相关
//获取某项dropdown 数据
void *CALLBACK  SkinGetDropdownItemData(HWND hWindow, const TCHAR *szCtrlName, const int nIdx)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->GetDropdownItemData_(szCtrlName, nIdx);
	return NULL;
}

//设置某项dropdown 数据
BOOL CALLBACK  SkinSetDropdownItemData(HWND hWindow, const TCHAR *szCtrlName, const int nIdx, void *pData)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->SetDropdownItemData_(szCtrlName, nIdx, pData);
	return FALSE;
}

//获取dropdown 项 string
BOOL CALLBACK  SkinGetDropdownItemString(HWND hWindow, const TCHAR *szCtrlName, const int nIdx, TCHAR *szText, int *nSize)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->GetDropdownItemString_(szCtrlName, nIdx, szText, nSize);
	return FALSE;
}

//设置dropdown 项 string
int CALLBACK  SkinSetDropdownItemString(HWND hWindow, const TCHAR *szCtrlName, const int nIdx, const TCHAR *szText, void *pData)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->SetDropdownItemString_(szCtrlName, nIdx, szText, pData);
	return -1;
}

//获取当前选择项
int  CALLBACK  SkinGetDropdownSelectIndex(HWND hWindow, const TCHAR *szCtrlName)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->GetDropdownSelIndex(szCtrlName);
	return -1;
}

//删除Dropdown 某项
BOOL CALLBACK  SkinDeleteDropdownItem(HWND hWindow, const TCHAR *szCtrlName, const int nIdx)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->DeleteDropdownItem_(szCtrlName, nIdx);
	return FALSE;
}

//选择dropdown某项
BOOL CALLBACK  SkinSelectDropdownItem(HWND hWindow, const TCHAR *szCtrlName, const int nIdx)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->SelectDropdownItem_(szCtrlName, nIdx);
	return FALSE;
}

//获取dropdown item 个数
int  CALLBACK  SkinGetDropdownItemCount(HWND hWindow, const TCHAR *szCtrlName)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->GetDropdownItemCount_(szCtrlName);
	return FALSE;
}

//
//set key word
BOOL CALLBACK SkinSetScintKeyWord(HWND hWindow, const TCHAR *szCtrlName, const TCHAR *szKeyWord)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->SetScintKeyWord_(szCtrlName, szKeyWord);
	return FALSE;
}

//set item style
BOOL CALLBACK SkinSetScintStyle(HWND hWindow, const TCHAR *szCtrlName, int nStyle, COLORREF clrFore, 
	       COLORREF clrBack, int nSize, TCHAR *szFace)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->SetScintStyle_(szCtrlName, nStyle, clrFore, clrBack, nSize, szFace);
	return FALSE;
}

//memnu 相关
BOOL CALLBACK SkinCreateMenu(HWND hWindow, const TCHAR *szMenuName)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->CreateSkinMenu_(szMenuName);
	return FALSE;
}


//
BOOL CALLBACK SkinPopTrackMenu(HWND hWindow, const TCHAR *szMenuName, UINT uFlags, const int X, const int Y)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->PopTrackSkinMenu_(szMenuName, uFlags, X, Y);
	return FALSE;
}

//
BOOL CALLBACK SkinDestroyMenu(HWND hWindow, const TCHAR *szMenuName)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->DestroySkinMenu_(szMenuName);
	return FALSE;
}

//
BOOL CALLBACK SkinMenuAppendItem(HWND hWindow, const TCHAR *szMenuName, int nParentId, const TCHAR *szMenuCaption, int nMenuId)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->MenuAppendItem(szMenuName, nParentId, szMenuCaption, nMenuId);
	return FALSE;
}

//
BOOL CALLBACK SkinMenuGetItemCaption(HWND hWindow, const TCHAR *szMenuName, int nMenuId, TCHAR *szwCaption, int *nSize)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->MenuGetItemCaption(szMenuName, nMenuId, szwCaption, nSize);
	return FALSE;
}

//
BOOL CALLBACK SkinGrayMenu(HWND hWindow, const TCHAR *szMenuName, UINT uMenuID, BOOL bGray)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->GraySkinMenu_(szMenuName, uMenuID, bGray);
	return FALSE;
}

BOOL CALLBACK  SkinSetMenuChecked(HWND hWindow, const TCHAR *szMenuName, UINT uMenuID, BOOL bChecked)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->SetMenuChecked(szMenuName, uMenuID, bChecked);
	return FALSE;
}

BOOL CALLBACK SkinSetMenuItemAttr(HWND hWindow, const TCHAR *szMenuName, UINT uMenuId, const TCHAR *szAttrName, const TCHAR *szValue)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->SetMenuItemAttr(szMenuName, uMenuId, szAttrName, szValue);
	return FALSE;
}

//
BOOL CALLBACK  SkinAddEmotion(HWND hWindow, const TCHAR *szCtrlName, const char *szFileName,
	                         const char *szEmotionTag, const char *szEmotionShortCut, const char *szEmotionComment)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->AddEmotion(szCtrlName, szFileName, szEmotionTag, szEmotionShortCut, szEmotionComment);
	return FALSE;
}

int  CALLBACK  SkinGetDisplayEmotionCount(HWND hWindow, const TCHAR *szCtrlName)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->GetDisplayEmotionCount(szCtrlName); 
	return FALSE;
}

BOOL CALLBACK  SkinClearAllEmotion(HWND hWindow, const TCHAR *szCtrlName)
{
#ifdef FIND_WINDOW_LOCK
	CGuardLock::COwnerLock guard(m_WinLock);
#endif
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->ClearAllEmotion(szCtrlName); 
	return FALSE;
}

BOOL CALLBACK SkinGetSelEmotion(HWND hWindow, const TCHAR *szCtrlName, char *szGifName, int *nNamSize,
	                char *szTag, int *nTagSize)
{ 
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->GetSelEmotion(szCtrlName, szGifName, nNamSize, szTag, nTagSize);
	return FALSE;
}

//加入一个快捷方式
BOOL CALLBACK SkinAddAutoShortCut(HWND hWindow, const TCHAR *szCtrlName, const TCHAR *szCaption, 
	const TCHAR *szFileName, const int nImageId, const TCHAR *szTip, TCHAR *szFlag)
{
	CSmartWindow *pWin = FindWindowByHWND(hWindow);
	if (pWin)
		return pWin->AddAutoShortCut(szCtrlName, szCaption, szFileName, nImageId, szTip, szFlag);
	return FALSE;
}

//debug
//debug function
BOOL CALLBACK SkinFillRectToFile(COLORREF clr, const char *szFileName)
{
	int cx = 100;
	int cy = 100;
	HDC hdc = ::GetDC(::GetDesktopWindow());
	HDC m_hMem = ::CreateCompatibleDC(hdc);
	HBITMAP hBitmap = ::CreateCompatibleBitmap(hdc, cx, cy);
    HBITMAP m_hOldBitmap = (HBITMAP)::SelectObject(m_hMem, hBitmap);
    RECT rc = {0, 0, cx, cy};
	HBRUSH hbr = ::CreateSolidBrush(clr);
	::FillRect(m_hMem, &rc, hbr);
	CGraphicPlus graphic;
	graphic.LoadFromBitmap(hBitmap);
	graphic.SaveToFile(szFileName, GRAPHIC_TYPE_BMP);
	::DeleteObject(hbr);
    ::SelectObject(m_hMem, m_hOldBitmap);
	::DeleteObject(hBitmap);
	::DeleteObject(m_hMem);
	::ReleaseDC(::GetDesktopWindow(), hdc);
	return TRUE;
}

//
BOOL CALLBACK SkinTransImage(const char *szSrcImageName, const char *szDstImageName,
	                         const int nWidth, const int nHeight)
{
	CGraphicPlus src, dst;
	if (src.LoadFromFile(szSrcImageName, FALSE))
	{
		SIZE szNew = {nWidth, nHeight};
		return src.SaveToFile(szDstImageName, GRAPHIC_TYPE_JPG, szNew);
	}
	return FALSE;
}

#pragma warning(default:4996)
