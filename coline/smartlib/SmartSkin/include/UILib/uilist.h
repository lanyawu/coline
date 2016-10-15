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
//Ŀǰֻ֧��list��report���

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
	int GetCurSel() const;//����ѡ��ģʽ����Ч(��ǰֻ�е�ѡģʽ)
	//���iIndexС��0����ڵ���item����
	//�ú�����ʹ������ѡ��Ŀ����δѡ״̬
	bool SelectItem(int iIndex);
	//listwork�������Ϸ���Ŀ�����
	int GetTopIndex() const;
	//listwork�������ʾ����Ŀ����ֻͳ����������Ŀ����
	int GetCountPerPage() const;
	//��ȡpt����Ŀindex
	int HitTest(POINT pt) const;
    //ʹ��������
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
	BOOL m_bDrawGrid; //�Ƿ��������
	UINT m_uiStyle;
	int m_iCurSel;
	BOOL m_bShowHeader;
	std::string m_strColumns;
	int m_iItemHeight;//�б���Ŀ�߶�
	RECT m_rcClient;//��ȥ�߿�Ŀͻ�������Ŀ���Ǳ߽����⣩

	CListWorkareaUI* m_pList;
	CListHeaderUI* m_pHeader;
};

#endif // !defined(AFX_UILIST_H__20060218_929F_BDFF_55AA_0080AD509054__INCLUDED_)
