#ifndef __HTTPUPDOWNLOAD_H___
#define __HTTPUPDOWNLOAD_H___

#include <curl/curl.h>

#define WM_HTTPDLPROGRESS (WM_USER + 0x301)

//http�ϴ�����
class CHttpUpDownload
{
public:
	CHttpUpDownload(void);
	~CHttpUpDownload(void);
public:
	static BOOL DownloadFile(const char *szUrl, const char *szLocalFileName, HWND hProgressWnd);
    static BOOL UploadLocalFile(const char *szUrl, const char *szLocalFileName, HWND hProgressWnd);
	//����post��ʽ�ϴ��ļ� �û�ID dwUserId, szUserFlag �û���־��
	static BOOL PostLocalFile(DWORD dwUserId, char *szUserFlag, char *szUrl, char *szLocalFileName, char *szFileType, HWND hProgressWnd);
private:
	//д�����ļ��ص� �������
	static size_t WriteLocalFile(void *buffer, size_t size, size_t nmemb, void *stream);
	static int DownloadProgressFun(void* ptr, double fDlTotal, double fDlNow, double fUlTotal, double fUlNow);	
	//�ϴ����
	static size_t ReadLocalFile(void *buffer, size_t size, size_t nmemb, void *stream);

};

#endif