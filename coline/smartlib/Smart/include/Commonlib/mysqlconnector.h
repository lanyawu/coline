#ifndef __MYSQLCONNECTOR_H____
#define __MYSQLCONNECTOR_H____

#include <commonlib/guardlock.h>
#include <string>
#include <vector>
#include <winsock2.h>
#include <mysql.h>

class COMMONLIB_API CDbConn
{
protected:
	std::string m_strHostName; //主机地址
	std::string m_strUserName; //用户名
	std::string m_strPassword; //用户口令
	std::string m_strDbName;   //数据库名
	std::string m_strCharSet;  //字符集
	short m_nPort;             //端口
	BOOL m_bConnected;		   //数据库是否已连接

protected:
	virtual BOOL InternalQuery(const char *szSQLCmd, int nSQLCmdLen, BOOL bGetResult) = 0;

public:
	CDbConn();
	CDbConn(const char *szHostName, short nPort, const char *szUserName, 
		const char *szPassword, const char *szDbName, const char *szCharSet);
	virtual ~CDbConn();

    static char *GetSafeSqlString(const char *szSql, char *szDest);
	virtual BOOL Open(const char *szHostName, short nPort, const char *szUserName, 
		const char *szPassword, const char *szDbName, const char *szCharSet);
	virtual void Close() {}
	virtual BOOL FetchRow() = 0;
	virtual BOOL MoveFirstRow() = 0;
	virtual BOOL MoveLastRow() = 0;
	virtual char *GetFieldData(int nFieldIndex) = 0;
	virtual int GetRecordCount() = 0;
	virtual int GetLastInsertId();
	BOOL Open();
	BOOL ReOpen();
	BOOL Query(const char *szSQLCmd, BOOL bGetResult);
	BOOL Query(const char *szSQLCmd, int nSQLCmdLen, BOOL bGetResult);
	BOOL IsOpen() { return m_bConnected; }
};

//////////////////////////////////////////////////////////////////////////////
// class CMySQLConn - 数据库(MySQL)连接类
//
// 说明:
// 1. 此类为MySQL连接类，负责MySQL数据库连接、关闭以及执行SQL等功能；
// 2. 对象创建时，并不马上建立数据库连接；
// 3. 在未连接情况下，执行SQL会自动连接到数据库；
// 4. 由于长时间无动作导致连接被中断时，执行SQL会自动重新连接；

class COMMONLIB_API CMySQLConn : public CDbConn
{
protected:
	MYSQL m_stMySQL;	//MySQL对象指针
	MYSQL_RES *m_pRes;	//MySQL查询结果集
	MYSQL_ROW m_pRow;	//MySQL查询结果行

protected:
	void FreeMySQLRes();
	virtual BOOL InternalQuery(const char *szSQLCmd, int nSQLCmdLen, BOOL bGetResult);
public:
	CMySQLConn();
	CMySQLConn(const char *szHostName, short nPort, const char *szUserName, 
		const char *szPassword, const char *szDbName, const char *szCharSet);
	virtual ~CMySQLConn();

	virtual BOOL Open(const char *szHostName, short nPort, const char *szUserName, 
		const char *szPassword, const char *szDbName, const char *szCharSet);
	virtual void Close();
	virtual BOOL FetchRow();
    virtual BOOL MoveFirstRow();
	virtual BOOL MoveLastRow();
	virtual char *GetFieldData(int nFieldIndex);
	virtual int GetRecordCount();
	virtual int GetLastInsertId();
};

//class 数据库连接池
class COMMONLIB_API CDbConnectionPool
{
public:
	CDbConnectionPool(int dwCount);
	~CDbConnectionPool();
private:
	std::vector<CMySQLConn *> m_DBList;
    CGuardLock m_DbLock;
	HANDLE m_hEvent;
	BOOL m_bTerminated;
public:
	BOOL OpenDatabase(const char *szHostName, short wPort, const char *szUserName, const char *szUserPwd,
	                  const char * szDatabase, const char *szCharSet);
	void InsertDBQuery(CMySQLConn *pDbQuery);
	CMySQLConn *GetDBConnection();
}; 


#endif
