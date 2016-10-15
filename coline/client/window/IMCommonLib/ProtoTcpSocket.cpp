#include <commonlib/debuglog.h>
#include <commonlib/classes.h>
#include <Crypto/Crypto.h>
#include "ProtoTcpSocket.h"

#define PROTOCOL_MAX_LENGTH  524288

//xml聊天用的压缩格式
#define XML_ENCODE_TYPE_NONE   0   //不采用压缩格式
#define XML_ENCODE_TYPE_ZLIB   1   //采用Zlib压缩格式
static const int XML_PROTOCOL_HEADER_SIZE = 9; //

CProtoTcpSocket::CProtoTcpSocket(void):
                 m_nLeaveSize(0),
			     m_pBuffer(NULL),
			     m_nBufSize(0),
			     m_nTotalSize(0)
{
}

CProtoTcpSocket::~CProtoTcpSocket(void)
{
	 if (m_pBuffer)
		 delete []m_pBuffer;
	 m_pBuffer = NULL;
}

BOOL CProtoTcpSocket::SendRawData(const char *pBuf, const int nBufSize)
{
	if (m_bConnected)
	{
		CMemoryStream Stream;
		Stream.SetSize(nBufSize + XML_PROTOCOL_HEADER_SIZE);
		memcpy(Stream.GetMemory() + XML_PROTOCOL_HEADER_SIZE, pBuf, nBufSize);
		//填充协议头
		int nAckSize = nBufSize + 5;
		memcpy(Stream.GetMemory(), &nAckSize, sizeof(int));
		*(Stream.GetMemory() + 4) = XML_ENCODE_TYPE_NONE;
		memmove(Stream.GetMemory() + 5, &nBufSize, sizeof(int));
		return (Send(Stream.GetMemory(), (int) Stream.GetSize()) == (int) Stream.GetSize());
	}
	return FALSE;
}

void CProtoTcpSocket::OnReceive(int nErrorCode)
{
	char Buf[8192] = {0};
	int nSize = Receive(Buf, 8192);
	m_Stream.Write(Buf, nSize);
	while (nSize == 8192)
	{
		nSize = Receive(Buf, 8192);
		m_Stream.Write(Buf, nSize);
	}
	ParserStream();
}

BOOL CProtoTcpSocket::IsConnected()
{
	if (m_hSocket != INVALID_SOCKET)
		return m_bConnected;
	return FALSE;
}

BOOL CProtoTcpSocket::OnRecvProtocol(const char *szBuf, const int nBufSize)
{
	//debug
	char *pTmp = new char[nBufSize + 1];
	memcpy(pTmp, szBuf, nBufSize);
	pTmp[nBufSize] = '\0';
	PRINTDEBUGLOG(dtInfo, "Recv Protocol:%s", pTmp);
	delete []pTmp;
	return TRUE;
}

BOOL CProtoTcpSocket::ParserStream()
{
	if (m_nLeaveSize > 0) 
	{
		int nSize = m_nLeaveSize;
		if (nSize > (int) m_Stream.GetSize())
			nSize = (int) m_Stream.GetSize();
		memcpy(m_pBuffer + (m_nTotalSize - m_nLeaveSize), m_Stream.GetMemory(), nSize);
		m_nLeaveSize -= nSize;
		m_Stream.RemoveFrontData(nSize);
		if (m_nLeaveSize == 0)  
		{
			//
			DoFullProtocol();
			if (m_Stream.GetSize() > sizeof(int))
				ParserStream();
		}		
	} else if (m_Stream.GetSize() > sizeof(int)) 
	{
		int nSize = 0;
		memcpy(&nSize, m_Stream.GetMemory(), sizeof(int));
		nSize += sizeof(int);
		if ((nSize > PROTOCOL_MAX_LENGTH) || (nSize < 1))
		{
			PRINTDEBUGLOG(dtInfo, "error protocol length:%d", nSize);
			return FALSE;
		}
		if (nSize > m_nBufSize)  
		{
			if (m_pBuffer)
				delete []m_pBuffer;
			m_pBuffer = new char[nSize];
			m_nBufSize = nSize;
		}
		m_nTotalSize = nSize;
		if (nSize > (int) m_Stream.GetSize())
		{
			nSize = (int) m_Stream.GetSize();
		}
		memcpy(m_pBuffer, m_Stream.GetMemory(), nSize);
		m_nLeaveSize = m_nTotalSize - nSize;
		m_Stream.RemoveFrontData(nSize);
		if (m_nLeaveSize == 0)  
		{
			//
			DoFullProtocol(); 
			if (m_Stream.GetSize() > sizeof(int))
				ParserStream();
		}
	}
	return TRUE;
}

BOOL CProtoTcpSocket::DoFullProtocol()
{
	int nDestSize = 0;
	int nOffset = sizeof(char) + sizeof(int) + sizeof(int); //
	memcpy(&nDestSize, m_pBuffer + sizeof(int) + sizeof(char), sizeof(int));
	if (nDestSize <= 0)
	{
		PRINTDEBUGLOG(dtInfo, "DestSize Error, size:%d", nDestSize);
		return FALSE;
	}
	BOOL bSucc = FALSE;
	switch(*(m_pBuffer + sizeof(int)))
	{
	case 0:  
		{
			bSucc = OnRecvProtocol(m_pBuffer + nOffset, nDestSize);
			if (!bSucc)
			{
				char *pLog = new char[nDestSize + 1];
				memcpy(pLog, m_pBuffer + nOffset, nDestSize);
				pLog[nDestSize] = '\0';
				PRINTDEBUGLOG(dtInfo, "error protocol:%s", pLog);
				delete []pLog;
			}
			break;
		}
	case 1:  
		{
			if (nDestSize < PROTOCOL_MAX_LENGTH)
			{
				char *pBuf = new char[nDestSize + 1];				
				memset(pBuf, 0, nDestSize + 1);
				if (zlib_uncompress((BYTE *)pBuf, (UINT32 *)&nDestSize, (BYTE *)(m_pBuffer + nOffset), 
					m_nTotalSize - nOffset))
				{
					bSucc = OnRecvProtocol(pBuf, nDestSize);
					if (!bSucc)
					{
						char *pLog = new char[nDestSize + 1];
						memcpy(pLog, pBuf, nDestSize);
						pLog[nDestSize] = '\0';
						PRINTDEBUGLOG(dtInfo, "error xml protocol:%s", pLog);
						delete []pLog;
					}
				} else
				{
					PRINTDEBUGLOG(dtInfo, "decompress failed, src size:%d dest size:%d", m_nTotalSize - nOffset, nDestSize);
				}
				delete []pBuf;
			} else
			{
				PRINTDEBUGLOG(dtInfo, "decompress size too large:%d", nDestSize);
			}
			break;
		}
	} 
	return  TRUE;
}


///CAIInterfaceSocket
CProtoInterfaceSocket::CProtoInterfaceSocket(IProtoSocketNotify *pNotify):
                    m_pNotify(pNotify) 
{

}

void CProtoInterfaceSocket::OnClose(int nErrorCode)
{
	CProtoTcpSocket::OnClose(nErrorCode);
	m_bConnected = FALSE;
	if (m_pNotify)
		m_pNotify->OnSocketClose(this, nErrorCode);
}

void CProtoInterfaceSocket::OnConnect(int nErrorCode)
{
	CProtoTcpSocket::OnConnect(nErrorCode);
	SetKeepLive();
	if (m_pNotify)
		m_pNotify->OnSocketConnect(this, nErrorCode);
}

BOOL CProtoInterfaceSocket::OnRecvProtocol(const char *szBuf, const int nBufSize)
{
	if (m_pNotify)
		return (SUCCEEDED(m_pNotify->OnRecvProtocol(szBuf, nBufSize)));
	return FALSE;
}