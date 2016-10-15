#include <stdio.h>
#include <setjmp.h>
#include <commonlib/debuglog.h>
#include <commonlib/PDFUtils.h>
#include <math.h>
jmp_buf env;

#pragma warning(disable:4996)

void PDF_Error_Handler(HPDF_STATUS hErrorNo, HPDF_STATUS hStatus, void *pUserData)
{
    PRINTDEBUGLOG(dtInfo, "ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)hErrorNo, (HPDF_UINT)hStatus);
    longjmp(env, 1);
}


void CPDFUtils::PrintPage(int &nPageNumber, HPDF_Page hPage)
{
	char szBuf[512] = {0};
    HPDF_Point hPos = HPDF_Page_GetCurrentTextPos(hPage);
    nPageNumber ++;
	_snprintf(szBuf, 512, ".[%d]%0.2f %0.2f", nPageNumber, hPos.x, hPos.y); 
    HPDF_Page_ShowText(hPage, szBuf);
}

BOOL CPDFUtils::SaveTxtToPDF(const char *szTxt, const char *szPDFFileName)
{
	HPDF_Doc  hPDF;
    HPDF_Page hPage;
    HPDF_Font hFont;
    HPDF_REAL hPage_Height;
    HPDF_Rect hRect;
	//create 
    hPDF = HPDF_New(PDF_Error_Handler, NULL);
	if (hPDF)
	{
		if (!setjmp(env))
		{
			//add new page
			hPage = HPDF_AddPage(hPDF);
			HPDF_Page_SetSize (hPage, HPDF_PAGE_SIZE_A5, HPDF_PAGE_PORTRAIT);
            hPage_Height = HPDF_Page_GetHeight(hPage);
			hFont = HPDF_GetFont(hPDF, "Helvetica", NULL);
			HPDF_Page_SetTextLeading(hPage, 20);
            
			//HPDF_TALIGN_LEFT
			hRect.left = 25;
			hRect.top = 545;
			hRect.right = 200;
            hRect.bottom = hRect.top - 40;
			HPDF_Page_Rectangle(hPage, hRect.left, hRect.bottom, hRect.right - hRect.left, 
				   hRect.top - hRect.bottom);
			HPDF_Page_Stroke(hPage);
            HPDF_Page_BeginText(hPage);
			HPDF_Page_SetFontAndSize(hPage, hFont, 10);
			HPDF_Page_TextOut(hPage, hRect.left, hRect.top + 3, "HPDF_TALIGN_LEFT");
			HPDF_Page_SetFontAndSize(hPage, hFont, 13);
			HPDF_Page_TextRect(hPage, hRect.left, hRect.top, hRect.right, hRect.bottom,
				szTxt, HPDF_TALIGN_LEFT, NULL);
			HPDF_Page_EndText(hPage);

            //HPDF_TALIGN_RIGHT
			hRect.left = 220;
			hRect.right = 395;
			HPDF_Page_Rectangle(hPage, hRect.left, hRect.bottom, hRect.right - hRect.left,
				hRect.top - hRect.bottom);
			HPDF_Page_Stroke(hPage);
			HPDF_Page_BeginText(hPage);
			HPDF_Page_SetFontAndSize(hPage, hFont, 10);
			HPDF_Page_TextOut(hPage, hRect.left, hRect.top + 3, "HPDF_TALIGN_RIGTH");
            HPDF_Page_SetFontAndSize(hPage, hFont, 13);
			HPDF_Page_TextRect(hPage, hRect.left, hRect.top, hRect.right, hRect.bottom,
				szTxt, HPDF_TALIGN_RIGHT, NULL);
			HPDF_Page_EndText(hPage);

			//HPDF_TALIGN_CENTER
            hRect.left = 25;
			hRect.top = 475;
			hRect.right = 200;
			hRect.bottom = hRect.top - 40;
			HPDF_Page_Rectangle(hPage, hRect.left, hRect.bottom, hRect.right - hRect.left,
				hRect.top - hRect.bottom);
			HPDF_Page_Stroke(hPage);
			HPDF_Page_BeginText(hPage);
			HPDF_Page_SetFontAndSize(hPage, hFont, 10);
			HPDF_Page_TextOut(hPage, hRect.left, hRect.top + 3, "HPDF_TALIGN_CENTER");
			HPDF_Page_SetFontAndSize(hPage, hFont, 13);
			HPDF_Page_TextRect(hPage, hRect.left, hRect.top, hRect.right, hRect.bottom,
				szTxt, HPDF_TALIGN_CENTER, NULL);
			HPDF_Page_EndText(hPage);
            
			//HPDF_TALIGN_JUSTIFY
			hRect.left = 220;
			hRect.right = 395;
			HPDF_Page_Rectangle(hPage, hRect.left, hRect.bottom, hRect.right - hRect.left,
				hRect.top - hRect.bottom);
			HPDF_Page_Stroke(hPage);
			HPDF_Page_BeginText(hPage);
			HPDF_Page_SetFontAndSize(hPage, hFont, 10);
			HPDF_Page_TextOut(hPage, hRect.left, hRect.top + 3, "HPDF_TALIGN_JUSTIFY");
			HPDF_Page_SetFontAndSize(hPage, hFont, 13);
			HPDF_Page_TextRect(hPage, hRect.left, hRect.top, hRect.right, hRect.bottom,
				szTxt, HPDF_TALIGN_JUSTIFY, NULL);
			HPDF_Page_EndText(hPage);
  
            //Skewed coordinate system
			HPDF_Page_GSave(hPage);
			float fAngle1 = 5;
			float fAngle2 = 10;
			float fRad1 = fAngle1 / 180 * 3.14159265;
			float fRad2 = fAngle2 / 180 * 3.14159265;
			HPDF_Page_Concat(hPage, 1, tan(fRad1), tan(fRad2), 1, 25, 350);
            hRect.left = 0;
			hRect.top = 40;
			hRect.right = 175;
			hRect.bottom = 0;
			HPDF_Page_Rectangle(hPage, hRect.left, hRect.bottom, hRect.right - hRect.left,
				hRect.top - hRect.bottom);
			HPDF_Page_Stroke(hPage);
			HPDF_Page_BeginText(hPage);
            HPDF_Page_SetFontAndSize(hPage, hFont, 10);
			HPDF_Page_TextOut(hPage, hRect.left, hRect.top + 3, "Skewed coordinate system");
			HPDF_Page_SetFontAndSize(hPage, hFont, 13);
			HPDF_Page_TextRect(hPage, hRect.left, hRect.top, hRect.right, hRect.bottom,
				szTxt, HPDF_TALIGN_LEFT, NULL);
			HPDF_Page_EndText(hPage);
			HPDF_Page_GRestore(hPage);

             
			//Rotated coordinate system
			HPDF_Page_GSave(hPage);
			fAngle1 = 5;
			fRad1 = fAngle1 / 180 * 3.14159265;
			HPDF_Page_Concat(hPage, cos(fRad1), sin(fRad1), -sin(fRad1), cos(fRad1), 220, 350);
			hRect.left = 0;
			hRect.top = 40;
			hRect.right = 175;
			hRect.bottom = 0;
			HPDF_Page_Rectangle(hPage, hRect.left, hRect.bottom, hRect.right - hRect.left,
				hRect.top - hRect.bottom);
			HPDF_Page_Stroke(hPage);
            HPDF_Page_BeginText(hPage);
			HPDF_Page_SetFontAndSize(hPage, hFont, 10);
			HPDF_Page_TextOut(hPage, hRect.left, hRect.top + 3, "Rotated coordinate system");
			HPDF_Page_SetFontAndSize(hPage, hFont, 13);
			HPDF_Page_TextRect(hPage, hRect.left, hRect.top, hRect.right, hRect.bottom,
				szTxt, HPDF_TALIGN_LEFT, NULL);
			HPDF_Page_EndText(hPage);
			HPDF_Page_GRestore(hPage);
 
			//text along a circle
			HPDF_Page_GetGrayStroke(hPage);
			HPDF_Page_Circle(hPage, 210, 190, 145);
			HPDF_Page_Circle(hPage, 210, 190, 113);
			HPDF_Page_Stroke(hPage);
			fAngle1 = 360 / (strlen(szTxt));
			fAngle2 = 180;
			HPDF_Page_BeginText(hPage);
			hFont = HPDF_GetFont(hPDF, "Courier-Bold", NULL);
			HPDF_Page_SetFontAndSize(hPage, hFont, 30);
			int nTxtLen = (int)::strlen(szTxt);
			for (int i = 0; i < nTxtLen; i ++)
			{
				char szBuf[3] = {0};
				float x, y;
				fRad1 = (fAngle2 - 90) / 180 * 3.14159265;
				fRad2 = fAngle2 / 180 * 3.14159265;
                x = 210 + cos(fRad2) * 122;
				y = 190 + sin(fRad2) * 122;
				HPDF_Page_SetTextMatrix(hPage, cos(fRad1), sin(fRad1), -sin(fRad1), cos(fRad1), x, y);
				szBuf[0] = szTxt[i];
				szBuf[1] = '\0';
				HPDF_Page_ShowText(hPage, szBuf);
				fAngle2 -= fAngle1;
			}
			HPDF_Page_EndText(hPage);

			//Save
			HPDF_SaveToFile(hPDF, szPDFFileName);
			HPDF_Free(hPDF);
			return TRUE;
		} else
		{
			HPDF_Free (hPDF);
		} //end if setjmp(enve)
	} else
	{
		PRINTDEBUGLOG(dtInfo, "HPDF_New Failed! can not create pdf object");
	}
	return FALSE;
}

#pragma warning(default:4996)
