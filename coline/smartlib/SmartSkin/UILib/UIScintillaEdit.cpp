#include <Commonlib/stringutils.h>
#include <UILib/UIScintillaEdit.h>
#include <scintilla/Scintilla.h>
#include <scintilla/SciLexer.h>

HMODULE CScintillaEditUI::m_hScEditModule = NULL;
volatile LONG CScintillaEditUI::m_hScEditRef = 0;

CScintillaEditUI::CScintillaEditUI(void):
                  m_hEdit(NULL)
{

}


CScintillaEditUI::~CScintillaEditUI(void)
{
   if (::InterlockedDecrement(&m_hScEditRef) == 0)
   {
	   ::FreeLibrary(m_hScEditModule);
	   m_hScEditModule = NULL;
	   ::CoUninitialize();
   }
   if (m_hEdit)
   {
	   if (::IsWindow(m_hEdit))
	   {
		   ::DestroyWindow(m_hEdit);
		   m_hEdit = NULL;
	   }
   }
}

LPCTSTR CScintillaEditUI::GetClass() const
{
	return _T("SCINTILLAEDITUI");
}

UINT CScintillaEditUI::GetControlFlags() const
{
	return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
}

void CScintillaEditUI::Init()
{
	CContainerUI::Init();
	if (!CScintillaEditUI::m_hScEditModule)
    {
	   ::CoInitialize(NULL);
	   CScintillaEditUI::m_hScEditModule = ::LoadLibrary(L"scintilla.dll");
	   if (CScintillaEditUI::m_hScEditModule <= (HMODULE)HINSTANCE_ERROR)
		   CScintillaEditUI::m_hScEditModule = (HMODULE)0;
    }
	::InterlockedIncrement(&m_hScEditRef);
	CreateScintillaEdit();
}

BOOL CScintillaEditUI::CreateScintillaEdit()
{
	if (m_hEdit)
	{
		return ::IsWindow(m_hEdit);
	} else
	{
		m_hEdit = CreateWindowEx(0, L"Scintilla", L"", 
										WS_CHILD | WS_VISIBLE | WS_TABSTOP | 
										WS_CLIPCHILDREN,
										10, 10, 500, 400,
										GetManager()->GetPaintWindow(), NULL /*(HMENU)GuiID*/, 
										GetManager()->GetResourceInstance(), NULL);
		if (::IsWindow(m_hEdit))
		{
			// Set number of style bits to use
			::SendMessage(m_hEdit, SCI_SETSTYLEBITS, 5, 0L );

			// Set tab width
			::SendMessage(m_hEdit, SCI_SETTABWIDTH, 4, 0L );

			// Set foreground color
			::SendMessage(m_hEdit, SCI_STYLESETFORE, STYLE_DEFAULT, (LPARAM)RGB( 255, 255, 255 ) );

			// Set background color
			::SendMessage(m_hEdit, SCI_STYLESETBACK, STYLE_DEFAULT, (LPARAM)RGB(0, 0, 0 ) );

			::SendMessage(m_hEdit, SCI_STYLESETSIZE, STYLE_DEFAULT, 12);
			// Set font
			::SendMessage(m_hEdit, SCI_STYLESETFONT, STYLE_DEFAULT, (LPARAM)L"Courier New" );

			// Set selection color
			::SendMessage(m_hEdit, SCI_SETSELBACK, (WPARAM)TRUE, (LPARAM)RGB(0, 0, 255 ) );
			
			// Set all styles
			::SendMessage(m_hEdit, SCI_STYLECLEARALL, 0, 0L );
			//set cache attr
			std::vector<LPATTR_CACHE_BUFFER>::iterator it;
			for (it = m_AttrList.begin(); it != m_AttrList.end(); it ++)
			{
				SetAttribute((*it)->szName, (*it)->szValue);
				delete [](*it)->szName;
				delete [](*it)->szValue;
				delete (*it);
			}
			m_AttrList.clear();
			return TRUE;
		}
		return FALSE;
	}
}

void  CScintillaEditUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (m_hEdit)
	{
		if (_tcscmp(pstrName, _T("keyword")) == 0)
		{
			SetKeyWord(pstrValue);
		} else if (_tcscmp(pstrName, _T("backcolor")) == 0)
		{
			::SendMessage(m_hEdit, SCI_STYLESETBACK, STYLE_DEFAULT, StringToColor(pstrValue));
		} else if (_tcscmp(pstrName, _T("forecolor")) == 0)
		{
			::SendMessage(m_hEdit, SCI_STYLESETFORE, STYLE_DEFAULT, StringToColor(pstrValue));
		} else if (_tcscmp(pstrName, _T("commentcolor")) == 0)
		{
			SetWordStyle(SCE_C_COMMENT, StringToColor(pstrValue), 0, -1, NULL);
		} else if (_tcscmp(pstrName, _T("commentlinecolor")) == 0)
		{
			SetWordStyle(SCE_C_COMMENTLINE, StringToColor(pstrValue), 0, -1, NULL);
		}  else if (_tcscmp(pstrName, _T("commentdoccolor")) == 0)
		{
			SetWordStyle(SCE_C_COMMENTDOC, StringToColor(pstrValue), 0, -1, NULL);
		}  else if (_tcscmp(pstrName, _T("numbercolor")) == 0)
		{
			SetWordStyle(SCE_C_NUMBER, StringToColor(pstrValue), 0, -1, NULL);
		}  else if (_tcscmp(pstrName, _T("stringcolor")) == 0)
		{
			SetWordStyle(SCE_C_STRING, StringToColor(pstrValue), 0, -1, NULL);
		}  else if (_tcscmp(pstrName, _T("charactercolor")) == 0)
		{
			SetWordStyle(SCE_C_CHARACTER, StringToColor(pstrValue), 0, -1, NULL);
		}  else if (_tcscmp(pstrName, _T("uuidcolor")) == 0)
		{
			SetWordStyle(SCE_C_UUID, StringToColor(pstrValue), 0, -1, NULL);
		}  else if (_tcscmp(pstrName, _T("operatorcolor")) == 0)
		{
			SetWordStyle(SCE_C_OPERATOR, StringToColor(pstrValue), 0, -1, NULL);
		}  else if (_tcscmp(pstrName, _T("preprocessorcolor")) == 0)
		{
			SetWordStyle(SCE_C_PREPROCESSOR, StringToColor(pstrValue), 0, -1, NULL);
		}  else if (_tcscmp(pstrName, _T("wordcolor")) == 0)
		{
			SetWordStyle(SCE_C_WORD, StringToColor(pstrValue), 0, -1, NULL);
		} else if (_tcscmp(pstrName, _T("lexer")) == 0)
		{ 
			if (_tcscmp(pstrValue, _T("python")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_PYTHON, 0L); 
			} else if (_tcscmp(pstrValue, _T("cpp")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_CPP, 0L); 
			} else if (_tcscmp(pstrValue, _T("html")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_HTML, 0L); 
			} else if (_tcscmp(pstrValue, _T("xml")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_XML, 0L); 
			} else if (_tcscmp(pstrValue, _T("perl")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_PERL, 0L); 
			} else if (_tcscmp(pstrValue, _T("sql")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_SQL, 0L); 
			} else if (_tcscmp(pstrValue, _T("vb")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_VB, 0L); 
			} else if (_tcscmp(pstrValue, _T("makefile")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_MAKEFILE, 0L); 
			} else if (_tcscmp(pstrValue, _T("batch")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_BATCH, 0L); 
			} else if (_tcscmp(pstrValue, _T("pascal")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_PASCAL, 0L); 
			} else if (_tcscmp(pstrValue, _T("lisp")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_LISP, 0L); 
			} else if (_tcscmp(pstrValue, _T("ruby")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_RUBY, 0L); 
			} else if (_tcscmp(pstrValue, _T("tcl")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_TCL, 0L); 
			} else if (_tcscmp(pstrValue, _T("vbscript")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_VBSCRIPT, 0L); 
			} else if (_tcscmp(pstrValue, _T("matlab")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_MATLAB, 0L); 
			} else if (_tcscmp(pstrValue, _T("asm")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_ASM, 0L); 
			} else if (_tcscmp(pstrValue, _T("fortran")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_FORTRAN, 0L); 
			} else if (_tcscmp(pstrValue, _T("css")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_CSS, 0L); 
			} else if (_tcscmp(pstrValue, _T("mssql")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_MSSQL, 0L); 
			} else if (_tcscmp(pstrValue, _T("phpscript")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_PHPSCRIPT, 0L); 
			} else if (_tcscmp(pstrValue, _T("smalltalk")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_SMALLTALK, 0L); 
			} else if (_tcscmp(pstrValue, _T("innosetup")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_INNOSETUP, 0L); 
			} else if (_tcscmp(pstrValue, _T("cmake")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_CMAKE, 0L); 
			} else if (_tcscmp(pstrValue, _T("gap")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_GAP, 0L); 
			} else if (_tcscmp(pstrValue, _T("plm")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_PLM, 0L); 
			} else if (_tcscmp(pstrValue, _T("progress")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_PROGRESS, 0L); 
			} else if (_tcscmp(pstrValue, _T("abaqus")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_ABAQUS, 0L); 
			} else if (_tcscmp(pstrValue, _T("asymptote")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_ASYMPTOTE, 0L); 
			} else if (_tcscmp(pstrValue, _T("r")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_R, 0L); 
			} else if (_tcscmp(pstrValue, _T("magik")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_MAGIK, 0L); 
			} else if (_tcscmp(pstrValue, _T("powershell")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_POWERSHELL, 0L); 
			} else if (_tcscmp(pstrValue, _T("mysql")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_MYSQL, 0L); 
			} else if (_tcscmp(pstrValue, _T("po")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_PO, 0L); 
			} else if (_tcscmp(pstrValue, _T("tal")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_TAL, 0L); 
			} else if (_tcscmp(pstrValue, _T("cobol")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_COBOL, 0L); 
			} else if (_tcscmp(pstrValue, _T("tacl")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_TACL, 0L); 
			} else if (_tcscmp(pstrValue, _T("automatic")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_AUTOMATIC, 0L); 
			} else if (_tcscmp(pstrValue, _T("markdown")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_MARKDOWN, 0L); 
			} else if (_tcscmp(pstrValue, _T("sml")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_SML, 0L); 
			} else if (_tcscmp(pstrValue, _T("nimrod")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_NIMROD, 0L); 
			} else if (_tcscmp(pstrValue, _T("powerpro")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_POWERPRO, 0L); 
			} else if (_tcscmp(pstrValue, _T("sorcus")) == 0)
			{
				::SendMessage(m_hEdit, SCI_SETLEXER, SCLEX_SORCUS, 0L); 
			}
		} else
			CContainerUI::SetAttribute(pstrName, pstrValue);
	} else
	{
		LPATTR_CACHE_BUFFER pBuf = new ATTR_CACHE_BUFFER();
		int nSize = ::lstrlen(pstrName);
		pBuf->szName = new TCHAR[nSize + 1];
		memset(pBuf->szName, 0, sizeof(TCHAR) * (nSize + 1));
		::lstrcpy(pBuf->szName, pstrName);
		nSize = ::lstrlen(pstrValue);
		pBuf->szValue = new TCHAR[nSize + 1];
		memset(pBuf->szValue, 0, sizeof(TCHAR) * (nSize + 1));
		::lstrcpy(pBuf->szValue, pstrValue);
		m_AttrList.push_back(pBuf);
	}
}

void CScintillaEditUI::SetKeyWord(LPCTSTR pstrKeyWord)
{
	if (m_hEdit && ::IsWindow(m_hEdit) && pstrKeyWord)
	{
		int nSize = ::lstrlen(pstrKeyWord) * 2;
		char *szValue = new char[nSize + 1];
		memset(szValue, 0, nSize + 1);
		CStringConversion::WideCharToString(pstrKeyWord, szValue, nSize);
		::SendMessageA(m_hEdit, SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>(szValue));
		delete []szValue;
	}
}

void CScintillaEditUI::SetWordStyle(int nStyle, COLORREF clrFore, COLORREF clrBack, int nSize, LPCTSTR szFace)
{
	if (m_hEdit && ::IsWindow(m_hEdit))
	{
		::SendMessage(m_hEdit, SCI_STYLESETFORE, nStyle, clrFore);
		::SendMessage(m_hEdit, SCI_STYLESETBACK, nStyle, clrBack);
		if (nSize >= 1)
			::SendMessage(m_hEdit, SCI_STYLESETSIZE, nStyle, nSize);
		if (szFace) 
		{
			char szValue[64] = {0};
			CStringConversion::WideCharToString(szFace, szValue, 63);
			::SendMessage(m_hEdit, SCI_STYLESETFONT, nStyle, reinterpret_cast<LPARAM>(szValue));
		}
	}
}

void CScintillaEditUI::SetPos(RECT rc)
{
	CControlUI::SetPos(rc);
	if (::IsWindow(m_hEdit))
	{
		//
		::MoveWindow(m_hEdit, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
	}
}

void CScintillaEditUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	//
}

CStdString CScintillaEditUI::GetText() const
{
	return CStdString(_T(""));
}

void CScintillaEditUI::SetText(LPCTSTR pstrText)
{
	//
}

void CScintillaEditUI::SetEnabled(bool bEnabled)
{
}

void CScintillaEditUI::SetVisible(bool bVisible)
{
}

void CScintillaEditUI::SetReadOnly(BOOL bReadOnly)
{
}

void CScintillaEditUI::SetFocus()
{
}

void CScintillaEditUI::Event(TEventUI& event)
{
}

void CScintillaEditUI::Notify(TNotifyUI& msg) 
{
}

SIZE CScintillaEditUI::EstimateSize(SIZE szAvailable)
{
	return szAvailable;
}
