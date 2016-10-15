#include <Commonlib/SqliteDBOP.h>
#include <Commonlib/StringUtils.h>
#include <stdio.h>

#pragma warning(disable:4996)

CSqliteDBOP::CSqliteDBOP(const char *szDbFileName, const char *szKey):
             m_sqlite(NULL),
			 m_szErrorMsg(NULL)
{
	int nSize = (int)(strlen(szDbFileName) * 2 + 1);
	char *szDest = new char[nSize];
	memset(szDest, 0, nSize);
	CStringConversion::StringToUTF8(szDbFileName, szDest, nSize);
	int rc = sqlite3_open(szDest, &m_sqlite);
	delete []szDest;
	if (rc)
	{
		throw "创建本地数据库出错";
	} else
	{
		if ((szKey) && (m_sqlite))
		{
			sqlite3_key(m_sqlite, szKey, (int)strlen(szKey));
		}
	}
}

CSqliteDBOP::~CSqliteDBOP(void)
{
	if (m_sqlite)
	{
		 sqlite3_close(m_sqlite);
		 m_sqlite = NULL;
	}
	if (m_szErrorMsg)
	{
		delete []m_szErrorMsg;
		m_szErrorMsg = NULL;
	}
}

BOOL CSqliteDBOP::TableIsExists(const char *szTableName)
{
	BOOL b= FALSE;
	char szSql[1024] = {0};
	sprintf(szSql, "select Count(*) from sqlite_master where name=\"%s\";", szTableName);
	char **result = NULL;
	int nrow,  ncol;
	if (Open(szSql, &result, nrow, ncol))
	{
		if (nrow > 0)
		{
			if (atoi(result[1]) > 0)
				b = TRUE;
		}
	}
	sqlite3_free_table(result);
	return b;
}

void CSqliteDBOP::Free_Result(char **result)
{
	sqlite3_free_table(result);
}

char * CSqliteDBOP::StrToSqlStr( const char *szSrc, char *szDest)
{
	char *szTemp = szDest;
	if (szSrc) 
	{
		while (*szSrc != '\0')
		{
			if (*szSrc == '\'')
			{
				*szTemp = '\'';
				szTemp ++;
			}
			if (*szSrc == '\"')
			{
				*szTemp = '\"';
				szTemp ++;
			}
			*szTemp = *szSrc;
			szTemp ++;
			szSrc ++;
		}
	}
    return szDest;
}

BOOL CSqliteDBOP::Execute(const char *szSql)
{
	if (m_sqlite)
	{
		if (m_szErrorMsg)
		{
			delete []m_szErrorMsg;
			m_szErrorMsg = NULL;
		}
		int rc = sqlite3_exec(m_sqlite, szSql, 0, 0, NULL);
		if (rc != SQLITE_OK)
		{
			int n = (int) strlen(sqlite3_errmsg(m_sqlite));
			m_szErrorMsg = new char[n + 1];
			memset(m_szErrorMsg, 0, n + 1);
			strncpy(m_szErrorMsg, sqlite3_errmsg(m_sqlite), n);
			//(m_sqlite);
		}
		return (rc == SQLITE_OK);
	}
	return FALSE;
}

BOOL CSqliteDBOP::InsertBlob(const char *szSql, const char *szBlob, DWORD dwBlobSize)
{
	if (m_sqlite)
	{
		sqlite3_stmt *stat = NULL;
		sqlite3_prepare(m_sqlite, szSql, -1, &stat, 0);
		sqlite3_bind_blob(stat, 1, szBlob, dwBlobSize, NULL);
		int iRet = sqlite3_step(stat);
		return ((SQLITE_OK == iRet) || (SQLITE_DONE == iRet));
	}
	return FALSE;
}

BOOL CSqliteDBOP::GetBlob(const char *szSql, char **szBlob, DWORD &dwBlobSize, DWORD dwFieldIdx)
{
	if (m_sqlite)
	{
		sqlite3_stmt *stat = NULL;		
		sqlite3_prepare(m_sqlite, szSql, -1, &stat, NULL);
		sqlite3_step(stat);
		//得到纪录中的BLOB字段
		const void *p = sqlite3_column_blob(stat, dwFieldIdx);
		//得到字段中数据的长度
		dwBlobSize = sqlite3_column_bytes(stat, dwFieldIdx);
        *szBlob = new char[dwBlobSize + 1];
		memset((*szBlob), 0, dwBlobSize + 1);
		memmove((*szBlob), p, dwBlobSize);
		if (stat)
			sqlite3_finalize( stat );
		return TRUE;
	}
	return FALSE;
}

void CSqliteDBOP::Free_Blob(char **szBlob)
{
	if (*szBlob)
	{
		delete [](*szBlob);
		*szBlob = NULL;
	}
}

BOOL CSqliteDBOP::GetTableCreateSql(const char *szTableName, std::vector<std::string> &strCreateSql)
{
	BOOL bSucc = FALSE;
	if (szTableName)
	{
		std::string strSql = "SELECT sql FROM (SELECT * FROM sqlite_master UNION ALL SELECT * FROM sqlite_temp_master) WHERE tbl_name = '";
		strSql += szTableName;
		strSql += "' AND type!='meta'";
		char **szResult = NULL;
		int nRow, nCol;
		strCreateSql.clear();
		if (Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			for (int i = 1; i <= nRow; i ++)
			{
				if (szResult[i])
					strCreateSql.push_back(szResult[i]);
			}
			bSucc = TRUE;
			Free_Result(szResult);			
		} 
	}
	return bSucc;
}

BOOL CSqliteDBOP::GetAllTables(std::vector<std::string> &strTables)
{
	static char strSql[] = "SELECT name FROM sqlite_master WHERE type IN ('table','view') AND name NOT LIKE 'sqlite_%'UNION ALL SELECT \
							   name FROM sqlite_temp_master WHERE type IN ('table','view') ORDER BY 1";
	char **szResult;
	int nRow, nCol;
	if (Open(strSql, &szResult, nRow, nCol))
	{
		std::string strName;
		for (int i = 1; i <= nRow; i ++)
		{
			if (szResult[i])
			{
				strName = szResult[i];
				strTables.push_back(strName);
			}
		}
		Free_Result(szResult);
		return TRUE;
	} else
		return FALSE;
}

BOOL CSqliteDBOP::CopyFromDBOP(CSqliteDBOP *pSrc, const char *szSrcTableName, const char *szSrcFields,
		              const char *szDestTableName, const char *szDestFields)
{
	//import data
	sqlite3_stmt *pStmt;
	char *szSql;   /* An SQL statement */
	int  nCol;
	int  nByte;
	int  i, j;
	int  rc;
	char *szCommit;
	szSql = sqlite3_mprintf("select %q from '%q'", szDestFields, szDestTableName);
	if (!szSql)
		return FALSE;
	rc = sqlite3_prepare(m_sqlite, szSql, -1, &pStmt, NULL);
	nByte = (int) ::strlen(szSql);
	sqlite3_free(szSql);
	if (rc == 0)
	{
		nCol = sqlite3_column_count(pStmt);
	} else
	{
		nCol = 0;

	}
	sqlite3_finalize(pStmt);
	if (nCol > 0)
	{
		szSql = new char[nByte + 20 + nCol * 2];
		memset(szSql, 0, nByte + 20 + nCol * 2);
		sqlite3_snprintf(nByte + 20, szSql, "insert into '%q'(%q) values(?", szDestTableName, szDestFields);
		j = (int)::strlen(szSql);
		for (i = 1; i < nCol; i ++)
		{
			szSql[j ++] = ',';
			szSql[j ++] = '?';
		}
		szSql[j ++] = ')';
		szSql[j] = '\0';
		rc = sqlite3_prepare(m_sqlite, szSql, -1, &pStmt, 0);
		delete []szSql;
		if (rc == 0)
		{
			char **szSrcResult = NULL;
			int nSrcRow, nSrcCol;
			std::string strSrcSql = "select ";
			strSrcSql += szSrcFields;
			strSrcSql += " from ";
			strSrcSql += szSrcTableName;
			sqlite3_exec(m_sqlite, "BEGIN", 0, 0, 0);
            szCommit = "COMMIT";
			if (pSrc->Open(strSrcSql.c_str(), &szSrcResult, nSrcRow, nSrcCol))
			{
				printf("\n\tsource record:%d\n ", nSrcRow);
				for (int nSrci = 1; nSrci <= nSrcRow; nSrci ++)
				{
					for (i = 0; i < nCol; i ++)
						sqlite3_bind_text(pStmt, i + 1, szSrcResult[nSrci * nSrcCol + i], -1, SQLITE_STATIC);
					sqlite3_step(pStmt);
					rc = sqlite3_reset(pStmt);
					if ((nSrci % 500) == 0)
						printf(".");
				}
				sqlite3_finalize(pStmt);
				sqlite3_exec(m_sqlite, szCommit, 0, 0, 0);
			} //end if (pSrc->
			Free_Result(szSrcResult);
		}  else
		{
			return FALSE;
		}//end if (rc == 0)
	}  else
	{
		return FALSE;
	}//end if (nCol > 0)
	return TRUE;
}

BOOL CSqliteDBOP::CopyFromDBOP(CSqliteDBOP *pSrc)
{
	BOOL bSucc = FALSE;
	std::vector<std::string> strTables;
	
	if (pSrc->GetAllTables(strTables))
	{
		bSucc = TRUE;
		std::vector<std::string>::iterator it;
		//create tables;
		std::vector<std::string> strCreateSql;
		std::vector<std::string>::iterator SqlIt;
		for (it = strTables.begin(); it != strTables.end(); it ++)
		{
			if (!TableIsExists(it->c_str()))
			{
				if (pSrc->GetTableCreateSql(it->c_str(), strCreateSql))
				{
					for (SqlIt = strCreateSql.begin(); SqlIt != strCreateSql.end(); SqlIt ++)
					{
						if (!Execute(SqlIt->c_str()))
						{
							return FALSE;
						}
					} //end for (
				} //end if (pSrc->GetTableCreateSql
			} //end if (!TableIsExists(it->c_str())
		} //end for(

 
		for (it = strTables.begin(); it != strTables.end(); it ++)
		{
			if (!CopyFromDBOP(pSrc, it->c_str(), "*", it->c_str(), "*"))
				break;
		} //end for (it =
	} //end if (pSrc
	return bSucc;
}

BOOL CSqliteDBOP::Open(const char *szSql, char ***result , int &nrow , int &ncolumn)
{
	if (m_sqlite)
	{
		if (m_szErrorMsg)
		{
			delete []m_szErrorMsg;
			m_szErrorMsg = NULL;
		}
		int rc = sqlite3_get_table(m_sqlite, szSql, result, &nrow, &ncolumn, NULL);
		if (rc != SQLITE_OK)
		{
			int n = (int) strlen(sqlite3_errmsg(m_sqlite));
			m_szErrorMsg = new char[n + 1];
			memset(m_szErrorMsg, 0, n + 1);
			strncpy(m_szErrorMsg, sqlite3_errmsg(m_sqlite), n);
		}
		return (rc == SQLITE_OK);
	} 
	return FALSE;
}

int CSqliteDBOP::LastInsertRowId()
{
	if (m_sqlite)
	{
		return (int)sqlite3_last_insert_rowid(m_sqlite);
	}
	return 0;
}

void CSqliteDBOP::ClearError()
{
	if (m_szErrorMsg)
	{
		delete []m_szErrorMsg;
		m_szErrorMsg = NULL;
	}
}

BOOL CSqliteDBOP::GetLastszErrorMsg(char *szErroMsg, int nMaxLen)
{
	if (m_szErrorMsg)
	{
		strncpy(szErroMsg, m_szErrorMsg, nMaxLen);
		return TRUE;
	} else
		return FALSE;
}

BOOL CSqliteDBOP::GetEncoding(char *szEncoding)
{
	char **szResult = NULL;
	BOOL bSucc = FALSE;
	int nRow, nCol;
	if (Open("pragma encoding", &szResult, nRow, nCol))
	{
		if (nRow > 0)
		{
			strcpy(szEncoding, szResult[1]);
			bSucc = TRUE;
		}
	}
	CSqliteDBOP::Free_Result(szResult);
	return bSucc;
}

#pragma warning(disable:4996)
