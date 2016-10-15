#include "common.h"

#include <UILib/UILabel.h>


/////////////////////////////////////////////////////////////////////////////////////
//
//

CLabelPanelUI::CLabelPanelUI(): 
	           m_uTextStyle(DT_VCENTER | DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS),
	           m_clrText(0),
	           m_clrBkgnd(0),
	           m_bBold(false),
	           m_bAutoEstimate(TRUE),
			   m_bLink(FALSE)
{
	SetBorder(FALSE);
}

void CLabelPanelUI::Init()
{
	CControlUI::Init();
	if( m_clrText == 0 ) 
		SetTextColor( UICOLOR_EDIT_TEXT_NORMAL ); 
	if( m_clrBkgnd == 0 )
		SetBkColor( UICOLOR__INVALID );
}

LPCTSTR CLabelPanelUI::GetClass() const
{
   return _T("LabelPanelUI");
}

void CLabelPanelUI::SetText(LPCTSTR pstrText)
{
   // Automatic assignment of keyboard shortcut
   if (_tcschr(pstrText, '&') != NULL)
	   m_chShortcut = *(_tcschr(pstrText, '&') + 1);
   CControlUI::SetText(pstrText);
}

 

void CLabelPanelUI::SetTextStyle(UINT uStyle)
{
   m_uTextStyle = uStyle;
   Invalidate();
}

void CLabelPanelUI::SetTextColor(UITYPE_COLOR TextColor)
{
	SetTextColor(m_pManager->GetThemeColor(TextColor));
}

void CLabelPanelUI::SetTextColor( COLORREF clr )
{
	m_clrText = clr;
	Invalidate();
}

void CLabelPanelUI::SetBkColor(UITYPE_COLOR BackColor)
{
	SetBkColor(m_pManager->GetThemeColor(BackColor));
}

void CLabelPanelUI::SetBkColor( COLORREF clr )
{
	m_clrBkgnd = clr;
	Invalidate();
}

void CLabelPanelUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcsicmp(pstrName, _T("horizonalign")) == 0) 
	{
		m_uTextStyle &= ~( DT_LEFT | DT_CENTER | DT_RIGHT );
		if (_tcsicmp(pstrValue, _T("center")) == 0)
		{
			m_uTextStyle |= DT_CENTER;
		} else if ( _tcsicmp(pstrValue, _T("right")) == 0){
			m_uTextStyle |= DT_RIGHT;
		} else
		{
			m_uTextStyle |= DT_LEFT;//left: default style
		}
	} else if (_tcsicmp(pstrName, _T("vertalign")) == 0)
	{
		//
	} else if (_tcsicmp(pstrName, _T("textColor")) == 0 )
	{
		SetTextColor(StringToColor(pstrValue));
	} else if (_tcsicmp(pstrName, _T("backColor")) == 0)
	{
		SetBkColor(StringToColor(pstrValue));
	} else if ( _tcsicmp(pstrName, _T("bold")) == 0)
	{
		SetBold(_tcsicmp(pstrValue, _T("true")) == 0);
	} else if (_tcsicmp(pstrName, _T("autoestimatesize")) == 0)
	{
		EnableAutoEstimateSize(_tcsicmp( pstrValue, _T("true")) == 0);
	} else if (_tcsicmp(pstrName, _T("end_ellipsis")) == 0)
	{
		EndEllipsis(_tcsicmp( pstrValue, _T("true")) == 0);
	} else if (_tcsicmp(pstrName, _T("link")) == 0)
	{
		m_bLink = (_tcsicmp(pstrValue, _T("true")) == 0);
	} else
	{
		CControlUI::SetAttribute(pstrName, pstrValue);
	}
}

void CLabelPanelUI::EndEllipsis( BOOL bEnable )
{
	if( bEnable )
		m_uTextStyle |= DT_END_ELLIPSIS;
	else
		m_uTextStyle &= ~DT_END_ELLIPSIS;
}

void CLabelPanelUI::EnableAutoEstimateSize( BOOL bEnable )
{
	m_bAutoEstimate = bEnable;
}

void CLabelPanelUI::SetBold( BOOL bBold )
{
	if (m_bBold != bBold)
	{
		m_bBold = bBold;
		Invalidate();
	}
}

SIZE CLabelPanelUI::EstimateSize(SIZE szAvailable)
{
	if ((!m_bAutoEstimate) || (!IsVisible()))
	{
		return CSize( 0, 0 );
	} else if (m_cxyFixed.cx)
	{
		//determined by user
		return CSize( m_cxyFixed.cx, 0 );
	} else
	{
		//determined by auto estimate
		RECT rcText = { 0, 0, szAvailable.cx, szAvailable.cy };
		//font
		UITYPE_FONT uiFont = UIFONT_NORMAL;
		if( m_bBold ) uiFont = UIFONT_BOLD;
		CBlueRenderEngineUI::DoPaintQuickText( m_pManager->GetPaintDC(), m_pManager, 
			rcText, m_sText, RGB(0,0,0), uiFont, DT_SINGLELINE | DT_CALCRECT );
		return CSize( rcText.right, rcText.bottom );
	}
}

void CLabelPanelUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	RECT rcTemp = {0};
	if (!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem)) 
		return;
	//border
	if ( HasBorder() ){
		CBlueRenderEngineUI::DoPaintRectangle( hDC, m_pManager, m_rcItem, 
			UICOLOR_TOOLBAR_BORDER, UICOLOR__INVALID );
	}
	//text
	UITYPE_FONT uiFont = UIFONT_NORMAL;
	if( m_bBold ) uiFont = UIFONT_BOLD;
	RECT rcText = m_rcItem;
	CBlueRenderEngineUI::DoPaintQuickText( hDC, m_pManager, 
		rcText, m_sText, m_clrText, uiFont, m_uTextStyle );
}
/////////////////////////////////////////////////////////////////////////////////////
//
//

LPCTSTR CGreyTextHeaderUI::GetClass() const
{
   return _T("GreyTextHeaderUI");
}

SIZE CGreyTextHeaderUI::EstimateSize(SIZE /*szAvailable*/)
{
   return CSize(0, 12 + m_pManager->GetThemeFontInfo(UIFONT_BOLD).tmHeight + 12);
}

void CGreyTextHeaderUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	RECT rcTemp = {0};
	if (!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem)) 
		return;
   COLORREF clrDarkText = m_pManager->GetThemeColor(UICOLOR_DIALOG_TEXT_DARK);
   RECT rcLine = { m_rcItem.left, m_rcItem.bottom - 6, m_rcItem.right, m_rcItem.bottom - 5 };
   CBlueRenderEngineUI::DoFillRect(hDC, m_pManager, rcLine, UICOLOR_DIALOG_TEXT_DARK, m_bTransparent);
   CBlueRenderEngineUI::DoPaintQuickText(hDC, m_pManager, m_rcItem, m_sText, UICOLOR_DIALOG_TEXT_DARK, UIFONT_BOLD, DT_SINGLELINE);
}

