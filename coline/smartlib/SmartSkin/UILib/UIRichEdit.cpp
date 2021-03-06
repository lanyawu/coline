#include "common.h"
#include <fstream>
#include <Crypto/Crypto.h>
#include <UILib/UIRichEdit.h>
#include <CommonLib/StringUtils.h>
#include <UILib/UIScroll.h>
#include <CommonLib/ImageDataObject.h>
#include <commonlib/debuglog.h>
#include <commonlib/classes.h>
#include <Commonlib/graphicplus.h>
#pragma warning(disable:4996)


#ifdef NDEBUG
	#ifdef __USER_QQ_IMAGEOLE___
		#import "..\\..\\bin\\debug\\imageole.dll" named_guids rename_namespace("GoComImageOle")
	#else
		#import "..\\..\\bin\\debug\\baseolectrl.dll" named_guids no_namespace
	#endif
#else
	#ifdef __USER_QQ_IMAGEOLE___
		#import "..\\..\\bin\\debug\\imageole.dll" named_guids rename_namespace("GoComImageOle")
	#else
		#import "..\\..\\bin\\debug\\baseolectrl.dll" named_guids no_namespace
	#endif
#endif

//oleobj格式
#define OLE_OBJECT_FORMAT_STR "/{%s/}"

#define EM_GETOLEINTERFACE  (WM_USER + 60)

#define EM_NOTIFY  0x204E  //通知消息

HMODULE CRichEditUI::m_hRichEdit2Module = NULL;
volatile LONG CRichEditUI::m_hRichEditRef = 0;


//文本提示分析器
class CTipParser
{
public:
	CTipParser(const TCHAR *szText);
public:
	enum{
		TIP_PARSER_TEXT = 1,
		TIP_PARSER_LINK,
		TIP_PARSER_FLAG
	};
	BOOL GetNextToken(TCHAR *szDest, int nInitToken, int &nNextToken);
private:
	const TCHAR *m_pszText;
	DWORD m_dwPos; //当前位置
	DWORD m_dwSize; //文本长度
};

CTipParser::CTipParser(const TCHAR *szText):
            m_pszText(szText),
			m_dwPos(0)
{
	m_dwSize = (DWORD)::lstrlen(m_pszText);
}

BOOL CTipParser::GetNextToken(TCHAR *szDest, int nInitToken, int &nNextToken)
{
	if (m_dwPos >= m_dwSize)
		return FALSE;
	const TCHAR *szTemp = m_pszText + m_dwPos;
	const TCHAR *szCurr = szTemp;
	switch(nInitToken)
	{
	case TIP_PARSER_TEXT:
		 while((*szCurr != L'\0') && (*szCurr != L'<'))
			 szCurr ++;
	     nNextToken = TIP_PARSER_LINK;
		 break;
	case TIP_PARSER_LINK:
		 while((*szCurr != L'\0') && (*szCurr != L','))
			 szCurr ++;
		 nNextToken = TIP_PARSER_FLAG;
		 break;
	case TIP_PARSER_FLAG:
		 while((*szCurr != L'\0') && (*szCurr != L'>'))
			 szCurr ++;
		 nNextToken = TIP_PARSER_TEXT;
		 break;
	default:
		 return FALSE;
	}
	::lstrcpyn(szDest, szTemp, szCurr - szTemp + 1);
	m_dwPos = m_dwPos + (szCurr - szTemp) + 1;
	return TRUE;
}

//CRichEditUI

CRichEditUI::CRichEditUI(void):
             m_pWindowlessRE(NULL),
			 m_dwOleSeq(0),
			 m_pApp(NULL),
			 m_pCallBack(NULL),
			 m_lpOverlapped(NULL), 
			 m_bAIMsg(TRUE), 
			 m_nTipBitmapId(0)
{
	memset(&m_fcDefaultStyle, 0, sizeof(CCharFontStyle));
	m_fcDefaultStyle.cfColor = RGB(0, 0, 0);
	m_fcDefaultStyle.nFontSize = 8;
	_tcscpy(m_fcDefaultStyle.szFaceName, L"Tahoma");
}

CRichEditUI::~CRichEditUI(void)
{
	Clear();
   ::InterlockedDecrement(&m_hRichEditRef);
   if (m_hRichEditRef == 0)
   {
	   ::FreeLibrary(m_hRichEdit2Module);
	   m_hRichEdit2Module = NULL;
	   ::CoUninitialize();
   }
   m_pRichEditOle = NULL;
   if (m_pWindowlessRE)
   {
	   delete m_pWindowlessRE;
	   m_pWindowlessRE = NULL;
   }
   if (m_pManager)
	   m_pManager->RemoveMessageFilter(this);
}

void CRichEditUI::Attach(IRichEditApp *pApp)
{
	m_pApp = pApp;
}

void CRichEditUI::Attach(LPSKIN_RICHEDIT_EVENT_CALLBACK pCallback, LPVOID lpOverlapped)
{
	m_pCallBack = pCallback;
	m_lpOverlapped = lpOverlapped;
}

void CRichEditUI::Clear()
{
	//清空自定义列表
	m_LinkLock.Lock();
	std::map<DWORD, LPCustomLinkItem>::iterator LinkIt;
	for (LinkIt = m_CustomLinkList.begin(); LinkIt != m_CustomLinkList.end(); LinkIt ++)
	{
		delete (*LinkIt).second;
	}
	m_CustomLinkList.clear();
	m_AckLinkList.clear();
	m_FileList.clear();
	m_LinkLock.UnLock();

	//删除ole列表
	m_OleLock.Lock();
	std::map<DWORD, LPOLE_ITEM>::iterator it;
	for (it = m_OleList.begin(); it != m_OleList.end(); it ++)
	{
		if (it->second)
			delete it->second;
	}
	m_OleList.clear();
	m_dwOleSeq = 0;
	m_rcMsgArea.clear();
	m_OleLock.UnLock();
	if (m_pWindowlessRE)
		m_pWindowlessRE->SetText(_T(""));
}

int CRichEditUI::GetCount()
{
	int nCount = (int)PerformMessage(EM_GETLINECOUNT, 0, 0);
    nCount = (int)PerformMessage(EM_LINEINDEX, nCount - 1, 0);
	nCount += (int)PerformMessage(EM_LINELENGTH, nCount, 0);
	return nCount;
}

//设置最大字符数
void CRichEditUI::SetMaxTextLen(int nLength)
{ 
	if (m_pWindowlessRE)
		m_pWindowlessRE->SetMaxTextLength(nLength);
}

void CRichEditUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (::_tcsicmp(pstrName, L"aspect") == 0)
	{
		//
		//ES_VERTICAL
	} else if (::_tcsicmp(pstrName, L"readonly") == 0)
	{
		if (m_pWindowlessRE)
			SetReadOnly(_tcscmp(pstrValue, L"true") == 0);
		else
			m_AttrList[pstrName] = pstrValue;
	} else if (::_tcsicmp(pstrName, L"autodetectlink") == 0)
	{
		if (m_pWindowlessRE)
			SetAutoDetectLink(_tcscmp(pstrValue, L"true") == 0);
		else
			m_AttrList[pstrName] = pstrValue;
	} else if (::_tcsicmp(pstrName, L"transparent") == 0)
	{
		if (m_pWindowlessRE)
			SetTransParent(_tcscmp(pstrValue, L"true") == 0);
		else
			m_AttrList[pstrName] = pstrValue;
	} else if (::_tcsicmp(pstrName, L"TipImage") == 0)
	{
		m_nTipBitmapId = _ttoi(pstrValue);
	} else if (::_tcsicmp(pstrName, L"mergemsg") == 0)
	{
		m_bAIMsg = (::_tcsicmp(pstrValue, L"true") == 0);
	} else if (::_tcsicmp(pstrName, L"text") == 0)
	{
		SetText(pstrValue);
	} else if (::_tcsicmp(pstrName, L"maxtextlength") == 0)
	{
		if (m_pWindowlessRE)
			SetMaxTextLen(_ttoi(pstrValue));
		else
			m_AttrList[pstrName] = pstrValue;
	} else
		CContainerUI::SetAttribute(pstrName, pstrValue);
}

BOOL CRichEditUI::GetCustomAckLink(DWORD dwMin, CCharRange &cr)
{
	std::map<DWORD, CCharRange>::iterator it = m_AckLinkList.find(dwMin);
	if (it != m_AckLinkList.end())
	{
		cr = it->second;
		m_AckLinkList.erase(it);
		return TRUE;
	}
	return FALSE;
}

BOOL CRichEditUI::InsertFileLink(CCharFontStyle *cfStyle, DWORD dwOffset, const char *szTip, const char *szFileName)
{
	if (szTip && szFileName)
	{ 
		std::string strTip = szTip;
		int nPos = strTip.find("%%FILE%%");
		if (nPos != std::string::npos)
		{
			strTip.replace(nPos, ::strlen("%%FILE%%"), szFileName);
		} else
			strTip += szFileName;
		TCHAR *szwTmp = new TCHAR[strTip.size() + 1];
		memset(szwTmp, 0, sizeof(TCHAR) * (strTip.size() + 1));
		CStringConversion::StringToWideChar(strTip.c_str(), szwTmp, strTip.size());
		InsertTip(cfStyle, dwOffset, szwTmp);
		delete []szwTmp;
		CCharRange range;
		BOOL b = TRUE;
		SetLeftIndent(dwOffset);
		range.dwMin = GetCount();
		range.dwMax = range.dwMin;
		PerformMessage(EM_EXSETSEL, 0, LPARAM(&range));
	#ifdef _UNICODE
		int  size = (int)::lstrlen(L"打开文件");
		PerformMessage(EM_REPLACESEL, 0, LPARAM(L"打开文件"));
	#else
		PerformMessage(EM_REPLACESEL, 0, LPARAM(szText));
	#endif	
		range.dwMax = range.dwMin + size;

		PerformMessage(EM_EXSETSEL, 0, LPARAM(&range));
		CHARFORMAT_RE param;
		memset(&param, 0, sizeof(CHARFORMAT_RE));
		param.cbSize = sizeof(param);
		param.dwMask = CFM_LINK;
		param.dwEffects = CFE_LINK;
		PerformMessage(EM_SETCHARFORMAT, SCF_SELECTION, LPARAM(&param));
		//加入到列表当中
		m_LinkLock.Lock(); 
		m_FileList.insert(pair<DWORD, std::string>(range.dwMin, szFileName));
		m_LinkLock.UnLock();

		//打开目录 
		Append(L"  ");
		range.dwMin = GetCount();
		range.dwMax = range.dwMin;
		PerformMessage(EM_EXSETSEL, 0, LPARAM(&range));
	#ifdef _UNICODE
		size = (int)::lstrlen(L"打开目录");
		PerformMessage(EM_REPLACESEL, 0, LPARAM(L"打开目录"));
	#else
		PerformMessage(EM_REPLACESEL, 0, LPARAM(szText));
	#endif	
		range.dwMax = range.dwMin + size;

		PerformMessage(EM_EXSETSEL, 0, LPARAM(&range)); 
		memset(&param, 0, sizeof(CHARFORMAT_RE));
		param.cbSize = sizeof(param);
		param.dwMask = CFM_LINK;
		param.dwEffects = CFE_LINK;
		PerformMessage(EM_SETCHARFORMAT, SCF_SELECTION, LPARAM(&param));
		char szPath[MAX_PATH] = {0};
		CSystemUtils::ExtractFilePath(szFileName, szPath, MAX_PATH - 1);
		//加入到列表当中
		m_LinkLock.Lock(); 
		m_FileList.insert(pair<DWORD, std::string>(range.dwMin, szPath));
		m_LinkLock.UnLock();
		//
		Append(L"\n");
	}
	return FALSE;
}

BOOL CRichEditUI::GetFileNameLink(DWORD dwMin, std::string &strFileName)
{
	std::map<DWORD, std::string>::iterator it = m_FileList.find(dwMin);
	if (it != m_FileList.end())
	{
		strFileName = it->second;
		return TRUE;
	}
	return FALSE;
}

void CRichEditUI::Notify(UINT uMsg, void *lpParam)
{
	if (uMsg == EN_LINK)
	{
		ENLINK *p = (ENLINK *)lpParam;
		if (p->msg  == WM_LBUTTONDOWN)
		{
			CCharRange crAck = {0};
			std::string strFileName;
			if (GetCustomAckLink(p->chrg.cpMin, crAck))
			{
				if (crAck.dwMax - crAck.dwMin > 128)
					crAck.dwMax = crAck.dwMin + 128;
				PerformMessage(EM_EXSETSEL, 0, LPARAM(&crAck));
				char szTitle[MAX_PATH] = {0};
				TCHAR szTemp[MAX_PATH] = {0};
				PerformMessage(EM_GETSELTEXT, 0, LPARAM(&szTemp));
				CStringConversion::WideCharToString(szTemp, szTitle, MAX_PATH);
				if (m_pApp)
					m_pApp->OnCustomLinkClick(szTitle, 0xFFFFFFFF);
				else if (m_pCallBack)
				{
					if (m_pManager)
						m_pCallBack(m_pManager->GetPaintWindow(), RICHEDIT_EVENT_CUSTOMLINKCLICK, 
						           szTitle, NULL, NULL, NULL, 0xFFFFFFFF, m_lpOverlapped);
					else
						m_pCallBack(NULL, RICHEDIT_EVENT_CUSTOMLINKCLICK, 
						           szTitle, NULL, NULL, NULL, 0xFFFFFFFF, m_lpOverlapped);
				}
				PerformMessage(EM_EXSETSEL, 0, LPARAM(&p->chrg));
				CHARFORMAT_RE param;
				memset(&param, 0, sizeof(CHARFORMAT_RE));
				param.cbSize = sizeof(param);
				param.dwMask = CFM_LINK;
				param.dwEffects = 0;
				PerformMessage(EM_SETCHARFORMAT, SCF_SELECTION, LPARAM(&param));
			} else if (GetFileNameLink(p->chrg.cpMin, strFileName))
			{
				CSystemUtils::OpenURL(strFileName.c_str());
			} else
			{
				PerformMessage(EM_EXSETSEL, 0, LPARAM(&(p->chrg)));
				char szTitle[MAX_PATH] = {0};
#ifdef _UNICODE
				TCHAR szTemp[MAX_PATH] = {0};
				PerformMessage(EM_GETSELTEXT, 0, LPARAM(&szTemp));
				CStringConversion::WideCharToString(szTemp, szTitle, MAX_PATH);
#else
				PerformMessage(EM_GETSELTEXT, 0, LPARAM(&szTitle));
#endif
				//查找是否存在自定义custom
				DWORD dwFlag = GetCustomLinkFlag(p->chrg.cpMin);
				if (dwFlag == 0)
				{
					CancelCustomLink(0);
					LinkOnClick(szTitle, TRUE, 0);
				}
				if (dwFlag > 0)
				   LinkOnClick(szTitle, TRUE, dwFlag);
				else
				   LinkOnClick(szTitle, FALSE, 0);
			}
		}
	}
}

void CRichEditUI::Init()
{
	CContainerUI::Init();
    if (!CRichEditUI::m_hRichEdit2Module)
    {
	   ::CoInitialize(NULL);
	   CRichEditUI::m_hRichEdit2Module = ::LoadLibrary(L"RICHED20.DLL");
	   if (CRichEditUI::m_hRichEdit2Module <= (HMODULE)HINSTANCE_ERROR)
		   CRichEditUI::m_hRichEdit2Module = (HMODULE)0;
    }
	::InterlockedIncrement(&m_hRichEditRef);
    m_pWindowlessRE = new CWindowlessRE(this);
	HWND hParent = ::GetParent(GetManager()->GetPaintWindow());

	//初始化
	m_rcItem.left = 0;
	m_rcItem.top = 0;
	m_rcItem.right = 100;
	m_rcItem.bottom = 100;
	m_pWindowlessRE->Init(hParent, GetManager()->GetPaintWindow(), 
		       ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL, ES_SUNKEN, m_rcItem); 

	//获取Ole接口
	m_pRichEditOle = NULL;
	PerformMessage(EM_GETOLEINTERFACE, 0, LONG(&m_pRichEditOle));
 
	SetFontStyle(m_fcDefaultStyle);

	std::map<CStdString, CStdString>::iterator it;
	for (it = m_AttrList.begin(); it != m_AttrList.end(); it ++)
	{
		SetAttribute(it->first, it->second);
	}
	m_AttrList.clear();
	//if (m_pManager)
	//	m_pManager->AddMessageFilter(this);
}

void CRichEditUI::SetReadOnly(BOOL bReadOnly)
{
	if (m_pWindowlessRE)
		m_pWindowlessRE->SetReadOnly(bReadOnly);
}

BOOL CRichEditUI::IsReadOnly() const
{
	return m_pWindowlessRE->GetReadOnly();
}

LPCTSTR CRichEditUI::GetClass() const
{
	return _T("RICHEDITLESS");
}

UINT CRichEditUI::GetControlFlags() const
{
	return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
}

CStdString CRichEditUI::GetText() const
{
	if (m_pWindowlessRE)
	{
		char *szTmp = m_pWindowlessRE->GetText();
		CStdString strTmp;
		TCHAR *szwTmp = new TCHAR[::strlen(szTmp) + 1];
		memset(szwTmp, 0, (strlen(szTmp) + 1) * sizeof(TCHAR));
		CStringConversion::StringToWideChar(szTmp, szwTmp, ::strlen(szTmp));
		strTmp = szwTmp;
		delete []szwTmp;
		return strTmp;
	} 
	return CStdString(_T(""));
}

void CRichEditUI::SetText(LPCTSTR pstrText)
{
	if (m_pWindowlessRE)
		m_pWindowlessRE->SetText(pstrText);
}

void CRichEditUI::SetEnabled(bool bEnabled)
{
	SetReadOnly(!bEnabled);
	CContainerUI::SetEnabled(bEnabled);
}

void CRichEditUI::SetVisible(bool bVisible)
{
	CContainerUI::SetVisible(bVisible);
	if (m_pWindowlessRE)
		m_pWindowlessRE->TxWindowProc(GetManager()->GetPaintWindow(), WM_SHOWWINDOW, (WPARAM)bVisible, 0);
}

//滚动条相关
BOOL CRichEditUI::RE_ShowScrollBar(int nBar, BOOL bShow)
{
	if (!m_pVScrollBar)
		return FALSE;
	
	//函数调用后，滚动条不一定立即变为为bShow指定的状态，
	//滚动条自动调整可见性，根据page，range，和bShow的综合
	//状态。但是，经过edit控件的多次回掉之后，scrollbar的状态
	//必须和bShow一致
	m_pVScrollBar->SetVisible( bShow == TRUE );

	//不管m_pVScrollBar是否可见，都根据bShow重新设置
	//richedit的位置
	RECT rcEdit = m_rcItem;
	if( bShow )
	{
		rcEdit.right -= SB_WIDTH;
	}
	m_pWindowlessRE->SetClientRect(&rcEdit, TRUE);
	//重新设置in-place active的矩形
	RECT rcActive = { 0 };
	m_pWindowlessRE->GetControlRect(&rcActive);
	m_pWindowlessRE->OnTxInPlaceActivate(&rcActive);

	LONG lMin, lMax, lPos, lPage;
	BOOL bEnabled;
	m_pWindowlessRE->TxGetVScroll(&lMin, &lMax, &lPos, &lPage, &bEnabled);
	SetScrollRange(UISB_VERT, lMin, lMax);
	return TRUE;
}

BOOL CRichEditUI::RE_EnableScrollBar(int nSBFlags, int nArrowFlags)
{
	return FALSE;
}

BOOL CRichEditUI::RE_SetScrollRange(int nBar, LONG nMinPos, int nMaxPos, BOOL bRedraw)
{
	SetScrollRange(UISB_VERT, nMinPos, nMaxPos-1);
	Invalidate();
	return TRUE;
}

BOOL CRichEditUI::RE_SetScrollPos(int nBar, int nPos, BOOL bRedraw)
{
	SetScrollPos(UISB_VERT, nPos);
	Invalidate();
	return TRUE;//FALSE 2009.1.14
}

void CRichEditUI::RE_TxScrollWindowEx (int dx, int dy, LPCRECT lprcScroll, LPCRECT lprcClip,	
		           HRGN hrgnUpdate, LPRECT lprcUpdate, UINT uScroll)
{
    //
}

//内容发送改变事件
void CRichEditUI::OnTextChange()
{
	if (m_pManager)
		m_pManager->SendNotify(this, _T("changed"));
}

//回车按下
BOOL CRichEditUI::OnEnterKeyDown()
{
	if (m_pManager)
	{
		TNotifyUI Msg;
		Msg.pSender = this;
		Msg.sType = _T("enterkeydown");
		Msg.wParam = 0;
		Msg.lParam = 0;
		Msg.bHandled = FALSE;
		m_pManager->SendNotify(Msg);
		return Msg.bHandled;
	}
	return FALSE;
}

BOOL CRichEditUI::OnSaveAs(HINSTANCE hInstance, HWND hParent)
{
	CCharRange rg = {0};
	PerformMessage(EM_EXGETSEL, 0, LPARAM(&rg));
	if (rg.dwMax > rg.dwMin)
	{
		if ((rg.dwMax - rg.dwMin) == 1)
		{
			DWORD dwUserId = GetUserIdByPosition(rg.dwMin);
			char szSrcFileName[MAX_PATH] = {0};
			m_OleLock.Lock();
			std::map<DWORD, LPOLE_ITEM>::iterator it = m_OleList.find(dwUserId);
			if (it != m_OleList.end())
			{
				strncpy(szSrcFileName, it->second->szFileName, MAX_PATH - 1);
			}
			m_OleLock.UnLock();
			if (szSrcFileName[0] != '\0')
			{
				BOOL b = FALSE;
				char szExt[16] = {0};
				char szFilter[MAX_PATH] = {0};
				CSystemUtils::ExtractFileExtName(szSrcFileName, szExt, 15);
				if (szExt[0] != '\0')
				{
					sprintf(szFilter, "%s文件(*.%s)|*.%s|所有文件(*.*)|*.*", szExt, szExt, szExt);
				} else
					sprintf(szFilter, "所有文件(*.*)|*.*");

				CStringList_ slFiles;
				if (CSystemUtils::OpenFileDialog(hInstance, hParent, "选择保存的文件名称", szFilter,
					"", slFiles, FALSE, TRUE))
				{
					std::string szFileName;
					if (!slFiles.empty())
					{
						szFileName = slFiles.back();
					}
					if (!szFileName.empty())
						b = CSystemUtils::CopyFilePlus(szSrcFileName, szFileName.c_str(), TRUE); 
										 
				}
				return b;
			}
		}
	}
	return FALSE;
}
BOOL CRichEditUI::RichEditCommand_(const char *szCommand, LPVOID lpParams)
{
	if (::stricmp(szCommand, "cut") == 0)
		return OnCut();
	else if (::stricmp(szCommand, "copy") == 0)
		return OnCopy();
	else if (::stricmp(szCommand, "paste") == 0)
		return OnPaste();
	else if (::stricmp(szCommand, "selectall") == 0)
	{
		SelectAll();
		return TRUE;
	} else if (::stricmp(szCommand, "clear") == 0)
	{
		Clear();
		return TRUE;
	}
	return FALSE;
}

BOOL CRichEditUI::OnCut()
{
	if (IsReadOnly())
		return FALSE;
	CCharRange rg = {0};
	PerformMessage(EM_EXGETSEL, 0, LPARAM(&rg));
	if (rg.dwMax > rg.dwMin)
	{
		if ((rg.dwMax - rg.dwMin) == 1)
		{
			DWORD dwUserId = GetUserIdByPosition(rg.dwMin);
			char szFileName[MAX_PATH] = {0};
			m_OleLock.Lock();
			std::map<DWORD, LPOLE_ITEM>::iterator it = m_OleList.find(dwUserId);
			if (it != m_OleList.end())
			{
				strncpy(szFileName, it->second->szFileName, MAX_PATH - 1);
				delete it->second;
				m_OleList.erase(it);
			}
			m_OleLock.UnLock();
			//替换
			TCHAR szwText[] = _T("");
			PerformMessage(EM_REPLACESEL, 0, LPARAM(szwText));
			if (szFileName[0] != '\0')
			{
				CGdiPlusGif *gif = new CGdiPlusGif(NULL, szFileName, FALSE);
				gif->CopyToClipboard();
				delete gif;
				return TRUE;
			}
		}
		return (BOOL)PerformMessage(WM_CUT, 0, 0);
	}
	return FALSE;
}

BOOL CRichEditUI::GetSelectImageFileName(char *szFileName, int &nSize)
{
	CCharRange rg = {0};
	BOOL bSucc = FALSE;
	PerformMessage(EM_EXGETSEL, 0, LPARAM(&rg));
	if (rg.dwMax > rg.dwMin)
	{
		if ((rg.dwMax - rg.dwMin) == 1)
		{
			DWORD dwUserId = GetUserIdByPosition(rg.dwMin); 
			m_OleLock.Lock();
			std::map<DWORD, LPOLE_ITEM>::iterator it = m_OleList.find(dwUserId);
			if ((it != m_OleList.end()) && (nSize >= (int)::strlen(it->second->szFileName)))
			{
				nSize = ::strlen(it->second->szFileName);
				strncpy(szFileName, it->second->szFileName, nSize);
				bSucc = TRUE;
			}
			m_OleLock.UnLock(); 
		} //end if (..
	} //end if (rg.dwMax..
	return bSucc;
}

BOOL CRichEditUI::OnCopy()
{
	CCharRange rg = {0};
	PerformMessage(EM_EXGETSEL, 0, LPARAM(&rg));
	if (rg.dwMax > rg.dwMin)
	{
		if ((rg.dwMax - rg.dwMin) == 1)
		{
			DWORD dwUserId = GetUserIdByPosition(rg.dwMin);
			char szFileName[MAX_PATH] = {0};
			m_OleLock.Lock();
			std::map<DWORD, LPOLE_ITEM>::iterator it = m_OleList.find(dwUserId);
			if (it != m_OleList.end())
			{
				strncpy(szFileName, it->second->szFileName, MAX_PATH - 1);
			}
			m_OleLock.UnLock();
			if (szFileName[0] != '\0')
			{
				CGdiPlusGif *gif = new CGdiPlusGif(NULL, szFileName, FALSE);
				gif->CopyToClipboard();
				delete gif;
				return TRUE;
			}
		}
		return (BOOL)PerformMessage( WM_COPY, 0, 0 );
	}
	return FALSE;
}

BOOL CRichEditUI::InsertOlePicture(const char *szFileName)
{ 
	if (szFileName)
	{
		CGraphicPlus srcGraph;
		if (srcGraph.LoadFromFile(szFileName, FALSE))
		{ 
			BYTE *pBuff = NULL;
			long nSize = 0;
			if (srcGraph.SaveToStream(pBuff, nSize, GRAPHIC_TYPE_GIF))
			{
				char szMd5[36] = {0};
				::md5_encode((char *)pBuff, nSize, szMd5);
				char szDestFileName[MAX_PATH] = {0};
				BOOL bSucc = FALSE;
				if (m_pApp)
					bSucc = m_pApp->GetFileNameByTag(szDestFileName, szMd5);
				else
				{
					DWORD nNameSize = MAX_PATH - 1;
					DWORD nTagSize = 35;
					if (m_pManager)
						bSucc = m_pCallBack(m_pManager->GetPaintWindow(), RICHEDIT_EVENT_GETFILEBYTAG, szDestFileName, &nNameSize,
						                 szMd5, &nTagSize, 0, m_lpOverlapped);
					else
						bSucc = m_pCallBack(NULL, RICHEDIT_EVENT_GETFILEBYTAG, szDestFileName, &nNameSize,
						                 szMd5, &nTagSize, 0, m_lpOverlapped);
				}
				if (bSucc)
				{
					TCHAR szTemp[MAX_PATH] = {0};
					CStringConversion::StringToWideChar(szDestFileName, szTemp, MAX_PATH);
					ofstream ofs(szTemp, std::ios::out | std::ios::binary);
					if (ofs.is_open())
					{
						ofs.write((char *)pBuff, nSize);
						ofs.close();
					}
					//插入到显示框
					InsertGif(szDestFileName, NULL, szMd5, 0);
				} else
				{
					InsertBitmap(srcGraph.GetBitmap());
				}	//end if (bSucc)	
			} //end if (srcGraph.SaveToStream(..
			if (pBuff)
				delete []pBuff;		
		}  
		ScrollToBottom();
		return TRUE;
	} //end if (sz
	return FALSE;
}

BOOL CRichEditUI::OnPaste()
{
	if( IsReadOnly() )
		return false;

	HANDLE hClip = NULL;
	//粘贴文件
	if (::IsClipboardFormatAvailable(CF_HDROP))
	{
		HDROP hDrop = (HDROP)::GetClipboardData(CF_HDROP);
		BOOL bIsOpenClip = FALSE;
		if (!hDrop)
		{
			::OpenClipboard(GetManager()->GetPaintWindow());
			bIsOpenClip = TRUE;
			hDrop = (HDROP)::GetClipboardData(CF_HDROP);
			if (!hDrop)
				return false;
		}
		if ((hDrop > 0) && (m_pApp || m_pCallBack))
		{
			WCHAR szDropFileName[MAX_PATH] = {0};
			char  szTemp[MAX_PATH];
			UINT nCount = ::DragQueryFile(hDrop, 0xFFFFFFFF, szDropFileName, MAX_PATH);
			for (UINT i = 0; i < nCount; i ++)
			{
				memset(szDropFileName, 0, MAX_PATH * sizeof(WCHAR));
				::DragQueryFile(hDrop, i, szDropFileName, MAX_PATH);
				memset(szTemp, 0, MAX_PATH);
				CStringConversion::WideCharToString(szDropFileName, szTemp, MAX_PATH);
				if (CSystemUtils::FileIsExists(szTemp))
				{
					if (m_pApp)
						m_pApp->SendFile(szTemp);
					else 
					{
						if (m_pManager)
							m_pCallBack(m_pManager->GetPaintWindow(), RICHEDIT_EVENT_SENDFILE, szTemp, NULL, NULL, 
						            NULL, 0, m_lpOverlapped);
						else
							m_pCallBack(NULL, RICHEDIT_EVENT_SENDFILE, szTemp, NULL, NULL, 
						            NULL, 0, m_lpOverlapped);
					}
				}
			}
		}
		if (bIsOpenClip)
			::CloseClipboard();
	}
	//粘贴图片
	if (::IsClipboardFormatAvailable(CF_BITMAP))
	{
		hClip = ::GetClipboardData(CF_BITMAP);
		BOOL bIsOpenClip = FALSE;
		if (!hClip)
		{
			::OpenClipboard(GetManager()->GetPaintWindow());
			bIsOpenClip = TRUE;
			hClip = ::GetClipboardData(CF_BITMAP);
			if (!hClip)
				return false;
		}
        if (m_pApp || m_pCallBack)
		{
			HPALETTE hPalette = (HPALETTE) ::GetClipboardData(CF_PALETTE);
			CGraphicPlus graph;
			graph.LoadFromBitmap((HBITMAP)hClip, hPalette);
			BYTE *pBuff = NULL;
			long nSize = 0;
			if (graph.SaveToStream(pBuff, nSize, GRAPHIC_TYPE_JPG))
			{
				char szMd5[36] = {0};
				::md5_encode((char *)pBuff, nSize, szMd5);
				char szFileName[MAX_PATH] = {0};
				BOOL bSucc = FALSE;
				if (m_pApp)
					bSucc = m_pApp->GetFileNameByTag(szFileName, szMd5);
				else
				{
					DWORD nNameSize = MAX_PATH - 1;
					DWORD nTagSize = 35;
					if (m_pManager)
						bSucc = m_pCallBack(m_pManager->GetPaintWindow(), RICHEDIT_EVENT_GETFILEBYTAG, szFileName, &nNameSize,
						szMd5, &nTagSize, 0, m_lpOverlapped);
					else
						bSucc = m_pCallBack(NULL, RICHEDIT_EVENT_GETFILEBYTAG, szFileName, &nNameSize,
						szMd5, &nTagSize, 0, m_lpOverlapped);
				}
				if (bSucc)
				{
					TCHAR szTemp[MAX_PATH] = {0};
					CStringConversion::StringToWideChar(szFileName, szTemp, MAX_PATH);
					ofstream ofs(szTemp, std::ios::out | std::ios::binary);
					if (ofs.is_open())
					{
						ofs.write((char *)pBuff, nSize);
						ofs.close();
					}
					//插入到显示框
					InsertGif(szFileName, NULL, szMd5, 0);
				}
			}
			if (pBuff)
				delete []pBuff;			
		} else //else if (m_pApp)
		{
			//
			InsertBitmap((HBITMAP)hClip);
		}
		if (bIsOpenClip)
			::CloseClipboard();
		ScrollToBottom();
		return true;		
	}

	//粘贴文字
	if ( ::IsClipboardFormatAvailable(CF_TEXT) )
	{
		hClip = ::GetClipboardData(CF_TEXT);
		BOOL bIsOpenClip = FALSE;
		if (!hClip)
		{
			::OpenClipboard(GetManager()->GetPaintWindow());
			bIsOpenClip = TRUE;
			hClip = ::GetClipboardData(CF_TEXT);
			if (!hClip)
				return false;
		}
		char *p = (char *)::GlobalLock(hClip);
		int nLen = (int)::strlen(p);
		if (nLen > m_pWindowlessRE->GetMaxTextLength())
			nLen = m_pWindowlessRE->GetMaxTextLength();
		TCHAR *szTemp = new TCHAR[nLen + 1];
		memset(szTemp, 0, sizeof(TCHAR) * (nLen + 1));
		CStringConversion::StringToWideChar(p, szTemp, nLen);
		CCharFontStyle cf = {0};
		GetFontStyle(cf);
		CHARFORMAT_RE cfmt;
		CharFontStyleToFormat(cf, cfmt );
		PerformMessage(EM_REPLACESEL, 0, LPARAM(szTemp));
		delete []szTemp;
		::GlobalUnlock(hClip);
		if (bIsOpenClip)
			::CloseClipboard();
		return true;
	}
	
	return false;
}

void CRichEditUI::SetFocus()
{
	if (m_pWindowlessRE)
	{
		m_pWindowlessRE->OnUIActivate();
		if (m_pManager)
			PerformMessage(WM_SETCURSOR, (WPARAM)m_pManager->GetPaintWindow(), 0);
	}
	CContainerUI::SetFocus();
}

LRESULT CRichEditUI::PerformMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_pWindowlessRE)
		return m_pWindowlessRE->TxWindowProc(GetManager()->GetPaintWindow(), uMsg, wParam, lParam);
	return S_FALSE;
}

void CRichEditUI::SetTransParent(bool b)
{
	if (m_pWindowlessRE)
		m_pWindowlessRE->SetTransparent(b);
}

void CRichEditUI::SetAutoDetectLink(bool bAuto)
{
	DWORD dwEventMask =  ENM_LINK | ENM_MOUSEEVENTS | ENM_SCROLLEVENTS | ENM_KEYEVENTS;
    PerformMessage(EM_SETEVENTMASK, 0, dwEventMask);
	PerformMessage(EM_AUTOURLDETECT, bAuto, 0);
}

void CRichEditUI::Event(TEventUI& e)
{
		UINT uMsg = 0;
	BOOL bValid = true;
	RECT rcEditClient;
	m_pWindowlessRE->GetControlRect(&rcEditClient);

	switch(e.Type)
	{
	//mouse event
	case UIEVENT_MOUSEMOVE:
	case UIEVENT_MOUSELEAVE:
	case UIEVENT_MOUSEENTER:
	case UIEVENT_MOUSEHOVER:
		 uMsg = WM_MOUSEMOVE;
		 break;
	case UIEVENT_BUTTONDOWN:
		 uMsg = WM_LBUTTONDOWN;
		 bValid = ::PtInRect(&rcEditClient, e.ptMouse);
		 break;
	case UIEVENT_BUTTONUP:
		 uMsg = WM_LBUTTONUP;
		 break;
	case UIEVENT_DBLCLICK:
		 uMsg = WM_LBUTTONDBLCLK;
		 bValid = ::PtInRect(&rcEditClient, e.ptMouse);
		 break;
	case UIEVENT_RBUTTONDOWN:
		//uMsg = WM_RBUTTONDOWN;
		bValid = ::PtInRect(&rcEditClient, e.ptMouse);
		break;
	case UIEVENT_RBUTTONUP:
		//uMsg = WM_RBUTTONUP;
		if (m_pManager && ::PtInRect(&m_rcItem, e.ptMouse))
		{
			m_pManager->SendNotify(this, _T("rbuttonclick"));
		}
		break;

	//...
	case UIEVENT_VSCROLL:
		 uMsg = WM_VSCROLL;
		 break;
	case UIEVENT_HSCROLL:
		 uMsg = WM_HSCROLL;
		 break;
	case UIEVENT_SCROLLWHEEL:
		 uMsg = WM_VSCROLL;
		 break;

	//key
	case UIEVENT_KEYDOWN:
		 uMsg = WM_KEYDOWN;
		 break;
	case UIEVENT_KEYUP:
		 uMsg = WM_KEYUP;
		 break;
	case UIEVENT_CHAR:
		 uMsg = WM_CHAR;
		 break;
	case UIEVENT_SYSKEY:
		 uMsg = WM_SYSKEYDOWN;
		 break;

	//focus
	case UIEVENT_KILLFOCUS:
		 uMsg = WM_KILLFOCUS;
		 break;
	case UIEVENT_SETFOCUS:
		 uMsg = WM_SETFOCUS;
		 break;
	case UIEVENT_SETCURSOR:
		 uMsg = WM_SETCURSOR;
		 break;

	case UIEVENT_CONTEXTMENU:
		 uMsg = WM_CONTEXTMENU;
		 break;

//	case UIEVENT_WINDOWSIZE:
//		 uMsg = WM_SIZE;
//		 break;

	case UIEVENT_TIMER:
		 uMsg = WM_TIMER;
		 break;
	case UIEVENT_NOTIFY:
		 uMsg = WM_NOTIFY;
		 break;
	case UIEVENT_COMMAND:
		 uMsg = WM_COMMAND;
		 break;
	case UIEVENT_LINKNOTIFY:
		 uMsg = WM_NOTIFY;
		 break;
	default:
		break;
	}
	if ((uMsg > 0) && m_pWindowlessRE && bValid)
	{
		m_pWindowlessRE->TxWindowProc(GetManager()->GetPaintWindow(), uMsg, e.wParam, e.lParam);
	} else
		CContainerUI::Event(e);
}

void CRichEditUI::Notify(TNotifyUI& msg)
{
	int iPos = GetScrollPos(UISB_VERT);
    if (msg.sType == _T("lineup"))
	{
		PerformMessage(WM_VSCROLL, (SB_LINEUP & 0x0000FFFF), 0);
		Invalidate();
	} else if (msg.sType == _T("linedown"))
	{
		PerformMessage(WM_VSCROLL, (SB_LINEDOWN & 0x0000FFFF), 0);
		Invalidate();
	} else if (msg.sType == _T("pageup"))
	{
		PerformMessage(WM_VSCROLL, (SB_PAGEUP & 0x0000FFFF), 0);
		Invalidate();
	} else if (msg.sType == _T("pagedown"))
	{
		PerformMessage(WM_VSCROLL, (SB_PAGEDOWN & 0x0000FFFF), 0);
		Invalidate();
	} else if (msg.sType == _T("thumbtrack"))
	{
		int iPos = (int)msg.wParam;
		if (iPos < 0) 
			iPos = 0;
		DWORD dwHigh = ((DWORD)iPos << 16 ) & 0xFFFF0000;
		PerformMessage(WM_VSCROLL, (SB_THUMBPOSITION & 0x0000FFFF) | dwHigh, 0);
		Invalidate();
	} else if (msg.sType == _T("visibilitychanged"))
	{
		//
	} else if (msg.sType == _T("setfont"))
	{
		CCharFontStyle *pStyle = (CCharFontStyle *) msg.lParam;
		CHARFORMAT_RE cfFormat;
		memset(&cfFormat, 0, sizeof(cfFormat));
		CharFontStyleToFormat(*pStyle, cfFormat);
		PerformMessage(EM_SETCHARFORMAT, SCF_DEFAULT, LPARAM(&cfFormat));
	} else
	{
		CContainerUI::Notify(msg);
	}
}

void CRichEditUI::SetPos(RECT rc)
{	
	if (!::EqualRect(&m_rcItem, &rc) && m_pWindowlessRE)
	{
		CControlUI::SetPos( rc );

		if (m_pVScrollBar)
		{
			//position the scrollbar no matter if it is visible
			RECT rcScrollBar = rc;
			rcScrollBar.left = rcScrollBar.right - SB_WIDTH;
			m_pVScrollBar->SetPos( rcScrollBar );
		}
		//text edit area, it depends on if the scrollbar is visible.
		RECT rcClient = rc;
		if (IsScrollBarVisible(UISB_VERT))
		{
			rcClient.right -= SB_WIDTH;	
		}
		m_pWindowlessRE->SetClientRect(&rcClient, TRUE);
		//set active rect
		RECT rcCtrl = { 0 };
		m_pWindowlessRE->GetControlRect(&rcCtrl);
		m_pWindowlessRE->OnTxInPlaceActivate(&rcCtrl);

		//scroll page size.
		//force the interface RE_SetScrollRange is called, and we can
		//get the scroll range
		SetScrollPage(UISB_VERT, rcCtrl.bottom - rcCtrl.top);
	}
}

void CRichEditUI::InvalidateRE(LPCRECT lprc, BOOL bMode)
{
	//::InvalidateRect(GetManager()->GetPaintWindow(), lprc, bMode);
	/*if (m_pWindowlessRE)
	{   
		m_pWindowlessRE->PaintRE(GetManager()->GetPaintDC(), (LPRECT)lprc, FALSE, FALSE); 
		//m_pWindowlessRE->TxWindowProc( WM_PAINT, 
		//	(WPARAM), (LPARAM)(lprc));
		//::InvalidateRect(GetManager()->GetPaintWindow(), (LPRECT) lprc, bMode);
	} */
	if (lprc)
		::InvalidateRect(GetManager()->GetPaintWindow(), (LPRECT) lprc, bMode);
	else
		::InvalidateRect(GetManager()->GetPaintWindow(), &m_rcItem, bMode);
}

void CRichEditUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	RECT rcTemp = { 0 };
    if( !::IntersectRect(&rcTemp, &rcPaint, &m_rcItem) ) 
		return;

    CRenderClip clip;
    CBlueRenderEngineUI::GenerateClip(hDC, rcTemp, clip);
   // CControlUI::DoPaint(hDC, rcPaint);

    if( m_pWindowlessRE ) {
         
        // Remember wparam is actually the hdc and lparam is the update
        // rect because this message has been preprocessed by the window.
		m_pWindowlessRE->PaintRE(hDC, (LPRECT) &rcTemp, FALSE, FALSE);
           	        // What view of the object
         	//scrollbar
		if (IsScrollBarVisible(UISB_VERT))
		{
			m_pVScrollBar->DoPaint(hDC, rcPaint);
		}
    
    }
	//border
/*	if (HasBorder())
	{
		UINT uState = 0;
		if (IsFocused()) 
			uState |= UISTATE_FOCUSED;
		//暂时不考虑控件的状态，如focus，disable等
		CBlueRenderEngineUI::DoPaintRectangle(hDC, m_rcItem, m_clrBorder, m_clrBkgnd);
	}

	//scrollbar
	if (IsScrollBarVisible(UISB_VERT))
	{
		m_pVScrollBar->DoPaint(hDC, rcPaint);
	}
	//content
	if (m_pWindowlessRE)
	{
		//protect rcPaint from being modified by the procedure
		RECT rc = rcPaint;
		m_pWindowlessRE->TxWindowProc(GetManager()->GetPaintWindow(), WM_PAINT, (WPARAM)hDC, (LPARAM)&rc);
	} */
}

SIZE CRichEditUI::EstimateSize(SIZE szAvailable)
{
	return szAvailable;
}

#ifdef __USER_QQ_IMAGEOLE___
//使用QQ imageole.dll
void CRichEditUI::InsertGif(const char *lpszFileName, HBITMAP hBitmap, const char *szTag, const int nPos)
{
	LPLOCKBYTES  lpLockBytes  =  NULL; 
    SCODE  sc; 
    HRESULT  hr;                     
    //print  to  RichEdit'  s  IClientSite 
    LPOLECLIENTSITE  m_lpClientSite; 
    //A  smart  point  to  IAnimator 
    GoComImageOle::IGifAnimatorPtr  m_lpAnimator; 

    //ptr  2  storage                     
    LPSTORAGE  m_lpStorage; 
    //the  object  2  b  insert  2 
    LPOLEOBJECT            m_lpObject; 
    //Create  lockbytes 
    sc  =  ::CreateILockBytesOnHGlobal(NULL,  TRUE,  &lpLockBytes); 
    if  (sc  !=  S_OK) 
        return ;
    ASSERT(lpLockBytes  !=  NULL); 
    //use  lockbytes  to  create  storage 
    sc  =  ::StgCreateDocfileOnILockBytes(lpLockBytes, 
                STGM_SHARE_EXCLUSIVE  |STGM_CREATE  |STGM_READWRITE,  0,  &m_lpStorage); 
    if  (sc  !=  S_OK) 
    {  
		lpLockBytes  =  NULL;   
    } 
    ASSERT(m_lpStorage  !=  NULL); 
    //get  the  ClientSite  of  the  very  RichEditCtrl 
    m_pRichEditOle->GetClientSite(&m_lpClientSite); 
    ASSERT(m_lpClientSite  !=  NULL); 
    try 
	{ 
        //Initlize  COM  interface 
           
        hr  =  ::CoInitialize(NULL)  ;//(  NULL,  COINIT_APARTMENTTHREADED  ); 
        if (FAILED(hr)) 
            _com_issue_error(hr); 
           
        //Get  GifAnimator  object 
        //here,  I  used  a  smart  point,  so  I  do  not  need  to  free  it 
        hr  =  m_lpAnimator.CreateInstance(GoComImageOle::CLSID_GifAnimator);             
        if (FAILED(hr)) 
            _com_issue_error(hr); 
        //COM  operation  need  BSTR,  so  get  a  BSTR  
		if (lpszFileName)
		{
#ifdef _UNICODE
		    TCHAR szTemp[MAX_PATH] = {0};
		    CStringConversion::StringToWideChar(lpszFileName, szTemp, MAX_PATH);
		    BSTR path = SysAllocString(szTemp);
#endif
		    //Load the gif
		    hr = m_lpAnimator->LoadFromFile(path);			
		    SysFreeString(path);
		} 
        if (FAILED(hr)) 
			_com_issue_error(hr);  
           
        //get  the  IOleObject 
        hr  =  m_lpAnimator.QueryInterface(IID_IOleObject,  (void**)&m_lpObject); 
        if (FAILED(hr)) 
			_com_issue_error(hr); 
           
        //Set  it  2  b  inserted 
        OleSetContainedObject(m_lpObject,  TRUE); 
        //获取编号
		DWORD dwUserId = 0;
		m_OleLock.Lock();
        m_dwOleSeq ++;
		dwUserId = m_dwOleSeq;
		LPOLE_ITEM pItem = new OLE_ITEM();
		memset(pItem, 0, sizeof(OLE_ITEM));
		strncpy(pItem->szFlag, szTag, MAX_OLE_FLAG_SIZE - 1);
		if (lpszFileName)
			strncpy(pItem->szFileName, lpszFileName, MAX_PATH - 1);
		m_OleList.insert(std::pair<DWORD, LPOLE_ITEM>(dwUserId, pItem));
		m_OleLock.UnLock();  

        //2  insert  in  2  richedit,  you  need  a  struct  of  REOBJECT 
        REOBJECT  reobject; 
        ZeroMemory(&reobject,  sizeof(REOBJECT)); 

        reobject.cbStruct  =  sizeof(REOBJECT);             
        CLSID  clsid; 
        sc  =  m_lpObject->GetUserClassID(&clsid); 
        if  (sc  !=  S_OK) 
            return ;
        //set  clsid 
        reobject.clsid  =  clsid; 
        //can  be  selected 
        reobject.cp  =  REO_CP_SELECTION; 
        //content,  but  not  static 
        reobject.dvaspect  =  DVASPECT_CONTENT; 
        //goes  in  the  same  line  of  text  line 
        reobject.dwFlags  =  REO_BELOWBASELINE | REO_RESIZABLE;
        reobject.dwUser  =  dwUserId; 
        //the  very  object 
        reobject.poleobj  =  m_lpObject; 
        //client  site  contain  the  object 
        reobject.polesite  =  m_lpClientSite; 
        //the  storage   
        reobject.pstg  =  m_lpStorage; 
           
        SIZEL  sizel; 
        sizel.cx  =  sizel.cy  =  0; 
        reobject.sizel  =  sizel; 
		HWND  hWndRT  = ::GetParent(GetManager()->GetPaintWindow()); 
        int nInsertPos = nPos;
		if (nPos == 0)
			nInsertPos = GetCount();
		CCharRange crg;
		if (nInsertPos >= 0)
		{
			crg.dwMax = nInsertPos + 1;
			crg.dwMin = nInsertPos;
			PerformMessage(EM_EXSETSEL, 0, LPARAM(&crg));
		}
		m_pRichEditOle->InsertObject(&reobject);  
        VARIANT_BOOL  ret; 
        //do  frame  changing 
        ret  =  m_lpAnimator->TriggerFrameChange(); 
		ScrollToBottom();
        //show  it 
        m_lpObject->DoVerb(OLEIVERB_UIACTIVATE,  NULL,  m_lpClientSite,  0,  hWndRT,  NULL); 
        m_lpObject->DoVerb(OLEIVERB_SHOW,  NULL,  m_lpClientSite,  0,   hWndRT,  NULL); 
           
        //redraw  the  window  to  show  animation 
        RedrawWindow(hWndRT, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME 
			| RDW_ERASENOW | RDW_ALLCHILDREN);  //根据MFC 定义
		ScrollToBottom();
        //m_lpAnimator->re
        if  (m_lpClientSite) 
        { 
			m_lpClientSite->Release();
			m_lpClientSite  =  NULL; 
        } 
        if  (m_lpObject) 
        { 
			m_lpObject->Release();
			m_lpObject  =  NULL; 
        } 
        if  (m_lpStorage) 
        { 
			m_lpStorage->Release(); 
			m_lpStorage  =  NULL; 
        }  
    } 	catch(  _com_error  e  ) 
    {  
		::CoUninitialize();             
    } 
}*

#else
//RichEdit应用相关 baseolectrl.dll
void CRichEditUI::InsertGif(const char *lpszFileName, HBITMAP hBitmap, const char *szTag, const int nPos)
{
	int nInsertPos = nPos;
	LPLOCKBYTES lpLockBytes = NULL;
	SCODE sc;
	HRESULT hr;
	//print to RichEdit' s IClientSite
	LPOLECLIENTSITE m_lpClientSite;
 
	IGifAnimatePtr pControl;
 
	LPSTORAGE m_lpStorage;
	//the object 2 b insert 2
	LPOLEOBJECT	m_lpObject;

	//Create lockbytes
	sc = ::CreateILockBytesOnHGlobal(NULL, TRUE, &lpLockBytes);
	if (sc != S_OK)
		return;
	ASSERT(lpLockBytes != NULL);
	
	//use lockbytes to create storage
	sc = ::StgCreateDocfileOnILockBytes(lpLockBytes,STGM_SHARE_EXCLUSIVE|STGM_CREATE|STGM_READWRITE, 0, &m_lpStorage);
	if (sc != S_OK)
	{
		lpLockBytes->Release();
		lpLockBytes = NULL;
		return ;
	}
	ASSERT(m_lpStorage != NULL);
	
	//get the ClientSite of the very RichEditCtrl
	m_pRichEditOle->GetClientSite(&m_lpClientSite);
	ASSERT(m_lpClientSite != NULL);

	try
	{
		//Initlize COM interface
		hr = ::CoInitialize( NULL);
		if (FAILED(hr))
			return;
		

		hr = pControl.CreateInstance(CLSID_GifAnimate);
		if (FAILED(hr))
				return;
		//COM operation need BSTR, so get a BSTR
		if (lpszFileName)
		{
#ifdef _UNICODE
		    TCHAR szTemp[MAX_PATH] = {0};
		    CStringConversion::StringToWideChar(lpszFileName, szTemp, MAX_PATH);
		    BSTR path = SysAllocString(szTemp);
#endif
		    //Load the gif
		    pControl->LoadFile(path, (ULONG)GetManager()->GetPaintWindow());			
		    SysFreeString(path);
		} else if (hBitmap)
		{
			pControl->LoadBitmap((ULONG)hBitmap, (ULONG)GetManager()->GetPaintWindow());
		}
		if (FAILED(hr))
			return;
			
		//get the IOleObject
		hr = pControl->QueryInterface(IID_IOleObject, (void **)&m_lpObject);

		if (FAILED(hr))
			return;
		

		OleSetContainedObject(m_lpObject, TRUE);
		

		//获取编号
		DWORD dwUserId = 0;
		m_OleLock.Lock();
        m_dwOleSeq ++;
		dwUserId = m_dwOleSeq;
		LPOLE_ITEM pItem = new OLE_ITEM();
		memset(pItem, 0, sizeof(OLE_ITEM));
		strncpy(pItem->szFlag, szTag, MAX_OLE_FLAG_SIZE - 1);
		if (lpszFileName)
			strncpy(pItem->szFileName, lpszFileName, MAX_PATH - 1);
		m_OleList.insert(std::pair<DWORD, LPOLE_ITEM>(dwUserId, pItem));
		m_OleLock.UnLock();

		REOBJECT reobject;
		ZeroMemory(&reobject, sizeof(REOBJECT));

		reobject.cbStruct = sizeof(REOBJECT);	
		CLSID clsid;
		sc = m_lpObject->GetUserClassID(&clsid);
		if (sc != S_OK)
			return ;
		m_lpObject->SetClientSite(m_lpClientSite);
		//set clsid
		reobject.clsid = clsid;
		//can be selected
		reobject.cp = REO_CP_SELECTION;
		//content, but not static
		reobject.dvaspect = DVASPECT_CONTENT;
		//goes in the same line of text line
		reobject.dwFlags = REO_BELOWBASELINE; //REO_RESIZABLE |
		reobject.dwUser = dwUserId;
		//the very object
		reobject.poleobj = m_lpObject;
		//client site contain the object
		reobject.polesite = m_lpClientSite;
		//the storage 
		reobject.pstg = m_lpStorage;
		
		SIZEL sizel;
		sizel.cx = sizel.cy = 0;
		reobject.sizel = sizel;

		if (nPos == 0)
			nInsertPos = GetCount();
		CCharRange crg;
		if (nInsertPos >= 0)
		{
			crg.dwMax = nInsertPos + 1;
			crg.dwMin = nInsertPos;
			PerformMessage(EM_EXSETSEL, 0, LPARAM(&crg));
		}
		m_pRichEditOle->InsertObject(&reobject);
		ScrollToBottom();
        pControl->Play(); 
		//m_lpObject->DoVerb(OLEIVERB_UIACTIVATE, NULL, m_lpClientSite, 0, GetManager()->GetPaintWindow(), NULL);
		//m_lpObject->DoVerb(OLEIVERB_SHOW, NULL, m_lpClientSite, 0, GetManager()->GetPaintWindow(), NULL);
        //SetActiveWindow( GetManager()->GetPaintWindow() );

		//取消当前的选择
		if (nInsertPos >= 0)
		{
			crg.dwMin = nInsertPos + 1;
			crg.dwMax = crg.dwMin;
			PerformMessage(EM_EXSETSEL, 0, LPARAM(&crg));
		}
 		if (m_lpClientSite)
		{
			m_lpClientSite->Release();
			m_lpClientSite = NULL;
		}
		if (m_lpObject)
		{
			m_lpObject->Release();
			m_lpObject = NULL;
		}
		if (m_lpStorage)
		{
			m_lpStorage->Release();
			m_lpStorage = NULL;
		}

	}
	catch( _com_error e )
	{
		::CoUninitialize();	
	}
}
#endif

//替换一个图片
void CRichEditUI::ReplaceOleObj(const char *szTag, const char *szNewFileName)
{
	//查找
	std::map<DWORD, DWORD> dwFlagList; //表情列表
	m_OleLock.Lock();
	std::map<DWORD, LPOLE_ITEM>::iterator it;
	for (it = m_OleList.begin(); it != m_OleList.end();)
	{
		if (strcmp((*it).second->szFlag, szTag) == 0)
		{
			dwFlagList.insert(std::pair<DWORD, DWORD>((*it).first, (*it).first));
			delete (*it).second;
			it = m_OleList.erase(it);
		} else
			it ++;
	}
	m_OleLock.UnLock();
	if (dwFlagList.size() > 0)
	{
		if (m_pRichEditOle)
		{
			std::map<DWORD, DWORD>::iterator it;
			for (int i = 0; i < m_pRichEditOle->GetObjectCount(); i ++)
			{
				REOBJECT reObj;
				memset(&reObj, 0, sizeof(REOBJECT));
				reObj.cbStruct = sizeof(REOBJECT);
				m_pRichEditOle->GetObject(i, &reObj, REO_GETOBJ_POLEOBJ); 
				it = dwFlagList.find(reObj.dwUser);
				if (it != dwFlagList.end())
				{
					//替换掉
					InsertGif(szNewFileName, NULL, szTag, reObj.cp);
				} //
			} //end for
		} // if (m_pRichEditOle
	} //end if (dwUser > 0)
}

void CRichEditUI::InsertBitmap(HBITMAP hBitmap)
{
	SCODE sc;

	// Get the image data object
	//
	CImageDataObject *pods = new CImageDataObject;
	LPDATAOBJECT lpDataObject;
	pods->QueryInterface(IID_IDataObject, (void **)&lpDataObject);

	pods->SetBitmap(hBitmap);

	// Get the RichEdit container site
	//
	IOleClientSite *pOleClientSite = NULL;	
	sc = m_pRichEditOle->GetClientSite(&pOleClientSite);
	if (sc != S_OK)
		return ;

	// Initialize a Storage Object
	//
	IStorage *pStorage;	

	LPLOCKBYTES lpLockBytes = NULL;
	sc = ::CreateILockBytesOnHGlobal(NULL, TRUE, &lpLockBytes);
	if (sc != S_OK)
		return; 
	
	sc = ::StgCreateDocfileOnILockBytes(lpLockBytes,
		STGM_SHARE_EXCLUSIVE|STGM_CREATE|STGM_READWRITE, 0, &pStorage);
	if (sc != S_OK)
	{
		lpLockBytes->Release();
		lpLockBytes = NULL;
	} 

	// The final ole object which will be inserted in the richedit control
	//
	IOleObject *pOleObject = NULL; 
	pOleObject = pods->GetOleObject(pOleClientSite, pStorage);

	if (pOleObject == NULL)
		return ;
	// all items are "contained" -- this makes our reference to this object
	//  weak -- which is needed for links to embedding silent update.
	OleSetContainedObject(pOleObject, TRUE);

	// Now Add the object to the RichEdit 
	//
	REOBJECT reobject;
	ZeroMemory(&reobject, sizeof(REOBJECT));
	reobject.cbStruct = sizeof(REOBJECT);
	
	CLSID clsid = {0};
	sc = pOleObject->GetUserClassID(&clsid);
	if (sc != S_OK)
		return;

	reobject.clsid = clsid;
	reobject.cp = REO_CP_SELECTION;
	//reobject.dvaspect = DVASPECT_CONTENT;
	reobject.dvaspect = DVASPECT_CONTENT;
	reobject.dwFlags = REO_RESIZABLE ;

	reobject.poleobj = pOleObject;
	reobject.polesite = pOleClientSite;
	reobject.pstg = pStorage;

	// Insert the bitmap at the current location in the richedit control
	//
	m_pRichEditOle->InsertObject(&reobject);

	// Release all unnecessary interfaces
	//
	pOleObject->Release();
	pOleClientSite->Release();
	pStorage->Release();
	lpDataObject->Release();
}

void CRichEditUI::InsertImage(char *lpszFileName, char *szTag, DWORD dwPos)
{
	SCODE sc;

	// Get the image data object
	CImageDataObject *pods = new CImageDataObject();
	LPDATAOBJECT lpDataObject;
	pods->QueryInterface(IID_IDataObject, (void **)&lpDataObject);

	pods->SetImageFileName(lpszFileName);

	// Get the RichEdit container site
	//
	IOleClientSite *pOleClientSite;	
	m_pRichEditOle->GetClientSite(&pOleClientSite);

	// Initialize a Storage Object
	//
	IStorage *pStorage;	

	LPLOCKBYTES lpLockBytes = NULL;
	sc = ::CreateILockBytesOnHGlobal(NULL, TRUE, &lpLockBytes);
	if (sc != S_OK)
		return ;

	
	sc = ::StgCreateDocfileOnILockBytes(lpLockBytes,STGM_SHARE_EXCLUSIVE|STGM_CREATE|STGM_READWRITE, 0, &pStorage);
	if (sc != S_OK)
	{
		lpLockBytes->Release();
		lpLockBytes = NULL;
		return;
	}

	// The final ole object which will be inserted in the richedit control
	//
	IOleObject *pOleObject; 
	pOleObject = pods->GetOleObject(pOleClientSite, pStorage);

	// all items are "contained" -- this makes our reference to this object
	//  weak -- which is needed for links to embedding silent update.
	OleSetContainedObject(pOleObject, TRUE);

	// Now Add the object to the RichEdit 
	//
	REOBJECT reobject;
	ZeroMemory(&reobject, sizeof(REOBJECT));
	reobject.cbStruct = sizeof(REOBJECT);
	
	CLSID clsid;
	sc = pOleObject->GetUserClassID(&clsid);
	if (sc != S_OK)
		return;

	reobject.clsid = clsid;
	reobject.cp = REO_CP_SELECTION;
	reobject.dvaspect = DVASPECT_CONTENT;
	reobject.poleobj = pOleObject;
	reobject.polesite = pOleClientSite;
	reobject.pstg = pStorage;

	// Insert the bitmap at the current location in the richedit control
	//
	m_pRichEditOle->InsertObject(&reobject);

	// Release all unnecessary interfaces
	//
	pOleObject->Release();
	pOleClientSite->Release();
	pStorage->Release();
	lpDataObject->Release();	
}

BOOL CRichEditUI::InsertText(CHARFORMAT_RE &cfFormat, DWORD dwOffset, char *szText)
{
	CCharRange range;
	BOOL b = TRUE;
	int size = (int)strlen(szText);
	SetLeftIndent(dwOffset);
	range.dwMin = GetCount();
	range.dwMax = range.dwMin;
	PerformMessage(EM_EXSETSEL, 0, LPARAM(&range));
	PerformMessage(EM_SETCHARFORMAT, SCF_SELECTION, LPARAM(&cfFormat));
#ifdef _UNICODE
	TCHAR *szwText = new TCHAR[size + 3];
	memset(szwText, 0, sizeof(TCHAR) * (size + 3));
	CStringConversion::StringToWideChar(szText, szwText, size + 3);
	PerformMessage(EM_REPLACESEL, 0, LPARAM(szwText));
	delete []szwText;
#else
	::SendMessage(m_hWnd, EM_REPLACESEL, 0, LPARAM(szText));
#endif

	return b;
}

BOOL CRichEditUI::InsertUnicodeText(CHARFORMAT_RE &cfFormat, DWORD dwOffset, TCHAR *szText)
{
	CCharRange range;
	BOOL b = TRUE;
	SetLeftIndent(dwOffset);
	range.dwMin = GetCount();
	range.dwMax = range.dwMin;
	PerformMessage(EM_EXSETSEL, 0, LPARAM(&range));
	PerformMessage(EM_SETCHARFORMAT, SCF_SELECTION, LPARAM(&cfFormat));
	PerformMessage(EM_REPLACESEL, 0, LPARAM(szText));
	return b;
}

//获取选择的状态
RE_SELECT_STATUS CRichEditUI::GetSelected()
{
	CCharRange rg = {0};
	PerformMessage(EM_EXGETSEL, 0, LPARAM(&rg));
	if (rg.dwMax > rg.dwMin)
	{
		//查找
		if ((rg.dwMax - rg.dwMin) == 1)
		{
			bool b = false;
            DWORD dwUserId = GetUserIdByPosition(rg.dwMin);
			m_OleLock.Lock();
			std::map<DWORD, LPOLE_ITEM>::iterator it = m_OleList.find(dwUserId);
			if (it != m_OleList.end())
			{
				b = true;
			}
			m_OleLock.UnLock();
			if (b)
				return RE_SELECT_STATUS_PICTURE;
		}
		return RE_SELECT_STATUS_TEXT;
	}
	return RE_SELECT_STATUS_EMPTY;
}

//全选
void CRichEditUI::SelectAll()
{
	PerformMessage(EM_SETSEL, 0, -1);
}

//nStatus = 0 字符串状态，有结果 1为下一个为ole字符串 2为获取到ole,3为结束状态，没有获取结果
const char * GetNextOleString(const char *p, char *szText, int &nStatus)
{
	const char *pStart = p;
	int nPrevStatus = nStatus;
	nStatus = 3;
	while (*p)
	{
		if (*p == '/')
		{
			p ++;
			if (*p)
			{
				if (((nPrevStatus == 0) || (nPrevStatus == 2)) && (*p == '{'))
				{
					//拷贝以前的串
					strncpy(szText, pStart, p - pStart - 1);
					p ++;
					nStatus = 1;
					break;
				} else if ((nPrevStatus == 1) && (*p == '}'))
				{
					strncpy(szText, pStart, p - pStart - 1);
					p ++;
					nStatus = 2;
					break;
				} else
					nPrevStatus = 0;
			}
			continue;
		}
		p ++;
	}
	if ((!(*p)) && (p > pStart))
	{
		strncpy(szText, pStart, p - pStart);
		nStatus = 0;
	}
	return p;
}

//插入一段Ole字符串，包括图片数据
BOOL CRichEditUI::InsertOleText(CHARFORMAT_RE &cfFormat, DWORD dwOffset, const char *szText, BOOL bBullet) 
{
	//设置边界 
	SetLeftIndent(dwOffset, bBullet);
	int nTextLen = (int)::strlen(szText);
	BOOL bAppendLN = FALSE;
	if (nTextLen > 0)
	{
		//#13#10  测试一下是否有回车符
		if ((szText[nTextLen - 1] != 13) && (szText[nTextLen - 1] != 10))
			bAppendLN = TRUE;
	}
    //设置字体
	const char *p = szText;
	int nLen = nTextLen + 1;
	char *szTemp = new char[nLen];
	int nStatus = 0;
	while(true)
	{
		memset(szTemp, 0, nLen);
		p = GetNextOleString(p, szTemp, nStatus);
		switch(nStatus)
		{
			case 0:
			case 1:
				{
					TCHAR *szwTmp = new TCHAR[nLen + 1];
					memset(szwTmp, 0, (nLen + 1) * sizeof(TCHAR));
					CStringConversion::StringToWideChar(szTemp, szwTmp, nLen);
					Append(cfFormat, szwTmp);
					break;
				}
			case 2:
				{
					//ole 链接
					if (m_pApp)
					{
						char szFileName[MAX_PATH] = {0};
						if (m_pApp->GetCustomPicFile(szFileName, szTemp))
						{
							InsertGif(szFileName, NULL,  szTemp, 0);
						}
					} else if (m_pCallBack)
					{
						char szFileName[MAX_PATH] = {0};
						DWORD nNameSize = MAX_PATH - 1;
						DWORD dwTag = 35;
						BOOL bSucc = FALSE;
						if (m_pManager)
							bSucc = m_pCallBack(m_pManager->GetPaintWindow(), RICHEDIT_EVENT_GETCUSTOMPIC, szFileName, &nNameSize, 
							            szTemp, &dwTag, 0, m_lpOverlapped);
						else
							bSucc = m_pCallBack(NULL, RICHEDIT_EVENT_GETCUSTOMPIC, szFileName, &nNameSize, 
							            szTemp, &dwTag, 0, m_lpOverlapped);
						if (bSucc)
						{
							InsertGif(szFileName, NULL, szTemp, 0);
						} //end if (m_pCallBack...
					} //end else if (m_pCallBack)
				} //end case 2
		} //end switch(...
		if (nStatus == 3)
			break;
	} //end while(true)
	
    if (bAppendLN)
		Append(L"\n"); 
	return TRUE;
}

void CRichEditUI::Append(const TCHAR *szText)
{
	CCharRange range;
	BOOL b = TRUE;
	range.dwMin = GetCount();
	range.dwMax = range.dwMin;
	PerformMessage(EM_EXSETSEL, 0, LPARAM(&range));
#ifdef _UNICODE
	PerformMessage(EM_REPLACESEL, 0, LPARAM(szText));
#else
	PerformMessage(EM_REPLACESEL, 0, LPARAM(szText));
#endif
}

//追加内容
void CRichEditUI::Append(CHARFORMAT_RE &cfFormat, const TCHAR *szText) 
{
	CCharRange range;
	BOOL b = TRUE;
	range.dwMin = GetCount();
	range.dwMax = range.dwMin;
	PerformMessage(EM_EXSETSEL, 0, LPARAM(&range));
#ifdef _UNICODE
	PerformMessage(EM_SETCHARFORMAT, SCF_SELECTION, LPARAM(&cfFormat));
	PerformMessage(EM_REPLACESEL, 0, LPARAM(szText));
#else
	PerformMessage(EM_REPLACESEL, 0, LPARAM(szText));
#endif
}

//
void CRichEditUI::AppendAckLink(CCharRange &cr)
{
	CHARFORMAT_RE cfFormat;
	m_fcDefaultStyle.cfColor = 0;
	CharFontStyleToFormat(m_fcDefaultStyle, cfFormat);
	Append(cfFormat, L"本条信息需要确认");
	CCharRange range;
	BOOL b = TRUE;
	range.dwMin = GetCount();
	range.dwMax = range.dwMin;
	PerformMessage(EM_EXSETSEL, 0, LPARAM(&range));
#ifdef _UNICODE
	int  size = (int)::lstrlen(L"回执");
	PerformMessage(EM_REPLACESEL, 0, LPARAM(L"回执"));
#else
	PerformMessage(EM_REPLACESEL, 0, LPARAM(szText));
#endif	
	range.dwMax = range.dwMin + size;

	PerformMessage(EM_EXSETSEL, 0, LPARAM(&range));
	CHARFORMAT_RE param;
	memset(&param, 0, sizeof(param));
	param.cbSize = sizeof(param);
	param.dwMask = CFM_LINK;
	param.dwEffects = CFE_LINK;
	PerformMessage(EM_SETCHARFORMAT, SCF_SELECTION, LPARAM(&param));
	//加入到列表当中
	m_LinkLock.Lock(); 
	m_AckLinkList.insert(pair<DWORD, CCharRange>(range.dwMin, cr));
	m_LinkLock.UnLock();
	Append(L"\n");
}

void CRichEditUI::AppendCustomLink(const TCHAR *szText, DWORD dwFlag)
{
	if (dwFlag == 0)
	{
		CancelCustomLink(0);
	}
	CCharRange range;
	BOOL b = TRUE;
	range.dwMin = GetCount();
	range.dwMax = range.dwMin;
	PerformMessage(EM_EXSETSEL, 0, LPARAM(&range));
#ifdef _UNICODE
	int  size = (int)::lstrlen(szText);
	PerformMessage(EM_REPLACESEL, 0, LPARAM(szText));
#else
	PerformMessage(EM_REPLACESEL, 0, LPARAM(szText));
#endif	
	range.dwMax = range.dwMin + size;

	PerformMessage(EM_EXSETSEL, 0, LPARAM(&range));
	CHARFORMAT_RE param;
	memset(&param, 0, sizeof(param));
	param.cbSize = sizeof(param);
	param.dwMask = CFM_LINK;
	param.dwEffects = CFE_LINK;
	PerformMessage(EM_SETCHARFORMAT, SCF_SELECTION, LPARAM(&param));
	//加入到列表当中
	m_LinkLock.Lock();
	LPCustomLinkItem pItem = new CCustomLinkItem();
	memset(pItem, 0, sizeof(CCustomLinkItem));
	pItem->crg.dwMax = range.dwMax;
	pItem->crg.dwMin = range.dwMin;
	pItem->dwFlag = dwFlag;
	m_CustomLinkList.insert(pair<DWORD, LPCustomLinkItem>(range.dwMin, pItem));
	m_LinkLock.UnLock();
}


//取消一个自定义链接
void CRichEditUI::CancelCustomLink(DWORD dwFlag)
{
	CCharRange crg = {0};
	m_LinkLock.Lock();
	std::map<DWORD, LPCustomLinkItem>::iterator it;
	for (it = m_CustomLinkList.begin(); it != m_CustomLinkList.end(); it ++)
	{
		if ((*it).second->dwFlag == dwFlag)
		{
			crg = (*it).second->crg;
			delete (*it).second;
			m_CustomLinkList.erase(it);
			break;
		}
	}
	m_LinkLock.UnLock();
	if ((crg.dwMax != 0) || (crg.dwMin != 0))
	{
		PerformMessage(EM_EXSETSEL, 0, LPARAM(&crg));
		CHARFORMAT_RE param;
		memset(&param, 0, sizeof(param));
		param.cbSize = sizeof(param);
		param.dwMask = CFM_LINK;
		param.dwEffects = 0;
		PerformMessage(EM_SETCHARFORMAT, SCF_SELECTION, LPARAM(&param));
	}
}


//====   szText Format ====
//  传文件, <接受,1>,<取消,2>,<另存为,3>
BOOL CRichEditUI::InsertTip(CCharFontStyle *cfStyle, DWORD dwOffset, const TCHAR *szText)
{
	m_strLastName.clear();
	CHARFORMAT_RE cfFormat;
	SetLeftIndent(dwOffset);
	if (cfStyle)
		CharFontStyleToFormat(*cfStyle, cfFormat);
	else
	{
		CCharFontStyle cfDefault = {0};
		cfDefault.cfColor = 0x0F0F0F;
		cfDefault.nFontSize = 8;
		::lstrcpy(cfDefault.szFaceName, L"Tohoma");
		CharFontStyleToFormat(cfDefault, cfFormat);
	}
	if (dwOffset == 0)
		dwOffset = DEFAULT_INDENT_CONTENT;
    CTipParser parser(szText);
	size_t size = ::lstrlen(szText) + 1;
	TCHAR *szDest = new TCHAR[size];
	TCHAR *pLink = NULL;
	int nInitToken = CTipParser::TIP_PARSER_TEXT;
	int nNextToken;
	Append(cfFormat, L"\n");
	char szFileName[MAX_PATH] = {0};
	if (m_pApp) 
	{
		if (m_pApp->GetTipPicFileName(szFileName))
			InsertGif(szFileName, NULL, "tippic", 0);
		else if (m_nTipBitmapId > 0)
		{
			LPUI_IMAGE_ITEM pItem = NULL;
			if ((m_pManager->GetImage(m_nTipBitmapId, &pItem)) && (!pItem->m_strFileName.empty()))
			{
				InsertGif(pItem->m_strFileName.c_str(), NULL, "tippic", 0);
			}
		}
	} else if (m_pCallBack)
	{
		DWORD nNameSize = MAX_PATH - 1;
		BOOL bSucc = FALSE;
		if (m_pManager)
			bSucc = m_pCallBack(m_pManager->GetPaintWindow(), RICHEDIT_EVENT_GETTIPPIC, szFileName, &nNameSize, NULL, NULL, 0, m_lpOverlapped);
		else
			bSucc = m_pCallBack(NULL, RICHEDIT_EVENT_GETTIPPIC, szFileName, &nNameSize, NULL, NULL, 0, m_lpOverlapped);
		if (bSucc)
			InsertGif(szFileName, NULL, "tippic", 0);
		else if (m_nTipBitmapId > 0)
		{
			LPUI_IMAGE_ITEM pItem;
			if ((m_pManager->GetImage(m_nTipBitmapId, &pItem)) && (!pItem->m_strFileName.empty()))
			{
				InsertGif(pItem->m_strFileName.c_str(), NULL, "tippic", 0);
			} //end if ((m_pManager->
		} //end else if (m_nTipBimapId > 0)
	} //end else if (m_pCallBack)
	while(true)
	{
		memset(szDest, 0, size * sizeof(TCHAR));
		if (!parser.GetNextToken(szDest, nInitToken, nNextToken))
			break;
		switch(nInitToken)
		{
		case CTipParser::TIP_PARSER_TEXT:
			 Append(cfFormat, szDest);
			 break;
		case CTipParser::TIP_PARSER_LINK:
			{
				int nSize = (int) ::lstrlen(szDest);
				if (pLink)
					delete []pLink;
				pLink = NULL;
				if (nSize > 0)
				{
					pLink = new TCHAR[nSize + 1];
					memset(pLink, 0, (nSize + 1) * sizeof(TCHAR));
					::lstrcpyn(pLink, szDest, nSize + 1);
				}
				break;
			}
		case CTipParser::TIP_PARSER_FLAG:
			{
				DWORD dwFlag = _ttoi(szDest);
				if (pLink)
				{
					AppendCustomLink(pLink, dwFlag);
					delete []pLink;
					pLink = NULL;
				}
				break;
			}
		}
		nInitToken = nNextToken;
	}
	Append(cfFormat, L"\n");
	if (pLink)
		delete []pLink;
	delete []szDest;
//	SetScrollPos(UISB_VERT,100);
//	SetFocus(); 2009.5.11
	ScrollToBottom();
	return TRUE;
}

void CRichEditUI::SetLeftIndent(DWORD dwOffset, BOOL bBullet)
{
	PARAFORMAT_RE param;
	memset(&param, 0, sizeof(param));
	param.cbSize = sizeof(param);
	param.dwMask = PFM_STARTINDENT;
	param.dxStartIndent = dwOffset * 20;
	if (bBullet)
	{
		param.dwMask |= PFM_NUMBERING;
		param.wNumbering = PFN_BULLET;
	}
	PerformMessage(EM_SETPARAFORMAT, 0, LPARAM(&param));
}

DWORD CRichEditUI::GetCustomLinkFlag(DWORD dwMin)
{
	CGuardLock::COwnerLock guard(m_LinkLock);
	std::map<DWORD, LPCustomLinkItem>::iterator it = m_CustomLinkList.find(dwMin);
	if (it != m_CustomLinkList.end())
		return (*it).second->dwFlag;
	return 0;
}

void CRichEditUI::CharFormatToStyle(const CHARFORMAT_RE &cfFormat, CCharFontStyle &cfStyle)
{
	memset(&cfStyle, 0, sizeof(CCharFontStyle));
	cfStyle.cfColor = cfFormat.crTextColor;
	cfStyle.nFontStyle = cfFormat.dwEffects;
	cfStyle.nFontSize = cfFormat.yHeight / 20;
	::_tcsncpy(cfStyle.szFaceName, cfFormat.szFaceName,  MAX_TEXT_FACENAME_SIZE - 1);
}

void CRichEditUI::CharFontStyleToFormat(const CCharFontStyle &cfStyle, CHARFORMAT_RE &cfFormat)
{
	memset(&cfFormat, 0, sizeof(CHARFORMAT_RE));
	cfFormat.cbSize = sizeof(cfFormat);
	cfFormat.crTextColor = cfStyle.cfColor;
	cfFormat.dwEffects = cfStyle.nFontStyle;
	cfFormat.yHeight = cfStyle.nFontSize * 20;
	::_tcsncpy(cfFormat.szFaceName,  cfStyle.szFaceName, MAX_TEXT_FACENAME_SIZE);
	cfFormat.dwMask = CFM_COLOR | CFM_FACE | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT 
		              | CFM_SIZE;
}

//链接点击事件
BOOL CRichEditUI::LinkOnClick(char *szTitle, BOOL bIsCustom, DWORD dwFlag)
{
	if (bIsCustom)
	{
		if (m_pApp)
			m_pApp->OnCustomLinkClick(szTitle, dwFlag);
		else if (m_pCallBack)
		{
			if (m_pManager)
				m_pCallBack(m_pManager->GetPaintWindow(), RICHEDIT_EVENT_CUSTOMLINKCLICK, 
				           szTitle, NULL, NULL, NULL, dwFlag, m_lpOverlapped);
			else
				m_pCallBack(NULL, RICHEDIT_EVENT_CUSTOMLINKCLICK, 
				           szTitle, NULL, NULL, NULL, dwFlag, m_lpOverlapped);
		}
	} else
	{
#ifdef _UNICODE
		TCHAR szTemp[MAX_PATH] = {0};
		CStringConversion::StringToWideChar(szTitle, szTemp, MAX_PATH);
		::ShellExecute(NULL, L"OPEN", szTemp, NULL, NULL, SW_SHOW);
#else
		::ShellExecute(NULL, "OPEN", szTitle, NULL, NULL, SW_SHOW);
#endif
	}
	return TRUE;
}

//获取图标文件路径
BOOL CRichEditUI::GetInfoIcon(DWORD dwIconFlag, char *szFileName)
{
	return FALSE;
}

//设置字体
void CRichEditUI::SetFontStyle(CCharFontStyle &cfStyle)
{
	CHARFORMAT_RE cf;
	memset(&cf, 0, sizeof(CHARFORMAT_RE));
    CharFontStyleToFormat(cfStyle, cf);
	PerformMessage(EM_SETCHARFORMAT, SCF_DEFAULT, LPARAM(&cf));
}

void CRichEditUI::GetFontStyle(CCharFontStyle &cfStyle)
{
	CHARFORMAT_RE cf;
	memset(&cf, 0, sizeof(cf));
	PerformMessage(EM_GETCHARFORMAT, SCF_DEFAULT, LPARAM(&cf));
	CharFormatToStyle(cf, cfStyle);
}

//
BOOL CRichEditUI::GetCurrentChatId(char *szId)
{
	//
	CCharRange rg = {0};
	PerformMessage(EM_EXGETSEL, 0, LPARAM(&rg));
	if (rg.dwMax >= rg.dwMin)
	{
		std::map<std::string, CCharRange>::iterator it;
		for (it = m_rcMsgArea.begin(); it != m_rcMsgArea.end(); it ++)
		{
			if ((it->second.dwMin <= rg.dwMin) && (it->second.dwMax >= rg.dwMax))
			{
				if (szId)
					strcpy(szId, it->first.c_str());
				return TRUE;
			} //end if (
		} //end for (..
	} //end if (rg.dwMax
	return FALSE;
}


BOOL CRichEditUI::ClearChatMsg(const char *szId)
{
	std::map<std::string, CCharRange>::iterator it = m_rcMsgArea.find(szId);
	int nMin = 0;
	int nSubCount = 0;
	BOOL bSucc = FALSE;
	if (it != m_rcMsgArea.end())
	{ 
		static const TCHAR NULL_TEXT[] = L"";
		it->second.dwMax ++;  //增加一个回车符
		PerformMessage(EM_EXSETSEL, 0, LPARAM(&it->second)); 
		nMin = it->second.dwMax;
		nSubCount = it->second.dwMax - it->second.dwMin;
		PerformMessage(EM_REPLACESEL, 0, LPARAM(NULL_TEXT));
		m_rcMsgArea.erase(it);
		bSucc = TRUE;
	}
	for (it = m_rcMsgArea.begin(); it != m_rcMsgArea.end(); it ++)
	{
		if ((int)it->second.dwMin > nMin)
		{
			it->second.dwMin -= nSubCount;
			it->second.dwMax -= nSubCount;
		}
	}
	return bSucc; 
}

void CRichEditUI::TestNewLine()
{
	int nCount = GetCount();
	int nLine = PerformMessage(EM_LINEFROMCHAR, nCount, 0); 
	int nCol = nCount - PerformMessage(EM_LINEINDEX, nLine,  0); 
	if (nCol > 1)
		Append(L"\n");
}

//加入一个聊天记录 szUserName, 显示的用户名或者昵称 szTime--时间 szText --内容 cfStyle- 字体
BOOL CRichEditUI::AddChatText(const char *szId, const DWORD dwUserId, const char *szUserName, const char *szTime, 
	                          const char *szText, const CCharFontStyle &cfStyle, 
							  const int nNickColor, BOOL bIsUTF8, BOOL bAck)
{
	BOOL b = FALSE;
	if (m_pRichEditOle)
	{
		CHARFORMAT_RE cfFormat;  
		CCharRange crArea = {0};
		if ((!m_bAIMsg) || (!szUserName) || (!szTime) || (bAck) || m_strLastMsgTime.empty() || m_strLastName.empty()
			|| (::stricmp(szUserName, m_strLastName.c_str()) != 0)
			|| (CSystemUtils::MinusTimeString(szTime, m_strLastMsgTime.c_str()) > 60))
		{ 
			m_strLastName = szUserName;
			m_strLastMsgTime = szTime;
			m_fcDefaultStyle.cfColor = nNickColor;
			CharFontStyleToFormat(m_fcDefaultStyle, cfFormat);

	        //获取当前光标所在列号
			DWORD dwCount = GetCount(); 
			//光标移动到未尾
			CCharRange crg;	
			crArea.dwMin = dwCount;
			crg.dwMin = dwCount;
			crg.dwMax = crg.dwMin;
			PerformMessage(EM_EXSETSEL, 0, LPARAM(&crg));
			if (bIsUTF8)
			{
				TCHAR szwTemp[256] = {0};
				TCHAR szwTime[64] = {0};
				CStringConversion::StringToWideChar(szTime, szwTime, 63);
				CStringConversion::UTF8ToWideChar(szUserName, szwTemp, 255);
				::lstrcat(szwTemp, L"  ");
				::lstrcat(szwTemp, szwTime);
				::lstrcat(szwTemp, L"\n");
				TestNewLine();
				InsertUnicodeText(cfFormat, 0, szwTemp);
			} else
			{
				char szTemp[256] = {0};
				sprintf(szTemp, "%s   %s\n", szUserName, szTime);
				TestNewLine();
				//加入聊天人员名称
				InsertText(cfFormat, 0, szTemp);
			} 
		} 
		//插入聊天内容
		if (crArea.dwMin == 0)
			crArea.dwMin = GetCount();
        CharFontStyleToFormat(cfStyle, cfFormat);
		b = InsertOleText(cfFormat, DEFAULT_INDENT_CONTENT, szText, FALSE); 
		crArea.dwMax = GetCount() - 1; 
		//SetBackGroundColor(0xFF0000, crArea);
		if (szId)
		{
			m_rcMsgArea[szId] = crArea;
		} else
		{
			char strGuid[128] = {0};
			int nSize = 127;
			if (CSystemUtils::GetGuidString(strGuid, &nSize))
				m_rcMsgArea[strGuid] = crArea;
		}
		if (bAck)
			AppendAckLink(crArea);
		ScrollToBottom();
		 
	} 
	return b;
}

bool CRichEditUI::GetTagFromId(char *szTag, int nOleId, BOOL bIsFlag)
{
	m_OleLock.Lock();
	std::map<DWORD, LPOLE_ITEM>::iterator it = m_OleList.find(nOleId);
	if (it != m_OleList.end())
	{
		if (bIsFlag)
			sprintf(szTag, OLE_OBJECT_FORMAT_STR, (*it).second->szFlag);
		else
			strcpy(szTag, (*it).second->szFlag);
	} else
	{
		char szTemp[16] = {0};
		sprintf(szTemp, "%d", nOleId);
		if (bIsFlag)
			sprintf(szTag, OLE_OBJECT_FORMAT_STR, szTemp);
		else
			strcpy(szTag, szTemp);
	}
	m_OleLock.UnLock();
	return true;
}

//根据位置获取ole的userid
DWORD CRichEditUI::GetUserIdByPosition(DWORD dwPos)
{
	REOBJECT reObj;
	for (int i = 0; i < m_pRichEditOle->GetObjectCount(); i ++)
	{	
		memset(&reObj, 0, sizeof(REOBJECT));
		reObj.cbStruct = sizeof(REOBJECT);
		m_pRichEditOle->GetObject(i, &reObj, REO_GETOBJ_POLEOBJ); 
		if (reObj.cp == dwPos)
			return reObj.dwUser;
	}
	return 0xFFFFFFFF;
}

#define CUSTOM_OLE_STRING_SIZE 36

char *CRichEditUI::GetOleText(DWORD dwStyle)
{
	//
	char *szTemp = m_pWindowlessRE->GetText();
	int nSize = ::strlen(szTemp) + m_pRichEditOle->GetObjectCount() * CUSTOM_OLE_STRING_SIZE + 1;
	char *szReturn = (char *) malloc(nSize);
	memset(szReturn, 0, nSize);
	nSize = InsertOleToString(szTemp, szReturn, nSize);
	if (nSize == 0)
	{
		delete []szReturn;
		return NULL;
	} else
		return szReturn;
}

//转换后的ole文字串
int CRichEditUI::GetOleText(char *szText, int nMaxLen)
{
	char *szTemp = m_pWindowlessRE->GetText();
	int nReturn = InsertOleToString(szTemp, szText, nMaxLen);
	delete []szTemp;
	return nReturn;
}

int CRichEditUI::InsertOleToString(const char *szTemp, char *szText, int nMaxLen)
{
	int nDestPos = 0;
	//单个空格和只有一个回车符都算空
	if ((szTemp) && (strcmp(szTemp, " ") != 0))
	{
		//加入ole
		int nSrcLen = (int)::strlen(szTemp);
		if ((nSrcLen > 1) || (szTemp[0] != 13))
		{
			//转成unicode
			TCHAR *szwTemp = new TCHAR[nSrcLen + 1];
			memset(szwTemp, 0, sizeof(TCHAR) * (nSrcLen + 1));
			CStringConversion::StringToWideChar(szTemp, szwTemp, nSrcLen);
			nSrcLen = ::lstrlen(szwTemp);
			int nPos = 0;
			char szTag[64] = {0};
			std::string str;
			char *szPart;
			TCHAR *szwPart;
			int nPartLen;
			if (m_pRichEditOle)
			{
				for (int i = 0; i < m_pRichEditOle->GetObjectCount(); i ++)
				{
					REOBJECT reObj;
					memset(&reObj, 0, sizeof(REOBJECT));
					reObj.cbStruct = sizeof(REOBJECT);
					m_pRichEditOle->GetObject(i, &reObj, REO_GETOBJ_POLEOBJ); 
					//拷贝前面的的字符
					if ((reObj.cp - nPos) > 0)
					{
						nPartLen = reObj.cp - nPos;
						szwPart = new TCHAR[nPartLen + 1];
						memset(szwPart, 0, sizeof(TCHAR) * (nPartLen + 1));
						memmove(szwPart, szwTemp + nPos, nPartLen * sizeof(TCHAR));
						szPart = new char[nPartLen * 2 + 1];
						memset(szPart, 0, nPartLen * 2 + 1);
						CStringConversion::WideCharToString(szwPart, szPart, nPartLen * 2);
						str += szPart;
						delete []szPart;
						delete []szwPart;
						nPos += nPartLen;
					}
					nPos = reObj.cp + 1;
					memset(szTag, 0, 64);
					GetTagFromId(szTag, reObj.dwUser, TRUE);
					str += szTag;
				}
			}
			if (nPos < nSrcLen)
			{
				nPartLen = nSrcLen - nPos;
				szwPart = new TCHAR[nPartLen + 1];
				memset(szwPart, 0, sizeof(TCHAR) * (nPartLen + 1));
				memmove(szwPart, szwTemp + nPos, nPartLen * sizeof(TCHAR));				
				szPart = new char[nPartLen * 2 + 1];
				memset(szPart, 0, nPartLen * 2 + 1);
				CStringConversion::WideCharToString(szwPart, szPart, nPartLen * 2);
				str += szPart;
				delete []szPart;
				delete []szwPart;				
			}
			strncpy(szText, str.c_str(), nMaxLen);
			nDestPos = (int)str.size();
			delete []szwTemp;
		}
	}
	//去掉最后一个换行符
	if (szText[strlen(szText) - 1] == '\n')
	{
		szText[strlen(szText) - 1] = '\0';
		nDestPos -= 1;
	}
	return nDestPos;
}

//获取所有插入到richedit 内的OLE对象flag
BOOL CRichEditUI::GetOleFlags(CStringList_ &Flags)
{
	if (m_pRichEditOle)
	{
		REOBJECT reObj;
		char *szTag;
		for (int i = 0; i < m_pRichEditOle->GetObjectCount(); i ++)
		{			
			memset(&reObj, 0, sizeof(REOBJECT));
			reObj.cbStruct = sizeof(REOBJECT);
			m_pRichEditOle->GetObject(i, &reObj, REO_GETOBJ_POLEOBJ); 		    
			szTag = new char[64];
			memset(szTag, 0, 64);
			GetTagFromId(szTag, reObj.dwUser, FALSE);
			Flags.push_back(szTag);
			delete []szTag;
		}
		return (Flags.size() > 0);
	}
	return FALSE;
}

LRESULT CRichEditUI::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
   if (!IsVisible() || !IsEnabled())
	   return 0; 
 
   
   bool bWasHandled = true;
   if ((uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST) || uMsg == WM_SETCURSOR )
   {
        switch (uMsg) 
		{
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
            {
                POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                CControlUI* pHover = GetManager()->FindControl(pt);
                if (pHover != this) 
				{
                    bWasHandled = false;
                    return 0;
                }
            }
            break;
        }
         
        if ( uMsg == WM_SETCURSOR ) 
			bWasHandled = false;
        else if( uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONDBLCLK || uMsg == WM_RBUTTONDOWN )
		{
            SetFocus();
        }
    }
#ifdef _UNICODE
    else if( uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST )
	{
#else
    else if( (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST) || uMsg == WM_CHAR || uMsg == WM_IME_CHAR ) {
#endif
        if( !IsFocused() ) return 0;
    }
    else
    {
        switch( uMsg ) {
        case WM_HELP:
        case WM_CONTEXTMENU:
            bWasHandled = false;
            break;
        default:
            return 0;
        }
    }
    LRESULT lResult = 0;
	HRESULT Hr = PerformMessage(uMsg, wParam, lParam); 
	if (Hr == S_OK)
		bHandled = bWasHandled;
    return lResult;
}

//滚动到底部
void CRichEditUI::ScrollToBottom()
{
	PerformMessage(WM_VSCROLL, (SB_BOTTOM & 0x0000FFFF), 0);
	PerformMessage(WM_PAINT, 0, 0);
}

DWORD CALLBACK StreamSave(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	CMemoryStream *pStream = (CMemoryStream *)dwCookie;
	if (pStream)
	{
		*pcb = pStream->Write(pbBuff, cb);
		return 0;
	} else
		return 2; //write
}

DWORD CALLBACK StreamLoad(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	CMemoryStream *pStream = (CMemoryStream *)dwCookie;
	if (pStream)
	{
		*pcb = pStream->ReadBuffer(pbBuff, cb);
		return 0;
	} else
		return 1; //read
}

BOOL CRichEditUI::GetRichText(DWORD dwStyle, char **pBuf, int &nSize)
{
	EDITSTREAM edtStream;
	CMemoryStream RTFStream;
	edtStream.dwCookie = (DWORD_PTR) &RTFStream;
	edtStream.pfnCallback = StreamSave;
	edtStream.dwError = 0;
	PerformMessage(EM_STREAMOUT, dwStyle, LPARAM(&edtStream));
	nSize = (int) RTFStream.GetSize();
	void *pOut = malloc(nSize);
	memcpy(pOut, RTFStream.GetMemory(), nSize);
	*pBuf = (char *)pOut;
	return TRUE;
}

void CRichEditUI::SetRichText(const char *szText, DWORD dwStyle)
{
	EDITSTREAM edtStream;
	CMemoryStream RTFStream;
	RTFStream.Write(szText, (int)::strlen(szText));
	RTFStream.SetPosition(0);
	edtStream.dwCookie = (DWORD_PTR) &RTFStream;
	edtStream.pfnCallback = StreamLoad;
	edtStream.dwError = 0;
	PerformMessage(EM_STREAMIN, dwStyle, LPARAM(&edtStream));
}

void  CRichEditUI::SetBackGroundColor(COLORREF clr, CCharRange cr)
{
    CHARFORMAT2 cf;
	memset(&cf, 0, sizeof(cf)); 
	cf.cbSize = sizeof(cf);
	cf.crBackColor = clr;
	
	cf.dwEffects = CFE_AUTOCOLOR;
	cf.dwMask = CFM_BACKCOLOR; 
	PerformMessage(EM_EXSETSEL, 0, LPARAM(&cr));
	PerformMessage(EM_SETCHARFORMAT, SCF_DEFAULT, (LPARAM) &cf);
}

void CRichEditUI::SetBullet(BOOL bBullet)
{
	PARAFORMAT_RE pf;
	memset(&pf, 0, sizeof(pf));
	pf.cbSize = sizeof(pf);
	pf.dxOffset = 0;
	pf.dwMask = PFM_NUMBERING;
	if (bBullet)
		pf.wNumbering = PFN_BULLET;//注意PFM_NUMBERING 
	else
		pf.wNumbering = 0;
	PerformMessage(EM_SETPARAFORMAT, 0, LPARAM(&pf));
}

#pragma warning(default:4996)
