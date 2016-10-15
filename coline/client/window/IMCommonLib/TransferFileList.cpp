#include "TransferFileList.h"

#pragma warning(disable:4996)

CTransferFileInfo::CTransferFileInfo():
                   m_wPeerIntranetPort(0),
				   m_wPeerInternetPort(0),
				   m_nPeerFileId(0),
				   m_nLocalFileId(0),
				   m_dwFileSize(0)
{
	//
}

CTransferFileList::CTransferFileList(void):
                   m_nCurrFileId(0)
{
	//
}

void CTransferFileInfo::Assign(const CTransferFileInfo &Src)
{
	hOwner = Src.hOwner;   //所有者窗体
	m_strPeerName = Src.m_strPeerName; //对方名称
	m_strDspName = Src.m_strDspName; //显示名称
	m_strLocalFileName = Src.m_strLocalFileName; //本地存储名称,包括路径
	m_strPeerIntranetIp = Src.m_strPeerIntranetIp; //对方局域网IP
	m_wPeerIntranetPort = Src.m_wPeerIntranetPort ; //对方局域网端口
	m_strPeerInternetIp = Src.m_strPeerInternetIp; //对方广域网IP
	m_wPeerInternetPort = Src.m_wPeerInternetPort; //对方广域网端口
	m_nPeerFileId = Src.m_nPeerFileId;       //对方生成的文件ID号
	m_strFileTag = Src.m_strFileTag;          //文件唯一标识
	m_nLocalFileId = Src.m_nLocalFileId;      //本地生成的I
	m_bSender = Src.m_bSender;
	m_dwFileSize = Src.m_dwFileSize; //文件大小
	m_strProFlag = Src.m_strProFlag; //
	m_OfflineSvr = Src.m_OfflineSvr; //
	m_RemoteName = Src.m_RemoteName; //
}


//CTransferFileList 
CTransferFileList::~CTransferFileList(void)
{
	Clear();
}

int CTransferFileList::GetCount()
{
	return m_FileList.size();
}

void CTransferFileList::Clear()
{
	CGuardLock::COwnerLock guard(m_Lock);
	std::map<int, CTransferFileInfo *>::iterator it;
	for (it = m_FileList.begin(); it != m_FileList.end(); it ++)
	{
		delete it->second;
	}
	m_FileList.clear();
}

//是否还有待处理的接收文件
BOOL CTransferFileList::HasPendingRecvFile(HWND hWnd)
{ 
	CGuardLock::COwnerLock guard(m_Lock);
	std::map<int, CTransferFileInfo *>::iterator it;
	for (it = m_FileList.begin(); it != m_FileList.end(); it ++)
	{
		if ((it->second->hOwner == hWnd)
			&& (!it->second->m_bSender)
			&& (it->second->m_strLocalFileName.empty()))
			return TRUE;
	} 
	return FALSE;
}

void CTransferFileList::GetPendingRecvFileList(HWND hOwner, std::vector<int> &List)
{
	CGuardLock::COwnerLock guard(m_Lock);
	std::map<int, CTransferFileInfo *>::iterator it;
	for (it = m_FileList.begin(); it != m_FileList.end(); it ++)
	{
		if ((it->second->hOwner == hOwner)
			&& (!it->second->m_bSender)
			&& (it->second->m_strLocalFileName.empty()))
		{
			List.push_back(it->first);
		}
	}  
}

int CTransferFileList::HasOwnerWindow(HWND hWnd)
{
	int nCount = 0;
	CGuardLock::COwnerLock guard(m_Lock);
	std::map<int, CTransferFileInfo *>::iterator it;
	for (it = m_FileList.begin(); it != m_FileList.end(); it ++)
	{
		if (it->second->hOwner == hWnd)
			nCount ++;
	} 
	return nCount;
}

BOOL CTransferFileList::CheckTransFileIsExists(const char *szPeerName, const char *szDspName)
{
	CGuardLock::COwnerLock guard(m_Lock);
	std::map<int, CTransferFileInfo *>::iterator it;
	for (it = m_FileList.begin(); it != m_FileList.end(); it ++)
	{
		if ((::stricmp(it->second->m_strPeerName.c_str(), szPeerName) == 0)
			&& (::stricmp(it->second->m_strDspName.c_str(), szDspName) == 0))
			return TRUE;
	} 
	return FALSE;
}

BOOL CTransferFileList::CheckIsTrans(const char *szPeerName, const char *szLocalFileName)
{
	CGuardLock::COwnerLock guard(m_Lock);
	std::map<int, CTransferFileInfo *>::iterator it;
	for (it = m_FileList.begin(); it != m_FileList.end(); it ++)
	{
		if ((::stricmp(it->second->m_strPeerName.c_str(), szPeerName) == 0)
			&& (::stricmp(it->second->m_strLocalFileName.c_str(), szLocalFileName) == 0))
			return TRUE;
	} 
	return FALSE;
}

void CTransferFileList::GetOwnerWindowList(HWND hWnd, std::vector<int> &List)
{
	int nCount = 0;
	CGuardLock::COwnerLock guard(m_Lock);
	std::map<int, CTransferFileInfo *>::iterator it;
	for (it = m_FileList.begin(); it != m_FileList.end(); it ++)
	{
		if (it->second->hOwner == hWnd)
			List.push_back(it->first);
	}  
}

BOOL CTransferFileList::DeleteFileInfo(const int nFileId)
{
	CGuardLock::COwnerLock guard(m_Lock);
	std::map<int, CTransferFileInfo *>::iterator it = m_FileList.find(nFileId);
	if (it != m_FileList.end())
	{
		delete it->second;
		m_FileList.erase(it);
		return TRUE;
	}
	return FALSE;
}

BOOL CTransferFileList::ChangeId(const int nOldId, const int nNewId, const char *szLocalFileName)
{
	CGuardLock::COwnerLock guard(m_Lock);
	CTransferFileInfo *pInfo = NULL;
	std::map<int, CTransferFileInfo *>::iterator it = m_FileList.find(nOldId);
	if (it != m_FileList.end())
	{
		pInfo = it->second;
		pInfo->m_nLocalFileId = nNewId;
		if (szLocalFileName)
			pInfo->m_strLocalFileName = szLocalFileName;
		m_FileList.erase(it);
		it = m_FileList.find(nNewId);
		if (it == m_FileList.end())
		{
			m_FileList.insert(std::pair<int, CTransferFileInfo *>(nNewId, pInfo));
			return TRUE;
		} else
		{
			//insert into old
			m_FileList.insert(std::pair<int, CTransferFileInfo *>(nOldId, pInfo));
		} //end else if (it == m_FileList.end())
	} //end if (it != m_FileList.end())
	return FALSE;
}

BOOL CTransferFileList::GetFileInfoByPeerId(const int nFileId, const char *szPeerName, CTransferFileInfo &FileInfo)
{
	CGuardLock::COwnerLock guard(m_Lock);
	std::map<int, CTransferFileInfo *>::iterator it = m_FileList.find(nFileId);
	for (it = m_FileList.begin(); it != m_FileList.end(); it ++)
	{
		if ((it->second->m_nPeerFileId == nFileId) && (stricmp(it->second->m_strPeerName.c_str(), szPeerName) == 0))
		{
			FileInfo.Assign(*(it->second));
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CTransferFileList::GetFileInfoById(const int nFileId, CTransferFileInfo &FileInfo)
{
	CGuardLock::COwnerLock guard(m_Lock);
	std::map<int, CTransferFileInfo *>::iterator it = m_FileList.find(nFileId);
	if (it != m_FileList.end())
	{
		FileInfo.Assign(*(it->second));
		return TRUE;
	}
	return FALSE;
}

BOOL CTransferFileList::GetFileInfoByFlag(const char *szFlag, CTransferFileInfo &FileInfo)
{
	CGuardLock::COwnerLock guard(m_Lock);
	TCHAR szTmp[128] = {0};
	CStringConversion::StringToWideChar(szFlag, szTmp, 127);
	std::map<int, CTransferFileInfo *>::iterator it;
	for (it = m_FileList.begin(); it != m_FileList.end(); it ++)
	{
		if ((!it->second->m_strProFlag.IsEmpty()) && (_tcsicmp(it->second->m_strProFlag.GetData(), szTmp) == 0))
		{
			FileInfo.Assign(*(it->second));
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CTransferFileList::SetFileProFlag(const int nFileId, TCHAR *szProFlag)
{
	CGuardLock::COwnerLock guard(m_Lock);
	std::map<int, CTransferFileInfo *>::iterator it = m_FileList.find(nFileId);
	if (it != m_FileList.end())
	{
		it->second->m_strProFlag = szProFlag;
		return TRUE;
	}
	return FALSE;
}

BOOL CTransferFileList::SetOfflineSvr(int nFileId, const char *szSvr)
{
	CGuardLock::COwnerLock guard(m_Lock);
	std::map<int, CTransferFileInfo *>::iterator it = m_FileList.find(nFileId);
	if (it != m_FileList.end())
	{
		it->second->m_OfflineSvr = szSvr;
		return TRUE;
	}
	return FALSE;
}


BOOL CTransferFileList::SetRemoteName(int nFileId, const char *szRemoteName)
{
	CGuardLock::COwnerLock guard(m_Lock);
	std::map<int, CTransferFileInfo *>::iterator it = m_FileList.find(nFileId);
	if (it != m_FileList.end())
	{
		it->second->m_RemoteName = szRemoteName;
		return TRUE;
	}
	return FALSE;
}

BOOL CTransferFileList::ModifyFilePeerInfo(const int nFileId, const char *szPeerFileId, const char *szPeerIntranetIp, 
		             const char *szPeerIntranetPort, const char *szPeerInternetIp, 
					 const char *szPeerInternetPort)
{
	CGuardLock::COwnerLock guard(m_Lock);
	std::map<int, CTransferFileInfo *>::iterator it = m_FileList.find(nFileId);
	if (it != m_FileList.end())
	{
		if (szPeerFileId)
			it->second->m_nPeerFileId = ::atoi(szPeerFileId);
		if (szPeerIntranetIp)
			it->second->m_strPeerIntranetIp = szPeerIntranetIp;
		if (szPeerIntranetPort)
			it->second->m_wPeerIntranetPort = ::atoi(szPeerIntranetPort);
		if (szPeerInternetIp)
			it->second->m_strPeerInternetIp = szPeerInternetIp;
		if (szPeerInternetPort)
			it->second->m_wPeerInternetPort = ::atoi(szPeerInternetPort);
		return TRUE;
	}
	return FALSE;
}

int  CTransferFileList::AddFileInfo(const char *szPeerName, const char *szDspName, const char *szLocalFileName, 
	                 const TCHAR *szProFlag, const char *szFileTag, const char *szPeerIntranetIp, 
					 const char *szPeerIntranetPort, const char *szPeerInternetIp, const char *szPeerInternetPort, 
					 const char *szPeerFileId, const int nFileId, const char *szFileSize, HWND hOwner, BOOL bSender)
{
	CTransferFileInfo *pItem = new CTransferFileInfo();
	if (szPeerName)
		pItem->m_strPeerName = szPeerName;
	if (szDspName)
		pItem->m_strDspName = szDspName;
	if (szLocalFileName)
		pItem->m_strLocalFileName = szLocalFileName;
	if (szProFlag)
		pItem->m_strProFlag = szProFlag;
	if (szFileTag)
		pItem->m_strFileTag = szFileTag;
	if (szPeerIntranetIp)
		pItem->m_strPeerIntranetIp = szPeerIntranetIp;
	if (szPeerIntranetPort)
		pItem->m_wPeerIntranetPort = (WORD) ::atoi(szPeerIntranetPort);
	if (szPeerInternetIp)
		pItem->m_strPeerInternetIp = szPeerInternetIp;
	if (szPeerInternetPort)
		pItem->m_wPeerInternetPort = (WORD)::atoi(szPeerInternetPort);
	if (szPeerFileId)
		pItem->m_nPeerFileId = ::atoi(szPeerFileId);
	if (szFileSize)
		pItem->m_dwFileSize = ::atol(szFileSize);
	pItem->m_bSender = bSender;
	pItem->hOwner = hOwner;
	m_Lock.Lock();
	m_nCurrFileId ++;
	if (nFileId == 0)
		pItem->m_nLocalFileId = m_nCurrFileId + 0xFFF;
	else
		pItem->m_nLocalFileId = nFileId;
	m_FileList.insert(std::pair<int, CTransferFileInfo *>(pItem->m_nLocalFileId, pItem));
	m_Lock.UnLock();
	return pItem->m_nLocalFileId;
}


//
BOOL CCustomPicItemList::AddItem(CCustomPicItem *pItem)
{
	if (pItem)
	{ 
		CGuardLock::COwnerLock guard(m_Lock);
		m_Items.push_back(pItem);
		return TRUE;		 
	}
	return FALSE;
}

BOOL CCustomPicItemList::DeleteItem(CCustomPicItem *pItem)
{
	CGuardLock::COwnerLock guard(m_Lock);
	if (pItem)
	{
		std::vector<CCustomPicItem *>::iterator it;
		for (it = m_Items.begin(); it != m_Items.end(); it ++) 
		{
			if ((*it) == pItem)
			{
				m_Items.erase(it);
				delete pItem;
				return TRUE;
			} //end if ((*it)
		} //end for (it =
	} //end if (pItem)
	return FALSE;
}

#pragma warning(default:4996)
