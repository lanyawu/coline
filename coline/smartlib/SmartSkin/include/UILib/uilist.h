#if !defined(AFX_UILIST_H__20060218_929F_BDFF_55AA_0080AD509054__INCLUDED_)
#define AFX_UILIST_H__20060218_929F_BDFF_55AA_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <UILib/UIContainer.h>

/////////////////////////////////////////////////////////////////////////////////////
//
static const UINT UILIST_MAX_COLUMNS = 10;
static const UINT UILIST_HEADER_HEIGHT = 16;
static const UINT UILIST_ITEM_HEIGHT = 20;
/////////////////////////////////////////////////////////////////////////////////////
//
//list styles
enum LISTUI_STYLE{
	LUIS_FIRST_ = 0,
	LUIS_LIST = 1,
	LUIS_REPORT,
	LUIS_LAST_
};

/////////////////////////////////////////////////////////////////////////////////////
//
//list item text format
enum LISTUI_TEXT_FMT{
	LUITF_FIRST_ = 0,
	LUITF_LEFT,
	LUITF_CENTER,
	LUITF_RIGHT,
	LUITF_LAST_
};


/////////////////////////////////////////////////////////////////////////////////////
//
//目前只支持list和report风格

class CListHeaderUI;
class CListWorkareaUI;

class CListUI : public CVerticalLayoutUI
{
public:
	CListUI();
	//CControlUI overridables
	LPCTSTR GetClass() const;
	UINT GetControlFlags() const;
	void Init();
	void SetPos(RECT rc);
	void Event(TEventUI& event);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	void DoPaint(HDC hDC, const RECT& rcPaint);
	
	//CContainerUI overridables
	CControlUI* GetItem(const int iIndex) const;
	int GetCount() const;
	bool Add(CControlUI* pControl, const int nIdx);
	bool Remove(CControlUI* pControl);
	void RemoveAll();
	//INotifyUI overridables
	void Notify(TNotifyUI& msg);

	//operation
	BOOL SetStyle(UINT);
	UINT GetStyle() const 
	{ 
		return m_uiStyle; 
	}

	//columns, 0-based
	//return value : column index of the new item, otherwise -1 for failure
	int InsertColumn(int iNewColIndex, const CStdString&, int iTextFmt = LUITF_LEFT, int iWidth = -1);
	BOOL DeleteColumn(int iColumn );
	int GetColumnCount() const;
	int GetColumnWidth(int iColumn) const;
	int GetHeaderWidth() const;
	int PtInSizingRect(POINT pt);
	//items, 0-based, subitem is 0-based too
	int InsertItem(int iItem );
	int InsertItem(int iItem, const CStdString& sLabel, int iImage = -1);
	int AppItem(const char *szDspText);
	int InsertItem(int iItem, const CStdString& sLabel, const CStdString& sImageFile);
	BOOL DeleteItem(const int iItem );
	BOOL DeleteAllItems();
	int SetItemText(const int iItem, int iSubItem, const CStdString& sText);
	BOOL GetItemText(int iItem, int iSubItem, CStdString&);
	int SetItemData(const int iItem, void* pData);
	int AppSubItem(const int nIdx, const int nSubIdx, const char *szDspText);
	void *GetItemData(const int iItem) const;
	//redrawing
	BOOL RedrawItems(int iItemFrom, int iItemTo);
	BOOL UpdateItem(int iItem, int iSubItem);
	int GetCurSel() const;//单行选择模式下有效(当前只有单选模式)
	//如果iIndex小于0或大于等于item总数
	//该函数将使所有已选条目处于未选状态
	bool SelectItem(int iIndex);
	//listwork区域最上方条目的序号
	int GetTopIndex() const;
	//listwork区域可显示的条目数（只统计完整的条目数）
	int GetCountPerPage() const;
	//获取pt下条目index
	int HitTest(POINT pt) const;
    //使能网格线
	void EnableGridLine(BOOL bDrawGrid);
	//
	void KeyDownEvent(WORD wKey);
private:
	void ProcessVScrollBar(int iHeight);
	void ProcessHScrollBar(int iWidth);
	void DrawReportList(HDC hDC, const RECT& rcPaint);
	void DrawNormalList(HDC hDC, const RECT& rcPaint);
	void GetRange(const RECT& rc, int& iFirst, int& iLast);
	void SetColumns(const char *szColumns);
private:
	BOOL m_bDrawGrid; //是否绘制网格
	UINT m_uiStyle;
	int m_iCurSel;
	BOOL m_bShowHeader;
	std::string m_strColumns;
	int m_iItemHeight;//列表条目高度
	RECT m_rcClient;//除去边框的客户区域（条目覆盖边界问题）

	CListWorkareaUI* m_pList;
	CListHeaderUI* m_pHeader;
};

#endif // !defined(AFX_UILIST_H__20060218_929F_BDFF_55AA_0080AD509054__INCLUDED_)
