#ifndef __ECONTRACTSIMPL_H___
#define __ECONTRACTSIMPL_H___
#define WIN32_LEAN_AND_MEAN	
#include <ComBase.h>
#include <smartskin/smartskin.h>
#include <Core/CoreInterface.h>
#include <Commonlib/SqliteDBOP.h>
#include <Commonlib/GuardLock.h>
#include <xml/tinyxml.h>

#include "../IMCommonLib/InstantUserInfo.h"


class CEContactsImpl: public CComBase<>,
	                  public InterfaceImpl<ICoreEvent>,
				      public InterfaceImpl<IProtocolParser>,
					  public InterfaceImpl<IContacts>
{
public:
	CEContactsImpl(void);
	~CEContactsImpl(void);
	friend  void CALLBACK EcontactsHttpCallBack(int nErrorCode, int nType,
		              WPARAM wParam, LPARAM lParam, void *pOverlapped);
	friend  void CALLBACK HeaderHttpDlCallBack(int nErrorCode, int nType,
		              WPARAM wParam, LPARAM lParam, void *pOverlapped);
	friend DWORD CALLBACK GetLinkImageIdByLink(LPCTSTR pstrLink, LPVOID pOverlapped);
	friend  void CALLBACK LinkImageHttpDlCallBack(int nErrorCode, int nType,
		              WPARAM wParam, LPARAM lParam, void *pOverlapped);
public:
	//IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppv);
	//ICoreEvent
	STDMETHOD (DoCoreEvent)(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
		                     LPARAM lParam, HRESULT *hResult);
	STDMETHOD (SetCoreFrameWork)(ICoreFrameWork *pCore);
	STDMETHOD (GetSkinXmlString)(IAnsiString *szXmlString);
	//
	STDMETHOD (CoreFrameWorkError)(int nErrorNo, const char *szErrorMsg);
	//�㲥��Ϣ
	STDMETHOD (DoBroadcastMessage)(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData);
	//
	STDMETHOD (DoWindowMessage)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes);
	//IProtocolParser
	STDMETHOD (DoRecvProtocol)(const BYTE *pData, const LONG lSize);
	STDMETHOD (DoPresenceChange)(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder);
	//IContacts
	STDMETHOD (OrderAllStatusFromSvr)();
	STDMETHOD (LoadContacts)();
	STDMETHOD (GetRealNameById)(const char *szUserName, const char *szDomain, IAnsiString *szName);
	STDMETHOD (GetContactUserInfo)(const char *szUserName, IInstantUserInfo *pInfo);
	STDMETHOD (SetContactUserInfo)(const char *szUserName, const char *szParam, const char *szValue);
	STDMETHOD (GetContactUserValue)(const char *szUserName, const char *szParam, IAnsiString *szValue);
	//xml <i u="user@doamin"/><i u="user2@doamin"/>....
	STDMETHOD (AddOrderUserList)(const char *szXml);
	//xml <i u="user@doamin"/><i u="user2@doamin"/>
	STDMETHOD (DeleteOrderUserList)(const char *szXml);
	//dept list ,����
	STDMETHOD (GetDeptListByUserName)(const char *szUserName, const char *szDomain, IAnsiString *strDeptList);
	//
	STDMETHOD (GetChildListByDeptId)(const char *szDeptId, IUserList *pUserList, int nType);
	//
	STDMETHOD (GetUserListByDeptId)(const char *szDeptId, IUserList *pUserList, BOOL bSubDeptId, int nType);
	//
	STDMETHOD (GetCacheContactsFileName)(IAnsiString *strFileName);
	//
	STDMETHOD (GetUserDeptPath)(const char *szUserName, const char *szDomain, IAnsiString *strDeptPath);
	//��ȡ����
	STDMETHOD (GetMailByUserName)(const char *szUserName, IAnsiString *strMail);
	//��ȡ�û�ĳ������
	STDMETHOD (GetUserValueByParam)(const char *szUserName, const char *szParam, IAnsiString *strValue);
	//��ȡ�ֻ�����
	STDMETHOD (GetPhoneByName)(const char *szUserName, IAnsiString *strPhone);
	//��ȡ�������
	STDMETHOD (GetFaxByName)(const char *szUserName, IAnsiString *strFax);
	//���ݴ���Ż�ȡ�û���
	STDMETHOD (GetUserNameByFax)(const char *szFax, IAnsiString *strUserName, IAnsiString *strRealName);
	//��ȡ�ֻ���
	STDMETHOD (GetCellPhoneByName)(const char *szUserName, IAnsiString *strCellPhone);
	//
	STDMETHOD (GetUserNameByNameOrPhone)(const char *szInput, IAnsiString *strUserName, IAnsiString *strRealName);
	//
	STDMETHOD (DrawContactToUI)(HWND hWnd, const TCHAR *szTreeName, const char *szUserName, void *pParentNode,
		       BOOL bOrder, BOOL bInitPresence, int nType);
	//��ȡ����ͷ��
	STDMETHOD (GetContactHead)(const char *szUserName, IAnsiString *strFileName, BOOL bRefresh);
	//�ϴ�ͷ��
	STDMETHOD (UploadHead)(const char *szFileName);
	//
	STDMETHOD (ExpandTreeNodeToUI)(HWND hWnd, const TCHAR *szTreeName, void *pParentNode, const int nPid); 
	//��ȡ����·��
	STDMETHOD (GetDeptPathNameByUserName)(const char *szUserName, IAnsiString *strPathName);
	//
	//
	STDMETHOD (GetPhoneByRealName)(const char *szRealName2, const char *szRealName3, 
		          IAnsiString *szRealName, IAnsiString *strPhone);
	//
	STDMETHOD (ShowHelpEditWindow)(ICoreEvent *pOwner, const char *szText, int x, int y, int w, int h);
	//
	STDMETHOD (HideHelpEditWindow)();
	//��ʾ������
	STDMETHOD (ShowSearchFrame)(int x, int y, int w, int h);
	//bActive �Ƿ�״̬ 
	//bEnded �Ƿ��������
	STDMETHOD (EditHelpSearchActive)(HWND hWndFrom, const char *szText, BOOL bActived, BOOL bEnded);
	//
	STDMETHOD (EditVirtualKeyUp)(WORD wKey);
	//
	STDMETHOD (AddExtractDept)(const char *szId, const char *szDeptName, const char *szDispSeq, 
		       const char *szParentId, int nType);
	//
	STDMETHOD (AddExtractUser)(const char *szId, const char *szUserName, const char *szRealName, 
		const char *szDeptId, const char *szMobile, const char *szTel, const char *szEmail,
		const char *szFax, int nType);
	// 
	STDMETHOD (DeleteExtractDept)(const char *szId, int nType);
	//
	STDMETHOD (DeleteExtractUser)(const char *szId, int nType);
	//
	STDMETHOD_(int, IsExistsExtraUsers)(const char *szUserName, int nType);
	//
	STDMETHOD (LoadMenuFromExtractDept)(HWND hWnd, const TCHAR *szParentMenu, const char *szParentId, int nType);
	//�Ƿ�Ϊ��֤��ϵ��
	STDMETHOD_(BOOL, IsAuthContact)(const char *szUserName);
	//��ȡ����Ȩ���б�
	STDMETHOD (GetRoleList)(const char *szUserName, IAnsiString *RoleList);
private:
	void ClearUserInfos();
	void SetUserStatus(const char *szUserName, const char *szStatus, const char *szMemo, BOOL bOrder);
	void SetUserInfo(const char *szUserName, const char *szParam, const char *szValue);
	BOOL GetUserSignVer(const char *szUserName, std::string &strVer);
	BOOL GetUsersByDeptId(const char *szDeptId, IUserList *pList, int nType);
	BOOL GetDeptsByParentId(const char *szDeptId, IUserList *pList);
	BOOL CheckContactValid();
	void SetAllUserStauts(const char *szStatus);
	//xml <i u="user@doamin/><i u="user2@domain/>
	BOOL OrderStatus(TiXmlElement *pNode);
	BOOL DeleteStatus(TiXmlElement *pNode);

 
	HRESULT DoItemActivate(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
 
	//
	HRESULT DoLinkEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	//
	HRESULT DoClickEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	//
	DWORD GetImageIdByLink(LPCTSTR lpszLink);
	//
	void ChangeLinkImageId(LPCTSTR lpszLink, DWORD dwId);
private:
	BOOL DownloadOrg(const char *szUrl);
	
	BOOL InitContactDb();
	static BOOL FreeTreeNodeData(CTreeNodeType nodeType, void **pData);
	BOOL DoStatusOrder(HWND hWnd, void *pNode);
	//����ID�Ų�ѯ�û�����
	BOOL GetUserNameById(const int nId, std::string &strUserName, std::string &strRealName);

	//do protocol
	BOOL DoStatusProtocol(TiXmlElement *pNode);
	//do sign change protocol
	BOOL DoSignChange(TiXmlElement *pNode);
	//
	BOOL DoStatusChange(TiXmlElement *pNode);

	//
	BOOL InitEditHelpWindow();

	//�Ƿ����ز���
	BOOL IsHideDept(const char *szDeptId, const char *szRoleId);
	//
	BOOL RoleIdHasHideDeptAuth(const char *szRoleId);
	//
	BOOL ShowEditHelpWindow(int x, int y, int w, int h);
	BOOL DataListDrawToUI(HWND hWnd, const TCHAR *szTreeName, IUserList  *pDataList, void *pParentNode, 
	                                  int nSaveNodeId, void **pSaveNode, BOOL bOrder);
	void LoadContactsByDepts(HWND hWnd, const TCHAR *szTreeName, void *pNode, const char *szDepts, BOOL bOrder);
	//���ڴ��м����û�����
	void LoadContactsByMemory(HWND hWnd, const TCHAR *szTreeName, void *pNode, const char *szParentId, BOOL bOrder,
		 BOOL bInitPresence, int nType);
	//
	void SearchList(const char *szKey, int nIdx, int &h);
	void OpenFrameById(HWND hWndFrom, const int nId);
	void OpenFrameByRealName(HWND hWndFrom, const char *szName); 
	//
	void ClearMemoryUsers();

	//
	int GetUserCorpId(const char *szUserName, int &nSign); //��ȡĳ�˵Ĺ�˾ID��
	//�������
	void SignUpdateToUI(const char *szUID, const char *szUTF8Sign);
	//
	void InitDeptAuthList();
	//
	void InitAuthList();
private:
	ICoreEvent *m_pEditHelpOwner;
	HWND m_hUI;
	HWND m_hEditHelp;
	HWND m_hWndSearch;  //������
	BOOL m_bDlOrgSucc;

	//�������
	int m_iSearchIdx;
	int m_nSelfCorpId; //�Լ��Ĺ�˾ID��
	int m_nUserSign; //�û���Ȩ��
	std::string m_strSearchKey;
	int m_nSearchShownCount;  //�Ѿ���ʾ�ĸ���
	int m_nSearchTotalCount;
	std::vector<std::string> m_DeptList; //�Լ������Ĳ����б�
	std::map<std::string, std::string> m_DeptAuthList; //��Ȩ�б�
	std::map<std::string, std::string> m_RoleList; //Ȩ���б�

	ICoreFrameWork *m_pCore;
	CSqliteDBOP *m_pOrgDb;
	CSqliteDBOP *m_pExtraDb; //��չ�û��ڴ��
	std::string m_strUserName;
	std::string m_strOrgLocalName;
	std::map<CAnsiString_, CInstantUserInfo *> m_UserInfos;
	CGuardLock m_InfoLock; 
	//��������ͼƬ
	std::map<CStdString_, DWORD> m_LinkList;
	CGuardLock m_LinkLock;
};


#endif
