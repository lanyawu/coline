#ifndef __MEMORYUSERLIST_H__
#define __MEMORYUSERLIST_H__

#include <Commonlib/Types.h>

#define MAX_DATA_COUNT  100000

//�ڴ��û������
//���ö��ֲ��ҷ�
class COMMONLIB_API CMemoryUserList
{
public:
	CMemoryUserList(void);
public:
	~CMemoryUserList(void);
protected:
	virtual int GetUserIDByIndex(int nIndex) = 0;
	virtual DWORD GetCount() const = 0;


	virtual int GetUserPos(int nId, int& nResultState);
	virtual int InternalFindUser(int nId);
public:
	virtual int FindInsertPos(int nId);
};

//�ڴ����������ַ�������������Сд������
class CStringMemoryUserList
{
public:
	CStringMemoryUserList(void);
	~CStringMemoryUserList(void);
protected:
	virtual const char *GetUserIDByIndex(int nIndex) = 0;
	virtual DWORD GetCount() const = 0;


	virtual int GetUserPos(const char *szName, int& nResultState);
	virtual int InternalFindUser(const char *szName);
public:
	virtual int FindInsertPos(const char *szName);
};

typedef struct CPointerDataItem
{
	DWORD dwKey; //�ؼ���
	void  *pData; //����
}PDI, *LPPDI;

//���ݹ����ڴ�
class CPointerDataShm
{
public:
	int dwDataCount;					//�û�����
	int nDataIdx[MAX_DATA_COUNT];			//��ֵ����ΪstUsers�����е��±�
	CPointerDataItem stDatas[MAX_DATA_COUNT];		//�û���Ϣ��λ
	int nAllocStack[MAX_DATA_COUNT];		//��ջ�ṹ�Ĳ�λ�����
	int nStackTos;					//��ջָ��
public:
	inline CPointerDataItem& operator[](int nIndex)
		{ return stDatas[nDataIdx[nIndex]]; }
};
//�洢Ϊ����ָʲ��map
class CPointerDataMap: public CMemoryUserList
{
public:
	CPointerDataMap();
	~CPointerDataMap();
protected:
	virtual int GetUserIDByIndex(int nIndex) 
		{ return (*m_pDatas)[nIndex].dwKey; }

	void InitAllocStack();
	int AllocUserSlot();
	void FreeUserSlot(int nIndex);
	bool InternalDeleteUser(DWORD dwPos);

public:
	DWORD GetCount() const { return m_pDatas->dwDataCount; };
	int FindDataPos(DWORD dwKey); //����λ��
	bool InsertData(DWORD dwKey, void *pData);
	bool EraseData(DWORD dwKey);
	void Delete(int nIdx);
	void Clear();
	//��ȡ����
	CPointerDataItem &operator[](int nIdx);
private:
	CPointerDataShm *m_pDatas; //����

};

#endif