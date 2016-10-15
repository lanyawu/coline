// SqliteView.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <commonlib/SqliteDBOP.h>
#include <commonlib/stringutils.h>
#include <list>

#pragma warning(disable:4996)

BOOL CopyDb(CSqliteDBOP *pFrom, CSqliteDBOP *pTo)
{
	if (!pTo->CopyFromDBOP(pFrom))
	{
		char szError[1024] = {0};
		pTo->GetLastszErrorMsg(szError, 1024);
		printf("copy db Failed:%s\n", szError);
	} else
		printf("copy db succ\n");
	return TRUE;
}

BOOL CopyDbTable(CSqliteDBOP *pFrom, CSqliteDBOP *pTo, const char *szSrcTableName, const char *szSrcFields,
	             const char *szDestTableName, const char *szDestFields)
{
	if (!pTo->CopyFromDBOP(pFrom, szSrcTableName, szSrcFields, szDestTableName, szDestFields))
	{
		char szError[1024] = {0};
		pTo->GetLastszErrorMsg(szError, 1024);
		printf("copy db Failed:%s\n", szError);
	} else
		printf("copy db succ\n");
	return TRUE;
}
BOOL ParserCopyTableParam(const char *szSql, std::string &strDbName, std::string &strPwd,
	                      std::string &strSrcTableName, std::string &strSrcFields,
						  std::string &strDestTableName, std::string &strDestFields)
{
	std::string strSql = szSql;
	std::string strTmp;
	std::string strTag;
	int nPos;
	nPos = (int) strSql.find(" ");
	if (nPos != std::string::npos)
	{
		strTmp = strSql.substr(0, nPos);
		strSql.erase(0, nPos + 1);
	} else
		return FALSE;
	while (TRUE)
	{
		nPos = (int) strSql.find(" ");
		if (nPos != std::string::npos)
		{
			strTmp = strSql.substr(0, nPos);
			strSql.erase(0, nPos + 1);
		} else
		{
			strTmp = strSql;
			strSql = ""; 
		}
		if (strTmp.size() > 2)
		{
			strTag = strTmp.substr(0, 2);
			strTmp.erase(0, 2);
			if (strTag.front() == '-')
			{
				switch(strTag.back())
				{
				case 'T':
				case 't':
					strDestTableName = strTmp;
					break;
				case 'D':
				case 'd':
					strDbName = strTmp;
					break;
				case 'F':
				case 'f':
					strDestFields = strTmp;
					break;
				case 'P':
				case 'p':
					strPwd = strTmp;
					break;
				case 'S':
				case 's':
					strSrcTableName = strTmp;
					break;
				case 'Y':
				case 'y':
					strSrcFields = strTmp;
					break;
				default:
					return FALSE;
				}
			} else
				return FALSE;
		} else
		{
			return FALSE;
		}
		if (strSql.empty())
			break;
	} 
	if (strSrcTableName.empty())
		strSrcTableName = strDestTableName;
	if (strSrcFields.empty())
		strSrcFields = strDestFields;
	if (strDbName.empty() || strDestTableName.empty() || strDestFields.empty())
		return FALSE;
	return TRUE;
}

BOOL Copytable(CSqliteDBOP *p, const char *szSql)
{
	std::string strDbName, strPwd;
	std::string strSrcTableName, strSrcFields;
	std::string strDestTableName, strDestFields;
	if (ParserCopyTableParam(szSql, strDbName, strPwd, strSrcTableName, strSrcFields,
		               strDestTableName, strDestFields))
	{
		CSqliteDBOP pSrc(strDbName.c_str(), strPwd.c_str());
		CopyDbTable(&pSrc, p, strSrcTableName.c_str(), strSrcFields.c_str(), 
			        strDestTableName.c_str(), strDestFields.c_str());
		return TRUE;
	} else
	{
		printf("\tcopy table param Failed\n\t copytable -Ttablename -Ffields -Ddbname -Ppwd -Stablename -Yfields\n");
		return FALSE;
	}
}

BOOL ParserParam(const char *szSql, std::string &strDbName, std::string &strPwd)
{
	std::string strSql = szSql;
	int nPos = (int) strSql.find(" ");
	if (nPos != std::string::npos)
	{
		strSql.erase(0, nPos + 1);
		nPos = (int) strSql.find(" ");
		if (nPos != std::string::npos)
		{
			strDbName = strSql.substr(0, nPos);
			strPwd = strSql.substr(nPos + 1);
		} else
		{
			strDbName = strSql;
		}
		return TRUE;
	}
	return FALSE;
}

void importDb(CSqliteDBOP *p, const char *szSql)
{
	std::string strDbName, strPwd;
	if (ParserParam(szSql, strDbName, strPwd))
	{
		CSqliteDBOP pSrc(strDbName.c_str(), strPwd.c_str());
		CopyDb(&pSrc, p);
	} else
		printf("import param error\n\t import dbname dbpwd\n");
}

void exportDb(CSqliteDBOP *p, const char *szSql)
{
	std::string strDbName, strPwd;
	if (ParserParam(szSql, strDbName, strPwd))
	{
		CSqliteDBOP pSrc(strDbName.c_str(), strPwd.c_str());
		CopyDb(p, &pSrc);
	} else
		printf("import param error\n\t import dbname dbpwd\n");
}

BOOL bIsUTF8 = FALSE;
int _tmain(int argc, _TCHAR* argv[])
{
	CSqliteDBOP *pSqlite;
	char szFileName[MAX_PATH] = {0};
	char szPwd[MAX_PATH] = {0};
	if (argc > 1)
	{
		CStringConversion::WideCharToString(argv[1], szFileName, MAX_PATH - 1);
	}
	if (argc > 2)
	{
		CStringConversion::WideCharToString(argv[2], szPwd, MAX_PATH - 1);
	}
	if (::strlen(szFileName) == 0)
	{
		printf("input sqlite file name:");
		scanf("%[^\n]", szFileName);
		printf("input sqlite password:");
		scanf("%[^\n]", szPwd);
	}
	try
	{
		pSqlite = new CSqliteDBOP(szFileName, szPwd);
		char szEncoding[MAX_PATH] = {0};
		if (pSqlite->GetEncoding(szEncoding))
		{
			bIsUTF8 = (::stricmp(szEncoding, "UTF-8") == 0);
			printf("\tDatabase Encoding:%s\n", szEncoding);
			std::string strSql = "PRAGMA encoding='";
			strSql += szEncoding;
			strSql += "'";
			if (pSqlite->Execute(strSql.c_str()))
				printf("\t set database encoding succ\n");
			else
				printf("\t set database encoding failed\n");
		} else
		{
			printf("\tGet Database Encoding Failed\n");
		}
	} catch(...)
	{
		printf("open sqlite file failed!!\n");
		pSqlite = NULL;
	}
	if (pSqlite)
	{
		char szSql[2048];
		while (true)
		{
			memset(szSql, 0, 2048);
			printf("sqlite->");
			scanf("%[^\n]", szSql);
			fflush(stdin);
			if (::stricmp(szSql, "exit") == 0)
				break;
			if (::stricmp("tables", szSql) == 0)
			{
				sprintf(szSql, "SELECT name FROM sqlite_master WHERE type IN ('table','view') AND name NOT LIKE 'sqlite_%'UNION ALL SELECT \
							   name FROM sqlite_temp_master WHERE type IN ('table','view') ORDER BY 1");

			} else if (::strnicmp("desc", szSql, strlen("desc")) == 0)
			{
				std::string strSrc = szSql;
				size_t nPos = strSrc.find(" ");
				if (nPos == std::string::npos)
				{
					sprintf(szSql, "SELECT sql FROM (SELECT * FROM sqlite_master UNION ALL SELECT * FROM sqlite_temp_master) \
								   WHERE type!='meta'ORDER BY tbl_name, type DESC, name");
				} else
				{
					char szTable[128] = {0};
					strncpy(szTable, szSql + strlen("desc "), strlen(szSql) - strlen("desc "));
					sprintf(szSql, "SELECT sql FROM (SELECT * FROM sqlite_master UNION ALL SELECT * FROM sqlite_temp_master) WHERE tbl_name = '%s' \
								   AND type!='meta' ORDER BY tbl_name, type DESC, name", szTable);
				}
			} else if (::stricmp("databases", szSql) == 0)
			{
				memset(szSql, 0, 2048);
				sprintf(szSql, "PRAGMA database_list;");
			}
			if ((::strnicmp("select", szSql, strlen("select")) == 0)
				|| (strnicmp("PRAGMA", szSql, strlen("PRAGMA")) == 0))
			{
				char **szResult = NULL;
		        int nRow = 0, nCol = 0;
				std::string strFileName, strSql;
				std::string strTmp = szSql;
				int nPos = strTmp.find(">>");
				FILE *fp = NULL;
				if (nPos != std::string::npos)
				{
					strSql = strTmp.substr(0, nPos);
					strFileName = strTmp.substr(nPos + 2);
					fp = fopen(strFileName.c_str(), "a");
				} else
					strSql = strTmp;
				if (pSqlite->Open(strSql.c_str(), &szResult, nRow, nCol))
				{
					for (int i = 0; i < nCol; i ++)
					{
						printf("\t%s", szResult[i]);
					}
					printf("\n");
					char strResult[2048];
					for (int i = 1; i <= nRow; i ++)
					{
						for (int j = 0; j < nCol; j ++)
						{
							memset(strResult, 0, 2048);
							if (szResult[i * nCol + j])
							{
								if (fp)
								{
									fprintf(fp, "%s\n", szResult[i * nCol + j]);
									fprintf(fp, "\t");
								} else
								{
									if (bIsUTF8)
									{
										CStringConversion::UTF8ToString(szResult[i * nCol + j], strResult, 2047);
										printf("\t%s", strResult);
									} else
									{
										printf("\t%s", szResult[i * nCol + j]);
									}
								}
							} else
								printf("\tNULL");
						}
						if (fp)
							fprintf(fp, "\n");
						else
							printf("\n");
					}
					pSqlite->Free_Result(szResult);
				} else
				{
					char szError[1024] = {0};
					pSqlite->GetLastszErrorMsg(szError, 1023);
					printf("Error:%s\n", szError);
				}
				if (fp)
					fclose(fp);
			} else
			{
				if (::strnicmp(szSql, "import", strlen("import")) == 0) //µ¼Èë
				{
					importDb(pSqlite, szSql);
				} else if (::strnicmp(szSql, "export", strlen("export")) == 0)
				{
					exportDb(pSqlite, szSql);
				} else if (::strnicmp(szSql, "copytable", strlen("copytable")) == 0)
				{
					Copytable(pSqlite, szSql);
				} else
				{
					BOOL bExecute = TRUE;
					if ((::strnicmp(szSql, "delete", strlen("delete")) == 0)
						|| (::strnicmp(szSql, "update", strlen("update")) == 0)
						|| (::strnicmp(szSql, "drop", strlen("drop")) == 0)
						|| (::strnicmp(szSql, "alter", strlen("alter")) == 0))
					{
						char c;
						printf("Execute:%s\n \t confirm:y/n:", szSql);
						c = getchar();
						if ((c == 'n') || (c == 'N'))
							bExecute = FALSE;
						fflush(stdin);
					}
					if (bExecute)
					{
						if (pSqlite->Execute(szSql))
						{
							printf("exec succ\n");
						} else
						{
							char szError[1024] = {0};
							pSqlite->GetLastszErrorMsg(szError, 1023);
							printf("Error:%s\n", szError);
						}
					} else
					{
						printf("\tabort execute:%s\n", szSql);
					}
				}
			}
		}
		delete pSqlite;
	}
	return 0;
}

#pragma warning(default:4996)
