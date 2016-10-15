#ifndef __CALENDARUTILS_H___
#define __CALENDARUTILS_H___

#include <commonlib/types.h>

const WORD START_YEAR = 1901;
const WORD END_YEAR   = 2050;

class COMMONLIB_API CCalendarUtils
{
public:
	//�ж�wYear�ǲ�������
    static BOOL IsLeapYear(WORD wYear);

	//����wYear,wMonth,wDay��Ӧ�����ڼ� 1��1��1�� --- 65535��12��31��
	static WORD WeekDay(WORD wYear, WORD wMonth, WORD wDay);

	//����wYear��iMonth�µ����� 1��1�� --- 65535��12��
	static WORD MonthDays(WORD wYear, WORD wMonth);

	//��������wLunarYer������wLunarMonth�µ����������wLunarMonthΪ���£�
	//����Ϊ�ڶ���wLunarMonth�µ��������������Ϊ0 
	// 1901��1��---2050��12��
	static LONG LunarMonthDays(WORD wLunarYear, WORD wLunarMonth);

	//��������wLunarYear���������
	// 1901��1��---2050��12��
	static WORD LunarYearDays(WORD wLunarYear);

	//��������wLunarYear��������·ݣ���û�з���0
	// 1901��1��---2050��12��
	static WORD GetLeapMonth(WORD wLunarYear);

	//��wYear���ʽ������ɼ��귨��ʾ���ַ���
	static void FormatLunarYear(WORD  wYear, char *pBuffer);

	//��wMonth��ʽ���������ַ���
	static void FormatMonth(WORD wMonth, char *pBuffer, BOOL bLunar = TRUE);

    //��wDay��ʽ���������ַ���
	static void FormatLunarDay(WORD  wDay, char *pBuffer);

	//���㹫���������ڼ���������  1��1��1�� --- 65535��12��31��
	static LONG CalcDateDiff(WORD wEndYear, WORD wEndMonth, WORD wEndDay,
		                     WORD wStartYear = START_YEAR, 
							 WORD wStartMonth =1, WORD wStartDay =1);

	//���㹫��wYear��wMonth��wDay�ն�Ӧ����������,���ض�Ӧ���������� 0-24
	//1901��1��1��---2050��12��31��
	static WORD GetLunarDate(WORD wYear, WORD wMonth, WORD wDay, WORD &wLunarYear, WORD &wLunarMonth, WORD &wLunarDay);

	//�����1901��1��1�չ�iSpanDays������������
	static void   CalcLunarDate(WORD &wYear, WORD &wMonth, WORD &wDay, LONG lSpanDays);
    //���㹫��iYear��iMonth��iDay�ն�Ӧ�Ľ��� 0-24��0���ǽ���
	static WORD   GetLunarHolDay(WORD wYear, WORD wMonth, WORD wDay);
};

#endif