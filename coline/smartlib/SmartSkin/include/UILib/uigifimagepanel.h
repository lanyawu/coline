#pragma once

#include <CommonLib/GdiPlusImage.h>

class CGifImagePanelUI: public CControlUI, 
	                    public IImageNotifyEvent
{
public:
	CGifImagePanelUI(void);
	~CGifImagePanelUI(void);
public:
   LPCTSTR GetClass() const;

   //INotifyUI
   void Notify(TNotifyUI& msg);
   void Init();
   UINT GetControlFlags() const;
   //CControUI
   SIZE EstimateSize(SIZE szAvailable);
   void DoPaint(HDC hDC, const RECT& rcPaint);
   void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
   void OnInvalidate(LPRECT lprc);
   void Event(TEventUI& e);

   //operation
   bool SetImage(LPCTSTR lpszFileName, BOOL bTransparent, COLORREF crTransparent, bool bAnimation = true);
   bool SetImage(char *szFileName, BOOL bTransparent, COLORREF crTransparent, bool bAnimation = true);
   void ClearImage();
   CStdString GetGifFileName() const;
   BOOL GetGifFileName(std::string &strFileName);
   void SetGifTag(LPCTSTR lpszTag);
   BOOL GetGifTag(std::string &strTag);
   CStdString GetGifTag() const;
   void SetGifShortcut(LPCTSTR lpszShortcut);
   CStdString GetGifShortcut() const;
   void SetIndex(UINT nIndex);
   UINT GetIndex() const;
   void SetGifPadding(UINT nPadding);
private:
	BOOL m_bAnimation;
	COLORREF m_crTransparent;
	BOOL m_bTransparent;
	CStdString m_sGifFile;//image file name
	CStdString m_sGifTag;//tag
	CStdString m_sShortcut;//
	CGdiPlusGif *m_pGif;
	SIZE m_ImageSize;
	UINT m_nIndex;
	UINT m_nGifPadding; 
	BOOL m_bLink;
};