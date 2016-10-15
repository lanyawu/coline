#include <commonlib/debuglog.h>
#include <commonlib/BaseStream.h>

CBaseStream::CBaseStream(void):
             m_nPosition(0),
			 m_nTotalSize(0)
{
}

CBaseStream::~CBaseStream(void)
{
}

//Œª÷√
int CBaseStream::Seek(int nPos, CStreamType nType) 
{
	switch(nType)
	{
	case CBaseStream::STREAM_BEGIN:
		 m_nPosition = nPos;
		 break;
	case CBaseStream::STREAM_CURR:
		 m_nPosition += nPos;
		 break;
	case CBaseStream::STREAM_END:
		 m_nPosition = m_nTotalSize - nPos;
		 break;
	}
	if (m_nPosition < 0)
		m_nPosition = 0;
	if (m_nPosition > m_nTotalSize)
		m_nPosition = m_nTotalSize;
	return m_nPosition;
}

int CBaseStream::GetPosition()
{
	return m_nPosition;
}

int CBaseStream::Read(char *pBuff, int nSize)
{
	return 0;
}

int CBaseStream::Write(const char *pBuff, int nSize)
{
	return 0;
}

 