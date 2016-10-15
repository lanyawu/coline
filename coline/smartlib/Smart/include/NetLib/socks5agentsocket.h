#ifndef __SOCKS5AGENTSOCKET_H___
#define __SOCKS5AGENTSOCKET_H___

#include <string>
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

class CSocks5AgentSocket
{
public:
	CSocks5AgentSocket(void);
	~CSocks5AgentSocket(void);
public:
	//����
	BOOL ConnectAgent(const char *szAgentIp, const WORD wPort, const char *szAgentName, 
		              const char *szAgentPwd, SOCKET hMapSocket);
	//TCP������Զ�̷�����
	BOOL ConnectServer(const char *szServerIp, const WORD wPort);
	//����UDP��Ϣ
	int SendUdp(SOCKET hSocket, const char *szBuff, const int nBufSize, const DWORD nDestIp, const WORD wDestPort);
	//����UDP��Ϣ
	int  RecvUdp(SOCKET hSocket, char *szBuff, int nBufSize, DWORD &nSrcIp, WORD &wSrcPort);
	//��������
    int SendTcp(const char *szBuff, const int nBufSize);
	//��������
	int RecvTcp(char *szBuff, int nBufSize);
	//�Ƿ������ݿɶ�
	BOOL CanRead(DWORD dwTimeout);
private:
	BOOL MapChannel(SOCKET hMapSocket);
	BOOL HandShake();
	BOOL CheckSocketLive();
	BOOL ReConnect();
private:
	struct sockaddr_in m_addrProxy;
	int m_nSocket; //
	SOCKET m_hMapSocket;
	std::string m_strAgentAddr;
	std::string m_strAgentName;
	std::string m_strAgentPwd;
	WORD m_nAgentPort;
	//
	int  m_nAddrType;
};

#endif