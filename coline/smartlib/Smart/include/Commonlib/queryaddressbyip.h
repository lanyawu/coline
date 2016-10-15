#ifndef __QUERYADDRESSBYIP_H__
#define __QUERYADDRESSBYIP_H__

#include <Commonlib/SqliteDBOP.h>

//根据用户的IP查询所在地
class COMMONLIB_API CQueryAddressByIP
{
public:
	CQueryAddressByIP(char *szIpFileName, char *szKey);
	~CQueryAddressByIP(void);
public:
	BOOL QueryAddrByIP(int nIp, char *szAddress, size_t nMaxLen);
private:
	CSqliteDBOP m_SqliteDBOP;
};

#endif