#include <uilib/CalendarMonth.h>
#include <uilib/uiscroll.h>

CCalendarMonth::CCalendarMonth(void)
{
}

CCalendarMonth::~CCalendarMonth(void)
{
}

//CControlUI Ðéº¯Êý
LPCTSTR CCalendarMonth::GetClass() const
{
	return L"CALENDARMONTH";
}

void CCalendarMonth::Notify(TNotifyUI &msg)
{
	CContainerUI::Notify( msg );
}

void CCalendarMonth::Event(TEventUI& event)
{
	CContainerUI::Event(event); //¸¸Àà 
}

void CCalendarMonth::DoPaint(HDC hDC, const RECT & rcPaint)
{
}

SIZE CCalendarMonth::EstimateSize(SIZE szAvailable)
{
	return CSize(0, 0);
}

void CCalendarMonth::SetPos(RECT rc)
{

}

void CCalendarMonth::Init()
{
	CContainerUI::Init();
	//
	EnableScrollBar(UISB_VERT, true);
}