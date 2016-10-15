#ifndef __ASYNC_ASOCKET_H__
#define __ASYNC_ASOCKET_H__


#define MAXEVENTCOUNT 63


#define NO_ENOUGH_SOCKET_SLOT 13001
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

#define SOFT_ERROR(e)	((e) == WSAEINTR || \
	(e) == WSA_IO_PENDING || \
	(e) == WSAEWOULDBLOCK || \
	(e) == EINTR || \
	(e) == EAGAIN || \
	(e) == 0)

#ifdef SOCKETPLUS_EXPORTS
 #define ASOCKET_API __declspec(dllexport)
#else
 #define ASOCKET_API __declspec(dllimport)
#endif

//�����ַ
class ASOCKET_API CNetAddress
{
public:
	CNetAddress();
	~CNetAddress();
public:
	CNetAddress & operator = (const CNetAddress &addr) ;
	CNetAddress & operator = (struct sockaddr_in &addr);
	CNetAddress & operator = (struct sockaddr &addr);
	void GetAddress(struct sockaddr &addr);
	void SetAddress(DWORD dwIp, WORD wPort);
private:
	struct sockaddr_in m_addr;
};

class ASOCKET_API CASocket
{
public:
	CASocket();
	virtual ~CASocket();
protected:
	WORD m_wBindPort;	
public:
	SOCKET m_hSocket;

	virtual BOOL Create(UINT nSocketPort = 0, int nSocketType=SOCK_STREAM,
		        long lEvent = FD_READ | FD_WRITE | FD_ACCEPT | FD_CONNECT | FD_CLOSE, char *lpszSocketAddress = NULL);
	
	BOOL GetPeerName(char *rPeerAddress, UINT& rPeerPort);
	BOOL GetSockName(char *rSocketAddress, UINT& rSocketPort);

	virtual BOOL Accept(CASocket& rConnectedSocket,	SOCKADDR* lpSockAddr = NULL, int* lpSockAddrLen = NULL);
    //�Ƿ����������ʽ
	void SetBlock(BOOL bBlock);

	BOOL Bind(UINT nSocketPort, char *lpszSocketAddress = NULL);
	BOOL Connect(const char *lpszHostAddress, const UINT nHostPort);
	BOOL Listen(int nConnectionBacklog=5);
    int Receive(void* lpBuf, int nBufLen, int nFlags = 0);
    int Send(const void* lpBuf, int nBufLen, int nFlags = 0);
	//����Socket����������
	BOOL SetSendBuffSize(DWORD dwBufferSize);
	//����Socket����������
	BOOL SetRecvBuffSize(DWORD dwBufferSize);

	virtual void Close();

	BOOL AsyncSelect(long lEvent = FD_READ | FD_WRITE | FD_ACCEPT | FD_CONNECT | FD_CLOSE);

	virtual void OnReceive(int nErrorCode){}
	virtual void OnSend(int nErrorCode){}
	virtual void OnOutOfBandData(int nErrorCode){}
	virtual void OnAccept(int nErrorCode){}
	virtual void OnConnect(int nErrorCode){}
	virtual void OnClose(int nErrorCode){}

	BOOL Socket(int nSocketType=SOCK_STREAM, long lEvent = FD_READ | FD_WRITE | FD_ACCEPT | FD_CONNECT | FD_CLOSE, int nProtocolType = 0, int nAddressFormat = PF_INET);
	WORD GetBindPort();
	BOOL SetKeepLive();
	static BOOL ASocketInit(void);
	static BOOL ASocketDestroy();
	
	//��ַת����غ���
	//IPת�ַ���
	static void IntToIpAddress(char *szAddress, DWORD dwIp);
	//�ַ���תIP
    static int StringIPToInt(const char *szAddress);
	//��ȡ���е�UDP�˿�
	static WORD GetFreeUdpPort(WORD wStartPort, WORD wTryTimes);
	//��ȡ���е�TCP�˿�
	static WORD GetFreeTcpPort(WORD wStartPort, WORD wTryTimes);
	//��������
	static DWORD GetIPByHostName(const char *szHostName);

};

#endif //end __ASYNC_ASOCKET_H__ 