#ifndef __SCHEDULE_H___
#define __SCHEDULE_H___

#include <map>
#include <commonlib/types.h>
#include <commonlib/guardlock.h>

#define  MAX_EXECUTE_TIEMS ((DWORD)0xFFFFFFFF) //ִ�д���
#define  SCHEDULE_MIN_INTERVAL  1  //��С�ļ��ʱ��
//�ص�����
typedef  void (CALLBACK *LPSCHEDULEFUNC)(LPVOID lpParam);

//����Ԫ
typedef struct CScheduleItem
{
	DWORD dwScheduleId; //����ɣ�
	DWORD dwInterval; //ʱ����
	DWORD dwTimes;    //ִ�д���
	DWORD dwNextExeTime; //��һ�ε�ִ��ʱ��
	BOOL  bIsSuspended; //�Ƿ����
	LPSCHEDULEFUNC pFun;
	LPVOID lpParam;
}SCHEDULE_ITEM, *LPSCHEDULE_ITEM;

//ʱ������
class COMMONLIB_API CSchedule
{
public:
	CSchedule(void);
	virtual ~CSchedule(void);
public:
	DWORD AddSchedule(LPSCHEDULEFUNC pFun, LPVOID lpParam, DWORD dwInterval, BOOL bIsSuspended = FALSE, DWORD dwTimes = MAX_EXECUTE_TIEMS); //����һ������
	void  DeleteSchedule(DWORD dwScheduleId); //ɾ��һ������
	void  Suspend(DWORD dwScheduleId); //����һ������
	void  Resume(DWORD dwScheduleId); //�ָ�һ������
	void  Clear();
private:
	static DWORD WINAPI ExecuteScheduleThread(LPVOID lpParam);
	void   ExecuteSchedule(DWORD &dwWaitTime);
private:
	BOOL   m_bTerminated;
	HANDLE m_hEvent;
	HANDLE m_hThread;
	DWORD  m_dwCurrId;
	CGuardLock m_Lock;
	std::map<DWORD, LPSCHEDULE_ITEM> m_List;
};

#endif