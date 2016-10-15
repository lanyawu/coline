#if !defined(AFX_UIDECORATION_H__20060218_5471_B259_0B2E_0080AD509054__INCLUDED_)
#define AFX_UIDECORATION_H__20060218_5471_B259_0B2E_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



/////////////////////////////////////////////////////////////////////////////////////
//

class  CTitleShadowUI : public CControlUI
{
public:
   LPCTSTR GetClass() const;
   SIZE EstimateSize(SIZE szAvailable);
   void DoPaint(HDC hDC, const RECT& rcPaint);
};


/////////////////////////////////////////////////////////////////////////////////////
//

class  CListHeaderShadowUI : public CControlUI
{
public:
   LPCTSTR GetClass() const;
   SIZE EstimateSize(SIZE szAvailable);
   void DoPaint(HDC hDC, const RECT& rcPaint);
};


/////////////////////////////////////////////////////////////////////////////////////
//

class  CSeparatorLineUI : public CControlUI
{
public:
   CSeparatorLineUI();
   LPCTSTR GetClass() const;
   SIZE EstimateSize(SIZE szAvailable);
   void DoPaint(HDC hDC, const RECT& rcPaint);
   void SetAttribute( LPCTSTR pstrName, LPCTSTR pstrValue );

   //operator
   void SetImage( UINT nImage );
   void SetLineHeight( UINT nHeight );
private:
	UINT m_nImageID;
	UINT m_nLineHeight;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class  CFadedLineUI : public CControlUI
{
public:
   LPCTSTR GetClass() const;
   SIZE EstimateSize(SIZE szAvailable);
   void DoPaint(HDC hDC, const RECT& rcPaint);
};



#endif // !defined(AFX_UIDECORATION_H__20060218_5471_B259_0B2E_0080AD509054__INCLUDED_)

