#ifndef __SCHEDULE_H___
#define __SCHEDULE_H___

#include <map>
#include <commonlib/types.h>
#include <commonlib/guardlock.h>

#define  MAX_EXECUTE_TIEMS ((DWORD)0xFFFFFFFF) //执行次数
#define  SCHEDULE_MIN_INTERVAL  1  //最小的间隔时间
//回调函数
typedef  void (CALLBACK *LPSCHEDULEFUNC)(LPVOID lpParam);

//任务单元
typedef struct CScheduleItem
{
	DWORD dwScheduleId; //任务ＩＤ
	DWORD dwInterval; //时间间隔
	DWORD dwTimes;    //执行次数
	DWORD dwNextExeTime; //下一次的执行时间
	BOOL  bIsSuspended; //是否挂起
	LPSCHEDULEFUNC pFun;
	LPVOID lpParam;
}SCHEDULE_ITEM, *LPSCHEDULE_ITEM;

//时间表调度
class COMMONLIB_API CSchedule
{
public:
	CSchedule(void);
	virtual ~CSchedule(void);
public:
	DWORD AddSchedule(LPSCHEDULEFUNC pFun, LPVOID lpParam, DWORD dwInterval, BOOL bIsSuspended = FALSE, DWORD dwTimes = MAX_EXECUTE_TIEMS); //增加一个任务
	void  DeleteSchedule(DWORD dwScheduleId); //删除一个任务
	void  Suspend(DWORD dwScheduleId); //挂起一个任务
	void  Resume(DWORD dwScheduleId); //恢复一个任务
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