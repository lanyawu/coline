#ifndef __TRANSFERFILELIST_H___
#define __TRANSFERFILELIST_H___

#include <string>
#include <map>
#include <vector>
#include <Commonlib/GuardLock.h>
#include <Commonlib/stringutils.h>
//�ļ���Ϣ
class CTransferFileInfo
{
public:
	CTransferFileInfo();
public:
	void Assign(const CTransferFileInfo &Src);
public:
	HWND hOwner;   //�����ߴ���
	std::string m_strPeerName; //�Է�����
	std::string m_strDspName; //��ʾ����
	std::string m_strLocalFileName; //���ش洢����,����·��
	std::string m_strPeerIntranetIp; //�Է�������IP
	WORD m_wPeerIntranetPort; //�Է��������˿�
	std::string m_strPeerInternetIp; //�Է�������IP
	WORD m_wPeerInternetPort; //�Է��������˿�
	int m_nPeerFileId;       //�Է����ɵ��ļ�ID��
	BOOL m_bSender; //�Ƿ����������ͷ�
	std::string m_strFileTag;          //�ļ�Ψһ��ʶ
	CStdString_ m_strProFlag; //
	std::string m_OfflineSvr; //�����ļ�������
	std::string m_RemoteName; //�������ϵ��ļ�����
	int m_nLocalFileId;      //�������ɵ�ID��
	DWORD m_dwFileSize; //�ļ���С
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
	//�Ƿ��д�����Ľ����ļ�
	BOOL HasPendingRecvFile(HWND hWnd);
	void GetPendingRecvFileList(HWND hOwner, std::vector<int> &List);
	BOOL ChangeId(const int nOldId, const int nNewId, const char *szLocalFileName = NULL);
	int  HasOwnerWindow(HWND hWnd);
	void GetOwnerWindowList(HWND hWnd, std::vector<int> &List);
    void Clear();
    int GetCount();
private:
	int m_nCurrFileId; //��ǰID��,����
	CGuardLock m_Lock;
	std::map<int, CTransferFileInfo *> m_FileList;  //�Ա���ID����Ϊ�ؼ���
};

//�Զ���ͼƬ
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
