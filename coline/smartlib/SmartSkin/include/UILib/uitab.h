#if !defined(AFX_UITAB_H__20060218_95D6_2F8B_4F7A_0080AD509054__INCLUDED_)
#define AFX_UITAB_H__20060218_95D6_2F8B_4F7A_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <UILib/UIList.h>
#include <UILib/UIButton.h>

/////////////////////////////////////////////////////////////////////////////////////
//
class  CTabFolderUI : public CContainerUI
{
public:
   CTabFolderUI();

   LPCTSTR GetClass() const;

   void Init();

   bool Add(CControlUI* pControl, const int nIdx);

   //IListOwnerUI overridables
   int GetCurSel() const;
   bool SelectItem(const int iIndex);
   bool SelectItem(LPCTSTR pstrPageName);
   BOOL TabGetSelItemName(TCHAR *szSelItemName, int *nSize);
   BOOL GetSelItemChildControl(const TCHAR *szClassName, TCHAR *szSelItemName, int *nSize);
   //CControlUI overridables
   void Event(TEventUI& Event);
   void SetPos(RECT rc);
   void DoPaint(HDC hDC, const RECT& rcPaint);
   void SetVisible(bool bVisible);
   void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

   //operations
   bool SetTabAlign(UINT);
   void SetTabSize( const SIZE& sz );
   SIZE GetTabSize() const;

private:
   void SetTabTopPos();

protected:
   int m_iCurSel;
   RECT m_rcTabs;//tabfoler的tab页标区域(除去client区域剩下的区域)
   RECT m_rcClient;//客户区域
   SIZE m_szClientCorner;//指定客户区域边框的圆角尺寸
   UINT m_nTabAlign;//tab的排列对齐方式
   SIZE m_szTab;//tab页标签按钮的宽度或高度
   CControlUI* m_pCurPage;
   CStdValArray m_aTabAreas;
};

/////////////////////////////////////////////////////////////////////////////////////
//
class CTabButtonUI : public CButtonUI
{
public:
	void DoPaint(HDC hDC, const RECT& rcPaint);

	virtual void Select( BOOL ); 
};

/////////////////////////////////////////////////////////////////////////////////////
//
class  CTabPageUI : public CContainerUI
{
public:
	CTabPageUI();
	LPCTSTR GetClass() const;

	CControlUI* FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

	//overridable operation
	virtual void Select( bool );
	virtual void SetEnabled(bool bEnable);
	virtual void Active( bool bActive );
	virtual bool IsActive() const;
private:
	bool m_bActive;
	CTabButtonUI m_tabButton;

	friend class CTabFolderUI;
};

/////////////////////////////////////////////////////////////////////////////////////
//
class CImageTabFolderUI : public CTabFolderUI
{
public:
	CImageTabFolderUI();
	~CImageTabFolderUI();

	LPCTSTR GetClass() const;

	void Init();
	void SetPos(RECT rc);
	void DoPaint(HDC hDC, const RECT& rcPaint);
	void SetAttribute( LPCTSTR pstrName, LPCTSTR pstrValue );

	//更换和查询客户区背景图片
	UINT GetImage() const;
	void SetImage( UINT );
private:
	void SetPosBottomTabs();
	void SetPosTopTabs();
	void SetPosLeftTabs();
	void SetPosRightTabs();
	void SetPosHiddenTabs();

	int GetTabButtonWidth( const CStdString& sText );

	void PaintTabButtons(HDC hDC, const RECT& rcPaint);
private:
	UINT m_nImageID;//client区域的背景图片
	UINT m_nTabImageId; //
	UINT m_nTabOffset;//tab区域向client区域偏移的距离，目的是用选中的tab按钮
						//覆盖client区域的相关边框部分
	BOOL m_bTabBtnWidthByText;//tab页按钮宽度由tab页文字决定，只适用于tab区
							//在上方的情形
};

/////////////////////////////////////////////////////////////////////////////////////
//
class CImageTabButtonUI : public CImageButtonUI
{
public:
	CImageTabButtonUI();
	~CImageTabButtonUI();

	//CControlUI overridable
	LPCTSTR GetClass() const;
	void DoPaint(HDC hDC, const RECT& rcPaint);

	virtual void Select( BOOL );
};

/////////////////////////////////////////////////////////////////////////////////////
//
class CImageTabPageUI : public CTabPageUI
{
public:
	CImageTabPageUI();
	~CImageTabPageUI();

	LPCTSTR GetClass() const;
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	CControlUI* FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags);

	void SetTabButtonTooltip( const CStdString& sTooltip );
	virtual void SetEnabled(bool bEnabled);
private:
	CImageTabButtonUI m_imageButton;

	friend class CImageTabFolderUI;
};
#endif // !defined(AFX_UITAB_H__20060218_95D6_2F8B_4F7A_0080AD509054__INCLUDED_)

