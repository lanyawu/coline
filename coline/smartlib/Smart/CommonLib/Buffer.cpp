#include <commonlib/Buffer.h>
#include <stdio.h>

CBuffer::CBuffer():
         m_pData(NULL),
		 m_dwReadPos(0),
		 m_dwWritePos(0),
		 m_dwAllocSize(0)
{
}

CBuffer::CBuffer(DWORD dwSize):
         m_pData(NULL),
		 m_dwReadPos(0),
		 m_dwWritePos(0),
		 m_dwAllocSize(0)
{
	m_dwAllocSize = dwSize;
	if (m_dwAllocSize > 0)
	{
		m_pData = new char[m_dwAllocSize];
		memset(m_pData, 0, m_dwAllocSize);
	}
}

CBuffer::CBuffer(const void *pData, int nSize):
         m_pData(NULL),
		 m_dwReadPos(0),
		 m_dwWritePos(0),
		 m_dwAllocSize(0)
{
	if (nSize > 0)
	{
		m_dwAllocSize = nSize;
		m_pData = new char[m_dwAllocSize];
		memmove(m_pData, pData, m_dwAllocSize);
		m_dwWritePos = m_dwAllocSize;
	}
}

CBuffer::CBuffer(const CBuffer &src):
         m_pData(NULL),
		 m_dwReadPos(0),
		 m_dwWritePos(0),
		 m_dwAllocSize(0)
{
	//初始化
	if (src.m_dwAllocSize > 0)
	{
		m_dwAllocSize = src.m_dwAllocSize;
		m_pData = new char[m_dwAllocSize];
		memset(m_pData, 0, m_dwAllocSize);
		m_dwReadPos = src.m_dwReadPos;
		m_dwWritePos = src.m_dwWritePos;
		if (m_dwWritePos > 0)
			memmove(m_pData, src.m_pData, m_dwWritePos);
	}
}

CBuffer::~CBuffer()
{
	Clear();
}

void CBuffer::Clear()
{
	m_dwAllocSize = 0;
	m_dwReadPos = 0;
	m_dwWritePos = 0;
	if (m_pData)
	{
		delete []m_pData;
		m_pData = NULL;
	}
}

//读取位置复位
void CBuffer::ResetReadPos()
{
	m_dwReadPos = 0;
}

//获取读取位置
DWORD CBuffer::GetReadPos()
{
	return m_dwReadPos;
}

CBuffer& CBuffer::operator=(CBuffer &src)
{
	//初始化
	Clear();
	if (src.m_dwAllocSize > 0)
	{
		m_dwAllocSize = src.m_dwAllocSize;
		m_pData = new char[m_dwAllocSize];
		memset(m_pData, 0, m_dwAllocSize);
		m_dwReadPos = src.m_dwReadPos;
		m_dwWritePos = src.m_dwWritePos;
		if (m_dwWritePos > 0)
			memmove(m_pData, src.m_pData, m_dwWritePos);
	}
	return (*this);
}

CBuffer& CBuffer::operator+=(CBuffer &src)
{
	DWORD dwSize = m_dwAllocSize + src.m_dwAllocSize;
	char *pData = new char[dwSize];
	memset(pData, 0, dwSize);
	//拷贝原有
	if (m_dwWritePos > 0)
		memmove(pData, m_pData, m_dwWritePos);
	//拷贝src
	if (src.m_dwWritePos > 0)
		memmove(pData + m_dwWritePos, src.m_pData, src.m_dwWritePos);
	m_dwWritePos += src.m_dwWritePos;
	if (m_pData)
		delete []m_pData;
	m_pData = pData;
	return (*this);
}

CBuffer operator+(CBuffer &src1, CBuffer &src2)
{
	DWORD dwSize = src1.m_dwAllocSize + src2.m_dwAllocSize;
	CBuffer bCat(dwSize);
	//拷贝原有
	if (src1.m_dwWritePos > 0)
		memmove(bCat.m_pData, src1.m_pData, src1.m_dwWritePos);
	//拷贝src
	if (src2.m_dwWritePos > 0)
		memmove(bCat.m_pData + src1.m_dwWritePos, src2.m_pData, src2.m_dwWritePos);
	bCat.m_dwWritePos = src1.m_dwWritePos + src2.m_dwWritePos;
	return bCat;
}
  
//重新分配大小，并拷贝源串
BOOL CBuffer::ReAllocBuffer(DWORD &dwNewSize)
{
	if (dwNewSize > m_dwAllocSize) //
	{
		char *pData = new char[dwNewSize];
		memset(pData , 0, dwNewSize);
		if (m_dwWritePos > 0)
			memmove(pData, m_pData, m_dwWritePos);
		if (m_pData)
			delete []m_pData;
		m_pData = pData;
		m_dwAllocSize = dwNewSize;
		return TRUE;
	} else
		return TRUE;
}

CBuffer& CBuffer::operator >> (char &in)
{
	if ((m_dwReadPos + sizeof(char)) <= m_dwWritePos)
	{
		memmove(&in, m_pData + m_dwReadPos, sizeof(char));
		m_dwReadPos += sizeof(char);
	} else
		in = '\0';
	return (*this);
}

CBuffer& CBuffer::operator >> (BYTE &in)
{
	if ((m_dwReadPos + sizeof(BYTE)) <= m_dwWritePos)
	{
		memmove(&in, m_pData + m_dwReadPos, sizeof(BYTE));
		m_dwReadPos += sizeof(BYTE);
	} else
		in = 0;
	return (*this);
}

CBuffer& CBuffer::operator >> (SHORT &in)
{
	if ((m_dwReadPos + sizeof(SHORT)) <= m_dwWritePos)
	{
		memmove(&in, m_pData + m_dwReadPos, sizeof(SHORT));
		m_dwReadPos += sizeof(SHORT);
	} else
		in = 0;
	return (*this);
}

CBuffer& CBuffer::operator >> (DWORD &in)
{
	if ((m_dwReadPos + sizeof(DWORD)) <= m_dwWritePos)
	{
		memmove(&in, m_pData + m_dwReadPos, sizeof(DWORD));
		m_dwReadPos += sizeof(DWORD);
	} else
		in = 0;
	return (*this);
}

//读取裸数据
BOOL CBuffer::UnpackRaw(char *szRaw, int nSize)
{
	if (m_dwReadPos + nSize <= m_dwWritePos)
	{
		memmove(szRaw, m_pData + m_dwReadPos, nSize);
		m_dwReadPos += nSize;
		return TRUE;
	}
	return FALSE;
}

BOOL CBuffer::End()
{
	if (m_dwReadPos >= m_dwWritePos)
		return TRUE;
	return FALSE;
}


//追加数据
CBuffer & CBuffer::Append(const void *pData, DWORD dwSize)
{
	DWORD dwNewSize = m_dwAllocSize + dwSize;
	if (ReAllocBuffer(dwNewSize))
	{
		memmove(m_pData + m_dwWritePos, pData, dwSize);
		m_dwWritePos += dwSize;
	}
	return (*this);
}

CBuffer & CBuffer::operator << (char &out)
{
	return Append(&out, sizeof(char));
}

CBuffer& CBuffer::operator << (BYTE &out)
{
	return Append(&out, sizeof(BYTE));
}

CBuffer& CBuffer::operator << (SHORT &out)
{
	return Append(&out, sizeof(SHORT));
}

CBuffer& CBuffer::operator << (DWORD &out)
{
	return Append(&out, sizeof(DWORD));
}

char *CBuffer::GetData() const
{
	return m_pData;
}

//获取实际的数据大小
DWORD CBuffer::GetDataSize()
{
	return m_dwWritePos;
}

//debug
void CBuffer::Print()
{
	if (m_pData)
		printf("%s\n", m_pData);
	else
		printf("is NULL\n");
}