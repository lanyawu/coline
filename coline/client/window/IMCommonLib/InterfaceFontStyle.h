#ifndef __INTERFACEFONTSTYLE_H___
#define __INTERFACEFONTSTYLE_H___

#include <string>
#include <ComBase.h>
#include <core/coreinterface.h>

class CInterfaceFontStyle :public CComBase<>, 
	                       public InterfaceImpl<IFontStyle>
{
public:
	CInterfaceFontStyle(void);
	~CInterfaceFontStyle(void);
public:
	//IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppv);
	//set
	STDMETHOD (SetName)(const char *szFontName);
	STDMETHOD (SetSize)(const int nFontSize);
	STDMETHOD (SetColor)(const int nColor);
	STDMETHOD (SetBold)(const BOOL bBold);
	STDMETHOD (SetItalic)(const BOOL bItalic);
	STDMETHOD (SetUnderline)(const BOOL bUnderline);
	STDMETHOD (SetStrikeout)(const BOOL bStrikeout);

	//get
	STDMETHOD (GetName)(IAnsiString *strName);
	STDMETHOD_ (int, GetSize)();
	STDMETHOD_ (int, GetColor)();
	STDMETHOD_ (BOOL, GetBold)();
	STDMETHOD_ (BOOL, GetItalic)();
	STDMETHOD_ (BOOL, GetUnderline)();
	STDMETHOD_ (BOOL, GetStrikeout)();

	//
	void CopyTo(IFontStyle *pStyle);
private:
	std::string m_strName;
	int m_nFontSize;
	int m_nColor;
	BOOL m_bBold;
	BOOL m_bItalic;
	BOOL m_bUnderline;
	BOOL m_bStrikeout;
};

#endif
