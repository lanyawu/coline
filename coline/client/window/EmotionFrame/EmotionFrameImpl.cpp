#include <Commonlib/DebugLog.h>
#include <Commonlib/systemutils.h>
#include <crypto/crypto.h>
#include "../IMCommonLib/InterfaceAnsiString.h"
#include "EmotionFrameImpl.h"
#include <Core/common.h>

#define EMOTION_FRAME_WIDTH  220
#define EMOTION_FRAME_HEIGHT 150

#define EMOTION_DEFAULT_FILENAME "sysemotion\\emotion.xml"
#define CUSTOM_EMOTION_FILENAME  "cusemotion.xml"

#pragma warning(disable:4996)

CEmotionFrameImpl::CEmotionFrameImpl(void):
                   m_pCore(NULL),
				   m_hWnd(NULL),
				   m_SysEmotionDoc(NULL),
				   m_CusEmotionDoc(NULL),
				   m_nSysPage(-1),
				   m_bSysTab(TRUE),
				   m_nCustomPage(-1),
				   m_pOwnerEvent(NULL)
{
}


CEmotionFrameImpl::~CEmotionFrameImpl(void)
{
	if (m_hWnd)
	{
		::SkinCloseWindow(m_hWnd);
		m_hWnd = NULL;
	}
	if (m_pCore)
		m_pCore->Release();
	m_pCore = NULL;
	if (m_pOwnerEvent)
		m_pOwnerEvent->Release();
	m_pOwnerEvent = NULL;
	if (m_SysEmotionDoc)
		delete m_SysEmotionDoc;
	if (m_CusEmotionDoc)
		delete m_CusEmotionDoc;
}

//IUnknown
STDMETHODIMP CEmotionFrameImpl::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IEmotionFrame)))
	{
		*ppv = (IEmotionFrame *) this;
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

//ICoreEvent
STDMETHODIMP CEmotionFrameImpl::DoCoreEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
	                     LPARAM lParam, HRESULT *hResult)
{
	if (::stricmp(szType, "itemclick") ==  0)
	{
		if (::stricmp(szName, "emotionpanel") == 0)
		{
			if (m_pOwnerEvent)
			{
				char szFileName[MAX_PATH] = {0};
				char szTag[64] = {0};
				int nNameSize = MAX_PATH - 1;
				int nTagSize = 63;
				if (::SkinGetSelEmotion(hWnd, L"emotionpanel", szFileName, &nNameSize, szTag, &nTagSize))
					m_pOwnerEvent->DoCoreEvent(m_pOwnerWnd, "click", szName, WPARAM(szFileName), LPARAM(szTag), hResult);
			} //end if (m_pOwnerEvent)
			::ShowWindow(hWnd, SW_HIDE);
		} else if (stricmp(szName, "customemotionpanel") == 0)
		{
			if (m_pOwnerEvent)
			{
				char szFileName[MAX_PATH] = {0};
				char szTag[64] = {0};
				int nNameSize = MAX_PATH - 1;
				int nTagSize = 63;
				if (::SkinGetSelEmotion(hWnd, L"customemotionpanel", szFileName, &nNameSize, szTag, &nTagSize))
					m_pOwnerEvent->DoCoreEvent(m_pOwnerWnd, "click", "emotionpanel", WPARAM(szFileName), LPARAM(szTag), hResult);
			}
			::ShowWindow(hWnd, SW_HIDE);
		}//end if (::stricmp(szName, "")
	} else if (::stricmp(szType, "itemselect") == 0)
	{
		if (::stricmp(szName, "mainfolder") == 0)
		{
			if (wParam == 0)
			{ 
				m_bSysTab = TRUE;
				RefrehPageTip();
				::SkinSetControlVisible(m_hWnd, L"addemotion", FALSE);
			} else
			{ 
				m_bSysTab = FALSE;
				RefrehPageTip();
				::SkinSetControlVisible(m_hWnd, L"addemotion", TRUE);
			}
		}
	} else if (::stricmp(szType, "link") == 0)
	{
		if (::stricmp(szName, "addemotion") == 0)
		{
			AddCustomEmotionEvent(hWnd, NULL);
		} else if (::stricmp(szName, "prepage") == 0)
		{
			int nPage = -1;
			if (m_bSysTab)
			{
				nPage = m_nSysPage;
			} else
				nPage = m_nCustomPage;
			nPage --;
			DisplayEmotion(nPage, m_bSysTab);
		} else if (::stricmp(szName, "nextpage") == 0)
		{
			int nPage = -1;
			if (m_bSysTab)
			{
				nPage = m_nSysPage;
			} else
				nPage = m_nCustomPage;
			nPage ++;
			DisplayEmotion(nPage, m_bSysTab);
		}
	} else if (::stricmp(szType, "click") == 0)
	{
		if (::stricmp(szName, "ok") == 0)
		{
			if (AddEmotion(hWnd))
				::SkinCloseWindow(hWnd);
		} else if (::stricmp(szName, "browserpic") == 0)
		{
			BrowserPicture(hWnd);
		} else if (::stricmp(szName, "cancel") == 0)
		{
			::SkinCloseWindow(hWnd);
		}
	} else if (::stricmp(szType, "layout") == 0) 
	{
		if (::stricmp(szName, "emotionpanel") == 0)
		{
			m_bSysTab = TRUE;
			DisplayEmotion(-1, m_bSysTab);
		} else if (::stricmp(szName, "customemotionpanel") == 0)
		{
			m_bSysTab = FALSE;
			DisplayEmotion(-1, m_bSysTab);
		}
	} else if (::stricmp(szType, "menucommand") == 0)
	{
		*hResult = DoMenuCommand(hWnd, szName, wParam, lParam);
	}
	//end if (::stricmp(szType...
	return E_FAIL;
}

//
HRESULT CEmotionFrameImpl::DoMenuCommand(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (wParam == 100) //添加到自定义表情
	{
		char szFileName[MAX_PATH] = {0};
		int nSize = MAX_PATH - 1;
		if (::SkinGetRESelectImageFile(hWnd, L"messagedisplay", szFileName, &nSize))
		{
			AddCustomEmotionEvent(hWnd, szFileName);
		}
	}
	return -1;
}

//
void CEmotionFrameImpl::BrowserPicture(HWND hWnd)
{
	CStringList_ FileList;
 	if (CSystemUtils::OpenFileDialog(NULL, hWnd, "选择要添加的表情文件", "所有图片文件(*.*)|*.bmp;*.gif;*.jpg;*.png", 
		                NULL, FileList, FALSE, FALSE))
	{
		std::string strFileName;
		if (!FileList.empty())
			strFileName = FileList.back();
		if ((!strFileName.empty()) && CSystemUtils::FileIsExists(strFileName.c_str()))
		{
			TCHAR szTmp[MAX_PATH] = {0};
			CStringConversion::StringToWideChar(strFileName.c_str(), szTmp, MAX_PATH - 1);
			::SkinSetControlTextByName(hWnd, L"picfilename", szTmp);  
			::SkinSetControlAttr(hWnd, L"gifpreview", L"image", szTmp);
		} //end if ((!strFileName.empty()) ...
	} //end if (CSystemUtils::OpenFileDialog(... 
}

inline BOOL MapUIToValue(HWND hWnd, const TCHAR *szUIName, char *szValue)
{
	TCHAR szTmp[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	if (::SkinGetControlTextByName(hWnd, szUIName, szTmp, &nSize))
	{
		CStringConversion::WideCharToString(szTmp, szValue, MAX_PATH - 1);
		return TRUE;
	}
	return FALSE;
}

 
BOOL CEmotionFrameImpl::AddEmotion2Xml(const char *szFileName, const char *szMd5, 
	const char *szShortCut, const char *szComment)
{
	if (m_CusEmotionDoc)
	{ 
		TiXmlElement *pList = m_CusEmotionDoc->FirstChildElement("EmotionList"); 
		if (pList)
		{
			TiXmlElement *pSys = pList->FirstChildElement("SystemEmotionList");
			if (pSys)
			{
				char szTmp[MAX_PATH] = {0};
				CSystemUtils::ExtractFileName(szFileName, szTmp, MAX_PATH - 1);
				TiXmlElement Child("Emotion");
				Child.SetAttribute("tag", szMd5);
				Child.SetAttribute("imagefile", szTmp);
				Child.SetAttribute("shortcut", szShortCut);
				memset(szTmp, 0, MAX_PATH);
				CStringConversion::StringToUTF8(szComment, szTmp, MAX_PATH - 1);
				Child.SetAttribute("comment", szTmp);
				pSys->InsertEndChild(Child);
				return m_CusEmotionDoc->SaveFile();
			} //end if (pSys)
		} //end (pSys)
	}
	return FALSE;
}

//加入到自定义文件中
BOOL CEmotionFrameImpl::AddEmotion(HWND hWnd)
{
	std::string strEmotionFileName; 
	char szShortCut[MAX_PATH] = {0};
	char szComment[MAX_PATH] = {0};
	char szSrcFile[MAX_PATH] = {0};
	MapUIToValue(hWnd, L"picfilename", szSrcFile);
	MapUIToValue(hWnd, L"shortcut", szShortCut);
	MapUIToValue(hWnd, L"emotionname", szComment);
	if (CSystemUtils::FileIsExists(szSrcFile))
	{
		char szMd5[64] = {0};
		char szExt[MAX_PATH] = {0};
		CSystemUtils::ExtractFileExtName(szSrcFile, szExt, MAX_PATH - 1);
		if (md5_encodefile(szSrcFile, szMd5))
		{
			strEmotionFileName = m_strCusEmotionPath;
			strEmotionFileName += szMd5;
			strEmotionFileName += ".";
			strEmotionFileName += szExt;
			CInterfaceAnsiString strTmp;
			if ((!CSystemUtils::FileIsExists(strEmotionFileName.c_str()))
				&& FAILED(GetSysEmotion(szMd5, &strTmp)))
			{
				CSystemUtils::CopyFilePlus(szSrcFile, strEmotionFileName.c_str(), TRUE); 
				if (AddEmotion2Xml(strEmotionFileName.c_str(), szMd5, szShortCut, szComment))
				{
					m_CusEmotionTables.insert(std::pair<CAnsiString_, std::string>(szMd5, strEmotionFileName));
					m_nTotalCustomCount ++;
					::SkinAddEmotion(m_hWnd, L"customemotionpanel", strEmotionFileName.c_str(), 
						szMd5, szShortCut, szComment);
					return TRUE;
				} else
				{
					::SkinMessageBox(hWnd, L"加入表情失败\n可能是自定义表情文件出错", L"提示", MB_OK);
				}
			} else
			{
				::SkinMessageBox(hWnd, L"表情已经存在", L"提示", MB_OK);
			}
		}
	} else
	{
		::SkinMessageBox(hWnd, L"请先选择表情文件", L"提示", MB_OK); 
	}
	return FALSE;
}

//弹出添加的模态窗口
void CEmotionFrameImpl::AddCustomEmotionEvent(HWND hWnd, const char *szFileName)
{
	IUIManager *pUI = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
	{
		HWND h = NULL;
		RECT rc = {0};
		RECT rcWindow = {0};
		static int RMC_SET_MODAL_WINDOW_WIDTH  = 320;
		static int RMC_SET_MODAL_WINDOW_HEIGHT = 260;
		CSystemUtils::GetScreenRect(&rcWindow);
		rc.left = (rcWindow.right - RMC_SET_MODAL_WINDOW_WIDTH) / 2;
		rc.right = rc.left + RMC_SET_MODAL_WINDOW_WIDTH;
		rc.top = (rcWindow.bottom - RMC_SET_MODAL_WINDOW_HEIGHT) / 2;
		rc.bottom = rc.top + RMC_SET_MODAL_WINDOW_HEIGHT;
		if (SUCCEEDED(pUI->CreateUIWindow(hWnd, "addemotionwindow", &rc, WS_POPUP, NULL, L"添加自定义表情", &h)))
		{
			if (szFileName)
			{
				TCHAR szTmp[MAX_PATH] = {0};
				CStringConversion::StringToWideChar(szFileName, szTmp, MAX_PATH - 1);
				::SkinSetControlTextByName(h, L"picfilename", szTmp);  
			    ::SkinSetControlAttr(h, L"gifpreview", L"image", szTmp);
			}
			::SkinShowModal(h);
		}
		pUI->Release();
	}
}

STDMETHODIMP CEmotionFrameImpl::SetCoreFrameWork(ICoreFrameWork *pCore)
{
	if (m_pCore)
		m_pCore->Release();
	m_pCore = pCore;
	if (m_pCore)
	{
		m_pCore->AddRef();
		//order
		m_pCore->AddOrderEvent((ICoreEvent *) this, "emotionframe", NULL, NULL);
		//
		m_pCore->AddOrderEvent((ICoreEvent *) this, "addemotionwindow", NULL, NULL);
		//
		m_pCore->AddOrderEvent((ICoreEvent *) this, "ChatWindow", "RichEditReadOnlyMenu", NULL); //order chat frame all event 
		m_pCore->AddOrderEvent((ICoreEvent *) this, "groupWindow", "RichEditReadOnlyMenu", NULL);
	}
	return S_OK;
}

STDMETHODIMP CEmotionFrameImpl::GetSkinXmlString(IAnsiString *szXmlString)
{
	IConfigure *pCfg = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		HRESULT hr = pCfg->GetSkinXml("EmotionFrame.xml", szXmlString);
		pCfg->Release();
		return hr;
	}
	return E_FAIL;
}

//
STDMETHODIMP CEmotionFrameImpl::CoreFrameWorkError(int nErrorNo, const char *szErrorMsg)
{
	return E_FAIL;
}

//广播消息
STDMETHODIMP CEmotionFrameImpl::DoBroadcastMessage(const char *szFromWndName, HWND hFromWnd, const char *szType,
	                     const char *pContent, void *pData)
{
	return E_FAIL;
}

//
STDMETHODIMP CEmotionFrameImpl::DoWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes)
{
	if ((hWnd == m_hWnd) && (uMsg == WM_KILLFOCUS))
	{
		::ShowWindow(m_hWnd, SW_HIDE);
	}

	return E_FAIL;
}

void CEmotionFrameImpl::RefrehPageTip()
{
	int nCount = 0, nTotalPage = 0, nCurPage = 0;
	CStdString_ strPanelName;
	if (m_bSysTab)
	{ 
		nCount = ::SkinGetDisplayEmotionCount(m_hWnd, L"emotionpanel"); 
		if (nCount > 0)
		{
			nTotalPage = m_nTotalSysCount / nCount;
			if ((m_nTotalSysCount % nCount) != 0)
				nTotalPage ++;
			nCurPage = m_nSysPage;
			strPanelName =  L"emotionpanel";
		}
	} else
	{ 
		nCount = ::SkinGetDisplayEmotionCount(m_hWnd, L"customemotionpanel"); 
		if (nCount > 0)
		{
			nTotalPage = m_nTotalCustomCount / nCount;
			if ((m_nTotalCustomCount % nCount) != 0)
				nTotalPage ++;  
			nCurPage = m_nCustomPage;
			strPanelName = L"customemotionpanel";
		}
	}
	TCHAR szwTmp[256] = {0};
	::wsprintf(szwTmp, L"%d/%d页", nCurPage + 1, nTotalPage);
	::SkinSetControlTextByName(m_hWnd, L"pagetip", szwTmp);	
}

BOOL CEmotionFrameImpl::DisplayEmotion(int nPage, BOOL bSysEmotion)
{
	TiXmlDocument *pDoc = NULL;
	int nCount = 0;
	std::string strEmotionFileName;
	std::string strPath;
	int nTotalPage = 0;
	CStdString_ strPanelName;
 
	if (bSysEmotion)
	{
		pDoc = m_SysEmotionDoc;
		nCount = ::SkinGetDisplayEmotionCount(m_hWnd, L"emotionpanel");
		strPath = m_strSysEmotionPath;
		if (nCount > 0)
		{
			nTotalPage = m_nTotalSysCount / nCount;
			if ((m_nTotalSysCount % nCount) != 0)
				nTotalPage ++; 
			if (nPage >= nTotalPage)
				return FALSE;  
			strPanelName =  L"emotionpanel";
		}
	} else
	{
		pDoc = m_CusEmotionDoc; 
		nCount = ::SkinGetDisplayEmotionCount(m_hWnd, L"customemotionpanel");
		strPath = m_strCusEmotionPath;
		if (nCount > 0)
		{
			nTotalPage = m_nTotalCustomCount / nCount;
			if ((m_nTotalCustomCount % nCount) != 0)
				nTotalPage ++; 
			if (nPage >= nTotalPage)
				return FALSE; 
			strPanelName = L"customemotionpanel";
		}
	}

	BOOL bRefresh = FALSE;
	if (nPage < 0)
	{
		if (bSysEmotion)
		{
			if (m_nSysPage < 0)
			{
				m_nSysPage = 0;
				bRefresh = TRUE;
			}
		} else
		{
			if (m_nCustomPage < 0)
			{
				m_nCustomPage = 0;
				bRefresh = TRUE;
			}
		}
		nPage = 0;
	} else
	{
		if (bSysEmotion)
		{
			if (nPage != m_nSysPage)
			{
				m_nSysPage = nPage;
				bRefresh = TRUE;
			}
		} else
		{
			if (nPage != m_nCustomPage)
			{
				m_nCustomPage = nPage;
				bRefresh = TRUE;
			}
		}
	}
	 
	TCHAR szwTmp[256] = {0};
	::wsprintf(szwTmp, L"%d/%d页", nPage + 1, nTotalPage);
	::SkinSetControlTextByName(m_hWnd, L"pagetip", szwTmp);	
	
	if (!bRefresh)
		return FALSE;
	
	::SkinClearAllEmotion(m_hWnd, strPanelName);
	if (pDoc && (nCount > 0))
	{
		TiXmlElement *pNode = pDoc->RootElement();
		TiXmlElement *pChild = NULL;
		if (pNode)
		{
			TiXmlElement *pList = pDoc->FirstChildElement("EmotionList"); 
			if (pList)
			{
				pChild = pList->FirstChildElement("SystemEmotionList");
				if (pChild)
					pChild = pChild->FirstChildElement();
			}
		}
		int nOffset = nCount * nPage;
		while (pChild && (nOffset > 0))
		{
			pChild = pChild->NextSiblingElement();
			nOffset --; 
		}
		nOffset = nCount;
		char szComment[MAX_PATH] = {0};
		while (pChild && (nOffset > 0))
		{
			strEmotionFileName = strPath;
			if (pChild->Attribute("imagefile") != NULL)
			{
				memset(szComment, 0, MAX_PATH);
				if (pChild->Attribute("comment"))
					CStringConversion::UTF8ToString(pChild->Attribute("comment"), szComment, MAX_PATH - 1);
				strEmotionFileName += pChild->Attribute("imagefile");
				::SkinAddEmotion(m_hWnd, strPanelName, strEmotionFileName.c_str(), pChild->Attribute("tag"), 
					pChild->Attribute("shortcut"), szComment); 
				nOffset --;
			}
			pChild = pChild->NextSiblingElement(); 
		} //end while

		//
		return TRUE;
	} //end if (pDoc)
	return FALSE;
}

//
STDMETHODIMP CEmotionFrameImpl::GetCustomEmotion(const char *szTag, IAnsiString *strFileName)
{
	if (!m_hWnd)
		InitEmotion();
	std::map<CAnsiString_, std::string>::iterator it = m_CusEmotionTables.find(szTag);
	if (it != m_CusEmotionTables.end())
	{
		strFileName->SetString(it->second.c_str());
		return S_OK;
	}
	return E_FAIL;
}

//IEmotionFrame
STDMETHODIMP CEmotionFrameImpl::ShowEmotionFrame(ICoreEvent *pOwner, HWND hOwner, int x, int y)
{
	if (m_pOwnerEvent)
		m_pOwnerEvent->Release();
	m_pOwnerEvent = pOwner;
	if (m_pOwnerEvent)
		m_pOwnerEvent->AddRef();
	m_pOwnerWnd = hOwner;
	if ((!m_hWnd) || (!::IsWindow(m_hWnd)))
	{
		InitEmotion(); 
	}

	if (m_hWnd && ::IsWindow(m_hWnd))
	{
		::ShowWindow(m_hWnd, SW_SHOW);
		::MoveWindow(m_hWnd, x, y, EMOTION_FRAME_WIDTH, EMOTION_FRAME_HEIGHT, TRUE);
		::CSystemUtils::BringToFront(m_hWnd); 
		return S_OK;
	}  
	return E_FAIL;
}

BOOL CEmotionFrameImpl::GetEmotionXmlFile(std::string &strFileName, BOOL bSysEmotion)
{
	BOOL bSucc = FALSE;
	IConfigure *pCfg = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		if (bSysEmotion)
		{
			CInterfaceAnsiString strTmp;
			if (SUCCEEDED(pCfg->GetParamValue(TRUE, "FileName", "EmotionFile", &strTmp)))
			{
				strFileName = strTmp.GetData();
			} else
			{
				char szAppFileName[MAX_PATH] = {0};
				char szAppPath[MAX_PATH] = {0};
				CSystemUtils::GetApplicationFileName(szAppFileName, MAX_PATH - 1);
				CSystemUtils::ExtractFilePath(szAppFileName, szAppPath, MAX_PATH - 1);
				strFileName = szAppPath;
				strFileName += EMOTION_DEFAULT_FILENAME;
			}
			if (CSystemUtils::FileIsExists(strFileName.c_str()))
				bSucc = TRUE;
		} else
		{
			CInterfaceAnsiString strTmp;
			if (SUCCEEDED(pCfg->GetPath(PATH_CUSTOM_EMOTION, &strTmp)))
			{
				strTmp.AppendString(CUSTOM_EMOTION_FILENAME);
				bSucc = TRUE;
				if (!CSystemUtils::FileIsExists(strTmp.GetData()))
				{ 
					TiXmlDocument xmldoc;
					static char CUSTOM_EMOTION_XML[] = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?><EmotionList><SystemEmotionList/></EmotionList>";
					if (xmldoc.Load(CUSTOM_EMOTION_XML, ::strlen(CUSTOM_EMOTION_XML)))
					{
						bSucc = xmldoc.SaveFile(strTmp.GetData());
					} else
						bSucc = FALSE;
				}
				if (bSucc)
					strFileName = strTmp.GetData();
			} //end if (SUCCEEDED(
		}
		pCfg->Release();
	}
	return bSucc;
}

STDMETHODIMP CEmotionFrameImpl::GetSysEmotion(const char *szTag, IAnsiString *strFileName)
{
	if (!m_hWnd)
		InitEmotion();
	std::map<CAnsiString_, std::string>::iterator it = m_EmotionTables.find(szTag);
	if (it != m_EmotionTables.end())
	{
		strFileName->SetString(it->second.c_str());
		return S_OK;
	}
	return E_FAIL;
}

void CEmotionFrameImpl::InitEmotion()
{
	IUIManager *pUI = NULL;
	RECT rc = {0, 0, EMOTION_FRAME_WIDTH, EMOTION_FRAME_HEIGHT};
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
	{
		if (FAILED(pUI->CreateUIWindow(NULL, "emotionframe", &rc, WS_POPUP | WS_MINIMIZEBOX, WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
				L"", &m_hWnd)))
		{ 
			//
			PRINTDEBUGLOG(dtInfo, "Create EmotionFrame Failed");
		} else
		{
			pUI->OrderWindowMessage("emotionframe", m_hWnd, WM_KILLFOCUS, (ICoreEvent *) this);
		}
		pUI->Release();
	}
	if (m_SysEmotionDoc)
		delete m_SysEmotionDoc;
	m_SysEmotionDoc = NULL;
	std::string strFileName; 
	m_nTotalCustomCount = 0;
	m_nTotalSysCount = 0;
	if (GetEmotionXmlFile(strFileName, TRUE))
	{
		m_SysEmotionDoc = new TiXmlDocument();
		char szEmotionPath[MAX_PATH] = {0};
		CSystemUtils::ExtractFilePath(strFileName.c_str(), szEmotionPath, MAX_PATH - 1);
		m_strSysEmotionPath = szEmotionPath;
		strDefaultSendFileName = szEmotionPath;
		strDefaultSendFileName += "sending.gif";
		strDefaultErrorFileName = szEmotionPath;
		strDefaultErrorFileName += "imageerror.gif";
		if (m_SysEmotionDoc->LoadFile(strFileName.c_str()))
		{ 
			TiXmlElement *pRoot = m_SysEmotionDoc->RootElement();
			if (pRoot && stricmp(pRoot->Value(), "EmotionList") == 0)
			{
				TiXmlElement *pSysList = pRoot->FirstChildElement();
				if (pSysList && (stricmp(pSysList->Value(), "SystemEmotionList") == 0))
				{
					std::string strTag, strCut;
					TiXmlElement *pEmotion = pSysList->FirstChildElement();
					std::string strEmotionFileName;  
					while (pEmotion)
					{
						strEmotionFileName = szEmotionPath;
			            strEmotionFileName += pEmotion->Attribute("imagefile");
					    //insert into tables
						strCut = pEmotion->Attribute("shortcut");
						m_EmotionTables.insert(std::pair<CAnsiString_, std::string>(pEmotion->Attribute("tag"), strEmotionFileName));
						pEmotion = pEmotion->NextSiblingElement();
						m_nTotalSysCount ++;
					} //end while (pEmotion)
				} //end if (pSysList)
			} //end if (pRoot &&
		} //end if (xmlDoc  
	} //end if (m_strEmotionName.c_str()...
	if (GetEmotionXmlFile(strFileName, FALSE))
	{
		if (m_CusEmotionDoc)
			delete m_CusEmotionDoc;
		m_CusEmotionDoc = new TiXmlDocument();
		if (m_CusEmotionDoc->LoadFile(strFileName.c_str()))
		{
			char szEmotionPath[MAX_PATH] = {0};
			CSystemUtils::ExtractFilePath(strFileName.c_str(), szEmotionPath, MAX_PATH - 1);
			m_strCusEmotionPath = szEmotionPath;
			TiXmlElement *pList = m_CusEmotionDoc->FirstChildElement("EmotionList");
			if (pList)
			{
				TiXmlElement *pSys = pList->FirstChildElement("SystemEmotionList");
				if (pSys)
				{
					TiXmlElement *pChild = pSys->FirstChildElement();
					std::string strEmotionFileName;
					while (pChild)
					{
						strEmotionFileName = szEmotionPath;
			            strEmotionFileName += pChild->Attribute("imagefile");
					    //insert into tables 
						m_CusEmotionTables.insert(std::pair<CAnsiString_, std::string>(pChild->Attribute("tag"), strEmotionFileName));
						 
						m_nTotalSysCount ++;
						m_nTotalCustomCount ++;
						pChild = pChild->NextSiblingElement();
					} //end while(
				} //end if (
			} //end if (pList)
		} else
		{
			delete m_CusEmotionDoc;
			m_CusEmotionDoc = NULL;
		}//end if (!m_CusEmotionDoc->
	} //end if (GetEmotionXmlFile(.. 
}

//
STDMETHODIMP CEmotionFrameImpl::GetDefaultEmotion(const char *szStyle, IAnsiString *strFileName)
{
	if (::stricmp(szStyle, "error") == 0)
	{
		if (!strDefaultErrorFileName.empty())
		{
			strFileName->SetString(strDefaultErrorFileName.c_str());
			return S_OK;
		}
	} else if (!strDefaultSendFileName.empty())
	{
		strFileName->SetString(strDefaultSendFileName.c_str());
		return S_OK;
	}
	return E_FAIL;
}

#pragma warning(default:4996)
