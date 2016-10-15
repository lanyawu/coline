#pragma once

#include <UILib/UIContainer.h>

class CGifImagePanelUI;
class CGifWindow;
class CGifGridPanelUI: public CContainerUI
{
public:
	CGifGridPanelUI();
	~CGifGridPanelUI();

	//INotifyUI overridable
	void Notify(TNotifyUI& msg);

	//CControlUI overridable
	LPCTSTR GetClass() const;
	void Init();
	void Event(TEventUI& event);
	void SetPos(RECT rc);
	void DoPaint(HDC hDC, const RECT& rcPaint);
	SIZE EstimateSize(SIZE szAvailable);
	int  GetShowGifPanelCount(); 
	//CContainerUI overridable
	bool Add(CControlUI* pControl, const int nIdx);
	virtual void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	BOOL AddGifUI(const char *szFileName, const char *szTag, const char *szCut, const char *szTip);
	BOOL GetSelGifInfo(char *szGifName, int *nNameSize, char *szGifTag, int *nTagSize);
	//operation
	HWND GetFloatingWnd() const;
	UINT GetGifPerLine() const 
	{ 
		return m_nGifPerLine; 
	}
	void SetGifPerLine(UINT nPerLine) 
	{ 
		if( m_nGifPerLine != nPerLine ) m_nGifPerLine = nPerLine; 
	}
private:
	void InitChildrenPos();
	void SelectGif( CGifImagePanelUI* );
	void DeleteChildWnd();
	void InitEmotion();
private:
	UINT m_nHCount;//实际一行gif个数
	UINT m_nVCount;//实际一列gif个数
	UINT m_nCurGif;//当前选中的gif
	UINT m_nGifPerLine;//设置一行gif个数
	BOOL m_bInit;
	RECT m_rcGridArea;//格子区域的位置
	SIZE m_szGifWnd;//每个gif格子的尺寸
	SIZE m_szFrame; //边框
	CGifWindow* m_pGifWnd;//浮动窗口
	std::string m_strEmotionName; //
	std::string m_strSelGifName;
	std::string m_strSelGifTag;
	std::map<std::string, std::string> m_EmotionTables;
};