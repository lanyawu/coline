#ifndef __SCINTILLAEDIT_H____
#define __SCINTILLAEDIT_H____

#include <UILib/UILib.h>
#include <vector>

typedef struct CAttrCacheBuff
{
	TCHAR *szName;
	TCHAR *szValue;
}ATTR_CACHE_BUFFER, *LPATTR_CACHE_BUFFER;

class CScintillaEditUI :public CContainerUI
{
public:
	CScintillaEditUI(void);
	~CScintillaEditUI(void);
public:
    //CControlUIÐéº¯Êý
	LPCTSTR GetClass() const;
	UINT GetControlFlags() const;
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	void Init();
	void SetPos(RECT rc);
	void DoPaint(HDC hDC, const RECT& rcPaint);
	CStdString GetText() const;
	void SetText(LPCTSTR pstrText);

	void SetEnabled(bool bEnabled);
	void SetVisible(bool bVisible);
	void SetReadOnly(BOOL bReadOnly);
	void SetFocus();
	void Event(TEventUI& event);
	void Notify(TNotifyUI& msg) ;
	SIZE EstimateSize(SIZE szAvailable);
	//ÉèÖÃ¹Ø¼ü´Ê
	void SetKeyWord(LPCTSTR pstrKeyWord);
	void SetWordStyle(int nStyle, COLORREF clrFore, COLORREF clrBack, int nSize, LPCTSTR szFace);
private:
	BOOL CreateScintillaEdit();
private:
	static HMODULE m_hScEditModule;
    static volatile LONG  m_hScEditRef;
	HWND m_hEdit;
	
	//
	std::vector<LPATTR_CACHE_BUFFER> m_AttrList;
};

#endif
