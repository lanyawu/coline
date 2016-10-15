#ifndef __XMLNODETRANSLATE_H____
#define __XMLNODETRANSLATE_H____

#include <string>
#include <xml/tinyxml.h>
#include <SmartSkin/smartskin.h>
#include <Core/CoreInterface.h>

class CXmlNodeTranslate
{
public:
	static int FontStringToSize(const char *szStr);
	static void FontSizeToString(int nSize, std::string &str);
	static int FontStringToColor(const char *szStr);
	static void FontColorToString(int nClr, std::string &str);
	static void FontStyleToXmlNode(const CCharFontStyle &cf, TiXmlElement *pFont);
	static void FontXmlNodeToStyle(TiXmlElement *pFont, CCharFontStyle &cf);
	static void StringFontToStyle(IFontStyle *pFont, CCharFontStyle &cf);
	static void StyleToStringFont(const CCharFontStyle &cf, IFontStyle *pFont);
};


#endif;
