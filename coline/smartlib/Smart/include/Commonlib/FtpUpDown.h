#ifndef __FTPUPDOWN_H___
#define __FTPUPDOWN_H___

#include <commonlib/types.h>
#include <wininet.h>

#define FTP_STATUS_CONNECTING 0 //��������
#define FTP_STATUS_CONNECTED  1 //���ӳɹ�
#define FTP_STATUS_TRANS_PRO  2 //���ͽ���
#define FTP_STATUS_TRANS_SUCC 3 //�������
#define FTP_STATUS_TRANS_FAIL 4 //����ʧ��

//FTP�ϴ�����
class COMMONLIB_API CFtpUpDown
{
public:
	CFtpUpDown(void);
	virtual ~CFtpUpDown(void);
public:
	BOOL UploadFile(char *szServerIp, short nServerPort, char *szUserName, char *szPwd, char *szLocalFileName,
		char *szRemoteFileName, DWORD dwFlag, BOOL bIsThread);
	BOOL DownLoadFile(char *szServerIp, short nServerPort, char *szUserName, char *szPwd, char *szLocalFileName, 
		char *szRemoteFileName, DWORD dwFlag, BOOL bIsThread);
	void Terminate();
	BOOL GetTerminated();
protected:
	virtual void NotifyStatus(int nStatus, int nExt, const DWORD &dwFlag, BOOL bIsUpload);
private:
	BOOL LoginServer();
	BOOL SetFtpDirectory(char *szRemoteDir, char *szFileName);	
	BOOL CreateDirectory(char *szDir);
	BOOL ChangeDirectory(char *szDir);

	//thread
	static DWORD WINAPI UploadThread(LPVOID lpParam);
	static DWORD WINAPI DownloadThread(LPVOID lpParam);
private:
	HINTERNET m_hFtp;
	HANDLE m_hThread;
	BOOL m_bTerminated;
	DWORD m_dwFlag;
	char m_szServerIp[32];
	short m_nServerPort;
	char m_szUserName[64];
	char m_szPwd[64];
	char m_szLocalName[MAX_PATH];
	char m_szRemoteName[MAX_PATH];
};


#endif