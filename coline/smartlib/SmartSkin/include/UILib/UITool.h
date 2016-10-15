#if !defined(AFX_UITOOL_H__20060218_57EB_12A7_9A10_0080AD509054__INCLUDED_)
#define AFX_UITOOL_H__20060218_57EB_12A7_9A10_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <UILib/UIButton.h>
#include <UILib/UIContainer.h>
/////////////////////////////////////////////////////////////////////////////////////
//

class  CToolbarUI : public CHorizontalLayoutUI
{
public:
   CToolbarUI();

   LPCTSTR GetClass() const;
   SIZE EstimateSize(SIZE szAvailable);
   void DoPaint(HDC hDC, const RECT& rcPaint);
   void SetAttribute( LPCTSTR pstrName, LPCTSTR pstrValue );
};


/////////////////////////////////////////////////////////////////////////////////////
//

class  CToolbarTitlePanelUI : public CControlUI
{
public:
   CToolbarTitlePanelUI();

   void SetPadding(int iPadding);

   LPCTSTR GetClass() const;   
   SIZE EstimateSize(SIZE szAvailable);
   void DoPaint(HDC hDC, const RECT& rcPaint);

protected:
   int m_iPadding;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class  CToolSeparatorUI : public CControlUI
{
public:
   LPCTSTR GetClass() const;
   SIZE EstimateSize(SIZE szAvailable);
   void DoPaint(HDC hDC, const RECT& rcPaint);
};


/////////////////////////////////////////////////////////////////////////////////////
//

class  CToolGripperUI : public CControlUI
{
public:
   LPCTSTR GetClass() const;
   SIZE EstimateSize(SIZE szAvailable);
   void DoPaint(HDC hDC, const RECT& rcPaint);
};


/////////////////////////////////////////////////////////////////////////////////////
//

class  CToolButtonUI : public CButtonUI
{
public:
   CToolButtonUI();

   LPCTSTR GetClass() const;

   void DoPaint(HDC hDC, const RECT& rcPaint);
};


/////////////////////////////////////////////////////////////////////////////////////
//

class  CStatusbarUI : public CContainerUI
{
public:
   LPCTSTR GetClass() const;

   SIZE EstimateSize(SIZE szAvailable);
   void DoPaint(HDC hDC, const RECT& rcPaint);
};




#endif // !defined(AFX_UITOOL_H__20060218_57EB_12A7_9A10_0080AD509054__INCLUDED_)

