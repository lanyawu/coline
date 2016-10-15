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
	UINT m_nHCount;//ʵ��һ��gif����
	UINT m_nVCount;//ʵ��һ��gif����
	UINT m_nCurGif;//��ǰѡ�е�gif
	UINT m_nGifPerLine;//����һ��gif����
	BOOL m_bInit;
	RECT m_rcGridArea;//���������λ��
	SIZE m_szGifWnd;//ÿ��gif���ӵĳߴ�
	SIZE m_szFrame; //�߿�
	CGifWindow* m_pGifWnd;//��������
	std::string m_strEmotionName; //
	std::string m_strSelGifName;
	std::string m_strSelGifTag;
	std::map<std::string, std::string> m_EmotionTables;
};