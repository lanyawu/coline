#ifndef __GIFIMAGEPANELUI_H__
#define __GIFIMAGEPANELUI_H__

#include <UILib/UILib.h>
#include <CommonLib/GdiPlusImage.h>

class CGifImagePanelUI :public CControlUI, public IImageNotifyEvent
{
public:
	CGifImagePanelUI(void);
	~CGifImagePanelUI(void);
public:
   LPCTSTR GetClass() const;

   SIZE EstimateSize(SIZE szAvailable);
   void DoPaint(HDC hDC, const RECT& rcPaint);
   void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
   void OnInvalidate(LPRECT lprc);
public:
	BOOL LoadImage(LPCTSTR lpszFileName);
private:
	CGdiPlusGif *m_pGif;
	SIZE m_ImageSize;
};

#endif