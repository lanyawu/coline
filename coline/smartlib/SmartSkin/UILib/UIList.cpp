#include "common.h"

#include <UILib/UIlist.h>
#include <UILib/UILabel.h>
#include <UILib/UIPanel.h>
#include <UILIb/UIScroll.h>
#include <CommonLib/SystemUtils.h>

/////////////////////////////////////////////////////////////////////////////////////
//
class CListSubItemUI
{
public:
	CListSubItemUI(): m_textFormat( DT_LEFT )//default center-left 
	{
	}

	void SetText(const CStdString& sText)
	{
		m_sText = sText;
	}
	CStdString GetText() const
	{
		return m_sText;
	}

	void DrawItem(HDC hDC, const RECT& rcItem, CPaintManagerUI* pManager)
	{
		CRect rc(rcItem);
		rc.left += 5;
		CBlueRenderEngineUI::DoPaintQuickText(hDC, pManager, rc, 
			m_sText, UICOLOR_EDIT_TEXT_NORMAL, UIFONT_NORMAL, 
			DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);
	}

private:
	CStdString m_sText;
	UINT m_textFormat;
};


/////////////////////////////////////////////////////////////////////////////////////
//
//
class CListElementUI : public CControlUI
{
public:
	CListElementUI(CListUI& owner): 
	      m_ownerList(owner),
		  m_bSelected(false),
		  m_pData(NULL),
		  m_itemState(0),
		  m_uTextStyle(DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS)
	{
	}
	~CListElementUI()
	{
		for (int i = 0; i < m_items.GetSize(); ++i)
		{
			delete static_cast<CListSubItemUI*>(m_items[i]);
		}
	}

	//CControlUI overridable
	LPCTSTR GetClass() const
	{
		return _T("ListElementUI");
	}

	SIZE EstimateSize(SIZE szAvailable)
	{
		return CSize( 0, 0 );
	}

	void DoPaint(HDC hDC, const RECT& rcPaint)
	{
	}

	UINT GetControlFlags() const
	{
		return UIFLAG_WANTRETURN;
	}

	//operation
	bool IsSelected() const
	{
		return m_bSelected;
	}

	bool Select(bool bSelect = true)
	{
		if (IsEnabled())
		{
			m_bSelected = bSelect;
			return true;
		}
		return false;
	}
	void SetTextStyle(UINT uStyle)
	{
		m_uTextStyle = uStyle;
	}

	//item state( HOT only )
	//TBD UISTATE_SELECTED,UISTATE_HOT等状态和IsSelected，IsEnabled等接口
	//冲突，内部使用不同变量区分
	UINT GetState() const
	{
		return m_itemState;
	}

	UINT SetState(UINT nState)
	{
		UINT nTmp = m_itemState;
		m_itemState = nState;
		return nTmp;
	}

	CListSubItemUI* GetSubItem(int iItem) const;
	int GetSubItemCount() const;
	BOOL SetSubItemCount(int iCount);
	int InsertSubItem(int iSubItem);
	int InsertSubItem(int iSubItem, const CStdString & );
	bool SetData( void* pData ) 
	{ 
		m_pData = pData; 
		return true; 
	}
	void* GetData() const
	{ 
		return m_pData; 
	}
	BOOL SetItemText( int iSubItem, const CStdString& );
	BOOL GetItemText( int iSubItem, CStdString& sText );
	BOOL DeleteSubItem( int iSubItem );
	void DrawSubItem( HDC hDC, int iSubItem, const RECT& rcSubItem );

	void DrawItem(HDC hDC, const RECT& rcItem, UINT uStyle);
	void DrawBkgnd(HDC hDC, const RECT& rcItem, UINT uStyle);
	void DrawText(HDC hDC, const RECT& rcItem, UINT uStyle);

protected:
	CListUI& m_ownerList;
	UINT m_itemState;
	UINT m_uTextStyle;
	bool m_bSelected;
	void* m_pData;

	CStdPtrArray m_items;
};


CListSubItemUI* CListElementUI::GetSubItem(int iItem) const
{
	return static_cast<CListSubItemUI*>(m_items.GetAt(iItem));
}

int CListElementUI::GetSubItemCount() const
{
	return m_items.GetSize();
}

BOOL CListElementUI::SetSubItemCount(int iCount)
{
	if ((iCount > 0) && (iCount <= UILIST_MAX_COLUMNS))
	{
		m_items.Resize( iCount );
		for (int i = 0; i < iCount; ++i)
		{
			CListSubItemUI* pItem = new CListSubItemUI();
			m_items.SetAt(i, pItem);
		}
		return TRUE;
	} 
	return FALSE;
}

int CListElementUI::InsertSubItem(int iSubItem)
{
	CStdString sText;
	return InsertSubItem(iSubItem, sText);
}

int CListElementUI::InsertSubItem(int iSubItem, const CStdString& sText)
{
	if (iSubItem < 0)
	{
		iSubItem = 0;
	}
	if (iSubItem > m_items.GetSize())
	{
		iSubItem = m_items.GetSize();
	}

	CListSubItemUI* pSubItem = new CListSubItemUI();
	pSubItem->SetText(sText);
	if (m_items.InsertAt(iSubItem, pSubItem))
		return iSubItem;
	else 
	{
		delete pSubItem;
		return -1;
	}
}

BOOL CListElementUI::SetItemText(int iSubItem, const CStdString& sText)
{
	if ((iSubItem >= 0) && iSubItem < m_items.GetSize())
	{ 
		CListSubItemUI* pSubItem = GetSubItem(iSubItem);
		ASSERT(pSubItem != NULL);
		pSubItem->SetText(sText);
		return TRUE;
	}
	return FALSE;
}

BOOL CListElementUI::GetItemText(int iSubItem, CStdString& sText)
{
	if( iSubItem >= 0 && iSubItem < m_items.GetSize() ){
		CListSubItemUI* pSubItem = GetSubItem( iSubItem );
		ASSERT( pSubItem != NULL );
		if( pSubItem != NULL ){
			sText = pSubItem->GetText();
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CListElementUI::DeleteSubItem( int iSubItem )
{
	return m_items.Remove( iSubItem );
}

void CListElementUI::DrawItem(HDC hDC, const RECT& rcItem, UINT uDrawStyle)
{
	DrawBkgnd( hDC, rcItem, uDrawStyle );
	DrawText( hDC, rcItem, uDrawStyle );
}

void CListElementUI::DrawBkgnd(HDC hDC, const RECT& rcItem, UINT uDrawStyle)
{
	UITYPE_COLOR iBackColor = UICOLOR__INVALID;
	//background color
	if( ( GetState() & UISTATE_HOT ) != 0 ) { 
		iBackColor = UICOLOR_CONTROL_BACKGROUND_HOVER;
	}
	if( IsSelected() ) { 
		iBackColor = UICOLOR_CONTROL_BACKGROUND_SELECTED;
	}
	if( !IsEnabled() ) {
		iBackColor = UICOLOR_CONTROL_BACKGROUND_DISABLED;
	}

	if (iBackColor != UICOLOR__INVALID)
	{
		CBlueRenderEngineUI::DoFillRect(hDC, NULL, rcItem, iBackColor, m_bTransparent);
	}
}

void CListElementUI::DrawText(HDC hDC, const RECT& rcItem, UINT uDrawStyle )
{
	// Paint text
	RECT rcText = rcItem;
	::InflateRect(&rcText, -4, 0);
	int nLinks = 0;
	CBlueRenderEngineUI::DoPaintQuickText( hDC, NULL, rcText, m_sText,
		UICOLOR_CONTROL_TEXT_NORMAL, UIFONT_NORMAL, DT_SINGLELINE | m_uTextStyle );
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

class CListHeaderItemUI
{
public:
	CListHeaderItemUI()	
		: m_cxWidth( 20 ),
		  m_nTextFmt( LUITF_LEFT )
	{
	}

	//operation
	void Draw( HDC hDC, const RECT& rc, CPaintManagerUI* pManager );
	void SetText( const CStdString& sText ){
		m_sText = sText;
	}
	void SetWidth(int cxWidth){
		m_cxWidth = cxWidth;
	}
	int GetWidth() const{ 
		return m_cxWidth;
	}
	void SetTextFmt( int nFmt ){
		m_nTextFmt = nFmt;
	}
	UINT GetTextFmt() const{
		return m_nTextFmt;
	}
	RECT GetThumbRect(RECT rc) const;

private:
	CStdString m_sText;
	int m_cxWidth;
	UINT m_nTextFmt;
};

void CListHeaderItemUI::Draw( HDC hDC, const RECT& rc, CPaintManagerUI* pManager )
{
	// Paint text (with some indent)
	RECT rcMessage = rc;
	rcMessage.left += 6;
	rcMessage.bottom -= 1;
	CBlueRenderEngineUI::DoPaintQuickText(hDC, pManager, rcMessage, 
		m_sText, UICOLOR_EDIT_TEXT_NORMAL, UIFONT_NORMAL, 
		DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS );
	// Draw gripper
	POINT ptTemp = { 0 };
	RECT rcThumb = GetThumbRect( rc );
	RECT rc1 = { rcThumb.left + 2, rcThumb.top + 4, rcThumb.left + 2, rcThumb.bottom - 1 };
	CBlueRenderEngineUI::DoPaintLine(hDC, pManager, rc1, UICOLOR_HEADER_SEPARATOR);
	RECT rc2 = { rcThumb.left + 3, rcThumb.top + 4, rcThumb.left + 3, rcThumb.bottom - 1 };
	CBlueRenderEngineUI::DoPaintLine(hDC, pManager, rc2, UICOLOR_STANDARD_WHITE);
}

RECT CListHeaderItemUI::GetThumbRect(RECT rc) const
{
	return CRect(rc.right - 4, rc.top, rc.right, rc.bottom - 3);
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

class CListHeaderUI : public CControlUI
{
public:
	CListHeaderUI( CListUI& list ) 
		: m_ownerList( list )
	{}
	~CListHeaderUI(){
		for( int i = 0; i < m_items.GetSize(); ++i ){
			delete static_cast<CListHeaderItemUI*>( m_items[i] );
		}
	}

	LPCTSTR GetClass() const{
		return _T("ListHeaderUI");
	}
	UINT GetControlFlags() const{
		return UIFLAG_SETCURSOR;
	}
	void SetPos(RECT rc){
		CControlUI::SetPos( rc );
	}
	SIZE EstimateSize(SIZE szAvailable){
		return CSize( 0, 0 );
	}
	void Event( TEventUI& e );
	void DoPaint(HDC hDC, const RECT& rcPaint);

	int GetCount() const{
		return m_items.GetSize();
	}
	CListHeaderItemUI* GetItem( int iItem ){
		if( iItem >= 0 && iItem < m_items.GetSize() ){
			return static_cast< CListHeaderItemUI* >( m_items[iItem] );
		}
		return NULL;
	}

	//operations
	int InsertItem( int iItem, const CStdString&, int iTextFmt = -1, int iWidth = -1 );
	BOOL DeleteItem( int iItem );
	int GetColumnWidth( int iColumn ) const;
	int GetTotalWidth() const;
	int PtInSizingRect( POINT pt, int iScrollPos );

private:
	CListUI& m_ownerList;
	CStdPtrArray m_items;
};

void CListHeaderUI::Event( TEventUI& e )
{
	switch( e.Type ){
	case UIEVENT_SETCURSOR:
		if( m_ownerList.PtInSizingRect( e.ptMouse ) >= 0 ){
			::SetCursor(::LoadCursor( NULL, IDC_SIZEWE ));
			return;
		}
		break;
	}
	CControlUI::Event( e );
}


void CListHeaderUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	CRect rcFill;
	if( !::IntersectRect( &rcFill, &rcPaint, &m_rcItem ) )
		return;

	// Draw background
	CRect rcLine( m_rcItem );
	rcLine.top = rcLine.bottom;
	rcLine.left += 1;
	rcLine.right -= 1;
	CBlueRenderEngineUI::DoPaintLine( hDC, rcLine, RGB(189,190,206) );
	rcLine.top -= 1;
	rcLine.bottom -= 1;
	CBlueRenderEngineUI::DoPaintLine( hDC, rcLine, RGB(209,210,222) );
	rcLine.top -= 1;
	rcLine.bottom -= 1;
	CBlueRenderEngineUI::DoPaintLine( hDC, rcLine, RGB(230,231,239) );
}

int CListHeaderUI::InsertItem( int iItem, const CStdString& sText, int iTextFmt, int iWidth )
{
	if( iItem < 0 ){
		iItem = 0;
	}
	else if( iItem > m_items.GetSize() ){
		iItem = m_items.GetSize();
	}

	CListHeaderItemUI* pItem = new CListHeaderItemUI;
	if( m_items.InsertAt( iItem, pItem ) ){
		pItem->SetText( sText );
		if( iTextFmt != -1 ){
			pItem->SetTextFmt( iTextFmt );
		}
		if( iWidth != -1 ){
			pItem->SetWidth( iWidth );
		}
		return iItem;
	}
	else{
		delete pItem;
		return -1;
	}
}

BOOL CListHeaderUI::DeleteItem( int iItem )
{
	return m_items.Remove( iItem );
}

int CListHeaderUI::GetColumnWidth( int iColumn ) const
{
	if( iColumn >= 0 && iColumn < m_items.GetSize() ){
		CListHeaderItemUI* pItem = static_cast<CListHeaderItemUI*>( m_items[iColumn] );
		return pItem->GetWidth();
	}
	return -1;
}

int CListHeaderUI::GetTotalWidth() const
{
	int iWidth = 0;
	for( int iItem = 0; iItem < m_items.GetSize(); ++iItem ){
		CListHeaderItemUI* pItem = static_cast<CListHeaderItemUI*>( m_items[iItem] );
		iWidth += pItem->GetWidth();
	}
	return iWidth;
}

int CListHeaderUI::PtInSizingRect( POINT pt, int iScrollPos )
{
	int iTotalColWidth = 0;
	for( int i = 0; i < GetCount(); ++i ){
		CListHeaderItemUI* pHeaderItem = static_cast<CListHeaderItemUI*>( GetItem(i) );
		ASSERT( pHeaderItem != NULL );
		int iColWidth = pHeaderItem->GetWidth();
		CRect rcColumn( m_rcItem );
		rcColumn.left = m_rcItem.left - iScrollPos + iTotalColWidth;
		rcColumn.right = rcColumn.left + iColWidth;
		iTotalColWidth += iColWidth;

		CRect rcThumb( pHeaderItem->GetThumbRect( rcColumn ) );
		if( ::PtInRect( &rcThumb, pt ) ){
			return i;
		}
	}
	return -1;
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

class CListWorkareaUI : public CVerticalLayoutUI
{
public:
	CListWorkareaUI( CListUI& list )
		: m_ownerList( list ),
		  m_iItemHeight( UILIST_ITEM_HEIGHT ),
		  m_iCurHot( -1 )
	{}

	//CControlUI overridable
	LPCTSTR GetClass() const{
		return _T("ListWorkareaUI");
	}
	void Event(TEventUI& e);
	void SetPos(RECT rc){
		CControlUI::SetPos( rc );
	}
	CControlUI* FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags){
		return CControlUI::FindControl( Proc, pData, uFlags );
	}

	//operation
	int InsertItem( int iItem );
	int InsertItem( int iItem, const CStdString& sLabel, int iImage = -1 );
	int InsertItem( int iItem, const CStdString& sLabel, const CStdString& sImageFile );
	BOOL DeleteItem(const int iItem );
	BOOL DeleteAllItems();
	int SetItemText(const int iItem, int iSubItem, const TCHAR *szDspText );
	BOOL GetItemText( int iItem, int iSubItem, CStdString& sText );
	int AppItem(const char *szDspText);

	int SetItemData(const int iItem, void* pData );
	void *GetItemData(const int iItem) const;
	BOOL InsertSubItems( int iSubIndex );
	BOOL DeleteSubItems( int iSubIndex );

	int HitTest( POINT pt ) const{
		if( ::PtInRect( &m_rcItem, pt ) ){
			int iOffsetIndex = ( pt.y - m_rcItem.top ) / m_iItemHeight;
			int iTopIndex = m_ownerList.GetTopIndex();
			if( iTopIndex + iOffsetIndex < GetCount() ){
				return ( iTopIndex + iOffsetIndex );
			}
		}
		return -1;
	}

private:
	CListUI& m_ownerList;
	int m_iItemHeight;
	int m_iCurHot;
};


void CListWorkareaUI::Event( TEventUI& e )
{
	switch( e.Type ){
	case UIEVENT_MOUSEMOVE:
		{
			int iHitIndex = HitTest( e.ptMouse );
			if( iHitIndex != m_iCurHot ){
				if( m_iCurHot != -1 ){
					CListElementUI* pEle = static_cast<CListElementUI*>( GetItem(m_iCurHot) );
					pEle->SetState( pEle->GetState() & ~UISTATE_HOT );
				}
				if( iHitIndex != -1 ){
					CListElementUI* pEle = static_cast<CListElementUI*>( GetItem(iHitIndex) );
					pEle->SetState( pEle->GetState() | UISTATE_HOT );
				}
				m_iCurHot = iHitIndex;
				Invalidate();
			}
		}
		break;
	case UIEVENT_BUTTONDOWN:
		{
			int iHitItem = HitTest(e.ptMouse);
			CListElementUI* pHitEle = static_cast<CListElementUI*>(GetItem(iHitItem));
			if (pHitEle != NULL && pHitEle->IsEnabled())
			{
				m_ownerList.SelectItem( iHitItem );
				m_pManager->SendNotify( &m_ownerList, _T("itemactivate"), iHitItem);				
				Invalidate();
			}
		}
		break;
	case UIEVENT_MOUSELEAVE:
		if (m_iCurHot != -1)
		{
			CListElementUI* pEle = static_cast<CListElementUI*>(GetItem(m_iCurHot));
			pEle->SetState(pEle->GetState() & ~UISTATE_HOT);
			Invalidate();
		}
		break;
	}
	CVerticalLayoutUI::Event( e );
}

int CListWorkareaUI::InsertItem( int iItem )
{
	return InsertItem(iItem, _T(""), -1);
}

int CListWorkareaUI::InsertItem(int iItem, const CStdString& sLabel, int iImage )
{
	//validate item index
	if (iItem > GetCount())
	{
		iItem = GetCount();
	}
	if (iItem < 0)
	{
		iItem = 0;
	}

	CListElementUI* pItem = new CListElementUI(m_ownerList);
	if (m_ChildList.InsertAt(iItem, pItem))
	{
		pItem->SetSubItemCount(m_ownerList.GetColumnCount());
		pItem->SetText(sLabel);
		return iItem;
	} else
	{
		delete pItem;
		return -1;
	}
}

int CListWorkareaUI::InsertItem(int iItem, const CStdString& sLabel, const CStdString& sImageFile)
{
	ASSERT( !_T("Not Support yet!") );
	return -1;
}

BOOL CListWorkareaUI::DeleteItem(const int iItem)
{
	if (m_ChildList.Remove(iItem))
	{
		if (iItem == m_iCurHot)
		{
			m_iCurHot = -1;
		}
		Invalidate();
		return true;
	}
	return false;
}

BOOL CListWorkareaUI::DeleteAllItems()
{
	CVerticalLayoutUI::RemoveAll();
	m_iCurHot = -1;
	return TRUE;
}

int CListWorkareaUI::SetItemText(const int iItem, int iSubItem, const TCHAR *sText)
{
	if ((iItem >= 0) && (iItem < m_ChildList.GetSize()))
	{
		CListElementUI* pListEleUI = dynamic_cast<CListElementUI*>(GetItem(iItem));
		ASSERT( pListEleUI != NULL );

		if (m_ownerList.GetStyle() == LUIS_REPORT)
		{	
			pListEleUI->InsertSubItem(iSubItem, sText);
		} else if (m_ownerList.GetStyle() == LUIS_LIST)
		{
			pListEleUI->SetText( sText );
		}
		return iItem;
	}
	return -1;
}

BOOL CListWorkareaUI::GetItemText( int iItem, int iSubItem, CStdString &sText )
{
	if (iItem >= 0 && iItem < m_ChildList.GetSize())
	{
		CListElementUI* pListEleUI = dynamic_cast<CListElementUI*>( GetItem( iItem ) );
		ASSERT( pListEleUI != NULL );

		if (m_ownerList.GetStyle() == LUIS_REPORT)
		{	
			return pListEleUI->GetItemText(iSubItem, sText);
		} else if (m_ownerList.GetStyle() == LUIS_LIST)
		{
			sText = pListEleUI->GetText();
			return TRUE;
		}
	}
	return FALSE;
}

int CListWorkareaUI::SetItemData(const int iItem, void* pData )
{
	if( iItem >= 0 && iItem < m_ChildList.GetSize() ){
		CListElementUI* pListEleUI = dynamic_cast<CListElementUI*>( GetItem( iItem ) );
		pListEleUI->SetData(pData );
		return iItem;
	}
	return -1;
}

void *CListWorkareaUI::GetItemData(const int iItem) const
{
	if ((iItem >= 0) && (iItem < m_ChildList.GetSize()))
	{
		CListElementUI* pListEleUI = dynamic_cast<CListElementUI*>(GetItem(iItem));
		return pListEleUI->GetData();
	}
	return NULL;
}

int  CListWorkareaUI::AppItem(const char *szDspText)
{
	 //end while (p1)
	CListElementUI* pItem = new CListElementUI(m_ownerList);
	int nIdx = GetCount();
	if (m_ChildList.InsertAt(nIdx, pItem))
	{
		int nSize = ::strlen(szDspText);
		TCHAR *szwTmp = new TCHAR[nSize + 1];
		memset(szwTmp, 0, sizeof(TCHAR) * (nSize + 1));
		CStringConversion::StringToWideChar(szDspText, szwTmp, nSize);
		pItem->InsertSubItem(0, szwTmp);
		delete []szwTmp; 
		return nIdx;
	} else
	{
		delete pItem;
		return -1;
	}
}

BOOL CListWorkareaUI::InsertSubItems( int iSubIndex )
{
	for( int i = 0; i < m_ChildList.GetSize(); ++i )
	{
		CListElementUI* pListItem = dynamic_cast<CListElementUI*>( GetItem( i ) );
		ASSERT( pListItem != NULL );
		pListItem->InsertSubItem( iSubIndex );
	}
	return TRUE;
}

BOOL CListWorkareaUI::DeleteSubItems( int iSubIndex )
{
	for( int i = 0; i < m_ChildList.GetSize(); ++i )
	{
		CListElementUI* pListItem = dynamic_cast<CListElementUI*>( m_ChildList[i] );
		ASSERT( pListItem != NULL );
		pListItem->DeleteSubItem( iSubIndex );
	}
	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////////////
//
//
CListUI::CListUI(): 
         m_iCurSel(-1), 
	     m_uiStyle(LUIS_REPORT),
	     m_iItemHeight(UILIST_ITEM_HEIGHT),
	     m_pHeader(NULL),
	     m_bDrawGrid(TRUE),
	     m_pList(NULL),
		 m_bShowHeader(TRUE)
{
}

void CListUI::Init()
{
	CVerticalLayoutUI::Init();
	m_pList = new CListWorkareaUI( *this );
	//only report style has header
	if (GetStyle() == LUIS_REPORT)
	{
		m_pHeader = new CListHeaderUI( *this );
		CVerticalLayoutUI::Add(m_pHeader);
		if (!m_bShowHeader)
			m_pHeader->SetVisible(false);
	}
	//workarea
	CVerticalLayoutUI::Add(m_pList);

	//vertical scrollbar
	EnableScrollBar( UISB_VERT, true );
	//horizontal scrollbar
	if (GetStyle() == LUIS_REPORT)
	{
		EnableScrollBar( UISB_HORZ, true );
	}
	if (!m_strColumns.empty())
		SetColumns(m_strColumns.c_str());
}

LPCTSTR CListUI::GetClass() const
{
   return _T("ListUI");
}

UINT CListUI::GetControlFlags() const
{
   return UIFLAG_TABSTOP;
}

bool CListUI::Add(CControlUI* pControl, const int nIdx)
{
	ASSERT(!"Not supported. You should use list operations instead of this interface!");
	return false;
}

bool CListUI::Remove(CControlUI* pControl)
{
   ASSERT(!"Not supported yet");
   return false; 
}

void CListUI::RemoveAll()
{
	SelectItem( -1 );
	m_pList->DeleteAllItems();
	RECT rcWorkArea = m_pList->GetPos();
	ProcessVScrollBar( rcWorkArea.bottom - rcWorkArea.top );
}

CControlUI *CListUI::GetItem(const int iIndex) const
{
   return m_pList->GetItem(iIndex);
}

int CListUI::GetCount() const
{
   return m_pList->GetCount();
}

void CListUI::ProcessVScrollBar( int iHeight )
{
	//adjust scroll range to be sure the last list item can be displayed correctly
	SetScrollRange( UISB_VERT, 0, GetCount() - 1 );
	SetScrollPage( UISB_VERT, iHeight / m_iItemHeight );
}

void CListUI::ProcessHScrollBar( int iWidth )
{
	if( m_pHeader )
		SetScrollRange( UISB_HORZ, 0, m_pHeader->GetTotalWidth() - 1 );
	else
		SetScrollRange( UISB_HORZ, 0, iWidth - 1 );//最长条目的长度决定水平滚动条
	SetScrollPage( UISB_HORZ, iWidth );
}

void CListUI::SetPos(RECT rc)
{
	CControlUI::SetPos( rc );
	//border is in consideration
	m_rcClient = m_rcItem;
	::InflateRect( &m_rcClient, -1, -1 );

	CRect rcVScroll( m_rcClient );//vert scroll
	rcVScroll.left = rcVScroll.right - SB_WIDTH;

	CRect rcHScroll( m_rcClient );//horizontall scroll
	rcHScroll.top = rcHScroll.bottom - SB_WIDTH;

	CRect rcHeader( m_rcClient );//header
	CRect rcWorkArea( m_rcClient );//workarea
	if (m_uiStyle == LUIS_REPORT)
	{
		rcHeader.bottom = rcHeader.top + UILIST_HEADER_HEIGHT;
		rcWorkArea.top = rcHeader.bottom + m_iItemHeight - UILIST_HEADER_HEIGHT;
	}

	//after that we will know if the vertical scrollbar is visible
	if( IsScrollBarVisible( UISB_HORZ ) )
		ProcessVScrollBar( rcWorkArea.bottom - rcWorkArea.top - SB_WIDTH );
	else
		ProcessVScrollBar( rcWorkArea.bottom - rcWorkArea.top );

	//also, after that we will know if the horizontal scrollbar is visible
	if( IsScrollBarVisible( UISB_VERT ) )
		ProcessHScrollBar( rcWorkArea.right - rcWorkArea.left - SB_WIDTH );
	else
		ProcessHScrollBar( rcWorkArea.right - rcWorkArea.left );

	if( IsScrollBarVisible( UISB_VERT ) ){//vert
		rcWorkArea.right -= SB_WIDTH;
		rcHeader.right = rcWorkArea.right;
		rcHScroll.right = rcWorkArea.right;
	}
	if( IsScrollBarVisible( UISB_HORZ ) ){//horizon
		rcWorkArea.bottom -= SB_WIDTH;
		rcVScroll.bottom -= SB_WIDTH;
	}
	m_pVScrollBar->SetPos( rcVScroll );
	if( m_pHScrollBar )
		m_pHScrollBar->SetPos( rcHScroll );
	if( m_pHeader ){
		m_pHeader->SetPos( rcHeader );
	}
	m_pList->SetPos( rcWorkArea );
}

//
void CListUI::KeyDownEvent(WORD wKey)
{
	switch(wKey)
	{
	case VK_UP:
		if (m_iCurSel > 0)
		{
			SelectItem(m_iCurSel - 1);
		}
		return;
	case VK_DOWN:
		if (m_iCurSel + 1 <= GetCount() - 1)
		{
			SelectItem(m_iCurSel + 1);
		}
		return;
	case VK_PRIOR:
		if (m_iCurSel > 0)
		{
			int iPage = GetScrollPage( UISB_VERT );
			int iNewSel = ( m_iCurSel - iPage ) >= 0 ? (m_iCurSel - iPage) : 0;
			SelectItem( iNewSel );
		}
		return;
	case VK_NEXT:
		if (m_iCurSel + GetScrollPage( UISB_VERT ) > GetCount() - 1)
		{
			SelectItem(GetCount() - 1);
		} else
		{
			SelectItem(m_iCurSel + GetScrollPage(UISB_VERT));
		}
		return;
	case VK_HOME:
		SelectItem(0);
		return;
	case VK_END:
		if (GetCount() > 0)
		{
			SelectItem(GetCount() - 1);
		}
		return;
	case VK_RETURN:
		if (GetCurSel() != -1)
		{
			m_pManager->SendNotify( this, _T("itemactivate"), GetCurSel() );
		}
		return;
	}
}

void CListUI::Event(TEventUI& event)
{
	static POINT ptStartResizing = { 0, 0 };
	static bool bResizing = false;
	static int iHeaderIndex = -1;

	switch( event.Type )
	{
	case UIEVENT_KEYDOWN:
		KeyDownEvent(event.chKey);
		break;
	case UIEVENT_SCROLLWHEEL:
		{
			int iPos = GetScrollPos( UISB_VERT );
			switch( LOWORD(event.wParam) ) {
			case SB_LINEUP:
				SetScrollPos( UISB_VERT, iPos - 3 );
				if( iPos != GetScrollPos( UISB_VERT ) ){
					Invalidate();
				}
				return;
			case SB_LINEDOWN:
				SetScrollPos( UISB_VERT, iPos + 3 );
				if( iPos != GetScrollPos( UISB_VERT ) ){
					Invalidate();
				}
				return;
			}
		}
		break;
	case UIEVENT_BUTTONDOWN:
		if( m_pHeader ){
			CRect rcHeader( m_pHeader->GetPos() );
			if( ::PtInRect( &rcHeader, event.ptMouse ) ){
				iHeaderIndex = m_pHeader->PtInSizingRect( event.ptMouse, 
					IsScrollBarVisible( UISB_HORZ ) ? GetScrollPos( UISB_HORZ ) : 0 );
				if( iHeaderIndex >= 0 ){
					ptStartResizing = event.ptMouse;
					bResizing = true;
				}
			}
		}
		break;
	case UIEVENT_MOUSEMOVE:
		if( bResizing == true ){
			CListHeaderItemUI* pHeaderItem = 
					static_cast<CListHeaderItemUI*>( m_pHeader->GetItem(iHeaderIndex) );
			int iColWidth = pHeaderItem->GetWidth();
			int iDelta = event.ptMouse.x - ptStartResizing.x;
			iColWidth += iDelta;
			if( iDelta != 0 && iColWidth >= 40 ){
				pHeaderItem->SetWidth( iColWidth );
				ptStartResizing.x = event.ptMouse.x;
				SetScrollRange( UISB_HORZ, 0, m_pHeader->GetTotalWidth() - 1 );
				Invalidate();
			}
		}
		break;
	case UIEVENT_DBLCLICK:
		m_pManager->SendNotify( this, _T("itemdblclk"));
		break;
	case UIEVENT_BUTTONUP:
		if( bResizing == true ){
			bResizing = false;
		}
		break;
	}
	CControlUI::Event(event);
}

int CListUI::GetCurSel() const
{
   return m_iCurSel;
}

bool CListUI::SelectItem(const int iIndex )
{
	if (iIndex == m_iCurSel)
	{
		return true;
	}
	//validate
	if( iIndex < 0 || iIndex >= GetCount() )
	{
		m_iCurSel = -1;
		return false;
	}

	// We should first unselect the currently selected item
	CListElementUI* pItem = static_cast<CListElementUI*>( GetItem(m_iCurSel) );
	if( pItem != NULL ) {
		m_iCurSel = -1;
		pItem->Select( false );
		Invalidate();
		m_pManager->SendNotify( this, _T("itemselect") );
	}
	// Now figure out if the control can be selected
	// TODO: Redo old selected if failure
	pItem = dynamic_cast<CListElementUI*>( GetItem(iIndex) );
	if( pItem != NULL ){
		if( pItem->Select(true) ) {
			m_iCurSel = iIndex;
			if( m_iCurSel < GetTopIndex() ){
				SetScrollPos( UISB_VERT, m_iCurSel );
			}
			else if( m_iCurSel > GetTopIndex() + GetCountPerPage() - 1 ){
				SetScrollPos( UISB_VERT, m_iCurSel - GetCountPerPage() + 1 );
			}
			m_pManager->SendNotify( this, _T("itemselect") );
			Invalidate();
			return true;
		}
	}
	return false;
}

void CListUI::Notify(TNotifyUI& msg)
{
	int iPos = GetScrollPos(UISB_VERT);
	int iHPos = GetScrollPos(UISB_HORZ);
	if (msg.sType == _T("lineup"))
	{
		SetScrollPos(UISB_VERT, iPos - 1);
		if (iPos != GetScrollPos(UISB_VERT))
			Invalidate();
	} else if (msg.sType == _T("linedown"))
	{
		SetScrollPos(UISB_VERT, iPos + 1);
		if (iPos != GetScrollPos(UISB_VERT))
			Invalidate();
	} else if (msg.sType == _T("pageup"))
	{
		SetScrollPos( UISB_VERT, iPos - m_pVScrollBar->GetScrollPage() );
		if (iPos != GetScrollPos(UISB_VERT))
			Invalidate();
	} else if (msg.sType == _T("pagedown"))
	{
		SetScrollPos(UISB_VERT, iPos + m_pVScrollBar->GetScrollPage());
		if (iPos != GetScrollPos(UISB_VERT))
			Invalidate();
	} else if (msg.sType == _T("thumbtrack"))
	{
		// msg.sType = _T("thumbtrack"), 我犯的错误
		SetScrollPos(UISB_VERT, (int)msg.wParam);
		if (iPos != GetScrollPos(UISB_VERT))
			Invalidate();
	} else if (msg.sType == _T("lineleft"))
	{
		SetScrollPos( UISB_HORZ, iHPos - 14 );
		if (iHPos != GetScrollPos(UISB_HORZ)) 
			Invalidate();
	} else if (msg.sType == _T("lineright"))
	{
		SetScrollPos( UISB_HORZ, iHPos + 14 );
		if (iHPos != GetScrollPos(UISB_HORZ)) 
			Invalidate();
	} else if (msg.sType == _T("pageleft"))
	{
		SetScrollPos(UISB_HORZ, iHPos - GetScrollPage(UISB_HORZ));
		if (iHPos != GetScrollPos(UISB_HORZ))
			Invalidate();
	} else if (msg.sType == _T("pageright"))
	{
		SetScrollPos(UISB_HORZ, iHPos + GetScrollPage(UISB_HORZ));
		if (iHPos != GetScrollPos(UISB_HORZ)) 
			Invalidate();
	} else if (msg.sType == _T("hthumbtrack"))
	{
		SetScrollPos( UISB_HORZ, (int)msg.wParam);
		if (iHPos != GetScrollPos(UISB_HORZ))
			Invalidate();
	} else if (msg.sType == _T("visibilitychanged"))
	{
		CRect rcHeader(m_rcClient);
		CRect rcWorkArea(m_rcClient);
		CRect rcVScroll(m_rcClient);
		CRect rcHScroll(m_rcClient);
		if (msg.pSender == m_pVScrollBar)
		{
			//vertical scroll bar
			if (msg.wParam == 1)
			{
				//vertical scroll bar appear
				//adjust self
				rcVScroll.left = rcVScroll.right - SB_WIDTH;
				if( IsScrollBarVisible(UISB_HORZ))
				{
					rcVScroll.bottom -= SB_WIDTH;
				}
				m_pVScrollBar->SetPos(rcVScroll);
				//adjust header position
				rcHeader.right -= SB_WIDTH;
				rcHeader.bottom = rcHeader.top + UILIST_HEADER_HEIGHT;
				if (m_pHeader) 
					m_pHeader->SetPos(rcHeader);
				//adjust workarea position
				if (m_pHeader)
					rcWorkArea.top = rcHeader.bottom + m_iItemHeight - UILIST_HEADER_HEIGHT;
				rcWorkArea.right -= SB_WIDTH;
				if (IsScrollBarVisible( UISB_HORZ) )
				{
					rcWorkArea.bottom -= SB_WIDTH;
				}
				m_pList->SetPos( rcWorkArea );
				//adjust horizontal scrollbar
				rcHScroll.top = rcHScroll.bottom - SB_WIDTH;
				rcHScroll.right = rcWorkArea.right;
				if( m_pHScrollBar )
					m_pHScrollBar->SetPos( rcHScroll );//sequence!!!
				ProcessHScrollBar(rcWorkArea.right - rcWorkArea.left);
			} else
			{
				//desappear
				//self
				//header
				rcHeader.bottom = rcHeader.top + UILIST_HEADER_HEIGHT;
				if( m_pHeader ){
					m_pHeader->SetPos( rcHeader );
				}
				//workarea
				if (m_pHeader)
					rcWorkArea.top = rcHeader.bottom + m_iItemHeight - UILIST_HEADER_HEIGHT;
				if (IsScrollBarVisible(UISB_HORZ))
				{
					rcWorkArea.bottom -= SB_WIDTH;
				}
				m_pList->SetPos( rcWorkArea );
				//horizontal scrollbar
				rcHScroll.right = rcWorkArea.right;
				rcHScroll.top = rcHScroll.bottom - SB_WIDTH;
				if( m_pHScrollBar )
					m_pHScrollBar->SetPos( rcHScroll );
				ProcessHScrollBar( rcWorkArea.right - rcWorkArea.left );
			}
		} else if (msg.pSender == m_pHScrollBar)
		{
			if (msg.wParam == 1)
			{
				//header, keep unchanged
				rcHeader.bottom = rcHeader.top + UILIST_HEADER_HEIGHT;
				//seft
				rcHScroll.top = rcHScroll.bottom - SB_WIDTH;
				if( m_pVScrollBar->IsVisible() ){
					rcHScroll.right -= SB_WIDTH; 
				}
				if( m_pHScrollBar )
					m_pHScrollBar->SetPos( rcHScroll );
				//workarea
				if( m_pHeader )
					rcWorkArea.top = rcHeader.bottom + m_iItemHeight - UILIST_HEADER_HEIGHT;
				if (m_pVScrollBar->IsVisible())
				{
					rcWorkArea.right -= SB_WIDTH;
				}
				rcWorkArea.bottom -= SB_WIDTH;
				m_pList->SetPos( rcWorkArea );
				//vertical scrollbar
				rcVScroll.left = rcVScroll.right - SB_WIDTH;
				rcVScroll.bottom = rcWorkArea.bottom;
				m_pVScrollBar->SetPos( rcVScroll );
				ProcessVScrollBar(rcWorkArea.bottom - rcWorkArea.top);
			} else
			{
				//header, keep unchanged
				rcHeader.bottom = rcHeader.top + UILIST_HEADER_HEIGHT;
				//self
				//workarea
				if( m_pHeader )
					rcWorkArea.top = rcHeader.bottom + m_iItemHeight - UILIST_HEADER_HEIGHT;
				if( m_pVScrollBar->IsVisible() ){
					rcWorkArea.right -= SB_WIDTH;
				}
				m_pList->SetPos( rcWorkArea );
				//vertical scrollbar
				rcVScroll.left = rcVScroll.right - SB_WIDTH;
				rcVScroll.bottom = rcWorkArea.bottom;
				m_pVScrollBar->SetPos( rcVScroll );
				ProcessVScrollBar( rcWorkArea.bottom - rcWorkArea.top );
			}
		}
	} else
	{
		CVerticalLayoutUI::Notify( msg );
	}
}

void CListUI::EnableGridLine(BOOL bDrawGrid)
{
	if (m_bDrawGrid != bDrawGrid)
	{
		m_bDrawGrid = bDrawGrid;
		Invalidate();
	}
}


void CListUI::GetRange( const RECT& rc, int& iFirst, int& iLast )
{
	//TBD 边界值
	POINT ptTopLeft = { rc.left, rc.top };
	iFirst = HitTest( ptTopLeft );
	if (iFirst == -1)
	{
		iFirst = GetScrollPos( UISB_VERT );
	}
	POINT ptBotRight = { rc.right, rc.bottom };
	iLast = HitTest( ptBotRight );
	if (iLast == -1)
	{
		iLast = GetTopIndex() + GetCountPerPage() + 1;
		if (iLast >= GetCount())
		{
			iLast = GetCount() - 1;
		}
	}
}


void CListUI::DrawReportList(HDC hDC, const RECT& rcPaint)
{
	ASSERT( m_pHeader != NULL );

	CRenderClip clip;
	RECT rcTmp = m_pHeader->GetPos();
	rcTmp.bottom = m_pList->GetPos().bottom;
	CBlueRenderEngineUI::GenerateClip(hDC, rcTmp, clip);

	//draw header ui and header items
	m_pHeader->DoPaint(hDC, rcPaint);

	//draw according to scroll position
	int iScrollPos = GetScrollPos(UISB_VERT);
	int iHScrollPos =  GetScrollPos(UISB_HORZ);

	int iFirst, iLast;
	GetRange( rcPaint, iFirst, iLast );

	CRect rcHeader(m_pHeader->GetPos());
	//we draw one column by one column
	int iTotalColWidth = 0;
	for (int iCol = 0; iCol < m_pHeader->GetCount(); ++iCol)
	{
		CListHeaderItemUI* pHeaderItem = static_cast<CListHeaderItemUI*>(m_pHeader->GetItem(iCol));
		ASSERT( pHeaderItem != NULL );
		int iColWidth = pHeaderItem->GetWidth();
		CRect rcColumn( rcHeader );
		rcColumn.left = rcHeader.left - iHScrollPos + iTotalColWidth;
		rcColumn.right = rcColumn.left + iColWidth;
		iTotalColWidth += iColWidth;
		
		//the current column is out of the list area
		if (rcColumn.right < rcHeader.left)
		{
			continue;
		}

		pHeaderItem->Draw(hDC, rcColumn, m_pManager);
	
		//draw columns
		int iStart = iScrollPos;
		CRect rcListUI = m_pList->GetPos();	
		CRect rcSubItem( rcColumn );
		int iDrawItem = 0;
		for (int iItem = iFirst; iItem <= iLast; ++iItem)
		{
			//list item
			CListElementUI* pListItem = static_cast<CListElementUI*>(GetItem(iItem));
			rcSubItem.top = rcListUI.top + iDrawItem * m_iItemHeight;
			rcSubItem.bottom = rcSubItem.top + m_iItemHeight;
			//list sub item
			CListSubItemUI* pSubItem = static_cast<CListSubItemUI*>(pListItem->GetSubItem(iCol)); 
			pListItem->DrawBkgnd( hDC, rcSubItem, 0 );
			if (pSubItem)
				pSubItem->DrawItem( hDC, rcSubItem, m_pManager );
			++iDrawItem;
		}//for( int j
	}//for( int i
	//draw grid lines
	if ((m_bDrawGrid) && ( GetCount() > 0))
	{
		//horizontal lines
		CRect rcLine( m_pList->GetPos() );
		rcLine.bottom = rcLine.top;
		while (rcLine.top < m_rcClient.bottom)
		{
			CBlueRenderEngineUI::DoPaintLine(hDC, rcLine, m_clrBorder);
			rcLine.top += m_iItemHeight;
			rcLine.bottom = rcLine.top;
		}
		//vertical lines
		CRect rcLine2(m_pList->GetPos());
		int iColsWidth = 0;
		for (int iCol = 0; iCol < GetColumnCount(); ++iCol)
		{
			rcLine2.left = rcLine2.right = iColsWidth + GetColumnWidth(iCol) + m_rcClient.left - iHScrollPos;
			iColsWidth += GetColumnWidth(iCol);
			CBlueRenderEngineUI::DoPaintLine(hDC, rcLine2, m_clrBorder);
		}
	}
}

void CListUI::DrawNormalList(HDC hDC, const RECT& rcPaint)
{
	CRenderClip clip;
	CBlueRenderEngineUI::GenerateClip(hDC, m_pList->GetPos(), clip);

	int iFirst, iLast;
	GetRange(rcPaint, iFirst, iLast);
	int iScrollPos = GetScrollPos(UISB_VERT);

	for (int iItem = iFirst; iItem <= iLast; ++iItem)
	{
		CListElementUI* pItem = static_cast<CListElementUI*>(m_pList->GetItem(iItem));
		CRect rcItem(m_pList->GetPos());
		rcItem.top += (iItem - iScrollPos) * m_iItemHeight;
		rcItem.bottom = rcItem.top + m_iItemHeight;
		pItem->DrawItem(hDC, rcItem, 0);
	}
}

void CListUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	RECT rcTemp = { 0 };
	if (!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem))
	{
		return;
	}
	
	CRenderClip clip;
	CBlueRenderEngineUI::GenerateClip(hDC, m_rcItem, clip);

	//draw border
	if (HasBorder())
	{
		CBlueRenderEngineUI::DoPaintRectangle(hDC, m_rcItem, m_clrBorder, m_clrBkgnd);
	}

	//draw the header control and client area
	if (::IntersectRect(&rcTemp, &rcPaint, &m_pList->GetPos()))
	{
		switch(m_uiStyle)
		{
		case LUIS_REPORT://report style
			DrawReportList(hDC, rcPaint);
			break;
		case LUIS_LIST://normal list
			DrawNormalList(hDC, rcPaint);
			break;
		default:
			break;
		}
	}

	//draw scroll bars
	if( m_pVScrollBar )
		m_pVScrollBar->DoPaint(hDC, rcPaint);
	if( m_pHScrollBar )
		m_pHScrollBar->DoPaint(hDC, rcPaint);
	//
	if (IsScrollBarVisible(UISB_VERT) && IsScrollBarVisible(UISB_HORZ))
	{
		CRect rcFoot( m_rcClient.right - SB_WIDTH, m_rcClient.bottom - SB_WIDTH, 
			m_rcClient.right, m_rcClient.bottom );
		CBlueRenderEngineUI::DoFillRect(hDC, m_pManager, rcFoot, m_clrBorder, m_bTransparent);
	}
}

void CListUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcsicmp(pstrName, _T("style")) == 0)
	{ 
		if (_tcsicmp(pstrValue, _T("report")) == 0)
		{
			SetStyle(LUIS_REPORT);
		} else
		{
			SetStyle(LUIS_LIST);//default style list
		}
	} else if (_tcsicmp(pstrName, _T("showheader")) == 0)
	{
		m_bShowHeader = (_tcsicmp(pstrName, _T("true")) == 0);
		if (m_pHeader)
			m_pHeader->SetVisible(m_bShowHeader == TRUE);
	} else if (_tcsicmp(pstrName, _T("coloumns")) == 0)
	{
		int nSize = ::lstrlen(pstrValue) * 2;
		char *szTmp = new char[nSize + 1];
		CStringConversion::WideCharToString(pstrValue, szTmp, nSize);
		SetColumns(szTmp);
		delete []szTmp;
	} else
	{
		CVerticalLayoutUI::SetAttribute(pstrName, pstrValue);
	}
}

void CListUI::SetColumns(const char *szColumns)
{
	if (m_pHeader)
	{
		const char *p1 = szColumns;
		int nIdx = 0;
		while (p1)
		{
			char szTmp[128] = {0};
			if (!CSystemUtils::GetStringBySep(p1, szTmp, ';', nIdx))
				break;
			//去除空格及回车
			int nSize = ::strlen(szTmp);
			while (nSize > 0)
			{
				if ((szTmp[nSize - 1] == '\n')
					|| (szTmp[nSize - 1] == ' ')
					|| (szTmp[nSize - 1] == 0xd))
				{
					szTmp[nSize - 1] = '\0';
					nSize --;
				} else
					break;
			} //end while (TRUE)
			nSize = ::strlen(szTmp);
			while (nSize > 0)
			{
				if (szTmp[nSize - 1] == ',')
					break;
				nSize --;
			}
			int nWidth = -1;
			if (nSize > 0)
			{
				char szWidth[32] = {0};
				::strncpy(szWidth, szTmp + nSize, 31);
				nWidth = ::atoi(szTmp + nSize);
				szTmp[nSize - 1] = '\0';
			}
			TCHAR *szwTmp = new TCHAR[nSize + 1];
			memset(szwTmp, 0, sizeof(TCHAR) * (nSize + 1));
			CStringConversion::StringToWideChar(szTmp, szwTmp, nSize);
			m_pHeader->InsertItem(nIdx, szwTmp, LUITF_CENTER, nWidth);
			nIdx ++;
		} //end while (p1)
	}  else
		m_strColumns = szColumns;//end if (m_pHeader)
}

BOOL CListUI::SetStyle( UINT nStyle )
{
	if ((nStyle > LUIS_FIRST_) && (nStyle < LUIS_LAST_ && nStyle != m_uiStyle))
	{
		m_uiStyle = nStyle;
		Invalidate();
		return TRUE;
	}
	return FALSE;
}

//columns
int CListUI::InsertColumn( int iNewColIndex, const CStdString& sText, int iTextFmt, int iWidth )
{
	if (m_uiStyle == LUIS_REPORT && m_pHeader != NULL)
	{
		int iNewCol = m_pHeader->InsertItem(iNewColIndex, sText, iTextFmt, iWidth);
		if (iNewCol != -1)
		{
			//insert column in workarea
			m_pList->InsertSubItems(iNewCol);
			//change horizontal scrollrange
			SetScrollRange(UISB_HORZ, 0, m_pHeader->GetTotalWidth());
			return iNewCol;
		}
	}
	return -1;
}

BOOL CListUI::DeleteColumn( int iColumn )
{
	if (m_uiStyle == LUIS_REPORT && m_pHeader != NULL)
	{
		if (m_pHeader->DeleteItem(iColumn))
		{
			m_pList->DeleteSubItems(iColumn);
			return true;
		}
	}
	return false;
}

int CListUI::GetColumnCount() const
{
	if (m_uiStyle == LUIS_REPORT && m_pHeader != NULL)
		return m_pHeader->GetCount();
	return 0;
}

int CListUI::GetColumnWidth( int iColumn ) const
{
	if (m_uiStyle == LUIS_REPORT && m_pHeader != NULL)
		return m_pHeader->GetColumnWidth(iColumn);
	return 0;
}

int CListUI::GetHeaderWidth() const
{
	if (m_uiStyle == LUIS_REPORT && m_pHeader != NULL)
		return m_pHeader->GetTotalWidth();
	return 0;
}

int CListUI::PtInSizingRect(POINT pt)
{
	return m_pHeader->PtInSizingRect(pt, 
		IsScrollBarVisible(UISB_HORZ) ? GetScrollPos(UISB_HORZ) : 0);
}

//items
int CListUI::InsertItem(int iItem)
{
	ASSERT(m_pList != NULL );
	return InsertItem(iItem, _T(""), -1);
}

int CListUI::InsertItem(int iItem, const CStdString& sLabel, int iImage)
{
	ASSERT( m_pList != NULL );
	int iNew = m_pList->InsertItem( iItem, sLabel, iImage );
	if (iNew != -1)
	{
		RECT rcWorkArea = m_pList->GetPos();
		ProcessVScrollBar(rcWorkArea.bottom - rcWorkArea.top);
	}
	return iNew;
}

int CListUI::AppItem(const char *szDspText)
{
	ASSERT( m_pList != NULL );
	int iNew = m_pList->AppItem(szDspText);
	if (iNew != -1)
	{
		RECT rcWorkArea = m_pList->GetPos();
		ProcessVScrollBar(rcWorkArea.bottom - rcWorkArea.top);
	}
	return iNew;
}

int CListUI::InsertItem(int iItem, const CStdString& sLabel, const CStdString& sImageFile)
{
	return -1;
}

BOOL CListUI::DeleteItem(const int iItem)
{
	if (m_iCurSel == iItem)
	{
		SelectItem(-1);
	}

	if (m_pList->DeleteItem(iItem))
	{
		SetScrollRange(UISB_VERT, 0, GetCount() - 1);
		Invalidate();
		return TRUE;
	}
	return FALSE;
}

BOOL CListUI::DeleteAllItems()
{
	RemoveAll();
	SetScrollRange(UISB_VERT, 0, 0);
	return TRUE;
}

int CListUI::SetItemText(const int iItem, int iSubItem, const CStdString& sText)
{
	return m_pList->SetItemText(iItem, iSubItem, sText);
}

BOOL CListUI::GetItemText(int iItem, int iSubItem, CStdString& sText)
{
	return m_pList->GetItemText(iItem, iSubItem, sText);
}

int CListUI::SetItemData(const int iItem, void *pData)
{
	return m_pList->SetItemData(iItem, pData);;
}

int CListUI::AppSubItem(const int nIdx, const int nSubIdx, const char *szDspText)
{
	int nSize = ::strlen(szDspText);
	TCHAR *szwTmp = new TCHAR[nSize + 1];
	memset(szwTmp, 0, sizeof(TCHAR) * (nSize + 1));
	CStringConversion::StringToWideChar(szDspText, szwTmp, nSize);
	int nRet = m_pList->SetItemText(nIdx, nSubIdx, szwTmp);
	delete []szwTmp;
	return nRet;
}

void *CListUI::GetItemData(const int iItem) const
{
	return m_pList->GetItemData(iItem);
}

//redrawing
int CListUI::RedrawItems(int iItemFrom, int iItemTo)
{
	return 0;
}

int CListUI::UpdateItem(int iItem, int iSubItem)
{
	return 0;
}

int CListUI::GetTopIndex() const
{
	int iPos = GetScrollPos(UISB_VERT);
	return iPos;
}

int CListUI::GetCountPerPage() const
{
	CRect rcWorkArea(m_pList->GetPos());
	int cyWorkArea = rcWorkArea.bottom - rcWorkArea.top;
	return (cyWorkArea / m_iItemHeight);
}

int CListUI::HitTest(POINT pt) const
{
	return m_pList->HitTest(pt);
}