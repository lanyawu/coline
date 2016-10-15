#ifndef __CALENDARMONTH__H___
#define __CALENDARMONTH__H___

 
#include <CommonLib/GraphicPlus.h>
#include <CommonLib/GdiPlusImage.h>
#include "common.h"
#include <UILib/UIContainer.h>
#include <UILib/uiresource.h>

class CCalendarMonth: public CContainerUI
{
public:
	CCalendarMonth(void);
	~CCalendarMonth(void);
public:
	//CControlUI Ðéº¯Êý
	LPCTSTR GetClass() const;
	void Notify(TNotifyUI &msg);
	void Event(TEventUI& event);
	void DoPaint(HDC hDC, const RECT & rcPaint);
    SIZE EstimateSize(SIZE szAvailable);
	void SetPos(RECT rc);
	void Init();
};

#endif
