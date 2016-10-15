#ifndef __SQLITEDBOP_H__
#define __SQLITEDBOP_H__

#include <vector>
#include <string>
#include <Sqlite/sqlite3.h>
#include <CommonLib/types.h>

//Sqlite �������ݲ�����
class COMMONLIB_API CSqliteDBOP
{
public:
	CSqliteDBOP(const char *szDbFileName, const char *szKey);
	~CSqliteDBOP(void);
public:
	BOOL TableIsExists(const char *szTableName); //��ѯ���Ƿ����
	BOOL Execute(const char *szSql); //ִ��sql��� �޽������
    BOOL Open(const char *szSql, char ***result , int &nrow , int &ncolumn); //�н���������� result��ά�����
	int LastInsertRowId();//���һ�β���ɹ���,���е�����
	BOOL InsertBlob(const char *szSql, const char *szBlob, DWORD dwBlobSize); //����һ����blob�ֶ����
	BOOL GetBlob(const char *szSql, char **szBlob, DWORD &dwBlobSize, DWORD dwFieldIdx); //�õ�blob�ֶ�
	static void Free_Result(char **result); //�ͷŽ����
	static void Free_Blob(char **szBlob); //�ͷ�BLOB�ֶ�
	void ClearError();   //���������־
	BOOL CopyFromDBOP(CSqliteDBOP *pSrc);  //
	BOOL CopyFromDBOP(CSqliteDBOP *pSrc, const char *szSrcTableName, const char *szSrcFields,
		              const char *szDestTableName, const char *szDestFields);
	BOOL GetAllTables(std::vector<std::string> &strTables);
	BOOL GetTableCreateSql(const char *szTableName, std::vector<std::string> &strCreateSql);  //
	BOOL GetLastszErrorMsg(char *szErroMsg, int nMaxLen); //��ȡ������Ϣ
	//��ȡ�ַ���
	BOOL GetEncoding(char *szEncoding);
	static char *StrToSqlStr(const char *szSrc, char *szDest); //ת���ɰ�ȫ��sql���
private:
    sqlite3  *m_sqlite;
    char *m_szErrorMsg;
};

#endif