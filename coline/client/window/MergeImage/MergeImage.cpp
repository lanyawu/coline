// MergeImage.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Commonlib/graphicplus.h>

#pragma warning(disable:4996)

bool MergeImage(const char *szSrcFile1, const char *szSrcFile2, const char *szSrcFile3, const char *szSrcFile4,
	            const char *szDstFile)
{
	CGraphicPlus src1, src2, src3, src4;
	CGraphicPlus dst;
	if (src1.LoadFromFile(szSrcFile1) 
		&& src2.LoadFromFile(szSrcFile2) 
		&& src3.LoadFromFile(szSrcFile3)
		&& src4.LoadFromFile(szSrcFile4))
	{
		int cx = src1.GetWidth();
		int cy = src1.GetHeight() * 4;
		
		HDC hdc = ::GetDC(::GetDesktopWindow());
	    HDC m_hMem = ::CreateCompatibleDC(hdc);
		HBITMAP hBitmap = ::CreateCompatibleBitmap(hdc, cx, cy);
        HBITMAP m_hOldBitmap = (HBITMAP)::SelectObject(m_hMem, hBitmap);
		RECT rc = {0, 0, cx, cy};
		HBRUSH hbr = ::CreateSolidBrush(RGB(255, 255, 255));
		::FillRect(m_hMem, &rc, hbr);
		::DeleteObject(hbr);
		src1.DrawToDc(m_hMem, 0, 0, cx, src1.GetHeight());
		src2.DrawToDc(m_hMem, 0, src1.GetHeight(), cx, src2.GetHeight());
		src3.DrawToDc(m_hMem, 0, src1.GetHeight() + src2.GetHeight(), cx, src3.GetHeight());
		src4.DrawToDc(m_hMem, 0, src1.GetHeight() + src2.GetHeight() + src3.GetHeight(), cx, src4.GetHeight());
		dst.LoadFromBitmap(hBitmap);
		dst.SaveToFile(szDstFile, GRAPHIC_TYPE_PNG);
		::SelectObject(m_hMem, m_hOldBitmap);
		::DeleteObject(hBitmap);
		::DeleteObject(m_hMem);
		::ReleaseDC(::GetDesktopWindow(), hdc);
		return true;
	}
	return false;
	 
}

bool MergeImageCheck(const char *szSrcFile1, const char *szSrcFile2, const char *szSrcFile3,
	            const char *szDstFile)
{
	CGraphicPlus src1, src2, src3;
	CGraphicPlus dst;
	if (src1.LoadFromFile(szSrcFile1) 
		&& src2.LoadFromFile(szSrcFile2) 
		&& src3.LoadFromFile(szSrcFile3))
	{
		int cx = src1.GetWidth();
		int cy = src1.GetHeight() * 3;
		
		HDC hdc = ::GetDC(::GetDesktopWindow());
	    HDC m_hMem = ::CreateCompatibleDC(hdc);
		HBITMAP hBitmap = ::CreateCompatibleBitmap(hdc, cx, cy);
        HBITMAP m_hOldBitmap = (HBITMAP)::SelectObject(m_hMem, hBitmap);
		RECT rc = {0, 0, cx, cy};
		HBRUSH hbr = ::CreateSolidBrush(RGB(255, 255, 255));
		::FillRect(m_hMem, &rc, hbr);
		::DeleteObject(hbr);
		src1.DrawToDc(m_hMem, 0, 0, cx, src1.GetHeight());
		src2.DrawToDc(m_hMem, 0, src1.GetHeight(), cx, src2.GetHeight());
		src3.DrawToDc(m_hMem, 0, src1.GetHeight() + src2.GetHeight(), cx, src3.GetHeight());
		dst.LoadFromBitmap(hBitmap);
		dst.SaveToFile(szDstFile, GRAPHIC_TYPE_PNG);
		::SelectObject(m_hMem, m_hOldBitmap);
		::DeleteObject(hBitmap);
		::DeleteObject(m_hMem);
		::ReleaseDC(::GetDesktopWindow(), hdc);
		return true;
	}
	return false;
	 
}

bool MergerBkg(const char *szLeftTopFile, const char *szTopCenterFile, const char *szTopRightFile,
	          const char *szBotLeftFile, const char *szBotCenterFile, const char *szBotRightFile, const char *szbkgFile,
			  const char *szLeftFile, const char *szRightFile, const char *szDstFile)
{
	CGraphicPlus srcLeftTop, srcTopCenter, srcTopRight;
	CGraphicPlus srcBotLeft, srcBotCenter, srcBotRight;
	CGraphicPlus srcBkg, srcLeft, srcRight;
	CGraphicPlus dst;
	if (srcLeftTop.LoadFromFile(szLeftTopFile)
		&& srcTopCenter.LoadFromFile(szTopCenterFile)
		&& srcTopRight.LoadFromFile(szTopRightFile)
		&& srcBotLeft.LoadFromFile(szBotLeftFile)
		&& srcBotCenter.LoadFromFile(szBotCenterFile)
		&& srcBotRight.LoadFromFile(szBotRightFile)
		&& srcBkg.LoadFromFile(szbkgFile)
		&& srcLeft.LoadFromFile(szLeftFile)
		&& srcRight.LoadFromFile(szRightFile))
	{
		int cx = srcLeftTop.GetWidth() + srcTopCenter.GetWidth() + srcTopRight.GetWidth();
		int cy = srcLeftTop.GetHeight() + srcBotLeft.GetHeight() + srcBkg.GetHeight();
		
		HDC hdc = ::GetDC(::GetDesktopWindow());
	    HDC m_hMem = ::CreateCompatibleDC(hdc);
		HBITMAP hBitmap = ::CreateCompatibleBitmap(hdc, cx, cy);
        HBITMAP m_hOldBitmap = (HBITMAP)::SelectObject(m_hMem, hBitmap);
		srcLeftTop.DrawToDc(m_hMem, 0, 0, srcLeftTop.GetWidth(), srcLeftTop.GetHeight());
		srcTopCenter.DrawToDc(m_hMem, srcLeftTop.GetWidth(), 0, srcTopCenter.GetWidth(), srcTopCenter.GetHeight());
		srcTopRight.DrawToDc(m_hMem, srcLeftTop.GetWidth() + srcTopCenter.GetWidth(), 0, 
			                 srcTopRight.GetWidth(), srcTopRight.GetHeight());
		srcLeft.DrawToDc(m_hMem, 0, srcLeftTop.GetHeight(), srcLeft.GetWidth(), srcBkg.GetHeight());
		srcBkg.DrawToDc(m_hMem, srcLeft.GetWidth(), srcLeftTop.GetHeight(), cx - srcLeft.GetWidth() - srcRight.GetWidth(),
			srcBkg.GetHeight());
		srcRight.DrawToDc(m_hMem, cx - srcRight.GetWidth(), srcLeftTop.GetHeight(), srcRight.GetWidth(), srcBkg.GetHeight());
		srcBotLeft.DrawToDc(m_hMem, 0, cy - srcBotLeft.GetHeight(), srcBotLeft.GetWidth(), srcBotLeft.GetHeight());
		srcBotRight.DrawToDc(m_hMem, cx - srcBotRight.GetWidth(), cy - srcBotRight.GetHeight(), srcBotRight.GetWidth(),
			srcBotRight.GetHeight());
		srcBotCenter.DrawToDc(m_hMem, srcBotLeft.GetWidth(), cy - srcBotCenter.GetHeight(), cx - srcBotLeft.GetWidth() - srcBotRight.GetWidth(),
			srcBotCenter.GetHeight());
		dst.LoadFromBitmap(hBitmap);
		dst.SaveToFile(szDstFile, GRAPHIC_TYPE_BMP);
		::SelectObject(m_hMem, m_hOldBitmap);
		::DeleteObject(hBitmap);
		::DeleteObject(m_hMem);
		::ReleaseDC(::GetDesktopWindow(), hdc);
	}
	return false;
}

void MergeIcon()
{
	static char PATH[] = "F:\\lanya\\workarea\\genersoft\\software\\GoComClient\\skins\\default\\loginform\\";
	static char DESTPATH[] = "F:\\lanya\\workarea\\GoCom\\Skin\\Images\\chatform\\";
	//static char PATH[] = "F:\\lanya\\workarea\\genersoft\\software\\GoComClient\\images\\loginform\\";
	char szSrcFile1[MAX_PATH] = {0};
	char szSrcFile2[MAX_PATH] = {0};
	char szSrcFile3[MAX_PATH] = {0};
	char szSrcFile4[MAX_PATH] = {0};
	char szDstFile[MAX_PATH] = {0};
	sprintf(szSrcFile1, "%s%s", PATH, "checkbox_uncheck.bmp");
	sprintf(szSrcFile2, "%s%s", PATH, "checkbox_uncheck_move.bmp");
	sprintf(szSrcFile3, "%s%s", PATH, "checkbox_check_down.bmp");
	sprintf(szSrcFile4, "%s%s", PATH, "checkbox_check_disable.bmp");
	sprintf(szDstFile, "%s%s", DESTPATH, "checkbox.bmp");
	MergeImage(szSrcFile1, szSrcFile2, szSrcFile3, szSrcFile4, szDstFile); 
}

void MergeCheckIcon()
{
	static char PATH[] = "F:\\lanya\\workarea\\genersoft\\software\\GoComClient\\skins\\default\\loginform\\";
	static char DESTPATH[] = "F:\\lanya\\workarea\\GoCom\\bin\\debug\\Skin\\Images\\chatform\\";
	//static char PATH[] = "F:\\lanya\\workarea\\genersoft\\software\\GoComClient\\images\\loginform\\";
	char szSrcFile1[MAX_PATH] = {0};
	char szSrcFile2[MAX_PATH] = {0};
	char szSrcFile3[MAX_PATH] = {0};
	char szSrcFile4[MAX_PATH] = {0};
	char szDstFile[MAX_PATH] = {0};
	sprintf(szSrcFile1, "%s%s", PATH, "checkbox_uncheck.bmp");
	sprintf(szSrcFile2, "%s%s", PATH, "checkbox_check.bmp");
	sprintf(szSrcFile3, "%s%s", PATH, "checkbox_check_disable.bmp");
	sprintf(szDstFile, "%s%s", DESTPATH, "checkstatus.bmp");
	MergeImageCheck(szSrcFile1, szSrcFile2, szSrcFile3, szDstFile); 
}

void MergeBackground()
{
	static char PATH[] = "F:\\lanya\\workarea\\genersoft\\software\\GoComClient\\skins\\default\\chatform\\";
	static char DESTPATH[] = "F:\\lanya\\workarea\\GoCom\\Skin\\Images\\chatform\\";
	char szTopLeftFile[MAX_PATH] = {0};
	char szTopCenter[MAX_PATH] = {0};
	char szTopRight[MAX_PATH] = {0};
	char szBotLeft[MAX_PATH] = {0};
	char szBotCenter[MAX_PATH] = {0};
	char szBotRight[MAX_PATH] = {0};
	char szDstFile[MAX_PATH] = {0};
	char szLeftFile[MAX_PATH] = {0};
	char szRightFile[MAX_PATH] = {0};
	char szBkgFile[MAX_PATH] = {0};
	sprintf(szTopLeftFile, "%s%s", PATH, "BackgroundTitleLeft.bmp");
	sprintf(szTopCenter, "%s%s", PATH, "BackgroundTitleCenter.bmp");
	sprintf(szTopRight, "%s%s", PATH, "BackgroundTitleRight.bmp");
	sprintf(szBotLeft, "%s%s", PATH, "BackgroundBottomLeft.bmp");
	sprintf(szBotCenter, "%s%s", PATH, "BackgroundBottomCenter.bmp");
	sprintf(szBotRight, "%s%s", PATH, "BackgroundBottomRight.bmp");
	sprintf(szLeftFile, "%s%s", PATH, "BackgroundMiddleLeft.bmp");
	sprintf(szRightFile, "%s%s", PATH, "BackgroundMiddleRight.bmp");
	sprintf(szBkgFile, "%s%s", PATH, "bkg.bmp");
	sprintf(szDstFile, "%s%s", DESTPATH, "background.bmp");
	MergerBkg(szTopLeftFile, szTopCenter, szTopRight, szBotLeft,szBotCenter, szBotRight, szBkgFile, 
		szLeftFile, szRightFile, szDstFile); 
}

int _tmain(int argc, _TCHAR* argv[])
{
	//MergeBackground();
	//MergeIcon();
	MergeCheckIcon();
	return 0;
}

#pragma warning(default:4996)
