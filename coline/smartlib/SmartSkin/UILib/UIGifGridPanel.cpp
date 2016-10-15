#include "common.h"

#include <Commonlib/systemutils.h>
#include <UILib/UIGifGridPanel.h>
#include <UILib/UIGifImagePanel.h>
#include <UILib/UIButton.h>
#include <UILib/UIPanel.h>

#define GIF_PANEL_WIDTH  24
#define GIF_PANEL_HEIGHT 24
#pragma warning(disable:4996)
///////////////////////////////////////////////////////////////////
//
//用于显示动态gif图片的窗口
class CGifWindow : public CWindowWnd
{
public:
	CGifWindow() : m_pHotPanel( NULL ){ }

protected:
	LPCTSTR GetWindowClassName() const{ return _T("GifWindow"); }
	void OnFinalMessage(HWND hWnd){ 
		delete this;
	}
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam){
		switch(uMsg){
		case WM_CREATE:
			{
				m_pm.Init(m_hWnd);
				CWhiteCanvasUI* pCanvas = new CWhiteCanvasUI;
				m_pHotPanel = new CGifImagePanelUI;
				pCanvas->Add(m_pHotPanel);
				m_pm.AttachDialog(pCanvas);
				return 0;
			}
		default:
			break;
		}
		LRESULT lRes = 0;
		if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) 
			return lRes;
		return CWindowWnd::HandleMessage( uMsg, wParam, lParam );
	}

private:
	CPaintManagerUI m_pm;
	CGifImagePanelUI* m_pHotPanel;

	friend class CGifGridPanelUI;
};

///////////////////////////////////////////////////////////////////
//
//
CGifGridPanelUI::CGifGridPanelUI()
	: m_nHCount(1),
	  m_nVCount(1),
	  m_nCurGif( -1 ),
	  m_nGifPerLine( 8 ),
	  m_bInit( false ),
	  m_pGifWnd( NULL )
{
	m_rcGridArea.left = m_rcGridArea.top = m_rcGridArea.right = m_rcGridArea.bottom = 0;
	m_szGifWnd.cx = GIF_PANEL_WIDTH;
	m_szGifWnd.cy = GIF_PANEL_HEIGHT;
	m_szFrame.cx = m_szFrame.cy = 2;
}

CGifGridPanelUI::~CGifGridPanelUI()
{
}

//INotifyUI overridable
void CGifGridPanelUI::Notify(TNotifyUI& msg)
{
	if (msg.sType == _T("click"))
	{
		if (m_pManager)
			m_pManager->SendNotify(this, _T("itemclick")); 
	} else if (msg.sType == _T("mouseenter"))
	{
		CGifImagePanelUI* pGif = dynamic_cast<CGifImagePanelUI*>(msg.pSender); 
		//from child image panel
		SelectGif( pGif );
	}
}

BOOL CGifGridPanelUI::AddGifUI(const char *szFileName, const char *szTag, const char *szCut, const char *szTip)
{ 
	CGifImagePanelUI *pUI = new CGifImagePanelUI();
	if (pUI)
	{
		if (Add(pUI, 99999))
		{
			TCHAR szwTemp[MAX_PATH] = {0}; 
			CStringConversion::StringToWideChar(szFileName, szwTemp, MAX_PATH - 1);
			pUI->SetImage(szwTemp, FALSE, 0, false);

			//set tag
			memset(szwTemp, 0, sizeof(TCHAR) * MAX_PATH);
			CStringConversion::StringToWideChar(szTag, szwTemp, MAX_PATH - 1);
			pUI->SetGifTag(szwTemp);

			//set shortcut
			memset(szwTemp, 0, sizeof(TCHAR) * MAX_PATH);
			CStringConversion::StringToWideChar(szCut, szwTemp, MAX_PATH - 1);
			pUI->SetGifShortcut(szwTemp);

			//set comment
			memset(szwTemp, 0, sizeof(TCHAR) * MAX_PATH);
			CStringConversion::StringToWideChar(szTip, szwTemp, MAX_PATH - 1);
			pUI->SetToolTip(szwTemp);	
			//
			pUI->SetBorder(true);
			return TRUE;
		} else 
			delete pUI;
	}
	return FALSE;
}

void CGifGridPanelUI::InitEmotion()
{
	if ((!m_strEmotionName.empty()) && CSystemUtils::FileIsExists(m_strEmotionName.c_str()))
	{
		TiXmlDocument xmlDoc;
		char szEmotionPath[MAX_PATH] = {0};
		CSystemUtils::ExtractFilePath(m_strEmotionName.c_str(), szEmotionPath, MAX_PATH - 1);
		if (xmlDoc.LoadFile(m_strEmotionName.c_str()))
		{ 
			TiXmlElement *pRoot = xmlDoc.RootElement();
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
						AddGifUI(strEmotionFileName.c_str(), pEmotion->Attribute("tag"), pEmotion->Attribute("comment"),
							pEmotion->Attribute("shortcut")); 
					    //insert into tables
						strTag = "/{";
						strTag += pEmotion->Attribute("tag");
						strTag += "/}";
						strCut = pEmotion->Attribute("shortcut");
						m_EmotionTables.insert(std::pair<std::string, std::string>(strTag, strCut));
						pEmotion = pEmotion->NextSiblingElement();
					} //end while (pEmotion)
				} //end if (pSysList)
			} //end if (pRoot &&
		} //end if (xmlDoc  
	} //end if (m_strEmotionName.c_str()...
}

void CGifGridPanelUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (::lstrcmpi(pstrName, L"EmotionFile") == 0)
	{
		char szTmp[MAX_PATH] = {0};
		CStringConversion::WideCharToString(pstrValue, szTmp, MAX_PATH - 1);
		m_strEmotionName = szTmp;
		InitEmotion();
	}
	CContainerUI::SetAttribute(pstrName, pstrValue);
}

//CControlUI overridable
LPCTSTR CGifGridPanelUI::GetClass() const
{
	return _T("GifGridPanelUI");
}

void CGifGridPanelUI::Init()
{
	CContainerUI::Init();
	InitEmotion();
}

#define PREV_SHOW_GIF_WINDOW_RATIO  3  //
//计算rect的时候牵涉到很多问题，如果Grid的足够长就没有问题
//最好一行不少于5个图片
void CGifGridPanelUI::SelectGif(CGifImagePanelUI* pHotGif)
{
	if (!m_bInit)
		return;
	if (!pHotGif)
		return ;
	UINT nIndex = pHotGif->GetIndex();
	if (m_nCurGif != nIndex)
	{
		m_nCurGif = nIndex;
		//calc hot rectangle
		RECT rcHotGif = { 0 };
		rcHotGif.top = m_rcGridArea.top;
		rcHotGif.bottom = LONG(m_rcGridArea.top + m_szGifWnd.cy * PREV_SHOW_GIF_WINDOW_RATIO);
		if ((nIndex % m_nHCount) <= (m_nHCount / 2))
		{
			rcHotGif.right = m_rcGridArea.right;
			rcHotGif.left = LONG( rcHotGif.right - PREV_SHOW_GIF_WINDOW_RATIO * m_szGifWnd.cx );
		} else
		{
			rcHotGif.left = m_rcGridArea.left;
			rcHotGif.right = LONG(m_rcGridArea.left + m_szGifWnd.cx * PREV_SHOW_GIF_WINDOW_RATIO);
		}

		if (m_pGifWnd == NULL)
		{
			//create window
			m_pGifWnd = new CGifWindow;
			m_pGifWnd->Create(m_pManager->GetPaintWindow(), _T("GifWnd"),
				WS_CHILD | WS_BORDER, WS_EX_TOOLWINDOW );
		}
		m_pGifWnd->m_pHotPanel->SetImage(pHotGif->GetGifFileName(),  FALSE, 0);		
		m_pGifWnd->m_pHotPanel->SetGifPadding(6);
		::MoveWindow(m_pGifWnd->GetHWND(), rcHotGif.left, rcHotGif.top,
			rcHotGif.right - rcHotGif.left, rcHotGif.bottom - rcHotGif.top,
			TRUE);
		m_pGifWnd->ShowWindow(true);
		pHotGif->GetGifFileName(m_strSelGifName);
		pHotGif->GetGifTag(m_strSelGifTag);
		Invalidate();
	}
}
 

void CGifGridPanelUI::Event(TEventUI& e)
{
	switch( e.Type )
	{
	case UIEVENT_MOUSELEAVE:
		//将m_rcItem换成panel区域会更好
		if (!::PtInRect(&m_rcItem, e.ptMouse))
		{
			if (m_pGifWnd)
			{
				m_pGifWnd->ShowWindow(false);
				m_pGifWnd->m_pHotPanel->ClearImage();
				m_nCurGif = -1;
			}
			Invalidate();
		}
		break;
	case UIEVENT_KILLFOCUS:
		{
			if (m_pGifWnd)
			{
				m_pGifWnd->ShowWindow(false);
				m_pGifWnd->m_pHotPanel->ClearImage();
				m_nCurGif = -1;
			}
			Invalidate();
		}
	default:
		break;
	}
	CContainerUI::Event(e);
}

void CGifGridPanelUI::InitChildrenPos()
{
	m_bInit = false;
	m_nHCount = m_nVCount = 0;
	::SetRect(&m_rcGridArea, 0, 0, 0, 0);

	if (m_ChildList.GetSize() > 0)
	{
		//we assume all the gif images have the same extension
		CGifImagePanelUI* pGif = dynamic_cast<CGifImagePanelUI*>(m_ChildList[0]);
		ASSERT( pGif != 0 );

		//计算区域内每行没列实际的gif个数
		int cxGrid = m_rcItem.right - m_rcItem.left;
		int cyGrid = m_rcItem.bottom - m_rcItem.top;
		m_nHCount = ( m_szGifWnd.cx != 0 ? cxGrid / m_szGifWnd.cx : 0 );
		m_nVCount = ( m_szGifWnd.cy != 0 ? cyGrid / m_szGifWnd.cy : 0 );
		//把gif区域居中在GifGridPanel容器中
		if (m_nHCount && m_nVCount)
		{ 
			int cxPadding = ( cxGrid % m_szGifWnd.cx ) / 2;
			int cyPadding = ( cyGrid % m_szGifWnd.cy ) / 2;
			::CopyRect( &m_rcGridArea, &m_rcItem );
			::InflateRect( &m_rcGridArea, -cxPadding, -cyPadding );
			m_bInit = true;
		}
	}
}

void CGifGridPanelUI::SetPos(RECT rc)
{
	CControlUI::SetPos( rc ); 
	GetShowGifPanelCount();
	InitChildrenPos();

	if (m_bInit)
	{
		//calculation each gif panels position
		for (int i = 0; i < m_ChildList.GetSize(); ++i)
		{
			unsigned int ixIndex = i % m_nHCount;
			unsigned int iyIndex = i / m_nHCount;
			if (iyIndex >= m_nVCount)
				break;

			CRect rcGif(m_rcGridArea.left + ixIndex * m_szGifWnd.cx + m_szFrame.cx, 
				m_rcGridArea.top + iyIndex * m_szGifWnd.cy + m_szFrame.cy,
				m_rcGridArea.left + ( ixIndex + 1 ) * m_szGifWnd.cx - m_szFrame.cx,
				m_rcGridArea.top + ( iyIndex + 1 ) * m_szGifWnd.cy - m_szFrame.cy);

			CGifImagePanelUI* pCtrl = dynamic_cast<CGifImagePanelUI*>( m_ChildList[i] );
			ASSERT( pCtrl != NULL );
			pCtrl->SetPos( rcGif );
		}
	}
}

int  CGifGridPanelUI::GetShowGifPanelCount()
{
	m_szGifWnd.cx = GIF_PANEL_WIDTH;
	m_szGifWnd.cy = GIF_PANEL_HEIGHT; //16 + 1v
	int m_nGifPerLine = (m_rcItem.right - m_rcItem.left) / m_szGifWnd.cx;
	int nRow = (m_rcItem.bottom - m_rcItem.top) / m_szGifWnd.cy;
	return (m_nGifPerLine * nRow);
}

SIZE CGifGridPanelUI::EstimateSize(SIZE szAvailable)
{
	/*CControlUI* pGif = static_cast<CControlUI*>(m_items[0]);
	if ((pGif != NULL) && (m_nGifPerLine >= 5))
	{
		SIZE szAvail = { 9999, 9999 };
		m_szGifWnd = pGif->EstimateSize(szAvail);
		
		int iLineCount = m_items.GetSize() / m_nGifPerLine;
		if ((m_items.GetSize() % m_nGifPerLine) != 0)
			++ iLineCount;

		return CSize( m_szGifWnd.cx * m_nGifPerLine, m_szGifWnd.cy * iLineCount);
	}
	return CSize( 0, 0 );*/
	return CContainerUI::EstimateSize(szAvailable);
}

void CGifGridPanelUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	CRect rcFill( 0, 0, 0, 0 );
	if (!m_bInit || !::IntersectRect(&rcFill, &m_rcItem, &rcPaint))
		return;

	//gif image panels
	CContainerUI::DoPaint( hDC, rcPaint );

	//画线
	//calculation each gif panels position
	RECT rcLine;
	rcLine.left = m_rcGridArea.left;
	rcLine.right = m_rcGridArea.right;
	for (int i = 0; i <= m_nVCount; i ++ )
	{
		rcLine.top = m_rcGridArea.top + i * m_szGifWnd.cy + m_szFrame.cy / 2;
		rcLine.bottom = rcLine.top;
		CBlueRenderEngineUI::DoPaintLine(hDC, rcLine, UICOLOR_GIFGRIDPANEL_SEPLINE);
	}
	 
	rcLine.top = m_rcGridArea.top;
	rcLine.bottom = m_rcGridArea.bottom;
	for (int i = 0; i <= m_nHCount; i ++)
	{
		rcLine.left = m_rcGridArea.left + i * m_szGifWnd.cx + m_szFrame.cx / 2;
		rcLine.right = rcLine.left;
		CBlueRenderEngineUI::DoPaintLine(hDC, rcLine, UICOLOR_GIFGRIDPANEL_SEPLINE);
	}
	 
	//the focus gif rect
	if (m_nCurGif != -1)
	{
		CControlUI* pHotUI = dynamic_cast<CControlUI*>(m_ChildList[m_nCurGif]);
		ASSERT( pHotUI != NULL );
		CRect rcHot = pHotUI->GetPos();
		::InflateRect( &rcHot, -1, -1 );
		CBlueRenderEngineUI::DoPaintRectangle(hDC, m_pManager,
			rcHot, UICOLOR_STANDARD_BLACK, UICOLOR__INVALID );
	}
}

bool CGifGridPanelUI::Add(CControlUI* pControl, const int nIdx)
{
	CGifImagePanelUI* pGif = dynamic_cast<CGifImagePanelUI*>(pControl);
	if (pGif)
	{
		pGif->SetIndex(m_ChildList.GetSize());
		return CContainerUI::Add(pControl);
	}
	return false;
}

void CGifGridPanelUI::DeleteChildWnd()
{
	if (m_pGifWnd)
	{
		delete m_pGifWnd;
		m_pGifWnd = NULL;
	}
	m_nCurGif = -1;
}

HWND CGifGridPanelUI::GetFloatingWnd() const
{
	if (m_pGifWnd != NULL)
		return m_pGifWnd->GetHWND();
	return NULL;
}

BOOL CGifGridPanelUI::GetSelGifInfo(char *szGifName, int *nNameSize, char *szGifTag, int *nTagSize)
{
	if (szGifName && szGifTag && nNameSize && nTagSize 
		&& (m_strSelGifName.size() > 0) && (m_strSelGifTag.size() > 0)
		&& (*nNameSize >= m_strSelGifName.size()) 
		&& (*nTagSize >= m_strSelGifTag.size()))
	{
		strcpy(szGifName, m_strSelGifName.c_str());
		strcpy(szGifTag, m_strSelGifTag.c_str());
		*nNameSize = m_strSelGifName.size();
		*nTagSize = m_strSelGifTag.size();
		return TRUE;
	}
	return FALSE;
}

#pragma warning(default:4996)
