#include "common.h"

#include <UILib/UIDecoration.h>

/////////////////////////////////////////////////////////////////////////////////////
//
//

LPCTSTR CTitleShadowUI::GetClass() const
{
	return _T("TitleShadowUI");
}

SIZE CTitleShadowUI::EstimateSize(SIZE /*szAvailable*/)
{
	return CSize(0, 3);
}

void CTitleShadowUI::DoPaint(HDC hDC, const RECT& /*rcPaint*/)
{
	COLORREF clrBack1 = m_pManager->GetThemeColor(UICOLOR_TITLE_BACKGROUND);
	COLORREF clrBack2 = m_pManager->GetThemeColor(UICOLOR_DIALOG_BACKGROUND);
	RECT rcTop = { m_rcItem.left, m_rcItem.top, m_rcItem.right, m_rcItem.top + 4 };
	CBlueRenderEngineUI::DoPaintGradient(hDC, m_pManager, rcTop, clrBack1, clrBack2, true, 4);
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

LPCTSTR CListHeaderShadowUI::GetClass() const
{
	return _T("ListHeaderShadowUI");
}

SIZE CListHeaderShadowUI::EstimateSize(SIZE /*szAvailable*/)
{
	return CSize(0, 3);
}

void CListHeaderShadowUI::DoPaint(HDC hDC, const RECT& /*rcPaint*/)
{
	COLORREF clrBack1, clrBack2;
	m_pManager->GetThemeColorPair(UICOLOR_HEADER_BACKGROUND, clrBack1, clrBack2);
	RECT rcTop = { m_rcItem.left + 1, m_rcItem.top, m_rcItem.right - 1, m_rcItem.top + 4 };
	CBlueRenderEngineUI::DoPaintGradient(hDC, m_pManager, rcTop, clrBack2, clrBack1, true, 8);
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

LPCTSTR CFadedLineUI::GetClass() const
{
	return _T("FadedLineUI");
}

SIZE CFadedLineUI::EstimateSize(SIZE /*szAvailable*/)
{
	return CSize(0, 10);
}

void CFadedLineUI::DoPaint(HDC hDC, const RECT& /*rcPaint*/)
{
	COLORREF clrLine = m_pManager->GetThemeColor(UICOLOR_NAVIGATOR_BORDER_NORMAL);
	COLORREF clrDialog = m_pManager->GetThemeColor(UICOLOR_DIALOG_BACKGROUND);
	int iOffset = (m_rcItem.bottom - m_rcItem.top) / 2;
	RECT rc1 = { m_rcItem.left, m_rcItem.top + iOffset, m_rcItem.right - 120, m_rcItem.top + iOffset };
	CBlueRenderEngineUI::DoPaintLine(hDC, m_pManager, rc1, UICOLOR_NAVIGATOR_BORDER_NORMAL);
	RECT rc2 = { m_rcItem.right - 120, m_rcItem.top + iOffset, m_rcItem.right - 40, m_rcItem.top + iOffset };
	CBlueRenderEngineUI::DoPaintGradient(hDC, m_pManager, rc2, clrLine, clrDialog, false, 16);
}


/////////////////////////////////////////////////////////////////////////////////////
//
//
CSeparatorLineUI::CSeparatorLineUI(): 
                  m_nImageID(IMGID_INVALID_),
				  m_nLineHeight(0)
{
}

LPCTSTR CSeparatorLineUI::GetClass() const
{
	return _T("SeparatorLineUI");
}

SIZE CSeparatorLineUI::EstimateSize(SIZE /*szAvailable*/)
{
	return CSize(0, 12);
}

void CSeparatorLineUI::SetImage( UINT nImage )
{
	if (m_nImageID != nImage)
	{
		m_nImageID = nImage;
		Invalidate();
	} //end if (m_nImageId != nImage)
}

void CSeparatorLineUI::SetLineHeight(UINT nHeight)
{
	if (m_nLineHeight != nHeight)
	{
		m_nLineHeight = nHeight;
		Invalidate();
	} //end if (m_nLineHeight...
}

void CSeparatorLineUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcsicmp(pstrName, _T("image")) == 0)
	{
		SetImage(_ttoi( pstrValue));
	} else if (_tcsicmp(pstrName, _T("linewidth")) == 0)
	{
		SetLineHeight(_ttoi(pstrValue));
	} else
	{
		CControlUI::SetAttribute(pstrName, pstrValue);
	}
}

void CSeparatorLineUI::DoPaint(HDC hDC, const RECT& /*rcPaint*/)
{
	if (m_nImageID != IMGID_INVALID_)
	{
		int iOffset = (m_rcItem.bottom - m_rcItem.top - m_nLineHeight) / 2;
		//draw it to the middle of this ui item
		RECT rcLine = { m_rcItem.left, m_rcItem.top + iOffset, m_rcItem.right, m_rcItem.top + iOffset + m_nLineHeight };
		CBlueRenderEngineUI::DoPaintGraphic(hDC, m_pManager, rcLine, m_nImageID);
	} else
	{
		//default separator line
		int iOffset = (m_rcItem.bottom - m_rcItem.top) / 2;
		RECT rc1 = { m_rcItem.left, m_rcItem.top + iOffset, m_rcItem.right - 1, m_rcItem.top + iOffset };
		CBlueRenderEngineUI::DoPaintLine(hDC, rc1, m_clrBorder);
		RECT rc2 = { m_rcItem.left, m_rcItem.top + iOffset + 1, m_rcItem.right, m_rcItem.top + iOffset + 1 };
		CBlueRenderEngineUI::DoPaintLine(hDC, rc1, m_clrBorder);
	}
}

