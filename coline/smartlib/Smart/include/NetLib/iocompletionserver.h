#ifndef __IOCOMPLETIONSERVER_H___
#define __IOCOMPLETIONSERVER_H___

#include <netlib/iocompletionport.h>
#include <netlib/iocompletionsocket.h>
#include <vector>
#include <MSWSock.h>

#define DEFAULT_PACKET_SIZE  4096  //默认数据包大小
#define DEFAULT_ACCEPT_TIMES 16

//TCP服务器相关参数
#define DEFAULT_KEEPALIVETIME		15000
#define DEFAULT_KEEPALIVEINTERVAL   3000

//IOCP服务器
class CIoCompletionServer
{
public:
	CIoCompletionServer(DWORD dwPktCount, DWORD dwPktSize);
	~CIoCompletionServer(void);
public:
    BOOL Start(const char *szServerIp, WORD wPort);  //启动监听
	BOOL Stop(); //停止工作
    
	//分配一个SOCKET
    virtual CIoCompletionSocket * AllocateSocket(SOCKET s,sockaddr_in * lpLocalAddr,sockaddr_in * lpRemoteAddr) = 0;
	//处理数据
	virtual BOOL ParseStream(CPacketStream *pStream, CIoCompletionSocket *pSocket);
    //关联一个SOCKET 到IOCP
	virtual BOOL AssociateDevice(SOCKET h, CIoCompletionSocket *pSocket);
	//关联一个SOCKET 到IOCP 测试是否连接成功
	virtual BOOL AssociateConnectStatus(SOCKET h, CIoCompletionSocket *pSocket);
    //分配一段内存
	CPacketStream *AllocatePacket();
	DWORD GetPacketSize();
	//debug 打印内存状态
	void PrintAllocatorStatus();
	//消息处理
	//Post IO操作
	void PostIoOperation(CPacketStream::CIoOperationType nType, CIoCompletionSocket *pSocket, CPacketStream *pStream);
	//IO操作
	void DoIoOperation(CIoCompletionSocket *pSocket, CPacketStream *pStream, DWORD dwIoSize, DWORD dwLastError);
	//读取请求消息
	void DoReadRequest(CIoCompletionSocket *pSocket, CPacketStream *pStream, DWORD dwIoSize, DWORD dwLastError);
	//读取完成
	void DoReadCompletion(CIoCompletionSocket *pSocket, CPacketStream *pStream, DWORD dwIoSize, DWORD dwLastError);
	//写入请求
	void DoWriteRequest(CIoCompletionSocket *pSocket, CPacketStream *pStream, DWORD dwIoSize, DWORD dwLastError);
    //写入完成
	void DoWriteCompletion(CIoCompletionSocket *pSocket, CPacketStream *pStream, DWORD dwIoSize, DWORD dwLastError);
	//接收到连接请求
	void DoAcceptCompletion(CIoCompletionSocket *pSocket, CPacketStream *pStream, DWORD dwIoSize, DWORD dwLastError);
    //连接成功
	void DoSocketConnect(CIoCompletionSocket *pSocket, CPacketStream *pStream, DWORD dwIoSize, DWORD dwLastError);
	//连接完成
	void DoConnectComplete(CIoCompletionSocket *pSocket, CPacketStream *pStream, DWORD dwIoSize, DWORD dwLastError);
protected:
	//事件
	virtual void OnClientClose(CIoCompletionSocket *pSocket, DWORD dwError); //客户端关闭
public:
	LPFN_ACCEPTEX   m_pAcceptEx;							// AcceptEx函数指针 
	LPFN_GETACCEPTEXSOCKADDRS m_pGetAcceptExSockaddrs;	    // GetAcceptExSockaddrs函数指针
private:
	//读取数据
	void Read(CIoCompletionSocket *pSocket, CPacketStream *pStream);
	//写入数据
	void Write(CIoCompletionSocket *pSocket, CPacketStream *pStream);
	//接收连接请求
	void PostAcceptEx(SOCKET s, int n); //
    //初始化函数指针
	void GetFunctionPointer();
	//线程
	static DWORD WINAPI ListenThread(LPVOID lpParam); //监听线程
	static DWORD WINAPI WorkThread(LPVOID lpParam); //工作线程
private:
	CIoCompletionPort m_IOCP;
	//内存分配器
	CPacketAllocator m_Allocator;
	BOOL m_bTerminated; //是否中止

	static GUID m_GUIDAcceptEx;
	static GUID m_GUIDGetAcceptExSockAddrs;
	//监听有关的变量
	SOCKET m_hListenSocket;
	HANDLE m_hListenEvent;
	HANDLE m_hListen;

	//工作线程相关
	std::vector<HANDLE> m_WorkThreadList;

	//debug
	volatile LONG m_nAllocSocket;
	volatile LONG m_nFreeSocket;
};

#endif