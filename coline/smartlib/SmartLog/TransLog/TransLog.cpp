// TransLog.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <commonlib/sqlitedbop.h>
#include <crypto/crypto.h> 
#include <commonlib/debuglog.h>

#define LOGLIST_CREATE_TABLE_SQL "create table loglist(id INTEGER PRIMARY KEY, parentlog INTEGER, logname VARCHAR(256),\
                                  createdate VARCHAR(64), lastmodidate VARCHAR(64), readtimes INTEGER, currlines INTEGER, orderseq INTEGER,\
                                  srclen INTEGER, logtext BLOB, plaintext BLOB);" //日志表记录
 
int _tmain(int argc, _TCHAR* argv[])
{
	/*CSqliteDBOP srcDb("F:\\lanya\\documents\\smartlog\\wxz.blog", "w~x@y$z^");
	CSqliteDBOP dstDb("F:\\temp.blog", "w~x@y$z^");
	if (!dstDb.TableIsExists("loglist"))
		dstDb.Execute(LOGLIST_CREATE_TABLE_SQL);
	std::string strSql;
	char szTmp[16] = {0};
	for (int i = 0; i < 1000; i ++)
	{
		strSql = "select parentlog, logname,createdate,lastmodidate,readtimes,currlines,orderseq,srclen from loglist where id=";
		strSql += ::itoa(i, szTmp, 10);
		char **szResult =  NULL;
		int nRow, nCol;
		if (srcDb.Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			if (nRow < 1)
				continue;
			strSql = "insert into loglist(parentlog,logname,createdate,lastmodidate,readtimes,currlines,orderseq,srclen) values('";
			if (szResult[8])
				strSql += szResult[8];
			strSql += "','";
			if (szResult[9])
				strSql += szResult[9];
			strSql += "','";
			if (szResult[10])
				strSql += szResult[10];
			strSql += "','";
			if (szResult[11])
				strSql += szResult[11];
			strSql += "','";
			if (szResult[12])
				strSql += szResult[12];
			strSql += "','";
			if (szResult[13])
				strSql += szResult[13];
			strSql += "','";
			if (szResult[14])
				strSql += szResult[14];
			strSql += "','";
			if (szResult[15])
				strSql += szResult[15];
			strSql += "')";
			if (dstDb.Execute(strSql.c_str()))
			{
				char *pBuf = NULL;
				DWORD dwSize = 0;
				strSql = "select logtext from loglist where id=";
				strSql += ::itoa(i, szTmp, 10);
				if (srcDb.GetBlob(strSql.c_str(), &pBuf, dwSize, 0))
				{
					strSql = "update loglist set logtext=? where id=";
					strSql += ::itoa(i, szTmp, 10);
					dstDb.InsertBlob(strSql.c_str(), pBuf, dwSize);
				}
				if (pBuf)
					delete []pBuf;
				pBuf = NULL;
				dwSize = 0;
				strSql = "select plaintext from loglist where id=";
				strSql += ::itoa(i, szTmp, 10);
				if (srcDb.GetBlob(strSql.c_str(), &pBuf, dwSize, 0))
				{
					strSql = "update loglist set plaintext=? where id=";
					strSql += ::itoa(i, szTmp, 10);
					dstDb.InsertBlob(strSql.c_str(), pBuf, dwSize);
				}
				if (pBuf)
					delete []pBuf;
			}
		}
		if (szResult)
			srcDb.Free_Result(szResult);
		printf(".");
		if ((i % 20) == 0)
			printf("\n%d", i);
	}
	//std::string strSql = "update loglist set readtimes=0, currlines=0";
 */
 
	return 0;
}

