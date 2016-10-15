#ifndef __ORACLEQUERY_H_____
#define __ORACLEQUERY_H_____
#include <vector>
#include <string>
#include <Commonlib/GuardLock.h>
#import "C:\\Program Files (x86)\\Common Files\\System\\ado\\msado15.dll" no_namespace rename ("EOF", "adoEOF")
#define MAX_CONNECTSTRING_LENGTH 512
#define MAX_DB_COUNT 10


class COracleQuery
{
public:
	COracleQuery(void);
	~COracleQuery(void);
private:
	BOOL ConnectDB();	
	void Close();
public:
	static char * SafeSQLString(char * szSQL);
	BOOL Connect(const char *strDbHost,const  WORD wHostPort, const char *strUserName, const  char *strUserPwd, 
		const char *strDbName, const char *szDSN = NULL);
	BOOL LoadFromFile(const char *szFileName);
	BOOL OpenTable(const char *strTableName, _RecordsetPtr &pRecordset);
    BOOL Execute(const char *lpQueryString, _RecordsetPtr& pRecordset);
	BOOL ExecuteProc(const char *lpQueryString, _RecordsetPtr &pRecordset);
	BOOL Execute(const char *lpQueryString); //没有结果集的操作
	BOOL Connected() { return (m_lpConnection != NULL); }; 
	int  GetLastInsertId();
private:
	char					m_strConnectString[MAX_CONNECTSTRING_LENGTH];
	_Connection *			m_lpConnection;
	DWORD					m_dwConnectionCookie;
    IGlobalInterfaceTable   *m_iGIT;
	std::string m_strUserName;
	std::string m_strUserPwd; 
};

//class COracleDBQueryBufferMan 数据库查询池
class COracleDBQueryBufferMan
{
public:
	COracleDBQueryBufferMan(DWORD dwCount);
	~COracleDBQueryBufferMan();
private:
	std::vector<COracleQuery *> m_DBList;
    CGuardLock m_DbLock;
	HANDLE m_hDbNotifyEvent;
public:
	bool OpenDatabase(const char *lpServer, WORD wPort, const char *lpUID, const char *lpPWD, const char * lpDatabase);
	void InsertDBQuery(COracleQuery *lpDbQuery);
	COracleQuery *GetDBQuery();
};

#endif
