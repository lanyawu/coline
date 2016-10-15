#include "InterfaceFontStyle.h"


CInterfaceFontStyle::CInterfaceFontStyle(void):
                     m_strName("tahoma"),
				     m_nFontSize(8),
					 m_nColor(0),
					 m_bBold(FALSE),
					 m_bItalic(FALSE),
					 m_bUnderline(FALSE),
					 m_bStrikeout(FALSE)
{ 
	//
}


CInterfaceFontStyle::~CInterfaceFontStyle(void)
{
	//
}

//IUnknown
STDMETHODIMP CInterfaceFontStyle::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IFontStyle)))
	{
		*ppv = (IFontStyle *) this;
		_AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

void CInterfaceFontStyle::CopyTo(IFontStyle *pStyle)
{
	if (pStyle)
	{
		pStyle->SetName(m_strName.c_str());
		pStyle->SetSize(m_nFontSize);
		pStyle->SetColor(m_nColor);
		pStyle->SetBold(m_bBold);
		pStyle->SetItalic(m_bItalic);
		pStyle->SetUnderline(m_bUnderline);
	}
}

//set
STDMETHODIMP CInterfaceFontStyle::SetName(const char *szFontName)
{
	if (szFontName)
		m_strName = szFontName;
	else
		m_strName = "tahoma";
	return S_OK;
}

STDMETHODIMP CInterfaceFontStyle::SetSize(const int nFontSize)
{
	if (nFontSize > 0)
		m_nFontSize = nFontSize;
	else
		m_nFontSize = 8;
	return S_OK;
}

STDMETHODIMP CInterfaceFontStyle::SetColor(const int nColor)
{
	m_nColor = nColor;
	return S_OK;
}

STDMETHODIMP CInterfaceFontStyle::SetBold(const BOOL bBold)
{
	m_bBold = bBold;
	return S_OK;
}

STDMETHODIMP CInterfaceFontStyle::SetItalic(const BOOL bItalic)
{
	m_bItalic = bItalic;
	return S_OK;
}

STDMETHODIMP CInterfaceFontStyle::SetUnderline(const BOOL bUnderline)
{
	m_bUnderline = bUnderline;
	return S_OK;
}

STDMETHODIMP CInterfaceFontStyle::SetStrikeout(const BOOL bStrikeout)
{
	m_bStrikeout = bStrikeout;
	return S_OK;
}


//get
STDMETHODIMP CInterfaceFontStyle::GetName(IAnsiString *strName)
{
	if (strName)
	{
		strName->SetString(m_strName.c_str());
		return S_OK;
	}
	return E_FAIL;

}

STDMETHODIMP_(int) CInterfaceFontStyle::GetSize()
{
	return m_nFontSize;
}

STDMETHODIMP_(int) CInterfaceFontStyle::GetColor()
{
	return m_nColor;
}

STDMETHODIMP_(BOOL) CInterfaceFontStyle::GetBold()
{
	return m_bBold; 
}

STDMETHODIMP_(BOOL) CInterfaceFontStyle::GetItalic()
{
	return m_bItalic; 
}

STDMETHODIMP_(BOOL) CInterfaceFontStyle::GetUnderline()
{
	return m_bUnderline;
}

STDMETHODIMP_(BOOL) CInterfaceFontStyle::GetStrikeout()
{
	return m_bStrikeout;
}
