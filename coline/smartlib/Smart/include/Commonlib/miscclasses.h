#ifndef __MISCCLASSES_H__
#define __MISCCLASSES_H__

#include <vector>
#include <deque>

#include <CommonLib/GuardLock.h>

const DWORD MAXUDPSIZE = 1536;
const DWORD MAX_MSGBUFFERSIZE = 5000; //在缓存中最大保存多少条记录
const DWORD MAX_CHUCK_SIZE    = 100; //当缓存中数据太多时，丢弃多少条？

using namespace std;
typedef struct CUdpMessageItem {
	int nPeerIp;
	int nPeerPort;
	int nMsgSize;
	int nReSendTimes;       //剩余的重发次数
	DWORD dwToUserId;       //发送时使用，接收方ID号
	DWORD tmLastSendTime;   //最后发送时间
	BOOL IsServerTrans;     //是否由服务器中转过来
	char strMessage[MAXUDPSIZE];
}UDPMESSAGEITEM, *LPUDPMESSAGEITEM;

class COMMONLIB_API CSafeUdpMessageVector
{
public:
	CSafeUdpMessageVector();
	~CSafeUdpMessageVector();
public:
	void Insert(LPUDPMESSAGEITEM pItem);
	LPUDPMESSAGEITEM GetItem();
	void Clear();
private:
	vector<LPUDPMESSAGEITEM> m_List;
	CGuardLock m_Lock;
};

class COMMONLIB_API CSafeUdpMessageDeque
{
public:
	CSafeUdpMessageDeque();
	virtual ~CSafeUdpMessageDeque();
public:
	void Insert(LPUDPMESSAGEITEM pItem);
	void InsertFront(LPUDPMESSAGEITEM pItem);
	DWORD GetSize() {return  (DWORD)m_List.size(); };
	void Clear();
	LPUDPMESSAGEITEM GetItem();
private:
	deque<LPUDPMESSAGEITEM> m_List;
	CGuardLock m_Lock;
};

#endif