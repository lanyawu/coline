#include <commonlib/IniConfigFile.h>
#include <stdio.h>

#pragma warning(disable:4996)

#define SKIP_BLANK(p) \
  while( *p && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') ) p++

#define SKIP_BLANK_BACK(p) \
  while( *p && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') ) p--

CIniConfigFile::CIniConfigFile(void)
{
 
}

CIniConfigFile::~CIniConfigFile(void)
{
	string_map::iterator it;
	for(it = m_maps.begin(); it != m_maps.end(); it++)
		delete (*it).second;
	m_maps.clear();
	m_sectors.clear();
}

// 加载配置文件
BOOL CIniConfigFile::Load(const char *szIniFile, const char *szSector, int nMaxLineLength)
{
	m_strIniFileName = szIniFile;
	FILE *fp = ::fopen(szIniFile, "r");
	if (!fp)
		return FALSE;
	char *szBuf = new char[nMaxLineLength];
	if (!szBuf)
	{
		::fclose(fp);
		return FALSE;
	}
    BOOL bRet = TRUE;
    std::string strSector; //
	while (::fgets(szBuf, nMaxLineLength - 1, fp))
	{
		char *p = szBuf;
		SKIP_BLANK(p);
		if (!(*p) || (*p == ';')) // 空串或注释
			continue;
		int nLen = (int)::strlen(p);
		char *p1 = p + nLen - 1;
        SKIP_BLANK_BACK(p1);
        *(p1+1) = 0;
		if (*p == '[') // 节
		{
			strSector.clear();
			if (*p1 != ']') // 错误的节
				continue;
			p ++;
			SKIP_BLANK(p);
			if (p == p1) // 空节
				continue;
			p1--;
			SKIP_BLANK_BACK(p1);
			strSector = std::string(p, p1 - p + 1);
			if (!szSector || strSector == szSector)
				m_sectors.insert(string_pair_sector(strSector, true));
			continue;
		} //end if (*p == '[')
		if (strSector.size() == 0) // 跳过没有节的所有键
			continue;
		if (szSector && strSector != szSector) // 跳过非指定的节
			continue;
		char *p2 = strchr(p, '=');
		if (!p2 || p2 == p) // 没有键，跳过
			continue;
		char *p3 = p2-1;
		SKIP_BLANK_BACK(p3);
		std::string strKey = std::string(p, p3 - p + 1);
		std::string strVal;
		if (p2 != p1) // 有值
		{
			p2++;
			SKIP_BLANK(p2);
			strVal = std::string(p2,p1 -p2 + 1);
		}
		if (!InsertKey(strSector.c_str(), strKey.c_str(), strVal.c_str()))
        {
			bRet = FALSE;
			break;
		}
	}
	delete []szBuf;
	::fclose(fp);
	return bRet;
}

#define CHECKKEY() \
   string_map::iterator it = FindKey(szSector, szKey); \
   if (it == m_maps.end() || (*(*it).second)[0].size() == 0) \
      return Default;
// 单值函数
char CIniConfigFile::GetChar(const char *szSector, const char *szKey, char Default)
{
	CHECKKEY();
	return (*(*(*it).second)[0].c_str());
}

BOOL CIniConfigFile::GetBool(const char *szSector, const char *szKey, BOOL Default)
{
	CHECKKEY();
	return (::stricmp((*(*it).second)[0].c_str(), "true") == 0);
}

int base(const char* p)
{
	if (*p == '0' && ( *(p+1) == 'x' || *(p+1) == 'X'))
		return 16;
	else if (*p == '0')
		return 8;
	else
		return 10;
}
int CIniConfigFile::GetInt(const char *szSector, const char *szKey, int Default)
{
	CHECKKEY();
	const char *p = (*(*it).second)[0].c_str();
	return (int)::strtol(p, 0, base(p));
}

long CIniConfigFile::GetLong(const char *szSector, const char *szKey, long Default)
{
	CHECKKEY();
	const char *p = (*(*it).second)[0].c_str();
	return ::strtol(p, 0, base(p));
}

long long CIniConfigFile::GetLonglong(const char *szSector, const char *szKey, long long Default)
{
	CHECKKEY();
	const char *p = (*(*it).second)[0].c_str();
	return ::strtol(p, 0, base(p));
}

double CIniConfigFile::GetDouble(const char *szSector, const char *szKey, double Default)
{
	CHECKKEY();
	return ::strtod((*(*it).second)[0].c_str(), 0);
}

const char *CIniConfigFile::GetString(const char *szSector, const char *szKey, const char *Default)
{
	CHECKKEY();
	return (*(*it).second)[0].c_str();
}

#undef CHECKKEY

#define CHECKKEY(val) \
  string_map::iterator it = FindKey(szSector, szKey); \
  if( it == m_maps.end() || (*(*it).second)[0].size() == 0 ) \
  { \
  	*val = Default; \
  	return (it != m_maps.end()); \
  }
// 在节或键不存在时返回false，存在返回true
BOOL CIniConfigFile::GetChar2(const char *szSector, const char *szKey, char *pchVal, char Default)
{
	CHECKKEY(pchVal);
	*pchVal = (*(*(*it).second)[0].c_str());
	return TRUE;
}

BOOL CIniConfigFile::GetBool2(const char *szSector, const char *szKey, BOOL *pbVal, BOOL Default)
{
	CHECKKEY(pbVal);
	*pbVal = (::stricmp((*(*it).second)[0].c_str(), "true") == 0);
	return TRUE;
}

BOOL CIniConfigFile::GetInt2(const char *szSector, const char *szKey, int *pnVal, int Default)
{
	CHECKKEY(pnVal);
	const char *p = (*(*it).second)[0].c_str();
	*pnVal = (int)strtol(p, 0, base(p));
	return TRUE;
}


BOOL CIniConfigFile::GetLong2(const char *szSector, const char *szKey, long *plVal, long Default)
{
	CHECKKEY(plVal);
	const char *p = (*(*it).second)[0].c_str();
	*plVal = ::strtol(p, 0, base(p));
	return TRUE;
}

BOOL CIniConfigFile::GetLonglong2(const char *szSector, const char *szKey, long long *pllVal, long long Default)
{
	CHECKKEY(pllVal);
	const char *p = (*(*it).second)[0].c_str();
	*pllVal = ::strtol(p, 0, base(p));
	return TRUE;
}

BOOL CIniConfigFile::GetDouble2(const char *szSector, const char *szKey, double* pfVal, double Default)
{
	CHECKKEY(pfVal);
	*pfVal = ::strtod((*(*it).second)[0].c_str(),0);
	return TRUE;
}

BOOL CIniConfigFile::GetStr2(const char *szSector, const char *szKey, const char** ppVal, const char *Default)
{
	CHECKKEY(ppVal);
	*ppVal = (*(*it).second)[0].c_str();
	return TRUE;
}

// 多值函数，如果不存在，则返回0
CIniConfigFile::string_vector *CIniConfigFile::GetVector(const char *szSector, const char *szKey)
{
	CIniConfigFile::string_map::iterator it = FindKey(szSector, szKey);
	if (it == m_maps.end())
		return NULL;
	return (*it).second;
}

BOOL CIniConfigFile::HasSector(const char *szSector)
{
	std::string strKey = szSector;
	return (m_sectors.find(strKey) != m_sectors.end());
}

BOOL CIniConfigFile::HasKey(const char *szSector, const char *szKey)
{
	return (FindKey(szSector, szKey) != m_maps.end());
}

const char *CIniConfigFile::GetName() 
{ 
	return m_strIniFileName.c_str(); 
} 

CIniConfigFile::string_map::iterator CIniConfigFile::FindKey(const char *szSector, const char *szKey)
{
	std::string strKey = szSector;
	strKey += "@";
	strKey += szKey;
	return m_maps.find(strKey);  
}

BOOL CIniConfigFile::InsertKey(const char *szSector, const char *szKey, const char *szValue)
{
	std::string strKey = szSector;
	strKey += "@";
	strKey += szKey;
	string_map::iterator it = m_maps.find(strKey);
	if (it == m_maps.end()) // 不存在
	{
		string_vector *pVal = new string_vector;
		if (!pVal)
			return FALSE;
		pVal->push_back(szValue);
		m_maps.insert(string_pair(strKey, pVal));
	}  else // 已经存在
	{
		(*it).second->push_back(std::string(szValue));
	}
	return TRUE;
}

#pragma warning(default:4996)
