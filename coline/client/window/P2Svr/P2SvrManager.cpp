#include <Commonlib/DebugLog.h>
#include <Commonlib/stringutils.h>
#include <Commonlib/systemutils.h>
#include <NetLib/httpupdownload.h>
#include "P2SvrManager.h"

#pragma warning(disable:4996)

CP2SvrItem::CP2SvrItem():
            m_nThreadId(0),
			m_hThread(NULL), 
			m_pUrl(NULL),
			m_bCancel(FALSE),
			m_pCallBack(NULL),
			m_fstream(NULL),
			m_nRes(-1),
			m_pOverlapped(NULL)
{
	m_hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

CP2SvrItem::~CP2SvrItem()
{
	if (m_fstream)
	{
		m_fstream->close();
		delete m_fstream;
	}
	if (m_pUrl)
	{
		curl_easy_cleanup(m_pUrl);
		m_pUrl = NULL;
	}
	if (m_hThread)
	{
		::CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	m_nThreadId = 0;
	if (m_hEvent)
		::CloseHandle(m_hEvent);
	m_hEvent = NULL;
}

//CP2SvrManager
CP2SvrManager::CP2SvrManager(void):
               m_nThreadCount(1),
			   m_bTerminated(FALSE)
{
	for (int i = 0; i <= MAX_THREAD_COUNT; i ++)
		m_hEvents[i] = NULL;
	m_hEvents[0] = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hCheckThread = ::CreateThread(NULL, 0, _CheckThread, (LPVOID) this,
		                            THREAD_PRIORITY_NORMAL, NULL);
}


CP2SvrManager::~CP2SvrManager(void)
{
	m_bTerminated = TRUE;
	::SetEvent(m_hEvents[0]);
	ClearAllThread();
	::WaitForSingleObject(m_hCheckThread, 5000);
	::CloseHandle(m_hCheckThread);
}

DWORD WINAPI CP2SvrManager::_CheckThread(LPVOID lpParam)
{
	//
	CP2SvrManager *pThis = (CP2SvrManager *) lpParam;
	while (TRUE)
	{
		int idx = ::WaitForMultipleObjects(pThis->m_nThreadCount, pThis->m_hEvents, FALSE, INFINITE);
		if (idx == 0)
		{
			if (pThis->m_bTerminated)
				break;
			else
				continue;
		}
		pThis->m_ThreadLock.Lock();
		if ((idx > 0) && (idx < pThis->m_nThreadCount))
		{
			std::map<HANDLE, CP2SvrItem *>::iterator it = pThis->m_Items.find(pThis->m_hEvents[idx]);
			if (it != pThis->m_Items.end())
			{
				if (it->second->m_pCallBack)
				{
					PRINTDEBUGLOG(dtInfo, "Thread File Callback, file: %s, Code:%d Thread:%d", it->second->m_strLocalFileName.c_str(), 
						it->second->m_nRes, pThis->m_nThreadCount);
					it->second->m_pCallBack(ERROR_CODE_COMPLETE, it->second->m_nFileType, 0, it->second->m_nRes,
						                      it->second->m_pOverlapped);
				} else
				{
					PRINTDEBUGLOG(dtInfo, "Thread File not Callback, file: %s, Code:%d, Thread:%d", it->second->m_strLocalFileName.c_str(), 
						it->second->m_nRes, pThis->m_nThreadCount);
				}
				delete it->second;
				pThis->m_Items.erase(it);
			} //end if (it != pThis->...				
			pThis->DeleteThreadEvent(idx);
		} // end if ((idx > 0 && idx < pThis->m_nThreadCount))
		pThis->m_ThreadLock.UnLock();
	} //end while(TRUE)
	return 0;
}

void CP2SvrManager::ClearAllThread()
{
	std::vector<HANDLE> vctList;
	m_ThreadLock.Lock();
	std::map<HANDLE, CP2SvrItem *>::iterator it;
	for (it = m_Items.begin(); it != m_Items.end(); it ++)
	{
		vctList.push_back(it->first);
	}
	m_ThreadLock.UnLock();
	while (!vctList.empty())
	{
		HANDLE h = vctList.back();
		vctList.pop_back();
		::SetEvent(h);
	}
}

HANDLE CP2SvrManager::PostHttpFile_(const char *szUrl, const char *szLocalFileName, const char *szParams, int nType,
	                        void *pOverlapped, LPP2SVRCALLBACK pCallBack, BOOL bWait)
{
	if (!CheckTaskIsExists(szUrl, szLocalFileName))
	{
		CP2SvrItem *pItem = new CP2SvrItem();
		pItem->m_pOverlapped = pOverlapped;
		pItem->m_strLocalFileName = szLocalFileName;
		pItem->m_strUrl = szUrl;
		pItem->m_nFileType = nType;
		pItem->m_pCallBack = pCallBack;
		if (szParams)
			pItem->m_strParams = szParams;

		m_ThreadLock.Lock();
		HANDLE hThread = NULL; 
		hThread = ::CreateThread(NULL, 0, _PostWorkThread, pItem, 0, (DWORD *) &pItem->m_nThreadId);
		pItem->m_hThread = hThread;
		if (pItem->m_hThread)
			m_Items.insert(std::pair<HANDLE, CP2SvrItem *>(pItem->m_hEvent, pItem));
		 
		if (!bWait)
			AddThreadEvent(pItem->m_hEvent);
		m_ThreadLock.UnLock();
		if (bWait)
		{
			::WaitForSingleObject(pItem->m_hEvent, INFINITE);
			m_ThreadLock.Lock();
			std::map<HANDLE, CP2SvrItem *>::iterator it = m_Items.find(pItem->m_hEvent);
			if (it != m_Items.end())
			{
				if (it->second->m_pCallBack)
				{
					PRINTDEBUGLOG(dtInfo, "wait Post File Callback, file: %s, Code:%d", it->second->m_strLocalFileName.c_str(), 
						it->second->m_nRes);
					it->second->m_pCallBack(ERROR_CODE_COMPLETE, it->second->m_nFileType, 0, it->second->m_nRes,
						                      it->second->m_pOverlapped);
				} else
				{
					PRINTDEBUGLOG(dtInfo, "wait Post File not Callback, file: %s, Code:%d", it->second->m_strLocalFileName.c_str(), 
						it->second->m_nRes);
				}
				delete it->second;
				m_Items.erase(it); 
			}
			m_ThreadLock.UnLock();
		}
		return hThread;
	}
	return NULL;
}

HANDLE CP2SvrManager::AddTask(const char *szUrl, const char *szLocalFileName, int nType,
		                    void *pOverlapped, LPP2SVRCALLBACK pCallBack, 
							BOOL bWait, BOOL bUp)
{
	if (!CheckTaskIsExists(szUrl, szLocalFileName))
	{
		CP2SvrItem *pItem = new CP2SvrItem();
		pItem->m_pOverlapped = pOverlapped;
		pItem->m_strLocalFileName = szLocalFileName;
		pItem->m_strUrl = szUrl;
		pItem->m_nFileType = nType;
		pItem->m_pCallBack = pCallBack;
		pItem->m_dwLastProTime = 0;
		m_ThreadLock.Lock();
		HANDLE hThread = NULL;
		if (bUp)
		{
			hThread = ::CreateThread(NULL, 0, _UpWorkThread, pItem, 0, (DWORD *) &pItem->m_nThreadId);
			pItem->m_hThread = hThread;
			if (pItem->m_hThread)
				m_Items.insert(std::pair<HANDLE, CP2SvrItem *>(pItem->m_hEvent, pItem));
		} else
		{
			hThread = ::CreateThread(NULL, 0, _DlWorkThread, pItem, 0, (DWORD *) &pItem->m_nThreadId);
			pItem->m_hThread = hThread;
			if (pItem->m_hThread)
				m_Items.insert(std::pair<HANDLE, CP2SvrItem *>(pItem->m_hEvent, pItem));
		}
		if (!bWait)
			AddThreadEvent(pItem->m_hEvent);
		m_ThreadLock.UnLock();
		if (bWait)
		{
			::WaitForSingleObject(pItem->m_hEvent, INFINITE);
			m_ThreadLock.Lock();
			std::map<HANDLE, CP2SvrItem *>::iterator it = m_Items.find(pItem->m_hEvent);
			if (it != m_Items.end())
			{
				if (it->second->m_pCallBack)
				{
					it->second->m_pCallBack(ERROR_CODE_COMPLETE, it->second->m_nFileType, 0, it->second->m_nRes,
						                      it->second->m_pOverlapped);
				}
				delete it->second;
				m_Items.erase(it); 
			}
			m_ThreadLock.UnLock();
		}
		return hThread;
	}
	return NULL;
}

BOOL  CP2SvrManager::CancelTask(HANDLE hTask)
{
	CGuardLock::COwnerLock guard(m_ThreadLock);
	std::map<HANDLE, CP2SvrItem *>::iterator it = m_Items.find(hTask);
	if (it != m_Items.end())
	{
		it->second->m_bCancel = FALSE;
		::SetEvent(it->second->m_hEvent);
		return TRUE;
	}
	return FALSE;
}

void CP2SvrManager::ResetEvent()
{
	::SetEvent(m_hEvents[0]);
}

//
void CP2SvrManager::AddThreadEvent(HANDLE hEvent)
{
	CGuardLock::COwnerLock guard(m_ThreadLock);
	m_nThreadCount ++;
	if (m_nThreadCount > MAX_THREAD_COUNT)
	{
		PRINTDEBUGLOG(dtInfo, "P2Svr Thread count out of ");
		return ;
	}
	m_hEvents[m_nThreadCount - 1] = hEvent;
	ResetEvent();
}

//
void CP2SvrManager::DeleteThreadEvent(int idx)
{
	if ((idx > 0) && (idx < m_nThreadCount))
	{
		std::string strTmp;
		char szTmp[64] = {0};
		for (int i = 0; i < m_nThreadCount; i ++)
		{
			sprintf(szTmp, "\tIdx:%d Event:%d", i, (int)m_hEvents[i]);
			strTmp += szTmp;
		}
		PRINTDEBUGLOG(dtInfo, "before delete idx:%d  status:%s", idx, strTmp.c_str());
		int nMoveCount = m_nThreadCount - idx - 1;
		if (nMoveCount > 0)
		{ 
			memmove(&m_hEvents[idx], &m_hEvents[idx + 1], nMoveCount * sizeof(HANDLE)); 
		}
		m_hEvents[m_nThreadCount -1] = NULL;
		m_nThreadCount --;
		strTmp.clear();
		for (int i = 0; i < m_nThreadCount; i ++)
		{
			sprintf(szTmp, "\tIdx:%d Event:%d", i, (int)m_hEvents[i]);
			strTmp += szTmp;
		}
		PRINTDEBUGLOG(dtInfo, "after delete idx:%d  status:%s", idx, strTmp.c_str());
	} else
	{
		PRINTDEBUGLOG(dtInfo, "Delete Thread Event Idx:%d", idx);
	}
	ResetEvent();
}


DWORD WINAPI CP2SvrManager::_UpWorkThread(LPVOID lpParam)
{
	DWORD dwRes = 0;
	CP2SvrItem *pItem = (CP2SvrItem *)lpParam;
	if (!pItem->m_bCancel)
	{
		int nFileSize = 0;
#ifdef _UNICODE
		TCHAR szTemp[MAX_PATH] = {0};
		CStringConversion::StringToWideChar(pItem->m_strLocalFileName.c_str(), szTemp, MAX_PATH - 1);
		pItem->m_fstream = new  fstream(szTemp, std::ios::in | std::ios::binary);
#endif
		if (pItem->m_fstream->is_open())
		{
			pItem->m_fstream->seekg(0, std::ios::end);
			nFileSize = (int) pItem->m_fstream->tellg();
			pItem->m_fstream->seekg(0, std::ios::beg);
			pItem->m_pUrl = curl_easy_init();
			CURLcode res = CURLE_CONV_FAILED;
			if (pItem->m_pUrl)
			{
				//设置读取回调
				curl_easy_setopt(pItem->m_pUrl, CURLOPT_READFUNCTION, ReadLocalFile);
				//设置上传状态
				curl_easy_setopt(pItem->m_pUrl, CURLOPT_UPLOAD, 1L);
				//设置调试信息
				curl_easy_setopt(pItem->m_pUrl, CURLOPT_VERBOSE, 1L);
				//设置上传方式
				curl_easy_setopt(pItem->m_pUrl, CURLOPT_POST, 1L);
				//设置接收地址
				curl_easy_setopt(pItem->m_pUrl, CURLOPT_URL, pItem->m_strUrl.c_str());
				//设置读取参数
				curl_easy_setopt(pItem->m_pUrl, CURLOPT_READDATA, pItem);
				//设置参数
				char szParam[MAX_PATH] = {0};
				sprintf(szParam, "upfile=@%s", pItem->m_strLocalFileName.c_str());
				curl_easy_setopt(pItem->m_pUrl, CURLOPT_POSTFIELDS, szParam);
				//设置文件大小
				//curl_easy_setopt(pCurl, CURLOPT_INFILESIZE_LARGE, nFileSize);
				//开始传送
				PRINTDEBUGLOG(dtInfo, "Start UPload file %s", pItem->m_strLocalFileName.c_str());
				res = curl_easy_perform(pItem->m_pUrl);
				int retcode = 0;
				CURLcode return_code = curl_easy_getinfo(pItem->m_pUrl, CURLINFO_RESPONSE_CODE , &retcode);    
	            if (CURLE_OK == return_code && 200 == retcode)  //返回200 表示成功
				{
					PRINTDEBUGLOG(dtInfo, "Upload file %s SUCC, Error:%d", pItem->m_strLocalFileName.c_str(), retcode); 
					res = CURLE_OK;
				} else
				{
					PRINTDEBUGLOG(dtInfo, "Upload file %s Failed, Error:%d", pItem->m_strLocalFileName.c_str(), retcode); 
					res = CURLE_SEND_ERROR;
				}
				//清除
				curl_easy_cleanup(pItem->m_pUrl);
			}
			pItem->m_fstream->close();
			dwRes = res;
		} else
		{
			PRINTDEBUGLOG(dtInfo, "upload file: %s ,create url failed", pItem->m_strLocalFileName.c_str());
			dwRes = 0xFFFFFFFF; 
		}
	} else
	{
		PRINTDEBUGLOG(dtInfo, "upload file: %s, cancel upload file", pItem->m_strLocalFileName.c_str());
	}
	::SetEvent(pItem->m_hEvent);
	return dwRes;
}

DWORD WINAPI CP2SvrManager::_DlWorkThread(LPVOID lpParam)
{
	CP2SvrItem *pItem = (CP2SvrItem *)lpParam;
	if (!pItem->m_bCancel)
	{
		pItem->m_pUrl = curl_easy_init();
	    CURLcode res;
	    if (pItem->m_pUrl)
	    { 
			curl_easy_setopt(pItem->m_pUrl, CURLOPT_URL, pItem->m_strUrl.c_str());
			//设置下载回调函数
			curl_easy_setopt(pItem->m_pUrl, CURLOPT_WRITEFUNCTION, WriteLocalFile);
			//设置参数
			curl_easy_setopt(pItem->m_pUrl, CURLOPT_WRITEDATA, pItem);
			//进度条设置
			if (pItem->m_pCallBack)
			{
				pItem->m_pCallBack(ERROR_CODE_START, 0, 0, 0, pItem->m_pOverlapped);
				curl_easy_setopt(pItem->m_pUrl, CURLOPT_USERAGENT, "FireFox"); 
				curl_easy_setopt(pItem->m_pUrl, CURLOPT_NOPROGRESS, FALSE);
				//设置进度条回调函数
				curl_easy_setopt(pItem->m_pUrl, CURLOPT_PROGRESSFUNCTION, DownloadProgressFun);
				//设置进度条参数
				curl_easy_setopt(pItem->m_pUrl, CURLOPT_PROGRESSDATA, pItem);
			}
			//
			res = curl_easy_perform(pItem->m_pUrl);
			if (pItem->m_fstream)
			{
				pItem->m_fstream->close();
			}
			int retcode = 0;
			CURLcode return_code = curl_easy_getinfo(pItem->m_pUrl, CURLINFO_RESPONSE_CODE , &retcode);    
            if (CURLE_OK == return_code && 200 == retcode)  //返回200 表示成功
			{
				PRINTDEBUGLOG(dtInfo, "download file %s SUCC, Error:%d", pItem->m_strLocalFileName.c_str(), retcode);
				res = CURLE_OK;
			} else
			{
				PRINTDEBUGLOG(dtInfo, "download file %s Failed, Error:%d", pItem->m_strLocalFileName.c_str(), retcode);
				CSystemUtils::DeleteFilePlus(pItem->m_strLocalFileName.c_str());
				res = CURLE_RECV_ERROR;
			}
	        //清除
			curl_easy_cleanup(pItem->m_pUrl);
			pItem->m_pUrl = NULL; 
			pItem->m_nRes = res; 
		} //end if (pItem->m_pUrl)
	} //end if (!pItem->m_bCancel)
	::SetEvent(pItem->m_hEvent);
	return 0;
}

DWORD WINAPI CP2SvrManager::_PostWorkThread(LPVOID lpParam)
{
	CP2SvrItem *pItem = (CP2SvrItem *)lpParam;
	if (!pItem->m_bCancel)
	{   
		struct curl_httppost *pFormPost = NULL;
		struct curl_httppost *pLastPtr = NULL;
		struct curl_slist *pHeaderList = NULL;
		//填充上传段 从文件装载数据
		curl_formadd(&pFormPost, &pLastPtr, CURLFORM_COPYNAME, "upfile", CURLFORM_FILE, pItem->m_strLocalFileName.c_str(), CURLFORM_END);
		
		if (!pItem->m_strParams.empty())
		{
			std::vector<std::string> vcParams;
			int nIdx = 0;
			char szTmp[512] = {0};
			while (TRUE)
			{
				memset(szTmp, 0, 512);
				if (!CSystemUtils::GetStringBySep(pItem->m_strParams.c_str(), szTmp, ';', nIdx))
					break;
				vcParams.push_back(szTmp);
				nIdx ++;
			}
			std::string strParam, strValue;
			while (!vcParams.empty())
			{
				strParam = vcParams.back();
				vcParams.pop_back();
				int nPos = strParam.find('=');
				if (nPos != std::string::npos)
				{
					strValue = strParam.substr(nPos + 1);
			        strParam = strParam.substr(0, nPos);
					curl_formadd(&pFormPost, &pLastPtr, CURLFORM_COPYNAME, strParam.c_str(), 
						CURLFORM_COPYCONTENTS, strValue.c_str(), CURLFORM_END);
				} //end if (nPos != std::string::npos)
			} //end while(!vcParams.empty())
		} //end if (pItem->m_strParams...
  
		//初始化
		pItem->m_pUrl = curl_easy_init();

		pHeaderList = curl_slist_append(pHeaderList, "Expect:");
		if (pItem->m_pUrl)
		{
			curl_easy_setopt(pItem->m_pUrl, CURLOPT_URL, pItem->m_strUrl.c_str());
			curl_easy_setopt(pItem->m_pUrl, CURLOPT_VERBOSE, 1L);
			curl_easy_setopt(pItem->m_pUrl, CURLOPT_HTTPHEADER, pHeaderList);
			curl_easy_setopt(pItem->m_pUrl, CURLOPT_HTTPPOST, pFormPost);
			if (pItem->m_pCallBack)
				pItem->m_pCallBack(ERROR_CODE_START, 0, 0, 0, pItem->m_pOverlapped);
			curl_easy_setopt(pItem->m_pUrl, CURLOPT_NOPROGRESS, FALSE);
			//设置进度条回调函数
			curl_easy_setopt(pItem->m_pUrl, CURLOPT_PROGRESSFUNCTION, UploadProgressFun);
			//设置进度条参数
			curl_easy_setopt(pItem->m_pUrl, CURLOPT_PROGRESSDATA, pItem);
			//提交
			CURLcode res = CURLE_CONV_FAILED;
			//开始传送
			PRINTDEBUGLOG(dtInfo, "Start Post file %s", pItem->m_strLocalFileName.c_str());
			res = curl_easy_perform(pItem->m_pUrl);
			int retcode = 0;
			CURLcode return_code = curl_easy_getinfo(pItem->m_pUrl, CURLINFO_RESPONSE_CODE , &retcode);    
            if (CURLE_OK == return_code && 200 == retcode)  //返回200 表示成功
			{
				PRINTDEBUGLOG(dtInfo, "Post file %s SUCC, Error:%d", pItem->m_strLocalFileName.c_str(), retcode); 
				res = CURLE_OK;
			} else
			{
				PRINTDEBUGLOG(dtInfo, "Post file %s Failed, Error:%d", pItem->m_strLocalFileName.c_str(), retcode); 
				res = CURLE_SEND_ERROR;
			}	
			curl_formfree(pFormPost);
			curl_slist_free_all(pHeaderList);
			curl_easy_cleanup(pItem->m_pUrl);
            pItem->m_pUrl = NULL;
			pItem->m_nRes = res;  
		} else
			pItem->m_nRes = 0xFFFFFFFF; 
	}
	SetEvent(pItem->m_hEvent);
	pItem = NULL;
	return 0;
}

BOOL CP2SvrManager::CheckTaskIsExists(const char *szUrl, const char *szLocalFileName)
{
	std::map<HANDLE, CP2SvrItem *>::iterator it;
	for (it = m_Items.begin(); it != m_Items.end(); it ++)
	{
		if ((::stricmp(szUrl, it->second->m_strUrl.c_str()) == 0)
			&& (::stricmp(szLocalFileName, it->second->m_strLocalFileName.c_str()) == 0))
			return TRUE;
	} //end for (it = m_Items.begin(); it != ...
	return FALSE;
}

//上传相关
size_t CP2SvrManager::ReadLocalFile(void *buffer, size_t size, size_t nmemb, void *stream)
{
	CP2SvrItem *lpData = (CP2SvrItem *)stream;
	if ((!lpData) || (lpData->m_bCancel))
		return 0;
	if (lpData->m_fstream && lpData->m_fstream->is_open())
	{
		lpData->m_fstream->read((char *)buffer, (std::streamsize)(size * nmemb));
		int nSize = (int) lpData->m_fstream->gcount();
		return nSize;
	}
	return 0;
}

size_t CP2SvrManager::WriteLocalFile(void *buffer, size_t size, size_t nmemb, void *stream)
{
	CP2SvrItem *lpData = (CP2SvrItem *)(stream);
	if ((!lpData) || (lpData->m_bCancel))
		return 0; //terminate
	if (!lpData->m_fstream)
	{
#ifdef _UNICODE
		TCHAR szTemp[MAX_PATH] = {0};
		CStringConversion::StringToWideChar(lpData->m_strLocalFileName.c_str(), szTemp, MAX_PATH - 1);
		lpData->m_fstream = new fstream(szTemp, std::ios::out | std::ios::binary);
#endif
	}
	if (lpData->m_fstream && lpData->m_fstream->is_open())
	{
		lpData->m_fstream->write((char *)buffer, (std::streamsize)(size * nmemb));
		return nmemb;
	}
	return 0;
}

int CP2SvrManager::DownloadProgressFun(void* ptr, double fDlTotal, double fDlNow, double fUlTotal, double fUlNow)
{	
	CP2SvrItem *lpData = (CP2SvrItem *)ptr;
	if (lpData->m_pCallBack)
	{

		if ((::GetTickCount() - lpData->m_dwLastProTime) > PROGRESS_NOTIFY_INTERVAL)
		{
		    int nDlNow = (int)(fDlNow);
		    int nDlTotal = (int)fDlTotal;
			lpData->m_pCallBack(ERROR_CODE_PROGRESS, lpData->m_nFileType, WPARAM(nDlTotal), LPARAM(nDlNow), lpData->m_pOverlapped);
			lpData->m_dwLastProTime = ::GetTickCount();
		}
	}
	return 0;
}

int CP2SvrManager::UploadProgressFun(void* ptr, double fDlTotal, double fDlNow, double fUlTotal, double fUlNow) 
{
	CP2SvrItem *lpData = (CP2SvrItem *)ptr;
	if (lpData->m_pCallBack)
	{
		if ((::GetTickCount() - lpData->m_dwLastProTime) > PROGRESS_NOTIFY_INTERVAL)
		{
			int nUpNow = (int)fUlNow;
		    int nUpTotal = (int)fUlTotal; 
			lpData->m_pCallBack(ERROR_CODE_PROGRESS, lpData->m_nFileType, WPARAM(nUpTotal), LPARAM(nUpNow), lpData->m_pOverlapped);
			lpData->m_dwLastProTime = ::GetTickCount();
		}
	}
	return 0;
}
#pragma warning(default:4996)
