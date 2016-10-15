#ifndef __IM_P2SVR_H___
#define __IM_P2SVR_H___

#include <windows.h>

//nErrorCode define
#define ERROR_CODE_START       1  //开始传送
#define ERROR_CODE_TERMINATED  2  //中止传送
#define ERROR_CODE_COMPLETE    3  //发送完成
#define ERROR_CODE_PROGRESS    4  //进度

//文件类型
#define FILE_TYPE_CUSTOMPIC    1 //用户自定义图片
#define FILE_TYPE_NORMAL       2 //普通文件
#define FILE_TYPE_ORGFILE      3 //组织结构文件

typedef void (CALLBACK *LPP2SVRCALLBACK)(int nErrorCode, int nType, WPARAM wParam, LPARAM lParam, void *pOverlapped);

BOOL CALLBACK P2SvrInit();
BOOL CALLBACK P2SvrDestroy();
HANDLE CALLBACK P2SvrAddDlTask(const char *szUrl, const char *szLocalFileName, int nType,
	                        void *pOverlapped, LPP2SVRCALLBACK pCallBack, BOOL bWait);
HANDLE CALLBACK P2SvrAddUpTask(const char *szUrl, const char *szLocalFileName, int nType,
	                        void *pOverlapped, LPP2SVRCALLBACK pCallBack, BOOL bWait);
//szParams updatefile="
HANDLE CALLBACK P2SvrPostFile(const char *szUrl, const char *szLocalFileName, const char *szParams, int nType,
	                        void *pOverlapped, LPP2SVRCALLBACK pCallBack, BOOL bWait);
BOOL CALLBACK P2SvrCancelTask(HANDLE hTask);

#endif
