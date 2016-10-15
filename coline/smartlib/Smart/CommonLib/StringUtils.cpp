#include <Commonlib/StringUtils.h>

#pragma warning(disable:4996)

int CStringConversion::WideCharToString(const TCHAR *lpstrSrc, char *szDesc, int iMaxLen)
{
	if (lpstrSrc && szDesc)
		return  WideCharToMultiByte(::GetACP(), 0, lpstrSrc, -1, szDesc, iMaxLen, NULL, NULL);
	return FALSE;
}

int CStringConversion::StringToWideChar(const char *szSrc, TCHAR *lpstrDest, int iMaxLen)
{
	if (szSrc && lpstrDest)
		return MultiByteToWideChar(::GetACP(), 0, szSrc, -1, lpstrDest, iMaxLen);
	return FALSE;
}

int CStringConversion::StringToUTF8(const char *szSrc, char *szDest, int iMaxLen)
{
	//先转UNICODE
	if (szSrc && szDest)
	{
		TCHAR *szTemp = new TCHAR[iMaxLen];
		memset(szTemp, 0, sizeof(TCHAR) * iMaxLen);
		StringToWideChar(szSrc, szTemp, iMaxLen);
		//转UTF8
		return ::WideCharToMultiByte(CP_UTF8, 0, szTemp, (int)::wcslen(szTemp), szDest, iMaxLen, NULL, NULL);
	}
	return FALSE;
}

int CStringConversion::UTF8ToWideChar(const char *szSrc, TCHAR *lpstrDest, int iMaxLen)
{
	if (szSrc && lpstrDest)
		return ::MultiByteToWideChar(CP_UTF8, 0, szSrc, (int) ::strlen(szSrc), lpstrDest, iMaxLen);
	return FALSE;
}

int CStringConversion::WideCharToUTF8(const TCHAR *lpstrSrc, char *szDest, int iMaxLen)
{
	if (lpstrSrc && szDest)
		return ::WideCharToMultiByte(CP_UTF8, 0, lpstrSrc, (int)::wcslen(lpstrSrc), szDest, iMaxLen, NULL, NULL);
	return FALSE;
}

//比较二进制流是否相等
BOOL CStringConversion::CompareMem(const void *pSrc, const DWORD dwSrcSize, const void *pDest,
				const DWORD dwDestSize)
{
	if (dwSrcSize != dwDestSize)
		return FALSE;
	const char *p1 = (char *)pSrc;
	const char *p2 = (char *)pDest;
	for (DWORD i = 0; i < dwSrcSize; i ++)
	{
		if ((*p1) != (*p2))
			return FALSE;
		p1 ++;
		p2 ++;
	}
	return TRUE;
}

int CStringConversion::UTF8ToString(const char* szSrc, char* szDest, int iMaxLen)
{
	int nLen = (int)strlen(szSrc);
	TCHAR *szTemp = new TCHAR[nLen + 1];
	memset(szTemp, 0, sizeof(TCHAR) * (nLen + 1));
	::MultiByteToWideChar(CP_UTF8, 0, szSrc, (int) ::strlen(szSrc), szTemp, nLen);
	int iRes = WideCharToString(szTemp, szDest, iMaxLen);
	delete [] szTemp;
	return iRes;
}

//去除空格
char *CStringConversion::LeftTrim(const char *szSrc, char *szDest)
{
	if ((!szSrc) || (!szDest))
		return NULL;
	const char *p = szSrc;
	while ((*p) && ((*p) == ' '))
		p ++;
	if (*p)
		strcpy(szDest, p);
	return szDest;
}

//去除右边空格
char *CStringConversion::RightTrim(const char *szSrc, char *szDest)
{
	if ((!szSrc) || (!szDest))
		return NULL;
	size_t dwSize = ::strlen(szSrc);
    const char *p = (szSrc + dwSize - 1);
	while ((p != szSrc) && ((*p) == ' '))
		p --;
	dwSize = p - szSrc + 1;
	if (dwSize > 0)
		strncpy(szDest, szSrc, dwSize);
	return szDest;
}

//去除两边空格
char *CStringConversion::Trim(const char *szSrc, char *szDest)
{
	if ((!szSrc) || (!szDest))
		return NULL;
	int dwSize = (int) ::strlen(szSrc);
	const char *p1 = szSrc;
	const char *p2 = (szSrc + dwSize - 1);
	while ((*p1) && ((*p1) == ' '))
		p1 ++;
	while((p2 != szSrc) && ((*p2) == ' '))
		p2 --;
	dwSize = p2 - p1 + 1;
	if (dwSize > 0)
		strncpy(szDest, p1, dwSize);
	return szDest;
}

//分隔字符
char *CStringConversion::GetStringBySep(char *szString, char *szDest, char c)
{
	if (!szString)
		return NULL;
	char *p = szString;
	while ((*p) && ((*p) != c))
		p ++;
	if ((p != szString) && (szDest))
	{
		strncpy(szDest, szString, p - szString);	
	}
	if (*p)
	{
		p ++;
		return p;
	}else 
		return NULL;
}

//替换
std::string CStringConversion::StringReplace(std::string &strSrc, const char *szOldSrc, const char *szNewStr, BOOL bIsAll)
{	
	int nNewLen = (int) ::strlen(szNewStr);
	int nOldLen = (int) ::strlen(szOldSrc);
	if (nOldLen > 0)
	{
		for(std::string::size_type  pos = 0; pos != std::string::npos; pos += nNewLen)   
		{  
			pos = strSrc.find(szOldSrc, pos);
			if(pos !=string::npos)
				strSrc.replace(pos, nOldLen, szNewStr);
			else
				break;
		}
	}
	return strSrc;
}

CStdString_::CStdString_() :
            m_szBuffer(NULL),
			m_dwStrLen(0)
{

}

CStdString_::CStdString_(const TCHAR ch):
            m_dwStrLen(0)
{
	m_dwStrLen = 1;
	m_szBuffer = new TCHAR[m_dwStrLen + 1];
	memset(m_szBuffer, 0, sizeof(TCHAR) * (m_dwStrLen + 1));
	memmove(m_szBuffer, &ch, sizeof(TCHAR));
}

CStdString_::CStdString_(LPCTSTR lpsz, int nLen):
            m_dwStrLen(0),
			m_szBuffer(NULL)
{  
	if ((!(IsBadStringPtr(lpsz, -1))) && (lpsz))
	{
		if (nLen > 0)
			m_dwStrLen = nLen;
		else
			m_dwStrLen = (DWORD)_tcslen(lpsz);
		m_szBuffer = new TCHAR[m_dwStrLen + 1];
		memset(m_szBuffer, 0, sizeof(TCHAR) * (m_dwStrLen + 1));
		memmove(m_szBuffer, lpsz, sizeof(TCHAR) * m_dwStrLen);
	}
}

CStdString_::CStdString_(const CStdString_& src):
            m_dwStrLen(0),
			m_szBuffer(NULL)
{
	if (&src != this)
	{
		m_dwStrLen = src.m_dwStrLen;
		m_szBuffer = new TCHAR[m_dwStrLen + 1];
		memset(m_szBuffer, 0, sizeof(TCHAR) * (m_dwStrLen + 1));
		memmove(m_szBuffer, src.m_szBuffer, m_dwStrLen * sizeof(TCHAR));
	}
}

CStdString_::~CStdString_()
{
	if (m_szBuffer)
		delete []m_szBuffer;
	m_szBuffer = NULL;
	m_dwStrLen = 0;
}


int CStdString_::GetLength() const
{ 
   return (int) m_dwStrLen;
}

CStdString_::operator LPCTSTR() const 
{ 
   return m_szBuffer; 
}

void CStdString_::Append(LPCTSTR pstr)
{
	if (!IsBadStringPtr(pstr, -1))
	{
	   int nNewLength = GetLength() + (int) _tcslen(pstr);
	  
	   TCHAR *szTemp = new TCHAR[nNewLength + 1];
	   memset(szTemp, 0, sizeof(TCHAR) * (nNewLength + 1));
	   if ((m_dwStrLen > 0) && (m_szBuffer))
		   ::lstrcpy(szTemp, m_szBuffer);
	   ::lstrcat(szTemp, pstr);
	   m_dwStrLen = nNewLength;
	   if (m_szBuffer)
		   delete []m_szBuffer;
	   m_szBuffer = szTemp;
	}
}

void CStdString_::Assign(LPCTSTR pstr, int cchMax)
{
   if( pstr == NULL ) pstr = _T("");
   if (m_szBuffer)
	   delete []m_szBuffer;
   m_szBuffer = NULL;
   m_dwStrLen = 0;
   Append(pstr);
}

bool CStdString_::IsEmpty() const 
{ 
   return m_dwStrLen == 0; 
}

void CStdString_::Empty() 
{ 
	if (m_szBuffer)
		delete []m_szBuffer;
	m_szBuffer = NULL;
	m_dwStrLen = 0;
}

LPCTSTR CStdString_::GetData()
{
   return m_szBuffer;
}

TCHAR CStdString_::GetAt(int nIndex) const
{
	if ((nIndex >= 0) && (nIndex < (int)m_dwStrLen))
        return m_szBuffer[nIndex];
	else
		return _T(' ');
}

TCHAR CStdString_::operator[] (int nIndex) const
{ 
   return GetAt(nIndex);
}   

const CStdString_& CStdString_::operator=(const CStdString_& src)
{      
   Assign(src);
   return *this;
}

const CStdString_& CStdString_::operator=(LPCTSTR lpStr)
{      
   Assign(lpStr);
   return *this;
}

#ifndef _UNICODE

const CStdString_& CStdString_::operator=(LPCWSTR lpwStr)
{      
	if (!::IsBadStringPtrW(lpwStr,-1))
	{
	   int cchStr = ((int) wcslen(lpwStr) * 2) + 1;
	   LPSTR pstr = (LPSTR) _alloca(cchStr);
	   if( pstr != NULL ) ::WideCharToMultiByte(::GetACP(), 0, lpwStr, -1, pstr, cchStr, NULL, NULL);
	   Assign(pstr);
	   return *this;
	}
}

#endif // _UNICODE

const CStdString_& CStdString_::operator=(const TCHAR ch)
{
    Empty();
	m_dwStrLen = 1;
	m_szBuffer = new TCHAR[m_dwStrLen + 1];
	memset(m_szBuffer, 0, sizeof(TCHAR) * (m_dwStrLen + 1));
	memmove(m_szBuffer, &ch, sizeof(TCHAR));   
    return *this;
}

CStdString_ CStdString_::operator+(const CStdString_& src)
{
   Append(src);
   return *this;
}

CStdString_ CStdString_::operator+(LPCTSTR lpStr)
{
   Append(lpStr);
   return *this;
}

const CStdString_& CStdString_::operator+=(const CStdString_& src)
{      
   Append(src);
   return *this;
}

const CStdString_& CStdString_::operator+=(LPCTSTR lpStr)
{      
   Append(lpStr);
   return *this;
}

const CStdString_& CStdString_::operator+=(const TCHAR ch)
{      
   TCHAR str[] = { ch, '\0' };
   Append(str);
   return *this;
}

bool CStdString_::operator == (LPCTSTR str) const { return (Compare(str) == 0); };
bool CStdString_::operator != (LPCTSTR str) const { return (Compare(str) != 0); };
bool CStdString_::operator <= (LPCTSTR str) const { return (Compare(str) <= 0); };
bool CStdString_::operator <  (LPCTSTR str) const { return (Compare(str) <  0); };
bool CStdString_::operator >= (LPCTSTR str) const { return (Compare(str) >= 0); };
bool CStdString_::operator >  (LPCTSTR str) const { return (Compare(str) >  0); };

void CStdString_::SetAt(int nIndex, TCHAR ch)
{
	if ((nIndex>=0) && (nIndex < GetLength()))
	   m_szBuffer[nIndex] = ch;
}

int CStdString_::Compare(LPCTSTR lpsz) const 
{ 
	return _tcscmp(m_szBuffer, lpsz); 
}

int CStdString_::CompareNoCase(LPCTSTR lpsz) const 
{ 
	return _tcsicmp(m_szBuffer, lpsz); 
}

void CStdString_::MakeUpper() 
{ 
	_tcsupr(m_szBuffer); 
}

void CStdString_::MakeLower() 
{ 
	_tcslwr(m_szBuffer); 
}

CStdString_ CStdString_::Left(int iLength) const
{
   if( iLength < 0 ) iLength = 0;
   if( iLength > GetLength() ) iLength = GetLength();
   return CStdString_(m_szBuffer, iLength);
}

CStdString_ CStdString_::Mid(int iPos, int iLength) const
{
   if( iLength < 0 ) iLength = GetLength() - iPos;
   if( iPos + iLength > GetLength() ) iLength = GetLength() - iPos;
   if( iLength <= 0 ) return CStdString_();
   return CStdString_(m_szBuffer + iPos, iLength);
}

CStdString_ CStdString_::Right(int iLength) const
{
   int iPos = GetLength() - iLength;
   if( iPos < 0 ) {
      iPos = 0;
      iLength = GetLength();
   }
   return CStdString_(m_szBuffer + iPos, iLength);
}

int CStdString_::Find(TCHAR ch, int iPos /*= 0*/) const
{
   if ((iPos >= 0) && (iPos <= GetLength()))
   {
	   if ((iPos != 0) && ((iPos < 0) || (iPos >= GetLength()))) 
		   return -1;
	   LPCTSTR p = _tcschr(m_szBuffer + iPos, ch);
	   if ( p == NULL ) 
		   return -1;
	   return (int)(p - m_szBuffer);
   } else
	   return -1;
}

int CStdString_::Find(LPCTSTR pstrSub, int iPos /*= 0*/) const
{
   if (!IsBadStringPtr(pstrSub, -1))
   {
	   if ((iPos != 0) && ((iPos < 0) || (iPos >= GetLength()))) 
		   return -1;
	   LPCTSTR p = _tcsstr(m_szBuffer + iPos, pstrSub);
	   if ( p == NULL ) 
		   return -1;
	   return (int)(p - m_szBuffer);
   } else
	   return -1;
}

int CStdString_::ReverseFind(TCHAR ch) const
{
   LPCTSTR p = _tcsrchr(m_szBuffer, ch);
   if( p == NULL ) 
	   return -1;
   return (int)(p - m_szBuffer);
}

int CStdString_::Replace(LPCTSTR pstrFrom, LPCTSTR pstrTo)
{
   CStdString_ sTemp;
   int nCount = 0;
   int iPos = Find(pstrFrom);
   if( iPos < 0 ) return 0;
   int cchFrom = (int) _tcslen(pstrFrom);
   int cchTo = (int) _tcslen(pstrTo);
   while( iPos >= 0 ) {
      sTemp = Left(iPos);
      sTemp += pstrTo;
      sTemp += Mid(iPos + cchFrom);
      Assign(sTemp);
      iPos = Find(pstrFrom, iPos + cchTo);
      nCount++;
   }
   return nCount;
}

int CStdString_::Format(LPCTSTR pstrFormat, ...)
{
   CStdString_ sFormat = pstrFormat;

   TCHAR szBuffer[1025] = { 0 };
   va_list argList;
   va_start(argList, pstrFormat);
   int iRet = ::wvsprintf(szBuffer, sFormat, argList);
   va_end(argList);
   Assign(szBuffer);
   return iRet;
}

//  ============ class CAnsiString_ ==================
CAnsiString_::CAnsiString_() :
              m_szBuffer(NULL),
			  m_dwStrLen(0)
{

}

CAnsiString_::CAnsiString_(const char ch):
              m_dwStrLen(0)
{
	m_dwStrLen = 1;
	m_szBuffer = new char[m_dwStrLen + 1];
	memset(m_szBuffer, 0, sizeof(char) * (m_dwStrLen + 1));
	memmove(m_szBuffer, &ch, sizeof(char));
}

CAnsiString_::CAnsiString_(const char *lpsz, int nLen):
              m_dwStrLen(0),
		  	  m_szBuffer(NULL)
{  
	if ((!(IsBadStringPtrA(lpsz, -1))) && (lpsz))
	{
		if (nLen > 0)
			m_dwStrLen = nLen;
		else
			m_dwStrLen = (DWORD)::strlen(lpsz);
		m_szBuffer = new char[m_dwStrLen + 1];
		memset(m_szBuffer, 0, sizeof(char) * (m_dwStrLen + 1));
		memmove(m_szBuffer, lpsz, sizeof(char) * m_dwStrLen);
	}
}

CAnsiString_::CAnsiString_(const CAnsiString_& src):
              m_dwStrLen(0),
			  m_szBuffer(NULL)
{
	if (&src != this)
	{
		m_dwStrLen = src.m_dwStrLen;
		m_szBuffer = new char[m_dwStrLen + 1];
		memset(m_szBuffer, 0, sizeof(char) * (m_dwStrLen + 1));
		memmove(m_szBuffer, src.m_szBuffer, m_dwStrLen * sizeof(char));
	}
}

CAnsiString_::~CAnsiString_()
{
	if (m_szBuffer)
		delete []m_szBuffer;
	m_szBuffer = NULL;
	m_dwStrLen = 0;
}


int CAnsiString_::GetLength() const
{ 
   return (int) m_dwStrLen;
}

CAnsiString_::operator const char *() const 
{ 
   return m_szBuffer; 
}

void CAnsiString_::Append(const char * pstr)
{
	if (!IsBadStringPtrA(pstr, -1))
	{
	   int nNewLength = GetLength() + (int) strlen(pstr);
	  
	   char *szTemp = new char[nNewLength + 1];
	   memset(szTemp, 0, sizeof(char) * (nNewLength + 1));
	   if ((m_dwStrLen > 0) && (m_szBuffer))
		   memmove(szTemp, m_szBuffer, m_dwStrLen * sizeof(char));
	   memmove(szTemp + sizeof(char) * m_dwStrLen, pstr, strlen(pstr));
	   m_dwStrLen = nNewLength;
	   if (m_szBuffer)
		   delete []m_szBuffer;
	   m_szBuffer = szTemp;
	}
}

void CAnsiString_::Assign(const char *pstr, int cchMax)
{
   if (m_szBuffer)
	   delete []m_szBuffer;
   m_szBuffer = NULL;
   m_dwStrLen = 0;
   if (pstr == NULL)
	   Append("");
   else
	   Append(pstr);
}

bool CAnsiString_::IsEmpty() const 
{ 
   return m_dwStrLen == 0; 
}

void CAnsiString_::Empty() 
{ 
	if (m_szBuffer)
		delete []m_szBuffer;
	m_szBuffer = NULL;
	m_dwStrLen = 0;
}

const char *CAnsiString_::c_str() const
{
   return m_szBuffer;
}

char CAnsiString_::GetAt(int nIndex) const
{
	if ((nIndex >= 0) && (nIndex < (int)m_dwStrLen))
        return m_szBuffer[nIndex];
	else
		return _T(' ');
}

char CAnsiString_::operator[] (int nIndex) const
{ 
   return GetAt(nIndex);
}   

const CAnsiString_& CAnsiString_::operator=(const CAnsiString_& src)
{      
   Assign(src);
   return *this;
}

const CAnsiString_& CAnsiString_::operator=(const char *lpStr)
{      
   Assign(lpStr);
   return *this;
}


const CAnsiString_& CAnsiString_::operator=(const char ch)
{
    Empty();
	m_dwStrLen = 1;
	m_szBuffer = new char[m_dwStrLen + 1];
	memset(m_szBuffer, 0, sizeof(char) * (m_dwStrLen + 1));
	memmove(m_szBuffer, &ch, sizeof(char));   
    return *this;
}

CAnsiString_ CAnsiString_::operator+(const CAnsiString_& src)
{
   Append(src);
   return *this;
}

CAnsiString_ CAnsiString_::operator+(const char * lpStr)
{
   Append(lpStr);
   return *this;
}

const CAnsiString_& CAnsiString_::operator+=(const CAnsiString_& src)
{      
   Append(src);
   return *this;
}

const CAnsiString_& CAnsiString_::operator+=(const char *lpStr)
{      
   Append(lpStr);
   return *this;
}

const CAnsiString_& CAnsiString_::operator+=(const char ch)
{      
   char str[] = {ch, '\0' };
   Append(str);
   return *this;
}

bool CAnsiString_::operator == (const char *str) const { return (CompareNoCase(str) == 0); };
bool CAnsiString_::operator != (const char * str) const { return (CompareNoCase(str) != 0); };
bool CAnsiString_::operator <= (const char * str) const { return (CompareNoCase(str) <= 0); };
bool CAnsiString_::operator <  (const char * str) const { return (CompareNoCase(str) <  0); };
bool CAnsiString_::operator >= (const char * str) const { return (CompareNoCase(str) >= 0); };
bool CAnsiString_::operator >  (const char * str) const { return (CompareNoCase(str) >  0); };

bool CAnsiString_::operator == (const CAnsiString_ &str) const { return (CompareNoCase(str.c_str()) == 0); };
bool CAnsiString_::operator != (const CAnsiString_ &str) const { return (CompareNoCase(str.c_str()) != 0); };
bool CAnsiString_::operator <= (const CAnsiString_ &str) const { return (CompareNoCase(str.c_str()) <= 0); };
bool CAnsiString_::operator <  (const CAnsiString_ &str) const { return (CompareNoCase(str.c_str()) < 0); };
bool CAnsiString_::operator >= (const CAnsiString_ &str) const { return (CompareNoCase(str.c_str()) >= 0); };
bool CAnsiString_::operator >  (const CAnsiString_ &str) const { return (CompareNoCase(str.c_str()) > 0); };

void CAnsiString_::SetAt(int nIndex, char ch)
{
	if ((nIndex>=0) && (nIndex < GetLength()))
	   m_szBuffer[nIndex] = ch;
}

int CAnsiString_::Compare(const char *lpsz) const 
{ 
	return strcmp(m_szBuffer, lpsz); 
}

int CAnsiString_::CompareNoCase(const char *lpsz) const 
{ 
	return ::stricmp(m_szBuffer, lpsz); 
}

void CAnsiString_::MakeUpper() 
{ 
	::_strupr(m_szBuffer); 
}

void CAnsiString_::MakeLower() 
{ 
	::_strlwr(m_szBuffer); 
}

CAnsiString_ CAnsiString_::Left(int iLength) const
{
   if(iLength < 0) 
	   iLength = 0;
   if (iLength > GetLength()) 
	   iLength = GetLength();
   return CAnsiString_(m_szBuffer, iLength);
}

CAnsiString_ CAnsiString_::Mid(int iPos, int iLength) const
{
   if ( iLength < 0 ) 
	   iLength = GetLength() - iPos;
   if ( iPos + iLength > GetLength() ) 
	   iLength = GetLength() - iPos;
   if( iLength <= 0 )
	   return CAnsiString_();
   return CAnsiString_(m_szBuffer + iPos, iLength);
}

CAnsiString_ CAnsiString_::Right(int iLength) const
{
   int iPos = GetLength() - iLength;
   if( iPos < 0 ) {
      iPos = 0;
      iLength = GetLength();
   }
   return CAnsiString_(m_szBuffer + iPos, iLength);
}

int CAnsiString_::Find(char ch, int iPos /*= 0*/) const
{
   if ((iPos >= 0) && (iPos <= GetLength()))
   {
	   if ((iPos != 0) && ((iPos < 0) || (iPos >= GetLength()))) 
		   return -1;
	   const char  *p = strchr(m_szBuffer + iPos, ch);
	   if ( p == NULL ) 
		   return -1;
	   return (int)(p - m_szBuffer);
   } else
	   return -1;
}

int CAnsiString_::Find(const char *pstrSub, int iPos /*= 0*/) const
{
   if (!IsBadStringPtrA(pstrSub, -1))
   {
	   if ((iPos != 0) && ((iPos < 0) || (iPos >= GetLength()))) 
		   return -1;
	   const char *p = ::strstr(m_szBuffer + iPos, pstrSub);
	   if ( p == NULL ) 
		   return -1;
	   return (int)(p - m_szBuffer);
   } else
	   return -1;
}

int CAnsiString_::ReverseFind(char ch) const
{
	const char *p = ::strchr(m_szBuffer, ch);
    if( p == NULL ) 
	    return -1;
   return (int)(p - m_szBuffer);
}

int CAnsiString_::Replace(const char *pstrFrom, const char *pstrTo)
{
   CAnsiString_ sTemp;
   int nCount = 0;
   int iPos = Find(pstrFrom);
   if( iPos < 0 ) return 0;
   int cchFrom = (int) ::strlen(pstrFrom);
   int cchTo = (int) ::strlen(pstrTo);
   while (iPos >= 0) 
   {
      sTemp = Left(iPos);
      sTemp += pstrTo;
      sTemp += Mid(iPos + cchFrom);
      Assign(sTemp);
      iPos = Find(pstrFrom, iPos + cchTo);
      nCount++;
   }
   return nCount;
}

int CAnsiString_::Format(const char *pstrFormat, ...)
{
   CAnsiString_ sFormat = pstrFormat;

   char szBuffer[1024] = { 0 };
   va_list argList;
   va_start(argList, pstrFormat);
   int iRet = ::vsnprintf(szBuffer, 1023, sFormat, argList);
   va_end(argList);
   Assign(szBuffer);
   return iRet;
}
// ================= class CAnsiString end ===============

//  ===========  CStdStringList =====================
CStdStringList::CStdStringList()
{
}

CStdStringList::~CStdStringList()
{
	Clear();
}

void CStdStringList::Add(CStdString_ &szStr)
{
	CStdString_ *szTemp = new CStdString_(szStr);
	m_strList.push_back(szTemp);
}

void CStdStringList::Add(LPCTSTR lpszStr)
{
	CStdString_ *szTemp = new CStdString_(lpszStr);
	m_strList.push_back(szTemp);
}

void CStdStringList::Delete(int idx)
{
	if ((idx >= 0) && (idx < (int)m_strList.size()))
	{
		int i = 0;
		vector<CStdString_ *>::iterator it;
		for (it = m_strList.begin(); it != m_strList.end(); it ++)
		{
			if (i == idx)
			{
				delete (*it);
				m_strList.erase(it);
				break;
			}
			i ++;
		}
	}
}

void CStdStringList::Clear()
{
	vector<CStdString_ *>::iterator it;
	for (it = m_strList.begin(); it != m_strList.end(); it ++)
	{
		delete (*it);
	}
	m_strList.clear();
}

int CStdStringList::GetCount()
{
	return (int)(m_strList.size());
}

int CStdStringList::Find(CStdString_ &szStr)
{
	int i = 0;
	vector<CStdString_ *>::iterator it;
	for (it = m_strList.begin(); it != m_strList.end(); it ++)
	{
		if ((*(*it)) == szStr)
		{
			return i;
		}
		i ++;
	}
	return -1;
}

int CStdStringList::Find(LPCTSTR lpszStr)
{
	int i = 0;
	vector<CStdString_ *>::iterator it;
	for (it = m_strList.begin(); it != m_strList.end(); it ++)
	{
		if ((*(*it)) == lpszStr)
		{
			return i;
		}
		i ++;
	}
	return -1;
}

CStdString_ &CStdStringList::operator[](int idx) 
{
	if ((idx >= 0) && (idx < (int)m_strList.size()))
	{
		int i = 0;
		vector<CStdString_ *>::iterator it;
		for (it = m_strList.begin(); it != m_strList.end(); it ++)
		{
			if (i == idx)
				return (*(*it));
			i ++;
		}
		return EMPTY_STDSTRING_DECLARE;
	} else
		return EMPTY_STDSTRING_DECLARE;
}


#pragma warning(default:4996)