#ifndef __HTTPUPDOWNLOAD_H___
#define __HTTPUPDOWNLOAD_H___

#include <curl/curl.h>

#define WM_HTTPDLPROGRESS (WM_USER + 0x301)

//http上传下载
class CHttpUpDownload
{
public:
	CHttpUpDownload(void);
	~CHttpUpDownload(void);
public:
	static BOOL DownloadFile(const char *szUrl, const char *szLocalFileName, HWND hProgressWnd);
    static BOOL UploadLocalFile(const char *szUrl, const char *szLocalFileName, HWND hProgressWnd);
	//采用post方式上传文件 用户ID dwUserId, szUserFlag 用户标志串
	static BOOL PostLocalFile(DWORD dwUserId, char *szUserFlag, char *szUrl, char *szLocalFileName, char *szFileType, HWND hProgressWnd);
private:
	//写本地文件回调 下载相关
	static size_t WriteLocalFile(void *buffer, size_t size, size_t nmemb, void *stream);
	static int DownloadProgressFun(void* ptr, double fDlTotal, double fDlNow, double fUlTotal, double fUlNow);	
	//上传相关
	static size_t ReadLocalFile(void *buffer, size_t size, size_t nmemb, void *stream);

};

#endif