#ifndef __PACKETSEQ_H___
#define __PACKETSEQ_H___

#include <commonlib/types.h>

//包序号类，实现包序号的增加与比较
class COMMONLIB_API CPacketSeq
{
public:
	CPacketSeq(DWORD nSeq = 0);
	~CPacketSeq();

public:
	//操作符重载 比较操作符
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

	//操作符重载 运算符
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

	//操作符重载 赋值运算符
	CPacketSeq & operator = (CPacketSeq &Other);
	CPacketSeq & operator = (DWORD nSeq);
	CPacketSeq & operator = (int nSeq);
	//转换
	operator DWORD() const;
	operator int() const;
private:
	DWORD m_nSeq;
};

#endif