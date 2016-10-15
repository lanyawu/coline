#ifndef __ASYNCTCPSOCKET_H__
#define __ASYNCTCPSOCKET_H__

#include <deque>
#include <Netlib/asock.h>
#include <Commonlib/GuardLock.h>

#define MAX_ASYNCTCP_SND_BUFFER_SIZE 32 //最大缓存包
using namespace std;


//发送数据缓存
typedef struct tagAsyncDataItem
{
	char *lpBuf;
	int nBufLen;
}ADI, *LPADI;

//异步TCP Socket处理线程
class CAsyncTcpSocket :public CASocket
{
public:
	CAsyncTcpSocket(void);
	~CAsyncTcpSocket(void);
public:
	//CASocket 基类虚函数
	virtual void OnSend(int nErrorCode);
    virtual void OnConnect(int nErrorCode);

    //发送数据
    int SendBuff(char *lpBuff, int &BufLen);
	//发送数据
	int SendBuff(const char *lpBuff, int BufLen, int nTimeOut);
	//接收数据
	int RecvBuffer(char *lpBuff, int BufLen, DWORD dwTimeOut);
	BOOL IsSendComplete(); //测试是否已经发送完毕
	//连接服务器
	virtual BOOL ConnectPeer(DWORD dwIp, WORD wPort, DWORD dwTimeOut);
	//
	BOOL GetConnected();
	//
	BOOL IsCanWriteBuff(int nTimeOut);
	//
	BOOL IsCanReadBuff(int nTimeOut);
	//
	void SetConnectStatus(BOOL bConnectStatus);
private:
	static DWORD WINAPI SendDataThread(LPVOID lpParam); //数据发送线程
	void ClearSendBuffer(); //清除发送缓存区
protected:
	BOOL m_bTerminated; //是否已经中止
	BOOL   m_bConnected; //是否已经连接
private:
	deque<LPADI> m_listADI; //待发送的数据列表
	CGuardLock  m_lockADI; //队列锁
	HANDLE m_hConnectEvent; //连接通知事件
	HANDLE m_hSendEvent; //发送通知事件
	HANDLE m_hEmptyEvent;
	HANDLE m_hSndThread; //发送线程
};

class IAsyncTcpServerApp
{
public:
	//接收到连接请求
	virtual void OnAccept(CAsyncTcpSocket *pSocket, CNetAddress &addr) = 0;
	//接收到数据
	virtual void OnReceiveBuffer(char *lpBuffer, int nBufSize, CAsyncTcpSocket *pSocket) = 0;
};

class CAsyncTcpSocketServer: public CAsyncTcpSocket
{
public:
	CAsyncTcpSocketServer(IAsyncTcpServerApp *pApp);
	~CAsyncTcpSocketServer();
public:
	void OnAccept(int nErrorCode);
	BOOL Start(WORD wPort);
protected:
	IAsyncTcpServerApp *m_pApp;
};

#endif