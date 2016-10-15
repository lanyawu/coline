#include <commonlib/debuglog.h>
#include <commonlib/Buffer.h>
#include <commonlib/Classes.h>

 
///////////////////////////////////////////////////////////////////////////////
// CStream

//-----------------------------------------------------------------------------
CStream::~CStream()
{
	//
}

int CStream::ReadBuffer(void *pBuffer, int nCount)
{
	return Read(pBuffer, nCount);
}

//-----------------------------------------------------------------------------

int CStream::WriteBuffer(void *pBuffer, int nCount)
{
    if (nCount != 0 && Write(pBuffer, nCount) != nCount)
        return 0;
	return nCount;
}

INT64 CStream::GetPosition() const 
{ 
	return const_cast<CStream&>(*this).Seek(0, SO_CURRENT);
}

void CStream::SetPosition(INT64 nPos) 
{
	Seek(nPos, SO_BEGINNING); 
}
//-----------------------------------------------------------------------------

INT64 CStream::CopyFrom(CStream& Source, INT64 nCount)
{
    const int MAX_BUF_SIZE = 0xF000;

    INT64 nResult;
    int nBufSize, n;

    if (nCount == 0)
    {
        Source.SetPosition(0);
        nCount = Source.GetSize();
    }

    nResult = nCount;
    nBufSize = (int)((nCount > MAX_BUF_SIZE)? MAX_BUF_SIZE : nCount);
    
    CBuffer Buffer(nBufSize);
    while (nCount != 0)
    {
        n = (int)((nCount > nBufSize)? nBufSize : nCount);
		Source.ReadBuffer(Buffer.GetData(), n);
		WriteBuffer(Buffer.GetData(), n);
        nCount -= n;
    }

    return nResult;
}

//-----------------------------------------------------------------------------

INT64 CStream::GetSize() const
{
    INT64 nPos, nResult;

    nPos = const_cast<CStream&>(*this).Seek(0, SO_CURRENT);
    nResult = const_cast<CStream&>(*this).Seek(0, SO_END);
    const_cast<CStream&>(*this).Seek(nPos, SO_BEGINNING);

    return nResult;
}

void CStream::SetSize(INT64 nSize) 
{
	//
}
///////////////////////////////////////////////////////////////////////////////
// CCustomMemoryStream

//-----------------------------------------------------------------------------

CCustomMemoryStream::CCustomMemoryStream() :
					 m_pMemory(NULL),
					 m_nSize(0),
					 m_nPosition(0)
{
}

//-----------------------------------------------------------------------------

void CCustomMemoryStream::SetPointer(char *pMemory, int nSize)
{
    m_pMemory = pMemory;
    m_nSize = nSize;
}

//-----------------------------------------------------------------------------

int CCustomMemoryStream::Read(void *pBuffer, int nCount)
{
    int nResult = 0;

    if (m_nPosition >= 0 && nCount >= 0)
    {
        nResult = m_nSize - m_nPosition;
        if (nResult > 0)
        {
            if (nResult > nCount) nResult = nCount;
            memmove(pBuffer, m_pMemory + (DWORD)m_nPosition, nResult);
            m_nPosition += nResult;
        }
    }
    return nResult;
}

char *CCustomMemoryStream::GetMemory() 
{ 
	return m_pMemory; 
}
//-----------------------------------------------------------------------------

INT64 CCustomMemoryStream::Seek(INT64 nOffset, SEEK_ORIGIN nSeekOrigin)
{
    switch (nSeekOrigin)
    {
		case SO_BEGINNING:
			m_nPosition = (int)nOffset;
			break;
		case SO_CURRENT:
			m_nPosition += (int)nOffset;
			break;
		case SO_END:
			m_nPosition = m_nSize + (int)nOffset;
			break;
    }

    return m_nPosition;
}

//-----------------------------------------------------------------------------

void CCustomMemoryStream::SaveToStream(CStream& Stream)
{
    if (m_nSize != 0)
        Stream.Write(m_pMemory, m_nSize);
}


///////////////////////////////////////////////////////////////////////////////
// CMemoryStream

//-----------------------------------------------------------------------------

CMemoryStream::CMemoryStream(int nMemoryDelta) :
               m_nCapacity(0)
{
    SetMemoryDelta(nMemoryDelta);
}

//-----------------------------------------------------------------------------

CMemoryStream::~CMemoryStream()
{
    Clear();
}

//-----------------------------------------------------------------------------

void CMemoryStream::SetMemoryDelta(int nNewMemoryDelta)
{
    if (nNewMemoryDelta != DEFAULT_MEMORY_DELTA)
    {
        if (nNewMemoryDelta < MIN_MEMORY_DELTA)
            nNewMemoryDelta = MIN_MEMORY_DELTA;

        // Ensure the nNewMemoryDelta is 2^N
        for (int i = sizeof(int) * 8 - 1; i >= 0; i--)
            if (((1 << i) & nNewMemoryDelta) != 0)
            {
                nNewMemoryDelta &= (1 << i);
                break;
            }
    }

    m_nMemoryDelta = nNewMemoryDelta;
}

//-----------------------------------------------------------------------------

void CMemoryStream::SetCapacity(int nNewCapacity)
{
    SetPointer(Realloc(nNewCapacity), m_nSize);
    m_nCapacity = nNewCapacity;
}

//-----------------------------------------------------------------------------

char* CMemoryStream::Realloc(int& nNewCapacity)
{
    char* pResult;

    if (nNewCapacity > 0 && nNewCapacity != m_nSize)
        nNewCapacity = (nNewCapacity + (m_nMemoryDelta - 1)) & ~(m_nMemoryDelta - 1);

    pResult = m_pMemory;
    if (nNewCapacity != m_nCapacity)
    {
        if (nNewCapacity == 0)
        {
            free(m_pMemory);
            pResult = NULL;
        } else
        {
            if (m_nCapacity == 0)
                pResult = (char*)malloc(nNewCapacity);
            else
                pResult = (char*)realloc(m_pMemory, nNewCapacity);
            if (!pResult)
			{
				PRINTDEBUGLOG(dtInfo, "alloc memory failed:%d, NewCapacity:%d", ::GetLastError(), nNewCapacity);
			}
        }
    }

    return pResult;
}

//移除前面的数据
void CMemoryStream::RemoveFrontData(int nSize)
{
	if (nSize >= m_nSize)
		SetSize(0);
	else
	{
		m_nSize -= nSize;
		memcpy(m_pMemory, m_pMemory + nSize, m_nSize);
		if (m_nPosition > m_nSize)
			m_nPosition = m_nSize;
	}
}

//-----------------------------------------------------------------------------

int CMemoryStream::Write(const void *pBuffer, int nCount)
{
    int nResult = 0;
    int nPos;

    if (m_nPosition >= 0 && nCount >= 0)
    {
        nPos = m_nPosition + nCount;
        if (nPos > 0)
        {
            if (nPos > m_nSize)
            {
                if (nPos > m_nCapacity)
                    SetCapacity(nPos);
                m_nSize = nPos;
            }
            memmove(m_pMemory + (DWORD)m_nPosition, pBuffer, nCount);
            m_nPosition = nPos;
            nResult = nCount;
        }
    }

    return nResult;
}

//-----------------------------------------------------------------------------

void CMemoryStream::SetSize(INT64 nSize)
{
    int nOldPos = m_nPosition;

    SetCapacity((int)nSize);
    m_nSize = (int)nSize;
    if (nOldPos > nSize) Seek(0, SO_END);
}

//-----------------------------------------------------------------------------

void CMemoryStream::LoadFromStream(CStream& Stream)
{
    INT64 nCount = Stream.GetSize();

    Stream.SetPosition(0);
    SetSize(nCount);
    if (nCount != 0)
        Stream.Read(m_pMemory, (int)nCount);
}


//-----------------------------------------------------------------------------

void CMemoryStream::Clear()
{
    SetCapacity(0);
    m_nSize = 0;
    m_nPosition = 0;
}
