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

   //׷������
   CBuffer & Append(const void *pData, DWORD dwSize);
   //��ȡ������
   BOOL  UnpackRaw(char *szRaw, int nSize);

   //���ú���
   char *GetData() const;
   DWORD GetDataSize(); //��ȡʵ�ʵ����ݴ�С
   virtual void Clear();
   //debug
   void Print();
   //��ȡλ�ø�λ
   void ResetReadPos();
   //��ȡ��ȡλ��
   DWORD GetReadPos();
   BOOL End();
protected:

private:
	//���·����С��������Դ��
	BOOL ReAllocBuffer(DWORD &dwNewSize);
protected:
	char *m_pData;
	DWORD m_dwReadPos; //��ȡλ��
	DWORD m_dwWritePos; //д��λ��
	DWORD m_dwAllocSize;//�ܴ�С
};

#endif