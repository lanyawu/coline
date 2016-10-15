#ifndef __P2SVRMANAGER_H___
#define __P2SVRMANAGER_H___


#include <fstream>
#include <string>
#include <map>
#include <Commonlib/GuardLock.h>
#include <curl/curl.h>

#include "P2Svr.h"

#define MAX_THREAD_COUNT  32 //最大的同时传送文件数
#define PROGRESS_NOTIFY_INTERVAL 2000  //2秒通知一次进度

class CP2SvrItem
{
public:
	CP2SvrItem();
	~CP2SvrItem();
public:
	std::string m_strUrl;
	std::string m_strLocalFileName;
	std::string m_strParams;
	std::fstream *m_fstream;
	LPP2SVRCALLBACK m_pCallBack;
	BOOL m_bCancel;
	int  m_nFileType;
	int  m_nRes;
	HANDLE m_hThread;
	HANDLE m_hEvent;
	void  *m_pOverlapped;
	int m_nThreadId;
	CURL *m_pUrl;
	DWORD m_dwLastProTime; //最后一次通知进度时间
};

class CP2SvrManager
{
public:
	CP2SvrManager(void);
	~CP2SvrManager(void);
public:
	HANDLE AddTask(const char *szUrl, const char *szLocalFileName, int nType,
		         void *pOverlapped, LPP2SVRCALLBACK pCallBack, BOOL bWait,
				 BOOL bUp = FALSE);
	HANDLE PostHttpFile_(const char *szUrl, const char *szLocalFileName, const char *szParams, int nType,
	                        void *pOverlapped, LPP2SVRCALLBACK pCallBack, BOOL bWait);
	BOOL  CancelTask(HANDLE hTask);
private:
	static DWORD WINAPI _CheckThread(LPVOID lpParam);
	static DWORD WINAPI _DlWorkThread(LPVOID lpParam);
	static DWORD WINAPI _UpWorkThread(LPVOID lpParam);
	static DWORD WINAPI _PostWorkThread(LPVOID lpParam);
	void ResetEvent();
	BOOL CheckTaskIsExists(const char *szUrl, const char *szLocalFileName);
	//
	void ClearAllThread();

	//
	void AddThreadEvent(HANDLE hThread);
	//
	void DeleteThreadEvent(int idx);

	//curl 回调相关
    //写本地文件回调 下载相关
	static size_t WriteLocalFile(void *buffer, size_t size, size_t nmemb, void *stream);
	static int DownloadProgressFun(void* ptr, double fDlTotal, double fDlNow, double fUlTotal, double fUlNow);	
	static int UploadProgressFun(void* ptr, double fDlTotal, double fDlNow, double fUlTotal, double fUlNow);	
	//上传相关
	static size_t ReadLocalFile(void *buffer, size_t size, size_t nmemb, void *stream);
private:
	HANDLE m_hEvents[MAX_THREAD_COUNT + 1];
	HANDLE m_hCheckThread;
	int m_nThreadCount;
	BOOL m_bTerminated;
	CGuardLock m_ThreadLock;
	std::map<HANDLE, CP2SvrItem *> m_Items;
};

#endif
