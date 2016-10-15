#if !defined(AFX_UIPANEL_H__20060218_451A_298B_7A16_0080AD509054__INCLUDED_)
#define AFX_UIPANEL_H__20060218_451A_298B_7A16_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <UILib/UIList.h>
#include <UILib/UILabel.h>
#include <CommonLib/GdiPlusImage.h>
#include <Commonlib/SystemUtils.h>
/////////////////////////////////////////////////////////////////////////////////////
//

class  CTaskPanelUI : public CVerticalLayoutUI
{
public:
   CTaskPanelUI();
   ~CTaskPanelUI();

   enum { FADE_TIMERID = 10 };
   enum { FADE_DELAY = 500UL };

   LPCTSTR GetClass() const;

   void Event(TEventUI& event);
   void SetPos(RECT rc);
   SIZE EstimateSize(SIZE szAvailable);
   void DoPaint(HDC hDC, const RECT& rcPaint);
   
protected:
   HBITMAP m_hFadeBitmap;
   DWORD m_dwFadeTick;
   RECT m_rcFade;
};

/////////////////////////////////////////////////////////////////////////////////////
//

class CSearchTitlePanelUI : public CHorizontalLayoutUI
{
public:
   CSearchTitlePanelUI();

   LPCTSTR GetClass() const;

   void SetImage(int iIndex);

   void SetPos(RECT rc);
   void DoPaint(HDC hDC, const RECT& rcPaint);
   void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

protected:
   int m_iIconIndex;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class CPaddingPanelUI : public CControlUI
{
public:
   CPaddingPanelUI();
   CPaddingPanelUI(int cx, int cy);

   LPCTSTR GetClass() const; 
   SIZE EstimateSize(SIZE szAvailable);
   void DoPaint(HDC hDC, const RECT& rcPaint);
   void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

protected:
 
};


class CNormalImagePanelUI :public CControlUI
{
public:
	CNormalImagePanelUI();
	virtual ~CNormalImagePanelUI();

	LPCTSTR GetClass() const; 
	BOOL SetImageByFileName(const TCHAR *szFileExt);
	SIZE EstimateSize(SIZE szAvailable);
	void DoPaint(HDC hDC, const RECT& rcPaint);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
    void SetGray(BOOL bIsGray);
protected:
 
	IImageInterface *m_pGraph;
	BOOL m_bIsGray; //是否灰化图像
};
/////////////////////////////////////////////////////////////////////////////////////
//

class CImagePanelUI : public CControlUI
{
public:
	CImagePanelUI();
	virtual ~CImagePanelUI();

	LPCTSTR GetClass() const;

	void SetImage(int iIndex);
 
	BOOL SetImage(const TCHAR *szwName, UINT uType);
	BOOL SetImage(char *szFileName, UINT uImageType);
	BOOL SetImageByFileExt(const char *szFileExt);
	SIZE EstimateSize(SIZE szAvailable);
	void DoPaint(HDC hDC, const RECT& rcPaint);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
    void SetGray(BOOL bIsGray);
protected: 
	//皮肤库中定义的图片
	int m_iImageID;
	int m_iSubImageIndex;
	//application supplied image
	IImageInterface *m_pGraph;
	UINT m_nGraphType;
	BOOL m_bIsGray; //是否灰化图像
};


/////////////////////////////////////////////////////////////////////////////////////
//

class CTextPanelUI : public CLabelPanelUI
{
public:
   CTextPanelUI();

   LPCTSTR GetClass() const;
   UINT GetControlFlags() const;

   bool Activate();

   void Event(TEventUI& event);
   SIZE EstimateSize(SIZE szAvailable);
   void DoPaint(HDC hDC, const RECT& rcPaint);
   void SetPos(RECT rc);
   void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
   //operation
   void EnableLink( BOOL );
   UITYPE_FONT GetFont() const;
protected:
   int m_nLinks;//现在程序只支持一个link rect
   RECT m_rcLinks[1];
   UINT m_uButtonState;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class  CWarningPanelUI : public CTextPanelUI
{
public:
   CWarningPanelUI();

   LPCTSTR GetClass() const;

   void SetWarningType(UINT uType);

   SIZE EstimateSize(SIZE szAvailable);
   void DoPaint(HDC hDC, const RECT& rcPaint);  
   void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

protected:
   UITYPE_COLOR m_BackColor;
};


#endif // !defined(AFX_UIPANEL_H__20060218_451A_298B_7A16_0080AD509054__INCLUDED_)

