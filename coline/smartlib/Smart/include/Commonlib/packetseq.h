#ifndef __PACKETSEQ_H___
#define __PACKETSEQ_H___

#include <commonlib/types.h>

//������࣬ʵ�ְ���ŵ�������Ƚ�
class COMMONLIB_API CPacketSeq
{
public:
	CPacketSeq(DWORD nSeq = 0);
	~CPacketSeq();

public:
	//���������� �Ƚϲ�����
	BOOL operator < (CPacketSeq &Other) const;
	BOOL operator < (DWORD nSeq) const;
	BOOL operator > (CPacketSeq &Other) const;
	BOOL operator > (DWORD nSeq) const;
	BOOL operator == (CPacketSeq &Other) const;
	BOOL operator == (DWORD nSeq) const;
	BOOL operator <= (CPacketSeq &Other) const;
	BOOL operator <= (DWORD nSeq) const;
	BOOL operator >= (CPacketSeq &Other) const;
	BOOL operator >= (DWORD nSeq) const;

	//���������� �����
	CPacketSeq & operator += (CPacketSeq &Other);
	CPacketSeq & operator += (DWORD nSeq);
	CPacketSeq & operator -= (CPacketSeq &Other);
	CPacketSeq & operator -= (DWORD nSeq);
	CPacketSeq & operator + (CPacketSeq &Other);
	CPacketSeq & operator + (DWORD nSeq);
    CPacketSeq & operator - (CPacketSeq &Other);
	CPacketSeq & operator - (DWORD nSeq);
    CPacketSeq & operator ++ ();
	CPacketSeq & operator --();
	CPacketSeq & operator ++(int);
	CPacketSeq & operator --(int);

	//���������� ��ֵ�����
	CPacketSeq & operator = (CPacketSeq &Other);
	CPacketSeq & operator = (DWORD nSeq);
	CPacketSeq & operator = (int nSeq);
	//ת��
	operator DWORD() const;
	operator int() const;
private:
	DWORD m_nSeq;
};

#endif