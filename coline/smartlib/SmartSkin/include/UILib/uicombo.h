#if !defined(AFX_UICOMBO_H__20060218_C01D_1618_FBA5_0080AD509054__INCLUDED_)
#define AFX_UICOMBO_H__20060218_C01D_1618_FBA5_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <UILib/UIList.h>
#include <CommonLib/GraphicPlus.h>
/////////////////////////////////////////////////////////////////////////////////////
//

class  CSingleLinePickUI : public CControlUI
{
public:
   CSingleLinePickUI();
    

   LPCTSTR GetClass() const;
   UINT GetControlFlags() const;
   void Event(TEventUI& event);

   SIZE EstimateSize(SIZE szAvailable);
   void DoPaint(HDC hDC, const RECT& rcPaint);

protected:
   RECT m_rcButton;
   RECT m_rcLinks[8];
   int m_nLinks; 
   UINT m_uButtonState;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class CDropDownEditWnd;
class CDropDownWnd;
class  CDropDownUI : public CContainerUI
{
public:
	CDropDownUI();
	~CDropDownUI();

	LPCTSTR GetClass() const;
	void Init();
	UINT GetControlFlags() const;
	bool Activate();
	void SetPos(RECT rc);
	void Event(TEventUI& event);
	SIZE EstimateSize(SIZE szAvailable);
	void DoPaint(HDC hDC, const RECT& rcPaint);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
    CStdString GetText() const;
	void SetText(LPCTSTR pstrText);

	bool Add(CControlUI* pControl, const int nIdx);
	bool Remove(CControlUI* pControl);
	void RemoveAll();
	int GetCount() const;
	int FindSelectable(int iIndex, bool bForward = true) const;
	int GetLeftPadding();
	int GetTopPadding();
	int GetRightPadding();
	int GetBottomPadding();
	//operations
	int GetCurSel() const;  
	bool SelectItem(const int iIndex);
	int GetItemCount() const;
	SIZE GetDropDownSize() const;
	int  InsertString(int idx, const TCHAR *szText);
	//
	int  AddString(const CStdString& sText);
	BOOL RemoveString( const CStdString& sText );
	BOOL RemoveAllString();
	BOOL SetString(const int iIndex, const CStdString& sText );
	BOOL GetString( int iIndex, CStdString& sText );
	BOOL SetItemData(const int iIndex, void *pData );
	int FindString(const CStdString& sText );
	BOOL DeleteItem(const int iIndex);
	void *GetItemData(const int iIndex) const;

	void SetMaxDropDownItems( int iItem );
protected:
	virtual void OnEventSetCursor( TEventUI& );
	virtual void OnEventButtonDown( TEventUI& );
	virtual void OnEventMouseMove( TEventUI& );
	virtual void OnEventButtonUp( TEventUI& );
	virtual void OnEventKeydown( TEventUI& );
	virtual void OnEventScrollWheel( TEventUI& );
	virtual void OnEventMouseEnter( TEventUI& );
	virtual void OnEventMouseLeave( TEventUI& );
	virtual void OnEventWindowSize( TEventUI& );
	virtual void OnEventDblClick( TEventUI& );
	virtual void OnEventSetFocus( TEventUI& e );
	virtual void OnEventKillFocus( TEventUI& e );
	virtual void OnEventWindowPosChanging( TEventUI& e );

	BOOL OnEditKeyDown(WPARAM wParam, LPARAM lParam);

private:
	BOOL WantReturn() const;
	void ActivateEditWnd();
	void ShowDropDownWnd( BOOL bShow );
private:
	int m_cxWidth;
	UINT m_uButtonState;
	int m_iMaxDropDownItems;
	int m_nBorderImageId;
	int m_iLeftPadding;
	int m_iTopPadding;
	int m_iRightPadding;
	int m_iBottomPadding;
	//下拉按钮的图片id及图片类
	UINT m_iImgId;
	RECT m_rcButton;
	//指定下拉框的输入区是否可编辑
	bool m_bEditable;
	CDropDownEditWnd* m_pEditWindow;
	CDropDownWnd* m_pDropDownWnd;
	CListUI* m_pList;

	friend class CDropDownEditWnd;
	friend class CDropDownWnd;
};


#endif // !defined(AFX_UICOMBO_H__20060218_C01D_1618_FBA5_0080AD509054__INCLUDED_)

