#if !defined(AFX_UIEDIT_H__20060218_14AE_7A41_09A2_0080AD509054__INCLUDED_)
#define AFX_UIEDIT_H__20060218_14AE_7A41_09A2_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <list>
/////////////////////////////////////////////////////////////////////////////////////
//

class CSingleLineEditWnd;
class CSingleLineEditUI : public CControlUI
{
public:
	CSingleLineEditUI();

	LPCTSTR GetClass() const;
	UINT GetControlFlags() const;

	CStdString GetText() const;
	void SetText(LPCTSTR pstrText);
	void Event(TEventUI& event);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	SIZE EstimateSize(SIZE szAvailable);
	void DoPaint(HDC hDC, const RECT& rcPaint);

	void SetEditStyle(UINT uStyle);
	void SetEnabled(bool bEnabled);
	void SetReadOnly(bool bReadOnly);
	bool IsReadOnly() const;
	void EnableLink( BOOL bEnable );
	BOOL IsLinkEnabled() const;
	void SetTextColor( COLORREF clr );
	void SetReadOnlyTextColor( COLORREF clr );
	void SetTextLimit( int iLimit );
	int TextLimit() const;
	void SetBytesLimit( int iLimit );
	int GetBytesLimit() const;
	//
	int GetLeftPadding();
	int GetRightPadding();
	int GetTopPadding();
	int GetBottomPadding();
	//¼üÅÌ°´ÏÂ
	BOOL OnChar( WPARAM wParam, LPARAM lParam );
	BOOL OnKeyDown(WPARAM wParam, LPARAM lParam);
protected:
	void SetPassword( bool );
	void SetNumber( bool bNumberOnly );

	virtual CStdString GetDisplayText();
	virtual COLORREF GetDisplayTextColor();

	virtual void OnEventMouseEnter();
	virtual void OnEventMouseLeave();
protected:
	CSingleLineEditWnd* m_pWindow;
	CStdString m_strTip;
	UINT m_uEditStyle;
	int  m_nBorderImageId;

	BOOL m_bHot;
	BOOL m_bPretty;
	int m_iTextLimit;
	int m_iBytesLimit;
	COLORREF m_clrText;
	COLORREF m_clrTextReadOnly;
	BOOL m_bHandleChar;
	BOOL m_bEnableLink;
	bool m_bReadOnly;
	int  m_iLeftPadding;
	int  m_iTopPadding;
	int  m_iRightPadding;
	int  m_iBottomPadding;
	StretchFixed m_BorderFixed;
	std::list<unsigned char> m_listChatLen;
	typedef std::list<unsigned char>::iterator LenIt;

	friend class CSingleLineEditWnd;
};

/////////////////////////////////////////////////////////////////////////////////////
//
class CTipEditUI : public CSingleLineEditUI
{
public:
	CTipEditUI();

	LPCTSTR GetClass() const;
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	void SetTipText( LPCTSTR );

protected:
	CStdString GetDisplayText();
	COLORREF GetDisplayTextColor();

private:
	CStdString m_sTipText;
};

/////////////////////////////////////////////////////////////////////////////////////
//

class CMultiLineEditWnd;


class CMultiLineEditUI : public CControlUI
{
friend CMultiLineEditWnd;
public:
   CMultiLineEditUI();
   ~CMultiLineEditUI();

   LPCTSTR GetClass() const;
   UINT GetControlFlags() const;

   void Init();
   
   CStdString GetText() const;
   void SetText(LPCTSTR pstrText);

   void SetEnabled(bool bEnabled);
   void SetVisible(bool bVisible);
   void SetReadOnly(bool bReadOnly);
   bool IsReadOnly() const;
   bool SetTextLimit( int iLimit );

   void Event(TEventUI& event);

   SIZE EstimateSize(SIZE szAvailable);
   void SetPos(RECT rc);
   void SetPos(int left, int top, int right, int bottom);
   void DoPaint(HDC hDC, const RECT& rcPaint);
   void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

   void Select( int iStart = 0, int iEnd = -1 );
   void AppendText(const char *szText);
   bool Copy();
private:
   BOOL OnKeyDown(WPARAM wParam, LPARAM lParam);
   BOOL OnChar(WPARAM wParam, LPARAM lParam);
protected:
   CMultiLineEditWnd* m_pWindow;

   DWORD m_dwEditStyle;

   friend class CMultiLineEditWnd;
};



#endif // !defined(AFX_UIEDIT_H__20060218_14AE_7A41_09A2_0080AD509054__INCLUDED_)

