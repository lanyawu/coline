#ifndef __INICONFIGFILE_H___
#define __INICONFIGFILE_H___


#include <string>
#include <vector>
#include <list>
#include <hash_map>
#include <commonlib/types.h>
// 配置文件格式：
// [sector]
// key=value
// ; 注释

// 注意：每个键必须在节内，否则被忽略
//       一个节内可以有多个同名的键

inline size_t __STL_Hash_String(const char *__s)
{
	unsigned long __h = 0;
	for ( ; *__s; ++__s)
		__h = 5 * __h + *__s;
	return size_t(__h);
}

class COMMONLIB_API CIniConfigFile
{
public:
	CIniConfigFile(void);
	~CIniConfigFile(void);
public:
    typedef std::vector<std::string> string_vector;
    
    // 加载配置文件
    BOOL Load(const char *szIniFile, const char *szSector = NULL, int nMaxLineLength = 2048);
    
    // 缺省值：
    // 当节或键不存在时，或存在但未提供值时，使用缺省值
    // 值的开始空格、tab键不作为值的内容，在值解析时被过滤
    
    // 单值函数
    char GetChar(const char *szSector, const char *szKey, char cDefault = '\0');
    BOOL GetBool(const char *szSector, const char *szKey, BOOL bDefault = FALSE);
    int GetInt(const char *szSector, const char *szKey, int nDefault = 0);   
    long GetLong(const char *szSector, const char *szKey, long lDefault = 0);
    long long GetLonglong(const char *szSector, const char *szKey, long long llDefault = 0);
    double GetDouble(const char *szSector, const char *szKey, double fDefault = 0.0 );
    const char *GetString(const char *szSector, const char *szKey, const char *szDefault = "");    

	// 在节或键不存在时返回false，存在返回true
    BOOL GetChar2(const char *szSector, const char *szKey, char *pchVal, char cDefault = 0);
    BOOL GetBool2(const char *szSector, const char *szKey, BOOL *pbVal, BOOL bDefault = FALSE);
    BOOL GetInt2(const char *szSector, const char *szKey, int *pnVal, int nDefault = 0);   
    BOOL GetLong2(const char *szSector, const char *szKey, long *plVal, long lDefault = 0);
    BOOL GetLonglong2(const char *szSector, const char *szKey, long long *pllVal, long long llDefault = 0);
    BOOL GetDouble2(const char *szSector, const char *szKey, double* pfVal, double fDefault = 0.0);
    BOOL GetStr2(const char *szSector, const char *szKey, const char** ppVal, const char *szDefault = "");    

    // 多值函数，如果不存在，则返回0
    string_vector *GetVector(const char *szSector, const char *szKey);
    
    BOOL HasSector(const char *szSector);
    BOOL HasKey(const char *szSector, const char *szKey);
    const char *GetName();  
protected:    
    struct hash_string
    {
		size_t operator()(const std::string &str) const
		{
			return __STL_Hash_String(str.c_str());
		} 
    };

    // hash比较equalto
    struct comp_string
    {
		bool operator()(const std::string &str1, const std::string &str2) const
		{
			return str1 == str2;
		}
    };  

	typedef stdext::hash_map<std::string, string_vector *> string_map;
    typedef std::pair<std::string, string_vector*> string_pair;

    typedef stdext::hash_map<std::string, bool> string_map_sector;
    typedef std::pair<std::string, bool> string_pair_sector;
      
	string_map m_maps;  /// 键值映射表<sector@key,value>
    string_map_sector m_sectors;  /// 节
    std::string m_strIniFileName;

    string_map::iterator FindKey(const char *szSector, const char *szKey);
    BOOL InsertKey(const char *szSector, const char *szKey, const char *szValue);
};

#endif
