#ifndef __MEMORYUSERLIST_H__
#define __MEMORYUSERLIST_H__

#include <Commonlib/Types.h>

#define MAX_DATA_COUNT  100000

//内存用户有序表
//采用二分查找法
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

//内存索引表，以字符串做索引，大小写不敏感
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
	DWORD dwKey; //关键字
	void  *pData; //数据
}PDI, *LPPDI;

//数据共享内存
class CPointerDataShm
{
public:
	int dwDataCount;					//用户个数
	int nDataIdx[MAX_DATA_COUNT];			//其值意义为stUsers数组中的下标
	CPointerDataItem stDatas[MAX_DATA_COUNT];		//用户信息槽位
	int nAllocStack[MAX_DATA_COUNT];		//堆栈结构的槽位分配表
	int nStackTos;					//堆栈指针
public:
	inline CPointerDataItem& operator[](int nIndex)
		{ return stDatas[nDataIdx[nIndex]]; }
};
//存储为数据指什的map
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
	int FindDataPos(DWORD dwKey); //查找位置
	bool InsertData(DWORD dwKey, void *pData);
	bool EraseData(DWORD dwKey);
	void Delete(int nIdx);
	void Clear();
	//获取数据
	CPointerDataItem &operator[](int nIdx);
private:
	CPointerDataShm *m_pDatas; //数据

};

#endif