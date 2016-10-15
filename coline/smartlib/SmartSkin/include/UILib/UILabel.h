#if !defined(AFX_UILABEL_H__20060218_34CC_2871_036E_0080AD509054__INCLUDED_)
#define AFX_UILABEL_H__20060218_34CC_2871_036E_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////////////////
//

class CLabelPanelUI : public CControlUI
{
public:
   CLabelPanelUI();

   LPCTSTR GetClass() const;

   void SetText(LPCTSTR pstrText);
    
   void SetTextStyle(UINT uStyle);
   UINT GetTextStyle() const { return m_uTextStyle; }
   void SetTextColor(UITYPE_COLOR TextColor);
   void SetTextColor( COLORREF );
   void SetBkColor(UITYPE_COLOR BackColor);
   void SetBkColor( COLORREF );

   void Init();
   SIZE EstimateSize(SIZE szAvailable);
   void DoPaint(HDC hDC, const RECT& rcPaint);
   void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

   void SetBold( BOOL bBold );
   void EnableAutoEstimateSize( BOOL bEnable );
   void EndEllipsis( BOOL bEnable );
protected: 
   UINT m_uTextStyle;
   BOOL m_bLink;
   BOOL m_bBold;
   COLORREF m_clrText;
   COLORREF m_clrBkgnd;
   BOOL m_bAutoEstimate;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class CGreyTextHeaderUI : public CControlUI
{
public:
   LPCTSTR GetClass() const;
   SIZE EstimateSize(SIZE szAvailable);
   void DoPaint(HDC hDC, const RECT& rcPaint);
};


#endif // !defined(AFX_UILABEL_H__20060218_34CC_2871_036E_0080AD509054__INCLUDED_)

