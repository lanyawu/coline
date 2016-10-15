#include "widgetboximpl.h"
#include <Commonlib/DebugLog.h>
#include <Commonlib/systemutils.h>
#include <SmartSkin/smartskin.h>
#include <xml/tinyxml.h>
#include <Core/common.h>
#define WM_DOCK_PARSER_PROTOCOL  (WM_USER + 0x300)

#include "../IMCommonLib/InterfaceAnsiString.h"
#include "../IMCommonLib/InterfaceFontStyle.h"

#pragma warning(disable:4996)

static char WIDGETBOX_CONTROL_XML[] = "<Control xsi:type=\"ImageButton\" image=\"6100\" name=\"btnwidget\"/>"; 

#define WIDGETBOX_HEIGHT 200

BOOL CALLBACK RichEditCallBack(HWND hWnd, DWORD dwEvent, char *szFileName, DWORD *dwFileNameSize,
			  char *szFileFlag, DWORD *dwFileFlagSize, DWORD dwFlag, void *pOverlapped)
{
	if (pOverlapped)
	{
		CWidgetBoxImpl *pThis = (CWidgetBoxImpl *) pOverlapped;
		return pThis->RECallBack(hWnd, dwEvent, szFileName, dwFileNameSize, szFileFlag, dwFileFlagSize, dwFlag);
	}
	return FALSE;
}

CWidgetBoxImpl::CWidgetBoxImpl(void):
                m_pCore(NULL),
				m_bDocked(FALSE),
				m_bShow2Dock(FALSE),
				m_hWnd(NULL)
{
}


CWidgetBoxImpl::~CWidgetBoxImpl(void)
{
	if (m_hWnd)
	{
		::SkinCloseWindow(m_hWnd);
		m_hWnd = NULL;
	}
	if (m_pCore)
		m_pCore->Release();
	m_pCore = NULL;
}

//IUnknown
STDMETHODIMP CWidgetBoxImpl::QueryInterface(REFIID riid, LPVOID *ppv)

{
	*ppv = NULL;
	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IWidgetBox)))
	{
		*ppv = (IWidgetBox *) this;
		_AddRef();
		return S_OK;
	} else if (IsEqualIID(riid, __uuidof(ICoreEvent)))
	{
		*ppv = (ICoreEvent *) this;
		_AddRef();
		return S_OK;
	}  
	return E_NOINTERFACE;
}

HRESULT CWidgetBoxImpl::DoClickEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "btnwidget") == 0)
	{
		if ((m_hWnd != NULL) && (::IsWindowVisible(m_hWnd)))
		{
			::ShowWindow(m_hWnd, SW_HIDE);
		} else
		{
			ShowWidgetBox(hWnd);
		}
		m_bDocked = FALSE;
	} else if (::stricmp(szName, "closebutton") == 0)
	{
		if ((hWnd != NULL) && (m_hWnd == hWnd))
		{
			::ShowWindow(m_hWnd, SW_HIDE);
			m_bDocked = FALSE;
			return 0;
		}
	} else  if (::stricmp(szName, "dockbutton") == 0)
	{
		if ((hWnd != NULL) && (m_hWnd == hWnd))
		{
			//::ShowWindow(m_hWnd, SW_HIDE);
			RECT rcScreen = {0};
			CSystemUtils::GetScreenRect(&rcScreen);
			RECT rc = {0};
			::GetClientRect(m_hWnd, &rc);
			int w = rc.right - rc.left;
			::MoveWindow(m_hWnd, rcScreen.right - w, rc.top, w, rcScreen.bottom - rcScreen.top, TRUE);
			::SkinSetControlVisible(m_hWnd, L"tipinfo", TRUE);
			::SkinSetDockDesktop(m_hWnd, TRUE, RGB(255, 255, 255), 220);
			m_bDocked = TRUE;
			return 0;
		}
	} else if (::stricmp(szName, "ok") == 0)
	{
		TCHAR szwTmp[MAX_PATH] = {0};
		int nSize = MAX_PATH - 1;
		if (::SkinGetControlTextByName(hWnd, L"edtnewtabname", szwTmp, &nSize))
		{
			char szUTF8Name[MAX_PATH] = {0};
			CStringConversion::WideCharToUTF8(szwTmp, szUTF8Name, MAX_PATH - 1);
			char szGuid[128] = {0};
			nSize = 127;
			CSystemUtils::GetGuidString(szGuid, &nSize);
			IConfigure *pCfg = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
			{
				pCfg->AddWidgetTab(szGuid, szUTF8Name);
				pCfg->Release();
			} //end if (SUCCEEDED(
			AddWidgetTab(m_hWnd, szGuid, szUTF8Name);
			::SkinCloseWindow(hWnd);
		} else
		{
			::SkinMessageBox(hWnd, L"标签名称不能为空", L"提示", MB_OK);
		}//end if (::SkinGetControl
	} else
	{
		std::map<std::string, CStdString_>::iterator it = m_Plugins.find(szName);
		if (it != m_Plugins.end())
		{
			if (::_tcsicmp(it->second, L"coline://") == 0)
			{
				IMainFrame *pFrame = NULL;
				if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMainFrame), (void **)&pFrame)))
				{
					pFrame->BringToFront();
					pFrame->Release();
				}
			} else
				::ShellExecute(NULL, L"open", it->second, NULL, NULL, SW_SHOW);
		} //end if (it != m_Plugins.end())
	}
	return -1;
}

void CWidgetBoxImpl::RefreshShow2Dock()
{
	m_bShow2Dock = FALSE;
	IConfigure *pCfg = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		CInterfaceAnsiString strTmp;
		if (SUCCEEDED(pCfg->GetParamValue(FALSE, "person", "show2dock", &strTmp)))
		{
			if (::stricmp(strTmp.GetData(), "true") == 0)
				m_bShow2Dock = TRUE;
		}
		pCfg->Release();
	}
}
 
static const char WIDGET_TAB_UI_XML[] = "<Container xsi:type=\"ImageTabPage\" image=\"35\" text=\"%s\" border=\"false\" background=\"#FF00FF\"><Container xsi:type=\"VerticalLayout\" border=\"false\" background=\"#FF00FF\"><Container xsi:type=\"AutoShortCut\" ButtonPad=\"80 100\" name=\"%s\" border=\"false\" background=\"#FF00FF\" scrollbar=\"vertical\"></Container></Container></Container>";
BOOL CWidgetBoxImpl::AddWidgetTab(HWND hWnd, const char *szTabId, const char *szUTF8TabName)
{
	//return TRUE;
	char szTmpUI[4096] = {0}; 
	sprintf(szTmpUI, WIDGET_TAB_UI_XML, szUTF8TabName, szTabId);
	return ::SkinAddChildControl(hWnd, L"widgettab", szTmpUI, NULL, NULL, 9999); 
	return TRUE;
}

void CWidgetBoxImpl::InitWidget(HWND hWnd)
{
	IConfigure *pCfg = NULL;
	//add default
	AddWidgetToUI(hWnd, "oftenusing", "统一通信平台", "coline://", "显示统一通信平台主界面", 25);
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		CInterfaceAnsiString strXml;
		//create widget tab
		//<widgets><tab id="016FD35A-4DA2-44F8-86EA-C845DDD5F953" name="标签1"/><tab.../></widgets>
		if (SUCCEEDED(pCfg->GetWidgetTabs(&strXml)))
		{
			TiXmlDocument xmldoc;
			if (xmldoc.Load(strXml.GetData(), strXml.GetSize()))
			{
				TiXmlElement *pWidget = xmldoc.FirstChildElement("widgets");
				if (pWidget)
				{
					TiXmlElement *pTab = pWidget->FirstChildElement("tab");
					while (pTab)
					{
						if ((pTab->Attribute("id") != NULL) && (pTab->Attribute("name") != NULL))
						{
							AddWidgetTab(hWnd, pTab->Attribute("id"), pTab->Attribute("name"));
						}
						pTab = pTab->NextSiblingElement("tab");
					} //end while (pTab)
				} //end if (pWidget)
			} //end if (xmldoc.Load(
		} //end if (SUCCEEDED(pCfg->..

		//load widget
		//<widgets><item tabid="016FD35A-4DA2-44F8-86EA-C845DDD5F953" id="id1" name="GoCom"
        //  caption="统一通信平台" url="gocom://" imageid="25" tip="统一通信平台"/><item... /></widgets>
		if (SUCCEEDED(pCfg->GetWidgetItems(&strXml)))
		{
			TiXmlDocument xmldoc;
			if (xmldoc.Load(strXml.GetData(), strXml.GetSize()))
			{
				TiXmlElement *pWidget = xmldoc.FirstChildElement("widgets");
				if (pWidget)
				{ 
					TiXmlElement *pItem = pWidget->FirstChildElement("item");
					while (pItem)
					{ 
						AddWidgetToUI(hWnd, pItem->Attribute("tabid"), pItem->Attribute("caption"),
							pItem->Attribute("url"), pItem->Attribute("tip"), ::atoi(pItem->Attribute("imageid"))); 
						//add widget...
						pItem = pItem->NextSiblingElement("item");
					} //end while (pItem)
				} //end if (pWidget)
			} //end if (xmldoc.Load(..
		} //end if (SUCCEEDED(..
		pCfg->Release();
	} //end if (SUCCEEDED(
}

BOOL CWidgetBoxImpl::AddWidgetToUI(HWND hWnd, const char *szTabId, const char *szCaption, const char *szUrl, 
		const char *szTip, const int nImageId)
{
	if (szUrl == NULL)
		return FALSE;
	TCHAR szwTabId[MAX_PATH] = {0}, szwCaption[MAX_PATH] = {0}, szwUrl[MAX_PATH] = {0};
	TCHAR szwTip[MAX_PATH] = {0}, szwFlag[MAX_PATH] = {0};

	if (szTabId)
		CStringConversion::StringToWideChar(szTabId, szwTabId, MAX_PATH - 1);
	if (szCaption)
		CStringConversion::StringToWideChar(szCaption, szwCaption, MAX_PATH - 1);
	if (szUrl)
		CStringConversion::StringToWideChar(szUrl, szwUrl, MAX_PATH - 1);
	if (szTip)
		CStringConversion::StringToWideChar(szTip, szwTip, MAX_PATH - 1);
 
	if (::SkinAddAutoShortCut(hWnd, szwTabId, szwCaption, szwUrl, nImageId, szwTip, szwFlag))
	{
		char szFlag[MAX_PATH] = {0};
		CStringConversion::WideCharToString(szwFlag, szFlag, 127);
		m_Plugins[szFlag] = szwUrl;
		return TRUE;
	}
	return FALSE;
}

void CWidgetBoxImpl::ShowWidgetBox(HWND hParent)
{
	if ((m_hWnd == NULL) || (!::IsWindow(m_hWnd)))
	{
		if (m_hWnd)
			::SkinCloseWindow(m_hWnd);
		m_hWnd = NULL;
		IUIManager *pUI = NULL;
		RECT rc = {2, 2, 10, 10};
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
		{
			if (SUCCEEDED(pUI->CreateUIWindow(NULL, "widgetbox", &rc, WS_POPUP | WS_MINIMIZEBOX, WS_EX_ACCEPTFILES | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
					L"CoLine工具箱", &m_hWnd)))
			{
				//设置回调
				::SkinSetRichEditCallBack(m_hWnd, L"messagedisplay", RichEditCallBack, this); 
				pUI->OrderWindowMessage("widgetbox", m_hWnd, WM_DROPFILES, (ICoreEvent *) this);
				pUI->OrderWindowMessage("widgetbox", m_hWnd, WM_DOCK_PARSER_PROTOCOL, (ICoreEvent *) this);
				InitWidget(m_hWnd);
				RefreshShow2Dock();
			} else
			{
				//
				PRINTDEBUGLOG(dtInfo, "Create WidgetBox Failed");
			}
			pUI->Release();
		}
	}
	if (m_hWnd && ::IsWindow(m_hWnd))
	{
		RECT rc = {0};
		::GetWindowRect(hParent, &rc);
		POINT pt = {0};
		::GetCursorPos(&pt);
		rc.bottom = pt.y;
		rc.top = rc.bottom - WIDGETBOX_HEIGHT - 15;
		::InflateRect(&rc, -15, -15);
		::MoveWindow(m_hWnd, rc.left, rc.top, rc.right - rc.left, WIDGETBOX_HEIGHT, TRUE);
		::ShowWindow(m_hWnd, SW_SHOW);
		::SkinSetControlVisible(m_hWnd, L"tipinfo", FALSE); 
		::SkinSetDockDesktop(m_hWnd, FALSE, 0, 0);
		::InvalidateRect(m_hWnd, &rc, TRUE);
		//CSystemUtils::BringToFront(m_hWnd);
	}
}

//ICoreEvent
STDMETHODIMP CWidgetBoxImpl::DoCoreEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
                             LPARAM lParam, HRESULT *hResult)
{	 
	if (::stricmp(szType, "afterinit") == 0)
	{
		if (::stricmp(szName, "mainwindow") == 0)
		{
			::SkinAddChildControl(hWnd, L"mainFooter", WIDGETBOX_CONTROL_XML, NULL, NULL, 1);
		}
	} else if (::stricmp(szType, "click") == 0)
	{
		 *hResult = DoClickEvent(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "killfocus") == 0)
	{
		if ((hWnd == m_hWnd) && ::stricmp(szName, "searchedit") == 0)
		{
			IContacts *pContact = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
			{
				pContact->HideHelpEditWindow();
				pContact->Release();
			}
		}
	} else if (::stricmp(szType, "editchanged") == 0)
	{
		if ((hWnd == m_hWnd) && ::stricmp(szName, "searchedit") == 0)
		{
			TCHAR szwText[64] = {0};
			int nSize = 63;
			if (::SkinGetControlTextByName(hWnd, L"searchedit", szwText, &nSize))
			{
				char szText[64] = {0};
				CStringConversion::WideCharToString(szwText, szText, 63);
				char szDest[64] = {0};
				CStringConversion::Trim(szText, szDest);
				RECT rc = {0};
				::SkinGetControlRect(hWnd, L"searchedit", &rc);
				POINT pt = {rc.left, rc.bottom};
				::ClientToScreen(hWnd, &pt);
				IContacts *pContact = NULL;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
				{
					pContact->ShowHelpEditWindow((ICoreEvent *) this, szDest, pt.x, pt.y, rc.right - rc.left, 100);
					pContact->Release();
				}
			} //end if (::
		} //end 
	}  else if (::stricmp(szType, "keydown") == 0)
	{
		if ((hWnd == m_hWnd) && ::stricmp(szName, "searchedit") == 0)
		{
			IContacts *pContact = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
			{
				switch(wParam)
				{
					case VK_RETURN: 
						{
						TCHAR szTmp[128] = {0};
						int nSize = 127;				
						char szRealName[128] = {0};
						if (::SkinGetControlTextByName(m_hWnd, L"searchedit", szTmp, &nSize))
						{
							CStringConversion::WideCharToString(szTmp, szRealName, 127);
						} 
						pContact->EditHelpSearchActive(hWnd, szRealName, TRUE, FALSE);
						break;
					    } 
					case VK_ESCAPE:
					{
						pContact->EditHelpSearchActive(hWnd, NULL, FALSE, FALSE);
						break;
					}
					case VK_UP:
					case VK_DOWN:
					case VK_PRIOR:
					case VK_NEXT:
					case VK_HOME:
					case VK_END:
					{
						pContact->EditVirtualKeyUp(wParam);
						*hResult = 0;
						break;
					}
				} //end switch(..
				pContact->Release();
			} 
		} //end if (::stricmp(szName...
	} else if (::stricmp(szType, "itemactivate") == 0)
	{
		if (::stricmp(szName, "resultlist") == 0)
		{
			::SkinSetControlTextByName(m_hWnd, L"searchedit", (TCHAR *)wParam);
			IChatFrame *pChat = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pChat)))
			{
				pChat->ShowChatFrame(m_hWnd, (char *)lParam, NULL);
				pChat->Release();
			}
		}
	} else if (::stricmp(szType, "menucommand") == 0)
	{
		*hResult = DoMenuCommand(hWnd, szName, wParam, lParam);
	}
	return E_NOTIMPL;
}

BOOL CWidgetBoxImpl::AddNewTab(HWND hWnd)
{
//
	IUIManager *pUI = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
	{
		HWND hTmp = NULL;
		RECT rcScreen = {0};
		CSystemUtils::GetScreenRect(&rcScreen);

		RECT rc = {0};
		rc.left = (rcScreen.right - 260) / 2;
		rc.top = (rcScreen.bottom - 120) / 2;
		rc.right = rc.left + 260;
		rc.bottom = rc.top + 120;
		pUI->CreateUIWindow(hWnd, "moditabnamewindow",  &rc, WS_POPUP | WS_SYSMENU,
	                0, L"输入新的标签名称", &hTmp);	 
		pUI->Release();
		if (hTmp != NULL)
		{ 
			::SkinShowModal(hTmp); 
		} //end if (hTmp != NULL)
		return TRUE;
	} //end if (m_pCore && SUCCEEDED(
	return FALSE;
}

HRESULT CWidgetBoxImpl::DoMenuCommand(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "tabpopmenu") == 0)
	{
		switch(wParam)
		{
		case 1: //创建标签
			AddNewTab(hWnd);
			break;
		case 2: //删除标签
			break;
		}
	}
	return -1;
}

//广播消息
STDMETHODIMP CWidgetBoxImpl::DoBroadcastMessage(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData)
{
	return E_NOTIMPL;
}

STDMETHODIMP CWidgetBoxImpl::SetCoreFrameWork(ICoreFrameWork *pCore)
{
	if (m_pCore)
		m_pCore->Release();
	m_pCore = pCore;
	if (m_pCore)
	{
		m_pCore->AddRef();
		//
		
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "mainwindow", "afterinit");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "btnwidget", "click");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "widgetbox", NULL, NULL);
		m_pCore->AddOrderEvent((ICoreEvent *) this, "moditabnamewindow", NULL, NULL);
		//
		m_pCore->AddOrderProtocol((IProtocolParser *) this, "msg", "p2p");
	}
	return S_OK;
}

STDMETHODIMP CWidgetBoxImpl::GetSkinXmlString(IAnsiString *szXmlString)
{
	IConfigure *pCfg = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		HRESULT hr = pCfg->GetSkinXml("WidgetBox.xml", szXmlString);
		pCfg->Release();
		return hr;
	}
	return E_FAIL;
}

//
STDMETHODIMP CWidgetBoxImpl::CoreFrameWorkError(int nErrorNo, const char *szErrorMsg)
{
	return E_NOTIMPL;
}

 void CWidgetBoxImpl::OnWMDropFiles(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HDROP hDrop = (HDROP)wParam;
	if (hDrop == NULL)
		return ;

	TCHAR szFile[MAX_PATH];  
	int iFileCount = ::DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	if (iFileCount <= 5)
	{
		for (int i = 0; i < iFileCount; ++i)
		{
			memset(szFile, 0, MAX_PATH * sizeof(TCHAR));
			if (::DragQueryFile(hDrop, i, szFile, MAX_PATH))
			{
				TCHAR szwTmp[MAX_PATH] = {0}; 
				TCHAR szSelName[MAX_PATH] = {0};
				int nSize = MAX_PATH - 1;
				char szFlag[128] = {0};
				int nFlagSize = 127;
				if (::SkinTabGetChildByClass(m_hWnd, L"widgettab", L"AutoShortCut", szSelName, &nSize))
				{ 
					char szSelTabId[MAX_PATH] = {0};
					char szSelFileName[MAX_PATH] = {0};
					char szTip[MAX_PATH] = {0};
					char szFileName[MAX_PATH] = {0};
					CStringConversion::WideCharToString(szSelName, szSelTabId, MAX_PATH - 1);
					CStringConversion::WideCharToString(szFile, szSelFileName, MAX_PATH - 1);
					CStringConversion::WideCharToString(szFile, szTip, MAX_PATH - 1);
					CSystemUtils::ExtractFileName(szSelFileName, szFileName, MAX_PATH - 1);					
					IConfigure *pCfg = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
					{
						if (SUCCEEDED(pCfg->AddWidgetItem(szSelTabId, szFileName, 
							szFileName, szSelFileName, 0, szTip)))
							AddWidgetToUI(m_hWnd, szSelTabId, szFileName, szSelFileName, szTip, 0); 
						pCfg->Release();
					}
				} //end if (::SkinTabGetSelItemName(..
			} //end if (::DragQueryFile(..
		} //end for (int i
	} else
	{
		::SkinMessageBox(hWnd, L"本应用程序最多只支持同时拖曳5个文件", L"提示", MB_OK); 
	}
	::DragFinish(hDrop);
}

void CWidgetBoxImpl::ParserProtocol(char *pData, LONG lSize)
{
	IChatFrame *pFrame = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pFrame)))
	{
		CInterfaceAnsiString strDispName, strDspTime, strDspText, strMsgType;
		BOOL bSelf;
		CInterfaceFontStyle fs;
		CCharFontStyle cf;
		CInterfaceAnsiString strFontName;
		int nClr;
		if (SUCCEEDED(pFrame->ParserP2PProtocol(pData, lSize, &strDispName, &strDspTime, &strDspText, 
			&fs, &strMsgType, &bSelf)))
		{
			if (::stricmp(strMsgType.GetData(), "tip") == 0)
			{
				TCHAR szwTime[64] = {0};
				CStdString_ strTip;
				if (strDspTime.GetSize() > 0)
				{
					CStringConversion::StringToWideChar(strDspTime.GetData(), szwTime, 63);
					strTip += szwTime;
					strTip += L"\n";
				}
				int nLen = strDspText.GetSize();
				TCHAR *szwText = new TCHAR[nLen + 1];
				CStringConversion::StringToWideChar(strDspText.GetData(), szwText, nLen);
				strTip += szwText;
				::SkinRichEditInsertTip(m_hWnd, L"messagedisplay", NULL, 0, strTip);
				delete []szwText;
			} else
			{
				if (bSelf)
					nClr = UI_NICK_NAME_COLOR;
				else
					nClr = UI_NICK_NAME_COLOR_PEER;
				cf.cfColor = fs.GetColor();
				cf.nFontSize = fs.GetSize();
				fs.GetName((IAnsiString *)&strFontName);
				CStringConversion::StringToWideChar(strFontName.GetData(), cf.szFaceName, 31);
				cf.nFontStyle = 0;
				if (fs.GetBold())
					cf.nFontStyle |= CFE_BOLD;
				if (fs.GetItalic())
					cf.nFontStyle |= CFE_ITALIC;
				if (fs.GetStrikeout())
					cf.nFontStyle |= CFE_STRIKEOUT;
				if (fs.GetUnderline())
					cf.nFontStyle |= CFE_UNDERLINE;
		 
				::SkinAddRichChatText(m_hWnd, L"messagedisplay", NULL, 0, strDispName.GetData(), strDspTime.GetData(), 
					        strDspText.GetData(), &cf,	nClr, TRUE, FALSE);					 
			} //end else
		} //end if (SUCCEEDED(
		pFrame->Release();
	} //end if (SUCCEEDED(m_pCore->
}

//IProtocolParser
STDMETHODIMP CWidgetBoxImpl::DoRecvProtocol(const BYTE *pData, const LONG lSize)
{
	if (m_bDocked && m_bShow2Dock && (m_hWnd != NULL) && (::IsWindow(m_hWnd)))
	{
		::SendMessage(m_hWnd, WM_DOCK_PARSER_PROTOCOL, (WPARAM)pData, (LPARAM)lSize);
	    return S_OK;
	}
	return E_FAIL;
}

STDMETHODIMP CWidgetBoxImpl::DoPresenceChange(const char *szUserName, const char *szNewPresence,
	                         const char *szMemo, BOOL bOrder)
{
	return E_NOTIMPL;
}

//
STDMETHODIMP CWidgetBoxImpl::DoWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes)
{
	if (hWnd == m_hWnd) 
	{
		switch(uMsg)
		{
		case WM_DROPFILES: 
			OnWMDropFiles(hWnd, wParam, lParam);
			break;
		case WM_DOCK_PARSER_PROTOCOL:
			ParserProtocol((char *)wParam, (LONG)lParam);
			break;
		}
	} 
	return E_NOTIMPL;
}

const char *CWidgetBoxImpl::GetImagePath()
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

BOOL CWidgetBoxImpl::RECallBack(HWND hWnd, DWORD dwEvent, char *szFileName, DWORD *dwFileNameSize,
			  char *szFileFlag, DWORD *dwFileFlagSize, DWORD dwFlag)
{
	switch(dwEvent)
	{
		case RICHEDIT_EVENT_SENDFILE: 
			 break;
		case RICHEDIT_EVENT_GETFILEBYTAG:
			 sprintf(szFileName, "%s%s.gif", GetImagePath(), szFileFlag);
			 return TRUE;
		case RICHEDIT_EVENT_GETCUSTOMPIC:
			{
				IEmotionFrame *pFrame = NULL;
				BOOL bSucc = FALSE;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IEmotionFrame), (void **)&pFrame)))
				{
					CInterfaceAnsiString strFileName;
					if (SUCCEEDED(pFrame->GetSysEmotion(szFileFlag, &strFileName)))
					{
						strcpy(szFileName, strFileName.GetData());
						bSucc = TRUE;
					} else
					{ 
						sprintf(szFileName, "%s%s.gif", GetImagePath(), szFileFlag);
						if (!CSystemUtils::FileIsExists(szFileName))
						{
							if (SUCCEEDED(pFrame->GetDefaultEmotion("sending", &strFileName)))
								strcpy(szFileName, strFileName.GetData());
						} //end if (!CSystemUtils::					  
					} //end else if (
					pFrame->Release();
				}
				
				return TRUE;
			}
		case RICHEDIT_EVENT_GETTIPPIC:
			break;
		case RICHEDIT_EVENT_CUSTOMLINKCLICK: 
			break;
	}
	return FALSE;
}

#pragma warning(default:4996)
