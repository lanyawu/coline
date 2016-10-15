#ifndef __BASESTREAM_H___
#define __BASESTREAM_H___

#include <commonlib/types.h>

class COMMONLIB_API CBaseStream
{
public:
	//位置定义
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
    int Seek(int nPos, CStreamType nType); //位置
	int Read(char *pBuff, int nSize);
	int Write(const char *pBuff, int nSize);
	int GetPosition(); //获取位置
protected:
	int m_nPosition; //当前位置
	int m_nTotalSize; //大小
};

#endif