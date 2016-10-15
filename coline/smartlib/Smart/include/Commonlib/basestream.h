#ifndef __BASESTREAM_H___
#define __BASESTREAM_H___

#include <commonlib/types.h>

class COMMONLIB_API CBaseStream
{
public:
	//λ�ö���
	enum CStreamType
	{
		STREAM_BEGIN,
		STREAM_CURR,
		STREAM_END
	};
public:
	CBaseStream(void);
	virtual ~CBaseStream(void);
public:
    int Seek(int nPos, CStreamType nType); //λ��
	int Read(char *pBuff, int nSize);
	int Write(const char *pBuff, int nSize);
	int GetPosition(); //��ȡλ��
protected:
	int m_nPosition; //��ǰλ��
	int m_nTotalSize; //��С
};

#endif