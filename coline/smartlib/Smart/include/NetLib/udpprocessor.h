#ifndef __ITMSG_UDPPROCESSOR_H__
#define __ITMSG_UDPPROCESSOR_H__

//UDP消息处理器
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <winsock2.h>

#include <netlib/MiscClasses.h>
#include <CommonLib/GuardLock.h>
#include <commonlib/MemoryUserList.h>


const DWORD RESEND_TIME_INTERVAL   = 5000; //重发时间， 以毫秒为单位
const DWORD PER_CHECK_COUNT        = 1000; //每次检测的消息个数
const int MESSAGE_TIMEOUT_INTERVAL = 5000; //超过多少毫秒的消息为过期消息，不处理


typedef struct tag_WorkParam
{
	LPVOID MsgInstance;
	HANDLE Event;
}THREADWORDPARAM, *LPTHREADWORKPARAM;

class IUdpProcessApp
{
public:
	virtual void OnRecvUdpMsg(LPUDPMESSAGEITEM pItem) = 0;
	virtual void OnSendMessageFail(LPUDPMESSAGEITEM pItem) = 0; //消息发送失败
};


//class CWorkThread;

class CUdpProcessor
{
public:
	CUdpProcessor(IUdpProcessApp *pApp, WORD dwWorkThdCount);
public:
	~CUdpProcessor(void);
public:
	//虚函数
	virtual void OnReceive(int nErrorCode);
	virtual void OnSend(int nErrorCode);
public:
	//外部调用函数
	void SendMsgTo(const char *lpBuff, const int nSize, const int nPeerIp, const int nPort,
		           const int ReSendTimes = 0, const DWORD dwToUserId = 0);
	//
	void SendMsgTo(const char *pBuf1, const int nBuf1Size, const char *pBuf2, const int nBuf2Size, 
		           const int nPeerIp, const int nPort, const int ReSendTimes = 0, const DWORD dwToUserId = 0);
	//外部调用函数
	void SendMsg(LPUDPMESSAGEITEM pItem);

	//终止
	void Terminate();
	//启动UDP监听
	bool Start(int nPort);

	static void IntToIpAddress(char *szAddress, DWORD dwIp);
	static char *ByteToString(char *szByte, BYTE bit);

	//调试用
	DWORD GetSendMsgSize() { return m_pSndMsgList.GetSize(); };
	DWORD GetRcvMsgSize() { return m_pRcvMsgList.GetSize(); };
protected:
	virtual void OnRecvUdpMessage(LPUDPMESSAGEITEM pItem);
	virtual void OnSendUdpFail(LPUDPMESSAGEITEM pItem); //消息发送失败
	virtual BOOL CheckIsAckMessage(const LPUDPMESSAGEITEM pItem); //是否是回复信息
	virtual DWORD GetKeyFromMessage(const LPUDPMESSAGEITEM pItem) = 0; //从消息中获取KEY
private:
	//线程函数
	static DWORD WINAPI CheckRcvDataThread(LPVOID pParam);   //检测是否有消息数据
	static DWORD WINAPI RcvMsgProcessThread(LPVOID pParam); //接收消息处理线程
	static DWORD WINAPI SndMsgProcessThread(LPVOID pParam); //发送消息处理线程
	static DWORD WINAPI ReSendMessageThread(LPVOID pParam); //重发消息线程
	static DWORD WINAPI _WorkThread(LPVOID pParam); //工作者线程

	void RcvMsgProcess();
	void SndMsgProcess();

	void CheckAckMessage(LPUDPMESSAGEITEM pItem);
	void SendMessageItem(LPUDPMESSAGEITEM pItem);
	void Clear();
	//插入到重发列表
	void InsertResendList(LPUDPMESSAGEITEM pItem);
	DWORD GetCurrSeq();
protected:
	IUdpProcessApp *m_pApp;
	DWORD m_dwCurrSeq; //当前消息包序号
	CPointerDataMap m_ReSendList; //重发消息列表 
    SOCKET m_hSocket;  //当前socket

	CSafeUdpMessageVector m_pRcvIdleList; //接收空闲列表
	CSafeUdpMessageVector m_pSndIdleList; //发送空闲列表
    CSafeUdpMessageDeque   m_pRcvMsgList;  //未处理的接收消息列表
	CSafeUdpMessageDeque   m_pSndMsgList;  //待发送的消息列表
    
    vector<HANDLE>       m_WorkThreadEvents;
	//对应队列锁

	CGuardLock  m_LockWorkEvents; //工作线程锁
    CGuardLock m_ReSendLock; //重发消息列表锁
	CGuardLock m_SeqLock; //序号锁
	sockaddr_in m_fromAddr;
	sockaddr_in m_toAddr;
	WORD m_dwWorkThdCount; //工作线程个数
	int m_sockaddrLen;
	HANDLE m_RcvThread; //接收线程
	HANDLE m_SndThread; //发送线程
	HANDLE m_ReSndThread; //重发线程
	HANDLE m_CheckRcvDataThread; //检测数据线程
	HANDLE m_RcvEvent;  //接收到消息的事件
	HANDLE m_SndEvent;  //发送消息事件
	HANDLE m_hSendFail; //消息发送失败事件
	HANDLE m_hBreak;   //中止事件

	bool m_bTerminated;

	//最小的数据包大小
	static int UDP_MIN_PACKET_SIZE;
};



#endif