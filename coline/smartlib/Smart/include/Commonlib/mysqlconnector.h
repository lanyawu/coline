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
	std::string m_strHostName; //������ַ
	std::string m_strUserName; //�û���
	std::string m_strPassword; //�û�����
	std::string m_strDbName;   //���ݿ���
	std::string m_strCharSet;  //�ַ���
	short m_nPort;             //�˿�
	BOOL m_bConnected;		   //���ݿ��Ƿ�������

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
// class CMySQLConn - ���ݿ�(MySQL)������
//
// ˵��:
// 1. ����ΪMySQL�����࣬����MySQL���ݿ����ӡ��ر��Լ�ִ��SQL�ȹ��ܣ�
// 2. ���󴴽�ʱ���������Ͻ������ݿ����ӣ�
// 3. ��δ��������£�ִ��SQL���Զ����ӵ����ݿ⣻
// 4. ���ڳ�ʱ���޶����������ӱ��ж�ʱ��ִ��SQL���Զ��������ӣ�

class COMMONLIB_API CMySQLConn : public CDbConn
{
protected:
	MYSQL m_stMySQL;	//MySQL����ָ��
	MYSQL_RES *m_pRes;	//MySQL��ѯ�����
	MYSQL_ROW m_pRow;	//MySQL��ѯ�����

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

//class ���ݿ����ӳ�
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
