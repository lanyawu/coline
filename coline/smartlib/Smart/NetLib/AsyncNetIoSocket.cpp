#include <commonlib/debuglog.h>
#include <netlib/AsyncNetIoSocket.h>

CAsyncNetIoSocket::CAsyncNetIoSocket(void):
                   m_lWritting(0),
				   m_bConnected(FALSE)
{
	Create();
}

CAsyncNetIoSocket::~CAsyncNetIoSocket(void)
{
	m_bConnected = FALSE;
	CGuardLock::COwnerLock guard(m_ListLock);
	while (!m_WrittingList.empty())
	{
		CMemoryStream *pStream = m_WrittingList.front();
		delete pStream;
		m_WrittingList.pop_front();
	}
	Close();
}

//导入未发送完毕的消息
void CAsyncNetIoSocket::ImportWrittingStream(CAsyncNetIoSocket *pSocket)
{
	if (!pSocket)
		return ;
	CGuardLock::COwnerLock guard1(m_ListLock);
	CGuardLock::COwnerLock guard2(pSocket->m_ListLock);
	while (!pSocket->m_WrittingList.empty())
	{
		CMemoryStream *pStream = pSocket->m_WrittingList.front();
		m_WrittingList.push_back(pStream);
		pSocket->m_WrittingList.pop_front();
	}
}

//处理协议
BOOL CAsyncNetIoSocket::DoRcvStream(CMemoryStream &Stream)
{
	Stream.RemoveFrontData((int)Stream.GetSize());
	return TRUE;
}

//CASocket implementation
void CAsyncNetIoSocket::OnReceive(int nErrorCode)
{
	const int MAX_RECV_BUFFER_SIZE = 4096;
	if (nErrorCode == 0)
	{
		int nBufSize = 0;
		char Buf[MAX_RECV_BUFFER_SIZE] = {0};
		while (TRUE)
		{
			nBufSize = Receive(Buf, MAX_RECV_BUFFER_SIZE);
			if (nBufSize > 0)
			{
				m_RcvStream.Write(Buf, nBufSize);
			}
			if (nBufSize < MAX_RECV_BUFFER_SIZE)
				break;
		}
		//处理协议
		DoRcvStream(m_RcvStream);
	} else
	{
		PRINTDEBUGLOG(dtInfo, "Receive Error:%d", nErrorCode);
	}
}

void CAsyncNetIoSocket::OnSend(int nErrorCode)
{
	if (m_bConnected)
	{
		SendLeaveStream();
	}
}

//发送剩余数据
void CAsyncNetIoSocket::SendLeaveStream()
{
	CMemoryStream * pStream = NULL;
	while (TRUE)
	{
		m_ListLock.Lock();
		if (!m_WrittingList.empty())
		{
			pStream = m_WrittingList.front();
			m_WrittingList.pop_front();
		}
		m_ListLock.UnLock();
		if (pStream)
		{
			int nSize = Send(pStream->GetMemory(), (int) pStream->GetSize());
			if (nSize != (int) pStream->GetSize())
			{
				if (nSize > 0) //只发送了一部分
				{
					AppendStream(pStream->GetMemory() + nSize, (int) pStream->GetSize() - nSize);
				} else
				{
					PRINTDEBUGLOG(dtInfo, "send buffer failed:%d", ::WSAGetLastError());
				}
				delete pStream;
				pStream = NULL;
				break;
			} //end if(nSize !=...
			delete pStream;
			pStream = NULL;
		} else
		{
			if (::InterlockedDecrement(&m_lWritting) != 0)
			{
				PRINTDEBUGLOG(dtInfo, "Writting Ref Failed:%d", m_lWritting);
			}
			break;
		}
	}  
}

void CAsyncNetIoSocket::OnConnect(int nErrorCode)
{
	if (nErrorCode == 0) //连接成功
	{
		::InterlockedIncrement(&m_lWritting);
		m_bConnected = TRUE;
	} else //连接失败
	{
		PRINTDEBUGLOG(dtInfo, "connect failed:%d", nErrorCode);
	}
}

void CAsyncNetIoSocket::OnClose(int nErrorCode)
{
	PRINTDEBUGLOG(dtInfo, "Close Event:%d", nErrorCode);
}

BOOL CAsyncNetIoSocket::AppendStream(const char *pBuf, const int nBufSize, BOOL bFront)
{
	CMemoryStream *pStream = new CMemoryStream();
	CGuardLock::COwnerLock guard(m_ListLock);
	pStream->SetSize(nBufSize);
	memcpy(pStream->GetMemory(), pBuf, nBufSize);
	if (bFront)
		m_WrittingList.push_front(pStream);
	else
		m_WrittingList.push_back(pStream);
	return TRUE;
}
 
BOOL CAsyncNetIoSocket::Write(const char *pBuf, const int nBufSize)
{
	if (m_bConnected)
	{
		if (::InterlockedExchangeAdd(&m_lWritting, 0) > 0)  //正在发送
		{
			return AppendStream(pBuf, nBufSize);
		} else //直接发送
		{
			if (m_lWritting < 0)
			{
				PRINTDEBUGLOG(dtInfo, "Writting Ref Failed:%d", m_lWritting);
				m_lWritting = 0;
			}
			::InterlockedIncrement(&m_lWritting);
			int nSize = Send(pBuf, nBufSize);
			if (nSize == nBufSize)
			{
				SendLeaveStream();
				return TRUE;
			} else if (nSize > 0)
			{
				AppendStream(pBuf + nSize, nBufSize - nSize, TRUE);
			} else
			{
				PRINTDEBUGLOG(dtInfo, "Send Buffer Failed:%d", ::WSAGetLastError());
				return FALSE;
			}
			return TRUE;
		}
	} else
	{
		return AppendStream(pBuf, nBufSize);
	}
}

LONG CAsyncNetIoSocket::AddRef()
{
	return ::InterlockedIncrement(&m_lRef);
}

LONG CAsyncNetIoSocket::Release()
{
	return ::InterlockedDecrement(&m_lRef);
}