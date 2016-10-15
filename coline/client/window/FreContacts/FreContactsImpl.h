#ifndef __FRECONTACTSIMPL_H___
#define __FRECONTACTSIMPL_H___

#include <map>
#include <ComBase.h>
#include <vector>
#include <Core/CoreInterface.h>
#include <Commonlib/GuardLock.h>
#include <Commonlib/stringutils.h> 
#include <xml/tinyxml.h>

#include "../IMCommonLib/InterfaceUserList.h"
#define FRE_CONTACT_DEPT_ID  "1"  //常用联系人
#define EXT_CONTACT_DEPT_ID  "2"  //外部联系人
#define TEL_CONTACT_DEPT_ID  "3"  //电话本联系人

typedef struct CFreContactItem
{
	enum emFreContactType
	{
		FRE_CONTACT_TYPE_ADDDEPT,
		FRE_CONTACT_TYPE_MODIFYDEPT,
		FRE_CONTACT_TYPE_DELDEPT,
		FRE_CONTACT_TYPE_ADDUSER,
		FRE_CONTACT_TYPE_REMARKUSER,
		FRE_CONTACT_TYPE_DELUSER
	} nType;
	BOOL bSucc; //
    std::string strId;
	std::string strUserName;
	std::string strRealName;
	std::string strRemarkName;
	std::string strParentId;
	std::string strMobile;
	std::string strTel;
	std::string strFax;
	std::string strEmail;
	void *pParentNode;
	BOOL bShowTip;
} FRE_CONTACT_ITEM, *LPFRE_CONTACT_ITEM;

//导入的用户结构
 
typedef struct CImport_User_Item
{
	std::string strFirstName;
	std::string strLastName;
	std::string strInfoName;
	std::string strGender;
	std::string strCompany;
	std::string strDepart;
	std::string strDuty;
	std::string strOfficeTel;
	std::string strOfficeEmail;
	std::string strOfficeFax;
	std::string strOfficeMobil;
	std::string strOfficeHomePage;
	std::string strOfficeAddress;
	std::string strOfficeZipCode;
	std::string strBirthady;
	std::string strAnniversary;
	std::string strPersonTel;
	std::string strPersonMobile;
	std::string strPersonEmail;
	std::string strPersonFax;
	std::string strPersonAddr;
	std::string strPersonZipCode;
	std::string strRemark;
	std::string strChineseCode;
	std::string strGroups;
}IMPORT_USER_ITEM, *LPIMPORT_USER_ITEM;

//导入的结构
typedef struct CImport_Dept_Item
{
	int nSrcId;
	BOOL bSucc;
	std::string strId;
	std::string strName;
} IMPORT_DEPT_ITEM, *LPIMPORT_DEPT_ITEM;

class CFreContactsImpl:  public CComBase<>, 
					     public InterfaceImpl<ICoreEvent>,
						 public InterfaceImpl<IProtocolParser>,
					     public InterfaceImpl<IFreContacts>
{
public:
	CFreContactsImpl(void);
	~CFreContactsImpl(void);
public:
		//IUnknown
	STDMETHOD (QueryInterface)(REFIID riid, LPVOID *ppv);

  
	//ICoreEvent
	STDMETHOD (DoCoreEvent)(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
		                     LPARAM lParam, HRESULT *hResult);
	STDMETHOD (SetCoreFrameWork)(ICoreFrameWork *pCore);
	STDMETHOD (GetSkinXmlString)(IAnsiString *szXmlString);
	//
	STDMETHOD (CoreFrameWorkError)(int nErrorNo, const char *szErrorMsg);
	//广播消息
	STDMETHOD (DoBroadcastMessage)(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData);
	//
	STDMETHOD (DoWindowMessage)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes);

	//
	STDMETHOD (DoRecvProtocol)(const BYTE *pData, const LONG lSize);
	STDMETHOD (DoPresenceChange)(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder);

	//
	STDMETHOD (AddFreContactDept)(const char *szId, const char *szDeptName, const char *szParentId, 
		const char *szDispSeq);
	STDMETHOD (AddFreContactUser)(const char *szId, const char *szUserName, const char *szRealName,
		const char *szRemark, const char *szDeptId);
	//
	//
	STDMETHOD (UpdateFreContactRemark)(const char *szId, const char *szRemark);
	//
	STDMETHOD (DeleteFreContactDept)(const char *szId, const char *szName);
	//
	STDMETHOD (DeleteFreContactUser)(const char *szId, const char *szUserName);
	//
	STDMETHOD_(int, IsExistsFreContact)(const char *szUserName); 
	//导入本地用户记录
	STDMETHOD (ImportContactFromLocal)(const char *szFileName);
private:
	BOOL DoContactProtocol(const char *szType, TiXmlElement *pNode);
	BOOL GetUserNameByTree(HWND hWnd, const TCHAR *szTreeName, std::string &strUserName, BOOL &bGroup);
	HWND OpenChatFrameByTree(HWND hWnd, const TCHAR *szTreeName);
	void ReCreateFreMenu();
	HRESULT DoMenuCommand(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	//
	HRESULT DoInitMenuPopup(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	
	//导入电话本
	BOOL ImportContactFromVCF(const char *szFileName);
	//备注联系人
	BOOL DoRemarkContact(HWND hWnd);
	//
	BOOL DoAddDept(const char *szParentId);
	//
	BOOL DoAddExtContact(HWND hWnd);
	//
	BOOL DoContactSvrAckProto(TiXmlElement *pNode, TiXmlElement *pChild);
	//
	int AddPendList(CFreContactItem::emFreContactType nType, void *pParentNode, const char *szId, const char *szUserName, 
		             const char *szRealName, const char *szRemark, const char *szMobile, 
					 const char *szTel, const char *szEmail, const char *szDeptId, BOOL bShowTip);
	void DoSvrAck(LPFRE_CONTACT_ITEM pItem);
	//添加外部联系人至服务器
	BOOL DoAddContactToSvr(HWND hWnd);
	void UIMapToXml(TiXmlElement *pNode, const char *szAttrName, const TCHAR *szUIName);
	//
	void MapToUI(const char *szText, const TCHAR *szUIName);
	//
	void ViewExtContactCard(const char *szUserName);
	//
	void SetExtContactEnable(BOOL bEnabled);
	//
	BOOL UpdateFromServer(const char *szUserName);
	//
	void ShowContactInfoToUI(const char *szXml, const int nXmlSize);
	//
	void DoImportDeptUser(const int nSrcId, const char *szSvrId, const char *szDeptName);
	//
	void DoAddImporUser(const char *szParentId, LPIMPORT_USER_ITEM pItem);
	//导入未分组成员
	void ImportUserByNullGroups();
	//
	BOOL GetCurrentChildNode(CInterfaceUserList &ulList);
	//查找并导入外部联系人
	BOOL SearchAndImportContact();
	//
	void ClearInfo(HWND hWnd);
private:
	ICoreFrameWork *m_pCore;
	HWND m_hWndMain;
	HWND m_hExtContact;
	BOOL m_bContactInit;
	CStdString_ m_strRemarkName;
	CGuardLock m_PendLock;
	void *m_pParentNode;
	int m_nParentId;
	int m_nPendId;
	std::map<int, LPFRE_CONTACT_ITEM> m_PendList;
	std::map<int, LPIMPORT_DEPT_ITEM> m_ImportList; //导入用的结构
	std::vector<LPIMPORT_USER_ITEM> m_ImportUsers; //导入的用户列表

};

#endif
