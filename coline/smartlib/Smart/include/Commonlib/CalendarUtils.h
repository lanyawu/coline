#ifndef __CALENDARUTILS_H___
#define __CALENDARUTILS_H___

#include <commonlib/types.h>

const WORD START_YEAR = 1901;
const WORD END_YEAR   = 2050;

class COMMONLIB_API CCalendarUtils
{
public:
	//判断wYear是不是闰年
    static BOOL IsLeapYear(WORD wYear);

	//计算wYear,wMonth,wDay对应是星期几 1年1月1日 --- 65535年12月31日
	static WORD WeekDay(WORD wYear, WORD wMonth, WORD wDay);

	//返回wYear年iMonth月的天数 1年1月 --- 65535年12月
	static WORD MonthDays(WORD wYear, WORD wMonth);

	//返回阴历wLunarYer年阴历wLunarMonth月的天数，如果wLunarMonth为闰月，
	//高字为第二个wLunarMonth月的天数，否则高字为0 
	// 1901年1月---2050年12月
	static LONG LunarMonthDays(WORD wLunarYear, WORD wLunarMonth);

	//返回阴历wLunarYear年的总天数
	// 1901年1月---2050年12月
	static WORD LunarYearDays(WORD wLunarYear);

	//返回阴历wLunarYear年的闰月月份，如没有返回0
	// 1901年1月---2050年12月
	static WORD GetLeapMonth(WORD wLunarYear);

	//把wYear年格式化成天干记年法表示的字符串
	static void FormatLunarYear(WORD  wYear, char *pBuffer);

	//把wMonth格式化成中文字符串
	static void FormatMonth(WORD wMonth, char *pBuffer, BOOL bLunar = TRUE);

    //把wDay格式化成中文字符串
	static void FormatLunarDay(WORD  wDay, char *pBuffer);

	//计算公历两个日期间相差的天数  1年1月1日 --- 65535年12月31日
	static LONG CalcDateDiff(WORD wEndYear, WORD wEndMonth, WORD wEndDay,
		                     WORD wStartYear = START_YEAR, 
							 WORD wStartMonth =1, WORD wStartDay =1);

	//计算公历wYear年wMonth月wDay日对应的阴历日期,返回对应的阴历节气 0-24
	//1901年1月1日---2050年12月31日
	static WORD GetLunarDate(WORD wYear, WORD wMonth, WORD wDay, WORD &wLunarYear, WORD &wLunarMonth, WORD &wLunarDay);

	//计算从1901年1月1日过iSpanDays天后的阴历日期
	static void   CalcLunarDate(WORD &wYear, WORD &wMonth, WORD &wDay, LONG lSpanDays);
    //计算公历iYear年iMonth月iDay日对应的节气 0-24，0表不是节气
	static WORD   GetLunarHolDay(WORD wYear, WORD wMonth, WORD wDay);
};

#endif