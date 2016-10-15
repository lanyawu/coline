#ifndef __BUFFER_LIB_H___
#define __BUFFER_LIB_H___

#include <string>
#include <list>
#include <commonlib/types.h>

class COMMONLIB_API CBuffer
{
public:
   CBuffer();
   CBuffer(DWORD dwSize);
   CBuffer(const CBuffer &);
   CBuffer(const void *pData, int nSize);
   virtual ~CBuffer();

   CBuffer& operator=(CBuffer &);
   CBuffer& operator+=(CBuffer &);
   friend CBuffer operator+(CBuffer &, CBuffer &);

   
   CBuffer& operator >> (char &in);
   CBuffer& operator >> (BYTE &in);
   CBuffer& operator >> (SHORT &in);
   CBuffer& operator >> (DWORD &in);
   //
   CBuffer& operator << (char &out);
   CBuffer& operator << (BYTE &out);
   CBuffer& operator << (SHORT &out);
   CBuffer& operator << (DWORD &out); 

   //追加数据
   CBuffer & Append(const void *pData, DWORD dwSize);
   //读取裸数据
   BOOL  UnpackRaw(char *szRaw, int nSize);

   //调用函数
   char *GetData() const;
   DWORD GetDataSize(); //获取实际的数据大小
   virtual void Clear();
   //debug
   void Print();
   //读取位置复位
   void ResetReadPos();
   //获取读取位置
   DWORD GetReadPos();
   BOOL End();
protected:

private:
	//重新分配大小，并拷贝源串
	BOOL ReAllocBuffer(DWORD &dwNewSize);
protected:
	char *m_pData;
	DWORD m_dwReadPos; //读取位置
	DWORD m_dwWritePos; //写入位置
	DWORD m_dwAllocSize;//总大小
};

#endif