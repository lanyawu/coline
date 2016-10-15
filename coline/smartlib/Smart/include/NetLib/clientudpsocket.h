#ifndef __CLIENTUDPSOCKET_H__
#define __CLIENTUDPSOCKET_H__
#include <winsock2.h>
#include <commonlib/types.h>
#include <NetLib/MiscClasses.h>
#include <commonlib/MemoryUserList.h>
#include <NetLib/socks5agentsocket.h>

const DWORD RESEND_TIME_INTERVAL   = 3000; //重发时间， 以毫秒为单位
const DWORD PER_CHECK_COUNT        = 1000; //每次检测的消息个数
const int MESSAGE_TIMEOUT_INTERVAL = 5000; //超过多少毫秒的消息为过期消息，不处理


class IClientUdpProcessApp
{
public:
	virtual void OnRecvUdpMsg(LPUDPMESSAGEITEM pItem) = 0;
	virtual void OnSendMessageFail(LPUDPMESSAGEITEM pItem) = 0; //消息发送失败
};
class CClientUdpSocket
{
public:
	CClientUdpSocket(IClientUdpProcessApp *pApp);
public:
	~CClientUdpSocket(void);
public:
	//外部调用函数
	void SendMsgTo(char *lpBuff, int nSize, int nPeerIp, int nPort, int ReSendTimes = 0, DWORD dwToUserId = 0);
	
	//外部调用函数
	void SendMsg(LPUDPMESSAGEITEM pItem);

	//终止
	void Terminate();
	//获取本SOCKET IP地址
	DWORD GetSocketIP();

	//启动UDP监听
	bool Start(int nPort);
	//内部端口
	WORD GetIntranetPort();
	WORD GetInternetPort();
	void SetInternetPort(WORD wPort);
	DWORD GetCurrSeq();
	//清除所有待发信息
	void Clear();
	//设置代理服务器信息
	BOOL SetAgentAddr(const char *szAgentIp, const WORD wPort, const char *szAgentName, 
		              const char *szAgentPwd);
private:
	BOOL GetSocketName(char *szAddress, WORD &wPort);
protected:
	//子类实现的虚函数
	virtual DWORD GetMessageSeqId(LPUDPMESSAGEITEM pItem) { return 0; } //从消息中获取seq id 号
	virtual BOOL  IsAckPacket(LPUDPMESSAGEITEM pItem) { return FALSE; }; //检测是否是一个回复包
	virtual void OnRecvUdpMessage(LPUDPMESSAGEITEM pItem);
	virtual void OnSendUdpFail(LPUDPMESSAGEITEM pItem); //消息发送失败	
	
	//插入到重发列表
	void InsertResendList(LPUDPMESSAGEITEM pItem);	
	//检查是否是合法数据
	virtual BOOL IsValidPacket(LPUDPMESSAGEITEM pItem) { return TRUE;  };
private:
	//线程函数
	static DWORD WINAPI CheckRcvDataThread(LPVOID pParam);   //检测是否有消息数据
	static DWORD WINAPI RcvMsgProcessThread(LPVOID pParam); //接收消息处理线程
	static DWORD WINAPI SndMsgProcessThread(LPVOID pParam); //发送消息处理线程
	static DWORD WINAPI ReSendMessageThread(LPVOID pParam); //重发消息线程

	BOOL CheckAckMessage(LPUDPMESSAGEITEM pItem);
	void SendMessageItem(LPUDPMESSAGEITEM pItem);
protected:
	//消息列表
	CSafeUdpMessageVector  m_pRcvIdleList; //接收空闲列表
	CSafeUdpMessageVector  m_pSndIdleList; //发送空闲列表
    CSafeUdpMessageDeque   m_pRcvMsgList;  //未处理的接收消息列表
	CSafeUdpMessageDeque   m_pSndMsgList;  //待发送的消息列表	
	
	//事件列表
	HANDLE m_RcvEvent;
	HANDLE m_hBreak;	
	HANDLE m_SndEvent;
private:
	IClientUdpProcessApp *m_pApp;
	CSocks5AgentSocket m_AgentSocket; //代理
	BOOL m_bAgent; //是否采用代理
	SOCKET m_hSocket;
	CPointerDataMap m_ReSendList; //重发消息列表 
	CGuardLock m_ReSendLock;
	sockaddr_in m_toAddr;

	//线程列表
	HANDLE m_RcvThread;
	HANDLE m_SndThread;
	HANDLE m_ReSndThread;
	HANDLE m_CheckRcvDataThread;
    
	//序号锁
	CGuardLock m_SeqLock;

	DWORD m_dwSeq;
	WORD m_wIntranetPort; //内部端口
	WORD m_wInternetPort; //外部端口
	bool m_bTerminated;
};

#endif