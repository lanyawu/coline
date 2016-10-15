#ifndef __BASETCPSOCKET_H____
#define __BASETCPSOCKET_H____

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

#include <netlib/socks5agentsocket.h>

#define SOFT_ERROR(e)	((e) == WSAEINTR || \
	(e) == WSA_IO_PENDING || \
	(e) == WSAEWOULDBLOCK || \
	(e) == EINTR || \
	(e) == EAGAIN || \
	(e) == 0)

class CBaseTcpSocket
{
public:
	CBaseTcpSocket(void);
	~CBaseTcpSocket(void);
public:
	int RecvBuffer(char *lpBuff, int nBufSize, int nTimeout);
	int SendBuffer(const char *lpBuff, int nBufSize, int nTimeOut);
	BOOL Connect(int nIp, int nPort, int nTimeOut);
	BOOL Connect(const char *szIp, int nPort, int nTimeOut);
	BOOL SetBlock(BOOL bBlock);
	BOOL SetSendBuffSize(int nBufSize);
	BOOL SetRecvBuffSize(int nBufSize);
	BOOL IsCanReadBuff(int nTimeOut);
	BOOL GetSocketInfo(char *szIp, WORD &wPort);
	BOOL GetPeerSocketInfo(char *szIp, WORD &wPort);
	SOCKET Detach();
	//设置代理服务器
	BOOL SetAgent(const char *szAgentIp, const WORD wPort, const char *szAgentName, 
		          const char *szAgentPwd);
private:
	CSocks5AgentSocket m_AgentSocket;
	DWORD m_dwPeerIp;
	WORD  m_wPeerPort;
	BOOL m_bAgent;
	SOCKET m_hSocket;
};

#endif
