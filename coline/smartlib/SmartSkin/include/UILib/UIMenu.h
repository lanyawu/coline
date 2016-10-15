#pragma once

#include <UILib/UIContainer.h>
#include <UILib/UIButton.h>
#include <string>
//////////////////////////////////////////////////////////////////////////////
//
//
class CMenuUI;
class CMenuButtonUI :public CImageButtonUI
{
public:
	CMenuButtonUI(void);
	~CMenuButtonUI(void);

	LPCTSTR GetClass() const;
	UINT GetControlFlags() const;

	void Notify(TNotifyUI& msg);
	//CControlUI overridable
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	void Init();
	void DoPaint(HDC hDC, const RECT& rcPaint);
	//operation
	//sMenuName: menu name in skin
	BOOL SetMenu( const CStdString& sMenuName );
	//此类负责释放pMenu
	void AttachMenu( CMenuUI* pMenu );
	void SetDisplayMenuText( BOOL );
	virtual CMenuUI *GetPopMenu();
protected:
	void OnEventButtonUp(TEventUI& e);

private:
	void LoadImage();
	BOOL ShowMenu();
	BOOL LoadMenu();
	void DestroyMenu(); 
private:
	//menu
	CStdString m_sMenuName;
	CMenuUI* m_pMenu;
	BOOL m_bDspMenuText;
};

class CPlusMenuButtonUI : public CMenuButtonUI
{
public:
	virtual void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue); 
	virtual void DoPaint(HDC hDC, const RECT& rcPaint);
	virtual SIZE EstimateSize(SIZE szAvailable);
private:
	UINT m_uArrowImage;
	UINT m_uSubImage;
};


//////////////////////////////////////////////////////////////////////////////
//
//
class CMenuItemUI: public CControlUI
{
public:
	CMenuItemUI( BOOL bLine = false );
	~CMenuItemUI();
	friend class CMenuUI;
	//CControlUI overridable
	LPCTSTR GetClass() const;
	void Init();
    SIZE EstimateSize(SIZE szAvailable);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	void DoPaint(HDC hDC, const RECT& rcPaint){}
	void DrawItem(HDC hDC, const RECT& rcItem,  UINT action, UINT state);

	//operation
	void SetID( UINT uID ){ 
		m_uID = uID; 
	}
	UINT GetID() const{ 
		return m_uID; 
	}
	void SetImage( UINT uID );
	BOOL HasPopupMenu() const;
	HMENU GetPopupMenuHandle() const;
	CMenuUI* GetPopupMenu() const;
	void AttachMenu(CMenuUI* pMenu );
	CMenuItemUI* FindItem( UINT uID );
	void SetUnChecked(UINT uGrpId, UINT uExceptId);
	BOOL IsLineOnly() const
	{
		return m_bLine;
	}
private:
	void LoadImage();

	void DrawSelectState( HDC hDC, const RECT& rcItem, UINT state );
	void DrawText( HDC hDC, const RECT& rcItem, UINT state );

private:
	UINT m_uID;//menu item identifier( Each item having a unique id is preferred ).
	UINT m_uImageID;//
	BOOL m_bLine;
	BOOL m_bAutoChecked;
	UINT m_uGroupId;
	BOOL m_bChecked;
	COLORREF m_bkColor;
	COLORREF m_bkImageClr;
	COLORREF m_clrSel;
	COLORREF m_clrText;
	//sub menu name
	CStdString m_sMenuName;
	CMenuUI* m_pSubMenu;
};

//////////////////////////////////////////////////////////////////////////////
//
//现在CMenuUI的功能还不完善，只能通过皮肤文件进行正常初始化。
//其它途径的创建，包括动态的Add或是AppendMenu，都未经测试。
class CMenuWnd;
class CMenuUI: public CContainerUI
{
public:
	CMenuUI();
	~CMenuUI();

	LPCTSTR GetClass() const;

	void Notify(TNotifyUI& msg);
	//CContaierUI
	void Init();
	bool Add(CControlUI* pControl, const int nIdx);
	bool Remove(CControlUI* pControl);
	void RemoveAll();

	//message handler
	BOOL OnMessage( UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT* plret );
	//operation
	//添加Add接口只能添加非line的menu item，此接口只能添加line
	BOOL AppendSeparatorLine();
	BOOL CreatePopupMenu();
	BOOL DestroyMenu();
	BOOL Remove( int i );
	CMenuItemUI* FindItem( UINT nID );
	HMENU GetHMENU() const 
	{ 
		return m_hMenu; 
	}
	BOOL TrackPopupMenu( UINT nFlags, int x, int y, BOOL bIconMenu = FALSE, LPCRECT lpRect = 0 );	
	void SetOwner( CControlUI* pOwner ){ 
		m_pOwner = pOwner; 
	}
	BOOL GrayItem(HMENU h, UINT nID, BOOL bGrayed);
	BOOL CheckItem(HMENU h, UINT nID, BOOL bCheck);
	BOOL CheckGroupExcept(HMENU h, UINT uID, BOOL bChecked);
	BOOL SetMenuItemAttr(UINT nId, LPCTSTR pstrName, LPCTSTR pstrValue);  
	void SetGroupUnCheckedExcept(HMENU h, int nExceptId, int nGroupId); 
protected:
	virtual BOOL DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
	virtual BOOL MeasureItem( LPMEASUREITEMSTRUCT lpMeasureItemStruct );
	BOOL DoCheckItem(HMENU h, UINT uId);
private:
	BOOL AppendMenuItem( const CMenuItemUI& item );
	HWND GetPaintWnd();
	void DrawSeparatorLine( HDC hDC, const RECT& rcItem );

private:
	static CMenuWnd* m_pParentWnd;
	
	HMENU m_hMenu;
	CControlUI* m_pOwner;

	friend class CMenuWnd;
	friend class CMenuItemUI;
};
