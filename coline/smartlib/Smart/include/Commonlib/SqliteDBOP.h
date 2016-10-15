#ifndef __SQLITEDBOP_H__
#define __SQLITEDBOP_H__

#include <vector>
#include <string>
#include <Sqlite/sqlite3.h>
#include <CommonLib/types.h>

//Sqlite 轻型数据操作类
class COMMONLIB_API CSqliteDBOP
{
public:
	CSqliteDBOP(const char *szDbFileName, const char *szKey);
	~CSqliteDBOP(void);
public:
	BOOL TableIsExists(const char *szTableName); //查询表是否存在
	BOOL Execute(const char *szSql); //执行sql语句 无结果返回
    BOOL Open(const char *szSql, char ***result , int &nrow , int &ncolumn); //有结果返回数据 result二维结果集
	int LastInsertRowId();//最近一次插入成功后,该行的主键
	BOOL InsertBlob(const char *szSql, const char *szBlob, DWORD dwBlobSize); //插入一条含blob字段语句
	BOOL GetBlob(const char *szSql, char **szBlob, DWORD &dwBlobSize, DWORD dwFieldIdx); //得到blob字段
	static void Free_Result(char **result); //释放结果集
	static void Free_Blob(char **szBlob); //释放BLOB字段
	void ClearError();   //清除错误日志
	BOOL CopyFromDBOP(CSqliteDBOP *pSrc);  //
	BOOL CopyFromDBOP(CSqliteDBOP *pSrc, const char *szSrcTableName, const char *szSrcFields,
		              const char *szDestTableName, const char *szDestFields);
	BOOL GetAllTables(std::vector<std::string> &strTables);
	BOOL GetTableCreateSql(const char *szTableName, std::vector<std::string> &strCreateSql);  //
	BOOL GetLastszErrorMsg(char *szErroMsg, int nMaxLen); //获取错误信息
	//获取字符集
	BOOL GetEncoding(char *szEncoding);
	static char *StrToSqlStr(const char *szSrc, char *szDest); //转换成安全的sql语句
private:
    sqlite3  *m_sqlite;
    char *m_szErrorMsg;
};

#endif