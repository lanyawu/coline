#include <CommonLib/StringUtils.h>
#include <CommonLib/SystemUtils.h>
#include <CommonLib/FtpUpDown.h>
#include <fstream>

#pragma warning(disable:4996)

CFtpUpDown::CFtpUpDown(void):
            m_hFtp(NULL),
			m_hThread(NULL),
			m_bTerminated(FALSE),
			m_dwFlag(0),
			m_nServerPort(0)
{
	memset(m_szServerIp, 0, 32);
	memset(m_szUserName, 0, 64);
	memset(m_szPwd, 0, 64);
	memset(m_szLocalName, 0, MAX_PATH);
	memset(m_szRemoteName, 0, MAX_PATH);
}

CFtpUpDown::~CFtpUpDown(void)
{
	m_bTerminated = TRUE;

	if (m_hThread)
	{
		WaitForSingleObject(m_hThread, 5000);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	if (m_hFtp)
		::InternetCloseHandle(m_hFtp);
	m_hFtp = NULL;
}


BOOL CFtpUpDown::LoginServer()
{
	if (m_hFtp)
		::InternetCloseHandle(m_hFtp);
	m_hFtp = ::InternetOpen(NULL, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (!m_hFtp)
		return FALSE;
	TCHAR wServerIp[64] = {0};
	TCHAR wUserName[64] = {0};
	TCHAR wPwd[64] = {0};
	CStringConversion::StringToWideChar(m_szServerIp, wServerIp, 64);
	CStringConversion::StringToWideChar(m_szUserName, wUserName, 64);
	CStringConversion::StringToWideChar(m_szPwd, wPwd, 64);
	m_hFtp = ::InternetConnect(m_hFtp, wServerIp, m_nServerPort, wUserName, wPwd, INTERNET_SERVICE_FTP,
		0, NULL);
    if (!m_hFtp) 
		return FALSE;
	return TRUE;
}


BOOL CFtpUpDown::UploadFile(char *szServerIp, short nServerPort, char *szUserName, char *szPwd, char *szLocalFileName, 
							char *szRemoteFileName, DWORD dwFlag, BOOL bIsThread)
{
	if (!m_hThread || !bIsThread)
	{
		if (szServerIp)
			strncpy(m_szServerIp, szServerIp, 31);
		m_nServerPort = nServerPort;
		if (szUserName)
			strncpy(m_szUserName, szUserName, 64);
		if (szPwd)
			strncpy(m_szPwd, szPwd, 64);
		if (szLocalFileName)
			strncpy(m_szLocalName, szLocalFileName, MAX_PATH);
		if (szRemoteFileName)
			strncpy(m_szRemoteName, szRemoteFileName, MAX_PATH);
		m_dwFlag = dwFlag;
		if (bIsThread)
			m_hThread = ::CreateThread(NULL, 0, UploadThread, this, 0, NULL);
		else
			UploadThread(this);
		return TRUE;
	}
	return FALSE;
}

BOOL CFtpUpDown::DownLoadFile(char *szServerIp, short nServerPort, char *szUserName, char *szPwd, char *szLocalFileName, 
							  char *szRemoteFileName, DWORD dwFlag, BOOL bIsThread)
{
	if ((!m_hThread) || (!bIsThread))
	{
		if (szServerIp)
			strncpy(m_szServerIp, szServerIp, 31);
		m_nServerPort = nServerPort;
		if (szUserName)
			strncpy(m_szUserName, szUserName, 64);
		if (szPwd)
			strncpy(m_szPwd, szPwd, 64);
		if (szLocalFileName)
			strncpy(m_szLocalName, szLocalFileName, MAX_PATH);
		if (szRemoteFileName)
			strncpy(m_szRemoteName, szRemoteFileName, MAX_PATH);
		m_dwFlag = dwFlag;
		if (bIsThread)
			m_hThread = ::CreateThread(NULL, 0, DownloadThread, this, 0, NULL);
		else
			DownloadThread(this);
		return TRUE;
	}
	return FALSE;
}

BOOL CFtpUpDown::CreateDirectory(char *szDir)
{
	if (m_hFtp)
	{
		TCHAR wDir[MAX_PATH] = {0};
		CStringConversion::StringToWideChar(szDir, wDir, MAX_PATH);
		return ::FtpCreateDirectory(m_hFtp, wDir);
	}
	return FALSE;
}

BOOL CFtpUpDown::ChangeDirectory(char *szDir)
{
	if (m_hFtp)
	{
		TCHAR wDir[MAX_PATH] = {0};
		CStringConversion::StringToWideChar(szDir, wDir, MAX_PATH);
		return ::FtpSetCurrentDirectory(m_hFtp, wDir);
	}
	return FALSE;
}

BOOL CFtpUpDown::SetFtpDirectory(char *szRemoteDir, char *szFileName)
{
	char szTemp[MAX_PATH] = {0};
	ChangeDirectory("/"); //回到根目录
	char *pStart;
	while(*szRemoteDir != '\0')
	{
		pStart = szRemoteDir;
		while((*szRemoteDir != '\0') && (*szRemoteDir != '/'))
			szRemoteDir ++;
		if (*szRemoteDir == '/') //一个目录
		{
			if (szRemoteDir > pStart)
			{
				memset(szTemp, 0, MAX_PATH);
				memmove(szTemp, pStart, szRemoteDir - pStart);
				if (!ChangeDirectory(szTemp))
				{
					if (!CreateDirectory(szTemp))
					{
						return FALSE;
					} else
						ChangeDirectory(szTemp);
				}
			}
			szRemoteDir ++;
			continue;
		} else //已经终止
		{
			memmove(szFileName, pStart, szRemoteDir - pStart);
		}
	}
	return TRUE;
}


DWORD WINAPI CFtpUpDown::UploadThread(LPVOID lpParam)
 {
	 const int MAX_BUFFER_SIZE = 2048;
	 CFtpUpDown *pThis = (CFtpUpDown *)lpParam;
	 pThis->NotifyStatus(FTP_STATUS_CONNECTING, 0, pThis->m_dwFlag, TRUE);
	 if (!pThis->LoginServer()) //登陆失败
	 {
		 return 0;
	 }
     pThis->NotifyStatus(FTP_STATUS_CONNECTED, 0, pThis->m_dwFlag, TRUE);
	 //创建目录
	 char szFileName[MAX_PATH] = {0};
	 if (!pThis->SetFtpDirectory(pThis->m_szRemoteName, szFileName))
	 {
		 pThis->NotifyStatus(FTP_STATUS_TRANS_FAIL, 0, pThis->m_dwFlag, TRUE);
		 return 0;
	 }
	 TCHAR wFileName[MAX_PATH] = {0};
	 CStringConversion::StringToWideChar(pThis->m_szRemoteName, wFileName, MAX_PATH);
	 HINTERNET h = ::FtpOpenFile(pThis->m_hFtp, wFileName, GENERIC_WRITE, FTP_TRANSFER_TYPE_BINARY, NULL);
	 if (!h)
	 {
		 pThis->NotifyStatus(FTP_STATUS_TRANS_FAIL, 0, pThis->m_dwFlag, TRUE);
		 return 0;
	 }
	 ifstream ifs;
	 memset(wFileName, 0, sizeof(TCHAR) * MAX_PATH);
	 CStringConversion::StringToWideChar(pThis->m_szLocalName, wFileName, MAX_PATH);
	 ifs.open(wFileName, std::ios::binary);
	 if (!ifs.is_open())
	 {
		 pThis->NotifyStatus(FTP_STATUS_TRANS_FAIL, 0, pThis->m_dwFlag, TRUE);
		 return 0;
	 }
	 ifs.seekg(0, std::ios::end);
	 int nTotalSize = ifs.tellg();
	 ifs.seekg(0, std::ios::beg);

	 char lpBuff[MAX_BUFFER_SIZE];
	 DWORD dwSrcSize, dwDstSize;
	 int nTotalWriteSize = 0;
	 while(!pThis->m_bTerminated)
	 {
		 ifs.read(lpBuff, MAX_BUFFER_SIZE);
		 dwSrcSize = ifs.gcount();
		 if (dwSrcSize > 0)
		 {
			 if (!InternetWriteFile(h, lpBuff, dwSrcSize, &dwDstSize))
				 break;
			 if (dwDstSize != dwSrcSize)
				 break;
		 } else
			 break;
		 nTotalWriteSize = nTotalWriteSize + dwDstSize;
		 if (dwSrcSize < MAX_BUFFER_SIZE)
			 break;
		 //显示进度
		 pThis->NotifyStatus(FTP_STATUS_TRANS_PRO, nTotalWriteSize, pThis->m_dwFlag, TRUE);
	 }

	 //close file
	 ifs.close();
	 ::InternetCloseHandle(h);

	 //show progress
	 if (nTotalWriteSize >= nTotalSize) //完成
		 pThis->NotifyStatus(FTP_STATUS_TRANS_SUCC, 0, pThis->m_dwFlag, TRUE);
	 else
		 pThis->NotifyStatus(FTP_STATUS_TRANS_FAIL, nTotalWriteSize, pThis->m_dwFlag, TRUE);
	 return 0;
 }

DWORD WINAPI CFtpUpDown::DownloadThread(LPVOID lpParam)
 {
	 const int MAX_BUFFER_SIZE = 2048;
	 CFtpUpDown *pThis = (CFtpUpDown *)lpParam;
	 pThis->NotifyStatus(FTP_STATUS_CONNECTING, 0, pThis->m_dwFlag, FALSE);
	 if (!pThis->LoginServer()) //登陆失败
	 {
		 return 0;
	 }
     pThis->NotifyStatus(FTP_STATUS_CONNECTED, 0, pThis->m_dwFlag, FALSE);

	 TCHAR wFileName[MAX_PATH] = {0};
	 CStringConversion::StringToWideChar(pThis->m_szRemoteName, wFileName, MAX_PATH);
	 HINTERNET h = ::FtpOpenFile(pThis->m_hFtp, wFileName, GENERIC_READ, FTP_TRANSFER_TYPE_BINARY, NULL);
	 if (!h)
	 {
		 pThis->NotifyStatus(FTP_STATUS_TRANS_FAIL, 0, pThis->m_dwFlag, FALSE);
		 return 0;
	 }
	 ofstream ofs;
	 memset(wFileName, 0, sizeof(TCHAR) * MAX_PATH);
	 CStringConversion::StringToWideChar(pThis->m_szLocalName, wFileName, MAX_PATH);
	 ofs.open(wFileName, std::ios::binary);
	 if (!ofs.is_open())
	 {
		 pThis->NotifyStatus(FTP_STATUS_TRANS_FAIL, 0, pThis->m_dwFlag, FALSE);
		 return 0;
	 }
	 DWORD dwSize = 0;
	 int nTotalSize = 0;
	 if (FtpGetFileSize(h, &dwSize))
	    nTotalSize = dwSize;
     
	 char lpBuff[MAX_BUFFER_SIZE];
	 DWORD dwSrcSize;
	 int nTotalWriteSize = 0;
	 while(!pThis->m_bTerminated)
	 {
		 if (!::InternetReadFile(h, lpBuff, MAX_BUFFER_SIZE, &dwSrcSize))
			 break;
         if (dwSrcSize > 0)
		 {
			 ofs.write(lpBuff, dwSrcSize);
		 }
		 nTotalWriteSize = nTotalWriteSize + dwSrcSize;
		 if (dwSrcSize < MAX_BUFFER_SIZE)
			 break;
		 //显示进度
		 pThis->NotifyStatus(FTP_STATUS_TRANS_PRO, nTotalWriteSize, pThis->m_dwFlag, FALSE);
	 }

	 //close file
	 ofs.close();
	 ::InternetCloseHandle(h);

	 //show progress
	 if (nTotalWriteSize >= nTotalSize) //完成
		 pThis->NotifyStatus(FTP_STATUS_TRANS_SUCC, 0, pThis->m_dwFlag, FALSE);
	 else
		 pThis->NotifyStatus(FTP_STATUS_TRANS_FAIL, nTotalWriteSize, pThis->m_dwFlag, FALSE);
	 return 0;
 }

void CFtpUpDown::NotifyStatus(int nStatus, int nExt, const DWORD &dwFlag, BOOL bIsUpload)
{
	printf("status: %d\n", nStatus);
}

BOOL CFtpUpDown::GetTerminated()
{
	return m_bTerminated;
}

#pragma warning(default:4996)