#if !defined(AFX_UICONTAINER_H__20060218_C077_501B_DC6B_0080AD509054__INCLUDED_)
#define AFX_UICONTAINER_H__20060218_C077_501B_DC6B_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <UILib/UIBlue.h>
/////////////////////////////////////////////////////////////////////////////////////
//
class CControlList
{
public:
   CControlList(int iPreallocSize = 0);
   virtual ~CControlList();

   void Empty();
   void Resize(int iSize);
   bool IsEmpty() const;
   int Find(CControlUI * iIndex) const;
   bool Add(CControlUI * pData);
   bool SetAt(int iIndex, CControlUI * pData);
   bool InsertAt(int iIndex, CControlUI * pData);
   bool Remove(const int iIndex);
   int GetSize() const;
   CControlUI ** GetData();

   CControlUI * GetAt(int iIndex) const;
   CControlUI * operator[] (int nIndex) const;

protected:
   CControlUI ** m_ppVoid;
   int m_nCount;
   int m_nAllocated;
};

class IContainerUI
{
public:
   virtual CControlUI* GetItem(const int iIndex) const = 0;
   virtual int GetCount() const = 0;
   virtual bool Add(CControlUI* pControl, const int nIdx) = 0;
   virtual bool Remove(CControlUI* pControl) = 0;
   virtual CControlUI* GetChildContrlByClass(LPCTSTR pstrClassName) = 0;
   virtual void AdjustLayout(int nDis, BOOL bVert, CControlUI *pUI) = 0;
   virtual void RemoveAll() = 0;
};


/////////////////////////////////////////////////////////////////////////////////////
//
class CScrollBarUI;
class  CContainerUI : public CControlUI, public IContainerUI
{
public:
   CContainerUI();
   virtual ~CContainerUI();

public:
   LPCTSTR GetClass() const;
   LPVOID GetInterface(LPCTSTR pstrName);

   virtual CControlUI * GetItem(const int iIndex) const;
   virtual int GetCount() const;
   virtual bool Add(CControlUI* pControl, const int nIdx = 999999);
   virtual CControlUI* GetChildContrlByClass(LPCTSTR pstrClassName);
   virtual bool Remove(CControlUI* pControl);
   virtual void AdjustLayout(int nDis, BOOL bVert, CControlUI *pUI);
   virtual void RemoveAll();

   void Event(TEventUI& event);
   void ParentChangeVisible(bool bVisible);
   void SetVisible(bool bVisible);
   virtual bool IsVisible() const;
   virtual void Init();
//   virtual void SetInset(SIZE szInset);
//   virtual void SetInset(RECT rcInset);
   virtual void SetPadding(int iPadding); 
   virtual void SetAutoDestroy(bool bAuto);

   virtual int FindSelectable(int iIndex, bool bForward = true) const;

   void SetPos(RECT rc);
   SIZE EstimateSize(SIZE szAvailable);
   void DoPaint(HDC hDC, const RECT& rcPaint);

   virtual BOOL GetAttribute(LPCTSTR pstrName, TCHAR *szValue, int &nMaxValueSize);
   virtual void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

   void SetManager(CPaintManagerUI* pManager, CControlUI* pParent);
   CControlUI* FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags);

public:
	void EnableScrollBar( UINT nScrollBarType, bool bEnable );
	virtual bool IsScrollBarVisible( UINT nScrollBar ) const;
	virtual void SetScrollBarPos( UINT nScrollBarType, const RECT& rc );
	virtual void SetScrollPos( UINT nScrollBarType, int iPos );
	virtual int GetScrollPos( UINT nScrollBar ) const;
	virtual void SetScrollRange( UINT nScrollBarType, int iMin, int iMax );
	virtual BOOL GetScrollRange( UINT nScrollBarType, int& iMin, int& iMax ) const;
	virtual void SetScrollPage( UINT nScrollBarType, int iPage );
	virtual int GetScrollPage( UINT nScrollBarType ) const;
	virtual void DoPaintScrollBar( UINT nScrollBarType, HDC hDC, const RECT& rcPaint );

	CScrollBarUI *GetVerticalScrollBar();
	CScrollBarUI *GetHorizontalScrollBar();
protected:
    CControlList m_ChildList;
    int m_iPadding; 
    bool m_bAutoDestroy;
	
	UINT m_nBkgImageId;
	BOOL m_bHole;
	int m_nBkgLeftImageId;
	int m_nBkgRightImageId;
	UINT m_nStretchMode;
	StretchFixed m_stretchFixed;
	SIZE m_szLeft;
	SIZE m_szRight;
protected:
	CScrollBarUI* m_pVScrollBar;
	CScrollBarUI* m_pHScrollBar;
private:
};


/////////////////////////////////////////////////////////////////////////////////////
//
class  CCanvasUI : public CContainerUI
{
public:
	CCanvasUI();
	virtual ~CCanvasUI();

	LPCTSTR GetClass() const;
	void DoPaint(HDC hDC, const RECT& rcPaint);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

private:
	SIZE m_szClientCorner;
};

/////////////////////////////////////////////////////////////////////////////////////
//
class CImageCanvasUI : public CCanvasUI
{
public:
	CImageCanvasUI();
	~CImageCanvasUI();

	LPCTSTR GetClass() const;

	void DoPaint(HDC hDC, const RECT& rcPaint);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

	void SetImage( UINT );
	void SetStretch( const StretchFixed& );
private:
	int m_iImageID;
	int m_iShadeImgId;
	IImageInterface *m_pGraphic;
	//窗口拉伸时，保持不变的元素
	UINT m_nStretchMode;
	StretchFixed m_stretchFixed;
};
/////////////////////////////////////////////////////////////////////////////////////
//

class CLuxCanvasUI :public CCanvasUI
{
public:
	CLuxCanvasUI();
	~CLuxCanvasUI();
	LPCTSTR GetClass() const;
public:
	void DoPaint(HDC hDC, const RECT& rcPaint);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
private:
	int m_iBkgImageId;
	int m_iBotLeftImageId;
	int m_iBotRightImageId;
	int m_nBotLeftX;  //距离左边距X
	int m_nBotLeftY;  //距离左下边距Y
	int m_nBotRightX; //..
	int m_nBotRightY; //..
};

class  CWindowCanvasUI : public CCanvasUI
{
public:
   CWindowCanvasUI();
   LPCTSTR GetClass() const;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class  CControlCanvasUI : public CCanvasUI
{
public:
   CControlCanvasUI();
   LPCTSTR GetClass() const;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class  CWhiteCanvasUI : public CCanvasUI
{
public:
   CWhiteCanvasUI();
   LPCTSTR GetClass() const; 
   virtual void SetBkgndColor(COLORREF clrBkgnd);
   void DoPaint(HDC hDC, const RECT& rcPaint);
};


/////////////////////////////////////////////////////////////////////////////////////
//

class  CDialogCanvasUI : public CCanvasUI
{
public:
   CDialogCanvasUI();
   LPCTSTR GetClass() const;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class  CTabFolderCanvasUI : public CCanvasUI
{
public:
   CTabFolderCanvasUI();
   LPCTSTR GetClass() const;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class  CVerticalLayoutUI : public CContainerUI
{
public:
   CVerticalLayoutUI();

   void Notify(TNotifyUI& msg);
   LPCTSTR GetClass() const;
   void SetPos(RECT rc);

protected:
   int m_cyNeeded;
   int m_nPrevItems;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class  CHorizontalLayoutUI : public CContainerUI
{
public:
   CHorizontalLayoutUI();

   

   LPCTSTR GetClass() const;
   void SetPos(RECT rc);
};

class CDivideLayoutUI :public CControlUI
{
public:
	CDivideLayoutUI();
	~CDivideLayoutUI();
public:
	LPCTSTR GetClass() const;
	void SetPos(RECT rc);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	SIZE EstimateSize(SIZE szAvailable);
	UINT GetControlFlags();
    void DoPaint(HDC hDC, const RECT& rcPaint);
	void Event(TEventUI& event);
private:
	void PaintMoveDivide(const POINT &pt);
private:
	virtual void OnEventButtonDown(TEventUI &e);
	virtual void OnEventButtonUp(TEventUI &e);
	virtual void OnEventMouseMove(TEventUI &e); 
	virtual void OnEventMouseEnter(TEventUI &e);
	virtual void OnEventMouseLeave(TEventUI &e);
	virtual void OnEventDragEnd(TEventUI &e);
	virtual void OnSetCursor(TEventUI &e);

	//
	void SetDivideCursor();
private:
	UINT m_uImageId;
	POINT m_ptDown;
	BOOL m_bVert; //是否为垂直线
};

/////////////////////////////////////////////////////////////////////////////////////
//

class  CTileLayoutUI : public CContainerUI
{
public:
   CTileLayoutUI();

   LPCTSTR GetClass() const;

   void SetPos(RECT rc);
   void SetColumns(int nCols);

protected:
   int m_nColumns;
   int m_cyNeeded;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class  CDialogLayoutUI : public CContainerUI
{
public:
   CDialogLayoutUI();

   LPCTSTR GetClass() const;
   LPVOID GetInterface(LPCTSTR pstrName);

   void SetStretchMode(CControlUI* pControl, UINT uMode);

   void SetPos(RECT rc);
   SIZE EstimateSize(SIZE szAvailable);

protected:
   void RecalcArea();

protected:  
   typedef struct 
   {
      CControlUI* pControl;
      UINT uMode;
      RECT rcItem;
   } STRETCHMODE;

   RECT m_rcDialog;
   RECT m_rcOriginal;
   bool m_bFirstResize;
   CStdValArray m_aModes;
};

class  CShortCutItem: public CVerticalLayoutUI
{
public:
	explicit CShortCutItem(const TCHAR *szLabelCaption, const TCHAR *szFilePath,  int nBkgImageId, int nFloatImageId,
		const TCHAR *szTip);
public:
	void SetShowCaption(BOOL bShow);
	void Notify(TNotifyUI& msg);
	void GetShortCutFlag(TCHAR *szShortCut);
	void SetPos(RECT rc);
private:
	CControlUI *m_pLbl;
	CControlUI *m_pButton;
	CStdString m_strShortCutName;
};

class  CAutoShortCutVertList : public CVerticalLayoutUI
{
public:
	CAutoShortCutVertList();
	~CAutoShortCutVertList();
public:
	void SetPos(RECT rc);
	//
	BOOL AddShortCut(const TCHAR *szLabelCaption, const TCHAR *szFilePath, int nImageId, const TCHAR *szTip,
		            TCHAR *szFlag);
	//
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	LPCTSTR GetClass() const;
private:
	int m_nButtonImageId;
	int m_nColWidth;
	int m_nRowHeight;
	SIZE m_szItem;
	BOOL m_bShowLabel;
};

#endif // !defined(AFX_UICONTAINER_H__20060218_C077_501B_DC6B_0080AD509054__INCLUDED_)

