#ifndef __UDPSOCKET_H__
#define __UDPSOCKET_H__

#define THREAD_WAIT_TIME_OUT_INTERVAL 5000  //线程等待超时
#include <NetLib/ASock.h>
#include <Commonlib/Types.h>
#include <Netlib/udt.h>

#define SELECT_HAS_DATA_READ  1  //有数据可读取
#define SELECT_READ_TIME_OUT  0  //超时
#define SELECT_READ_ERROR     -1 //socket损坏

//UDT服务器实例接口定义
class IUDTTransServerApp
{
public:
	virtual void OnAccept(UDTSOCKET u, CNetAddress &addr) = 0;
};

class CUDPTransSocket
{
public:
	CUDPTransSocket(void);
	~CUDPTransSocket(void);

public:
	static void InitUDT();
	static void CleanUpUDT();
	BOOL Bind(short nPort); //绑定端口
	BOOL Listen(); //启动监听
	int64_t SendFile(char *szFileName);
	int64_t RecvFile(char *szFileName, int64_t size);
    BOOL Connect(int nIp, short nPort, DWORD dwTimeOut);
	void Close();
	//是否能读取数据
	int CanRead(const DWORD dwTimeOut);
	int SendBuffer(const char *lpBuffer, int nSize);
	int RecvBuffer(char *lpBuffer, int nSize);
    BOOL GetSocketInfo(char *szIp, WORD &wPort);
	BOOL GetPeerSocketInfo(char *szIp, WORD &wPort);
	//虚函数
	virtual UDTSOCKET accept(struct sockaddr* addr, int* addrlen);

	//通过最底层socket发送消息，绕过ＵＤＴ机制
	int SendRawData(char *lpBuffer, int nSize, DWORD dwIp, WORD wPort);
    
	//传送速率 byte/s
	int GetSendRate();
	//接收速度 byte/s
	int GetRecvRate();
	//清除统计数据
	void ClearRate();
	//赋值
	void Attach(UDTSOCKET u); //赋值一个新的UDT 句柄
	//开始传输
	void StartTransfer();
	//中止传输
	void Teminate();
	//等待缓存发送完毕
	void WaitSendComplete();
	//获取错误
	static UDT::ERRORINFO & GetLastError();
	//
	int GetUdtSocketName(sockaddr *addr, int *nlen);
	//设置缓存区
	BOOL SetSndBufferSize(DWORD dwBufSize);
	BOOL SetRecvBufferSize(DWORD dwBufSize);
	//设置异步方式
	BOOL SetSyncBlock(bool bSync);
	//获取缓冲区大小
	int  GetCurrSndBufferSize();
	int  GetCurrRcvBufferSize();
public:
	DWORD m_dwInternetIP; //对外监听ＩＰ地址
	WORD m_wInternetPort; //对外监听端口
private:
	HANDLE m_hWaitEvent; //等待事件
	UDTSOCKET m_hSocket;
};

//UDT服务器类
class CUDPTransSocketServer
{
public:
	CUDPTransSocketServer(IUDTTransServerApp *pApp);
	~CUDPTransSocketServer();
public:
	void SetInternetIP(DWORD dwIp);
	void SetInternetPort(WORD wPort);
	DWORD GetInternetIP();
	WORD  GetInternetPort();
	BOOL  StartListen(WORD wPort);
	WORD  GetListenPort();
	//通过最底层socket发送消息，绕过ＵＤＴ机制
	int SendRawData(char *lpBuffer, int nSize, DWORD dwIp, WORD wPort);
	//设置发送和接收缓存区大小
	BOOL SetSocketBufferSize(int nSndBufSize, int nRcvBufSize);
	//
	BOOL SetSocketSync(bool bSync);
private:
	static DWORD WINAPI QueryAcceptThread(LPVOID lpParam);
private:
	CUDPTransSocket m_UdtSocket;
	IUDTTransServerApp *m_pApp;
	WORD m_wListenPort;
	HANDLE m_hThread; //线程
	BOOL m_bTerminated;

};

#endif