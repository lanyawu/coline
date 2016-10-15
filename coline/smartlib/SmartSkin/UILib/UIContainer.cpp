#include "common.h"

#include <UILib/UIContainer.h>
#include <UILib/UIScroll.h>
#include <UILib/UIButton.h>
#include <UILib/UILabel.h>
#include <UILib/UIPanel.h>
#include <Commonlib/stringutils.h>
#include <Commonlib/systemutils.h>
/////////////////////////////////////////////////////////////////////////////////////
//
//
#pragma warning(disable:4996)

CControlList::CControlList(int iPreallocSize):
              m_ppVoid(NULL), 
			  m_nCount(0), 
			  m_nAllocated(iPreallocSize)
{
	if (iPreallocSize > 0) 
		m_ppVoid = static_cast<CControlUI **>(malloc(iPreallocSize * sizeof(CControlUI *)));
}

CControlList::~CControlList()
{
	if (m_ppVoid != NULL)
		free(m_ppVoid);
}

void CControlList::Empty()
{
	if (m_ppVoid != NULL)
		free(m_ppVoid);
	m_ppVoid = NULL;
	m_nCount = m_nAllocated = 0;
}

void CControlList::Resize(int iSize)
{
	Empty();
	m_ppVoid = static_cast<CControlUI **>(malloc(iSize * sizeof(CControlUI *)));
	::ZeroMemory(m_ppVoid, iSize * sizeof(CControlUI *));
	m_nAllocated = iSize;
	m_nCount = iSize;
}

bool CControlList::IsEmpty() const
{
	return m_nCount == 0;
}

bool CControlList::Add(CControlUI * pData)
{
	if (++m_nCount >= m_nAllocated)
	{
		m_nAllocated *= 2;
		if (m_nAllocated == 0)
			m_nAllocated = 11;
		m_ppVoid = static_cast<CControlUI **>(realloc(m_ppVoid, m_nAllocated * sizeof(CControlUI *)));
		if (m_ppVoid == NULL) 
			return false;
	}
	m_ppVoid[m_nCount - 1] = pData;
	return true;
}

bool CControlList::InsertAt(int iIndex, CControlUI * pData)
{
	if (iIndex == m_nCount) 
		return Add(pData);
	if ((iIndex < 0) || (iIndex > m_nCount)) 
		return false;
	if (++m_nCount >= m_nAllocated)
	{
		m_nAllocated *= 2;
		if (m_nAllocated == 0)
			m_nAllocated = 11;
		m_ppVoid = static_cast<CControlUI **>(realloc(m_ppVoid, m_nAllocated * sizeof(CControlUI *)));
		if (m_ppVoid == NULL)
			return false;
	}
	memmove(&m_ppVoid[iIndex + 1], &m_ppVoid[iIndex], (m_nCount - iIndex - 1) * sizeof(CControlUI *));
	m_ppVoid[iIndex] = pData;
	return true;
}

bool CControlList::SetAt(int iIndex, CControlUI * pData)
{
	if ((iIndex < 0) || (iIndex >= m_nCount))
		return false;
	m_ppVoid[iIndex] = pData;
	return true;
}

bool CControlList::Remove(const int iIndex)
{
	if ((iIndex < 0) || (iIndex >= m_nCount)) 
		return false;
	--m_nCount;
	for (int i = iIndex; i < m_nCount; ++i)
	{
		m_ppVoid[i] = m_ppVoid[i+1];
	}
	return true;
}

int CControlList::Find(CControlUI * pData) const
{
	for (int i = 0; i < m_nCount; i++)
		if (m_ppVoid[i] == pData)
			return i;
	return -1;
}

int CControlList::GetSize() const
{
	return m_nCount;
}

CControlUI ** CControlList::GetData()
{
	return m_ppVoid;
}

CControlUI * CControlList::GetAt(int iIndex) const
{
	if ((iIndex < 0) || (iIndex >= m_nCount)) 
		return NULL;
	return m_ppVoid[iIndex];
}

CControlUI * CControlList::operator[] (int iIndex) const
{
   if ((iIndex>=0) && (iIndex<m_nCount))
	   return m_ppVoid[iIndex];
   return NULL;
}

//
CContainerUI::CContainerUI() : 
              m_pVScrollBar(NULL),
              m_pHScrollBar(NULL),
              m_iPadding(0),
			  m_nBkgImageId(0),
			  m_nBkgLeftImageId(0),
			  m_nBkgRightImageId(0),
			  m_nStretchMode(SM_NORMALSTRETCH),
			  m_bHole(FALSE),
              m_bAutoDestroy(true)
{ 
	m_szLeft.cx = m_szLeft.cy = 0;
	m_szRight.cx = m_szRight.cy = 0;
}

CContainerUI::~CContainerUI()
{
	RemoveAll();
	EnableScrollBar(UISB_VERT, false);
	EnableScrollBar(UISB_HORZ, false);
}

LPCTSTR CContainerUI::GetClass() const
{
	return _T("ContainerUI");
}

LPVOID CContainerUI::GetInterface(LPCTSTR pstrName)
{
	if (_tcsicmp(pstrName, _T("Container")) == 0)
	{
		return static_cast<IContainerUI*>(this);
	}
	return CControlUI::GetInterface(pstrName);
}

CControlUI* CContainerUI::GetItem(const int iIndex) const
{
	if ((iIndex >= 0) && (iIndex < m_ChildList.GetSize()))
	{
		return  m_ChildList[iIndex];
	}
	return NULL;
}

int CContainerUI::GetCount() const
{
	return m_ChildList.GetSize();
}

void CContainerUI::Init()
{
	CControlUI::Init();
	 
}

bool CContainerUI::Add(CControlUI* pControl, const int nIdx)
{
	if (m_pManager != NULL)
	{
		m_pManager->InitControls(pControl, this);
	}
	if (m_pManager != NULL)
	{
		m_pManager->UpdateLayout();
	}
	bool bSucc = false;
	int n = m_ChildList.GetSize();
	if (nIdx < 0)
		n += nIdx;
	else
		n = nIdx;
	if (!IsVisible())
	{
		pControl->SetVisible(false);
	}
	if (n >= m_ChildList.GetSize())
		bSucc = m_ChildList.Add(pControl);
	else if (n < 0)
		bSucc = m_ChildList.InsertAt(0, pControl);
	else
		bSucc = m_ChildList.InsertAt(n, pControl);

	return bSucc ;
}

bool CContainerUI::Remove(CControlUI* pControl)
{
	for (int it = 0; m_bAutoDestroy && it < m_ChildList.GetSize(); it++ )
	{
		if (m_ChildList[it] == pControl)
		{
			if (m_pManager != NULL)
			{  
				m_pManager->UpdateLayout();
			}
			
			delete pControl;
			return m_ChildList.Remove(it);
		} //end if (static_cast<...
	} //end for (int it = 0;...
	return false;
}

void CContainerUI::AdjustLayout(int nDis, BOOL bVert, CControlUI *pUI)
{
	CControlUI *pPrev = NULL, *pNext = NULL, *p = NULL;;
	for (int it = 0; m_bAutoDestroy && it < m_ChildList.GetSize(); it++ )
	{
		if (m_ChildList[it] == pUI)
		{
			if (p)
			{
				pPrev = p;
				it ++;
				if (it < m_ChildList.GetSize())
				{
					pNext = m_ChildList[it];
				}
			}
			break;
		} //end if (static_cast<...
		p = m_ChildList[it];
	} //end for (int it = 0;...
	if (pPrev && pNext)
	{
		if (bVert) 
		{
			//水平方向调整
			SIZE sz = {99999, 99999};
			SIZE szClient = pPrev->EstimateSize(sz);
			if (szClient.cx != 0)
			{
				int w = szClient.cx + nDis;
				pPrev->SetWidth(w);
			} else
			{
				sz.cx = 99999;
				sz.cy = 99999;
				szClient = pNext->EstimateSize(sz);
				if (szClient.cx != 0)
				{
					int w = szClient.cx + nDis;
					pNext->SetWidth(w);
				}
			} //end else if (szClient.cx..
		} else
		{
			//垂直方向调整
			SIZE sz = {99999, 99999};
			SIZE szClient = pPrev->EstimateSize(sz);
			if (szClient.cy != 0)
			{
				int h = szClient.cy + nDis;
				pPrev->SetHeight(h);
			} else
			{
				sz.cx = 99999;
				sz.cy = 99999;
				szClient = pNext->EstimateSize(sz);
				if (szClient.cy != 0)
				{
					int h = szClient.cy + nDis;
					pNext->SetHeight(h);
				}
			}
		} //end else if (bVert)
	} //end if (pPrev && pNext
}

void CContainerUI::RemoveAll()
{
	for (int it = 0; m_bAutoDestroy && it < m_ChildList.GetSize(); it++) 
	{ 
		delete m_ChildList[it];
	}
	m_ChildList.Empty();
	UpdateLayout();
}

CControlUI *CContainerUI::GetChildContrlByClass(LPCTSTR pstrClassName)
{
	for (int it = 0; it < m_ChildList.GetSize(); it++) 
	{
		CControlUI *pControl = m_ChildList[it]; 
		if (::lstrcmp(pControl->GetClass(), pstrClassName) == 0)
			return pControl;
		IContainerUI *pContainer = static_cast<IContainerUI *>(pControl->GetInterface(L"Container"));
		if (pContainer)
		{
			CControlUI *pFind = pContainer->GetChildContrlByClass(pstrClassName);
			if (pFind)
				return pFind;
		} //end if (pContainer)
	} //end for (int it = 0; 
	return NULL;
}

void CContainerUI::SetAutoDestroy(bool bAuto)
{
	m_bAutoDestroy = bAuto;
}

/*
void CContainerUI::SetInset(SIZE szInset)
{
   m_rcInset.left = m_rcInset.right = szInset.cx;
   m_rcInset.top = m_rcInset.bottom = szInset.cy;
}

void CContainerUI::SetInset(RECT rcInset)
{
   m_rcInset = rcInset;
}
*/

void CContainerUI::SetPadding(int iPadding)
{
	m_iPadding = iPadding;
}

 
void CContainerUI::ParentChangeVisible(bool bVisible)
{
	for (int it = 0; it < m_ChildList.GetSize(); it++) 
	{ 
		m_ChildList[it]->ParentChangeVisible(bVisible);
	}
	CControlUI::ParentChangeVisible(bVisible);
}

CScrollBarUI *CContainerUI::GetVerticalScrollBar()
{
	return m_pVScrollBar;
}

CScrollBarUI *CContainerUI::GetHorizontalScrollBar()
{
	return m_pHScrollBar;
}

void CContainerUI::SetVisible(bool bVisible)
{
   // Hide possible scrollbar control
	if (m_pVScrollBar)
	{
		m_pVScrollBar->SetVisible(bVisible);
	}
	if (m_pHScrollBar) 
	{
		m_pHScrollBar->SetVisible(bVisible);
	}
	// Hide children as well, scrollbar not included
	for (int it = 0; it < m_ChildList.GetSize(); it++) 
	{ 
		m_ChildList[it]->ParentChangeVisible(bVisible);
	}
	CControlUI::SetVisible(bVisible);
}

bool CContainerUI::IsVisible() const
{
	return CControlUI::IsVisible(); // (m_bVisible && (m_items.GetSize() > 0));
}

void CContainerUI::Event(TEventUI& event)
{
	CControlUI::Event(event);
}

void CContainerUI::EnableScrollBar(UINT nType, bool bEnable)
{
	//vertical scroll bar
	if (nType == UISB_VERT)
	{
		if (bEnable && m_pVScrollBar == NULL)
		{
			m_pVScrollBar = new CScrollBarUI(this, UISB_VERT);
			m_pVScrollBar->SetManager(m_pManager, this);
		} //end if (bEnable && ..
		if (!bEnable && m_pVScrollBar != NULL)
		{
			delete m_pVScrollBar;
			m_pVScrollBar = NULL;
		} //
	} else if (nType == UISB_HORZ)
	{
		//horizontal scrollbar
		if (bEnable && m_pHScrollBar == NULL)
		{
			m_pHScrollBar = new CScrollBarUI(this, UISB_HORZ);
			m_pHScrollBar->SetManager(m_pManager, this);
		}
		if (!bEnable && m_pHScrollBar != NULL)
		{
			delete m_pHScrollBar;
			m_pHScrollBar = NULL;
		}	//end if (!bEnable ...
	} //end else if (nType == UISB_VERT)
}

int CContainerUI::FindSelectable(int iIndex, bool bForward /*= true*/) const
{
   // NOTE: This is actually a helper-function for the list/combo/ect controls
   //       that allow them to find the next enabled/available selectable item
   if (GetCount() == 0) 
	   return -1;
   iIndex = CLAMP(iIndex, 0, GetCount() - 1);
   if (bForward)
   {
      for (int i = iIndex; i < GetCount(); i++) 
	  {
         if ((GetItem(i)->GetInterface(_T("ListItem")) != NULL) 
             && GetItem(i)->IsVisible()
             && GetItem(i)->IsEnabled()) 
			 return i;
      }
      return -1;
   }  else 
   {
      for (int i = iIndex; i >= 0; --i) 
	  {
         if ((GetItem(i)->GetInterface(_T("ListItem")) != NULL)
             && GetItem(i)->IsVisible()
             && GetItem(i)->IsEnabled()) 
			 return i;
      }
      return FindSelectable(0, true);
   }
}

void CContainerUI::SetPos(RECT rc)
{
	CControlUI::SetPos(rc);
    if (m_ChildList.IsEmpty()) 
		return;
	//   rc.left += m_rcInset.left;
	//   rc.top += m_rcInset.top;
	//   rc.right -= m_rcInset.right;
	//   rc.bottom -= m_rcInset.bottom;
	// We'll position the first child in the entire client area.
	// Do not leave a container empty.
	//ASSERT(m_items.GetSize()==1);
	m_ChildList[0]->SetPos(m_rcItem);
}

SIZE CContainerUI::EstimateSize(SIZE /*szAvailable*/)
{
	return m_cxyFixed;
}

BOOL CContainerUI::GetAttribute(LPCTSTR pstrName, TCHAR *szValue, int &nMaxValueSize)
{
	//   if( _tcscmp(pstrName, _T("inset")) == 0 ) SetInset(CSize(_ttoi(pstrValue), _ttoi(pstrValue)));
	if (_tcsicmp(pstrName, _T("padding")) == 0)
	{
		TCHAR szTmp[16] = {0};
		_ltow(m_iPadding, szTmp, 16);
		if (nMaxValueSize > lstrlen(szTmp))
		{
			lstrcpy(szValue, szTmp);
			nMaxValueSize = lstrlen(szTmp);
			return TRUE;
		} 
	} else if (_tcsicmp(pstrName, _T("background")) == 0)
	{
		TCHAR szTmp[16] = {0};
		_ltow(m_nBkgImageId, szTmp, 16);
		if (nMaxValueSize > lstrlen(szTmp))
		{
			lstrcpy(szValue, szTmp);
			nMaxValueSize = lstrlen(szTmp);
			return TRUE;
		} 
	}  else
	{
		return CControlUI::GetAttribute(pstrName, szValue, nMaxValueSize);
	}
	return FALSE;
}

void CContainerUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	//   if( _tcscmp(pstrName, _T("inset")) == 0 ) SetInset(CSize(_ttoi(pstrValue), _ttoi(pstrValue)));
	if (_tcsicmp(pstrName, _T("padding")) == 0)
	{
		SetPadding(_ttoi(pstrValue));
	} else if (_tcsicmp(pstrName, _T("background")) == 0)
	{
		m_nBkgImageId = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("leftimageid")) == 0)
	{
		m_nBkgLeftImageId = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("rightimageid")) == 0)
	{
		m_nBkgRightImageId = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("leftsize")) == 0)
	{ 
	    LPTSTR pstr = NULL;
	    m_szLeft.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
	    m_szLeft.cy = _tcstol(pstr + 1, &pstr, 10);  
	} else if (_tcsicmp(pstrName, _T("rightsize")) == 0)
	{ 
	    LPTSTR pstr = NULL;
	    m_szRight.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
	    m_szRight.cy = _tcstol(pstr + 1, &pstr, 10); 
	} else if (_tcsicmp(pstrName, _T("bkgsize")) == 0)
	{
		LPTSTR pstr = NULL;
	    int cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
	    int cy = _tcstol(pstr + 1, &pstr, 10);
		if ((cx > 0) && (cy > 0))
		{
			m_nStretchMode = SM_FIXED4CORNERS;
			m_stretchFixed.m_iBotHeight = cy;
			m_stretchFixed.m_iBotLeftWidth = cx;
			m_stretchFixed.m_iBotRightWidth = cx;
			m_stretchFixed.m_iCenterLeftWidth = cx;
			m_stretchFixed.m_iCenterRightWidth = cx;
			m_stretchFixed.m_iTopHeight = cy;
			m_stretchFixed.m_iTopLeftWidth = cx;
			m_stretchFixed.m_iTopRightWidth = cx;
		}
	} else if (_tcsicmp(pstrName, _T("bkghole")) == 0)
	{
		m_bHole = (_tcsicmp(pstrValue, _T("true")) == 0);
	} else if (_tcsicmp(pstrName, _T("scrollbar")) == 0)
	{
		if (_tcsicmp(pstrValue, _T("vertical")) == 0)
		{
			EnableScrollBar(UISB_VERT, true); 
		} else if (_tcsicmp(pstrValue, _T("horizontal")) == 0)
		{
			EnableScrollBar(UISB_HORZ, true); 
		} else if (_tcsicmp(pstrValue, _T("both")) == 0)
		{
			EnableScrollBar(UISB_VERT, true);
			EnableScrollBar(UISB_HORZ, true); 
		} else if (_tcsicmp(pstrValue, _T("neither")) == 0)
		{
			EnableScrollBar(UISB_VERT, false);
			EnableScrollBar(UISB_HORZ, false);
		}
	} else if (_tcsicmp(pstrName, _T("style")) == 0)
	{
		//只是一个标志节点 继承处理
	} else if (_tcsicmp(pstrName, _T("windowbackimage")) == 0)
	{
		//
	} else if (_tcsicmp(pstrName, _T("CaptionSize")) == 0)
	{
		//
	} else
	{
		CControlUI::SetAttribute(pstrName, pstrValue);
	}
}

void CContainerUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent)
{
	for (int it = 0; it < m_ChildList.GetSize(); it++)
	{
		m_ChildList[it]->SetManager(pManager, this);
	}
	//scrollbar
	if (m_pVScrollBar)
	{
		m_pVScrollBar->SetManager(pManager, this);
	}
	if (m_pHScrollBar)
	{
		m_pHScrollBar->SetManager(pManager, this);
	}
	CControlUI::SetManager(pManager, pParent);
}

CControlUI* CContainerUI::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags)
{
	// Check if this guy is valid
	if ((uFlags & UIFIND_VISIBLE) != 0 && !IsVisible())
		return NULL;
	if ((uFlags & UIFIND_ENABLED) != 0 && !IsEnabled()) 
		return NULL;
	if ((uFlags & UIFIND_HITTEST) != 0 && !::PtInRect(&m_rcItem, *(static_cast<LPPOINT>(pData))))
		return NULL;
	if ((uFlags & UIFIND_ME_FIRST) != 0) 
	{
		CControlUI* pControl = CControlUI::FindControl(Proc, pData, uFlags);
		if ( pControl != NULL) 
			return pControl;
	}
	for (int it = 0; it != m_ChildList.GetSize(); it++) 
	{
		CControlUI* pControl = m_ChildList[it]->FindControl(Proc, pData, uFlags);
		if (pControl != NULL) 
			return pControl;
	}
	//现在，不把scrollbar添加到container中
	CControlUI* pCtrl = NULL;
	if (m_pVScrollBar && (pCtrl = m_pVScrollBar->FindControl(Proc, pData, uFlags)) != NULL)
	{
		return pCtrl;
	}
	if (m_pHScrollBar && (pCtrl = m_pHScrollBar->FindControl(Proc, pData, uFlags)) != NULL)
	{
		return pCtrl;
	}
	return CControlUI::FindControl(Proc, pData, uFlags);
}

void CContainerUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	if (IsVisible())
	{
		RECT rcTemp = { 0 };
		if (!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem)) 
			return;
		CControlUI::DoPaint(hDC, rcPaint);
		CRenderClip clip;
		CBlueRenderEngineUI::GenerateClip(hDC, m_rcItem, clip);
		if (m_nBkgImageId > 0) 
			CBlueRenderEngineUI::DoPaintGraphic(hDC, m_pManager, m_rcItem, m_nBkgImageId,
			                       m_stretchFixed, m_nStretchMode);
		if (m_nBkgLeftImageId > 0)
		{
			RECT rc = m_rcItem;
			rc.bottom -= m_szLeft.cy;
			rc.left += m_szLeft.cx;
			CBlueRenderEngineUI::DoPaintGraphic(hDC, m_pManager, rc, m_nBkgLeftImageId, PA_BOTTOMLEFT);
		} 
		if (m_nBkgRightImageId > 0)
		{
			RECT rc = m_rcItem;
			rc.right -= m_szRight.cx;
			rc.bottom -= m_szRight.cy;
			CBlueRenderEngineUI::DoPaintGraphic(hDC, m_pManager, rc, m_nBkgRightImageId, PA_BOTTOMRIGHT);
		}
		
		if (m_ChildList.GetSize() == 0)
			return;
		for (int it = 0; it < m_ChildList.GetSize(); it++)
		{
			CControlUI* pControl =  m_ChildList[it];
			if (!pControl->IsVisible()) 
				continue;
			if (!::IntersectRect(&rcTemp, &rcPaint, &pControl->GetPos()))
				continue;
			if (!::IntersectRect(&rcTemp, &m_rcItem, &pControl->GetPos()))
				continue;
			pControl->DoPaint(hDC, rcPaint);
		}
		//绘制滚动条
		if (m_pVScrollBar && m_pVScrollBar->IsVisible())
		{
			m_pVScrollBar->DoPaint(hDC, rcPaint);
		}
		if (m_pHScrollBar && m_pHScrollBar->IsVisible())
		{
			m_pHScrollBar->DoPaint(hDC, rcPaint);
		}
	}
	CControlUI::DoPaintBorder(hDC, rcPaint);
}

bool CContainerUI::IsScrollBarVisible(UINT nScrollBarType) const
{
	if (nScrollBarType == UISB_VERT && m_pVScrollBar)
		return m_pVScrollBar->IsVisible();
	else if (UISB_HORZ == nScrollBarType && m_pHScrollBar)
		return m_pHScrollBar->IsVisible();
	return false;
}

void CContainerUI::SetScrollBarPos(UINT nScrollBarType, const RECT& rc)
{
	if (nScrollBarType == UISB_VERT && m_pVScrollBar)
		m_pVScrollBar->SetPos( rc );
	else if (UISB_HORZ == nScrollBarType && m_pHScrollBar)
		m_pHScrollBar->SetPos( rc );
}

void CContainerUI::SetScrollPos(UINT nScrollBarType, int iPos)
{
	if (nScrollBarType == UISB_VERT && m_pVScrollBar)
		m_pVScrollBar->SetScrollPos(iPos);
	else if (UISB_HORZ == nScrollBarType && m_pHScrollBar)
		m_pHScrollBar->SetScrollPos(iPos);
}

int CContainerUI::GetScrollPos(UINT nScrollBarType) const
{
	if (nScrollBarType == UISB_VERT && m_pVScrollBar)
		return m_pVScrollBar->GetScrollPos();
	else if (UISB_HORZ == nScrollBarType && m_pHScrollBar)
		return m_pHScrollBar->GetScrollPos();
	return 0;
}

void CContainerUI::SetScrollRange(UINT nScrollBarType, int iMin, int iMax)
{
	if (nScrollBarType == UISB_VERT && m_pVScrollBar)
		m_pVScrollBar->SetScrollRange(iMin, iMax);
	else if (UISB_HORZ == nScrollBarType && m_pHScrollBar)
		m_pHScrollBar->SetScrollRange(iMin, iMax);
}
BOOL CContainerUI::GetScrollRange(UINT nScrollBarType, int& iMin, int& iMax) const
{
	if (nScrollBarType == UISB_VERT && m_pVScrollBar)
		return m_pVScrollBar->GetScrollRange(iMin, iMax);
	else if (UISB_HORZ == nScrollBarType && m_pHScrollBar)
		return m_pHScrollBar->GetScrollRange(iMin, iMax);
	return false;
}
void CContainerUI::SetScrollPage(UINT nScrollBarType, int iPage)
{
	if (nScrollBarType == UISB_VERT && m_pVScrollBar)
		m_pVScrollBar->SetScrollPage(iPage);
	else if (UISB_HORZ == nScrollBarType && m_pHScrollBar)
		m_pHScrollBar->SetScrollPage(iPage);
}
int CContainerUI::GetScrollPage(UINT nScrollBarType) const
{
	if (nScrollBarType == UISB_VERT && m_pVScrollBar)
		return m_pVScrollBar->GetScrollPage();
	else if (UISB_HORZ == nScrollBarType && m_pHScrollBar)
		return m_pHScrollBar->GetScrollPage();
	return 0;
}

void CContainerUI::DoPaintScrollBar(UINT nScrollBarType, HDC hDC, const RECT& rcPaint)
{
	if (nScrollBarType == UISB_VERT && m_pVScrollBar)
		m_pVScrollBar->DoPaint(hDC, rcPaint);
	else if (UISB_HORZ == nScrollBarType && m_pHScrollBar)
		m_pHScrollBar->DoPaint(hDC, rcPaint);
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

CCanvasUI::CCanvasUI() 
{
	m_szClientCorner.cx = m_szClientCorner.cy = 0;
}

CCanvasUI::~CCanvasUI()
{
}

LPCTSTR CCanvasUI::GetClass() const
{
	return _T("CanvasUI");
}

void CCanvasUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	RECT rcFill = { 0 };
	if (!::IntersectRect(&rcFill, &rcPaint, &m_rcItem))
		return;
	// Fill background & Draw Border 
	COLORREF clrBorder = HasBorder() ? GetBorderColor() : GetBkgndColor();
	if ((m_szClientCorner.cx != 0) || (m_szClientCorner.cy != 0))
	{
		CBlueRenderEngineUI::DoPaintRoundRect(hDC, m_rcItem, clrBorder, GetBkgndColor(), m_szClientCorner);
	} else
	{
		CBlueRenderEngineUI::DoPaintRectangle(hDC, m_rcItem, clrBorder, GetBkgndColor());
	}
	CContainerUI::DoPaint(hDC, rcPaint);
}

void CCanvasUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcsicmp(pstrName, _T("watermark")) == 0)
	{
	} else if (_tcsicmp(pstrName, _T("childindent")) == 0)
	{
	} else if (_tcsicmp(pstrName, _T("roundrect")) == 0)
	{
         m_szClientCorner.cx = m_szClientCorner.cy = _ttoi(pstrValue);
	} else 
	{ 
		CContainerUI::SetAttribute(pstrName, pstrValue);
	}
}
/////////////////////////////////////////////////////////////////////////////////////
//
//
CImageCanvasUI::CImageCanvasUI():
	            m_iImageID(IMGID_INVALID_),
	            m_nStretchMode(SM_FIXED4CORNERS),
				m_iShadeImgId(0),
				m_pGraphic(NULL)
{
	memset(&m_stretchFixed, 0, sizeof(m_stretchFixed));
}

CImageCanvasUI::~CImageCanvasUI()
{
	if (m_pGraphic)
		delete m_pGraphic;
	m_pGraphic = NULL;
}

LPCTSTR CImageCanvasUI::GetClass() const
{
	return _T("ImageCanvasUI");
}

void CImageCanvasUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	RECT rcFill = { 0 };
	if (!::IntersectRect( &rcFill, &m_rcItem, &rcPaint))
		return;

	//paint background
	CRect rcImage(m_rcItem);
	if (m_pGraphic)
		m_pGraphic->DrawToDc(hDC, rcImage);
	else if (m_iImageID > 0)
		CBlueRenderEngineUI::DoPaintGraphic(hDC, m_pManager, rcImage, m_iImageID, 
		                             m_stretchFixed, m_nStretchMode);
	if (m_iShadeImgId > 0)
	{
		CBlueRenderEngineUI::DoPaintGraphic(hDC, m_pManager, rcImage, m_iShadeImgId);
	}
	//as container, child uis
	CContainerUI::DoPaint(hDC, rcPaint);	
}

void CImageCanvasUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcsicmp( pstrName, _T("image")) == 0)
	{
		m_iImageID = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("stretchmode") ) == 0)
	{
		if (_tcsicmp(pstrValue, _T("normal")) == 0)
			m_nStretchMode = SM_NORMALSTRETCH;
		else if (_tcsicmp(pstrValue, _T("fix4corners")) == 0)
			m_nStretchMode = SM_FIXED4CORNERS;
		else if (_tcsicmp(pstrValue, _T("horizontal")) == 0)
			m_nStretchMode = SM_HORIZONTAL;
		else if (_tcsicmp(pstrValue, _T("vertical")) == 0) 
			m_nStretchMode = SM_VERTICAL;
		else if (_tcsicmp(pstrValue, _T("fix4c_vertical")) == 0)
			m_nStretchMode = SM_FIX4CVER;
	} else if (_tcsicmp(pstrName, _T("fixedcorner")) == 0)
	{
		m_stretchFixed.SetFixed(_ttoi(pstrValue));
	} else if (_tcsicmp(pstrName, _T("topleft_width")) == 0)
	{
		m_stretchFixed.m_iTopLeftWidth =_ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("topright_width")) == 0)
	{
		m_stretchFixed.m_iTopRightWidth = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("top_height")) == 0)
	{
		m_stretchFixed.m_iTopHeight = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("botleft_width")) == 0)
	{
		m_stretchFixed.m_iBotLeftWidth = _ttoi(pstrValue);
	} else if ( _tcsicmp(pstrName, _T("botright_width")) == 0)
	{
		m_stretchFixed.m_iBotRightWidth = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("bot_height")) == 0)
	{
		m_stretchFixed.m_iBotHeight = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("center_left_width")) == 0)
	{
		m_stretchFixed.m_iCenterLeftWidth = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("center_right_width")) == 0)
	{
		m_stretchFixed.m_iCenterRightWidth = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("center_height")) == 0)
	{
		m_stretchFixed.m_iCenterHeight = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("texture_colorfirst")) == 0)
	{
		m_stretchFixed.m_crTexture1 = StringToColor(pstrValue);
	} else if (_tcsicmp(pstrName, _T("texture_colorsecond")) == 0)
	{
		m_stretchFixed.m_crTexture2 = StringToColor(pstrValue);
	} else if (_tcsicmp(pstrName, _T("texture_width")) == 0)
	{
		m_stretchFixed.m_nTextureWidth = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("texture_image")) == 0)
	{
		m_stretchFixed.m_nTextureImage = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("imagefile")) == 0)
	{
		if (m_pGraphic)
			delete m_pGraphic;
		m_pGraphic = NULL;
		if (pstrValue)
		{
		    char szFileName[MAX_PATH] = {0};
			CStringConversion::WideCharToString(pstrValue, szFileName, MAX_PATH - 1);
			m_pGraphic = NEWIMAGE;
			if (!m_pGraphic->LoadFromFile(szFileName, FALSE))
			{
				delete m_pGraphic;
				m_pGraphic = NULL;
			}
		}
	} else if (_tcsicmp(pstrName, _T("shadeimage")) == 0)
	{
		m_iShadeImgId = _ttoi(pstrValue);
	} else
		CCanvasUI::SetAttribute(pstrName, pstrValue);
}

void CImageCanvasUI::SetImage(UINT nImageID)
{
	if (m_iImageID != nImageID)
	{
		m_iImageID = nImageID;
		Invalidate();
	}
}

void CImageCanvasUI::SetStretch(const StretchFixed& sf)
{
	m_stretchFixed = sf;
	Invalidate();
}

//CLuxCanvasUI
CLuxCanvasUI::CLuxCanvasUI():
              m_iBkgImageId(0),
			  m_iBotLeftImageId(0),
			  m_iBotRightImageId(0),
			  m_nBotLeftX(0),
			  m_nBotLeftY(0),
			  m_nBotRightX(0),
			  m_nBotRightY(0)
{
	//
}

CLuxCanvasUI::~CLuxCanvasUI()
{
	//
}

LPCTSTR CLuxCanvasUI::GetClass() const
{
	return _T("LuxCanvasUI");
}
 
void CLuxCanvasUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	//
}

void CLuxCanvasUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcsicmp(pstrName, _T("BackImageId")) == 0)
	{
		m_iBkgImageId = _ttoi(pstrValue); 
	} else if (_tcsicmp(pstrName, _T("BotLeftImageId")) == 0)
	{
		m_iBotLeftImageId = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("BotRightImageId")) == 0)
	{
		m_iBotRightImageId = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("BotLeftX")) == 0)
	{
		m_nBotLeftX = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("BotLeftY")) == 0)
	{
		m_nBotLeftY = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("BotRightX")) == 0)
	{
		m_nBotRightX = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("BotRightY")) == 0)
	{
		m_nBotRightY = _ttoi(pstrValue);
	}
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

CWindowCanvasUI::CWindowCanvasUI()
{
	SetBkgndColor(m_pManager->GetThemeColor(UICOLOR_WINDOW_BACKGROUND));
}

LPCTSTR CWindowCanvasUI::GetClass() const
{
	return _T("WindowCanvasUI");
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CControlCanvasUI::CControlCanvasUI()
{
	SetBkgndColor(m_pManager->GetThemeColor(UICOLOR_CONTROL_BACKGROUND_NORMAL));
}

LPCTSTR CControlCanvasUI::GetClass() const
{
	return _T("ControlCanvasUI");
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CWhiteCanvasUI::CWhiteCanvasUI()
{
	SetBkgndColor(m_pManager->GetThemeColor(UICOLOR_STANDARD_WHITE)); 
}

void CWhiteCanvasUI::SetBkgndColor(COLORREF clrBkgnd )
{
	if (clrBkgnd == 0)
		m_clrBkgnd = m_pManager->GetThemeColor(UICOLOR_STANDARD_WHITE);
	else
		CCanvasUI::SetBkgndColor(clrBkgnd);
}

LPCTSTR CWhiteCanvasUI::GetClass() const
{
	return _T("WhiteCanvasUI");
}

void CWhiteCanvasUI::DoPaint(HDC hDC, const RECT& rcPaint)
{ 
	if (HasBorder())
	{
		CCanvasUI::DoPaint(hDC, rcPaint);
	} else
	{ 
		CContainerUI::DoPaint(hDC, rcPaint);
	}
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

CDialogCanvasUI::CDialogCanvasUI()
{
	SetBkgndColor(m_pManager->GetThemeColor(UICOLOR_DIALOG_BACKGROUND));
}

LPCTSTR CDialogCanvasUI::GetClass() const
{
	return _T("DialogCanvasUI");
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CTabFolderCanvasUI::CTabFolderCanvasUI()
{
	COLORREF clrColor1, clrColor2;
	m_pManager->GetThemeColorPair(UICOLOR_TAB_FOLDER_NORMAL, clrColor1, clrColor2);
	SetBkgndColor(clrColor2);
}

LPCTSTR CTabFolderCanvasUI::GetClass() const
{
	return _T("TabFolderCanvasUI");
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CVerticalLayoutUI::CVerticalLayoutUI():
                   m_cyNeeded(0)
{
}

LPCTSTR CVerticalLayoutUI::GetClass() const
{
	return _T("VertialLayoutUI");
}

void CVerticalLayoutUI::Notify(TNotifyUI& msg)
{
	int iPos = GetScrollPos(UISB_VERT);
	if (msg.sType == _T("lineup"))
	{
		SetScrollPos(UISB_VERT, iPos - 20);
		if (iPos != GetScrollPos( UISB_VERT)) 
			UpdateLayout();
	} else if (msg.sType == _T("linedown"))
	{
		SetScrollPos(UISB_VERT, iPos + 20);
		if (iPos != GetScrollPos( UISB_VERT)) 
			UpdateLayout();
	} else if (msg.sType == _T("pageup"))
	{
		SetScrollPos(UISB_VERT, iPos - GetScrollPage(UISB_VERT));
		if (iPos != GetScrollPos(UISB_VERT)) 
			UpdateLayout();
	} else if (msg.sType == _T("pagedown"))
	{
		SetScrollPos(UISB_VERT, iPos + GetScrollPage(UISB_VERT));
		if (iPos != GetScrollPos(UISB_VERT))
			UpdateLayout();
	} else if (msg.sType == _T("thumbtrack"))
	{
		// msg.sType = _T("thumbtrack"), 我犯的错误
		SetScrollPos(UISB_VERT, (int)msg.wParam);
		if (iPos != GetScrollPos(UISB_VERT)) 
			UpdateLayout();
	} else if (msg.sType == _T("visibilitychanged"))
	{
		if (msg.wParam == 1)
		{
			RECT rc = m_rcItem;
			SetPos(rc);
		} else
		{
			RECT rc = m_rcItem;
			SetPos(rc);
		}
	}
}

void CVerticalLayoutUI::SetPos(RECT rc)
{
	//   m_rcItem = rc;
	// Adjust for inset
	//   rc.left += m_rcInset.left;
	//   rc.top += m_rcInset.top;
	//   rc.right -= m_rcInset.right;
	//   rc.bottom -= m_rcInset.bottom;
	//TBD
	CControlUI::SetPos(rc);//container position
	rc = m_rcItem;
	//scrollbar position TBD
	int iScrollPos = 0;
	if (IsScrollBarVisible(UISB_VERT))
	{
		CRect rcScroll(rc);
		rcScroll.left = rcScroll.right - SB_WIDTH;
		if (m_pVScrollBar)
		{
			m_pVScrollBar->SetPos(rcScroll);
			rc.right -= SB_WIDTH;
			iScrollPos = GetScrollPos(UISB_VERT);
		}
	}
	// Determine the minimum size
	SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
	int nAdjustables = 0;
	int cyFixed = 0;
	for (int it1 = 0; it1 < m_ChildList.GetSize(); it1++) 
	{
		CControlUI* pControl = m_ChildList[it1];
		if (!pControl->IsVisible()) 
			continue;
		SIZE sz = pControl->EstimateSize(szAvailable);
		if( sz.cy == 0 ) nAdjustables++;
		cyFixed += sz.cy + m_iPadding;
	}
	// Place elements
	int cyNeeded = 0;
	int cyExpand = 0;
	if (nAdjustables > 0)
	{
		cyExpand = MAX(0, (szAvailable.cy - cyFixed) / nAdjustables);
	}
	// Position the elements
	SIZE szRemaining = szAvailable;
	int iPosY = rc.top - iScrollPos;
	int iAdjustable = 0;
	for (int it2 = 0; it2 < m_ChildList.GetSize(); it2++)
	{
		CControlUI* pControl = m_ChildList[it2];
		if (!pControl->IsVisible()) 
			continue;
		SIZE sz = pControl->EstimateSize(szRemaining);
		if (sz.cy == 0)
		{
			iAdjustable++;
			sz.cy = cyExpand;
			// Distribute remaining to last element (usually round-off left-overs)
			if (iAdjustable == nAdjustables)
			{
				sz.cy += MAX(0, szAvailable.cy - (cyExpand * nAdjustables) - cyFixed);
			}
		}
		RECT rcCtrl = { rc.left, iPosY, rc.right, iPosY + sz.cy };
		pControl->SetPos(rcCtrl);
		iPosY += sz.cy + m_iPadding;
		cyNeeded += sz.cy + m_iPadding;
		szRemaining.cy -= sz.cy + m_iPadding;
	}
	// Handle overflow with scrollbars
	SetScrollRange(UISB_VERT, 0, cyNeeded-1);
	SetScrollPage(UISB_VERT, rc.bottom - rc.top);
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CHorizontalLayoutUI::CHorizontalLayoutUI()
{
	//
}

LPCTSTR CHorizontalLayoutUI::GetClass() const
{
	return _T("HorizontalLayoutUI");
}

void CHorizontalLayoutUI::SetPos(RECT rc)
{
	//  m_rcItem = rc;
	// Adjust for inset
	// rc.left += m_rcInset.left;
	//   rc.top += m_rcInset.top;
	//   rc.right -= m_rcInset.right;
	//   rc.bottom -= m_rcInset.bottom;
	CControlUI::SetPos( rc );
	rc = m_rcItem;
	// Determine the width of elements that are sizeable
	SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
	int nAdjustables = 0;
	int cxFixed = 0;
	for (int it1 = 0; it1 < m_ChildList.GetSize(); it1++) 
	{
		CControlUI* pControl = m_ChildList[it1];
		if (!pControl->IsVisible()) 
			continue;
		SIZE sz = pControl->EstimateSize(szAvailable);
		if (sz.cx == 0) 
			nAdjustables++;
		cxFixed += sz.cx + m_iPadding;
	}
	int cxExpand = 0;
	if (nAdjustables > 0) 
		cxExpand = MAX(0, (szAvailable.cx - cxFixed) / nAdjustables);
	// Position the elements
	SIZE szRemaining = szAvailable;
	int iPosX = rc.left;
	int iAdjustable = 0;
	for (int it2 = 0; it2 < m_ChildList.GetSize(); it2++)
	{
		CControlUI* pControl = m_ChildList[it2];
		if( !pControl->IsVisible() ) 
			continue;
		SIZE sz = pControl->EstimateSize(szRemaining);
		if (sz.cx == 0)
		{
			iAdjustable++;
			sz.cx = cxExpand;
			if (iAdjustable == nAdjustables)
				sz.cx += MAX(0, szAvailable.cx - (cxExpand * nAdjustables) - cxFixed);
		}
		RECT rcCtrl = { iPosX, rc.top, iPosX + sz.cx, rc.bottom };
		pControl->SetPos(rcCtrl);
		iPosX += sz.cx + m_iPadding;
		szRemaining.cx -= sz.cx + m_iPadding;
	}
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CTileLayoutUI::CTileLayoutUI(): 
               m_nColumns(2), 
			   m_cyNeeded(0)
{
	SetPadding(10);
	SetInset(CRect(10,10,10,10));
}

LPCTSTR CTileLayoutUI::GetClass() const
{
	return _T("TileLayoutUI");
}

void CTileLayoutUI::SetColumns(int nCols)
{
	if (nCols <= 0)
		return;
	m_nColumns = nCols;
	UpdateLayout();
}

void CTileLayoutUI::SetPos(RECT rc)
{
	//   m_rcItem = rc;
	// Adjust for inset
	//   rc.left += m_rcInset.left;
	//   rc.top += m_rcInset.top;
	//   rc.right -= m_rcInset.right;
	//   rc.bottom -= m_rcInset.bottom;
	CControlUI::SetPos( rc );
	rc = m_rcItem;
	//	if( IsScrollBarVisible() ){
	//		CRect rcScroll( rc );
	//		rc.left = rc.right - SB_WIDTH;
	//		SetScrollBarPos( rcScroll );
	//		rc.right -= SB_WIDTH;
	//	}
	// Position the elements
	int cxWidth = (rc.right - rc.left) / m_nColumns;
	int cyHeight = 0;
	int iCount = 0;
	POINT ptTile = { rc.left, rc.top /*- m_iScrollPos*/ };
	for (int it1 = 0; it1 < m_ChildList.GetSize(); it1++)
	{
		CControlUI* pControl = m_ChildList[it1];
		if (!pControl->IsVisible())
			continue;
		// Determine size
		RECT rcTile = { ptTile.x, ptTile.y, ptTile.x + cxWidth, ptTile.y };
		// Adjust with element padding
		if ((iCount % m_nColumns) == 0)
			rcTile.right -= m_iPadding / 2;
		else if ((iCount % m_nColumns) == m_nColumns - 1) 
			rcTile.left += m_iPadding / 2;
		else ::InflateRect(&rcTile, -(m_iPadding / 2), 0);
		// If this panel expands vertically
		if (m_cxyFixed.cy == 0) 
		{
			SIZE szAvailable = { rcTile.right - rcTile.left, 9999 };
			int iIndex = iCount;
			for (int it2 = it1; it2 < m_ChildList.GetSize(); it2++)
			{
				SIZE szTile = m_ChildList[it2]->EstimateSize(szAvailable);
				cyHeight = MAX(cyHeight, szTile.cy);
				if ((++iIndex % m_nColumns) == 0)
					break;
			}
		}
		// Set position
		rcTile.bottom = rcTile.top + cyHeight;
		pControl->SetPos(rcTile);
		// Move along...
		if ((++iCount % m_nColumns) == 0)
		{
			ptTile.x = rc.left;
			ptTile.y += cyHeight + m_iPadding;
			cyHeight = 0;
		} else 
		{
			ptTile.x += cxWidth;
		}
		m_cyNeeded = rcTile.bottom - (rc.top /*- m_iScrollPos*/ );
	}
	// Process the scrollbar
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CDialogLayoutUI::CDialogLayoutUI() : m_bFirstResize(true), m_aModes(sizeof(STRETCHMODE))
{
	::ZeroMemory(&m_rcDialog, sizeof(m_rcDialog));
}

LPCTSTR CDialogLayoutUI::GetClass() const
{
	return _T("DialogLayoutUI");
}

LPVOID CDialogLayoutUI::GetInterface(LPCTSTR pstrName)
{
   if (_tcscmp(pstrName, _T("DialogLayout")) == 0)
	   return this;
   return CContainerUI::GetInterface(pstrName);
}

void CDialogLayoutUI::SetStretchMode(CControlUI* pControl, UINT uMode)
{
	STRETCHMODE mode;
	mode.pControl = pControl;
	mode.uMode = uMode;
	mode.rcItem = pControl->GetPos();
	m_aModes.Add(&mode);
}

SIZE CDialogLayoutUI::EstimateSize(SIZE szAvailable)
{
	RecalcArea();
	return CSize(m_rcDialog.right - m_rcDialog.left, m_rcDialog.bottom - m_rcDialog.top);
}

void CDialogLayoutUI::SetPos(RECT rc)
{
	m_rcItem = rc;
	RecalcArea();
	// Do Scrollbar
	//   ProcessScrollbar(rc, m_rcDialog.bottom - m_rcDialog.top);
	//   if( m_pVScrollBar != NULL ) rc.right -= m_pManager->GetSystemMetrics().cxvscroll;
	// Determine how "scaled" the dialog is compared to the original size
	int cxDiff = (rc.right - rc.left) - (m_rcDialog.right - m_rcDialog.left);
	int cyDiff = (rc.bottom - rc.top) - (m_rcDialog.bottom - m_rcDialog.top);
	if (cxDiff < 0)
		cxDiff = 0;
	if (cyDiff < 0) 
		cyDiff = 0;
	// Stretch each control
	// Controls are coupled in "groups", which determine a scaling factor.
	// A "line" indicator is used to apply the same scaling to a new group of controls.
	int nCount, cxStretch, cyStretch, cxMove, cyMove;
	for (int i = 0; i < m_aModes.GetSize(); i++) 
	{
		STRETCHMODE* pItem = static_cast<STRETCHMODE*>(m_aModes[i]);
		if (i == 0 || (pItem->uMode & UISTRETCH_NEWGROUP) != 0) 
		{
			nCount = 0;
			for (int j = i + 1; j < m_aModes.GetSize(); j++) 
			{
				STRETCHMODE* pNext = static_cast<STRETCHMODE*>(m_aModes[j]);
				if ((pNext->uMode & (UISTRETCH_NEWGROUP | UISTRETCH_NEWLINE)) != 0)
					break;
				if ((pNext->uMode & (UISTRETCH_SIZE_X | UISTRETCH_SIZE_Y)) != 0) 
					nCount++;
			}
			if (nCount == 0) 
				nCount = 1;
			cxStretch = cxDiff / nCount;
			cyStretch = cyDiff / nCount;
			cxMove = 0;
			cyMove = 0;
		}
		if ((pItem->uMode & UISTRETCH_NEWLINE) != 0)
		{
			cxMove = 0;
			cyMove = 0;
		}
		RECT rcPos = pItem->rcItem;
		::OffsetRect(&rcPos, rc.left, rc.top /*- m_iScrollPos*/ );
		if ((pItem->uMode & UISTRETCH_MOVE_X) != 0) 
			::OffsetRect(&rcPos, cxMove, 0);
		if ((pItem->uMode & UISTRETCH_MOVE_Y) != 0) 
			::OffsetRect(&rcPos, 0, cyMove);
		if ((pItem->uMode & UISTRETCH_SIZE_X) != 0) 
			rcPos.right += cxStretch;
		if ((pItem->uMode & UISTRETCH_SIZE_Y) != 0)
			rcPos.bottom += cyStretch;
		if ((pItem->uMode & (UISTRETCH_SIZE_X | UISTRETCH_SIZE_Y)) != 0)
		{
			cxMove += cxStretch;
			cyMove += cyStretch;
		}      
		pItem->pControl->SetPos(rcPos);
	}
}

void CDialogLayoutUI::RecalcArea()
{
	if (!m_bFirstResize)
		return;
	// Add the remaining control to the list
	// Controls that have specific stretching needs will define them in the XML resource
	// and by calling SetStretchMode(). Other controls needs to be added as well now...
	for (int it = 0; it < m_ChildList.GetSize(); it++)
	{
		CControlUI* pControl = m_ChildList[it];
		bool bFound = false;
		for (int i = 0; !bFound && i < m_aModes.GetSize(); i++)
		{
			if (static_cast<STRETCHMODE*>(m_aModes[i])->pControl == pControl)
				bFound = true;
		}
		if (!bFound)
		{
			STRETCHMODE mode;
			mode.pControl = pControl;
			mode.uMode = UISTRETCH_NEWGROUP;
			mode.rcItem = pControl->GetPos();
			m_aModes.Add(&mode);
		}
	}
	// Figure out the actual size of the dialog so we can add proper scrollbars later
	CRect rcDialog(9999, 9999, 0, 0);
	for (int i = 0; i < m_ChildList.GetSize(); i++) 
	{
		const RECT& rcPos = m_ChildList[i]->GetPos();
		rcDialog.Join(rcPos);
	}
	for (int j = 0; j < m_aModes.GetSize(); j++) 
	{
		RECT& rcPos = static_cast<STRETCHMODE*>(m_aModes[j])->rcItem;
		::OffsetRect(&rcPos, -rcDialog.left, -rcDialog.top);
	}
	m_rcDialog = rcDialog;
	// We're done with initialization
	m_bFirstResize = false;
}

//CShortCutItem
CShortCutItem::CShortCutItem(const TCHAR *szLabelCaption, const TCHAR *szFilePath,  int nBkgImageId,
	int nFloatImageId,	const TCHAR *szTip):
               CVerticalLayoutUI()
        
{
	char szTmp[128] = {0};
	int nSize = 127;
	CSystemUtils::GetGuidString(szTmp, &nSize);
	TCHAR szwTmp[128] = {0};
	CStringConversion::StringToWideChar(szTmp, szwTmp, 127);
	m_strShortCutName = szwTmp;
	//
	CImageButtonUI *pButton = new CImageButtonUI();
	pButton->SetImage((UINT)nBkgImageId);
	pButton->SetToolTip(szTip);
	pButton->SetName(szwTmp);
	if (nFloatImageId > 0)
	{
		pButton->SetFloatImage(nFloatImageId);
	} else if (szFilePath)
	{
		pButton->SetAttribute(L"floatshortcutname", szFilePath);
	} 
	Add(pButton);
	m_pButton = pButton; 

	//add CLabelPanelUI
	CLabelPanelUI *pLbl = new CLabelPanelUI(); 
	pLbl->SetToolTip(szTip);
	pLbl->SetText(szLabelCaption);
	m_pLbl = pLbl; 
	Add(pLbl); 
}
 
void CShortCutItem::Notify(TNotifyUI& msg)
{
	CVerticalLayoutUI::Notify(msg);
}

void CShortCutItem::GetShortCutFlag(TCHAR *szShortCut)
{
	if (!m_strShortCutName.IsEmpty())
		::lstrcpy(szShortCut, m_strShortCutName);
}

#define SHORTCUT_ITEM_PAD_HEIGHT   3
#define SHORTCUT_ITEM_LABEL_HEIGHT 18
void CShortCutItem::SetPos(RECT rc)
{
	CControlUI::SetPos(rc);
	int h = rc.bottom - rc.top;
	int yPos = h;
	RECT rcCtrl = {rc.left, rc.top, rc.right, rc.top};
	//BUTTON RECT 
	yPos -= SHORTCUT_ITEM_PAD_HEIGHT;
	if (m_pLbl->IsVisible())
	{
		yPos -= SHORTCUT_ITEM_PAD_HEIGHT;
		yPos -= SHORTCUT_ITEM_LABEL_HEIGHT;
	}
	rcCtrl.bottom = rc.top + yPos;
	m_pButton->SetPos(rcCtrl);
	if (m_pLbl->IsVisible())
	{
		rcCtrl.top = rc.top + yPos + SHORTCUT_ITEM_PAD_HEIGHT;
		rcCtrl.bottom = rcCtrl.top + SHORTCUT_ITEM_LABEL_HEIGHT;
		m_pLbl->SetPos(rcCtrl);
	}
}

void CShortCutItem::SetShowCaption(BOOL bShow)
{
	//
	if (m_pLbl)
		m_pLbl->SetVisible(bShow == TRUE);
}

#define SHORTCUT_ITEM_WIDTH  40
#define SHORTCUT_ITEM_HEIGHT 40

#define SHORTCUT_ITEM_COL_WIDTH 10
#define SHORTCUT_ITEM_ROW_HEIGHT 10

//CAutoShortCutVertList
CAutoShortCutVertList::CAutoShortCutVertList():
                       m_nButtonImageId(0),
					   m_nColWidth(10),
					   m_nRowHeight(10),
					   m_bShowLabel(TRUE)
{
	m_szItem.cx = SHORTCUT_ITEM_WIDTH;
	m_szItem.cy = SHORTCUT_ITEM_HEIGHT;
}

CAutoShortCutVertList::~CAutoShortCutVertList()
{
}

BOOL CAutoShortCutVertList::AddShortCut(const TCHAR *szLabelCaption, const TCHAR *szFilePath, int nImageId,
	          const TCHAR *szTip, TCHAR *szFlag)
{
	CShortCutItem *pItem = new CShortCutItem(szLabelCaption, szFilePath,  m_nButtonImageId, nImageId, szTip);
	pItem->SetHeight(m_szItem.cy);
	pItem->SetWidth(m_szItem.cx);
	pItem->GetShortCutFlag(szFlag);
	return Add(pItem); 
}

LPCTSTR CAutoShortCutVertList::GetClass() const
{
	return _T("AutoShortCut");
}

void CAutoShortCutVertList::SetPos(RECT rc)
{
	//   m_rcItem = rc;
	// Adjust for inset
	//   rc.left += m_rcInset.left;
	//   rc.top += m_rcInset.top;
	//   rc.right -= m_rcInset.right;
	//   rc.bottom -= m_rcInset.bottom;
	//TBD

	CControlUI::SetPos(rc);//container position
	rc = m_rcItem;
	//scrollbar position TBD
	int iScrollPos = 0;
	if (IsScrollBarVisible(UISB_VERT))
	{
		CRect rcScroll(rc);
		rcScroll.left = rcScroll.right - SB_WIDTH;
		if (m_pVScrollBar)
		{
			m_pVScrollBar->SetPos(rcScroll);
			rc.right -= SB_WIDTH;
			iScrollPos = GetScrollPos(UISB_VERT);
		}
	}
	if (m_szItem.cx == 0)
		m_szItem.cx = SHORTCUT_ITEM_WIDTH;
	if (m_nColWidth == 0)
		m_nColWidth = SHORTCUT_ITEM_COL_WIDTH;
	int nCol = (rc.right - rc.left) / (m_szItem.cx + m_nColWidth);
	nCol = max(1, nCol);

	// Determine the minimum size
 
	int iPosY = rc.top - iScrollPos;
	int cyNeeded = 0;
	for (int it2 = 0; it2 < m_ChildList.GetSize(); it2++)
	{
		CControlUI* pControl = m_ChildList[it2];
		if (!pControl->IsVisible()) 
			continue;
		int nColSeq = it2 % nCol;

		RECT rcCtrl = {0, iPosY, 0, iPosY + m_szItem.cy};
		rcCtrl.left = rc.left + nColSeq * (m_szItem.cx + m_nColWidth) + m_nColWidth;
		rcCtrl.right = rcCtrl.left + m_szItem.cx;
		pControl->SetPos(rcCtrl);
				
		nColSeq ++;
		nColSeq %= nCol;
		if ((nColSeq == 0) || (nCol == 1))
		{
			iPosY +=( m_szItem.cy + m_iPadding + m_nRowHeight);
			cyNeeded += (m_szItem.cy + m_iPadding + m_nRowHeight); 
		}
	}
	// Handle overflow with scrollbars
	SetScrollRange(UISB_VERT, 0, cyNeeded-1);
	SetScrollPage(UISB_VERT, rc.bottom - rc.top);
}

void CAutoShortCutVertList::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcsicmp(pstrName, _T("ButtonImage")) == 0)
	{
		m_nButtonImageId = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("ColWidth")) == 0)
	{
		m_nColWidth = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("RowHeight")) == 0)
	{
		m_nRowHeight = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("ButtonPad")) == 0)
	{
		RECT rcInset = {0};
		LPTSTR pstr = NULL; 
		m_szItem.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
        m_szItem.cy = _tcstol(pstr + 1, &pstr, 10);  
	} else if (_tcsicmp(pstrName, _T("ShowCaption")) == 0)
	{
		m_bShowLabel = (_tcsicmp(pstrValue, _T("true")) == 0);
	} else
		CVerticalLayoutUI::SetAttribute(pstrName, pstrValue);
}

#define DIVIDE_WIDTH  3  //分割条宽度
 
CDivideLayoutUI::CDivideLayoutUI():
	             m_bVert(TRUE)
{
}

CDivideLayoutUI::~CDivideLayoutUI()
{
}

 
LPCTSTR CDivideLayoutUI::GetClass() const
{
	return _T("DivideLayout");
}

void CDivideLayoutUI::SetPos(RECT rc)
{
	CControlUI::SetPos(rc);
}

void CDivideLayoutUI::Event(TEventUI &e)
{
	switch(e.Type)
	{ 
		case UIEVENT_BUTTONDOWN:
			OnEventButtonDown(e);
			break;
		case UIEVENT_BUTTONUP:
			OnEventButtonUp(e);
			break;
		case UIEVENT_MOUSEMOVE:
			OnEventMouseMove(e);
			break; 
		case UIEVENT_MOUSEENTER:
			OnEventMouseEnter(e);
			break;
		case UIEVENT_MOUSELEAVE:
			OnEventMouseLeave(e);
			break;
		case UIEVENT_DRAGEND:
			OnEventDragEnd(e);
			break;
		case UIEVENT_SETCURSOR:
			OnSetCursor(e);
			break;
		default: 
	        CControlUI::Event(e);
			break;
	}
}

void CDivideLayoutUI::OnSetCursor(TEventUI &e)
{
	if (::PtInRect(&m_rcItem, e.ptMouse))
	{
		SetDivideCursor();
	} else
		CControlUI::Event(e);
}

void CDivideLayoutUI::OnEventButtonDown(TEventUI &e)
{
	//
	m_ptDown = e.ptMouse;
}

void CDivideLayoutUI::OnEventButtonUp(TEventUI &e)
{
	if (GetParent())
	{
		IContainerUI *pParent = static_cast<IContainerUI *>(GetParent()->GetInterface(L"Container"));
		if (pParent)
		{
			int nDis = 0;
			if (m_bVert)
			{
				nDis = e.ptMouse.x - m_ptDown.x;
			} else
			{
				nDis = e.ptMouse.y - m_ptDown.y;
			}
			pParent->AdjustLayout(nDis, m_bVert, this);
		} //end if (pParent)
	}
}

UINT CDivideLayoutUI::GetControlFlags()
{
	return UIFLAG_SETCURSOR;
}

void CDivideLayoutUI::OnEventMouseEnter(TEventUI &e)
{
	SetDivideCursor();
}

//
void CDivideLayoutUI::SetDivideCursor()
{
	if (m_bVert)
		//
		::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE)));
	else
		::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENS)));
}

void CDivideLayoutUI::OnEventMouseLeave(TEventUI &e)
{
	::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
}

void CDivideLayoutUI::OnEventMouseMove(TEventUI &e)
{
	//

}

void CDivideLayoutUI::OnEventDragEnd(TEventUI &e)
{

}

void CDivideLayoutUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcsicmp(pstrName, L"Layout") == 0)
	{
		if (_tcsicmp(pstrValue, L"Vert") == 0)
			m_bVert = TRUE;
		else
			m_bVert = FALSE;
	}
}

SIZE CDivideLayoutUI::EstimateSize(SIZE szAvailable)
{
	SIZE sz = szAvailable;
	if (m_bVert)
		sz.cx = DIVIDE_WIDTH;
	else
		sz.cy = DIVIDE_WIDTH;
	return sz;
}

void CDivideLayoutUI::PaintMoveDivide(const POINT &pt)
{
 
}

void CDivideLayoutUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	
}

#pragma warning(default:4996)
