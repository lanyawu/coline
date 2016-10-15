#include <Commonlib/stringutils.h>
#include "../IMCommonLib/InterfaceAnsiString.h"
#include "XmlNodeTranslate.h"

const int CFE_BOLD = 0x0001;
const int CFE_ITALIC = 0x0002;
const int CFE_UNDERLINE = 0x0004;
const int CFE_STRIKEOUT = 0x0008;

#pragma warning(disable:4996)

int  CXmlNodeTranslate::FontStringToSize(const char *szStr)
{
	int n = 0;
	const char *p = szStr;
	while (*p)
	{
		if (((*p) >= '0') && ((*p) <= '9'))
		{
			n *= 10;
			n += (*p) - '0';
		} else
			break;
		p++;
	}
	return n;
}

void CXmlNodeTranslate::FontSizeToString(int nSize, std::string &str)
{
	char szTmp[16] = {0};
	::itoa(nSize, szTmp, 10);
	str = szTmp;
	str += "pt";
}

int CXmlNodeTranslate::FontStringToColor(const char *szStr)
{
	COLORREF clr = 0;
	if (*szStr == '#')
	{
		szStr++;
		clr = ::strtol(szStr, const_cast<char **>(&szStr), 16);		 
	}
	return clr;
}

void CXmlNodeTranslate::FontColorToString(int nClr, std::string &str)
{
	char szTmp[16] = {0};
	::itoa(nClr, szTmp, 16);
	str = "#";
	str += szTmp;
}
 
void CXmlNodeTranslate::FontStyleToXmlNode(const CCharFontStyle &cf, TiXmlElement *pFont)
{
	char szTmp[36] = {0};
	CStringConversion::WideCharToString(cf.szFaceName, szTmp, 35);
	pFont->SetAttribute("name", szTmp);
	std::string strTmp;
	FontSizeToString(cf.nFontSize, strTmp);
	pFont->SetAttribute("size", strTmp.c_str());
	FontColorToString(cf.cfColor, strTmp);
	pFont->SetAttribute("color", strTmp.c_str());
	if (cf.nFontStyle & CFE_BOLD)
		pFont->SetAttribute("bold", "true");
	else
		pFont->SetAttribute("bold", "false");
	if (cf.nFontStyle & CFE_UNDERLINE)
		pFont->SetAttribute("underline", "true");
	else
		pFont->SetAttribute("underline", "false");
	if (cf.nFontStyle & CFE_ITALIC)
		pFont->SetAttribute("italic", "true");
	else
		pFont->SetAttribute("italic", "false");
	if (cf.nFontStyle & CFE_STRIKEOUT)
		pFont->SetAttribute("strikeout", "true");
	else
		pFont->SetAttribute("strikeout", "false");
}

void CXmlNodeTranslate::FontXmlNodeToStyle(TiXmlElement *pFont, CCharFontStyle &cf)
{
	const char *szAttr = pFont->Attribute("name");
	if (szAttr)
		CStringConversion::StringToWideChar(szAttr, cf.szFaceName, 31);
	szAttr = pFont->Attribute("size");
	if (szAttr)
		cf.nFontSize =CXmlNodeTranslate::FontStringToSize(szAttr);
	else
		cf.nFontSize = 8;
	szAttr = pFont->Attribute("color");
	if (szAttr)
		cf.cfColor = CXmlNodeTranslate::FontStringToColor(szAttr);
	szAttr = pFont->Attribute("bold");
	if (szAttr && ::stricmp(szAttr, "true") == 0)
		cf.nFontStyle |= CFE_BOLD;
	szAttr = pFont->Attribute("strikeout");
	if (szAttr && ::stricmp(szAttr, "true") == 0)
		cf.nFontStyle |= CFE_STRIKEOUT;
	szAttr = pFont->Attribute("italic");
	if (szAttr && ::stricmp(szAttr, "true") == 0)
		cf.nFontStyle |= CFE_ITALIC;
	szAttr = pFont->Attribute("underline");
	if (szAttr && ::stricmp(szAttr, "true") == 0)
		cf.nFontStyle |= CFE_UNDERLINE;
}

void CXmlNodeTranslate::StringFontToStyle(IFontStyle *pFont, CCharFontStyle &cf)
{
	CInterfaceAnsiString strTmp;
	pFont->GetName((IAnsiString *)&strTmp);
	CStringConversion::StringToWideChar(strTmp.GetData(), cf.szFaceName, 31);

	cf.nFontSize = pFont->GetSize();
	cf.cfColor = pFont->GetColor();

	if (pFont->GetBold())
		cf.nFontStyle |= CFE_BOLD; 
	if (pFont->GetItalic()) 
		cf.nFontStyle |= CFE_ITALIC; 
	if (pFont->GetUnderline()) 
		cf.nFontStyle |= CFE_UNDERLINE;
}

void CXmlNodeTranslate::StyleToStringFont(const CCharFontStyle &cf, IFontStyle *pFont)
{
	pFont->SetColor(cf.cfColor);
	pFont->SetSize(cf.nFontSize);
	char szTmp[36] = {0};
	CStringConversion::WideCharToString(cf.szFaceName, szTmp, 35);
	pFont->SetName(szTmp);
	if ((cf.nFontStyle & CFE_BOLD) > 0)
		pFont->SetBold(TRUE);
	if ((cf.nFontStyle & CFE_STRIKEOUT) > 0)
		pFont->SetStrikeout(TRUE);
	if ((cf.nFontStyle & CFE_ITALIC) > 0)
		pFont->SetItalic(TRUE);
	if ((cf.nFontStyle & CFE_UNDERLINE) > 0)
		pFont->SetUnderline(TRUE); 
}

#pragma warning(default:4996)
