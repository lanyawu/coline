#include <atlbase.h>
#include <map>
#include <CommonLib/StringUtils.h>
#include <UILib/UIRichEdit2.h>
#include <RichEdit.h>
#include <RichOle.h>

#pragma warning(disable:4996)
//#import "D:\\Program Files\\Tencent\\TM\TMDLLS\\ImageOle.dll" named_guids no_namespace
#import "D:\\download\\NewIM\\bin\\debug\\baseolectrl.dll" named_guids no_namespace
//#import "D:\\Lanya\\WorkArea\\IT168IM\\bin\\debug\\Gif89.dll" named_guids no_namespace

#define EM_GETOLEINTERFACE  (WM_USER + 60)

#define EM_NOTIFY  0x204E  //通知消息

HMODULE CRichEdit2UI::m_hRichEdit2Module = NULL;
volatile LONG CRichEdit2UI::m_hRichEditRef = 0;

//文本提示分析器
class CTipParser
{
public:
	CTipParser(char *szText);
public:
	enum{
		TIP_PARSER_TEXT = 1,
		TIP_PARSER_LINK,
		TIP_PARSER_FLAG
	};
	BOOL GetNextToken(char *szDest, int nInitToken, int &nNextToken);
private:
	const char *m_pszText;
	DWORD m_dwPos; //当前位置
	DWORD m_dwSize; //文本长度
};

CTipParser::CTipParser(char *szText):
            m_pszText(szText),
			m_dwPos(0)
{
	m_dwSize = strlen(m_pszText);
}

BOOL CTipParser::GetNextToken(char *szDest, int nInitToken, int &nNextToken)
{
	if (m_dwPos >= m_dwSize)
		return FALSE;
	const char *szTemp = m_pszText + m_dwPos;
	const char *szCurr = szTemp;
	switch(nInitToken)
	{
	case TIP_PARSER_TEXT:
		 while((*szCurr != '\0') && (*szCurr != '<'))
			 szCurr ++;
	     nNextToken = TIP_PARSER_LINK;
		 break;
	case TIP_PARSER_LINK:
		 while((*szCurr != '\0') && (*szCurr != ','))
			 szCurr ++;
		 nNextToken = TIP_PARSER_FLAG;
		 break;
	case TIP_PARSER_FLAG:
		 while((*szCurr != '\0') && (*szCurr != '>'))
			 szCurr ++;
		 nNextToken = TIP_PARSER_TEXT;
		 break;
	default:
		 return FALSE;
	}
    strncpy(szDest, szTemp, szCurr - szTemp);
	m_dwPos = m_dwPos + (szCurr - szTemp) + 1;
	return TRUE;
}

// =================================
//       class CRichEdit2Wnd 
// =================================

class CRichEdit2Wnd :public CWindowWnd
{
public:
	CRichEdit2Wnd();
	~CRichEdit2Wnd();
private:
	IRichEditOle *m_pOle;
private:
	void SetLeftIndent(DWORD dwOffset);
	DWORD GetCustomLinkFlag(DWORD dwMin);
	void  PaintRichEditBackGround(); //绘制背景图
public:
   void Init(CRichEdit2UI* pOwner);

   LPCTSTR GetWindowClassName() const;
   LPCTSTR GetSuperClassName() const;
   void OnFinalMessage(HWND hWnd);

   LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
   LRESULT OnEditChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   void    InsertGif(char *lpszFileName);
   void    InsertBmp(char *lpszFileName);
   BOOL    InsertText(CCharFormat &cfFormat, DWORD dwOffset, char *szText);
   BOOL    InsertOleText(DWORD dwOffset, char *szText); //插入一段Ole字符串，包括图片数据
   void    Append(CCharFormat &cfFormat, char *szText); //追加内容
   void    AppendCustomLink(char *szText, DWORD dwFlag);
   BOOL    InsertTip(CCharFormat &cfFormat, DWORD dwOffset, char *szText);
protected:
   CRichEdit2UI* m_pOwner;
private:
	std::map<DWORD, DWORD> m_CustomLinkList;  //自定义连接参数
};

CRichEdit2Wnd::CRichEdit2Wnd():
               m_pOle(NULL)
{

}

CRichEdit2Wnd::~CRichEdit2Wnd()
{
	if (m_pOle)
	{
		m_pOle->Release();
		m_pOle = NULL;
	}
}

DWORD CRichEdit2Wnd::GetCustomLinkFlag(DWORD dwMin)
{
	std::map<DWORD, DWORD>::iterator it = m_CustomLinkList.find(dwMin);
	if (it != m_CustomLinkList.end())
		return (*it).second;
	return 0;
}

void CRichEdit2Wnd::Init(CRichEdit2UI* pOwner)
{
   RECT rcPos = pOwner->GetPos();
   ::InflateRect(&rcPos, -1, -3);
   Create(pOwner->GetManager()->GetPaintWindow(), L"RICHEDIT2.0", WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL, 
	       0, rcPos);
   SetWindowFont(m_hWnd, pOwner->GetManager()->GetThemeFont(UIFONT_NORMAL), TRUE);
   Edit_SetText(m_hWnd, pOwner->GetText());
   Edit_SetModify(m_hWnd, FALSE);
   SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(2, 2));
   Edit_SetReadOnly(m_hWnd, pOwner->IsReadOnly() == true);
   Edit_Enable(m_hWnd, pOwner->IsEnabled() == true);
   if( pOwner->IsVisible() ) 
	   ::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
   ::SetFocus(m_hWnd);
   m_pOwner = pOwner;
   SendMessage(EM_GETOLEINTERFACE, 0, LONG(&m_pOle));
}



void CRichEdit2Wnd::SetLeftIndent(DWORD dwOffset)
{
	PARAFORMAT param;
	memset(&param, 0, sizeof(PARAFORMAT));
	param.cbSize = sizeof(PARAFORMAT);
	param.dwMask = PFM_STARTINDENT;
	param.dxStartIndent = dwOffset * 20;
	::SendMessage(m_hWnd, EM_SETPARAFORMAT, 0, LPARAM(&param));
}

void CRichEdit2Wnd::PaintRichEditBackGround()
{
	//
}

void CRichEdit2Wnd::InsertBmp(char *lpszFileName)
{

}

void CRichEdit2Wnd::InsertGif(char *lpszFileName)
{
	LPLOCKBYTES lpLockBytes = NULL;
	SCODE sc;
	HRESULT hr;
	//print to RichEdit' s IClientSite
	LPOLECLIENTSITE m_lpClientSite;
	//A smart point to IAnimator
	//IGifAnimatorPtr m_lpAnimator;
	IGifAnimatePtr pControl;
	//IGif89aPtr pGif;
	//ptr 2 storage	
	LPSTORAGE m_lpStorage;
	//the object 2 b insert 2
	LPOLEOBJECT	m_lpObject;

	//Create lockbytes
	sc = ::CreateILockBytesOnHGlobal(NULL, TRUE, &lpLockBytes);
	if (sc != S_OK)
		return;
	ASSERT(lpLockBytes != NULL);
	
	//use lockbytes to create storage
	sc = ::StgCreateDocfileOnILockBytes(lpLockBytes,
		STGM_SHARE_EXCLUSIVE|STGM_CREATE|STGM_READWRITE, 0, &m_lpStorage);
	if (sc != S_OK)
	{
		lpLockBytes->Release();
		lpLockBytes = NULL;
		return ;
	}
	ASSERT(m_lpStorage != NULL);
	
	//get the ClientSite of the very RichEditCtrl
	m_pOle->GetClientSite(&m_lpClientSite);
	ASSERT(m_lpClientSite != NULL);

	try
	{
		//Initlize COM interface
		hr = ::CoInitialize( NULL);
		if( FAILED(hr) )
			return;
		
		//Get GifAnimator object
		//here, I used a smart point, so I do not need to free it
		//
		//hr = m_lpAnimator.CreateInstance(CLSID_GifAnimator);	
		hr = pControl.CreateInstance(CLSID_GifAnimate);
		//hr = pGif.CreateInstance(CLSID_Gif89a);
		if( FAILED(hr) )
				return;
		//COM operation need BSTR, so get a BSTR
#ifdef _UNICODE
		TCHAR szTemp[MAX_PATH] = {0};
		CStringConversion::StringToWideChar(lpszFileName, szTemp, MAX_PATH);
		BSTR path = SysAllocString(szTemp);
#endif
		//Load the gif
		//m_lpAnimator->LoadFromFile(path);
        pControl->LoadFile(path);
		//pGif->put_FileName(path);
		if( FAILED(hr) )
			return;
			

		
		//get the IOleObject
		//hr = m_lpAnimator->QueryInterface(IID_IOleObject, (void**)&m_lpObject);
		hr = pControl->QueryInterface(IID_IOleObject, (void **)&m_lpObject);
		//hr = pGif->QueryInterface(IID_IOleObject, (void **)&m_lpObject);
		if( FAILED(hr) )
			return;
		
		//Set it 2 b inserted
		OleSetContainedObject(m_lpObject, TRUE);
		
		//2 insert in 2 richedit, you need a struct of REOBJECT
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
		reobject.dwUser = 0;
		//the very object
		reobject.poleobj = m_lpObject;
		//client site contain the object
		reobject.polesite = m_lpClientSite;
		//the storage 
		reobject.pstg = m_lpStorage;
		
		SIZEL sizel;
		sizel.cx = sizel.cy = 0;
		reobject.sizel = sizel;
		HWND hWndRT = this->m_hWnd;
		//Sel all text
		::SendMessage(hWndRT, EM_SETSEL, 0, -1);
		DWORD dwStart, dwEnd;
		::SendMessage(hWndRT, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
		::SendMessage(hWndRT, EM_SETSEL, dwEnd+1, dwEnd+1);
		//Insert after the line of text
		m_pOle->InsertObject(&reobject);
		::SendMessage(hWndRT, EM_SCROLLCARET, (WPARAM)0, (LPARAM)0);
		VARIANT_BOOL ret;
		//do frame changing
		//ret = m_lpAnimator->Play();
		//ret = m_lpAnimator->TriggerFrameChange();
		//show it
		m_lpObject->DoVerb(OLEIVERB_UIACTIVATE, NULL, m_lpClientSite, 0, m_hWnd, NULL);
		m_lpObject->DoVerb(OLEIVERB_SHOW, NULL, m_lpClientSite, 0, m_hWnd, NULL);
		
		//redraw the window to show animation
		//RedrawWindow(m_hWnd, NULL, NULL, 0);
        pControl->Play(); 
		//pGif->put_AutoStart(TRUE);
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

		SysFreeString(path);
	}
	catch( _com_error e )
	{
		::CoUninitialize();	
	}
}
BOOL CRichEdit2Wnd::InsertOleText(DWORD dwOffset, char *szText)
{
	//
	return FALSE;
	
}

BOOL CRichEdit2Wnd::InsertText(CCharFormat &cfFormat, DWORD dwOffset, char *szText)
{
	CCharRange range;
	BOOL b = TRUE;
	size_t size = strlen(szText);
	char *szTemp = new char[size + 3];
	memset(szTemp, 0, size + 3);
	int idx;
	SetLeftIndent(dwOffset);
	DWORD dwCount = ::SendMessage(m_hWnd, EM_GETLINECOUNT, 0, 0);
	range.dwMin = ::SendMessage(m_hWnd, EM_LINEINDEX, dwCount - 1, 0);
	if (range.dwMin >= 0)
	{
		sprintf(szTemp, "%s\n", szText);
	}
	else
	{
		range.dwMin = ::SendMessage(m_hWnd, EM_LINEINDEX, -1, 0);
		if (range.dwMin < 0)
		{
			return FALSE;
		} else
		{
			DWORD L = ::SendMessage(m_hWnd, EM_LINELENGTH, range.dwMin, 0);
			if (L == 0)
				b = FALSE;
			else
			{
				range.dwMin -= L;
				sprintf(szTemp, "\n%s", szText);
			}
		}
	}
	if (b)
	{
		range.dwMax = range.dwMin;
		::SendMessage(m_hWnd, EM_EXSETSEL, 0, LPARAM(&range));
		::SendMessage(m_hWnd, EM_SETCHARFORMAT, SCF_SELECTION, LPARAM(&cfFormat));
#ifdef _UNICODE
		TCHAR *szwText = new TCHAR[size + 3];
		memset(szwText, 0, sizeof(TCHAR) * (size + 3));
		CStringConversion::StringToWideChar(szTemp, szwText, size + 3);
		::SendMessage(m_hWnd, EM_REPLACESEL, 0, LPARAM(szwText));
		::SendMessage(m_hWnd, EM_SETCHARFORMAT, SCF_SELECTION, LPARAM(&cfFormat));
		delete []szwText;
#else
		::SendMessage(m_hWnd, EM_REPLACESEL, 0, LPARAM(szTemp));
#endif
	}
	delete []szTemp;
	return b;
}

void CRichEdit2Wnd::Append(CCharFormat &cfFormat, char *szText)
{
	CCharRange range;
	BOOL b = TRUE;
	DWORD dwCount = ::SendMessage(m_hWnd, EM_GETLINECOUNT, 0, 0);
	range.dwMin = ::SendMessage(m_hWnd, EM_LINEINDEX, dwCount, 0);
	range.dwMax = range.dwMin;
	::SendMessage(m_hWnd, EM_EXSETSEL, 0, LPARAM(&range));
#ifdef _UNICODE
	size_t size = strlen(szText);
	TCHAR *szwText = new TCHAR[size + 3];
	memset(szwText, 0, sizeof(TCHAR) * (size + 3));
	CStringConversion::StringToWideChar(szText, szwText, size + 3);
	::SendMessage(m_hWnd, EM_REPLACESEL, 0, LPARAM(szwText));
	::SendMessage(m_hWnd, EM_SETCHARFORMAT, SCF_SELECTION, LPARAM(&cfFormat));
	delete []szwText;
#else
	::SendMessage(m_hWnd, EM_REPLACESEL, 0, LPARAM(szText));
#endif
}

BOOL CRichEdit2Wnd::InsertTip(CCharFormat &cfFormat, DWORD dwOffset, char *szText)
{
	SetLeftIndent(dwOffset);
    CTipParser parser(szText);
	size_t size = strlen(szText) + 1;
	char *szDest = new char[size];
	char *pLink = NULL;
	int nInitToken = CTipParser::TIP_PARSER_TEXT;
	int nNextToken;
	Append(cfFormat, "\n");
	while(true)
	{
		memset(szDest, 0, size);
		if (!parser.GetNextToken(szDest, nInitToken, nNextToken))
			break;
		switch(nInitToken)
		{
		case CTipParser::TIP_PARSER_TEXT:
			 Append(cfFormat, szDest);
			 break;
		case CTipParser::TIP_PARSER_LINK:
			{
				int nSize = strlen(szDest);
				if (pLink)
					delete []pLink;
				pLink = NULL;
				if (nSize > 0)
				{
					pLink = new char[nSize + 1];
					memset(pLink, 0, nSize + 1);
					strncpy(pLink, szDest, nSize);
				}
				break;
			}
		case CTipParser::TIP_PARSER_FLAG:
			{
				DWORD dwFlag = atoi(szDest);
				if ((dwFlag > 0) && (pLink))
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
	if (pLink)
		delete []pLink;
	delete []szDest;
	return TRUE;
}

void CRichEdit2Wnd::AppendCustomLink(char *szText, DWORD dwFlag)
{
	CCharRange range;
	BOOL b = TRUE;
	DWORD dwCount = ::SendMessage(m_hWnd, EM_GETLINECOUNT, 0, 0);
	range.dwMin = ::SendMessage(m_hWnd, EM_LINEINDEX, dwCount, 0);
	range.dwMax = range.dwMin;
	::SendMessage(m_hWnd, EM_EXSETSEL, 0, LPARAM(&range));
#ifdef _UNICODE
	size_t size = strlen(szText);
	TCHAR *szwText = new TCHAR[size + 3];
	memset(szwText, 0, sizeof(TCHAR) * (size + 3));
	CStringConversion::StringToWideChar(szText, szwText, size + 3);
	::SendMessage(m_hWnd, EM_REPLACESEL, 0, LPARAM(szwText));
	delete []szwText;
#else
	::SendMessage(m_hWnd, EM_REPLACESEL, 0, LPARAM(szText));
#endif
	dwCount = ::SendMessage(m_hWnd, EM_GETLINECOUNT, 0, 0);
	range.dwMax = ::SendMessage(m_hWnd, EM_LINEINDEX, dwCount - 1, 0);
	//判断最后一个是否为中文
	bool bChinese = false;
	if (size > 0)
	{
		if (szText[size -1] > 0x7F)
			bChinese = true;
	}
	if (b)
		range.dwMax += sizeof(TCHAR);
	else
		range.dwMax ++;
	range.dwMin = range.dwMax - size;
	::SendMessage(m_hWnd, EM_EXSETSEL, 0, LPARAM(&range));
	CCharFormat param;
	memset(&param, 0, sizeof(CCharFormat));
	param.cbSize = sizeof(CCharFormat);
	param.dwMask = CFM_LINK;
	param.dwEffects = CFE_LINK;
	::SendMessage(m_hWnd, EM_SETCHARFORMAT, SCF_SELECTION, LPARAM(&param));
	//加入到列表当中
	m_CustomLinkList.insert(pair<DWORD, DWORD>(range.dwMin, dwFlag));
}


LPCTSTR CRichEdit2Wnd::GetWindowClassName() const
{
   return _T("UIRICHEDIT");
}

LPCTSTR CRichEdit2Wnd::GetSuperClassName() const
{
   return WC_RICHEDIT;
}

void CRichEdit2Wnd::OnFinalMessage(HWND /*hWnd*/)
{
   m_pOwner->m_pWindow = NULL;
   delete this;
}

LRESULT CRichEdit2Wnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   LRESULT lRes = 0;
   BOOL bHandled = TRUE;
   if( uMsg == OCM_COMMAND && GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE ) 
	   lRes = OnEditChanged(uMsg, wParam, lParam, bHandled);
  // else if (uMsg == WM_NCPAINT)
  // {
	//   PaintRichEditBackGround();
	//   bHandled = FALSE;
  // }
   else if (uMsg == EM_NOTIFY)
   {
	   if ((LPNMHDR(lParam))->code == EN_LINK)
	   {
		   ENLINK *p = (ENLINK *)lParam;
		   if ((p->msg  == WM_LBUTTONDOWN) && (m_pOwner))
		   {
			   ::SendMessage(m_hWnd,EM_EXSETSEL, 0, LPARAM(&(p->chrg)));
			   char szTitle[MAX_PATH] = {0};
#ifdef _UNICODE
			   TCHAR szTemp[MAX_PATH] = {0};
			   ::SendMessage(m_hWnd, EM_GETSELTEXT, 0, LPARAM(&szTitle));
			  // CStringConversion::WideCharToString(szTemp, szTitle, MAX_PATH);
#else
			   ::SendMessage(m_hWnd, EM_GETSELTEXT, 0, LPARAM(&szTitle));
#endif
			   //查找是否存在自定义custom
			   DWORD dwFlag = GetCustomLinkFlag(p->chrg.cpMin);
			   if (dwFlag > 0)
				   m_pOwner->LinkOnClick(szTitle, TRUE, dwFlag);
			   else
				   m_pOwner->LinkOnClick(szTitle, FALSE, 0);
		   } else
			   bHandled = FALSE;
	   } else
		   bHandled = FALSE;

   } else
	   bHandled = FALSE;
   if( !bHandled ) return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
   return lRes;
}

LRESULT CRichEdit2Wnd::OnEditChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   if( m_pOwner == NULL ) return 0;
   // Copy text back
   int cchLen = ::GetWindowTextLength(m_hWnd) + 1;
   LPTSTR pstr = static_cast<LPTSTR>(_alloca(cchLen * sizeof(TCHAR)));
   ASSERT(pstr);
   ::GetWindowText(m_hWnd, pstr, cchLen);
   m_pOwner->m_sText = pstr;
   m_pOwner->GetManager()->SendNotify(m_pOwner, _T("changed"));
   return 0;
}


//  =========================================
//           class CRichEdit2UI
//  =========================================

CRichEdit2UI::CRichEdit2UI() : m_pWindow(NULL)
{
	//初始化默认字体
	memset(&m_fcDefaultStyle, 0, sizeof(CCharFontStyle));
	m_fcDefaultStyle.cfColor = RGB(0, 0, 0);
	m_fcDefaultStyle.nFontSize = 8;
	_tcscpy(m_fcDefaultStyle.szFaceName, L"Tahoma");
}

CRichEdit2UI::~CRichEdit2UI()
{
   if( m_pWindow != NULL && ::IsWindow(*m_pWindow)) 
	   m_pWindow->Close();
   ::InterlockedDecrement(&m_hRichEditRef);
   if (m_hRichEditRef == 0)
   {
	   ::FreeLibrary(m_hRichEdit2Module);
	   m_hRichEdit2Module = NULL;
	   ::CoUninitialize();
   }
}

void CRichEdit2UI::Init()
{
   if (!CRichEdit2UI::m_hRichEdit2Module)
   {
	   ::CoInitialize(NULL);
	   CRichEdit2UI::m_hRichEdit2Module = ::LoadLibrary(L"RICHED32.DLL");
	   if (CRichEdit2UI::m_hRichEdit2Module <= (HMODULE)HINSTANCE_ERROR)
		   CRichEdit2UI::m_hRichEdit2Module = (HMODULE)0;
	   
   }
   ::InterlockedIncrement(&m_hRichEditRef);
   m_pWindow = new CRichEdit2Wnd();
   ASSERT(m_pWindow);
   m_pWindow->Init(this);
   SetAutioDetect(true);
   SetReadOnly(true);
   SetTransParent(true);
   //获取初始化值
  // m_pWindow->InsertGif("D:\\a.gif");
   //m_pWindow->InsertText(0, "插入一段中文，ABCDE中文");
  // m_pWindow->InsertGif("D:\\b.gif");
  // m_pWindow->InsertText(20, "insert english 中文");
   CCharFontStyle cfStyle;
   memset(&cfStyle, 0, sizeof(CCharFontStyle));
   cfStyle.cfColor = RGB(255, 0, 0);
   cfStyle.nFontSize = 20;
   cfStyle.nFontStyle = CFE_BOLD | CFE_UNDERLINE;
   _tcscpy(cfStyle.szFaceName, L"宋体");
   AddChatText(12344, "渔夫", "2008-10-20 15:12:12", "聊天内容，CRichEdit2UI::m_hRichEdit2Module \n\
												  if (CRichEdit2UI::m_hRichEdit2Module <= (HMODULE)HINSTANCE_ERROR)\
												  CRichEdit2UI::m_hRichEdit2Module = (HMODULE)0;", cfStyle);
   cfStyle.cfColor = RGB(0, 255, 0);
   cfStyle.nFontSize = 30;
   cfStyle.nFontStyle = CFE_ITALIC;
   AddChatText(12344, "渔夫对话", "2008-10-20 16:11:33", "http://www.xinhuanet.com 回复记录, CRichEdit2UI::m_hRichEdit2Module \n\
												  if (CRichEdit2UI::m_hRichEdit2Module <= (HMODULE)HINSTANCE_ERROR)\
        										  CRichEdit2UI::m_hRichEdit2Module 插入中文= (HMODULE)0;", cfStyle);
   CCharFormat cfFormat = {0};
   m_pWindow->Append(cfFormat, "追加的内容");
   //m_pWindow->InsertGif("D:\\a.gif");
   m_pWindow->Append(cfFormat, "追加内容2");
  // m_pWindow->InsertGif("D:\\b.gif");
   m_pWindow->Append(cfFormat, "追加内容3");
   m_pWindow->AppendCustomLink("超级链接", 3434);
   CharFontStyleToFormat(m_fcDefaultStyle, cfFormat);
   m_pWindow->InsertTip(cfFormat, 0, "测试自定义链接<取消,120>,第二组链接<确定,120>,第三组链接<另存为,120>");
}

LPCTSTR CRichEdit2UI::GetClass() const
{
   return _T("RichEdit2UI");
}


void CRichEdit2UI::SetTransParent(bool b)
{
	if (m_pWindow)
	{
		long style = ::GetWindowLong(m_pWindow->GetHWND(), GWL_EXSTYLE);
		if (b)
			style &= WS_EX_TRANSPARENT;
		else
			style &= ~WS_EX_TRANSPARENT;
		::SetWindowLong(m_pWindow->GetHWND(), GWL_EXSTYLE, style);
	}
}

void CRichEdit2UI::SetAutioDetect(bool bIsAuto)
{
	DWORD dwMask = ::SendMessage(m_pWindow->GetHWND(), EM_GETEVENTMASK, 0, 0);
	dwMask = dwMask | ENM_LINK | ENM_MOUSEEVENTS | ENM_SCROLLEVENTS | ENM_KEYEVENTS;
    SendMessage(m_pWindow->GetHWND(), EM_SETEVENTMASK, 0, dwMask);
    SendMessage(m_pWindow->GetHWND(), EM_AUTOURLDETECT, bIsAuto, 0);
}

void CRichEdit2UI::SetSelStart(DWORD dwStart)
{
	CCharRange CharRange;
	CharRange.dwMax = dwStart;
	CharRange.dwMin = dwStart;
	SendMessage(m_pWindow->GetHWND(), EM_EXSETSEL, 0, LPARAM(&CharRange));
}

void CRichEdit2UI::SetSelLength(DWORD dwLength)
{
	CCharRange CharRange;
	SendMessage(m_pWindow->GetHWND(), EM_EXGETSEL, 0, LPARAM(&CharRange));
	CharRange.dwMax = CharRange.dwMin + dwLength;
	SendMessage(m_pWindow->GetHWND(), EM_EXSETSEL, 0, LPARAM(&CharRange));
	SendMessage(m_pWindow->GetHWND(), EM_SCROLLCARET, 0, 0);
}

DWORD CRichEdit2UI::GetSelLength()
{
	CCharRange CharRange;
	SendMessage(m_pWindow->GetHWND(), EM_EXGETSEL, 0, LPARAM(&CharRange));
	return (CharRange.dwMax - CharRange.dwMin);
}

DWORD CRichEdit2UI::GetSelStart()
{
	CCharRange CharRange;
	SendMessage(m_pWindow->GetHWND(), EM_EXGETSEL, 0, LPARAM(&CharRange));
    return CharRange.dwMin;
}


UINT CRichEdit2UI::GetControlFlags() const
{
   return UIFLAG_TABSTOP;
}

void CRichEdit2UI::SetText(LPCTSTR pstrText)
{
   m_sText = pstrText;
   if( m_pWindow != NULL ) SetWindowText(*m_pWindow, pstrText);
   if( m_pManager != NULL ) m_pManager->SendNotify(this, _T("changed"));
   Invalidate();
}

CStdString CRichEdit2UI::GetText() const
{
   if( m_pWindow != NULL ) {
      int cchLen = ::GetWindowTextLength(*m_pWindow) + 1;
      LPTSTR pstr = static_cast<LPTSTR>(_alloca(cchLen * sizeof(TCHAR)));
      ASSERT(pstr);
      ::GetWindowText(*m_pWindow, pstr, cchLen);
      return CStdString(pstr);
   }
   return m_sText;
}

void CRichEdit2UI::SetVisible(bool bVisible)
{
   CControlUI::SetVisible(bVisible);
   if( m_pWindow != NULL ) ::ShowWindow(*m_pWindow, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
}

void CRichEdit2UI::SetEnabled(bool bEnabled)
{
   CControlUI::SetEnabled(bEnabled);
   if( m_pWindow != NULL ) ::EnableWindow(*m_pWindow, bEnabled == true);
}

void CRichEdit2UI::SetReadOnly(bool bReadOnly)
{
   if( m_pWindow != NULL ) 
	   Edit_SetReadOnly(*m_pWindow, bReadOnly == true);
   Invalidate();
}

bool CRichEdit2UI::IsReadOnly() const
{
   return (GetWindowStyle(*m_pWindow) & ES_READONLY) != 0;
}

SIZE CRichEdit2UI::EstimateSize(SIZE /*szAvailable*/)
{
   return CSize(m_rcItem);
}

void CRichEdit2UI::SetPos(RECT rc)
{
   if( m_pWindow != NULL ) {
      CRect rcEdit = rc;
      rcEdit.Deflate(3, 3);
      ::SetWindowPos(*m_pWindow, HWND_TOP, rcEdit.left, rcEdit.top, rcEdit.GetWidth(), rcEdit.GetHeight(), SWP_NOACTIVATE);
   }
   CControlUI::SetPos(rc);
}

void CRichEdit2UI::SetPos(int left, int top, int right, int bottom)
{
   SetPos(CRect(left, top, right, bottom));
}

void CRichEdit2UI::Event(TEventUI& event)
{
   if( event.Type == UIEVENT_WINDOWSIZE )
   {
      if( m_pWindow != NULL ) 
		  m_pManager->SetFocus(NULL);
   }
   if( event.Type == UIEVENT_SETFOCUS ) 
   {
      if( m_pWindow != NULL ) 
		  ::SetFocus(*m_pWindow);
   }
   CControlUI::Event(event);
}

void CRichEdit2UI::DoPaint(HDC hDC, const RECT& /*rcPaint*/)
{
   UINT uState = 0;
   if( IsFocused() ) uState |= UISTATE_FOCUSED;
   if( IsReadOnly() ) uState |= UISTATE_READONLY;
   if( !IsEnabled() ) uState |= UISTATE_DISABLED;
   CBlueRenderEngineUI::DoPaintEditBox(hDC, m_pManager, m_rcItem, _T(""), uState, 0, true);
}

void CRichEdit2UI::CharFontStyleToFormat(CCharFontStyle cfStyle, CCharFormat &cfFormat)
{
	memset(&cfFormat, 0, sizeof(CCharFormat));
	cfFormat.cbSize = sizeof(CCharFormat);
	cfFormat.dwTextColor = cfStyle.cfColor;
	cfFormat.dwEffects = cfStyle.nFontStyle;
	cfFormat.dwHeight = cfStyle.nFontSize * 20;
	::_tcsncpy(cfFormat.szFaceName,  cfStyle.szFaceName, MAX_TEXT_FACENAME_SIZE);
	cfFormat.dwMask = CFM_COLOR | CFM_FACE | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT 
		              | CFM_SIZE;
}


BOOL CRichEdit2UI::AddChatText(DWORD dwUserId, char *szUserName, char *szTime, char *szText, CCharFontStyle &cfStyle)
{
	if (m_pWindow)
	{
		CCharFormat cfFormat;
		CharFontStyleToFormat(m_fcDefaultStyle, cfFormat);
		char szTemp[256] = {0};
		sprintf(szTemp, "%s   %s", szUserName, szTime);
		m_pWindow->InsertText(cfFormat, 0, szTemp);
        CharFontStyleToFormat(cfStyle, cfFormat);
		return m_pWindow->InsertText(cfFormat, DEFAULT_INDENT_CONTENT, szText);
	} else
		return FALSE;
}

BOOL CRichEdit2UI::LinkOnClick(char *szTitle, BOOL bIsCustom, DWORD dwFlag)
{
#ifdef _UNICODE
	TCHAR szTemp[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szTitle, szTemp, MAX_PATH);
	::ShellExecute(NULL, L"OPEN", szTemp, NULL, NULL, SW_SHOW);
#endif
	return TRUE;
}

#pragma warning(default:4996)