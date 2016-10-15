#pragma once

static const int SB_WIDTH = 16;//��ֱ���������
static const int SB_HEIGHT = SB_WIDTH;//ˮƽ�������߶�

static const int SB_SCROLLBOX_MINIMUM_HEIGHT = 6;//

const int UISB_VERT = 0;
const int UISB_HORZ = 1;

//������UI 
class CContainerUI;
class CImageButtonUI;
class CScrollBarUI: public CControlUI
{
public:
	CScrollBarUI(CContainerUI *pOwner, unsigned int type );
	~CScrollBarUI();
public:
	//CControlUI �麯��
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
	
	double m_dragSensitive;//��קһ����λ����껬���ľ��롣
	BOOL   m_bDragged; //�Ƿ�������ק
	int m_iDragPos;//��ק��ʼʱ��scrollpos
	POINT  m_ptDraggedPoint; //��ק��ʼʱ���λ��

	RECT   m_rcPrior; //��ǰ������ťλ��
	RECT	m_rcPriorBlank;//
	RECT   m_rcMid;//�м���ק��ťλ��
	RECT	m_rcNextBlank;//
	RECT   m_rcNext;//���ťλ��
	CImageButtonUI* m_imagePrior;//���Ϲ�����ͷ��ť
	CImageButtonUI* m_imageMid;//�м���ק��ť
	CImageButtonUI* m_imageNext;//���¹�����ͷ��ť
};