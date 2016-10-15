#ifndef __STRINGUTILS_H__
#define __STRINGUTILS_H__

#include <Commonlib/Types.h>
#include <vector>
#include <string>
using namespace std;

//字符处理相关类

//字符串转换类
class COMMONLIB_API CStringConversion
{
public:
	//宽字符转换成单节符
	static int WideCharToString(const TCHAR *lpstrSrc, char *szDest, int iMaxLen);
	static int StringToWideChar(const char *szSrc, TCHAR *lpstrDest, int iMaxLen);
	static int StringToUTF8(const char *szSrc, char *szDest, int iMaxLen);
	static int UTF8ToString(const char* szSrc, char* szDest, int iMaxLen);
	static int UTF8ToWideChar(const char *szSrc, TCHAR *lpstrDest, int iMaxLen);
	static int WideCharToUTF8(const TCHAR *lpstrSrc, char *szDest, int iMaxLen);
	//去除空格
	static char *LeftTrim(const char *szSrc, char *szDest);
	//去除右边空格
	static char *RightTrim(const char *szSrc, char *szDest);
	//去除两边空格
	static char *Trim(const char *szSrc, char *szDest);
	//分隔字符
	static char *GetStringBySep(char *szString, char *szDest, char c); 
    //替换
	static std::string StringReplace(std::string &strSrc, const char *szOldSrc, const char *szNewStr, BOOL bIsAll);
	//比较二进制流是否相等
	static BOOL CompareMem(const void *pSrc, const DWORD dwSrcSize, const void *pDest, const DWORD dwDestSize);
};

//字符串类
class COMMONLIB_API CStdString_
{
public:
   CStdString_();
   CStdString_(const TCHAR ch);
   CStdString_(const CStdString_& src);
   CStdString_(LPCTSTR lpsz, int nLen = -1);
   virtual ~CStdString_();


   void Empty();
   int GetLength() const;
   bool IsEmpty() const;
   TCHAR GetAt(int nIndex) const;
   void Append(LPCTSTR pstr);
   void Assign(LPCTSTR pstr, int nLength = -1);
   LPCTSTR GetData();
   
   void SetAt(int nIndex, TCHAR ch);
   operator LPCTSTR() const;

   TCHAR operator[] (int nIndex) const;
   const CStdString_& operator=(const CStdString_& src);
   const CStdString_& operator=(const TCHAR ch);
   const CStdString_& operator=(LPCTSTR pstr);
#ifndef _UNICODE
   const CStdString_& operator=(LPCWSTR lpwStr);
#endif
   CStdString_ operator+(const CStdString_& src);
   CStdString_ operator+(LPCTSTR pstr);
   const CStdString_& operator+=(const CStdString_& src);
   const CStdString_& operator+=(LPCTSTR pstr);
   const CStdString_& operator+=(const TCHAR ch);

   bool operator == (LPCTSTR str) const;
   bool operator != (LPCTSTR str) const;
   bool operator <= (LPCTSTR str) const;
   bool operator <  (LPCTSTR str) const;
   bool operator >= (LPCTSTR str) const;
   bool operator >  (LPCTSTR str) const;

   int Compare(LPCTSTR pstr) const;
   int CompareNoCase(LPCTSTR pstr) const;
   
   void MakeUpper();
   void MakeLower();

   CStdString_ Left(int nLength) const;
   CStdString_ Mid(int iPos, int nLength = -1) const;
   CStdString_ Right(int nLength) const;

   int Find(TCHAR ch, int iPos = 0) const;
   int Find(LPCTSTR pstr, int iPos = 0) const;
   int ReverseFind(TCHAR ch) const;
   int Replace(LPCTSTR pstrFrom, LPCTSTR pstrTo);
   
   int __cdecl Format(LPCTSTR pstrFormat, ...);

protected:
   DWORD m_dwStrLen; //字符串长度
   TCHAR *m_szBuffer;
};

//标准字符串类
class COMMONLIB_API CAnsiString_
{
public:
   CAnsiString_();
   CAnsiString_(const char ch);
   CAnsiString_(const CAnsiString_& src);
   CAnsiString_(const char *lpsz, int nLen = -1);
   virtual ~CAnsiString_();


   void Empty();
   int GetLength() const;
   bool IsEmpty() const;
   char GetAt(int nIndex) const;
   void Append(const char *pstr);
   void Assign(const char *pstr, int nLength = -1);
   const char *c_str() const;
   
   void SetAt(int nIndex, char ch);
   operator const char *() const;

   char operator[] (int nIndex) const;
   const CAnsiString_& operator=(const CAnsiString_& src);
   const CAnsiString_& operator=(const char ch);
   const CAnsiString_& operator=(const char *pstr);
   CAnsiString_ operator+(const CAnsiString_& src);
   CAnsiString_ operator+(const char *pstr);
   const CAnsiString_& operator+=(const CAnsiString_& src);
   const CAnsiString_& operator+=(const char *pstr);
   const CAnsiString_& operator+=(const char ch);

   bool operator == (const char *str) const;
   bool operator != (const char *str) const;
   bool operator <= (const char * str) const;
   bool operator <  (const char * str) const;
   bool operator >= (const char * str) const;
   bool operator >  (const char * str) const;
  
   bool operator == (const CAnsiString_ &str) const;
   bool operator != (const CAnsiString_ &str) const;
   bool operator <= (const CAnsiString_ &str) const;
   bool operator <  (const CAnsiString_ &str) const;
   bool operator >= (const CAnsiString_ &str) const;
   bool operator >  (const CAnsiString_ &str) const;

   int Compare(const char *pstr) const;
   int CompareNoCase(const char *pstr) const;
   
   void MakeUpper();
   void MakeLower();

   CAnsiString_ Left(int nLength) const;
   CAnsiString_ Mid(int iPos, int nLength = -1) const;
   CAnsiString_ Right(int nLength) const;

   int Find(char ch, int iPos = 0) const;
   int Find(const char *pstr, int iPos = 0) const;
   int ReverseFind(char ch) const;
   int Replace(const char *pstrFrom, const char *pstrTo);
   
   int __cdecl Format(const char *pstrFormat, ...);

protected:
   DWORD m_dwStrLen; //字符串长度
   char *m_szBuffer;
};

static CStdString_ EMPTY_STDSTRING_DECLARE = _T(" ");

//字符串列表
class COMMONLIB_API CStdStringList
{
public:
	CStdStringList();
	~CStdStringList();
public:
	void Add(CStdString_ &szStr);
	void Add(LPCTSTR lpszStr);
    void Delete(int idx);
	int GetCount();
	void Clear();
	int Find(CStdString_ &szStr);
	int Find(LPCTSTR lpszStr);
	CStdString_ & operator[](int idx) ;
private:
	vector<CStdString_ *> m_strList;
};

#endif