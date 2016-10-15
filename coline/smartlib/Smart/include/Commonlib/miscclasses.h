#ifndef __MISCCLASSES_H__
#define __MISCCLASSES_H__

#include <vector>
#include <deque>

#include <CommonLib/GuardLock.h>

const DWORD MAXUDPSIZE = 1536;
const DWORD MAX_MSGBUFFERSIZE = 5000; //�ڻ�������󱣴��������¼
const DWORD MAX_CHUCK_SIZE    = 100; //������������̫��ʱ��������������

using namespace std;
typedef struct CUdpMessageItem {
	int nPeerIp;
	int nPeerPort;
	int nMsgSize;
	int nReSendTimes;       //ʣ����ط�����
	DWORD dwToUserId;       //����ʱʹ�ã����շ�ID��
	DWORD tmLastSendTime;   //�����ʱ��
	BOOL IsServerTrans;     //�Ƿ��ɷ�������ת����
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