#pragma once

static const int SB_WIDTH = 16;//垂直滚动条宽度
static const int SB_HEIGHT = SB_WIDTH;//水平滚动条高度

static const int SB_SCROLLBOX_MINIMUM_HEIGHT = 6;//

const int UISB_VERT = 0;
const int UISB_HORZ = 1;

//滚动条UI 
class CContainerUI;
class CImageButtonUI;
class CScrollBarUI: public CControlUI
{
public:
	CScrollBarUI(CContainerUI *pOwner, unsigned int type );
	~CScrollBarUI();
public:
	//CControlUI 虚函数
	LPCTSTR GetClass() const;
	SIZE EstimateSize(SIZE szAvailable);
	void DoPaint(HDC hDC, const RECT& rcPaint);
	void Event(TEventUI& event);
	void Init();
	void SetManager(CPaintManagerUI* pManager, CControlUI* pParent);
	CControlUI* FindControl( FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags );
	void SetVisible( bool );
	void SetPos(RECT rc);
	//Set
	void SetScrollPos( int iPos );
	void SetScrollRange( int dwMin, int dwMax);
	void SetScrollPage( int iPage );
	//Get
	int GetScrollPos() const { 
		return m_iScrollPos; 
	}
	UINT GetScrollStyle() const { 
		return m_SBSStyle; 
	}
	BOOL GetScrollRange( int& iMin, int& iMax) const;
	int GetScrollPage() const { 
		return m_iPage; 
	}
	int GetFixedHeight();
	int GetFixedWidth();
private:
	void SetScrollBoxPos();
	void ValidateScrollPos();
private:
	CContainerUI *m_pOwner;
	UINT m_SBSStyle;
	int m_iMin;//srcoll range minimum
	int m_iMax;//scroll range maximum
	int m_iPage;//
	int m_iScrollPos;//
	
	double m_dragSensitive;//拖拽一个单位，鼠标滑过的距离。
	BOOL   m_bDragged; //是否正在拖拽
	int m_iDragPos;//拖拽开始时的scrollpos
	POINT  m_ptDraggedPoint; //拖拽开始时鼠标位置

	RECT   m_rcPrior; //向前翻滚按钮位置
	RECT	m_rcPriorBlank;//
	RECT   m_rcMid;//中间拖拽按钮位置
	RECT	m_rcNextBlank;//
	RECT   m_rcNext;//向后按钮位置
	CImageButtonUI* m_imagePrior;//向上滚动箭头按钮
	CImageButtonUI* m_imageMid;//中间拖拽按钮
	CImageButtonUI* m_imageNext;//向下滚动箭头按钮
};