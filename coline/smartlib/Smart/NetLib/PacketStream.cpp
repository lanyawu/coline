#include <netlib/PacketStream.h>
#include <commonlib/debuglog.h>

CPacketStream::CPacketStream(CPacketAllocator *pAllocator, DWORD dwPacketSize):
               CBaseStream(),
			   m_pAllocator(pAllocator),
			   m_lRef(0)			   
{
	m_pBuff = new char[dwPacketSize];
	m_nTotalSize = dwPacketSize;
}

CPacketStream::~CPacketStream(void)
{
	delete []m_pBuff;
}

LONG CPacketStream::AddRef()
{
	return ::InterlockedIncrement(&m_lRef);
}

LONG CPacketStream::Release()
{
	if (0 == m_lRef)
	{
		PRINTDEBUGLOG(dtInfo, "error packet release");
		return 0;
	}
	LONG lRes = ::InterlockedDecrement(&m_lRef);
	if (0 == lRes)
	{
		//回收
		m_pAllocator->RecyclePacket(this);
	}
	return lRes;
}

LONG CPacketStream::GetRef()
{
	return m_lRef;
}

int CPacketStream::Read(char *pBuff, int nSize)
{
	int nCanRead = m_nTotalSize - m_nPosition;
	if (nSize > nCanRead)
		nSize = nCanRead;
	memmove(pBuff, m_pBuff + m_nPosition, nSize);
	m_nPosition += nSize;
	return nSize;
}

int CPacketStream::Write(const char *pBuff, int nSize)
{
	int nCanWrite = m_nTotalSize - m_nPosition;
	if (nSize > nCanWrite)
		nSize = nCanWrite;
	memmove(m_pBuff + m_nPosition, pBuff, nSize);
	m_nPosition += nSize;
	return nSize;
}

//移除多少数据
void CPacketStream::RemoveData(int nSize)
{
	if (nSize >= m_nPosition)
	{
		m_nPosition = 0;
	} else
	{
		memmove(m_pBuff, m_pBuff + nSize, (m_nPosition - nSize));
		m_nPosition -= nSize;
	}
}

//打印内存数据
void CPacketStream::PrintStream()
{
	std::string str;
	for(int i = 0; i < m_nPosition; i ++)
	{
		if (m_pBuff[i] == 0)
			str += 'A';
		else
			str += m_pBuff[i];
	}
	PRINTDEBUGLOG(dtInfo, "%s", str.c_str());
}

char *CPacketStream::GetData() const
{
	return m_pBuff;
}

//获取剩余的可用大小
int CPacketStream::GetLeaveSize()
{
	return m_nTotalSize - m_nPosition;
}

//初始化
void CPacketStream::Initialize()
{
	m_nPosition = 0;
}

//设置接受连接操作 填充WSABUF结构
void CPacketStream::SetupAccept()
{
	memset(&m_Overlapped, 0, sizeof(OVERLAPPED));
	m_WsaBuf.buf = reinterpret_cast<char *>(m_pBuff);
	m_WsaBuf.len = m_nTotalSize;
}

 //设置读操作
void CPacketStream::SetupRead()
{
	memset(&m_Overlapped, 0, sizeof(OVERLAPPED)); //初始化
	m_WsaBuf.buf = reinterpret_cast<char *>(m_pBuff + m_nPosition);
	m_WsaBuf.len = m_nTotalSize - m_nPosition;
}

 //设置写操作
void CPacketStream::SetupWrite()
{
	memset(&m_Overlapped, 0, sizeof(OVERLAPPED)); //初始化
	m_WsaBuf.buf = reinterpret_cast<char *>(m_pBuff);
	m_WsaBuf.len = m_nPosition;
}

WSABUF *CPacketStream::GetWSABUF() const
{
	return const_cast<WSABUF *>(&m_WsaBuf);
}

void CPacketStream::SetIoOperation(CIoOperationType nType)
{
	m_IoType = nType;
}

CPacketStream::CIoOperationType CPacketStream::GetIoOperator()
{
	return m_IoType;
}

void CPacketStream::SetSocket(SOCKET h)
{
	m_hSocket = h;
}

void CPacketStream::SetListenSocket(SOCKET h)
{
	m_hListen = h;
}

SOCKET CPacketStream::GetSocket()
{
	return m_hSocket;
}

SOCKET CPacketStream::GetListenSocket()
{
	return m_hListen;
}

//设置属于哪个宿主
void CPacketStream::SetOwnerList(CPacketList *pList)
{
	m_pOwnerList = pList;
}

void CPacketStream::RemoveFromList()
{
	if (m_pOwnerList)
	{
		std::vector<CPacketStream *>::iterator it;
		for (it = m_pOwnerList->begin(); it != m_pOwnerList->end(); it ++)
		{
			if ((*it) == this)
			{
				m_pOwnerList->erase(it);
				break;
			} //end fi ((*it) == this)
		} //end for
	}
}

// 数据包内存分配器
// class CPacketAllocator
CPacketAllocator::CPacketAllocator(DWORD dwMaxBufCount, DWORD dwPacketSize):
                  m_dwMaxBufCount(dwMaxBufCount),
#ifdef DEBUG_PRINT_ALLOC_COUNT
				  m_nAllocCount(0),
				  m_nNewCount(0),
				  m_nReleaseCount(0),
#endif
				  m_dwPacketSize(dwPacketSize)
{
}

CPacketAllocator::~CPacketAllocator()
{
	Flush();
}

CPacketStream *CPacketAllocator::AllocPacket()
{
	CPacketStream *pStream = NULL;
	m_Lock.Lock();
	if (!m_FreeList.empty())
	{		
		pStream = m_FreeList.back();
		pStream->Initialize();
		m_FreeList.pop_back();		
	}
	m_Lock.UnLock();
	if (!pStream)
	{
		pStream = new CPacketStream(this, m_dwPacketSize);
#ifdef DEBUG_PRINT_ALLOC_COUNT
		::InterlockedIncrement(&m_nNewCount);
#endif
	}
	if (!pStream)
	{
		PRINTDEBUGLOG(dtError, "alloc packet stream failed");
		return NULL;
	}
#ifdef DEBUG_PRINT_ALLOC_COUNT 
	::InterlockedIncrement(&m_nAllocCount);
#endif
	pStream->AddRef();

	return pStream;
}

 //debug
void CPacketAllocator::GetAllocStatus(int &nAllocCount, int &nReleaseCount, int &nListCount, int &nNewCount)
{
#ifdef DEBUG_PRINT_ALLOC_COUNT
	m_Lock.Lock();
	nAllocCount = m_nAllocCount;
	nReleaseCount = m_nReleaseCount;
	nNewCount = m_nNewCount;
	nListCount = (int) m_FreeList.size();
	m_Lock.UnLock();
#endif
}

//回收
void CPacketAllocator::RecyclePacket(CPacketStream *pPacket)
{
	m_Lock.Lock();
	m_FreeList.push_back(pPacket);
#ifdef DEBUG_PRINT_ALLOC_COUNT
	::InterlockedIncrement(&m_nReleaseCount);
#endif
	m_Lock.UnLock();
}

//更新内存
void CPacketAllocator::Flush()
{
	m_Lock.Lock();
	CPacketStream *pStream = NULL;
	while (!m_FreeList.empty())
	{
		pStream = m_FreeList.back();
		delete pStream;
		m_FreeList.pop_back();
	}
	m_Lock.UnLock();
}

DWORD CPacketAllocator::GetPacketSize()
{
	return m_dwPacketSize;
}
