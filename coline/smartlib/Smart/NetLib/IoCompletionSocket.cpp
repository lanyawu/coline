#include <commonlib/debuglog.h>
#include <netlib/IoCompletionSocket.h>
#include <netlib/iocompletionserver.h>

#define MAX_SOCKET_BUFFER_SIZE 2097152
#define MIN_SOCKET_BUFFER_SIZE 65536

#pragma warning(disable:4996)

CIoCompletionSocket::CIoCompletionSocket(CIoCompletionServer *pServer):
                     m_hSocket(INVALID_SOCKET),
					 m_lWritting(0),
					 m_lRef(0),
					 m_pServer(pServer)
{
	m_dwActiveTime = ::GetTickCount();
	AddRef();
}

CIoCompletionSocket::~CIoCompletionSocket(void)
{
	ClearPackets();
	if (m_hSocket != INVALID_SOCKET)
		::closesocket(m_hSocket);
	m_hSocket = INVALID_SOCKET;
}

 

//��ȡ�����Ͷ����еĸ���
DWORD CIoCompletionSocket::GetWritePendingSize()
{
	return (DWORD)m_sqList.size();
}

//�����Ƿ�ʱ
BOOL CIoCompletionSocket::SocketIsTimeout()
{
	return (::GetTickCount() > (m_dwActiveTime + IO_COMPLETION_ACTIVE_TIME_INTERVAL));
}

SOCKET CIoCompletionSocket::GetSocket()
{
	return m_hSocket;
}

//д������
BOOL CIoCompletionSocket::WriteBuff(const char *pBuf, int nSize)
{
	if (!IsValid())
	{
		PRINTDEBUGLOG(dtInfo, "Invalid socket status");
		return FALSE;
	}
	PRINTDEBUGLOG(dtInfo, "Write Buffer size:%d", nSize);
	m_dwActiveTime = ::GetTickCount();
	int nMaxSize = m_pServer->GetPacketSize();
	CPacketStream *pStream = m_pServer->AllocatePacket();
	int nPktSize = min(nMaxSize, nSize);
	pStream->Write(pBuf, nPktSize);
	m_Lock.Lock();
	if (::InterlockedExchangeAdd(&m_lWritting, 0) > 0)
	{
		//���ڷ���
		pStream->AddRef();
	    m_sqList.push_back(pStream);
		//����ʣ���
		nSize -= nPktSize;
		CPacketStream *pTmp;
		while (nSize >0)
		{	
			pBuf = pBuf + nPktSize;
			nPktSize = min(nMaxSize, nSize);
			pTmp = m_pServer->AllocatePacket();
			pTmp->Write(pBuf, nPktSize);
			pTmp->AddRef();
	        m_sqList.push_back(pTmp);
			pTmp->Release();
			nSize -= nPktSize;
		}
	} else
	{
		::InterlockedIncrement(&m_lWritting);
		
		//����ʣ���
		nSize -= nPktSize;
		CPacketStream *pTmp;
		while (nSize >0)
		{	
			pBuf = pBuf + nPktSize;
			nPktSize = min(nMaxSize, nSize);
			pTmp = m_pServer->AllocatePacket();
			pTmp->Write(pBuf, nPktSize);
			pTmp->AddRef();
	        m_sqList.push_back(pTmp);
			pTmp->Release();
			nSize -= nPktSize;
		}
		m_pServer->PostIoOperation(CPacketStream::IO_TYPE_WRITE_REQUEST, this, pStream);
	}	
	m_Lock.UnLock();
	pStream->Release();   

 	return TRUE;
}

 //���Է��ر�
void CIoCompletionSocket::CloseByPeer()
{
	if (m_hSocket != INVALID_SOCKET)
	{
		::closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
	}
}

//��ȡsocket Peer IP and Port
BOOL CIoCompletionSocket::GetPeerName(char *szPeerIp, WORD &wPeerPort)
{
	if (m_hSocket == INVALID_SOCKET)
		return FALSE;
	SOCKADDR_IN sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);	
	BOOL bResult = (getpeername(m_hSocket, (SOCKADDR*)&sockAddr, &nSockAddrLen) != SOCKET_ERROR);
	if (bResult)
	{
		wPeerPort = ntohs(sockAddr.sin_port);
		strcpy(szPeerIp, inet_ntoa(sockAddr.sin_addr));
	}
	return bResult;
}

//���SOCKET �Ƿ�Ϸ�
BOOL CIoCompletionSocket::IsValid()
{
	return (m_hSocket != INVALID_SOCKET);
}

//����������ݰ�
void CIoCompletionSocket::ClearPackets()
{
	CGuardLock::COwnerLock guard(m_Lock);
	CPacketStream *pStream = NULL;
	while (!m_sqList.empty())
	{
		pStream = m_sqList.front();
		pStream->Release();
		m_sqList.pop_front();
	}
}

void CIoCompletionSocket::Attach(SOCKET h)
{
	if (m_hSocket != INVALID_SOCKET)
		::closesocket(m_hSocket);
	m_hSocket = h;
}

//�麯��
BOOL CIoCompletionSocket::ParseStream(CPacketStream *pStream)
{
	pStream->Initialize();
	return TRUE;
}

//���û���
void CIoCompletionSocket::SetSocketBufferSize(const int nMaxBufSize, const int nMinBufSize)
{
	if (m_hSocket != INVALID_SOCKET)
	{
		//
		int nBufSize = MAX_SOCKET_BUFFER_SIZE;
		int nLowBufSize = MIN_SOCKET_BUFFER_SIZE;
		if (nMaxBufSize > 0)
			nBufSize = nMaxBufSize;
		if (nMinBufSize > 0)
			nLowBufSize = nMinBufSize;
		while (nBufSize > nLowBufSize)
		{
	        if (::setsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF, (char *)&nBufSize, sizeof(int)) == 0)
				break;
			nBufSize /= 2;
		}
		nBufSize = MAX_SOCKET_BUFFER_SIZE;
		nLowBufSize = MIN_SOCKET_BUFFER_SIZE;
		if (nMaxBufSize > 0)
			nBufSize = nMaxBufSize;
		if (nMinBufSize > 0)
			nLowBufSize = nMinBufSize;		
		while (nBufSize > nLowBufSize)
		{
			if (::setsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, (char *)&nBufSize, sizeof(int)) == 0)
				break;
			nBufSize /= 2;
		}
	}
}

 //��ȡ����״̬
BOOL CIoCompletionSocket::ReadConnectStatus(CPacketStream *pPacket)
{
	if (!IsValid())
	{
		PRINTDEBUGLOG(dtInfo, "invalid socket in ReadConnectStatus");
		return FALSE;
	}
	if (!pPacket)
	{
		pPacket = m_pServer->AllocatePacket();
	}
	else
	{
		pPacket->AddRef();
	}
	m_dwActiveTime = ::GetTickCount();
	m_pServer->PostIoOperation(CPacketStream::IO_TYPE_CONNECT_REQUEST, this, pPacket);
	pPacket->Release();

	return TRUE;
}

//��ȡ����
BOOL CIoCompletionSocket::Read(CPacketStream *pPacket/* = NULL */)
{
	if (!IsValid())
	{
		PRINTDEBUGLOG(dtInfo, "invalid socket");
		return FALSE;
	}
	if (!pPacket)
	{
		pPacket = m_pServer->AllocatePacket();
	}
	else
	{
		pPacket->AddRef();
	}
	m_dwActiveTime = ::GetTickCount();
	m_pServer->PostIoOperation(CPacketStream::IO_TYPE_READ_REQUEST, this, pPacket);

	pPacket->Release();

	return TRUE;
}

//д��δ���͵�����
void CIoCompletionSocket::WriteCompleted()
{
	if (m_sqList.empty())
	{
		if (::InterlockedDecrement(&m_lWritting) == 0)
		{
			//�ж��Ƿ񱻹ر�
			//PRINTDEBUGLOG(dtInfo, "Socket Is Closed in Write Completed");
		}
	} else
	{
		//���ͻ����е�
		m_Lock.Lock();
		CPacketStream *pStream = NULL;
		if (!m_sqList.empty())
		{
			pStream = m_sqList.front();
			m_sqList.pop_front();
		}
		m_Lock.UnLock();
		if (pStream)
		{
			m_pServer->PostIoOperation(CPacketStream::IO_TYPE_WRITE_REQUEST, this, pStream);
		}
		pStream->Release();
	}
}

LONG CIoCompletionSocket::AddRef()
{
	return ::InterlockedIncrement(&m_lRef);
}

LONG CIoCompletionSocket::Release()
{
	return ::InterlockedDecrement(&m_lRef);
}

LONG  CIoCompletionSocket::GetRef()
{
	return m_lRef;
}

//���ӷ�����
BOOL CIoCompletionSocket::ConnectSvr(const char *szIp, const WORD wPort)
{
	if (m_hSocket == INVALID_SOCKET)
	{
		m_hSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
		if (m_hSocket == INVALID_SOCKET)
		{
			PRINTDEBUGLOG(dtInfo, "create socket failed in connect:%d", WSAGetLastError());
			return FALSE;
		}
		//������IOCP
		if (m_pServer && m_pServer->AssociateConnectStatus(m_hSocket, this))
		{
			SOCKADDR_IN sockAddr;
			memset(&sockAddr, 0, sizeof(sockAddr));
			sockAddr.sin_family = AF_INET;
			sockAddr.sin_addr.s_addr = inet_addr(szIp);
			sockAddr.sin_port = htons((u_short)wPort);
			int nError = ::connect(m_hSocket, (SOCKADDR*)&sockAddr, sizeof(sockAddr));
			if ((nError != SOCKET_ERROR) || (::WSAGetLastError() != ERROR_IO_PENDING))
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

#pragma warning(default:4996)
