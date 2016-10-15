#include <Netlib/ASock.h>
#include <Commonlib/QueryAddressByIP.h>
#include <stdio.h>

#pragma warning(disable:4996)

CQueryAddressByIP::CQueryAddressByIP(char *szIpFileName, char *szKey):
                   m_SqliteDBOP(szIpFileName, szKey)
{
}

CQueryAddressByIP::~CQueryAddressByIP(void)
{
}

BOOL CQueryAddressByIP::QueryAddrByIP(int nIp, char *szAddress, size_t nMaxLen)
{
	char szSql[256] = {0};
	sprintf(szSql, "select province, address from iplab where (startip <= %u) and (endip >= %u);", nIp, nIp);
	char **szResult = NULL;
	int nRow, nCol;
	BOOL b = FALSE;
	CASocket::IntToIpAddress(szAddress, nIp);
	if (m_SqliteDBOP.Open(szSql, &szResult, nRow, nCol))
	{
		if (nRow > 0)
		{
			memset(szAddress, 0, nMaxLen);
			size_t len = strlen(szResult[2]);
			if (nMaxLen > len)
			{
				strncpy(szAddress, szResult[2], len);
				nMaxLen -= len;
			} else
			{
				strncpy(szAddress, szResult[2], nMaxLen);
				nMaxLen = 0;
			}
			size_t addrlen = strlen(szResult[3]);
			if ((nMaxLen > 0) && (nMaxLen > addrlen))
			{
				strncpy(szAddress + len, szResult[3], addrlen);
			} else
				strncpy(szAddress + len, szResult[3], nMaxLen);
			b = TRUE;
		}
	}
	m_SqliteDBOP.Free_Result(szResult);
	return b;
}

#pragma warning(default:4996)