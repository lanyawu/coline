#include "common.h"

#include <UILib/UISlideFolder.h>
///////////////////////////////////////////////////////////////////////////////
//
//
CSlidePageUI::CSlidePageUI(): 
              m_uIndex(-1),
			  m_cyFixedButton(10)
{
	//
}

CSlidePageUI::~CSlidePageUI()
{
	//
}

//CControlUI overridable
LPCTSTR CSlidePageUI::GetClass() const
{
	return _T("SlidePageUI");
}

void CSlidePageUI::Init()
{
	CContainerUI::Init();
	if (m_pManager != NULL)
	{
		//check the destruction seq to avoid crash
		m_imageButton.SetManager(m_pManager, this);
		m_imageButton.SetFixedCorner(3);
		m_imageButton.SetStretchMode(SM_HORIZONTAL);
	}
}

bool CSlidePageUI::Activate()
{
	CSlideFolderUI* pParent = dynamic_cast<CSlideFolderUI*>(GetParent());
	if (pParent)
	{
		return pParent->SelectPage(GetIndex());
	}
	return false;
}

void CSlidePageUI::SetPos(RECT rc)
{
	if (!::EqualRect(&rc, &m_rcItem))
	{
		//��������ť�⣬ֻ����һ���ӿؼ�
		if (m_ChildList.GetSize() == 1)
		{
			CControlUI* pControl = m_ChildList[0];
			if (pControl != NULL)
			{
				UINT uHeight = rc.bottom - rc.top;
				if (uHeight <= m_cyFixedButton)
				{
					m_imageButton.SetPos(rc);
					pControl->SetVisible(false);
				} else
				{
					CRect rcButton(rc);
					rcButton.bottom = rcButton.top + m_cyFixedButton;
					m_imageButton.SetPos(rcButton);
					CRect rcPage(rc);
					rcPage.top = rcButton.bottom;
					pControl->SetVisible(true);
					pControl->SetPos(rcPage);
				} //end else if (uHeight <= ...
			} //end if (pControl != NULL...
		} //end if (m_items...
		CControlUI::SetPos(rc);
	} //end if (EqualRect...
}

void CSlidePageUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcscmp(pstrName, _T("image")) == 0)
	{
		m_imageButton.SetAttribute(pstrName, pstrValue);
	} else
	{
		CContainerUI::SetAttribute(pstrName, pstrValue);
	}
}

void CSlidePageUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	RECT rcFill = { 0 };
	if (!::IntersectRect(&rcFill, &m_rcItem, &rcPaint))
		return;
	//button
	m_imageButton.DoPaint(hDC, rcPaint);
	//button text
	int iLinks = 0;
	RECT rcText = m_imageButton.GetPos();
	CBlueRenderEngineUI::DoPaintPrettyText(hDC, m_pManager, 
		rcText, GetText(), UICOLOR_TEXTCOLOR_NORMAL, UICOLOR__INVALID,
		NULL, iLinks, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	//container
	CContainerUI::DoPaint(hDC, rcPaint);
}

void CSlidePageUI::Event(TEventUI& event)
{
	CRect rcButton = m_imageButton.GetPos();
	if (::PtInRect(&rcButton, event.ptMouse) 
		&& (event.Type == UIEVENT_BUTTONUP))
	{
		Activate();
	}
	CContainerUI::Event(event);
}

CControlUI *CSlidePageUI::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags)
{
	if (m_imageButton.FindControl(Proc, pData, uFlags))
		return &m_imageButton;
	return CContainerUI::FindControl(Proc, pData, uFlags);
}

///////////////////////////////////////////////////////////////////////////////
//
//
CSlideFolderUI::CSlideFolderUI(): 
                m_uCurPage(-1),
				m_uPrePage(-1),
				m_bSliding(FALSE),
				m_uMaxSlideStep(4),
				m_uCurSldingStage(0),
				m_cyFixedSlidingBtn(10)
{
	//
}

CSlideFolderUI::~CSlideFolderUI()
{
	//
}

//CControlUI overridable
LPCTSTR CSlideFolderUI::GetClass() const
{
	return _T("SlideFolderUI");
}


void CSlideFolderUI::Init()
{
	CContainerUI::Init();
	for (int i = 0; i < m_ChildList.GetSize(); ++i)
	{
		CSlidePageUI* pPage = dynamic_cast<CSlidePageUI*>(m_ChildList[i]);
		if (pPage != NULL)
			pPage->SetButtonHeight(m_cyFixedSlidingBtn);
	}
	m_uCurPage = 0;
}

bool CSlideFolderUI::IsSliding()
{
	if ((!m_bSliding) || (m_uMaxSlideStep == 0) 
		|| (m_uCurSldingStage > m_uMaxSlideStep))
		return false; 
	return true;
}

void CSlideFolderUI::SetChildrenUIPos(const RECT& rc)
{
	//step by step
	if (IsSliding())
	{
		//�������ҳ��Ĵ�С
		//ѡ��ҳ���ҳ�����ݸ߶�
		int cyPageContent = rc.bottom - rc.top - m_cyFixedSlidingBtn * m_ChildList.GetSize();
		if (m_uMaxSlideStep == 0) 
			m_uMaxSlideStep = 4;//У�黬���Ĳ��裬���û���趨����Ĭ�ϻ���4��
		int top = rc.top;
		for (int i = 0; i < m_ChildList.GetSize(); ++i)
		{
			CSlidePageUI* pPage = dynamic_cast<CSlidePageUI*>(m_ChildList[i]);
			RECT rcPage = { 0 };
			UINT iIndex = pPage->GetIndex();
			if (iIndex < MIN(m_uCurPage, m_uPrePage))
			{
				//we shouldn't change the pos of this page, because it reside on top of the
				//previous selected page and the currently selected page.
				top += m_cyFixedSlidingBtn;
			} else if (iIndex > MAX(m_uCurPage, m_uPrePage))
			{
				//do nothing yet
				top += m_cyFixedSlidingBtn;
				break;//leaving the next pages unchanged
			} else if (iIndex == m_uCurPage)
			{
				//inflate this page
				int cy = cyPageContent * m_uCurSldingStage / m_uMaxSlideStep;
				::SetRect(&rcPage, rc.left, top, rc.right, top + m_cyFixedSlidingBtn + cy);
				top += (m_cyFixedSlidingBtn + cy);
				pPage->SetPos(rcPage);
			} else if (iIndex == m_uPrePage)
			{
				//deflate the previous selected page
				int cy = cyPageContent - cyPageContent * m_uCurSldingStage / m_uMaxSlideStep;
				::SetRect(&rcPage, rc.left, top, rc.right, top + m_cyFixedSlidingBtn + cy);
				top += (cy + m_cyFixedSlidingBtn);
				pPage->SetPos(rcPage);
			} else
			{
				//change the pos of this page but keeping its size unchanged.
				//this page reside between the previous page and the current page
				::SetRect(&rcPage, rc.left, top, rc.right, top + m_cyFixedSlidingBtn);
				top += m_cyFixedSlidingBtn;
				pPage->SetPos(rcPage);
			} // end else ..
		} //end for (int i ...
		//determine if we have reached the final situation
		++m_uCurSldingStage;
		if (m_uCurSldingStage <= m_uMaxSlideStep)
		{
			Invalidate();
		}	
	} else
	{
		int top = rc.top;
		//SlidePage��ҳ�����ݸ߶�
		int cyPageContent = rc.bottom - rc.top - m_cyFixedSlidingBtn * m_ChildList.GetSize();
		for (int i = 0; i < m_ChildList.GetSize(); ++i)
		{
			CSlidePageUI* pPage = dynamic_cast<CSlidePageUI*>(m_ChildList[i]);
			RECT rcPage = { 0 };
			if (pPage->GetIndex() == m_uCurPage)
			{
				//��ǰҳ�棬ҳ��߶�Ϊ��ť��ҳ�����ݵĸ߶�
				::SetRect(&rcPage, rc.left, top, rc.right, 
					top + m_cyFixedSlidingBtn + cyPageContent);
				top += (m_cyFixedSlidingBtn + cyPageContent);
			} else
			{
				//�ǵ�ǰҳ�棬ҳ��߶�Ϊ��ť�߶�
				::SetRect(&rcPage, rc.left, top, rc.right, top + m_cyFixedSlidingBtn);
				top += m_cyFixedSlidingBtn;
			}
			pPage->SetPos(rcPage);
		}//end for( int i = 0; i < m_items.GetSize(); ++i )
	}//end if (IsSliding()) else
}

void CSlideFolderUI::SetPos(RECT rc)
{
	CControlUI::SetPos(rc);
}

void CSlideFolderUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	RECT rcFill = { 0 };
	if (!::IntersectRect(&rcFill, &m_rcItem, &rcPaint))
		return;
	//�����Է���SetPos�ӿ��С���ť���������У����û�е�����λ�ã�
	//��Ҫ���ϼ���SlidePage��λ�ã����һ��ơ�Ϊ�˽�ʡcpu��SetChildrenUIPos
	//����Invalidate������UpdateLayout�ӿڣ�����ŵ�SetPos�У����������
	//�������
	SetChildrenUIPos(m_rcItem);
	CContainerUI::DoPaint(hDC, rcPaint);
}

void CSlideFolderUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcscmp(pstrName, _T("maxslidestep")) == 0)
	{
		m_uMaxSlideStep = _ttoi( pstrValue );
	} else if (_tcscmp(pstrName, _T("buttonheight")) == 0)
	{
		m_cyFixedSlidingBtn = _ttoi(pstrValue);
	} else
	{
		CContainerUI::SetAttribute(pstrName, pstrValue);
	}
}

//CContainerUI overridable
bool CSlideFolderUI::Add(CControlUI* pControl, const int nIdx)
{
	CSlidePageUI* pPage = dynamic_cast<CSlidePageUI*>(pControl);
	if (pPage)
	{
		pPage->SetIndex(m_ChildList.GetSize());
		return CContainerUI::Add(pControl);
	}
	return false;
}

bool CSlideFolderUI::Remove(CControlUI* pControl)
{
	//maybe we can try to delete a sub control dynamicly :)
	ASSERT( !_T("Not supported yet!") );
	return false;
}

//operation
bool CSlideFolderUI::SelectPage(int index )
{
	if (index == -1)
		return false;
	if (index == m_uCurPage)
		return true;
	//begin sliding pages
	m_uPrePage = m_uCurPage;
	m_uCurPage = index;
	m_uCurSldingStage = 1;
	m_bSliding = TRUE;
	Invalidate();
	if (m_pManager)
	{
		m_pManager->SendNotify(this, _T("itemselect"), (WPARAM)m_uCurPage);
	}
	return false;
}

UINT CSlideFolderUI::GetCurPage() const
{
	return m_uCurPage;
}