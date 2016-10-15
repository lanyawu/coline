#ifndef __PDFUTILS_H___________
#define __PDFUTILS_H___________

#include <commonlib/types.h>
#include <pdflib/hpdf.h>

class COMMONLIB_API CPDFUtils
{
private:
	static void PrintPage(int &nPageNumber, HPDF_Page hPage);
public:
	static BOOL SaveTxtToPDF(const char *szTxt, const char *szPDFFileName);
};

#endif