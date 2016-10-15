#ifndef ___CLIENTCOREINTERFACE_H____
#define ___CLIENTCOREINTERFACE_H____

#include <ComDef.h>
#include <string> 
// GUID of our COM server
//CORE GUID,��ʼ����
_declspec(selectany) GUID CLSID_CORE_FRAMEWORK = { 0x8d53ab6, 0xf686, 0x445f, 
                                                 { 0xb7, 0x53, 0x2f, 0x86, 0xe8, 0x48, 0xd4, 0x1f } };

//�����ļ����GUID
_declspec(selectany) GUID CLSID_CORE_FRAMEWORKCFG = { 0xc8ffaf80, 0x9c89, 0x47b7, 
                                                    { 0xa4, 0x3, 0xc4, 0xb7, 0xba, 0xec, 0xf6, 0x29 } };
interface ICoreEvent;
interface IProtocolParser;

#define MAX_USER_NAME_SIZE         64 
#define NODE_DATA_CELL_PHONE_SIZE  16

//���ṹ�Է�����
typedef struct
{
	int id;          //id ��
	int pid;         //���ڵ�ID��
	int nDisplaySeq; //��ʾ˳��
	int nStamp;      //������ʱ��
	int bOpened; //0--FALSE  1--TRUE
	char szUserName[MAX_USER_NAME_SIZE];    //�û���
	char szCell[NODE_DATA_CELL_PHONE_SIZE]; //�û��ֻ���
	char *szDisplayName;                    //��ʾ����
}ORG_TREE_NODE_DATA, *LPORG_TREE_NODE_DATA;

//����ͼ������
typedef struct 
{
	int ExtraData;     //��չ���ݳ���
	TCHAR *szImageFile; //ͼƬ�ļ�����
	TCHAR *szTip;       //��ʾ
	TCHAR *szUrl;       //����URL
	TCHAR *szCaption;   //����
}TRAY_ICON_TIP_INFO, *LPTRAY_ICON_TIP_INFO;

//�ַ����ӿ�
interface __declspec(uuid("D92DB011-23D9-4D2E-ADE7-843E3D4B360B")) IAnsiString: public IUnknown
{
	//@�����ַ���
	STDMETHOD (SetString)(const char *strInput) PURE;
	//@׷���ַ���
	STDMETHOD (AppendString)(const char *strAppend) PURE;
	//@��ȡ�ַ�����szOutput �ڴ�ռ��Ѿ�����nSize��ʼ��ֵΪ
	STDMETHOD (GetString)(char *szOutput, int *nSize) PURE;
	//@�����ַ�������
	STDMETHOD_(int,GetSize)() PURE;
};

//XML�ڵ����
interface __declspec(uuid("FD2DA0B1-6F8F-4231-A0A4-CB7D05B4290B")) ITinyXmlNode: public IUnknown
{
};

//XML �ĵ�����
interface __declspec(uuid("1D60AC06-084D-400D-9CCC-DB7F0EBBB06F")) ITinyXmlDocument: public ITinyXmlNode
{
};

//�������ͽӿ�
interface __declspec(uuid("2BEE78E9-5470-4DE6-AB26-1090594415B6")) IFontStyle: public IUnknown
{
	//@������������
	STDMETHOD (SetName)(const char *szFontName) PURE;
	//@���������С
	STDMETHOD (SetSize)(const int nFontSize) PURE;
	//@����������ɫ
	STDMETHOD (SetColor)(const int nColor) PURE;
	//@���������Ƿ�Ӵ�
	STDMETHOD (SetBold)(const BOOL bBold) PURE;
	//@���������Ƿ�б��
	STDMETHOD (SetItalic)(const BOOL bItalic) PURE;
	//@���������Ƿ���»���
	STDMETHOD (SetUnderline)(const BOOL bUnderline) PURE;
	//@
	STDMETHOD (SetStrikeout)(const BOOL bStrikeout) PURE;
	//@��ȡ��������
	STDMETHOD (GetName)(IAnsiString *strName) PURE;
	//@��ȡ�����С
	STDMETHOD_(int, GetSize)() PURE;
	//@��ȡ������ɫ
	STDMETHOD_(int, GetColor)() PURE;
	//@��ȡ�����Ƿ�Ӵ�
	STDMETHOD_ (BOOL, GetBold)() PURE;
	//@��ȡ�����Ƿ�б��
	STDMETHOD_ (BOOL, GetItalic)() PURE;
	//@��ȡ�����Ƿ���»���
	STDMETHOD_ (BOOL, GetUnderline)() PURE;
	//
	STDMETHOD_ (BOOL, GetStrikeout)() PURE;
};
//ĳ���û�����Ϣ��Ҫ
interface __declspec(uuid("324A3120-C426-44E7-905E-25A3921D03B7")) IUserPendMessageTip :public IUnknown
{
	//get
	STDMETHOD (GetUserName)(IAnsiString *strUserName) PURE;
	STDMETHOD_ (int, GetMsgCount)() PURE;
	STDMETHOD (GetMessageTip)(IAnsiString *strMsg) PURE;
	//set
	STDMETHOD (SetUserName)(const char *szUserName) PURE;
	STDMETHOD (SetMsgCount)(const int nCount) PURE;
	STDMETHOD (SetMessageTip)(const char *szMsg) PURE;
};

//��������Ϣ��Ҫ�б�
interface __declspec(uuid("A9418014-1787-4ADC-886C-15431FDFEE04")) IUserPendMessageTipList :public IUnknown
{
	STDMETHOD_ (int, GetPendMsgTipCount)() PURE;
	STDMETHOD (GetFrontMessage)(IUserPendMessageTip *pTip) PURE;
	//
	STDMETHOD (AddPendMsgTip)(IUserPendMessageTip *pTip, BOOL bCopy) PURE;
};

//�û���Ϣ�б�
interface __declspec(uuid("89C554E9-6102-4CDF-9A8B-F9E156FDEA34")) IUserList :public IUnknown
{
	//@����һ���û���Ϣ��
	//@bCopy - TRUE ��Ҫ���·����ڴ沢����ֵ
	STDMETHOD (AddUserInfo)(LPORG_TREE_NODE_DATA pData, BOOL bCopy) PURE;
	//@���Ӳ�����Ϣ
	STDMETHOD (AddDeptInfo)(LPORG_TREE_NODE_DATA pData, BOOL bCopy) PURE;
	//@ȡ��һ���û���Ϣ ����ȳ�
	STDMETHOD (PopBackUserInfo)(LPORG_TREE_NODE_DATA *pData) PURE;
	//@ȡ��һ��������Ϣ 
	STDMETHOD (PopBackDeptInfo)(LPORG_TREE_NODE_DATA *pData) PURE;
	//@ȡ��һ���û���Ϣ �Ƚ��ȳ�
	STDMETHOD (PopFrontUserInfo)(LPORG_TREE_NODE_DATA *pData) PURE;
	//@ȡ��һ��������Ϣ
	STDMETHOD (PopFrontDeptInfo)(LPORG_TREE_NODE_DATA *pData) PURE;
	//@��ȡ�û���¼����
	STDMETHOD_ (DWORD, GetUserCount)() PURE;
	//@��ȡ��������
	STDMETHOD_ (DWORD, GetDeptCount)() PURE;
	//@�б��û��Ƿ����
	STDMETHOD_ (BOOL, UserIsExists)(const char *szUserName) PURE;
	//@�����û�����ȡһ���û���Ϣ
	STDMETHOD (PopsUserByName)(const char *szUserName, LPORG_TREE_NODE_DATA *pData) PURE;
	//@�����û��б� 
	STDMETHOD  (CopyTo)(IUserList *pDst, BOOL bCopy) PURE;
	//@������������ID
	STDMETHOD  (SetDeptId)(const char *szPid) PURE;
	//@��ȡ��������ID
	STDMETHOD  (GetDeptId)(IAnsiString *strPid) PURE;
};

//���Ŀ�ܽӿڣ�ICoreFrameWork��
interface __declspec(uuid("730FBAD5-70D5-4898-84DF-5AEBFB6C35F4")) ICoreFrameWork :public IUnknown
{
	//@���ô����������Ϣ
	//@szType - ��������  @szAddress-�����������ַ  @uPort-����������˿�
	//@szUserName - ����������û��� @szUserPwd - �������������
	STDMETHOD (SetAgent)(const char *szType, const char *szAddress, USHORT uPort, 
		                     const char *szUserName, const char *szUserPwd) PURE;
	//send raw protocol
	//@pData -- �������� @lSize --���͵����ݳ���  @lStyle - �����������ͣ���ʱ��0����
	STDMETHOD (SendRawMessage)(const BYTE *pData, const LONG lSize, const LONG lStyle) PURE;
	//��ȡ��½�û���
	STDMETHOD (GetUserName)(IAnsiString *szUserName) PURE;
	//��ȡ��½�û���ʵ���� ���ص�ΪUTF���ַ���
	STDMETHOD (GetUserNickName)(IAnsiString *szUTF8Name) PURE;
	//��ȡ�û���½����
	STDMETHOD (GetUserPassword)(IAnsiString *szUserPwd) PURE;
	//��ȡ�û���½����
	STDMETHOD (GetUserDomain)(IAnsiString *szDomain) PURE;
	//��ȡ�û���½���������
	STDMETHOD (GetUserInGroup)(IAnsiString *szGroupId, IAnsiString *szUTF8GrpName) PURE;
	//�ӷ�������ȡ������Ϣ
	STDMETHOD (GetOfflineMsg)() PURE;
	//�Ƿ�����,���Ѿ����ӵ�����������ʾ������
	STDMETHOD_ (BOOL, GetIsOnline)() PURE;
	//��ʼ������ע����
	STDMETHOD (InitPlugins)(HINSTANCE hInstace) PURE;
	//������м��ز��
	STDMETHOD (ClearPlugins)() PURE;
	//��ʼ����ȫ����
	STDMETHOD (InitSafeSocket)() PURE;
	//����
	STDMETHOD (Offline)() PURE;
	//ע��
	STDMETHOD (Logout)() PURE;
	//������ȫ����
	STDMETHOD (EstablishSafeSocket)(const char *szSvrHost, USHORT uPort) PURE;
	//��֤�û�
	STDMETHOD (AuthUser)(const char *szUserName, const char *szUserPwd, const char *szPresence, const char *szPresenceMemo) PURE;
	//�ں��¼�����
	STDMETHOD (DoCoreEvent)(HWND hWnd, const char *szWndName, const char *szType, const char *szName, WPARAM wParam, 
		                     LPARAM lParam, HRESULT *hResult) PURE;
	//�㲥��Ϣ
	STDMETHOD (BroadcastMessage)(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData) PURE;
	//�޸ĵ�ǰ��½״̬ @szPresence - ״̬���� online offline away busy
	//@szMemo - ״̬˵��
	STDMETHOD (ChangePresence)(const char *szPresence, const char *szMemo) PURE;
	
	//��ȡ�û���½״̬
	STDMETHOD (GetPresence)(const char *szUserName, IAnsiString *strPresence, IAnsiString *strPresenceMemo) PURE;
	//�û�״̬�ı�֪ͨ
	//@bOrder -- �Ƿ�Ϊ����ʱ������״̬֪ͨ
	STDMETHOD (DoPresenceChanged)(const char *szUserName, const char *szPresence, const char *szMemo, BOOL bOrder) PURE;
	//��ȡ��������Ϣ�û���
	//��ʼ�������ļ�
	STDMETHOD (InitConfigure)(const char *szCfgFileName) PURE;
	//��ȡ����������
	//@bRealTime - �Ƿ���Ҫ�ӷ���������
	STDMETHOD (GetSvrParams)(const char *szParamName, IAnsiString *szParamValue, BOOL bRealTime) PURE;
	//����һ������Э��
	//@szProtoName -- Э�����ƣ� xml �ڵ����� 
	//@szProtoType -- Э������   xml type ���� Ϊ NULL������ʱ������������
	STDMETHOD (AddOrderProtocol)(IProtocolParser *pOrder, const char *szProtoName, const char *szProtoType) PURE;
	//ɾ��Э�鶩��
	STDMETHOD (DelOrderProtocol)(IProtocolParser *pOrder, const char *szProtoName, const char *szProtoType) PURE;
	//�����¼�
	//@szWndName  -- �����ĸ������¼�
	//@szCtrlName -- ���ƵĿؼ�  ΪNULLʱ�����ƴ�������пؼ��¼�
	//@szEventType -- ���ƵĿؼ��¼����� ��ΪNULLʱ���ƿؼ��������¼�
	STDMETHOD (AddOrderEvent)(ICoreEvent *pEvent, const char *szWndName, const char *szCtrlName, 
		                      const char *szEventType) PURE;
	//ɾ���¼�����
	//
	STDMETHOD (DeleteOrderEvent)(ICoreEvent *pEvent, const char *szWndName, const char *szCtrlName, 
		                      const char *szEventType) PURE;
	//��ȡ��ǰ�Ĵ�������Ϣ
	//@szUserName -- 
	//@szType -- ��Ϣ���� p2p  grp
	//@strProtocol -- ��Ϣ����
	//@bPop -- �Ƿ�ɾ����Ϣ
	STDMETHOD (GetFrontPendingMsg)(const char *szUserName, const char *szType, IAnsiString *strProtocol, BOOL bPop) PURE;
	//��ȡ�������Ĵ�������Ϣ
	//@strFromName -- 
	STDMETHOD (GetLastPendingMsg)(IAnsiString *strFromName, IAnsiString *strType) PURE;
	//��ȡ��������Ϣ��Ҫ��Ϣ
	STDMETHOD (GetUserPendMsgTipList)(IUserPendMessageTipList *pList) PURE;
	//����һ��������Ϣͼ��
	STDMETHOD (AddTrayMsgTypeIcon)(const char *szMsgType, HICON hIcon) PURE;
	//��ʼ������Ϣ����
	STDMETHOD (StartTrayIcon)(const char *szMsgType, const char *szTip, HICON hIcon) PURE;
	//��ʾ������Ϣ���� -- ֧���߳�ͬ��
	STDMETHOD (ShowTrayTipInfo)(const TCHAR *szImageFile, const TCHAR *szTip, const TCHAR *szUrl,
		                        const TCHAR *szCaption, ICoreEvent *pEvent) PURE; 
	//Pick ��������Ϣ
	STDMETHOD (PickPendingMessage)() PURE; 
	//Ȩ����� �Ƿ���Ȩ��
	STDMETHOD (CanAllowAction)(int nAction) PURE;
};

//�����¼�֪ͨ�ӿڣ�ICoreEvent��
interface __declspec(uuid("596F8BF3-52CC-4BA8-8447-2D0B61D1C0DC")) ICoreEvent :public IUnknown
{
	//�ں��¼�
	//@hWnd -- �¼������Ĵ���HANDLE
	//@szType -- �¼�����
	//@szName -- �¼��ؼ�����
	//@wParam -- ����
	//@lParam -- ����
	//hResult -- �¼���������  0 ��ʾ����Ҫ�ٽ��к�������
	STDMETHOD (DoCoreEvent)(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
		                     LPARAM lParam, HRESULT *hResult) PURE;
	//�����ں˽ӿ�ָ��
	STDMETHOD (SetCoreFrameWork)(ICoreFrameWork *pCore) PURE;
	//��ȡ����Ľ������� XML����
	STDMETHOD (GetSkinXmlString)(IAnsiString *szXmlString) PURE;
	//�ں˴����¼�
	STDMETHOD (CoreFrameWorkError)(int nErrorNo, const char *szErrorMsg) PURE;
	//�㲥��Ϣ
	//@szFromWndName -- ���Դ�������
	//@hWnd -- ���Դ���HANDLE
	//@szType --�㲥��Ϣ����
	//@szContent -- �㲥��Ϣ����
	//@pData -- �㲥��Ϣ����
	STDMETHOD (DoBroadcastMessage)(const char *szFromWndName, HWND hWnd, const char *szType,
		                           const char *szContent, void *pData) PURE;
	//������Ϣ�¼�
	//��Ҫͨ��IUIManager �ӿڶ��ƺ�Żᷢ��
	STDMETHOD (DoWindowMessage)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes) PURE;
};

//Э������ӿڣ�IProtocolParser��
interface __declspec(uuid("561CCE58-CF30-49C5-AE20-BCA2B2B24B02")) IProtocolParser :public IUnknown
{
	//�յ�Э������
	STDMETHOD (DoRecvProtocol)(const BYTE *pData, const LONG lSize) PURE;
	//�û�״̬�ı� ֻ�ж��� sys - presence Э���Ż��յ����¼�
	STDMETHOD (DoPresenceChange)(const char *szUserName, const char *szNewPresence,
		const char *szMemo, BOOL bOrder) PURE;
};


//��������Ϣ�ӿ�(IPendingMsg)
interface __declspec(uuid("551A974C-8149-4C3A-ACBE-D2E431AAEC47")) IPendingMsg: public IUnknown
{
	//����һ����������Ϣ
	STDMETHOD (AddPendingMsg)(const char *szMsgXml) PURE;
};

//ʵʱ�û���Ϣ �ӿ� (IInstantUserInfo)
interface __declspec(uuid("AF392DB7-F7A2-4A63-8324-D29147332C0A")) IInstantUserInfo :public IUnknown
{
	//SET�û���Ϣ����  
	STDMETHOD (SetUserInfo)(const char *szParam, const char *szValue) PURE;
	//�����û�״̬
	STDMETHOD (SetUserStatus)(const char *szStatus) PURE;
	//GET
	STDMETHOD (GetUserInfo)(const char *szParam, IAnsiString *szValue) PURE;
	STDMETHOD (GetUserStatus)(IAnsiString *szStatus) PURE;
};

//�������ӿڣ�IUIManager��
interface __declspec(uuid("278BE7FB-E3D9-4465-A6EA-825B88E9334C")) IUIManager :public IUnknown
{
	//����һ�����Ƥ������Ҫ��IniSkinXmlFile����ܵ��ô˽ӿ�
	STDMETHOD (AddPluginSkin)(const char *szXmlString) PURE;
	//��ʼ��Ƥ������
	STDMETHOD (InitSkinXmlFile)(const char *szXmlFile) PURE;
	//��ʼ��Ƥ��
	STDMETHOD (CreateSkinByXmlStream)(const char *szXml, const int nXmlSize, const char *szSkinPath) PURE;
	//����UI����
	//
	STDMETHOD (CreateUIWindow)(HWND hParent, const char *szWindowName, const PRECT lprc, DWORD dwStyle, DWORD dwExStyle, 
		                     const TCHAR *szCaption, HWND *hWnd) PURE;
	//��ʾһ��ģ̬����
	STDMETHOD (ShowModalWindow)(HWND hParent, const char *szWindowName, const TCHAR *szCaption, 
		           const int X, const int Y, const int nWidth, const int nHeight, int *nModalResult) PURE;
	//������ƴ�����Ϣ
	STDMETHOD (ClearOrderAllMessage)() PURE;
	//���ƴ�����Ϣ 
	STDMETHOD (OrderWindowMessage)(const char *szWndName, HWND hWnd, UINT uMsg, ICoreEvent *pCore) PURE;
	//ɾ��������Ϣ����
	STDMETHOD (DeleteWindowMessage)(const char *szWndName, UINT uMsg, ICoreEvent *pCore) PURE;
	//��ȡ����HANDLE 
	STDMETHOD (GetWindowHWNDByName)(const char *szWndName, HWND *hWnd) PURE;
	//���ʹ�����Ϣ
	STDMETHOD (SendMessageToWindow)(const char *szWndName, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *hr) PURE;
	//���д�����ѭ��
	STDMETHOD (UIManagerRun)() PURE;
	//���ô����ɫ
	STDMETHOD (BlendSkinStyle)(COLORREF cr) PURE;
	//���봰�����
	STDMETHOD (AlphaBackImage)(const char *szFileName) PURE;
	//���ÿؼ�����
	STDMETHOD (GetControlText)(HWND hWnd, const TCHAR *szCtrlName, TCHAR *szText, int *nSize) PURE;
	//��ȡ�ؼ�����
	STDMETHOD (SetControlText)(HWND hWnd, const TCHAR *szCtrlName, const TCHAR *szText) PURE;
};

//��½�ӿڣ�ICoreLogin��
interface __declspec(uuid("80AB8198-5DC5-464B-9282-500161B76A6F")) ICoreLogin :public IUnknown
{
	//position from config
	//��ʾ��½���� 
	//@bAutoLogin - �Ƿ��Զ���ʼ��½
	STDMETHOD (ShowLogonWindow)(const char *szInitUserName, const char *szInitUserPwd, BOOL bAutoLogin) PURE;
	//��ʼ��½������
	STDMETHOD (LogonSvr)(const char *szUserName, const char *szUserPwd, const char *szDomain, 
		                 const char *szPresence, const char *szPresenceMemo) PURE;
	//ȡ����½
	STDMETHOD (CancelLogon)() PURE;
};

//��ϵ�˽ӿڣ�IContacts��
interface __declspec(uuid("08C1DB13-FA4D-476F-8625-3218855B9DEE")) IContacts :public IUnknown
{
	//������ϵ��
	STDMETHOD (LoadContacts)() PURE;
	//��ȡ��ϵ�˻����ļ�����
	STDMETHOD (GetCacheContactsFileName)(IAnsiString *strFileName) PURE;
	//��ȡ�û���ʵ����
	//@szUserName -- �û��� szDomain Ϊ��ʱ  �û���@�ָ�����
	//@szDomain --- ��½����
	STDMETHOD (GetRealNameById)(const char *szUserName, const char *szDomain, IAnsiString *szName) PURE;
	//��ȡ�û�ʵʱ״̬��Ϣ
	STDMETHOD (GetContactUserInfo)(const char *szUserName, IInstantUserInfo *pInfo) PURE;
	//�����û�ʵʱ״̬��Ϣ
	//@szParam
	//@szValue -- ֵ
	STDMETHOD (SetContactUserInfo)(const char *szUserName, const char *szParam, const char *szValue) PURE;
	//��ȡ�û�״̬��Ϣ
	STDMETHOD (GetContactUserValue)(const char *szUserName, const char *szParam, IAnsiString *szValue) PURE;
	//xml <i u="user@doamin"/><i u="user2@doamin"/>....
	STDMETHOD (AddOrderUserList)(const char *szXml) PURE;
	//�����ڴ����û�״̬����Ҫ���û����ߺ�������
	STDMETHOD (OrderAllStatusFromSvr)() PURE;
	//��ȡ����ͷ�� �Ƿ�ӷ������ϸ���
	STDMETHOD (GetContactHead)(const char *szUserName, IAnsiString *strFileName, BOOL bRefresh) PURE;
	//�ϴ�ͷ��
	STDMETHOD (UploadHead)(const char *szFileName) PURE;
	//ɾ���û�״̬������Ϣ <i u="user@doamin"/><i u="user2@doamin"/>
	STDMETHOD (DeleteOrderUserList)(const char *szXml) PURE;
	//dept list ,����
	STDMETHOD (GetDeptListByUserName)(const char *szUserName, const char *szDomain, IAnsiString *strDeptList) PURE;
	//��ȡ�Ӳ����б�
	STDMETHOD (GetChildListByDeptId)(const char *szDeptId, IUserList *pUserList, int nType) PURE;
	// bSubDeptId, �Ƿ�����Ӳ��� nType ���ͣ��Ƿ�Ϊ�ⲿ��ϵ��
	STDMETHOD (GetUserListByDeptId)(const char *szDeptId, IUserList *pUserList, BOOL bSubDeptId, int nType) PURE;
	//��ȡ����ȫ·��
	STDMETHOD (GetUserDeptPath)(const char *szUserName, const char *szDomain, IAnsiString *strDeptPath) PURE;
	//������ϵ����UI
	STDMETHOD (DrawContactToUI)(HWND hWnd, const TCHAR *szTreeName, const char *szUserName, void *pParentNode,
		       BOOL bOrder, BOOL bInitPresence, int nType) PURE;
	//չ���ڵ���UI
	STDMETHOD (ExpandTreeNodeToUI)(HWND hWnd, const TCHAR *szTreeName, void *pParentNode,
		const int nPid) PURE;
	//��ȡ�ֻ�����
	STDMETHOD (GetPhoneByName)(const char *szUserName, IAnsiString *strPhone) PURE;
	//��ȡ�������
	STDMETHOD (GetFaxByName)(const char *szUserName, IAnsiString *strFax) PURE;
	//���ݴ���Ż�ȡ�û���
	STDMETHOD (GetUserNameByFax)(const char *szFax, IAnsiString *strUserName, IAnsiString *strRealName) PURE;
	//��ȡ�ֻ���
	STDMETHOD (GetCellPhoneByName)(const char *szUserName, IAnsiString *strCellPhone) PURE;
	//��ȡ����·��
	STDMETHOD (GetDeptPathNameByUserName)(const char *szUserName, IAnsiString *strPathName) PURE;
	//��ȡ����
	STDMETHOD (GetMailByUserName)(const char *szUserName, IAnsiString *strMail) PURE;
	//��ȡ�û�ĳ������
	STDMETHOD (GetUserValueByParam)(const char *szUserName, const char *szParam, IAnsiString *strValue) PURE;
	//���ݵ绰�����ȡ�û�������ʵ����
	STDMETHOD (GetUserNameByNameOrPhone)(const char *szInput, IAnsiString *strUserName, IAnsiString *strRealName) PURE;
	//��ѯĳ�˵ĵ绰�������ʵ�������²��û���
	STDMETHOD (GetPhoneByRealName)(const char *szRealName2, const char *szRealName3, IAnsiString *szRealName, IAnsiString *strPhone) PURE;
	//��ʾ���������
	STDMETHOD (ShowHelpEditWindow)(ICoreEvent *pOwner, const char *szText, int x, int y, int w, int h) PURE;
	//�������������
	STDMETHOD (HideHelpEditWindow)() PURE;
	//��ʾ������
	STDMETHOD (ShowSearchFrame)(int x, int y, int w, int h) PURE;
	//bActive �Ƿ�״̬ 
	//bEnded �Ƿ��������
	STDMETHOD (EditHelpSearchActive)(HWND hWndFrom, const char *szText, BOOL bActived, BOOL bEnded) PURE;
	//�༭��������
	STDMETHOD (EditVirtualKeyUp)(WORD wKey) PURE;
	//����һ����չ�û���
	STDMETHOD (AddExtractDept)(const char *szId, const char *szDeptName, const char *szDispSeq, 
		                       const char *szParentId, int nType) PURE;
	//����һ����չ�û�
	STDMETHOD (AddExtractUser)(const char *szId, const char *szUserName, const char *szRealName, 
		const char *szDeptId, const char *szMobile, const char *szTel, const char *szEmail,
		const char *szFax, int nType) PURE;
	//ɾ����չ�û����
	STDMETHOD (DeleteExtractDept)(const char *szId, int nType) PURE;
	//ɾ����չ�û�
	STDMETHOD (DeleteExtractUser)(const char *szId, int nType) PURE;
	//�Ƿ������չ�û�
	STDMETHOD_(int, IsExistsExtraUsers)(const char *szUserName, int nType) PURE;
	//װ�س�����ϵ�˲˵�
	STDMETHOD (LoadMenuFromExtractDept)(HWND hWnd, const TCHAR *szParentMenu, const char *szParentId, int nType) PURE;
	//�Ƿ�Ϊ��֤��ϵ��
	STDMETHOD_(BOOL, IsAuthContact)(const char *szUserName) PURE; 
	//��ȡ����Ȩ���б�
	STDMETHOD (GetRoleList)(const char *szUserName, IAnsiString *RoleList) PURE;
};

//��Ϣ�б�
interface __declspec(uuid("B8B2E959-C643-4BCA-914D-87017FA49040")) IMessageList :public IUnknown
{
	//����һ����Ϣ
	STDMETHOD (AddMsg)(const int nMsgId, const char *szType, const char *szFromName,
		               const char *szToName, const char *szTime, const char *szMsg) PURE;
	//����һ������Ϣ
	STDMETHOD (AddRawMsg)(const int nMsgId, const char *szRawMsg) PURE;
	//��ȡ��Ϣ����
	STDMETHOD_(DWORD, GetCount)() PURE;
	//���������Ϣ
	STDMETHOD (Clear)() PURE;
	//��ȡһ����Ϣ
	STDMETHOD (GetMsg)(const int nIdx, int *nMsgId, IAnsiString *szType, IAnsiString *strFromName,
		               IAnsiString *strToName, IAnsiString *strTime, IAnsiString *strMsg) PURE;
	//��ȡһ������Ϣ
	STDMETHOD (GetRawMsg)(const int nIdx, int *nMsgId, IAnsiString *strRawMsg) PURE;
};

//��Ϣ����ӿڣ�IMsgMgr��
interface __declspec(uuid("35F3DD63-2D92-4CC0-8169-C00E2AD00F62")) IMsgMgr :public IUnknown
{
	//init 
	STDMETHOD (InitMsgMgr)(const char *szFileName, const char *szUserName) PURE;
	//����������Ϣ
	STDMETHOD (SaveMsg)(const char *szType, const char *szFromName, const char *szToName,
	                 const char *szTime, const char *szMsg, const char *szBody) PURE;
	//get message 
	STDMETHOD (GetMsg)(const char *szType, const char *szFromName, const int nPage, 
	                const int nPageCount, IMessageList *pList) PURE;
	//��ȡ��Ϣ
	STDMETHOD (GetMsgById)(const char *szType, const int nMsgId, IMessageList *pList) PURE;
	//��ȡRaw��Ϣ
	STDMETHOD (GetRawMsg)(const char *szType, const char *szFromName, const int nPage, 
	                const int nPageCount, IMessageList *pList) PURE;
	//��ȡ��Ϣ����
	STDMETHOD_ (int, GetMsgCount)(const char *szType, const char *szFromName) PURE;
	//nMsg != 0 delete one message
	//nMsg == 0 delete all message from == szFromName and type == szType
	STDMETHOD (ClearMsg)(const int nMsgId, const char *szType, const char *szFromName) PURE;
	//��ȡ��ѯ��Ϣ����
	STDMETHOD_ (int, GetSearchMsgCount)(const char *szType, const char *szKey, const char *szFromName) PURE;
	//��ѯ��ʷ��Ϣ
	STDMETHOD (SearchMsg)(const char *szKey, const char *szType, const char *szFromName,
	                   const int nPage, const int nPageCount, IMessageList *pList) PURE;
	//��ѯ��ʷ��Ϣ
	STDMETHOD (SearchRawMsg)(const char *szKey, const char *szType, const char *szFromName,
	                   const int nPage, const int nPageCount, IMessageList *pList) PURE;
	//������������Ϣ
	STDMETHOD (SaveGroupInfo)(const char *szGrpId, const char *szGrpDspName, const char *szCreator) PURE;
	//��ȡ������������б�
	STDMETHOD (GetGroups)(IUserList *pList) PURE;
	//��ȡ���ŷ������б�
	STDMETHOD (GetSmsUserList)(IUserList *pList) PURE;
};

//����ӿڣ�IChatFrame��
interface __declspec(uuid("8E09D17D-D1D2-47F1-BAC4-D4522CFF4A7F")) IChatFrame :public IUnknown
{
	//��ʾ���촰��
	STDMETHOD_ (HWND, ShowChatFrame)(HWND hWndFrom, const char *szUserName, const char *szUTF8DspName) PURE;
	//�����ļ����󣬵�szFileName --ΪNULL���������ļ�ѡ�񴰿�
	STDMETHOD (SendFileToPeer)(const char *szUserName, const char *szFileName) PURE;
	//����Զ��Э������
	STDMETHOD (SendRmcRequest)(const char *szUserName) PURE;
	//������Ƶ����
	STDMETHOD (SendVideoRequest)(const char *szUserName) PURE;
	//������Ƶ����
	STDMETHOD (SendAudioRequest)(const char *szUserName) PURE;
	//��ʾ������Ϣ
	STDMETHOD (ShowChatMessage)(HWND hOwner, const char *szMsg) PURE;
	//��ʾ������ʾ��Ϣ
	STDMETHOD (ShowChatTipMsg)(HWND hOwner, const TCHAR *szMsg) PURE;
	//��Ƶ���ӳɹ�
	STDMETHOD (VideoConnected)(LPARAM lParam, WPARAM wParam) PURE;
	//���ݴ���Handle ��ȡ�û���
	STDMETHOD (GetUserNameByHWND)(HWND hOwner, IAnsiString *strUserName) PURE;
	//strMsgType ==== "tip"-- ��ʾ
	STDMETHOD (ParserP2PProtocol)(const char *szContent, const int nContentSize, IAnsiString *strDispName,
	                          IAnsiString *strDspTime, IAnsiString *strDspText, IFontStyle *fs, IAnsiString *strMsgType,
							  BOOL *bSelf) PURE;
};

//������ӿڣ�IGroupFrame��
interface __declspec(uuid("69EFE8BC-9A6E-4BA6-A157-CBDE881A1D2A")) IGroupFrame :public IUnknown
{
	//��ʾ�����鴰��
	STDMETHOD (ShowGroupFrame)(const char *szGrpId, const char *szUTF8DspName) PURE;
	//�����������Ա
	STDMETHOD (DrawGroupToUI)(HWND hWnd, const char *szGrpId) PURE;
	//��ʾ��������Ϣ
	STDMETHOD (ShowGroupMessage)(const char *szGrpId, const char *szMsg) PURE;
	//�����ļ���Ⱥ
	STDMETHOD (SendFileToGroup)(const char *szGrpId, const char *szFileName) PURE;
	//���Ͷ�����Ⱥ
	STDMETHOD (SendSMSToGroup)(const char *szGrpId, const char *szSMSText) PURE;
	//�����ʼ���������
	STDMETHOD (SendMailToGroup)(const char *szGrpId) PURE;
	//��ʾ��������ʾ��Ϣ
	STDMETHOD (ShowGroupTipMsg)(HWND hOwner, TCHAR *szMsg) PURE;
	//���ݴ���HANDLE ��ȡ������ID
	STDMETHOD (GetGroupIdByHWND)(HWND hOwner, IAnsiString *strGroupId) PURE;
	//����������ID��ȡ���� 
	STDMETHOD (GetGroupNameById)(const char *szGrpId, IAnsiString *strGrpName) PURE;
	//����������Э��
	STDMETHOD (ParserGroupProtocol)(const char *szContent, const int nContentSize, IAnsiString *strDispName,  IAnsiString *strUserList,
	                          IAnsiString *strDspTime, IAnsiString *strDspText, IFontStyle *fs, IAnsiString *strMsgType,
							  BOOL *bSelf) PURE; 
	//szGrpId �������޸�
	STDMETHOD (CreateGroup)(const char *szGrpId, const char *szGrpName, IUserList *pUser) PURE;
	//�޸ķ�������
	STDMETHOD (UpdateGroupName)(const char *szGrpId, const char *szNewGrpName) PURE;
	//�˳�����
	STDMETHOD (ExitGroupById)(const char *szGrpId) PURE;
	//ɾ������
	STDMETHOD (DeleteGroupById)(const char *szGrpId) PURE; 
};

//���ýӿڣ�IConfigure��
interface __declspec(uuid("C991C7E1-0F8D-467C-986B-48F9BD7D2CA8")) IConfigure :public IUnknown
{
	//�������� or ˽������
	STDMETHOD (InitCfgFileName)(const char *szFileName, const char *szPersonName, BOOL bCommon) PURE;
	//��ȡ���ò���
	STDMETHOD (GetParamValue)(BOOL bCommon, const char *szSection, const char *szParamName, 
		        IAnsiString *szValue) PURE;
	//�������ò���
	STDMETHOD (SetParamValue)(BOOL bCommon, const char *szSection, 
		             const char *szParamName, const char *szValue) PURE;
	//���÷���������
	STDMETHOD (SetSvrParam)(const char *szParam, const char *szValue) PURE;
	//�����û���½��Ϣ
	//
	STDMETHOD (SetUserLoginInfo)(const char *szUserName, const char *szUserPwd, const char *szUserDomain,
		               BOOL bSavePwd, const char *szStatus, const char *szLoginSvrHost, const int nLoginSvrPort) PURE;

	//���õ�½�û���ʵ����
	STDMETHOD (SetUserRealName)(const char *szUserName, const char *szUserDomain, const char *szUTF8RealName) PURE;
	//��ȡ�û���½��½��Ϣ
	STDMETHOD (GetUserInfoByRealName)(const char *szName, IAnsiString *szUserName, IAnsiString *szDomain,
		                          IAnsiString *szPwd, IAnsiString *szPic, IAnsiString *strRealName, IAnsiString *szStatus, 
								  IAnsiString *szSvrHost, IAnsiString *szPort) PURE;
	//��ȡ�û���½����
	STDMETHOD (GetUserNameByRealName)(const char *szName, IAnsiString *szUserName) PURE;
	//szUserInfos XML
	//<?xml version="1.0" encoding="gb2312"?>
	//<users>
	//  <item username="username" password="password" savepwd="y" realname="realname" userpic="12" loginsvrhost="www.nowhelp.cn"
	//            loginsvrport="9902" logindomain="gocom"/>
	//  <item ...>
	//</users>
	STDMETHOD (GetUserLoginUserList)(IAnsiString *szUserInfos) PURE;
	//�ؼ��ּ��
	STDMETHOD (CheckKeyWord)(const char *UTF8Chars) PURE;
	//nId != 0 ==> modify
	STDMETHOD_ (int, AddReplyMessage)(int nId, int nType, const char *szReply) PURE;
	//��ȡ�Զ��ظ���Ϣ
	STDMETHOD (GetReplyMessage)(int nType, IMessageList *strReplys) PURE;
	//ɾ���Զ��ظ�����
	STDMETHOD (DelReplyMessage)(int nId, int nType) PURE;
	//��ȡ�Զ��ظ��ַ�
	STDMETHOD (GetAutoReplyMessage)(IAnsiString *strMsg) PURE;
	//��ȡ�����ϵ���б�
	STDMETHOD (GetRecentlyList)(IUserList *List) PURE;
	//����һ�������ϵ������
	STDMETHOD (AddRecentlyList)(const char *szUserName, const char *szDispName) PURE;
	//�������ϵ����ɾ���û�
	//szUserName Ϊ��ʱɾ�������û�
	STDMETHOD (DelUserFromRecently)(const char *szUserName) PURE;
	//��ȡ�û�����
	STDMETHOD (GetChatFontStyle)(IFontStyle *Style) PURE;
	//��ȡƤ������
	STDMETHOD (GetSkinXml)(const char *szDefaultName, IAnsiString *szXmlString) PURE;
	//��ȡ��ȡ·��
	STDMETHOD (GetPath)(int nPathId, IAnsiString *szPath) PURE;
	//��ȡ��������ַ
	STDMETHOD (GetServerAddr)(int nServerId, IAnsiString *szSvrAddr) PURE;
	//��������
	STDMETHOD (PlayMsgSound)(const char *szType, const char *szUserName, BOOL bLoop) PURE;
	//��ע״̬�ı��û�
	STDMETHOD (AddContactOnlineTip)(const char *szUserName) PURE;
	//ɾ����ע״̬�ı��û�
	STDMETHOD (DelContactOnlineTip)(const char *szUserName) PURE;
	///��ȡ״̬��ע�û��б�
	STDMETHOD (GetContactOnlineTipUsers)(IUserList *List) PURE;
	//�����û������Ƿ���ʾ
	STDMETHOD_(BOOL, IsContactOnlineTip)(const char *szUserName) PURE;
	//
	STDMETHOD (AddWidgetTab)(const char *szTabId, const char *szTabDspName) PURE;
	//
	STDMETHOD (AddWidgetItem)(const char *szTabId, const char *szItemName, const char *szItemDspName,
		                      const char *szItemUrl, const int nImageId, const char *szItemTip) PURE;
	//����XML�ַ��� <widgets><tab id="016FD35A-4DA2-44F8-86EA-C845DDD5F953" name="��ǩ1"/><tab.../></widgets>
	STDMETHOD (GetWidgetTabs)(IAnsiString *strTabs) PURE;
	//����XML�ַ��� <widgets><item tabid="016FD35A-4DA2-44F8-86EA-C845DDD5F953" id="id1" name="GoCom"
	//  caption="ͳһͨ��ƽ̨" url="gocom://" imageid="25" tip="ͳһͨ��ƽ̨"/><item... /></widgets>
	STDMETHOD (GetWidgetItems)(IAnsiString *strTabs) PURE;
	//
	STDMETHOD (DeleteWidgetTab)(const char *szTabId) PURE;
	//
	STDMETHOD (DeleteWidgetItem)(const char *szItemUrl) PURE;
};

//��������Ϣ�ӿڣ�ITrayMsg��
interface __declspec(uuid("016FDF5A-4DA2-44F8-86EA-C845DDD5F955")) ITrayMsg :public IUnknown
{
	//��ʼ��ϵͳ����
	STDMETHOD (InitTrayMsg)(HINSTANCE hInstance, HICON hDefaultIcon, const char *szTip) PURE;
	//���µ�½״̬
	STDMETHOD (RefreshPresence)(const char *szPresence, const char *szMemo) PURE;
	//����һ�����̶���ͼ��
	STDMETHOD (AddAnimateIcon)(HICON hAnimIcon) PURE;
	//��ʼ���̶���
	STDMETHOD (StartAnimate)(const char *szTip) PURE;
	//ֹͣ���̶���
	STDMETHOD (StopAnimate)() PURE;
	//��ʾ����ͼ��
	STDMETHOD (ShowTrayIcon)() PURE;
	//��������ͼ��
	STDMETHOD (HideTrayIcon)() PURE;
	//��ʾ������Ϣ����
	STDMETHOD (ShowTipInfo)(const TCHAR *szImage, const TCHAR *szTipText, 
		                    const TCHAR *szCaption, const TCHAR *szUrl, BOOL bUserClosed) PURE;
	STDMETHOD (ShowTipPanel)(const char *szCaption, const char *szContent) PURE;
};

//IConfigureUI
interface __declspec(uuid("2BEEF816-A4E7-4CB0-BB6F-BC910623511C")) IConfigureUI :public IUnknown
{
	//���������ô���
	STDMETHOD (Navegate2Frame)(const LPRECT lprc, const char *szItem) PURE; 
	//��ʾ�û���Ϣ
	STDMETHOD (ViewUserInfo)(const char *szUserName) PURE;
};

//����������IMainFrame��
interface __declspec(uuid("FC18AF99-4A8D-4C73-B8FB-3A40239CE8DC")) IMainFrame :public IUnknown
{
	//��������ʾ����ǰ��
	STDMETHOD (BringToFront)() PURE; 
	//��ʾ������
	STDMETHOD (ShowMainFrame)() PURE;
	//��ʼ�� ֻ��������ʾ
	STDMETHOD (InitMainFrame)() PURE;
	//��ʾ��ϵ��
	STDMETHOD (ShowContacts)() PURE;
	//�����û�����ǩ��
	//@szUserName -- �û���
	//@szUTF8Label -- �û�����ǩ�� �գԣƣ��ַ�
	STDMETHOD (UpdateUserLabel)(const char *szUserName,  const char *szUTF8Label) PURE;
	//�����û�״̬
	STDMETHOD (UpdateUserPresence)(const char *szUserName, const char *szPresence, BOOL bSort) PURE;
	//��ʾ�͸��������ϵ��
	STDMETHOD (ShowRecentlyUser)(const char *szUserName, const char *szDispName) PURE;
	//��ȡ������HANDLE
	STDMETHOD_(HWND, GetSafeWnd)() PURE;
	//����HotKey
	STDMETHOD (UpdateHotKey)(int nType) PURE;
};
 

//��ϵ��ѡ�����
interface __declspec(uuid("B3586B3F-77D8-45F0-AED6-1583AA6F4F9E")) IContactPanel :public IUnknown
{
	//��ʾ��ϵ��ѡ�����
	//@hParent -- ������
	//@szCaption -- ���ڱ���
	//@lprc -- ����λ�ü���С
	//@bModal --�Ƿ�ģ̬��ʾ
	//@UserList -- ��ѡ���û�
	STDMETHOD (ShowPanel)(HWND hParent, const TCHAR *szCaption, LPRECT lprc, BOOL bModal, IUserList *pInitList) PURE; 
	//��ȡѡ����û��б�
	STDMETHOD (GetSelContact)(IUserList *pList) PURE;
};

//���ŷ���
interface __declspec(uuid("2071FEF9-3EDF-48DF-9137-FB06C0A56E72")) ISMSFrame :public IUnknown
{
	//��ʾ���ŷ��ʹ���
	STDMETHOD (ShowSMSFrame)(LPRECT lprc, IUserList *pList) PURE; 
	//szPeerNumbers �Էֺ�;����
	STDMETHOD (SendSMS)(const char *szPeerNumbers, const char *szContent, const char *szSign) PURE;
	//
	STDMETHOD (ShowSmsView)(const char *szUserName, const char *szType) PURE;
	//
	STDMETHOD (ShowFaxFrame)(LPRECT lprc, IUserList *pList) PURE;
	//��������Э��
	
	STDMETHOD (ParserSMSProtocol)(const char *szContent, IAnsiString *strSender, IAnsiString *strReceiver,
		IAnsiString *strTime, IAnsiString *strGuid, IAnsiString *strSign, IAnsiString *strText) PURE;
};

//�㲥��Ϣ
interface __declspec(uuid("F82ECE9D-FCE6-4221-86D9-FC09394928A0")) IBCFrame :public IUnknown
{
	//nTyle = 1 �������ֹ㲥 2 ���Ͷ��� 3 Ⱥ���ļ�
	STDMETHOD (ShowBCFrame)(const TCHAR *szCaption, LPRECT lprc, int nStyle, IUserList *pUsers) PURE;
};

//ҵ����Ϣ����
interface __declspec(uuid("7C030435-5A43-4D83-B989-CF756AED502D")) IBMCFrame :public IUnknown
{
	STDMETHOD (ShowBMCFrame)(LPRECT lprc) PURE;
};

//MINI ���Ͽ�
interface __declspec(uuid("18FBBD8D-B8C6-455B-BAA1-1E6FFFD4DF5D")) IMiniCard :public IUnknown
{
	//nAlign = 1 left
	//       = 2 right
	//       = 3 top
	//       = 4 bottom
	STDMETHOD (ShowMiniCard)(const char *szUserName, int x, int y, int nAlign) PURE;
}; 

//�������
interface __declspec(uuid("8A1A5AF6-39BA-40D9-9903-E687EFF0627B")) IEmotionFrame :public IUnknown
{
	//��ʾ�����ļ���ʾ����
	STDMETHOD (ShowEmotionFrame)(ICoreEvent *pOwner, HWND hOwner, int x, int y) PURE;
	//��ȡϵͳ�����ļ�����
	STDMETHOD (GetSysEmotion)(const char *szTag, IAnsiString *strFileName) PURE;
	//��ȡĬ�ϵı����ļ�����
	//@szStyle -- ��������
	STDMETHOD (GetDefaultEmotion)(const char *szStyle, IAnsiString *strFileName) PURE;
	//��ȡ�Զ��ļ������ļ�����
	//@szTag -- ����Tag
	//@strFileName -- ���صı����ļ����� 
	STDMETHOD (GetCustomEmotion)(const char *szTag, IAnsiString *strFileName) PURE;
};

//������ϵ��
interface __declspec(uuid("1D1D90B1-B2D8-4602-96BE-E0C3562F6A31")) IFreContacts :public IUnknown
{
	STDMETHOD (AddFreContactDept)(const char *szId, const char *szDeptName, const char *szParentId, 
		const char *szDispSeq) PURE;
	//
	STDMETHOD (AddFreContactUser)(const char *szId, const char *szUserName, const char *szRealName,
		const char *szRemark, const char *szDeptId) PURE;
	//
	STDMETHOD (DeleteFreContactDept)(const char *szId, const char *szUserName) PURE;
	//
	STDMETHOD (DeleteFreContactUser)(const char *szId, const char *szUserName) PURE;
	//
	STDMETHOD (UpdateFreContactRemark)(const char *szId, const char *szRemark) PURE;
	//�Ƿ��Ѿ������ڳ�����ϵ�˵���
	STDMETHOD_(int, IsExistsFreContact)(const char *szUserName) PURE;
	//���뱾���û���¼
	STDMETHOD (ImportContactFromLocal)(const char *szFileName) PURE;
	
};

interface __declspec(uuid("0EE7F772-645A-4A49-944D-882C63DC201A")) IMsgMgrUI :public IUnknown
{
	STDMETHOD (ShowMsgMgrFrame)(const char *szType, const char *szInitUserName, LPRECT lprc) PURE;
};


interface __declspec(uuid("4427100C-7640-4A0C-9D82-F2FB602C6239")) IOATip :public IUnknown
{
	STDMETHOD (ShowOATipPanel)(const TCHAR *szSender, const TCHAR *szFrom, const TCHAR *szTime,
		                     const TCHAR *szCatalog, const TCHAR *szId, const TCHAR *szTip, const TCHAR *szBody) PURE;

};

interface __declspec(uuid("AD504AFB-FA23-40B8-A3E4-C3038CB79848")) IGPlus :public IUnknown
{
	STDMETHOD (ShowGPlusToolBar)() PURE;
};

#endif