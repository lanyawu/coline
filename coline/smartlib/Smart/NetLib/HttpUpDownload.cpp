#include <fstream>
#include <CommonLib/StringUtils.h>
#include <CommonLib/SystemUtils.h>
#include <NetLib/HttpUpDownload.h>

#pragma warning(disable:4996)

typedef struct CDlLocalFileItem
{
	char szFileName[MAX_PATH];
	ofstream *ofs;
}DL_LOCAL_FILE_ITEM, *LPDL_LOCAL_FILE_ITEM;

CHttpUpDownload::CHttpUpDownload(void)
{
}

CHttpUpDownload::~CHttpUpDownload(void)
{
}

size_t CHttpUpDownload::WriteLocalFile(void *buffer, size_t size, size_t nmemb, void *stream)
{
	LPDL_LOCAL_FILE_ITEM lpData = (LPDL_LOCAL_FILE_ITEM)(stream);
	if (lpData && (!lpData->ofs))
	{
#ifdef _UNICODE
		TCHAR szTemp[MAX_PATH] = {0};
		CStringConversion::StringToWideChar(lpData->szFileName, szTemp, MAX_PATH - 1);
		lpData->ofs = new ofstream(szTemp, std::ios::out | std::ios::binary);
#endif
	}
	if ((lpData && lpData->ofs && lpData->ofs->is_open()))
	{
		lpData->ofs->write((char *)buffer, (std::streamsize)(size * nmemb));
		return nmemb;
	}
	return 0;
}

//上传相关
size_t CHttpUpDownload::ReadLocalFile(void *buffer, size_t size, size_t nmemb, void *stream)
{
	ifstream *ifs = (ifstream *)stream;
	if (ifs)
	{
		ifs->read((char *)buffer, (std::streamsize)(size * nmemb));
		int nSize = (int) ifs->gcount();
		return nSize;
	}
	return 0;
}

int CHttpUpDownload::DownloadProgressFun(void* ptr, double fDlTotal, double fDlNow, double fUlTotal, double fUlNow)
{
	HWND * phWnd = (HWND *)ptr;
	if (phWnd)
	{
		int n = (int)(fDlNow);
		int nDlTotal = (int)fDlTotal;
		::PostMessage((*phWnd), WM_HTTPDLPROGRESS, WPARAM(nDlTotal) , LPARAM(n));
	}
	return 0;
}

BOOL CHttpUpDownload::DownloadFile(const char *szUrl, const char *szLocalFileName, HWND hProgressWnd)
{
	CURL *pCurl = curl_easy_init();
	CURLcode res;
	if (pCurl)
	{
		DL_LOCAL_FILE_ITEM Param = {0};
		strncpy(Param.szFileName, szLocalFileName, MAX_PATH - 1);
		curl_easy_setopt(pCurl, CURLOPT_URL, szUrl);
		//设置下载回调函数
		curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteLocalFile);
		//设置参数
		curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &Param);
		//进度条设置
		if (hProgressWnd)
		{
			curl_easy_setopt(pCurl, CURLOPT_NOPROGRESS, FALSE);
			//设置进度条回调函数
			curl_easy_setopt(pCurl, CURLOPT_PROGRESSFUNCTION, DownloadProgressFun);
			//设置进度条参数
			curl_easy_setopt(pCurl, CURLOPT_PROGRESSDATA, &hProgressWnd);
		}
		//
		res = curl_easy_perform(pCurl);
        //清除
		curl_easy_cleanup(pCurl);
		if (Param.ofs)
		{
			Param.ofs->close();
			delete Param.ofs;
		}
		return (res == CURLE_OK);
	}
	return FALSE;
}


BOOL CHttpUpDownload::UploadLocalFile(const char *szUrl, const char *szLocalFileName, HWND hProgressWnd)
{
	ifstream ifs;
	int nFileSize = 0;
#ifdef _UNICODE
	TCHAR szTemp[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szLocalFileName, szTemp, MAX_PATH - 1);
	ifs.open(szTemp, std::ios::in | std::ios::binary);
#endif
	if (ifs.is_open())
	{
		ifs.seekg(0, std::ios::end);
		nFileSize = (int) ifs.tellg();
		ifs.seekg(0, std::ios::beg);
		CURL *pCurl = curl_easy_init();
		CURLcode res = CURLE_CONV_FAILED;
		if (pCurl)
		{
			//设置读取回调
			//curl_easy_setopt(pCurl, CURLOPT_READFUNCTION, ReadLocalFile);
			//设置上传状态
			curl_easy_setopt(pCurl, CURLOPT_UPLOAD, 1L);
			//设置调试信息
			curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);
			//设置上传方式
			curl_easy_setopt(pCurl, CURLOPT_POST, 1L);
			//设置接收地址
			curl_easy_setopt(pCurl, CURLOPT_URL, szUrl);
			//设置读取参数
			//curl_easy_setopt(pCurl, CURLOPT_READDATA, &ifs);
			//设置参数
			char szParam[MAX_PATH] = {0};
			sprintf(szParam, "upfile=@%s", szLocalFileName);
			curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, szParam);
			//设置文件大小
			//curl_easy_setopt(pCurl, CURLOPT_INFILESIZE_LARGE, nFileSize);
			//开始传送
			res = curl_easy_perform(pCurl);
			//清除
			curl_easy_cleanup(pCurl);
		}
		ifs.close();
		return (res == CURLE_OK);
	}
	return FALSE;
}

//采用post方式上传文件
BOOL CHttpUpDownload::PostLocalFile(DWORD dwUserId, char *szUserFlag, char *szUrl, char *szLocalFileName, char *szFileType, HWND hProgressWnd)
{
	//测试文件是否存在
	if (CSystemUtils::FileIsExists(szLocalFileName))
	{
		CURL *pCurl;
		struct curl_httppost *pFormPost = NULL;
		struct curl_httppost *pLastPtr = NULL;
		struct curl_slist *pHeaderList = NULL;
		//填充上传段 从文件装载数据
		curl_formadd(&pFormPost, &pLastPtr, CURLFORM_COPYNAME, "upfile", CURLFORM_FILE, szLocalFileName, CURLFORM_END);
		//填充用户ID号
		char szUserId[16] = {0};
		sprintf(szUserId, "%d", dwUserId);
		curl_formadd(&pFormPost, &pLastPtr, CURLFORM_COPYNAME, "userid", CURLFORM_COPYCONTENTS, szUserId, CURLFORM_END);
		//填充flag
		curl_formadd(&pFormPost, &pLastPtr, CURLFORM_COPYNAME, "userflag", CURLFORM_COPYCONTENTS, szUserFlag, CURLFORM_END);
		//填充提交
		curl_formadd(&pFormPost, &pLastPtr, CURLFORM_COPYNAME, "submit", CURLFORM_COPYCONTENTS, "send", CURLFORM_END);
        //填充类型
		curl_formadd(&pFormPost, &pLastPtr, CURLFORM_COPYNAME, "filetype", CURLFORM_COPYCONTENTS, szFileType, CURLFORM_END);
		//初始化
		pCurl = curl_easy_init();

		pHeaderList = curl_slist_append(pHeaderList, "Expect:");
		if (pCurl )
		{
			curl_easy_setopt(pCurl, CURLOPT_URL, szUrl);
			curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);
			curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pHeaderList);
			curl_easy_setopt(pCurl, CURLOPT_HTTPPOST, pFormPost);
			//提交
			CURLcode res = CURLE_CONV_FAILED;
			res = curl_easy_perform(pCurl);

			curl_easy_cleanup(pCurl);
			curl_formfree(pFormPost);
			curl_slist_free_all(pHeaderList);
			return (res == CURLE_OK);
		}
	}
	return FALSE;                                                                              
}

#pragma warning(default:4996)
