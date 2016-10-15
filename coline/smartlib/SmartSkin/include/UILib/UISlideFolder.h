#pragma once

#include <UILib/UIContainer.h>
#include <UILib/UIButton.h>

class CSlidePageUI : public CContainerUI
{
public:
	CSlidePageUI();
	~CSlidePageUI();

	//CControlUI overridable
	LPCTSTR GetClass() const;
	void Init();
	bool Activate();//notify parent that page selection happened
	void SetPos(RECT rc);
	void DoPaint(HDC hDC, const RECT& rcPaint);
	void Event(TEventUI& event);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	CControlUI* FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags);

	//operation
	void SetIndex(UINT index){ m_uIndex = index; }
	UINT GetIndex() const { return m_uIndex; }
	void SetButtonHeight(UINT nHeight) { m_cyFixedButton = nHeight; }
private:
	UINT m_uIndex;//page index
	UINT m_cyFixedButton;//按钮的固定高度
	CImageButtonUI m_imageButton;//浏览按钮
};

//////////////////////////////////////////////////////////////////////////////////
//
//目前CSlideFolderUI只能包含CSlidePageUI子控件
//添加其它类型控件会失败
class CSlideFolderUI : public CContainerUI
{
public:
	CSlideFolderUI();
	~CSlideFolderUI();

	//CControlUI overridable
	LPCTSTR GetClass() const;
	void Init();
	void DoPaint(HDC hDC, const RECT& rcPaint);
	void SetAttribute( LPCTSTR pstrName, LPCTSTR pstrValue );
	void SetPos(RECT rc);

	//CContainerUI overridable
	bool Add( CControlUI* pControl, const int nIdx);
	bool Remove( CControlUI* pControl );

	//operation
	bool SelectPage(int index );
	UINT GetCurPage() const;

private:
	void SetChildrenUIPos( const RECT& rc );

	bool IsSliding();
private:
	UINT m_uCurPage;
	UINT m_uPrePage;
	BOOL m_bSliding;
	UINT m_uMaxSlideStep;
	UINT m_uCurSldingStage;

	UINT m_cyFixedSlidingBtn;
};
