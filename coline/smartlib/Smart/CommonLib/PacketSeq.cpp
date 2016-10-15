#include <commonlib/packetseq.h>

#define MAX_SEQ   0x7FFFFFFF
#define HALF_SEQ  0x3FFFFFFF

//∞¸–Ú∫≈CPacketSeq¿‡
CPacketSeq::CPacketSeq(DWORD nSeq):
            m_nSeq(nSeq)
{

}

CPacketSeq::~CPacketSeq()
{
}

BOOL CPacketSeq::operator <(DWORD nSeq) const
{
	return (((m_nSeq < nSeq) && ((nSeq - m_nSeq) < HALF_SEQ))
		    || ((m_nSeq > nSeq) && ((m_nSeq - nSeq) > HALF_SEQ)));
}

BOOL CPacketSeq::operator <(CPacketSeq &Other) const
{
	return operator <(Other.m_nSeq);
}

BOOL CPacketSeq::operator >(DWORD nSeq) const
{
	return (((m_nSeq > nSeq) && ((m_nSeq - nSeq) < HALF_SEQ))
		   || ((m_nSeq < nSeq) && ((nSeq - m_nSeq) > HALF_SEQ)));
}

BOOL CPacketSeq::operator >(CPacketSeq &Other) const
{
	return operator >(Other.m_nSeq);
}

BOOL CPacketSeq::operator ==(DWORD nSeq) const
{
	return (m_nSeq == nSeq);
}

BOOL CPacketSeq::operator ==(CPacketSeq &Other) const
{
	return (operator == (Other.m_nSeq));
}

BOOL CPacketSeq::operator <=(DWORD nSeq) const
{
	return ((operator == (nSeq)) || (operator <(nSeq)));
}

BOOL CPacketSeq::operator <= (CPacketSeq &Other) const
{
	return (operator <=(Other.m_nSeq));
}

BOOL CPacketSeq::operator >=(DWORD nSeq) const
{
	return (operator == (nSeq) || (operator >(nSeq)));
}

BOOL CPacketSeq::operator >=(CPacketSeq &Other) const
{
	return (operator >=(Other.m_nSeq));
}

//‘ÀÀ„∑˚
CPacketSeq &CPacketSeq::operator +(DWORD nSeq)
{
	static CPacketSeq Res;
	Res.m_nSeq = m_nSeq + nSeq;
	if (Res.m_nSeq > MAX_SEQ)
		Res.m_nSeq -= MAX_SEQ;
	return Res;
}

CPacketSeq &CPacketSeq::operator +(CPacketSeq &Other)
{
	return (operator +(Other.m_nSeq));
}

CPacketSeq &CPacketSeq::operator ++()
{
	m_nSeq ++;
	if (m_nSeq > MAX_SEQ)
		m_nSeq -= MAX_SEQ;
	return *this;

}

CPacketSeq &CPacketSeq::operator ++(int)
{
	m_nSeq ++;
	if (m_nSeq > MAX_SEQ)
		m_nSeq -= MAX_SEQ;
	return *this;
}

CPacketSeq &CPacketSeq::operator --()
{
	m_nSeq --;
	if (m_nSeq == 0)
		m_nSeq = MAX_SEQ;
	return *this;
}

CPacketSeq &CPacketSeq::operator --(int)
{
	m_nSeq --;
	if (m_nSeq == 0)
		m_nSeq = MAX_SEQ;
	return *this;
}

CPacketSeq &CPacketSeq::operator -(DWORD nSeq)
{
	static CPacketSeq Res;
	Res = (DWORD)0;
	if (m_nSeq != nSeq)
	{
		DWORD nMax, nMin;
		if (m_nSeq > nSeq)
		{
			nMax = m_nSeq;
			nMin = nSeq;
		} else
		{
			nMax = nSeq;
			nMin = m_nSeq;
		}
	    
		Res.m_nSeq = nMax - nMin;
		if (Res.m_nSeq> HALF_SEQ)
			Res.m_nSeq = MAX_SEQ - Res.m_nSeq;
	}
	return Res;
}

CPacketSeq &CPacketSeq::operator -(CPacketSeq &Other)
{
	return (operator -(Other.m_nSeq));
}

CPacketSeq &CPacketSeq::operator -=(DWORD nSeq)
{
	return (operator -(nSeq));
}

CPacketSeq &CPacketSeq::operator -=(CPacketSeq &Other)
{
	return (operator -=(Other.m_nSeq));
}

CPacketSeq &CPacketSeq::operator +=(DWORD nSeq)
{
	return (operator +(nSeq));
}

CPacketSeq &CPacketSeq::operator +=(CPacketSeq &Other)
{
	return (operator +=(Other.m_nSeq));
}

CPacketSeq &CPacketSeq::operator = (CPacketSeq &Other)
{
	if (this == &Other)
		return *this;
	m_nSeq = Other.m_nSeq;
	return *this;
}

CPacketSeq &CPacketSeq::operator =(DWORD nSeq)
{
	m_nSeq = nSeq;
	return *this;
}


CPacketSeq &CPacketSeq::operator = (int nSeq)
{
	m_nSeq = nSeq;
	return *this;
}

CPacketSeq::operator int() const
{
	return m_nSeq;
}

CPacketSeq::operator DWORD() const
{
	return m_nSeq;
}