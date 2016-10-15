#ifndef __IOCOMPLETIONSOCKET_H____
#define __IOCOMPLETIONSOCKET_H____

#include <commonlib/types.h>
#include <commonlib/guardlock.h>
#include <netlib/packetstream.h>
#include <queue>
#include <winsock.h>

#define IO_COMPLETION_ACTIVE_TIME_INTERVAL  15000  //15秒超时

class CIoCompletionServer;
class CIoCompletionSocket
{
public:
	CIoCompletionSocket(CIoCompletionServer *pServer);
	virtual ~CIoCompletionSocket(void);
public:
	friend class CIoCompletionServer;
	//虚函数
	virtual BOOL ParseStream(CPacketStream *pStream);
	void Attach(SOCKET h);
	void CloseByPeer(); //被对方关闭
	BOOL IsValid(); //检测SOCKET 是否合法
	BOOL Read(CPacketStream *pPacket = NULL); //读取数据
	BOOL ReadConnectStatus(CPacketStream *pPacket = NULL); //读取连接状态
    virtual void WriteCompleted(); //写完未发送的数据
	BOOL WriteBuff(const char *pBuf, int nSize); //写入数据
	void ClearPackets(); //清除所有数据包
	//设置缓存
	void SetSocketBufferSize(const int nMaxBufSize = 0, const int nMinBufSize = 0);
	//获取socket Peer IP and Port
	BOOL GetPeerName(char *szPeerIp, WORD &wPeerPort);
	//计数器
	LONG  AddRef();
	LONG  Release();
	LONG  GetRef();
	SOCKET GetSocket();
    BOOL SocketIsTimeout(); //连接是否超时
    DWORD GetWritePendingSize(); //获取待发送队列中的个数
protected:
	BOOL ConnectSvr(const char *szIp, const WORD wPort); //连接服务器
	
 
protected:
	CIoCompletionServer *m_pServer;
private:
	volatile LONG m_lWritting; //判断是否正在写入的锁
	volatile LONG m_lRef; //计数器
	BOOL m_bPauseRead; //是否处于暂停状态
	DWORD m_dwActiveTime; //最后活动时间
	CGuardLock m_wLock; //写入锁
	CGuardLock m_Lock; //待发送数据锁
	std::deque<CPacketStream *> m_sqList; //待发送对列
	SOCKET m_hSocket;
};

#endif