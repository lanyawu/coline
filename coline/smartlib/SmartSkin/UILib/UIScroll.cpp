#include "common.h"

#include <UILib/UIScroll.h>
#include <UILib/UIContainer.h>
#include <UILib/UIButton.h>

//class CScrollBarUI
CScrollBarUI::CScrollBarUI(CContainerUI *pOwner, unsigned int type):
              m_pOwner(pOwner),
			  m_iMin(0),
			  m_iMax(0),
			  m_iPage(0),
			  m_iScrollPos(0),
			  m_dragSensitive(0),
			  m_bDragged(FALSE),
			  m_imagePrior(new CImageButtonUI()),
			  m_imageMid(new CImageButtonUI()),
			  m_imageNext(new CImageButtonUI())

{
	memset(&m_rcPrior, 0, sizeof(RECT));
	memset(&m_rcPriorBlank, 0, sizeof(RECT));
	memset(&m_rcMid, 0, sizeof(RECT));
	memset(&m_rcNextBlank, 0, sizeof(RECT));
	memset(&m_rcNext, 0, sizeof(RECT));

	if (type == UISB_HORZ)
	{
		m_SBSStyle = UISB_HORZ;
	} else
	{
		m_SBSStyle = UISB_VERT;
	}
	m_bVisible = false;
}

CScrollBarUI::~CScrollBarUI()
{
	if (m_imagePrior)
	{
		delete m_imagePrior;
	}
	if (m_imageMid)
	{
		delete m_imageMid;
	}
	if (m_imageNext)
	{
		delete m_imageNext;
	}
}

void CScrollBarUI::Init()
{
	CControlUI::Init();
	if (m_pManager)
	{
		m_imagePrior->SetManager(m_pManager, this);
		m_imageMid->SetManager(m_pManager, this);
		m_imageNext->SetManager(m_pManager, this);	

		UINT nPrior, nMid, nNext;
		m_pManager->GetScrollBarImage(nPrior, nMid, nNext, (m_SBSStyle != UISB_HORZ));

		m_imagePrior->SetImage(nPrior);


		m_imageMid->SetImage(nMid);
		m_imageMid->SetStretchMode(m_SBSStyle == UISB_HORZ ? SM_HORIZONTAL_CENTER : SM_VERTICAL_CENTER);
		m_imageMid->SetFixedCorner(1);
		
		m_imageNext->SetImage(nNext);
	} //end if (m_pManager)
}


void CScrollBarUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent)
{
	m_imagePrior->SetManager(pManager, this);
	m_imageMid->SetManager(pManager, this);
	m_imageNext->SetManager(pManager, this);
	CControlUI::SetManager(pManager, pParent);
}

CControlUI* CScrollBarUI::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags)
{
	if (m_imagePrior->FindControl(Proc, pData, uFlags) != NULL)
		return m_imagePrior;
	if (m_imageMid->FindControl(Proc, pData, uFlags) != NULL)
		return m_imageMid;
	if (m_imageNext->FindControl(Proc, pData, uFlags) != NULL)
		return m_imageNext;
	return CControlUI::FindControl(Proc, pData, uFlags);
}

void CScrollBarUI::SetVisible(bool bVisible)
{
	if (bVisible && (m_iPage >= m_iMax - m_iMin + 1))
	{
		bVisible = false;
	}
	bool bTmp = IsVisible();
	CControlUI::SetVisible(bVisible);
	//whennever the visibility changed, notify its parent
	if (bTmp != IsVisible())
	{
		TNotifyUI msg;
		msg.sType = _T("visibilitychanged");
		msg.wParam = IsVisible();//new
		msg.pSender = this;
		m_pOwner->Notify(msg);
	}
}

LPCTSTR CScrollBarUI::GetClass() const
{
	return _T("ScrollBarUI");
}

SIZE CScrollBarUI::EstimateSize(SIZE szAvailable)
{
	return CSize(SB_WIDTH, 0);
}

void CScrollBarUI::SetPos(RECT rc)
{
	CControlUI::SetPos(rc);
	//position prior and next button
	m_rcPrior = m_rcItem;
	m_rcNext = m_rcItem;
	if (GetScrollStyle() == UISB_VERT)
	{
		m_rcPrior.bottom = m_rcPrior.top + SB_WIDTH;//prior
		m_rcNext.top = m_rcNext.bottom - SB_WIDTH;//next
	} else
	{
		m_rcPrior.right = m_rcPrior.left + SB_WIDTH;//prior
		m_rcNext.left = m_rcNext.right - SB_WIDTH;//next
	}
	m_imagePrior->SetPos(m_rcPrior);
	m_imageNext->SetPos(m_rcNext);

	//position center( drag ) button 
	SetScrollBoxPos();//mid button
}

int CScrollBarUI::GetFixedHeight()
{
	return SB_HEIGHT;
}

int CScrollBarUI::GetFixedWidth()
{
	return SB_WIDTH;
}

void CScrollBarUI::Event(TEventUI &event)
{
	int iDelta = 0;
	switch(event.Type)
	{
		case UIEVENT_BUTTONUP:
			//dragging
			if (m_bDragged)
			{
				m_bDragged = FALSE;
			}
			break;
		case UIEVENT_MOUSEMOVE:
			if (m_bDragged)
			{
				TNotifyUI msg;
				double deltaPos = 0;//
				if (GetScrollStyle() == UISB_VERT) 
				{
					msg.sType = _T("thumbtrack");
					deltaPos = (double)(event.ptMouse.y - m_ptDraggedPoint.y) / m_dragSensitive;
				} else if (GetScrollStyle() == UISB_HORZ)
				{
					msg.sType = _T("hthumbtrack");
					deltaPos = (double) (event.ptMouse.x - m_ptDraggedPoint.x) / m_dragSensitive;
				}
				
				int iNewPos = m_iDragPos + (int)deltaPos;
				msg.wParam = (WPARAM)iNewPos; 
				m_pOwner->Notify(msg);
			}
			break; 
		case UIEVENT_DRAG:
			{
				TNotifyUI msg;
				double deltaPos = 0;//
				if (GetScrollStyle() == UISB_VERT) 
				{
					msg.sType = _T("thumbtrack");
					deltaPos = (double)(event.ptMouse.y - m_ptDraggedPoint.y) / m_dragSensitive;
				} else if (GetScrollStyle() == UISB_HORZ)
				{
					msg.sType = _T("hthumbtrack");
					deltaPos = (double) (event.ptMouse.x - m_ptDraggedPoint.x) / m_dragSensitive;
				}
				
				int iNewPos = m_iDragPos + (int)deltaPos;
				msg.wParam = (WPARAM)iNewPos; 
				m_pOwner->Notify(msg);
				break;
			}
		case UIEVENT_BUTTONDOWN:
			if (::PtInRect(&m_rcMid, event.ptMouse))
			{
				//hit in drag box
				m_bDragged = TRUE;
				m_ptDraggedPoint = event.ptMouse;
				m_iDragPos = m_iScrollPos;
			} else if (::PtInRect(&m_rcPrior, event.ptMouse))
			{ 
				//左上 
				TNotifyUI msg;
				if (GetScrollStyle() == UISB_VERT) 
					msg.sType = _T("lineup");
				else if (GetScrollStyle() == UISB_HORZ) 
					msg.sType = _T("lineleft");
				m_pOwner->Notify(msg);
			} else if (::PtInRect(&m_rcNext, event.ptMouse))
			{ 
				//右下 
				TNotifyUI msg;
				if (GetScrollStyle() == UISB_VERT) 
					msg.sType = _T("linedown");
				else if (GetScrollStyle() == UISB_HORZ) 
					msg.sType = _T("lineright");
				m_pOwner->Notify(msg);
			} else if (::PtInRect(&m_rcPriorBlank, event.ptMouse))
			{
				TNotifyUI msg;
				if (GetScrollStyle() == UISB_VERT) 
					msg.sType = _T("pageup");
				else if (GetScrollStyle() == UISB_HORZ) 
					msg.sType = _T("pageleft");
				m_pOwner->Notify(msg);
			} else if (::PtInRect(&m_rcNextBlank, event.ptMouse))
			{
				TNotifyUI msg;
				if (GetScrollStyle() == UISB_VERT) 
					msg.sType = _T("pagedown");
				else if (GetScrollStyle() == UISB_HORZ) 
					msg.sType = _T("pageright");
				m_pOwner->Notify(msg);
			}
			break;
		default:
			break;
	}
	CControlUI::Event(event);
}

void CScrollBarUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	if (!IsVisible())
		return;
	RECT rcTemp = {0};
	if (!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem)) 
		return;
	//background
	CBlueRenderEngineUI::DoFillRect(hDC, m_pManager, m_rcItem, UICOLOR_SCROLLBAR_BACK, m_bTransparent);
	//up arrow button
	//CBlueRenderEngineUI::DoPaintRectangle(hDC, m_rcPrior, 0, RGB(255,0,0));			
	m_imagePrior->DoPaint(hDC, m_rcPrior);
	//down arrow button
	
	//CBlueRenderEngineUI::DoPaintRectangle(hDC, m_rcNext, 0, RGB(0,255,255));
	m_imageNext->DoPaint(hDC, m_rcNext);
	//drag button
	//CBlueRenderEngineUI::DoPaintRectangle(hDC, m_rcMid, 0, RGB(0,0,255));
	m_imageMid->DoPaint(hDC, m_rcMid);
}

//滚动条相关
void CScrollBarUI::SetScrollPos(int iPos)
{
	//validate parameter
	if (iPos > m_iMax - m_iMin + 1 - m_iPage)
	{
		iPos = m_iMax - m_iMin + 1 - m_iPage;
	}
	if (iPos < m_iMin)
	{
		iPos = m_iMin;
	}
	//apply changes
	if (m_iScrollPos != iPos)
	{
		m_iScrollPos = iPos;
		SetScrollBoxPos();
		Invalidate();
	}
}

void CScrollBarUI::SetScrollRange(int iMin, int iMax)
{
	BOOL bChanged = FALSE;
	//validate params
	if (iMin < 0)
		iMin = 0;
	if (iMax < 0)
		iMax = 0;
	if (iMax < iMin)
		iMax = iMin;

	//apply changes
	if (iMin != m_iMin)
	{
		m_iMin = iMin;
		bChanged = TRUE;
	}
	if (iMax != m_iMax)
	{
		m_iMax = iMax;
		bChanged = TRUE;
	}
	if (m_iMax >= m_iMin)
	{
		if (bChanged)
		{
			//after scroll range changed, we should adjust others attributes
			ValidateScrollPos();
			SetVisible(m_iPage < m_iMax - m_iMin + 1);
			SetScrollBoxPos();
			Invalidate();
		}
	}
}

void CScrollBarUI::SetScrollPage(int iPage)
{
	if (iPage < 0)
		iPage = 0;

	if (m_iPage != iPage)
	{
		m_iPage = iPage;
		ValidateScrollPos();
		SetVisible(m_iPage < m_iMax - m_iMin + 1);
		SetScrollBoxPos();
		Invalidate();
	}
}

BOOL CScrollBarUI::GetScrollRange(int& iMin, int& iMax) const
{
	iMin = m_iMin;
	iMax = m_iMax;
	return TRUE;
}

void CScrollBarUI::ValidateScrollPos()
{
	if (m_iScrollPos > m_iMax - m_iMin + 1 - m_iPage)
	{
		m_iScrollPos = m_iMax - m_iMin + 1 - m_iPage;
	}
	if (m_iScrollPos < m_iMin)
	{
		m_iScrollPos = m_iMin;
	}
}

void CScrollBarUI::SetScrollBoxPos()
{
	//recalc the scroll box size and position
	if (m_iMax > m_iMin)
	{
		//滚动条滑动区域长度
		int scrollChannelLen = 0;
		if (GetScrollStyle() == UISB_VERT)
		{
			scrollChannelLen = m_rcItem.bottom - m_rcItem.top - SB_WIDTH * 2;
		} else
		{
			scrollChannelLen = m_rcItem.right - m_rcItem.left - SB_WIDTH * 2;
		}
		int iScrollSteps = m_iMax - m_iMin + 1;
		int cyScrollBox = scrollChannelLen * m_iPage / iScrollSteps;//滚动按钮的高度或宽度
		cyScrollBox = MAX(cyScrollBox, SB_SCROLLBOX_MINIMUM_HEIGHT);//滚动按钮不能小于某个高度

		//drag button TBD 这一部分的计算有问题，还没有滚动到页面的底部，中间按钮已经
		//和next 按钮接触了。
		::CopyRect(&m_rcMid, &m_rcItem);
		if (GetScrollStyle() == UISB_VERT)
		{
			m_rcMid.top = m_rcPrior.bottom + scrollChannelLen * m_iScrollPos / iScrollSteps;
			m_rcMid.bottom = m_rcMid.top + cyScrollBox;
			if ((m_rcMid.bottom > m_rcNext.top) || (m_iScrollPos + m_iPage >= m_iMax - m_iMin + 1))
			{
				m_rcMid.bottom = m_rcNext.top;
				m_rcMid.top = m_rcMid.bottom - cyScrollBox;
			}
			::SetRect(&m_rcPriorBlank, m_rcMid.left, m_rcPrior.bottom, m_rcMid.right, m_rcMid.top);
			::SetRect(&m_rcNextBlank, m_rcMid.left, m_rcMid.bottom, m_rcMid.right, m_rcNext.top);
		} else
		{
			m_rcMid.left = m_rcPrior.right + scrollChannelLen * m_iScrollPos / iScrollSteps;
			m_rcMid.right = m_rcMid.left + cyScrollBox;
			if ((m_rcMid.right > m_rcNext.left) || (m_iScrollPos + m_iPage >= m_iMax - m_iMin + 1))
			{
				m_rcMid.right = m_rcNext.left;
				m_rcMid.left = m_rcMid.right - cyScrollBox;
			}
			::SetRect(&m_rcPriorBlank, m_rcPrior.right, m_rcMid.top, m_rcMid.left, m_rcMid.bottom);
			::SetRect(&m_rcNextBlank, m_rcMid.right, m_rcMid.top, m_rcNext.left, m_rcMid.bottom);
		}
		m_imageMid->SetPos(m_rcMid);
		//拖动时，超过该距离滚动按钮才会产生消息
		m_dragSensitive = (double)scrollChannelLen / (double)iScrollSteps;
	}
}
