#ifndef __TRANSFERFILELIST_H___
#define __TRANSFERFILELIST_H___

#include <string>
#include <map>
#include <vector>
#include <Commonlib/GuardLock.h>
#include <Commonlib/stringutils.h>
//文件信息
class CTransferFileInfo
{
public:
	CTransferFileInfo();
public:
	void Assign(const CTransferFileInfo &Src);
public:
	HWND hOwner;   //所有者窗体
	std::string m_strPeerName; //对方名称
	std::string m_strDspName; //显示名称
	std::string m_strLocalFileName; //本地存储名称,包括路径
	std::string m_strPeerIntranetIp; //对方局域网IP
	WORD m_wPeerIntranetPort; //对方局域网端口
	std::string m_strPeerInternetIp; //对方广域网IP
	WORD m_wPeerInternetPort; //对方广域网端口
	int m_nPeerFileId;       //对方生成的文件ID号
	BOOL m_bSender; //是否是主动发送方
	std::string m_strFileTag;          //文件唯一标识
	CStdString_ m_strProFlag; //
	std::string m_OfflineSvr; //离线文件服务器
	std::string m_RemoteName; //服务器上的文件名称
	int m_nLocalFileId;      //本地生成的ID号
	DWORD m_dwFileSize; //文件大小
};


class CTransferFileList
{
public:
	CTransferFileList(void);
	~CTransferFileList(void);
public:
	int  AddFileInfo(const char *szPeerName, const char *szDspName, const char *szLocalFileName, const TCHAR *szProFlag, const char *szFileTag,
		             const char *szPeerIntranetIp, const char *szPeerIntranetPort, const char *szPeerInternetIp, 
					 const char *szPeerInternetPort, const char *szPeerFileId, const int nFileId, const char *szFileSize,
					 HWND hOwner, BOOL bSender = FALSE);
	BOOL ModifyFilePeerInfo(const int nFileId, const char *szPeerFileId, const char *szPeerIntranetIp, 
		             const char *szPeerIntranetPort, const char *szPeerInternetIp, 
					 const char *szPeerInternetPort);
	BOOL CheckIsTrans(const char *szPeerName, const char *szLocalFileName);
	BOOL CheckTransFileIsExists(const char *szPeerName, const char *szDspName);
	BOOL GetFileInfoById(const int nFileId, CTransferFileInfo &FileInfo); 
	BOOL GetFileInfoByPeerId(const int nFileId, const char *szPeerName, CTransferFileInfo &FileInfo);
	BOOL SetFileProFlag(const int nFileId, TCHAR *szProFlag);
	BOOL GetFileInfoByFlag(const char *szFlag, CTransferFileInfo &FileInfo);
	BOOL SetOfflineSvr(int nFileId, const char *szSvr);
	BOOL SetRemoteName(int nFileId, const char *szRemoteName);
	BOOL DeleteFileInfo(const int nFileId);
	//是否还有待处理的接收文件
	BOOL HasPendingRecvFile(HWND hWnd);
	void GetPendingRecvFileList(HWND hOwner, std::vector<int> &List);
	BOOL ChangeId(const int nOldId, const int nNewId, const char *szLocalFileName = NULL);
	int  HasOwnerWindow(HWND hWnd);
	void GetOwnerWindowList(HWND hWnd, std::vector<int> &List);
    void Clear();
    int GetCount();
private:
	int m_nCurrFileId; //当前ID号,递增
	CGuardLock m_Lock;
	std::map<int, CTransferFileInfo *> m_FileList;  //以本地ID号作为关键字
};

//自定义图片
class CCustomPicItem
{
public:
	std::string m_strUrl;
	std::string m_strFlag;
	std::string m_strLocalFileName;
	std::string m_strPeerName; 
	int  m_nFileSize;
	int  m_nFileId;
	HWND m_hOwner;
	void *m_pOverlapped;
};

class CCustomPicItemList
{
public:
	BOOL AddItem(CCustomPicItem *pItem);
	BOOL DeleteItem(CCustomPicItem *pItem);
private:
	CGuardLock m_Lock;
	std::vector<CCustomPicItem *> m_Items;
};

#endif
