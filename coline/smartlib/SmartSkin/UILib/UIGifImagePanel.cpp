#include "common.h"

#include <UILib/UIGifImagePanel.h>
#include <CommonLib/StringUtils.h>

///////////////////////////////////////////////////////////////////
//
//
CGifImagePanelUI::CGifImagePanelUI(void):
	              m_pGif(NULL),
	              m_nGifPadding(0),
				  m_bLink(FALSE),
				  m_bAnimation(TRUE),
	              m_sGifFile(_T(""))
{ 
	CGdiPlusImage::InitGdiPlus();
}

CGifImagePanelUI::~CGifImagePanelUI(void)
{
	if (m_pGif)
		delete m_pGif;
	m_pGif = NULL;
	CGdiPlusImage::DestroyGdiPlus();
}

BOOL CGifImagePanelUI::GetGifFileName(std::string &strFileName)
{
	char szTmp[MAX_PATH] = {0};
	if (m_sGifFile.GetData(szTmp, MAX_PATH - 1))
	{
		strFileName = szTmp;
		return TRUE;
	}
	return FALSE;
}

BOOL CGifImagePanelUI::GetGifTag(std::string &strTag)
{
	char szTmp[MAX_PATH] = {0};
	if (m_sGifTag.GetData(szTmp, MAX_PATH - 1))
	{
		strTag = szTmp;
		return TRUE;
	}
	return FALSE;
}

UINT CGifImagePanelUI::GetControlFlags() const
{
	return m_bLink ? UIFLAG_SETCURSOR : 0;
}

CStdString CGifImagePanelUI::GetGifFileName() const
{ 
	return m_sGifFile; 
}

void CGifImagePanelUI::SetGifTag(LPCTSTR lpszTag)
{ 
	m_sGifTag = lpszTag; 
}

CStdString CGifImagePanelUI::GetGifTag() const 
{ 
	return m_sGifTag; 
}

void CGifImagePanelUI::SetGifShortcut(LPCTSTR lpszShortcut)
{ 
	m_sShortcut = lpszShortcut; 
}

CStdString CGifImagePanelUI::GetGifShortcut() const 
{ 
	return m_sShortcut; 
}
void CGifImagePanelUI::SetIndex(UINT nIndex)
{ 
	m_nIndex = nIndex; 
}

UINT CGifImagePanelUI::GetIndex() const
{ 
	return m_nIndex; 
}

void CGifImagePanelUI::SetGifPadding(UINT nPadding)
{ 
	m_nGifPadding = nPadding; 
}

LPCTSTR CGifImagePanelUI::GetClass() const
{
	return L"GIFIMAGEPANELUI";
}

void CGifImagePanelUI::Notify(TNotifyUI& msg)
{
	if (msg.sType == _T("click") || msg.sType == _T("mouseenter"))
	{
		if (m_pParent)
		{
			m_pParent->Notify(msg);
		} //end if (m_pParent)
	} //end if (msg.sType == _T("click")
}

void CGifImagePanelUI::Event(TEventUI& e)
{
	switch(e.Type)
	{
	    case UIEVENT_SETCURSOR:
			{
				if (m_bLink)
				{
					::PtInRect(&m_rcItem, e.ptMouse);
					{
						::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND))); 
					}
				} //end if (m_bLink)
			}  //end case UIEVENT_SETCURSOR
			return;
		case UIEVENT_BUTTONUP:
			if (::PtInRect(&m_rcItem, e.ptMouse))
			{
				m_pManager->SendNotify(this, _T("click"));
			}
			break;
		case UIEVENT_MOUSEENTER:
			m_pManager->SendNotify(this, _T("mouseenter"));
			break;
		default:
			break;
	}
	CControlUI::Event(e);
}

SIZE CGifImagePanelUI::EstimateSize(SIZE szAvailable)
{
	if ((m_cxyFixed.cx != 0) || (m_cxyFixed.cy != 0))
		return m_cxyFixed;
	return CSize(m_ImageSize.cx + 2, m_ImageSize.cy + 2);
}

void CGifImagePanelUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	RECT rcTemp = {0};
	if (!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem)) 
		return;
	if (m_pGif)
	{
		
		//将图片居中在magePanel中
		int cxPanel = m_rcItem.right - m_rcItem.left;
		int cyPanel = m_rcItem.bottom - m_rcItem.top;

		CRect rcGif;
		rcGif.left = max(m_rcItem.left, m_rcItem.left + (cxPanel - m_ImageSize.cx) / 2) ;
		rcGif.top = max(m_rcItem.top, m_rcItem.top + (cyPanel - m_ImageSize.cy) / 2);
		rcGif.right = min(m_rcItem.right, m_rcItem.right - (cxPanel - m_ImageSize.cx) / 2) ;
		rcGif.bottom = min(m_rcItem.bottom, m_rcItem.bottom - (cyPanel - m_ImageSize.cy) / 2);
 
		m_pGif->Paint(hDC, rcGif);
	}
}

void CGifImagePanelUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
   if (_tcsicmp(pstrName, _T("image")) == 0)  
	   SetImage(pstrValue, FALSE, 0);
   else if (_tcsicmp(pstrName, _T("imagefile")) == 0)
   {
	   //
   } else if (_tcsicmp(pstrName, _T("enablelink")) == 0)
   {
	   m_bLink = (_tcsicmp(pstrValue, _T("true")) == 0);
   } else
   {
	   CControlUI::SetAttribute(pstrName, pstrValue);
   }
}

void CGifImagePanelUI::Init()
{
	CControlUI::Init();
	TCHAR szwTmp[MAX_PATH] = {0};
	int nId = _ttol(m_sGifFile.GetData());
	LPUI_IMAGE_ITEM Item = NULL;
	if ((nId != 0) && m_pManager && m_pManager->GetImage(nId, &Item))
	{
		CStringConversion::StringToWideChar(Item->m_strFileName.c_str(), szwTmp, MAX_PATH - 1);
		m_sGifFile = szwTmp;
	} else
		::lstrcpy(szwTmp, m_sGifFile.GetData());
 
	ClearImage();
#ifdef _UNICODE
	char szTemp[MAX_PATH] = {0};
	CStringConversion::WideCharToString(szwTmp, szTemp, MAX_PATH);
	m_pGif = new CGdiPlusGif(this, szTemp, m_bAnimation);
	m_sGifFile = szwTmp;
#else
	m_pGif = new CGdiPlusGif(this, lpszFileName, bAnimation);
	m_sGifFile = lpszFileName;
#endif
	m_ImageSize.cx = m_pGif->GetImageWidth();
	m_ImageSize.cy = m_pGif->GetImageHeight(); 
	m_pGif->SetTransparent(m_bTransparent, m_crTransparent);
	UpdateLayout();
}

bool CGifImagePanelUI::SetImage(LPCTSTR lpszFileName, BOOL bTransparent, COLORREF crTransparent, bool bAnimation)
{
	m_bAnimation = bAnimation;
	m_sGifFile = lpszFileName;
	m_crTransparent = crTransparent;
	m_bTransparent = bTransparent;
	if (m_pManager)
	{
		Init();
		return true;
	}
	return false;
}

bool CGifImagePanelUI::SetImage(char *szFileName, BOOL bTransparent, COLORREF crTransparent, bool bAnimation)
{	
	m_bAnimation = bAnimation; 
	m_crTransparent = crTransparent;
	m_bTransparent = bTransparent;
	TCHAR szwTmp[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szFileName, szwTmp, MAX_PATH - 1);
	m_sGifFile = szwTmp;
	if (m_pManager)
	{
		Init();
	} 
	return true;
}

void CGifImagePanelUI::ClearImage()
{
	if (m_pGif)
	{
		delete m_pGif;
		m_pGif = NULL;
	}
	m_sGifFile.Empty();
}

void CGifImagePanelUI::OnInvalidate(LPRECT lprc)
{
	Invalidate();
}