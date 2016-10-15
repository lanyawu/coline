// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <Windows.h>
#include "../P2Svr/P2Svr.h"

#pragma warning(disable:4996)

BOOL IsMobileNumber(const char *szNumber)
{
	if (szNumber && (::strlen(szNumber) == 11) && (szNumber[0] == '1')) //mobile phone number is 11
	{
		for (int i = 1; i < 11; i ++)
		{
			if (szNumber[i] < '0' || szNumber[i] > '9')
				return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

int GetDate(const char **p)
{
	const char *p1 = *p;
	while (*p1)
	{
		if ((*p1 == '/') || (*p1 == '-') || (*p1 == ' ')
			|| (*p1 == ':'))
			break;
		p1 ++;
	}
	char szTmp[6] = {0};
	int nSize = p1 - *p;
	if ((nSize > 0) && (nSize < 5))
	{
		strncpy(szTmp, *p, nSize);
		if (*p1)
			p1 ++;
		*p = p1;		
		return ::atoi(szTmp);
	}
	return 0;
}

 
int GetTime(const char **p)
{
	const char *p1 = *p;
	while (*p1)
	{
		if (*p1 == ':')
			break;
		p1 ++;
	}
	char szTmp[6] = {0};
	int nSize = p1 - *p;
	if ((nSize > 0) && (nSize < 5))
	{
		strncpy(szTmp, *p, nSize);
		if (*p1)
			p1 ++;
		*p = p1;
		return ::atoi(szTmp);
	}
	return 0;
}

int  GetDays(int y, int m, int d)
{
	static int months[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	int nDays = (y - 2000) * 365;
	nDays += (y - 2000) / 4 + 1;
	for (int i = 0; i < m - 1; i ++)
	{
		nDays += months[i];
	}
	if ((m > 2) && ((y % 4) == 0))
		nDays ++;
	nDays += d;
	return nDays;
}

void SplitDays(int nDays, int &y, int &m, int &d)
{
	static int months[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	y = nDays / 365;
	nDays -= ((y / 4) + 1);
	y = nDays / 365;
	nDays %= 365;
	m = 1;
	bool bIsLeap = ((y % 4) == 0);
    for (int i = 0; i < 11; i ++)
	{
		if (bIsLeap && (m == 3))
			nDays --;
		if ((m == 2) && bIsLeap && nDays == 29)
			break;
		if (nDays <= months[i])
			break;	
		nDays -= months[i];
		m ++;	
	}
	d = nDays;
}

void SplitTime(const int nStamp, int &h, int &m, int &s)
{
	h = nStamp / 3600;
	m = (nStamp % 3600) / 60;
	s = (nStamp % 60);
}

bool StringToTimeStamp(const char *szTime, int &nStamp)
{
	const char *p = szTime;
	int y = GetDate(&p);
	int m = GetDate(&p);
	int d = GetDate(&p);
	int h = GetTime(&p);
	int min = GetTime(&p);
	int sec = GetTime(&p);
	if ((y > 0) && (m > 0) && (d > 0))
	{
		int nDays = GetDays(y, m, d);
		nStamp = nDays * 24 * 3600;
		nStamp += (h * 3600);
		nStamp += (min * 60);
		nStamp += sec;
		return true;
	}
	return false;
}

void TestMobileNumber()
{
	std::string strTmp = "1213p13414";
	std::string strResult;
	if (IsMobileNumber(strTmp.c_str()))
		strResult = "TRUE";
	else
		strResult = "FALSE";
	printf("Number:%s  is %s\n", strTmp.c_str(), strResult.c_str());

	strTmp = "32324214213"; 
	if (IsMobileNumber(strTmp.c_str()))
		strResult = "TRUE";
	else
		strResult = "FALSE";
	printf("Number:%s  is %s\n", strTmp.c_str(), strResult.c_str());

	strTmp = "13501323416";
	if (IsMobileNumber(strTmp.c_str()))
		strResult = "TRUE";
	else
		strResult = "FALSE";
	printf("Number:%s  is %s\n", strTmp.c_str(), strResult.c_str());

	strTmp = "18611721860";
	if (IsMobileNumber(strTmp.c_str()))
		strResult = "TRUE";
	else
		strResult = "FALSE";
	printf("Number:%s  is %s\n", strTmp.c_str(), strResult.c_str());

	strTmp = "13501@gocom";
	if (IsMobileNumber(strTmp.c_str()))
		strResult = "TRUE";
	else
		strResult = "FALSE";
	printf("Number:%s  is %s\n", strTmp.c_str(), strResult.c_str());

	strTmp = "wuxz@gocom";
	if (IsMobileNumber(strTmp.c_str()))
		strResult = "TRUE";
	else
		strResult = "FALSE";
	printf("Number:%s  is %s\n", strTmp.c_str(), strResult.c_str());
}

void TestHttpUpload()
{
	const char p[] = "2012-12-29 15:16:34";
	int nStamp = 0;
	StringToTimeStamp(p, nStamp);
	printf("%d", nStamp);
	int y, m, d;
	SplitDays(nStamp / 24 / 3600, y, m, d);
	printf("\n\t%d-%d-%d", y + 2000, m, d);
	int h, min, sec;
	nStamp = (nStamp % (3600 * 24));
	SplitTime(nStamp, h, min, sec);
	printf("\n\t%d:%d:%d", h, min, sec);
	//DWORD w = GetTickCount();
	//int n = w;
	//printf("DWORD: %u   int:%d", w, n);
	//::PostHttpFile("http://172.20.40.15:9910/uppic.php", "F:\\a.vpx", "username=a@gocom", NULL, NULL, TRUE);
}

void ParserTelQuery(const char *p)
{
	std::string strQuery = p;
	int nPos = strQuery.find("电话");
	if (nPos != std::string::npos)
	{
		std::string strTmp = strQuery.substr(0, nPos);
		TCHAR *pTmp = new TCHAR[strTmp.size() + 1];
		memset(pTmp, 0, sizeof(TCHAR) * (strTmp.size() + 1));
		MultiByteToWideChar(::GetACP(), 0, strTmp.c_str(), -1, pTmp, strTmp.size());
		TCHAR szName3[4] = {0}, szName2[3] = {0}, szName1[2] = {0};
		szName1[0] = pTmp[::lstrlen(pTmp) - 1];
		if (::lstrcmpi(szName1, L"的") == 0)
			pTmp[::lstrlen(pTmp) - 1] = L'\0';
		szName1[0] = pTmp[::lstrlen(pTmp) - 1];
		if ((::lstrcmpi(szName1, L"你") == 0)
			|| (::lstrcmpi(szName1, L"您") == 0))
		{
			printf("你  电话\n");
		} else
		{
			int nSize = ::lstrlen(pTmp);
			if (nSize >= 2)
			{
				szName2[0] = pTmp[nSize - 2];
				szName2[1] = pTmp[nSize - 1];
			}
			if (nSize >= 3)
			{
				szName3[0] = pTmp[nSize - 3];
				szName3[1] = pTmp[nSize - 2];
				szName3[2] = pTmp[nSize - 1];
			}
			::wprintf(L"Name1:%s    Name2:%s\n", szName2, szName3);
		}
	}
	printf("invalid\n");
}
void TestParserTel()
{
	ParserTelQuery("想问一下你的电话");
	ParserTelQuery("你电话");
	ParserTelQuery("胡的电话");
	ParserTelQuery("邬晓忠的电话");
	ParserTelQuery("晓中电话");
}

BOOL TestCheckVersion()
{
	typedef BOOL (CALLBACK *UPDATE_FILE_FROM_SVR)(const char *szCurrVer, const char *szUrl, const char *szTempPath);
	HMODULE h = ::LoadLibrary(L"checkVersion.dll");
	if (h != NULL)
	{
		UPDATE_FILE_FROM_SVR pProc = (UPDATE_FILE_FROM_SVR)::GetProcAddress(h, "UpdateFilesFromSvr");
		if (pProc)
			pProc("E:\\update.xml", "http://172.20.40.15:9910/updater/newver.xml", "C:\\Users\\sunnyu\\AppData\\Local\\GoCom\\update\\");
		while(1)
		{
			Sleep(500);
		}
		FreeLibrary(h);
	}
	return TRUE;
}

DWORD StringTimeToSecond(const char *strTime)
{
	int nSec = 0;
	if (strTime)
	{
		char szTmp[3] = {0};
		int j = 2;
		int f = 0;
		int n = ::strlen(strTime);
		for (int i = n -1; i >= 0; i --)
		{
			if (strTime[i] ==  ':')
			{
				if (j != 0)
					f =  (szTmp[j] - '0');
				else
					f = ::atoi(szTmp);
				if (nSec == 0)
					nSec = f;
				else
				{
					nSec += (f * 60);
					break;
				}
				memset(szTmp, 0, 3);
				j = 2;
				continue;
			} else
			{
				if (j == 0)
				{
					nSec = 0;
					break;
				} else
				{
					j --;
					szTmp[j] = strTime[i];
				}
			} //end else
		}
	}
	return nSec;
}

//计算两个字符串的日期差 秒为单位 只取最后 分:秒
DWORD  MinusTimeString( )
{
	char strTime[] = "2011-04-22 445:44:683";
	printf("%s --- %d\n", strTime, StringTimeToSecond(strTime)); 
	return 0;
}

void TestErro()
{
	char *p = NULL;
	sprintf(p, "afklajd;f");
	printf("%s", p);
}

LONG __stdcall MyExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
	exit( pExceptionInfo->ExceptionRecord->ExceptionCode ); 
	return EXCEPTION_EXECUTE_HANDLER;
}

int _tmain(int argc, _TCHAR* argv[])
{
	SetUnhandledExceptionFilter(MyExceptionFilter);
	//MinusTimeString();
	TestErro();
	//TestCheckVersion();
	while (1)
	{
		Sleep(500);
	}
	return 0;
}

#pragma warning(default:4996)
