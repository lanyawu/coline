#include <time.h>
#include <commonlib/DebugLog.h>
#include <Commonlib/systemutils.h>
#include <FileTransfer/filetransfer.h>
#include <rmc/remotemachinecontrol.h>
//#include <Media/VideoSDL.h>
#include <Core/common.h>
#include "../P2Svr/P2Svr.h"
#include "ChatFrameImpl.h"
#include "../econtacts/econtactsimpl.h"
#include "../IMCommonLib/InterfaceAnsiString.h"
#include "../IMCommonLib/InterfaceFontStyle.h"
#include "../IMCommonLib/InstantUserInfo.h"
#include "../IMCommonLib/InterfaceUserList.h"
#include "../IMCommonLib/XmlNodeTranslate.h"
#include <ShellAPI.h>
#pragma warning(disable:4996)

//远程控制SHIFT状态定义
#define  KEY_SHIFT_LSHIFT        0x0001  //左shift 键
#define  KEY_SHIFT_RSHIFT        0x0002  //右shift 键
#define  KEY_SHIFT_LCTRL         0x0004  //左ctrl  键
#define  KEY_SHIFT_RCTRL         0x0008  //右ctrl  键
#define  KEY_SHIFT_LALT          0x0010  //左ALT   键
#define  KEY_SHIFT_RALT          0x0020  //右ALT   键
#define  KEY_SHIFT_CAPSOPEN      0x0040  //键盘大小写
#define  KEY_SHIFT_CAPSCLOSE     0x0080  //大小写关闭
#define  KEY_SHIFT_KEYDOWN       0x0100  //键按下
#define  KEY_SHIFT_KEYUP         0x0200  //键扦起
#define  KEY_SHIFT_KEYVALUE      0x0400  //键值

//鼠标事件
#define  KEY_SHIFT_MOUSE_NONE     0  //无鼠标事件
#define  KEY_SHIFT_LBUTTONDOWN    1  //左鼠标按下
#define  KEY_SHIFT_LBUTTONUP      2  //左鼠标扦起
#define  KEY_SHIFT_RBUTTONDOWN    3  //右鼠标按下
#define  KEY_SHIFT_RBUTTONUP      4  //右鼠标扦起
#define  KEY_SHIFT_MOUSEMOVE      5  //鼠标移动
#define  KEY_SHIFT_LBUTTONDBLK    6  //左键双击
#define  KEY_SHIFT_RBUTTONDBLK    7  //右键双击
#define  KEY_SHIFT_MBUTTONDOWN    8  //中键按下
#define  KEY_SHIFT_MBUTTONUP      9  //中键扦起
#define  KEY_SHIFT_MOUSEWHEEL     10 //中键滚动

 
int CALLBACK CutImage(HWND hParent, BOOL bHideParent);

typedef struct CFileLinkData
{
	std::string strTip;
	std::string strFileName;
}FILE_LINK_DATA, *LPFILE_LINK_DATA;


BOOL CALLBACK RichEditCallBack(HWND hWnd, DWORD dwEvent, char *szFileName, DWORD *dwFileNameSize,
			  char *szFileFlag, DWORD *dwFileFlagSize, DWORD dwFlag, void *pOverlapped)
{
	if (pOverlapped)
	{
		CChatFrameImpl *pThis = (CChatFrameImpl *) pOverlapped;
		return pThis->RECallBack(hWnd, dwEvent, szFileName, dwFileNameSize, szFileFlag, dwFileFlagSize, dwFlag);
	}
	return FALSE;
}


CChatFrameImpl::CChatFrameImpl(void):
                m_pCore(NULL),
				m_hMainFrame(NULL),
				m_rmcChlId(0),
				m_bEnterSend(TRUE),
				m_VideoChlId(0)  
{
	memset(&m_rcLastOpen, 0, sizeof(RECT));
	memset(&m_rmcInfo, 0, sizeof(m_rmcInfo));
	memset(&m_VideoInfo, 0, sizeof(m_VideoInfo));
}


CChatFrameImpl::~CChatFrameImpl(void)
{
	ClearChatFrame();
	if (m_pCore)
		m_pCore->Release();
	m_pCore = NULL;
	m_hMainFrame = NULL;
}

void CChatFrameImpl::ClearChatFrame()
{
	std::map<HWND, CUserChatFrame *>::iterator it;
	for (it = m_ChatFrameList.begin(); it != m_ChatFrameList.end(); it ++)
	{
		::SkinCloseWindow(it->first);
	}
	m_ChatFrameList.clear();
}

//IUnknown
STDMETHODIMP CChatFrameImpl::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IChatFrame)))
	{
		*ppv = (IChatFrame *) this;
		_AddRef();
		return S_OK;
	} else if (IsEqualIID(riid, __uuidof(ICoreEvent)))
	{
		*ppv = (ICoreEvent *) this;
		_AddRef();
		return S_OK;
	} else if (IsEqualIID(riid, __uuidof(IProtocolParser)))
	{
		*ppv = (IProtocolParser *) this;
		_AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

void CChatFrameImpl::DoUserSignChanged(const char *szUserName)
{
	HWND hOwner = GetHWNDByUserName(szUserName);
	if (hOwner)
	{
		IContacts *pContact = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
		{
			CInterfaceAnsiString strValue;
			if (SUCCEEDED(pContact->GetContactUserValue(szUserName, "sign", &strValue)))
			{
				TCHAR *szValue = new TCHAR[strValue.GetSize() + 1];
				memset(szValue, 0, sizeof(TCHAR) * (strValue.GetSize() + 1));
				CStringConversion::UTF8ToWideChar(strValue.GetData(), szValue, strValue.GetSize());
				::SkinSetControlTextByName(hOwner, L"lblSign", szValue);
				::SkinSetControlAttr(hOwner, L"lblSign", L"tooltip", szValue);
				delete []szValue;
			}
			pContact->Release();
		} //end if (SUCCEEDED(
	} //end if (hOwner)
}

STDMETHODIMP CChatFrameImpl::DoPresenceChange(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder)
{
	if ((!m_strUserName.empty()) && ::stricmp(m_strUserName.c_str(), szUserName) == 0)
	{
		//
		if (::stricmp(szNewPresence, "offline") == 0) //下线
		{
			std::map<HWND, CUserChatFrame *>::iterator it;
			for (it = m_ChatFrameList.begin(); it != m_ChatFrameList.end(); it ++)
			{
				TerminatedTransFileByHWND(it->first, TRUE); 
				::SkinSetControlTextByName(it->first, L"lblstatus", L"(离线)");
				::SkinUpdateControlUI(it->first, L"lblstatus");
			    ::SkinSetControlAttr(it->first, L"peerlogo", L"gray", L"true");
			} 
		}
	} else
	{
		HWND hOwner = GetHWNDByUserName(szUserName);
		if (hOwner != NULL)  //状态改变
		{
			//
			const char *szRealName = GetRealNameByHWND(hOwner);
			TCHAR szwText[MAX_PATH] = {0};
			CStringConversion::UTF8ToWideChar(szRealName, szwText, MAX_PATH - 1);
			BOOL bGray = FALSE; 
			CStdString_ strStatusTip;
			if (stricmp(szNewPresence, "online") == 0)
			{
				::lstrcat(szwText, L"(在线)");
			} else if (stricmp(szNewPresence, "away") == 0)
			{
				::lstrcat(szwText, L"(离开)");
				strStatusTip = L"对方处于<离开>状态，可能无法及时回复您的消息";
			} else if (stricmp(szNewPresence, "busy") == 0)
			{
				::lstrcat(szwText, L"(忙碌)");
				strStatusTip = L"对方处于<忙碌>状态，可能无法及时回复您的消息";
			} else if ((stricmp(szNewPresence, "iphone") == 0)
				|| (stricmp(szNewPresence, "ipad") == 0)
				|| (stricmp(szNewPresence, "ipod") == 0)
				|| (stricmp(szNewPresence, "itouch") == 0)
				|| (stricmp(szNewPresence, "android") == 0))
			{
				::lstrcat(szwText, L"(移动设备)");
				strStatusTip = L"对方使用移动设备在线，无法接收文件";
			} else 
			{
				::lstrcat(szwText, L"(离线)");
				strStatusTip = L"对方隐身或离线，您可以给对方留言或者发送离线文件";
				bGray = TRUE;
			}
			if (szMemo)
			{
				TCHAR szwTmp[128] = {0};
				CStringConversion::StringToWideChar(szMemo, szwTmp, 127);
				::lstrcat(szwText, L"(");
				::lstrcat(szwText, szwTmp);
				::lstrcat(szwText, L")");
			} 
			::SkinSetControlTextByName(hOwner, L"lblstatus", szwText);
			::SkinUpdateControlUI(hOwner, L"lblstatus");

			if (bGray) 
			{
			    ::SkinSetControlAttr(hOwner, L"peerlogo", L"gray", L"true");
			} else 
			{
			    ::SkinSetControlAttr(hOwner, L"peerlogo", L"gray", L"false"); 
			}

			if (!strStatusTip.IsEmpty())
			{ 
				::SkinSetControlVisible(hOwner, L"offlinetip", TRUE);
				::SkinSetControlTextByName(hOwner, L"statustip", strStatusTip.GetData());
				::SkinSetControlAttr(hOwner, L"statustip", L"tooltip", strStatusTip.GetData());
			} else
			{
				::SkinSetControlVisible(hOwner, L"offlinetip", FALSE);
			}
			return S_OK;
		}
	}
	return E_FAIL;
}

//广播消息
STDMETHODIMP CChatFrameImpl::DoBroadcastMessage(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData)
{
	if ((::stricmp(szFromWndName, "ConfigureUI") == 0) && (::stricmp(szType, "uploadheader") == 0)
		&& (::stricmp(pContent, "succ") == 0)) //上传头像成功
	{
		IContacts *pContact = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
		{
			CInterfaceAnsiString strFileName;
			if (SUCCEEDED(pContact->GetContactHead(m_strUserName.c_str(), &strFileName, FALSE)))
			{
				TCHAR szTmp[MAX_PATH] = {0};
				CStringConversion::StringToWideChar(strFileName.GetData(), szTmp, MAX_PATH - 1); 
				std::map<HWND, CUserChatFrame *>::iterator it;
				for (it = m_ChatFrameList.begin(); it != m_ChatFrameList.end(); it ++)
				{
					::SkinSetControlAttr(it->first, L"SelfHeader", L"floatimagefilename", szTmp);
				}
			}
			pContact->Release();
		}
	} else if ((::stricmp(szFromWndName, "mainwindow") == 0) && (::stricmp(szType, "dlheader") == 0))
	{
		
		std::map<HWND, CUserChatFrame *>::iterator it;
		for (it = m_ChatFrameList.begin(); it != m_ChatFrameList.end(); it ++)
		{
			if (stricmp(it->second->GetUserName(), pContent) == 0)
			{
				//用户头你下载完成
				char *pFileName = (char *)pData;
				TCHAR szTmp[MAX_PATH] = {0};
				CStringConversion::StringToWideChar(pFileName, szTmp, MAX_PATH - 1); 
				::SkinSetControlAttr(it->first, L"PeerHeader", L"floatimagefilename", szTmp);
				::SkinSetControlAttr(it->first, L"peerlogo", L"floatimagefilename", szTmp);
			} //end if 
		} //end for(..
	}
	return E_NOTIMPL;
}


//ICoreEvent
STDMETHODIMP CChatFrameImpl::DoCoreEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
	                     LPARAM lParam, HRESULT *hResult)
{
    if (::stricmp(szType, "lbdblclick") == 0)
	{
		*hResult = DoDblClkEvent(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "click") == 0)
	{
		*hResult = DoClickEvent(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "menucommand") == 0)
	{
		*hResult = DoMenuClickEvent(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "enterkeydown") == 0)
	{
		SHORT sCtrl = ::GetKeyState(VK_CONTROL) & 0xF000;
		if ((sCtrl != 0) && (!m_bEnterSend))
		{
			SendMessageToPeer(hWnd);
			*hResult = 0;
		} else if ((sCtrl == 0) && m_bEnterSend)
		{
			SendMessageToPeer(hWnd);
		    *hResult = 0;
		}
	} else if (::stricmp(szType, "OnPosChanged") == 0)
	{
		*hResult = DoWindowPosChanged(hWnd, wParam, lParam);
	} else if (::stricmp(szType, "afterinit") == 0)
	{
		*hResult = DoAfterInitEvent(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "itemselect") == 0)
	{
		*hResult = DoItemSelectEvent(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "layout") == 0)
	{
		*hResult = DoLayoutEvent(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "initmenupopup") == 0)
	{
		*hResult = DoInitMenuPopup(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "afterinitmenu") == 0)
	{
		*hResult = DoAfterInitMenu(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "link") == 0)
	{
		*hResult = DoLinkEvent(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "onclosequery") == 0)
	{
		if (::stricmp(szName, "chatWindow") == 0)
		{
			if (!CanClosed(hWnd))
				*hResult = 0;
		}
	} else
	{
		//PRINTDEBUGLOG(dtInfo, "chat frame event, type:%s name:%s", szType, szName);
	}
	//end if (::stricmp(szType, "lbdblclick") == 0
	return S_OK;
}

STDMETHODIMP CChatFrameImpl::SetCoreFrameWork(ICoreFrameWork *pCore)
{
	if (m_pCore)
		m_pCore->Release();
	m_pCore = pCore;
	if (m_pCore)
	{
		m_pCore->AddRef();

		//order event
		m_pCore->AddOrderEvent((ICoreEvent *) this, UI_MAIN_WINDOW_NAME, UI_COMPANY_TREE_NAME_STR, "lbdblclick");
		m_pCore->AddOrderEvent((ICoreEvent *) this, UI_MAIN_WINDOW_NAME, "treegroup", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, UI_MAIN_WINDOW_NAME, "treeleaf", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, UI_MAIN_WINDOW_NAME, UI_MAIN_WINDOW_NAME, "afterinit");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "ChatWindow", NULL, NULL); //order chat frame all event 
		m_pCore->AddOrderEvent((ICoreEvent *) this, "rmcWindow", NULL, NULL);
		m_pCore->AddOrderEvent((ICoreEvent *) this, "rmcsetwindow", NULL, NULL);
		m_pCore->AddOrderEvent((ICoreEvent *) this, "authwindow", NULL, NULL);
		m_pCore->AddOrderEvent((ICoreEvent *) this, "rmcimagesetwindow", NULL, NULL);

		//order protocol
		m_pCore->AddOrderProtocol((IProtocolParser *) this, "msg", NULL); //msg all protocol
		m_pCore->AddOrderProtocol((IProtocolParser *) this, "trs", NULL); //中转协议
		m_pCore->AddOrderProtocol((IProtocolParser *) this, "sys", "login"); //登陆成功后的协议
		m_pCore->AddOrderProtocol((IProtocolParser *) this, "sys", "presence"); //状态消息
	}
	return S_OK;
}

void CChatFrameImpl::GetChatFrameResource(const char *p, int nSize)
{
	m_strFileTransSkinXml = "<Control xsi:type=\"FileProgressBar\" name=\"filename\" filename=\"文件2\" filesize=\"224123413\" fileposition=\"23428342\"/>";
	TiXmlDocument xmldoc;
	if (xmldoc.Load(p, nSize))
	{
		 		//find frame window
		TiXmlElement *pXmlRoot = xmldoc.RootElement();
		if (pXmlRoot)
		{ 
			TiXmlElement *pFrame = pXmlRoot->FirstChildElement();
			for (;pFrame != NULL;)
			{
				if (stricmp(pFrame->Value(), "FrameWindow") == 0)
					break;
				pFrame = pFrame->NextSiblingElement();
			}
			if (pFrame)
			{
				TiXmlElement *pWindow = pFrame->FirstChildElement();
				while (pWindow)
				{
					if (stricmp(pWindow->Attribute("name"), "filetransprogress") == 0)
					{
						TiXmlElement *pTrans = pWindow->FirstChildElement();
						TiXmlString xml;
						if (pTrans)
						{
							pTrans->SaveToString(xml, 0);
							m_strFileTransSkinXml = xml.c_str();
						} //end if (pTrans)
					} //end if (stricmp(pFrame->...
					pWindow = pWindow->NextSiblingElement();
				} //end while (pWindow)
			} //end if (pFrame)
		} //end if (pXmlRoot)
	} //end if (xmldoc..
}

STDMETHODIMP CChatFrameImpl::GetSkinXmlString(IAnsiString *szXmlString)
{
	HRESULT hr = E_FAIL;
	IConfigure *pCfg = NULL; 
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{ 
		hr = pCfg->GetSkinXml("chatframe.xml",szXmlString); 
		if (SUCCEEDED(hr))
		{
			int nSize = szXmlString->GetSize() + 1;
			char *p = new char[nSize];
			if (SUCCEEDED(szXmlString->GetString(p, &nSize)))
			{
				p[nSize] = '\0';
				GetChatFrameResource(p, nSize - 1);
			}
			delete []p;
		}
		pCfg->Release();
	}
	return hr;  
}

STDMETHODIMP CChatFrameImpl::VideoConnected(LPARAM lParam, WPARAM wParam)
{
	if (m_VideoChlId)
	{
		::SkinRichEditInsertTip(m_VideoInfo.m_hOwner, UI_CHATFRAME_DISPLAY_EDIT_NAME,
			          NULL, 0, L"连接成功，开始视频通讯");
		RECT rcPlayer = {0}, rcCapture = {0};
		::SkinGetControlRect(m_VideoInfo.m_hOwner, L"PeerVideo", &rcPlayer);
		::SkinGetControlRect(m_VideoInfo.m_hOwner, L"SelfVideo", &rcCapture);
//		::VideoSetPlayerRect(m_VideoChlId, &rcPlayer);
//		::VideoStartCapture(m_VideoInfo.m_hOwner, &rcCapture);
	    return S_OK;
	}
	return E_FAIL;
}

CUserChatFrame *CChatFrameImpl::GetChatFrameByHWND(HWND hChat)
{
	std::map<HWND, CUserChatFrame *>::iterator it = m_ChatFrameList.find(hChat);
	if (it != m_ChatFrameList.end())
		return it->second;
	return FALSE;
}

STDMETHODIMP CChatFrameImpl::GetUserNameByHWND(HWND hOwner, IAnsiString *strUserName)
{
	CUserChatFrame *pFrame = GetChatFrameByHWND(hOwner);
	if (pFrame)
	{
		strUserName->SetString(pFrame->GetUserName());
		return S_OK;
	}
	return E_FAIL;
}

//
STDMETHODIMP CChatFrameImpl::CoreFrameWorkError(int nErrorNo, const char *szErrorMsg)
{
	switch(nErrorNo)
	{
	case CORE_ERROR_SOCKET_CLOSED:
	case CORE_ERROR_KICKOUT:
		 {
			std::map<HWND, CUserChatFrame *>::iterator it;
			for (it = m_ChatFrameList.begin(); it != m_ChatFrameList.end(); it ++)
			{
				TerminatedTransFileByHWND(it->first, TRUE);
			}
		 }
		 break;
	case CORE_ERROR_LOGOUT:
		 {
			 std::map<HWND, CUserChatFrame *>::iterator it;
			 for (it = m_ChatFrameList.begin(); it != m_ChatFrameList.end(); it ++)
			 {
				TerminatedTransFileByHWND(it->first, TRUE);
			 }
			 ClearChatFrame();
			 m_TransFileList.Clear();
		    // m_CustomPics;
		     m_dwLastShake = 0; 
			 m_strImagePath.clear();
			 m_strRealName.clear();
			 m_strUserName.clear();
			 m_strTransferPort.clear();
			 m_strTransferIp.clear();
		 }
		 break;
	}
	return E_NOTIMPL;
}

LRESULT CChatFrameImpl::OnWMDropFiles(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HDROP hDrop = (HDROP)wParam;
	if (hDrop == NULL)
		return - 1;

	TCHAR szFile[MAX_PATH]; 
	char szFileName[MAX_PATH] = {0};
	int iFileCount = ::DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	if (iFileCount <= 5)
	{
		for (int i = 0; i < iFileCount; ++i)
		{
			memset(szFile, 0, MAX_PATH * sizeof(TCHAR));
			if (::DragQueryFile(hDrop, i, szFile, MAX_PATH))
			{
				memset(szFileName, 0, MAX_PATH);
				CStringConversion::WideCharToString(szFile, szFileName, MAX_PATH - 1);
				if (CSystemUtils::FileIsExists(szFileName))
				{
					SendFileToPeer(hWnd, szFileName); 
				} //防止拖入文件
			} //end if (::DragQueryFile(..
		} //end for (int i
	} else
	{
		::SkinMessageBox(hWnd, L"本应用程序最多只支持同时拖曳5个文件", L"提示", MB_OK); 
	}
	::DragFinish(hDrop);
	return 0;
}

WORD  GetKeyShiftState()
{ 
	WORD wShift = 0;
    if ((GetAsyncKeyState(VK_LSHIFT) & 0x8000) != 0)
		wShift |= KEY_SHIFT_LSHIFT; 
	if ((GetAsyncKeyState(VK_RSHIFT) & 0x8000) != 0)
		wShift |= KEY_SHIFT_RSHIFT;
    if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) != 0)
		wShift |= KEY_SHIFT_LCTRL; 
    if ((GetAsyncKeyState(VK_RCONTROL) & 0x8000) != 0)
		wShift |= KEY_SHIFT_RCTRL; 
	if ((GetAsyncKeyState(VK_LMENU) & 0x8000) != 0)
		wShift |= KEY_SHIFT_LALT; 
    if ((GetAsyncKeyState(VK_RMENU) & 0x8000) != 0)
		wShift |= KEY_SHIFT_RALT;
	return wShift;
} 

//
STDMETHODIMP CChatFrameImpl::DoWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes)
{
	switch(uMsg)
	{
		case WM_DESTROY:
		{
			std::map<HWND, CUserChatFrame *>::iterator it = m_ChatFrameList.find(hWnd);
			if (it != m_ChatFrameList.end())
			{
				//delete Order Status
				IContacts *pContacts = NULL;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContacts)))
				{
					std::string strXml = "<i u=\"";
					strXml += it->second->GetUserName();
					strXml += "\"/>";
					pContacts->DeleteOrderUserList(strXml.c_str());
					pContacts->Release();
				}
				delete it->second;
				m_ChatFrameList.erase(it);
				//save frame pos
				RECT rc = {0};
				if (::GetWindowRect(hWnd, &rc))
				{
					m_rcLastOpen.right = m_rcLastOpen.left + rc.right - rc.left;
					m_rcLastOpen.bottom = m_rcLastOpen.top + rc.bottom - rc.top;
					IConfigure *pCfg = NULL;		
					HRESULT hr = m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg);
					if (SUCCEEDED(hr))
					{ 
						std::string strRect;
						CSystemUtils::RectToString(rc, strRect);
						pCfg->SetParamValue(FALSE, "Position", "ChatFrame", strRect.c_str());
						pCfg->Release();
					}
				}
				return S_OK;
			}
			break;
		}
		case WM_DROPFILES:
		{
			OnWMDropFiles(hWnd, wParam, lParam);
			break;
		}	
		case WM_KEYDOWN:
		{
			if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) == 0)
				::SkinSetControlFocus(hWnd, UI_CHARFRAME_INPUT_EDIT_NAME, TRUE); 
			
			break;
		}
	    case WM_SHOWCHATMESSAGE:
		{
			ShowChatMessage((HWND)wParam, (char *)lParam); 
			break;
		}
		case WM_SHOWCHATTIPMSG:
		{
			ShowChatTipMsg((HWND)wParam, (TCHAR *)lParam); 
			break;
		} 	
		case WM_OPENCHATFRAME:
		{ 
			ShowChatFrame(hWnd, (char *)lParam, NULL);
			*lRes = 0;  
			break;
		}

		case WM_VIDEO_CONNECTED:
		{
			VideoConnected(wParam, lParam); 
			break;
		}
		case  WM_CHAT_UPDL_PROGR:
		{
			CTransferFileInfo Info;
			if (m_TransFileList.GetFileInfoById(wParam, Info))
			{ 
				TCHAR szTmp[20] = {0};
				::_itow(lParam, szTmp, 10);
				::SkinSetControlAttr(Info.hOwner, Info.m_strProFlag.GetData(), L"currfilesize", szTmp);						 
			}
			break;
		} 
		case WM_TIMER:
		{
			CUserChatFrame *pFrame = GetChatFrameByHWND(hWnd);
			if (pFrame)
				pFrame->Shake();
			break;
		} 
		case WM_CHAT_RMFILE_PRO:
		{
			//
			RMFileProgress(wParam, lParam);
			break;
		} 
		case WM_CHAT_APPEND_PRO:
		{
			CTransferFileInfo *pInfo = (CTransferFileInfo *)lParam;
			TCHAR szTmp[MAX_PATH] = {0};
			int nFlagSize = MAX_PATH - 1;
			::SkinAddChildControl(pInfo->hOwner, L"fileprogress", m_strFileTransSkinXml.c_str(), szTmp, &nFlagSize, 999999);
			pInfo->m_strProFlag = szTmp;
			memset(szTmp, 0, MAX_PATH * sizeof(TCHAR));
			CStringConversion::StringToWideChar(pInfo->m_strDspName.c_str(), szTmp, MAX_PATH - 1);
			::SkinSetControlAttr(pInfo->hOwner, pInfo->m_strProFlag.GetData(), L"filename", szTmp);
			memset(szTmp, 0, MAX_PATH * sizeof(TCHAR));
			::_itow(pInfo->m_dwFileSize, szTmp, 10);
			::SkinSetControlAttr(pInfo->hOwner, pInfo->m_strProFlag.GetData(), L"filesize", szTmp);
			::SkinSetControlAttr(pInfo->hOwner, pInfo->m_strProFlag.GetData(), L"currfilesize", L"0");
			::SkinSetControlAttr(pInfo->hOwner, L"chattabtip", L"currentpage", L"filetab");
			::SkinSetControlAttr(pInfo->hOwner, pInfo->m_strProFlag.GetData(), L"progrestyle", L"recv");
			::SkinUpdateControlUI(pInfo->hOwner, pInfo->m_strProFlag.GetData());
			if (!pInfo->m_bSender)
				::SkinSetControlVisible(pInfo->hOwner, L"allfileset", TRUE);
			CSystemUtils::FlashWindow(pInfo->hOwner);
			break;
		}
		case WM_CHAT_REPLACE_PIC:
		{
			char *szFileName = (char *)wParam;
			char *szTag = (char *)lParam;
			::SkinReplaceImageInRichEdit(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, 
							szFileName, szTag);
			break;
		} 
		case WM_CHAT_COMMAND:
		{
			HWND h = (HWND)wParam;
			DoChatCommand(h, (int) lParam);
			break;
		}
		case WM_SHOW_FILE_LINK:
		{
			HWND h = (HWND) wParam;
			LPFILE_LINK_DATA pData = (LPFILE_LINK_DATA) lParam;
			::SkinRichEditAddFileLink(h, UI_CHATFRAME_DISPLAY_EDIT_NAME, NULL, 20, pData->strTip.c_str(), pData->strFileName.c_str());
			delete pData;
			break;
		}
		case WM_RM_FILEPROGRESS:
		{
			HWND h = (HWND) wParam;
			TCHAR *szwTmp = (TCHAR *)lParam;
			::SkinRemoveChildControl(h, L"fileprogress", szwTmp);
			delete []szwTmp;
			break;
		}
	}
	return E_FAIL;
}

void CChatFrameImpl::RMFileProgress(WPARAM wParam, LPARAM lParam)
{
	HWND h = NULL; 
	if (lParam == 0)
	{
		CTransferFileInfo *pInfo = (CTransferFileInfo *)wParam; 
		if (GetChatFrameByHWND(pInfo->hOwner) != NULL)
		{
			::SkinRemoveChildControl(pInfo->hOwner, L"fileprogress", pInfo->m_strProFlag.GetData());
			
		    //CancelCustomLink(pInfo->hOwner, pInfo->m_nLocalFileId, CUSTOM_LINK_FLAG_RECV | CUSTOM_LINK_FLAG_SAVEAS
			//	              | CUSTOM_LINK_FLAG_CANCEL | CUSTOM_LINK_FLAG_REFUSE | CUSTOM_LINK_FLAG_OFFLINE);
			//::lstrcpy(szwProFlag, pInfo->m_strProFlag.GetData());
			h = pInfo->hOwner; 
		}
	} else
	{
		h = (HWND)wParam;
		TCHAR *szFlag = (TCHAR *)lParam;
		::SkinRemoveChildControl(h, L"fileprogress", szFlag);
		//::lstrcpy(szwProFlag, szFlag);
		delete []szFlag;
	}
	if (h != NULL)
	{ 
		ChangeAllFileSetVisible(h);
		if (!m_TransFileList.HasOwnerWindow(h))
		{
			if ((m_rmcChlId > 0) && (m_rmcInfo.m_hOwner == h))
			{
				if (m_rmcInfo.m_bRequest)
				{
					DoChatCommand(h, CHAT_COMMAND_TAB2RMC_REQUEST);
					if (m_rmcInfo.m_bConnected)
					{
						DoChatCommand(h, CHAT_COMMAND_TAB2RMC_SWCTRL);
					}  
				} else
				{ 
					DoChatCommand(h, CHAT_COMMAND_TAB2RMC_CONTROL);
					if (m_rmcInfo.m_bConnected)
					{
						DoChatCommand(h, CHAT_COMMAND_TAB2RMC_SHOW);
					}  
				}
			} else
				DoChatCommand(h, CHAT_COMMAND_TAB2INFO);
		}
		//::PostMessage(h, WM_RM_FILEPROGRESS, (WPARAM)h, (LPARAM)szwProFlag);
	}
}
//
void CChatFrameImpl::ChangeAllFileSetVisible(HWND hWnd)
{
	if (m_TransFileList.HasPendingRecvFile(hWnd))
		::SkinSetControlVisible(hWnd, L"allfileset", TRUE);
	else
		::SkinSetControlVisible(hWnd, L"allfileset", FALSE);
}

void CChatFrameImpl::DoChatCommand(HWND hOwner, int nCommand)
{
	switch(nCommand)
	{
	case CHAT_COMMAND_TAB2RMC_REQUEST:    //切换至远程协助的请求界面
		{
			::SkinSetControlAttr(hOwner, L"chattabtip", L"currentpage", L"rmctab");
			::SkinSetControlVisible(hOwner, L"rmcRequestFrame", TRUE);
			::SkinSetControlVisible(hOwner, L"rmcControlFrame", FALSE);
			::SkinSetControlVisible(hOwner, L"requestContrl", FALSE); 
			break;
		}
	case CHAT_COMMAND_TAB2RMC_CONTROL:  //切换至远程协助的控制界面
		{
			::SkinSetControlAttr(hOwner, L"chattabtip", L"currentpage", L"rmctab");
			::SkinSetControlVisible(hOwner, L"rmcRequestFrame", FALSE);
			::SkinSetControlVisible(hOwner, L"rmcControlFrame", TRUE);
			::SkinSetControlVisible(hOwner, L"rmcControlCnnt", TRUE);
			::SkinSetControlVisible(hOwner, L"rmcShowArea", FALSE); 
			break;
		}
	case CHAT_COMMAND_TAB2FILE_PRO:  //切换至文件传输进度界面
		{
			if ((m_rmcChlId > 0) && (hOwner == m_rmcInfo.m_hOwner))
			{
				RECT rc = {0};
				::RmcAdjustPlayRect(m_rmcChlId, &rc);
			}
			::SkinSetControlAttr(hOwner, L"chattabtip", L"currentpage", L"filetab");
			ChangeAllFileSetVisible(hOwner); 
			break;
		}
	case CHAT_COMMAND_TAB2INFO:   //切换至信息显示界面
		{
			if ((m_rmcChlId > 0) && (hOwner == m_rmcInfo.m_hOwner))
			{
				RECT rc = {0};
				::RmcAdjustPlayRect(m_rmcChlId, &rc);
			}
			::SkinSetControlAttr(hOwner, L"chattabtip", L"currentpage", L"infotab");
			break;
		}
	case CHAT_COMMAND_CLEAR_RMC_CHL:
		{
			ClearRmcChannel();
			break;
		}
	case CHAT_COMMAND_TAB2RMC_SHOW: //显示远程桌面
		{
			::SkinSetControlVisible(hOwner, L"rmcShowArea", TRUE);
			::SkinSetControlVisible(hOwner, L"rmcControlCnnt", FALSE);
			if ((m_rmcInfo.m_hOwner == hOwner) && (m_rmcInfo.m_hSetWnd == NULL))
			{
				RECT rc = {0};
			    ::SkinGetControlRect(hOwner, L"rmcShowWindow", &rc);
			    ::RmcAdjustPlayRect(m_rmcChlId, &rc);
			}
			break;
		}
	case CHAT_COMMAND_TAB2RMC_SWCTRL: //显示申请控制
		{
			::SkinSetControlVisible(hOwner, L"requestContrl", TRUE);
			::SkinUpdateControlUI(hOwner, L"requestContrl");
			break;
		} //end case 
	case CHAT_COMMAND_TAB2AV: //音视频显示界面
		{
			if ((m_rmcChlId > 0) && (hOwner == m_rmcInfo.m_hOwner))
			{
				RECT rc = {0};
				::RmcAdjustPlayRect(m_rmcChlId, &rc);
			}
			::SkinSetControlAttr(hOwner, L"chattabtip", L"currentpage", L"mediatab");
			break;
		}
	} //end switch(..
	//
	::SkinSetControlFocus(hOwner, UI_CHARFRAME_INPUT_EDIT_NAME, TRUE);
}

//IProtocolParser
STDMETHODIMP CChatFrameImpl::DoRecvProtocol(const BYTE *pData, const LONG lSize)
{
	TiXmlDocument xmlDoc;
	BOOL bDid = FALSE;
	if (xmlDoc.Load((char *)pData, lSize))
	{
		TiXmlElement *pNode = xmlDoc.FirstChildElement();
		if (pNode)
		{
			const char *szName = pNode->Value();
			const char *szType = pNode->Attribute("type"); 
			if (::stricmp(szName, "msg") == 0)
			{
				bDid = DoMessageProtocol(szType, pNode);
			} else if (::stricmp(szName, "trs") == 0)
			{
				bDid = DoTransferProtocol(szType, pNode);
			} else if (::stricmp(szName, "sys") == 0)
			{
				bDid = DoSystemProtocol(szType, pNode);
			} else 
			{
				PRINTDEBUGLOG(dtInfo, "invalid chat protocol, node name:%s type:%s", szName, szType);
			} //end if (stricmp(szNodeName...			  
		} //end if (pNode)
	} //end if (xmlDoc.Load(...
	if (bDid)
		return S_OK;
	else
		return E_FAIL;
}

//IChatFrame
STDMETHODIMP_(HWND) CChatFrameImpl::ShowChatFrame(HWND hWndFrom, const char *szUserName, const char *szUTF8DspName)
{
	std::map<HWND, CUserChatFrame *>::iterator it;
	BOOL bOpened = FALSE;
	for (it = m_ChatFrameList.begin(); it != m_ChatFrameList.end(); it ++)
	{
		if (it->second->IsUserNameFrame(szUserName))
		{
			if (::IsWindow(it->first))
			{
				CSystemUtils::BringToFront(it->first);
				return it->first;
			} else
			{
				delete it->second;
				m_ChatFrameList.erase(it);
				break;
			}
		} //end if (it->second->...
	} //end for (it = m_ChatFrameList...
	return OpenChatFrame(hWndFrom, szUserName, szUTF8DspName); 
}

STDMETHODIMP CChatFrameImpl::SendFileToPeer(const char *szUserName, const char *szFileName)
{
	HWND h = ShowChatFrame(NULL, szUserName, NULL);
	if (h != NULL)
	{
		BOOL bSucc = FALSE;
		if (szFileName != NULL)
			bSucc = SendFileToPeer(h, szFileName);
		else
			bSucc = SelectFileAndSend(h); 
		return S_OK;
	}
	return E_FAIL;
}

STDMETHODIMP CChatFrameImpl::SendRmcRequest(const char *szUserName)
{
	HWND h = ShowChatFrame(NULL, szUserName, NULL);
    if ((h != NULL) && SendRtoRequest(h))
		return S_OK;
	return E_FAIL; 
}

STDMETHODIMP CChatFrameImpl::SendVideoRequest(const char *szUserName)
{
	HWND h = ShowChatFrame(NULL, szUserName, NULL);
	if ((h != NULL) && SendVideoRequest(h))
		return S_OK;
	return E_FAIL;
}

STDMETHODIMP CChatFrameImpl::SendAudioRequest(const char *szUserName)
{
	return E_NOTIMPL;
}

void CChatFrameImpl::RefreshLastOpenFrameRect()
{
	if (::IsRectEmpty(&m_rcLastOpen))
	{
		if (m_pCore)
		{
			IConfigure *pCfg = NULL;		
			HRESULT hr = m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg);
			if (SUCCEEDED(hr))
			{
				CInterfaceAnsiString strPos;
				if (SUCCEEDED(pCfg->GetParamValue(FALSE, "Position", "ChatFrame", (IAnsiString *)&strPos)))
				{
					CSystemUtils::StringToRect(&m_rcLastOpen, strPos.GetData());
				} else
				{
					memset(&m_rcLastOpen, 0, sizeof(RECT));
				} //end else if (SUCCEEDED(pCfg->GetParamValue...
				pCfg->Release();
			} //end if (SUCCEEDED(hr))
		} //end if (m_pCore)
	} else
	{
		::OffsetRect(&m_rcLastOpen, CHAT_FRAME_OFFSET_X, CHAT_FRAME_OFFSET_Y);
		RECT rcScreen;
		CSystemUtils::GetScreenRect(&rcScreen);
		if ((m_rcLastOpen.right > rcScreen.right)
			|| (m_rcLastOpen.bottom > rcScreen.bottom))
		{
			int dx =  CHAT_FRAME_OFFSET_X / 2 - m_rcLastOpen.left;
			int dy = CHAT_FRAME_OFFSET_Y / 2 - m_rcLastOpen.top;
			::OffsetRect(&m_rcLastOpen, dx, dy);
		} //end if ((m_rcLastOpen..
	} //end else if (::IsRectEmpty(...
	if (((m_rcLastOpen.right - m_rcLastOpen.left) < 100) || ((m_rcLastOpen.bottom - m_rcLastOpen.top) < 100)
		|| (m_rcLastOpen.right < 0) || (m_rcLastOpen.bottom < 0) || ::IsRectEmpty(&m_rcLastOpen))
	{
		m_rcLastOpen.left = 100;
		m_rcLastOpen.top = 100;
		m_rcLastOpen.right = 700;
		m_rcLastOpen.bottom = 600;
	}	 
}

//
void CChatFrameImpl::SendAuthRequest(const char *szUserName)
{
	SendReceiptToPeer(szUserName, "对方发起了与您的会话请求", "request");
}

//
void CChatFrameImpl::InitSelfChatFrame()
{
	if (m_strUserName.empty())
	{
		CInterfaceAnsiString strUserName;
		m_pCore->GetUserName((IAnsiString *)&strUserName);
		m_strUserName = strUserName.GetData();
		m_pCore->GetUserDomain((IAnsiString *)&strUserName);
		m_strUserName += "@";
		m_strUserName += strUserName.GetData();
	}
	if (m_strRealName.empty())
	{
		CInterfaceAnsiString strTmp; 
		m_pCore->GetUserNickName((IAnsiString *)&strTmp);
		m_strRealName = strTmp.GetData();
	}
}

HWND CChatFrameImpl::OpenChatFrame(HWND hWndFrom, const char *szUserName, const char *szUTF8DspName)
{
	HRESULT hr = E_FAIL;
	if (m_pCore)
	{
		InitSelfChatFrame();
		if (::stricmp(szUserName, m_strUserName.c_str()) == 0)
			return NULL;
		//m_pCor
		if (FAILED(m_pCore->GetFrontPendingMsg(szUserName, "p2p", NULL, FALSE)))
		{
			//检测是否具有与对方发起谈话的权限
			IContacts *pContact = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
			{
				BOOL bNeedAuth = FALSE;
				if (!pContact->IsAuthContact(szUserName))
				{ 
					bNeedAuth = TRUE;
					HWND h = hWndFrom;
					if (h == NULL)
						h = GetMainFrameHWND(); 
					if (::SkinMessageBox(h, L"没有发起权限, 是否要向对方申请发起谈话",
						L"提示", 2) == IDOK)
					{
						//发起申请
						SendAuthRequest(szUserName);
					}  
				}
				pContact->Release(); 
				if (bNeedAuth)
					return NULL;
			} //end if (SUCCEEDED(m_pCore->
		} //end if (FAILED(..

		IUIManager *pUI = NULL; 		
		hr = m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI);
		if (SUCCEEDED(hr) && pUI)
		{
			HWND hTemp;
			char szDspName[MAX_PATH] = {0};
			TCHAR szTmp[MAX_PATH] = {0};
			CInterfaceAnsiString strDeptName;
			CInterfaceAnsiString strUserStatus;
			if (szUTF8DspName && (::strlen(szUTF8DspName) > 0))
			{
				strncpy(szDspName, szUTF8DspName, MAX_PATH - 1);
			} else
			{
				IContacts *pContact = NULL;
				if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
				{
					std::string strName, strDomain;
					strName = szUserName;
					int nPos = strName.find('@');
					if (nPos != std::string::npos)
					{
						strDomain = strName.substr(nPos + 1);
						strName = strName.substr(0, nPos);
					}
					CInterfaceAnsiString strDspName;
					pContact->GetRealNameById(strName.c_str(), strDomain.c_str(), (IAnsiString *)&strDspName);
					strncpy(szDspName, strDspName.GetData(), MAX_PATH - 1);
					pContact->Release();
				} // end if (SUCCEEDED(...
			} //end else if (szUTF8DspName)
			CStringConversion::UTF8ToWideChar(szDspName, szTmp, MAX_PATH - 1);
			TCHAR szCaption[512] = {0};
			::wsprintf(szCaption, L"与 %s 交谈中", szTmp);
			RefreshLastOpenFrameRect();
			m_bInitFrame = TRUE;
			//PRINTDEBUGLOG(dtInfo, "Open Chat Frame: left:%d Top:%d Right:%d Bottom:%d", m_rcLastOpen.left, m_rcLastOpen.top, m_rcLastOpen.right, m_rcLastOpen.bottom);
			pUI->CreateUIWindow(NULL, "ChatWindow", &m_rcLastOpen, WS_POPUP | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX
				                | WS_MINIMIZEBOX, WS_EX_ACCEPTFILES, szCaption, &hTemp);
			CUserChatFrame *pFrame = new CUserChatFrame(szUserName, hTemp);
			pFrame->SetDispName(szDspName);
			m_ChatFrameList.insert(std::pair<HWND, CUserChatFrame *>(hTemp, pFrame));
			//Order window message
			pUI->OrderWindowMessage("ChatWindow", hTemp, WM_DESTROY, (ICoreEvent *) this);
			pUI->OrderWindowMessage("ChatWindow", hTemp, WM_DROPFILES, (ICoreEvent *) this);
			pUI->OrderWindowMessage("ChatWindow", hTemp, WM_TIMER, (ICoreEvent *) this);
			pUI->OrderWindowMessage("ChatWindow", hTemp, WM_CHAT_REPLACE_PIC, (ICoreEvent *) this);
			pUI->OrderWindowMessage("ChatWindow", hTemp, WM_RM_FILEPROGRESS, (ICoreEvent *) this);
			pUI->OrderWindowMessage("ChatWindow", hTemp, WM_CHAT_RMFILE_PRO, (ICoreEvent *) this);
			pUI->OrderWindowMessage("CHatWindow", hTemp, WM_SHOW_FILE_LINK, (ICoreEvent *) this);

			IMainFrame *pMain = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMainFrame), (void **)&pMain)))
			{
				pMain->ShowRecentlyUser(szUserName, szDspName);
				pMain->Release();
			}

			//设置回调
			::SkinSetRichEditCallBack(hTemp, UI_CHATFRAME_DISPLAY_EDIT_NAME, RichEditCallBack, this);
			::SkinSetRichEditCallBack(hTemp, UI_CHARFRAME_INPUT_EDIT_NAME, RichEditCallBack, this);
			
			//
			
			IConfigure *pCfg = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
			{
				RefreshInputChatFont(hTemp, pCfg);
				CInterfaceAnsiString strValue;
				m_bEnterSend = TRUE;
				if (SUCCEEDED(pCfg->GetParamValue(FALSE, "hotkey", "ctrlentersendmsg", &strValue)))
				{
					if (::stricmp(strValue.GetData(), "true") == 0)
						m_bEnterSend = FALSE;
				}
				//设置是否智能合并
				if (SUCCEEDED(pCfg->GetParamValue(FALSE, "normal", "aimsg", &strValue)))
				{
					if (::stricmp(strValue.GetData(), "false") == 0)
					{
						::SkinSetControlAttr(hTemp, UI_CHATFRAME_DISPLAY_EDIT_NAME, L"mergemsg", L"false");
					} else
						::SkinSetMenuChecked(hTemp, L"chatshortcutmenu", 101, TRUE);
				}
				//设置是否透明
				if (SUCCEEDED(pCfg->GetParamValue(FALSE, "normal", "msgtransparent", &strValue)))
				{
					if (::stricmp(strValue.GetData(), "true") == 0)
					{
						::SkinSetControlAttr(hTemp, UI_CHATFRAME_DISPLAY_EDIT_NAME, L"transparent", L"true");
						::SkinSetMenuChecked(hTemp, L"chatshortcutmenu", 102, TRUE);
						if (SUCCEEDED(pCfg->GetParamValue(FALSE, "normal", "transimagefile", &strValue)))
						{
							if ((strValue.GetSize() > 0) && CSystemUtils::FileIsExists(strValue.GetData()))
							{
								TCHAR szTmp[MAX_PATH] = {0};
								CStringConversion::StringToWideChar(strValue.GetData(), szTmp, MAX_PATH - 1);
								::SkinSetControlAttr(hTemp, L"chatdisplaycanvs", L"imagefile", szTmp);
							}
						} //end if (SUCCEEDED(pCfg->
					} //end if (::stricmp(..
				} //end if (SUCCEEDED(
				pCfg->Release();
				//
			}

			if (m_bEnterSend)
				::SkinSetMenuChecked(hTemp, L"sendmenu", 30001, TRUE);
			else
				::SkinSetMenuChecked(hTemp, L"sendmenu", 30002, TRUE);

			//
			/*std::string strXml = "<msg type=\"p2p\" from=\"";
			strXml += m_strUserName;
			strXml += "\" to=\"";
			strXml += szUserName;
			strXml += "\"><version mode=\"request\"/></msg>";
			m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG)strXml.size(), 0);*/

			GetPendingMsgByName(hTemp, szUserName, szDspName);
			m_bInitFrame = FALSE;
			
			//order status
			IContacts *pContacts = NULL;
			hr = m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContacts);
			CInterfaceAnsiString strEMail, strCellPhone, strMobile, strHeaderName, strMyHeaderName;
			if (SUCCEEDED(hr))
			{
				CInstantUserInfo Info;
				pContacts->GetContactUserInfo(szUserName, &Info);
				Info.GetUserStatus(&strUserStatus);
				//
				pContacts->GetDeptPathNameByUserName(szUserName, &strDeptName);
				pContacts->GetMailByUserName(szUserName, &strEMail);
				pContacts->GetCellPhoneByName(szUserName, &strCellPhone);
				pContacts->GetPhoneByName(szUserName, &strMobile);
				pContacts->GetContactHead(szUserName, &strHeaderName, TRUE);
				pContacts->GetContactHead(m_strUserName.c_str(), &strMyHeaderName, FALSE);
				std::string strXml = "<i u=\"";
				strXml += szUserName;
				strXml += "\"/>";
				pContacts->AddOrderUserList(strXml.c_str());
				pContacts->Release();
			}
			if (strDeptName.GetSize() > 0)
			{
				memset(szTmp, 0, sizeof(TCHAR) * MAX_PATH);
				CStringConversion::UTF8ToWideChar(strDeptName.GetData(), szTmp, MAX_PATH - 1);
				::SkinSetControlTextByName(hTemp, L"lbldept", szTmp);
				::SkinSetControlAttr(hTemp, L"lbldept", L"tooltip", szTmp);
			}
			if (strEMail.GetSize() > 0)
			{
				memset(szTmp, 0, sizeof(TCHAR) * MAX_PATH);
				CStringConversion::UTF8ToWideChar(strEMail.GetData(), szTmp, MAX_PATH - 1);
				::SkinSetControlTextByName(hTemp, L"peermail", szTmp);
				::SkinSetControlAttr(hTemp, L"peermail", L"tooltip", szTmp);
			} 
			if (strCellPhone.GetSize() > 0)
			{
				memset(szTmp, 0, sizeof(TCHAR) * MAX_PATH);
				CStringConversion::UTF8ToWideChar(strCellPhone.GetData(), szTmp, MAX_PATH - 1);
				::SkinSetControlTextByName(hTemp, L"peertel", szTmp);
			}
			if (strMobile.GetSize() > 0)
			{
				memset(szTmp, 0, sizeof(TCHAR) * MAX_PATH);
				CStringConversion::UTF8ToWideChar(strMobile.GetData(), szTmp, MAX_PATH - 1);
				::SkinSetControlTextByName(hTemp, L"peerphone", szTmp);
			}
			if (strHeaderName.GetSize() > 0)
			{
				memset(szTmp, 0, sizeof(TCHAR) * MAX_PATH);
				CStringConversion::StringToWideChar(strHeaderName.GetData(), szTmp, MAX_PATH - 1);
				::SkinSetControlAttr(hTemp, L"PeerHeader", L"floatimagefilename", szTmp);
				::SkinSetControlAttr(hTemp, L"peerlogo", L"floatimagefilename", szTmp);
			}
			if (strMyHeaderName.GetSize() > 0)
			{
				memset(szTmp, 0, sizeof(TCHAR) * MAX_PATH);
				CStringConversion::StringToWideChar(strMyHeaderName.GetData(), szTmp, MAX_PATH - 1);
				::SkinSetControlAttr(hTemp, L"SelfHeader", L"floatimagefilename", szTmp);
			}
			DoPresenceChange(szUserName, strUserStatus.GetData(), NULL, FALSE);
			DoUserSignChanged(szUserName);
			if (::IsWindow(hTemp))
			{		 
				 ::ShowWindow(hTemp, SW_SHOW); 
				 ::SkinSetControlFocus(hTemp, UI_CHARFRAME_INPUT_EDIT_NAME, TRUE);
				 CSystemUtils::BringToFront(hTemp); 
			}
			pUI->Release();
			pUI = NULL;
			return hTemp;
		} //end if (SUCCEEDED(hr) && pUI)
	} //end if (m_pCore)
	return NULL;
}

void CChatFrameImpl::RefreshInputChatFont(HWND hWnd, IConfigure *pCfg)
{
	CInterfaceFontStyle FontStyle;
	if (SUCCEEDED(pCfg->GetChatFontStyle((IFontStyle *)&FontStyle)))
	{
		CCharFontStyle fs = {0};
		CXmlNodeTranslate::StringFontToStyle(&FontStyle, fs);
		::SkinNotifyEvent(hWnd, UI_CHARFRAME_INPUT_EDIT_NAME, L"setfont", 0, LPARAM(&fs));
	}
}

BOOL CChatFrameImpl::GetPendingMsgByName(HWND hWnd, const char *szUserName, const char *szDspName)
{
	if (m_pCore)
	{
		CInterfaceAnsiString strProtocol;
		while (SUCCEEDED(m_pCore->GetFrontPendingMsg(szUserName, "p2p", (IAnsiString *)&strProtocol, TRUE)))
		{
			TiXmlDocument xmlDoc;
			if (xmlDoc.Load(strProtocol.GetData(), strProtocol.GetSize()))
			{
				TiXmlElement *pNode = xmlDoc.FirstChildElement();
				RecvMessageFromPeer(hWnd, pNode, szDspName);
			} //end if (xmlDoc.Load(..
		} //end while (SUCCEEDED(m_pCore->...
		return TRUE;
	} // end if (m_pCore)
	return FALSE;
}

void CALLBACK HttpUpCallBack(int nErrorCode, int nType, WPARAM wParam, LPARAM lParam, void *pOverlapped)
{
	switch(nErrorCode)
	{
	case ERROR_CODE_COMPLETE:
		{
			if (nType == FILE_TYPE_CUSTOMPIC)
			{
				//custom picture download notify
				//// <trs type="custompic" from="wuxiaozhong@gocom"  to="admin@gocom" 
				//   filename="30f79afad4318774e447dc2db96936e0.gif" fileserver="http://imbs.smartdot.com.cn:9910" 
				CCustomPicItem *pItem = (CCustomPicItem *)pOverlapped;				
				CChatFrameImpl *pThis = (CChatFrameImpl *)pItem->m_pOverlapped;
				std::string strXml = "<trs type=\"custompic\" from=\"";
				strXml += pThis->m_strUserName;
				strXml += "\" to=\"";
				strXml += pItem->m_strPeerName;
				strXml += "\" filename=\"";
				char szTmp[MAX_PATH] = {0};
				CSystemUtils::ExtractFileName(pItem->m_strLocalFileName.c_str(), szTmp, MAX_PATH - 1);
				strXml += szTmp;
				strXml += "\" fileserver=\"";
				//url = "http://imbs.smartdot.com.cn:9910/upcustompic.php"
				int nPos = pItem->m_strUrl.find_last_of(":");
				std::string strSvr;
				if (nPos != std::string::npos)
				{
					nPos = pItem->m_strUrl.find("/", nPos);
					if (nPos != std::string::npos)
					{
						strSvr = pItem->m_strUrl.substr(0, nPos);
					} //end if (nPos != ...
				} //end if (nPos != std::...
				if (!strSvr.empty())
					strXml += strSvr;
				else
					strXml += pItem->m_strUrl;
				strXml += "\"/>"; 
				if (pThis->m_pCore)
					pThis->m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG)strXml.size(), 0); 
				PRINTDEBUGLOG(dtInfo, "Up Ole Picture %s SUCC", pItem->m_strLocalFileName.c_str());
				pThis->m_CustomPics.DeleteItem(pItem);
			} else if (nType == FILE_TYPE_NORMAL)
			{
				//<msg type="offlinefile" from="admin@gocom" to="wuxiaozhong@gocom" name="apss.dll.mui" filesize="3072" url="A90C62138DFB74BEA86244E2432D133EF.mui
				CCustomPicItem *pItem = (CCustomPicItem *)pOverlapped;				
				CChatFrameImpl *pThis = (CChatFrameImpl *)pItem->m_pOverlapped;
				std::string strXml = "<msg type=\"offlinefile\" from=\"";
				strXml += pThis->m_strUserName;
				strXml += "\" to=\"";
				strXml += pItem->m_strPeerName;
				strXml += "\" name=\"";
				char szTmp[MAX_PATH] = {0};
				CSystemUtils::ExtractFileName(pItem->m_strLocalFileName.c_str(), szTmp, MAX_PATH - 1);
				strXml += szTmp;
				strXml += "\" filesize=\"";
				memset(szTmp, 0, MAX_PATH);
				::itoa(pItem->m_nFileSize, szTmp, 10);
				strXml += szTmp;
				strXml += "\" url=\"";
				strXml += pItem->m_strFlag;
				memset(szTmp, 0, MAX_PATH);
				CSystemUtils::ExtractFileExtName(pItem->m_strLocalFileName.c_str(), szTmp, MAX_PATH - 1);
				strXml += ".";  //szTmp 扩展名不带 .
				strXml += szTmp;
				strXml += "\" fileserver=\"";
				//url = "http://imbs.smartdot.com.cn:9910/upcustompic.php"
				int nPos = pItem->m_strUrl.find_last_of(":");
				std::string strSvr;
				if (nPos != std::string::npos)
				{
					nPos = pItem->m_strUrl.find("/", nPos);
					if (nPos != std::string::npos)
					{
						strSvr = pItem->m_strUrl.substr(0, nPos);
					} //end if (nPos != ...
				} //end if (nPos != std::...
				if (!strSvr.empty())
					strXml += strSvr;
				else
					strXml += pItem->m_strUrl;
				strXml += "\"/>";
				if (pThis->m_pCore)
					pThis->m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG)strXml.size(), 0); 
				CStdString_ strTip = L"文件 ";
				TCHAR szwTmp[MAX_PATH] = {0};
				CStringConversion::StringToWideChar(pItem->m_strLocalFileName.c_str(), szwTmp, MAX_PATH - 1);
				strTip += szwTmp;
				strTip += L"  发送完毕";
				//add msg
				char szTime[64] = {0};
				CSystemUtils::DateTimeToStr((DWORD)::time(NULL), szTime);
				std::string strMsg, strBody;
				strBody = "离线文件";
				strBody += pItem->m_strLocalFileName.c_str();
				strBody += " 发送完毕";
				strMsg = "<tip datetime=\"";
				strMsg += szTime;
				strMsg += "\">";
				strMsg += strBody;
				strMsg += "</tip>";
				pThis->SaveP2PMsg("p2p",  pThis->m_strUserName.c_str(), pItem->m_strPeerName.c_str(), szTime, 
					strMsg.c_str(), strBody.c_str());
				pThis->AnsycShowTip(pItem->m_hOwner, strTip.GetData());
				pThis->RemoveTransFile(pItem->m_nFileId, NULL, TRUE); 
				pThis->m_CustomPics.DeleteItem(pItem);
			}//end if (wParam ==
			break;
		} //end case Error
	case ERROR_CODE_PROGRESS: //进度
		{
			if (nType == FILE_TYPE_NORMAL)
			{
				//显示进度				
				CCustomPicItem *pItem = (CCustomPicItem *)pOverlapped;				
				CChatFrameImpl *pThis = (CChatFrameImpl *)pItem->m_pOverlapped;
				::PostMessage(pThis->GetMainFrameHWND(), WM_CHAT_UPDL_PROGR, pItem->m_nFileId, lParam); 
			}
			break;
		}
	} //end switch(..
}

BOOL CChatFrameImpl::UploadLocalFileToServer(HWND hWnd, const char *szLocalFileName)
{
	CUserChatFrame *pFrame = GetChatFrameByHWND(hWnd);
	if (!pFrame)
		return FALSE;
	if (m_TransFileList.CheckIsTrans(pFrame->GetUserName(), szLocalFileName))
	{
		AnsycShowTip(hWnd, L"文件正在传送中");
		return FALSE;
	} 
	DWORD dwFileSize = (DWORD) CSystemUtils::GetFileSize(szLocalFileName);
	int nMaxFileSize = 0;
	IConfigure *pCfg = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		CInterfaceAnsiString strTmp;
		if (SUCCEEDED(m_pCore->GetSvrParams("offline.maxfilesize", &strTmp, FALSE)))
		{
			nMaxFileSize = ::atoi(strTmp.GetData());
		}
		pCfg->Release();
	}
	pCfg = NULL;
	if ((nMaxFileSize > 0) && (dwFileSize > nMaxFileSize))
	{
		TCHAR szwTmp[MAX_PATH] = {0};
		::wsprintf(szwTmp, L"文件太大，单个离线文件大小不能超过 %K", nMaxFileSize / 1024);
		ShowChatTipMsg(hWnd, szwTmp);
		return FALSE;
	}
	char szDspName[MAX_PATH] = {0};
	char szTag[MAX_PATH] = {0};
	char szFileId[32] = {0};
	int nTagSize = MAX_PATH - 1;
	::GetFileTagByName(szLocalFileName, szTag, &nTagSize);
	char szFileSize[32] = {0};
	char szTmp[32] = {0};
	::itoa(dwFileSize, szFileSize, 10);					
	CSystemUtils::ExtractFileName(szLocalFileName, szDspName, MAX_PATH - 1);
	TCHAR szFlag[MAX_PATH] = {0};
	TCHAR szwTmp[MAX_PATH] = {0};
	int nFlagSize = MAX_PATH - 1;
	::SkinAddChildControl(hWnd, L"fileprogress", m_strFileTransSkinXml.c_str(), szFlag, &nFlagSize, 999999);
	int nFileId = m_TransFileList.AddFileInfo(pFrame->GetUserName(), szDspName, szLocalFileName, szFlag, 
		                            szTag, "", "0", "", "0", "0", 0, szFileSize, hWnd, TRUE);
	DoChatCommand(hWnd, CHAT_COMMAND_TAB2FILE_PRO);
	memset(szwTmp, 0, MAX_PATH * sizeof(TCHAR));
	CStringConversion::StringToWideChar(szDspName, szwTmp, MAX_PATH - 1);
	::SkinSetControlAttr(hWnd, szFlag, L"filename", szwTmp);
	memset(szwTmp, 0, MAX_PATH * sizeof(TCHAR));
	CStringConversion::StringToWideChar(szFileSize, szwTmp, MAX_PATH - 1);
	::SkinSetControlAttr(hWnd, szFlag, L"filesize", szwTmp); 
	::SkinSetControlAttr(hWnd, szFlag, L"progrestyle", L"offline");
	::SkinUpdateControlUI(hWnd, szFlag);
	// 
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		CInterfaceAnsiString strUrl;
		if (SUCCEEDED(pCfg->GetServerAddr(HTTP_SVR_URL_OFFLINE_FILE, &strUrl)))
		{ 
			//
			CCustomPicItem *pItem = new CCustomPicItem();
			pItem->m_hOwner = hWnd;
			pItem->m_strFlag = szTag;
			pItem->m_pOverlapped = this;
			pItem->m_strLocalFileName = szLocalFileName; 
			CInterfaceAnsiString strUserName;
			GetUserNameByHWND(hWnd, &strUserName);
			pItem->m_strPeerName = strUserName.GetData(); 
			pItem->m_strUrl = strUrl.GetData();
			pItem->m_nFileSize = dwFileSize;
			pItem->m_nFileId = nFileId;
			std::string strParam = "filename=";
			strParam += szTag;
			strParam += ";username=";
			strParam += strUserName.GetData();
			if (m_CustomPics.AddItem(pItem))
			{			
				//draw to ui
				CStdString_ strTip = _T("您给\"");
				TCHAR szTmp[MAX_PATH] = {0};
				CStringConversion::UTF8ToWideChar(pFrame->GetDspName(), szTmp, MAX_PATH - 1);	
				strTip += szTmp;
				strTip += _T("\" 发送离线文件 \"");
				memset(szTmp, 0, MAX_PATH * sizeof(TCHAR));
				CStringConversion::StringToWideChar(szDspName, szTmp, MAX_PATH - 1);
				strTip += szTmp; 						
				
				//wsprintf(szTmp, L"\"\n \t <取消,%d>",
				//	            ((nFileId << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_CANCEL);
				//strTip + szTmp;
				::SkinRichEditInsertTip(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, NULL, 0, strTip.GetData());

				::P2SvrPostFile(strUrl.GetData(), szLocalFileName, strParam.c_str(), FILE_TYPE_NORMAL, 
					pItem, HttpUpCallBack, FALSE);
			} else
				delete pItem;
		}
		pCfg->Release();
		return TRUE;
	}
	return FALSE;
}

BOOL CChatFrameImpl::UploadCustomPicToServer(HWND hWnd, const char *szFlag)
{
	BOOL bSucc = FALSE;
	IEmotionFrame *pFrame = NULL; 
	CInterfaceAnsiString strFileName;
	BOOL bCustom = FALSE;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IEmotionFrame), (void **)&pFrame)))
	{
		if (SUCCEEDED(pFrame->GetSysEmotion(szFlag, &strFileName)))
			bSucc = TRUE;
		else if (SUCCEEDED(pFrame->GetCustomEmotion(szFlag, &strFileName)))
		{
			bCustom = TRUE;
		}
		pFrame->Release();
	}
	if (bSucc)
		return TRUE;
	
	char szFileName[MAX_PATH] = {0};
	if (bCustom)
	{
		strcpy(szFileName, strFileName.GetData());
	} else
		sprintf(szFileName, "%s%s.gif", GetImagePath(), szFlag);
	//
	IConfigure *pCfg = NULL;
	if ((CSystemUtils::FileIsExists(szFileName)) && m_pCore 
		&& SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		PRINTDEBUGLOG(dtInfo, "Up Ole Picture %s", szFileName);
		CInterfaceAnsiString strUrl;
		if (SUCCEEDED(pCfg->GetServerAddr(HTTP_SVR_URL_CUSTOM_PICTURE, &strUrl)))
		{
			CCustomPicItem *pItem = new CCustomPicItem();
			pItem->m_hOwner = hWnd;
			pItem->m_strFlag = szFlag;
			pItem->m_pOverlapped = this;
			pItem->m_strLocalFileName = szFileName; 
			CInterfaceAnsiString strUserName;
			GetUserNameByHWND(hWnd, &strUserName);
			pItem->m_strPeerName = strUserName.GetData(); 
			pItem->m_strUrl = strUrl.GetData();
			if (m_CustomPics.AddItem(pItem))
			{				
				::P2SvrPostFile(strUrl.GetData(), szFileName, NULL, FILE_TYPE_CUSTOMPIC, 
					pItem, HttpUpCallBack, FALSE);
			} else
				delete pItem;
		}
		pCfg->Release();
		return TRUE;
	}
	return FALSE;
}

BOOL CChatFrameImpl::SendOleResourceToPeer(HWND hWnd)
{
	//
	char *pOle = NULL;
	if (::SkinGetRichEditOleFlag(hWnd, UI_CHARFRAME_INPUT_EDIT_NAME, &pOle) && pOle)
	{ 
		int nIdx = 0;
		char szTmp[MAX_PATH] = {0};
		while (TRUE)
		{
			memset(szTmp, 0, MAX_PATH);
			if (!CSystemUtils::GetStringBySep(pOle, szTmp, ';', nIdx))
				break; 
			if (::strlen(szTmp) > 0)
				UploadCustomPicToServer(hWnd, szTmp);
			nIdx ++;
		}
		free(pOle);
	}
	return TRUE;
}

 
//<msg type="p2p" from="admin@gocom" to="wuxiaozhong@gocom" Receipt="" datetime="2010-09-28 15:09:49">
//    <font name="Arial" size="9pt" color="#000000" bold="false" underline="false" strikeout="false" italic="false"/>
//    <body>adfadfadf</body>
//</msg>
static const char P2P_MESSAGE_XML_FORMAT[] = "<msg type=\"p2p\"><font/><body></body></msg>";

BOOL CChatFrameImpl::SendReceiptToPeer(const char *szUserName, const char *szText, const char *szType)
{
	InitSelfChatFrame();	
	char szTime[64] = {0};
	CSystemUtils::DateTimeToStr((DWORD)::time(NULL), szTime);
	TiXmlDocument xmlDoc;
	if (xmlDoc.Load(P2P_MESSAGE_XML_FORMAT, ::strlen(P2P_MESSAGE_XML_FORMAT)))
	{
		TiXmlElement *pNode = xmlDoc.FirstChildElement();
		pNode->SetAttribute("from", m_strUserName.c_str());
		pNode->SetAttribute("to", szUserName);
		pNode->SetAttribute("Receipt", szType);
		pNode->SetAttribute("datetime", szTime);
		CCharFontStyle cf = {0};
		//Font
		TiXmlElement *pFont = pNode->FirstChildElement("font");
		if (pFont)
		{
			CXmlNodeTranslate::FontStyleToXmlNode(cf, pFont);
		}
		TiXmlElement *pBody = pNode->FirstChildElement("body");
		if (pBody)
		{
			TiXmlText pText(szText);
			pBody->InsertEndChild(pText);
		}
		TiXmlString strXml;
		xmlDoc.SaveToString(strXml, 0);
		//send to peer
		return SUCCEEDED(m_pCore->SendRawMessage((BYTE *)strXml.c_str(), strXml.size(), 0));
	} //end if (xmlDoc..
	 
	return FALSE;
}

BOOL CChatFrameImpl::CheckInputChars(const char *p)
{
	if (p)
	{
		int nSize = ::strlen(p); 
		const char *p1 = p;
		while (*p1 == ' ')
		{
			p1 ++;
			nSize --;
		}
		if (nSize > 1)
		{
			IConfigure *pCfg = NULL;
			if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
			{	
				BOOL bSucc = SUCCEEDED(pCfg->CheckKeyWord(p));
				pCfg->Release();
				return bSucc;
			} //end if (m_pCore ..
		} //end if (nSize > 1)
	}
	return FALSE;
}

#define MAX_SEND_TEXT_LENGTH 2000  //每次最多发送的字节数
#define MAX_SEND_WCHAR_TEXT_LENGTH 1000  //每次最多发送的汉字
BOOL CChatFrameImpl::SendMessageToPeer(HWND hWnd)
{
	if (m_pCore && m_pCore->CanAllowAction(USER_ROLE_SEND_MESSAGE) == S_OK)
	{
		//
		CCharFontStyle cf = {0};
		if (::SkinGetCurrentRichEditFont(hWnd, UI_CHARFRAME_INPUT_EDIT_NAME, &cf))
		{
			char *p = ::SkinGetRichEditOleText(hWnd, UI_CHARFRAME_INPUT_EDIT_NAME, 0);
			if (CheckInputChars(p)) //  p && strlen(p) > 1) //p 13
			{
				std::map<HWND, CUserChatFrame *>::iterator it = m_ChatFrameList.find(hWnd);
				if (it != m_ChatFrameList.end())
				{				
					TCHAR szValue[32] = {0};
					int nSize = 31;
					::SkinGetControlAttr(hWnd, L"receipt", L"down", szValue, &nSize);
					char szReceipt[32] = {0};
					CStringConversion::WideCharToString(szValue, szReceipt, 31);
					char szTime[64] = {0};
					CSystemUtils::DateTimeToStr((DWORD)::time(NULL), szTime);
					TiXmlDocument xmlDoc;
					if (xmlDoc.Load(P2P_MESSAGE_XML_FORMAT, ::strlen(P2P_MESSAGE_XML_FORMAT)))
					{
						TiXmlElement *pNode = xmlDoc.FirstChildElement();
						pNode->SetAttribute("from", m_strUserName.c_str());
						pNode->SetAttribute("to", it->second->GetUserName());
						pNode->SetAttribute("Receipt", szReceipt);
						pNode->SetAttribute("datetime", szTime);
						//Font
						TiXmlElement *pFont = pNode->FirstChildElement("font");
						if (pFont)
						{
							CXmlNodeTranslate::FontStyleToXmlNode(cf, pFont);
						}
						TiXmlElement *pBody = pNode->FirstChildElement("body"); 
						if (pBody)
						{ 
							TiXmlText pText("");
							pBody->InsertEndChild(pText);
						}
						int nSize = ::strlen(p);
						if (nSize > MAX_SEND_TEXT_LENGTH)
						{
							TCHAR *szwSrc = new TCHAR[nSize + 1];
							memset(szwSrc, 0, sizeof(TCHAR) * (nSize + 1));
							CStringConversion::StringToWideChar(p, szwSrc, nSize);
							int nOffset = 0;
							char szTmp[MAX_SEND_TEXT_LENGTH + 1];
							TCHAR szwTmp[MAX_SEND_WCHAR_TEXT_LENGTH + 1];
							int nwSize = ::lstrlen(szwSrc);
							while (nwSize > 0)
							{ 
								TiXmlNode *pText = pBody->FirstChild();
								if (nSize > MAX_SEND_WCHAR_TEXT_LENGTH)
								{
									memset(szTmp, 0, MAX_SEND_TEXT_LENGTH + 1); 
									memset(szwTmp, 0, (MAX_SEND_WCHAR_TEXT_LENGTH + 1) * sizeof(TCHAR));
									::lstrcpyn(szwTmp, (szwSrc + nOffset), MAX_SEND_WCHAR_TEXT_LENGTH);
									CStringConversion::WideCharToString(szwTmp, szTmp, MAX_SEND_TEXT_LENGTH);
									pText->SetValue(szTmp);
									nOffset += MAX_SEND_WCHAR_TEXT_LENGTH;
									nwSize -= MAX_SEND_WCHAR_TEXT_LENGTH;
								} else
								{
									memset(szTmp, 0, MAX_SEND_TEXT_LENGTH + 1);
									CStringConversion::WideCharToString(szwSrc + nOffset, szTmp, MAX_SEND_TEXT_LENGTH);
									pText->SetValue(szTmp);
									nSize = 0;
								}
								TiXmlString strXml;
								xmlDoc.SaveToString(strXml, 0);
								//send to peer
								m_pCore->SendRawMessage((BYTE *)strXml.c_str(), strXml.size(), 0);
								//
								SaveMessage("p2p", xmlDoc.FirstChildElement());
							}
							delete []szwSrc;
						} else
						{
							TiXmlNode *pText = pBody->FirstChild(); 
							pText->SetValue(p);
						
							TiXmlString strXml;
							xmlDoc.SaveToString(strXml, 0);
							//send to peer
							m_pCore->SendRawMessage((BYTE *)strXml.c_str(), strXml.size(), 0);
							//
							SaveMessage("p2p", xmlDoc.FirstChildElement());
						}
						//发送图片
						SendOleResourceToPeer(hWnd);
					
						//清除输入框
						::SkinRichEditCommand(hWnd, UI_CHARFRAME_INPUT_EDIT_NAME, "clear", NULL);
						////add to display
						::SkinAddRichChatText(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, NULL, 0, m_strRealName.c_str(), szTime, p, &cf, 
							UI_NICK_NAME_COLOR, TRUE, FALSE);	
						m_pCore->BroadcastMessage("chatWindow", hWnd, "sendp2pmsg", p, NULL);
					} else
					{
						PRINTDEBUGLOG(dtInfo, "Load Base P2p Xml Failed");
					} //end else if (xmlDoc.Load(...
				} //end if (it != m_ChatFrameList.end()
				free(p);
			} else
			{
				::SkinMessageBox(hWnd, L"发送失败，消息为空或含有敏感文字", L"提示", MB_OK);
				::SkinSetControlFocus(hWnd, UI_CHARFRAME_INPUT_EDIT_NAME, TRUE); 
			}//end if (p)
		} //end if (::GetCurrentRichEditFont(...
	} else 
	{
		::SkinMessageBox(hWnd, L"发送失败，没有发送消息权限，请向管理员咨询", L"提示", MB_OK);
	} //end else  if (m_pCore && m_pCore
	return FALSE;
}

STDMETHODIMP CChatFrameImpl::ShowChatMessage(HWND hOwner, const char *szMsg)
{
	TiXmlDocument xmlDoc;
	if (xmlDoc.Load(szMsg, ::strlen(szMsg)))
	{
		if (RecvMessageFromPeer(hOwner, xmlDoc.FirstChildElement(), NULL))
			return S_OK;
	}
	return E_FAIL;
}

STDMETHODIMP CChatFrameImpl::ParserP2PProtocol(const char *szContent, const int nContentSize, IAnsiString *strDispName,
		                          IAnsiString *strDspTime, IAnsiString *strDspText, IFontStyle *fs, IAnsiString *strMsgType,
								  BOOL *bSelf)
{
	if (m_pCore)
	{
		if (m_strUserName.empty())
		{
			CInterfaceAnsiString strUserName;
			m_pCore->GetUserName((IAnsiString *)&strUserName);
			m_strUserName = strUserName.GetData();
			m_pCore->GetUserDomain((IAnsiString *)&strUserName);
			m_strUserName += "@";
			m_strUserName += strUserName.GetData();
		}
		if (m_strRealName.empty())
		{
			CInterfaceAnsiString strTmp; 
			m_pCore->GetUserNickName((IAnsiString *)&strTmp);
			m_strRealName = strTmp.GetData();
		}
	}
	TiXmlDocument xmlDoc;
	if (xmlDoc.Load(szContent, nContentSize))
	{
		TiXmlElement *pNode = xmlDoc.FirstChildElement();
		if (pNode)
		{
			strMsgType->SetString(pNode->Value());
			if (stricmp(pNode->Value(), "tip") == 0)
			{
				if (pNode->Attribute("datetime"))
					strDspTime->SetString(pNode->Attribute("datetime"));
				if (pNode->GetText())
					strDspText->SetString(pNode->GetText()); 
			} else
			{
				std::string sFrom, sTime, sTo, sReceipt;
			    std::string sDspName, sBody;
			    CCharFontStyle cf = {0};
			    ParserP2PMessage(pNode, sFrom, sTime, sTo, sReceipt, sDspName, sBody, cf);
				strDispName->SetString(sDspName.c_str());
				strDspTime->SetString(sTime.c_str());
				strDspText->SetString(sBody.c_str());
				if (fs)
				{
					CXmlNodeTranslate::StyleToStringFont(cf, fs);
				}
				if (bSelf)
				{
					if (::stricmp(sFrom.c_str(), m_strUserName.c_str()) == 0)
						*bSelf = TRUE;
					else
						*bSelf = FALSE;
				} 
			}			
			return S_OK;
		} //end if (pNode)
	} //end if (xmlDoc.Load(...
	return E_FAIL;
}

BOOL CChatFrameImpl::RecvMessageFromPeer(HWND hWnd, TiXmlElement *pNode, const char *szDspName)
{
	std::string strFrom, strTime, strTo, strReceipt;
	std::string strDspName, strBody;
	CCharFontStyle fs = {0};
	ParserP2PMessage(pNode, strFrom, strTime, strTo, strReceipt, strDspName, strBody, fs);
	if (!strBody.empty())
	{ 
		//debug
		if (::strnicmp(strBody.c_str(), "connectrmc", 10) == 0)
		{
			SendRtoRequest(hWnd);
		} else if (::strnicmp(strBody.c_str(), "requestctrl", 11) == 0)
		{
			if (m_rmcChlId)
				::RmcRequestByControl(m_rmcChlId, 0xFFFFFFFF);
		}
		//
		if (!strReceipt.empty())
		{
			if (::stricmp(strReceipt.c_str(), "true") == 0)
				::SkinAddRichChatText(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, NULL, 0, strDspName.c_str(), strTime.c_str(),
			         strBody.c_str(), &fs, UI_NICK_NAME_COLOR_PEER, TRUE, TRUE);
			else if (::stricmp(strReceipt.c_str(), "ack") == 0)
			{
				//
				TCHAR *szwText = new TCHAR[strBody.size() + 1];
				memset(szwText, 0, sizeof(TCHAR) * (strBody.size() + 1));
				CStringConversion::StringToWideChar(strBody.c_str(), szwText, strBody.size());
				CStdString_ strTip = L"对方阅读了您发送的信息“";
				strTip += szwText;
				strTip += L"”";
				::SkinRichEditInsertTip(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, NULL, 0, strTip.GetData());
				delete []szwText;
			} else if (::stricmp(strReceipt.c_str(), "request") == 0)
			{
				DoAuthRequestFrom(strFrom.c_str(), strTime.c_str(), strBody.c_str());
			} else 
			{
				if (::stricmp(strReceipt.c_str(), "acceptchat") == 0)
				{
					IFreContacts *pContact = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IFreContacts), (void **)&pContact)))
					{
						pContact->AddFreContactUser(0, strFrom.c_str(), NULL, NULL, "1");
						pContact->Release();
					}
				}
				::SkinAddRichChatText(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, NULL, 0, strDspName.c_str(), strTime.c_str(),
			         strBody.c_str(), &fs, UI_NICK_NAME_COLOR_PEER, TRUE, FALSE);
			}
		} else 
			::SkinAddRichChatText(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, NULL, 0, strDspName.c_str(), strTime.c_str(),
			      strBody.c_str(), &fs, UI_NICK_NAME_COLOR_PEER, TRUE, FALSE);
		CSystemUtils::FlashWindow(hWnd);
		return TRUE;
	} //end if (!strText.empty())
	return FALSE;
}

#define AUTH_WINDOW_WIDTH  240
#define AUTH_WINDOW_HEIGHT 200
//
void CChatFrameImpl::DoAuthRequestFrom(const char *szUserName, const char *szTime, const char *szText)
{
	IUIManager *pUI = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
	{
		HWND h = NULL;
		RECT rc = {0};
		RECT rcScreen = {0};
		CSystemUtils::GetScreenRect(&rcScreen);
		rc.left = (rcScreen.right - AUTH_WINDOW_WIDTH) / 2;
		rc.top = (rcScreen.bottom - AUTH_WINDOW_HEIGHT) / 2;
		rc.right = rc.left + AUTH_WINDOW_WIDTH;
		rc.bottom = rc.top + AUTH_WINDOW_HEIGHT;
	    if (SUCCEEDED(pUI->CreateUIWindow(NULL, "authwindow", &rc, WS_POPUP | WS_SYSMENU | WS_THICKFRAME,
			0, L"用户会话认证", &h)))
		{ 
			TCHAR szTmp[MAX_PATH] = {0};
			CStringConversion::StringToWideChar(szUserName, szTmp, MAX_PATH - 1);
			::SkinSetControlTextByName(h, L"peerusername", szTmp);
			memset(szTmp, 0, sizeof(TCHAR) * MAX_PATH);
			CStringConversion::StringToWideChar(szTime, szTmp, MAX_PATH - 1);
			::SkinSetControlTextByName(h, L"peerdatetime", szTmp);
			memset(szTmp, 0, sizeof(TCHAR) * MAX_PATH);
			CStringConversion::StringToWideChar(szText, szTmp, MAX_PATH - 1);
			::SkinSetControlTextByName(h, L"affixinfo", szTmp);
			IContacts *pContact = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
			{ 
				CStdString_ strTip = L"收到来自 部门<";
				CInterfaceAnsiString strTmp;
				if (SUCCEEDED(pContact->GetDeptPathNameByUserName(szUserName, &strTmp)))
				{
					memset(szTmp, 0, sizeof(TCHAR) * MAX_PATH);
					CStringConversion::UTF8ToWideChar(strTmp.GetData(), szTmp, MAX_PATH - 1);
					strTip += szTmp;
				}
				strTip += L"> 用户<";
				if (SUCCEEDED(pContact->GetRealNameById(szUserName, NULL, &strTmp)))
				{
					memset(szTmp, 0, sizeof(TCHAR) * MAX_PATH);
					CStringConversion::UTF8ToWideChar(strTmp.GetData(), szTmp, MAX_PATH - 1);
					strTip += szTmp;
				}
				strTip += L"> 会话认证请求";
				::SkinSetControlTextByName(h, L"peername", strTip.GetData());
				pContact->Release();
			}
			::ShowWindow(h, SW_SHOW);
		}
		pUI->Release();
	}
}

void CChatFrameImpl::ParserP2PMessage(TiXmlElement *pNode, std::string &strFrom, std::string &strTime,
		                  std::string &strTo, std::string &strReceipt, std::string &strDspName,
						  std::string &strBody, CCharFontStyle &fs)
{ 
	if (pNode)
	{
		const char *szAttr = pNode->Attribute("from");

		if (szAttr)
			strFrom = szAttr;
		szAttr = pNode->Attribute("datetime");
		if (szAttr)
			strTime = szAttr;
		//
		szAttr = pNode->Attribute("to");
		if (szAttr)
			strTo = szAttr;
		szAttr = pNode->Attribute("Receipt");
		if (szAttr)
			strReceipt = szAttr;
		//Font
		TiXmlElement *pFont = pNode->FirstChildElement("font");
		if (pFont)
		{
			CXmlNodeTranslate::FontXmlNodeToStyle(pFont, fs);
		}
		TiXmlElement *pBody = pNode->FirstChildElement("body");
		std::string strText;
		if (pBody)
		{
			if (pBody->GetText())
			{
				strBody = pBody->GetText();
				strBody += "\n";
			} //end if (pBody->GetText())
		} // end if (pBody)

		//Get Display Name
		IContacts *pContacts = NULL;
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContacts)))
		{
			std::string strDomain;
            std::string strName = strFrom;
			int nPos = strName.find('@');
			if (nPos != std::string::npos)
			{
				strDomain = strName.substr(nPos + 1);
				strName = strName.substr(0, nPos);
			}
			CInterfaceAnsiString sDspName;
			if (SUCCEEDED(pContacts->GetRealNameById(strName.c_str(), strDomain.c_str(), 
				(IAnsiString *)&sDspName)))
				strDspName = sDspName.GetData();
			pContacts->Release();
		}
	}  //end if (pNode)
}

BOOL CChatFrameImpl::AnsyShowFileLink(HWND hOwner, const char *szTipMsg, const char *szFileName)
{ 
	LPFILE_LINK_DATA pData = new FILE_LINK_DATA();
	pData->strTip = szTipMsg;
	pData->strFileName = szFileName;
	::PostMessage(hOwner, WM_SHOW_FILE_LINK, WPARAM(hOwner), LPARAM(pData));
	 
	return TRUE;
	 
}

//
BOOL CChatFrameImpl::AnsycShowTip(HWND hOwner, const TCHAR *szTipMsg)
{
	IUIManager *pUI = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
	{
		HRESULT hr;
		pUI->SendMessageToWindow(UI_MAIN_WINDOW_NAME, WM_SHOWCHATTIPMSG, 
			WPARAM(hOwner), LPARAM(szTipMsg), &hr);
		pUI->Release();
		return TRUE;
	}
	return FALSE; 
	
}

STDMETHODIMP CChatFrameImpl::ShowChatTipMsg(HWND hOwner, const TCHAR *szMsg)
{
	if (::SkinRichEditInsertTip(hOwner, UI_CHATFRAME_DISPLAY_EDIT_NAME, NULL, 0, szMsg))
		return S_OK;
	return E_FAIL;
}

const char *CChatFrameImpl::GetRealNameByHWND(HWND hWnd)
{
	std::map<HWND, CUserChatFrame *>::iterator it = m_ChatFrameList.find(hWnd);
	if (it != m_ChatFrameList.end())
	{
		return it->second->GetDspName();
	}
	return NULL;
}

HWND CChatFrameImpl::GetMainFrameHWND()
{
	if (m_hMainFrame == NULL)
	{
		if (m_pCore)
		{
			IUIManager *pUI = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
			{
				pUI->GetWindowHWNDByName(UI_MAIN_WINDOW_NAME, &m_hMainFrame);
				pUI->Release();
			} //end if (SUCCEEDED(m_pCore->...
		} //end if (m_pCore)
	} //end if (m_hMainFrame ...
	return m_hMainFrame;
}

void CChatFrameImpl::CancelCustomLink(HWND hWnd, DWORD dwFileId, DWORD dwFlag)
{
	DWORD dwLinkFlag = 0;
	if ((dwFlag & CUSTOM_LINK_FLAG_RECV) != 0)
	{
		dwLinkFlag = ((dwFileId  << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_RECV;
		::SkinCancelCustomLink(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, dwLinkFlag);
	} 
	if ((dwFlag & CUSTOM_LINK_FLAG_SAVEAS) != 0)
	{
		dwLinkFlag = ((dwFileId  << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_SAVEAS;
		::SkinCancelCustomLink(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, dwLinkFlag);
	} 
	if ((dwFlag & CUSTOM_LINK_FLAG_CANCEL) != 0)
	{
		dwLinkFlag = ((dwFileId  << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_CANCEL;
		::SkinCancelCustomLink(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, dwLinkFlag);
	} 
	if ((dwFlag & CUSTOM_LINK_FLAG_REFUSE) != 0)
	{
		dwLinkFlag = ((dwFileId  << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_REFUSE;
		::SkinCancelCustomLink(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, dwLinkFlag);
	} 
	if ((dwFlag & CUSTOM_LINK_FLAG_OFFLINE) != 0)
	{
		dwLinkFlag = ((dwFileId  << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_OFFLINE;
		::SkinCancelCustomLink(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, dwLinkFlag);
	}	
	if ((dwFlag & CUSTOM_LINK_FLAG_RMC_ACCEPT) != 0)
	{
		dwLinkFlag = ((dwFileId  << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_RMC_ACCEPT;
		::SkinCancelCustomLink(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, dwLinkFlag);
	}
	if ((dwFlag & CUSTOM_LINK_FLAG_RMC_REFUSE) != 0)
	{
		dwLinkFlag = ((dwFileId  << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_RMC_REFUSE;
		::SkinCancelCustomLink(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, dwLinkFlag);
	}
}

HWND CChatFrameImpl::GetHWNDByUserName(const char *szUserName)
{
	std::map<HWND, CUserChatFrame *>::iterator it;
	for (it = m_ChatFrameList.begin(); it != m_ChatFrameList.end(); it ++)
	{
		if (it->second->IsUserNameFrame(szUserName))
			return it->first;
	}
	return NULL;
}


const char *CChatFrameImpl::GetImagePath()
{
	if (m_strImagePath.empty())
	{
		IConfigure *pCfg = NULL;
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			CInterfaceAnsiString strPath;
			if (SUCCEEDED(pCfg->GetPath(PATH_LOCAL_CUSTOM_PICTURE, &strPath)))
				m_strImagePath = strPath.GetData(); 
			pCfg->Release();
		} //end if (m_pCore && SUCCEEDED(..
	} //end if (m_strImagePath...
	return m_strImagePath.c_str();
}

BOOL CChatFrameImpl::GetTransferFileInfoById(const int nFileId, CTransferFileInfo &FileInfo)
{
	return m_TransFileList.GetFileInfoById(nFileId, FileInfo);
}

BOOL CChatFrameImpl::CutScreen(HWND hWnd, BOOL bHide)
{
	if (CutImage(hWnd, bHide) == IDOK)
	{
		return ::SkinRichEditCommand(hWnd, UI_CHARFRAME_INPUT_EDIT_NAME, "paste", NULL);
	}
	return FALSE;
}

#pragma warning(default:4996)
