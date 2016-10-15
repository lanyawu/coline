#ifndef ___CLIENTCOREINTERFACE_H____
#define ___CLIENTCOREINTERFACE_H____

#include <ComDef.h>
#include <string> 
// GUID of our COM server
//CORE GUID,初始创建
_declspec(selectany) GUID CLSID_CORE_FRAMEWORK = { 0x8d53ab6, 0xf686, 0x445f, 
                                                 { 0xb7, 0x53, 0x2f, 0x86, 0xe8, 0x48, 0xd4, 0x1f } };

//配置文件插件GUID
_declspec(selectany) GUID CLSID_CORE_FRAMEWORKCFG = { 0xc8ffaf80, 0x9c89, 0x47b7, 
                                                    { 0xa4, 0x3, 0xc4, 0xb7, 0xba, 0xec, 0xf6, 0x29 } };
interface ICoreEvent;
interface IProtocolParser;

#define MAX_USER_NAME_SIZE         64 
#define NODE_DATA_CELL_PHONE_SIZE  16

//树结构显法数据
typedef struct
{
	int id;          //id 号
	int pid;         //父节点ID号
	int nDisplaySeq; //显示顺序
	int nStamp;      //最后更新时间
	int bOpened; //0--FALSE  1--TRUE
	char szUserName[MAX_USER_NAME_SIZE];    //用户名
	char szCell[NODE_DATA_CELL_PHONE_SIZE]; //用户分机号
	char *szDisplayName;                    //显示名称
}ORG_TREE_NODE_DATA, *LPORG_TREE_NODE_DATA;

//托盘图标数据
typedef struct 
{
	int ExtraData;     //扩展数据长度
	TCHAR *szImageFile; //图片文件名称
	TCHAR *szTip;       //提示
	TCHAR *szUrl;       //链接URL
	TCHAR *szCaption;   //标题
}TRAY_ICON_TIP_INFO, *LPTRAY_ICON_TIP_INFO;

//字符串接口
interface __declspec(uuid("D92DB011-23D9-4D2E-ADE7-843E3D4B360B")) IAnsiString: public IUnknown
{
	//@设置字符串
	STDMETHOD (SetString)(const char *strInput) PURE;
	//@追加字符串
	STDMETHOD (AppendString)(const char *strAppend) PURE;
	//@获取字符串，szOutput 内存空间已经分配nSize初始化值为
	STDMETHOD (GetString)(char *szOutput, int *nSize) PURE;
	//@现在字符串个数
	STDMETHOD_(int,GetSize)() PURE;
};

//XML节点对象
interface __declspec(uuid("FD2DA0B1-6F8F-4231-A0A4-CB7D05B4290B")) ITinyXmlNode: public IUnknown
{
};

//XML 文档对像
interface __declspec(uuid("1D60AC06-084D-400D-9CCC-DB7F0EBBB06F")) ITinyXmlDocument: public ITinyXmlNode
{
};

//字体类型接口
interface __declspec(uuid("2BEE78E9-5470-4DE6-AB26-1090594415B6")) IFontStyle: public IUnknown
{
	//@设置字体名称
	STDMETHOD (SetName)(const char *szFontName) PURE;
	//@设置字体大小
	STDMETHOD (SetSize)(const int nFontSize) PURE;
	//@设置字体颜色
	STDMETHOD (SetColor)(const int nColor) PURE;
	//@设置字体是否加粗
	STDMETHOD (SetBold)(const BOOL bBold) PURE;
	//@设置字体是否斜体
	STDMETHOD (SetItalic)(const BOOL bItalic) PURE;
	//@设置字体是否加下划线
	STDMETHOD (SetUnderline)(const BOOL bUnderline) PURE;
	//@
	STDMETHOD (SetStrikeout)(const BOOL bStrikeout) PURE;
	//@获取字体名称
	STDMETHOD (GetName)(IAnsiString *strName) PURE;
	//@获取字体大小
	STDMETHOD_(int, GetSize)() PURE;
	//@获取字体颜色
	STDMETHOD_(int, GetColor)() PURE;
	//@获取字体是否加粗
	STDMETHOD_ (BOOL, GetBold)() PURE;
	//@获取字体是否斜体
	STDMETHOD_ (BOOL, GetItalic)() PURE;
	//@获取字体是否加下划线
	STDMETHOD_ (BOOL, GetUnderline)() PURE;
	//
	STDMETHOD_ (BOOL, GetStrikeout)() PURE;
};
//某个用户的消息简要
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

//待处理消息简要列表
interface __declspec(uuid("A9418014-1787-4ADC-886C-15431FDFEE04")) IUserPendMessageTipList :public IUnknown
{
	STDMETHOD_ (int, GetPendMsgTipCount)() PURE;
	STDMETHOD (GetFrontMessage)(IUserPendMessageTip *pTip) PURE;
	//
	STDMETHOD (AddPendMsgTip)(IUserPendMessageTip *pTip, BOOL bCopy) PURE;
};

//用户信息列表
interface __declspec(uuid("89C554E9-6102-4CDF-9A8B-F9E156FDEA34")) IUserList :public IUnknown
{
	//@增加一条用户信息，
	//@bCopy - TRUE 需要重新分配内存并拷贝值
	STDMETHOD (AddUserInfo)(LPORG_TREE_NODE_DATA pData, BOOL bCopy) PURE;
	//@增加部门信息
	STDMETHOD (AddDeptInfo)(LPORG_TREE_NODE_DATA pData, BOOL bCopy) PURE;
	//@取出一条用户信息 后进先出
	STDMETHOD (PopBackUserInfo)(LPORG_TREE_NODE_DATA *pData) PURE;
	//@取出一条部门信息 
	STDMETHOD (PopBackDeptInfo)(LPORG_TREE_NODE_DATA *pData) PURE;
	//@取出一条用户信息 先进先出
	STDMETHOD (PopFrontUserInfo)(LPORG_TREE_NODE_DATA *pData) PURE;
	//@取出一条部门信息
	STDMETHOD (PopFrontDeptInfo)(LPORG_TREE_NODE_DATA *pData) PURE;
	//@获取用户记录条数
	STDMETHOD_ (DWORD, GetUserCount)() PURE;
	//@获取部门条数
	STDMETHOD_ (DWORD, GetDeptCount)() PURE;
	//@判别用户是否存在
	STDMETHOD_ (BOOL, UserIsExists)(const char *szUserName) PURE;
	//@跟据用户名获取一条用户信息
	STDMETHOD (PopsUserByName)(const char *szUserName, LPORG_TREE_NODE_DATA *pData) PURE;
	//@复制用户列表 
	STDMETHOD  (CopyTo)(IUserList *pDst, BOOL bCopy) PURE;
	//@设置所属部门ID
	STDMETHOD  (SetDeptId)(const char *szPid) PURE;
	//@获取所属部门ID
	STDMETHOD  (GetDeptId)(IAnsiString *strPid) PURE;
};

//核心框架接口（ICoreFrameWork）
interface __declspec(uuid("730FBAD5-70D5-4898-84DF-5AEBFB6C35F4")) ICoreFrameWork :public IUnknown
{
	//@设置代理服务器信息
	//@szType - 代理类型  @szAddress-代理服务器地址  @uPort-代理服务器端口
	//@szUserName - 代理服务器用户名 @szUserPwd - 代理服务器密码
	STDMETHOD (SetAgent)(const char *szType, const char *szAddress, USHORT uPort, 
		                     const char *szUserName, const char *szUserPwd) PURE;
	//send raw protocol
	//@pData -- 发送数据 @lSize --发送的数据长度  @lStyle - 发送数据类型，暂时以0代替
	STDMETHOD (SendRawMessage)(const BYTE *pData, const LONG lSize, const LONG lStyle) PURE;
	//获取登陆用户名
	STDMETHOD (GetUserName)(IAnsiString *szUserName) PURE;
	//获取登陆用户真实姓名 返回的为UTF８字符串
	STDMETHOD (GetUserNickName)(IAnsiString *szUTF8Name) PURE;
	//获取用户登陆密码
	STDMETHOD (GetUserPassword)(IAnsiString *szUserPwd) PURE;
	//获取用户登陆域名
	STDMETHOD (GetUserDomain)(IAnsiString *szDomain) PURE;
	//获取用户登陆的所在组别
	STDMETHOD (GetUserInGroup)(IAnsiString *szGroupId, IAnsiString *szUTF8GrpName) PURE;
	//从服务器获取离线消息
	STDMETHOD (GetOfflineMsg)() PURE;
	//是否在线,即已经连接到服务器并显示主窗口
	STDMETHOD_ (BOOL, GetIsOnline)() PURE;
	//初始化所有注册插件
	STDMETHOD (InitPlugins)(HINSTANCE hInstace) PURE;
	//清除所有加载插件
	STDMETHOD (ClearPlugins)() PURE;
	//初始化安全连接
	STDMETHOD (InitSafeSocket)() PURE;
	//下线
	STDMETHOD (Offline)() PURE;
	//注销
	STDMETHOD (Logout)() PURE;
	//建立安全连接
	STDMETHOD (EstablishSafeSocket)(const char *szSvrHost, USHORT uPort) PURE;
	//认证用户
	STDMETHOD (AuthUser)(const char *szUserName, const char *szUserPwd, const char *szPresence, const char *szPresenceMemo) PURE;
	//内核事件处理
	STDMETHOD (DoCoreEvent)(HWND hWnd, const char *szWndName, const char *szType, const char *szName, WPARAM wParam, 
		                     LPARAM lParam, HRESULT *hResult) PURE;
	//广播消息
	STDMETHOD (BroadcastMessage)(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData) PURE;
	//修改当前登陆状态 @szPresence - 状态类型 online offline away busy
	//@szMemo - 状态说明
	STDMETHOD (ChangePresence)(const char *szPresence, const char *szMemo) PURE;
	
	//获取用户登陆状态
	STDMETHOD (GetPresence)(const char *szUserName, IAnsiString *strPresence, IAnsiString *strPresenceMemo) PURE;
	//用户状态改变通知
	//@bOrder -- 是否为订制时产生的状态通知
	STDMETHOD (DoPresenceChanged)(const char *szUserName, const char *szPresence, const char *szMemo, BOOL bOrder) PURE;
	//获取待处理消息用户表
	//初始化配置文件
	STDMETHOD (InitConfigure)(const char *szCfgFileName) PURE;
	//获取服务器参数
	//@bRealTime - 是否需要从服务器更新
	STDMETHOD (GetSvrParams)(const char *szParamName, IAnsiString *szParamValue, BOOL bRealTime) PURE;
	//加入一个订制协议
	//@szProtoName -- 协议名称， xml 节点名称 
	//@szProtoType -- 协议类型   xml type 属性 为 NULL所，表时订制所有类型
	STDMETHOD (AddOrderProtocol)(IProtocolParser *pOrder, const char *szProtoName, const char *szProtoType) PURE;
	//删除协议订制
	STDMETHOD (DelOrderProtocol)(IProtocolParser *pOrder, const char *szProtoName, const char *szProtoType) PURE;
	//订制事件
	//@szWndName  -- 订制哪个窗体事件
	//@szCtrlName -- 订制的控件  为NULL时，订制窗体的所有控件事件
	//@szEventType -- 订制的控件事件类型 ，为NULL时订制控件的所有事件
	STDMETHOD (AddOrderEvent)(ICoreEvent *pEvent, const char *szWndName, const char *szCtrlName, 
		                      const char *szEventType) PURE;
	//删除事件订制
	//
	STDMETHOD (DeleteOrderEvent)(ICoreEvent *pEvent, const char *szWndName, const char *szCtrlName, 
		                      const char *szEventType) PURE;
	//获取最前的待处理消息
	//@szUserName -- 
	//@szType -- 消息类型 p2p  grp
	//@strProtocol -- 消息内容
	//@bPop -- 是否删除消息
	STDMETHOD (GetFrontPendingMsg)(const char *szUserName, const char *szType, IAnsiString *strProtocol, BOOL bPop) PURE;
	//获取排在最后的待处理消息
	//@strFromName -- 
	STDMETHOD (GetLastPendingMsg)(IAnsiString *strFromName, IAnsiString *strType) PURE;
	//获取待处理消息简要信息
	STDMETHOD (GetUserPendMsgTipList)(IUserPendMessageTipList *pList) PURE;
	//加入一个托盘消息图标
	STDMETHOD (AddTrayMsgTypeIcon)(const char *szMsgType, HICON hIcon) PURE;
	//开始托盘消息闪动
	STDMETHOD (StartTrayIcon)(const char *szMsgType, const char *szTip, HICON hIcon) PURE;
	//显示托盘消息弹窗 -- 支持线程同步
	STDMETHOD (ShowTrayTipInfo)(const TCHAR *szImageFile, const TCHAR *szTip, const TCHAR *szUrl,
		                        const TCHAR *szCaption, ICoreEvent *pEvent) PURE; 
	//Pick 待处理消息
	STDMETHOD (PickPendingMessage)() PURE; 
	//权限相关 是否有权限
	STDMETHOD (CanAllowAction)(int nAction) PURE;
};

//核心事件通知接口（ICoreEvent）
interface __declspec(uuid("596F8BF3-52CC-4BA8-8447-2D0B61D1C0DC")) ICoreEvent :public IUnknown
{
	//内核事件
	//@hWnd -- 事件产生的窗口HANDLE
	//@szType -- 事件类型
	//@szName -- 事件控件名称
	//@wParam -- 参数
	//@lParam -- 参数
	//hResult -- 事件返回数据  0 表示不需要再进行后续处理
	STDMETHOD (DoCoreEvent)(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
		                     LPARAM lParam, HRESULT *hResult) PURE;
	//设置内核接口指针
	STDMETHOD (SetCoreFrameWork)(ICoreFrameWork *pCore) PURE;
	//获取插件的界面配置 XML数据
	STDMETHOD (GetSkinXmlString)(IAnsiString *szXmlString) PURE;
	//内核错误事件
	STDMETHOD (CoreFrameWorkError)(int nErrorNo, const char *szErrorMsg) PURE;
	//广播消息
	//@szFromWndName -- 来自窗体名称
	//@hWnd -- 来自窗口HANDLE
	//@szType --广播消息类型
	//@szContent -- 广播消息内容
	//@pData -- 广播消息数据
	STDMETHOD (DoBroadcastMessage)(const char *szFromWndName, HWND hWnd, const char *szType,
		                           const char *szContent, void *pData) PURE;
	//窗口消息事件
	//需要通过IUIManager 接口订制后才会发送
	STDMETHOD (DoWindowMessage)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes) PURE;
};

//协议解析接口（IProtocolParser）
interface __declspec(uuid("561CCE58-CF30-49C5-AE20-BCA2B2B24B02")) IProtocolParser :public IUnknown
{
	//收到协议数据
	STDMETHOD (DoRecvProtocol)(const BYTE *pData, const LONG lSize) PURE;
	//用户状态改变 只有订制 sys - presence 协议后才会收到此事件
	STDMETHOD (DoPresenceChange)(const char *szUserName, const char *szNewPresence,
		const char *szMemo, BOOL bOrder) PURE;
};


//待处理消息接口(IPendingMsg)
interface __declspec(uuid("551A974C-8149-4C3A-ACBE-D2E431AAEC47")) IPendingMsg: public IUnknown
{
	//加入一个待处理消息
	STDMETHOD (AddPendingMsg)(const char *szMsgXml) PURE;
};

//实时用户信息 接口 (IInstantUserInfo)
interface __declspec(uuid("AF392DB7-F7A2-4A63-8324-D29147332C0A")) IInstantUserInfo :public IUnknown
{
	//SET用户信息数据  
	STDMETHOD (SetUserInfo)(const char *szParam, const char *szValue) PURE;
	//设置用户状态
	STDMETHOD (SetUserStatus)(const char *szStatus) PURE;
	//GET
	STDMETHOD (GetUserInfo)(const char *szParam, IAnsiString *szValue) PURE;
	STDMETHOD (GetUserStatus)(IAnsiString *szStatus) PURE;
};

//界面管理接口（IUIManager）
interface __declspec(uuid("278BE7FB-E3D9-4465-A6EA-825B88E9334C")) IUIManager :public IUnknown
{
	//加入一个插件皮肤，需要在IniSkinXmlFile后才能调用此接口
	STDMETHOD (AddPluginSkin)(const char *szXmlString) PURE;
	//初始化皮肤定义
	STDMETHOD (InitSkinXmlFile)(const char *szXmlFile) PURE;
	//初始化皮肤
	STDMETHOD (CreateSkinByXmlStream)(const char *szXml, const int nXmlSize, const char *szSkinPath) PURE;
	//创建UI窗体
	//
	STDMETHOD (CreateUIWindow)(HWND hParent, const char *szWindowName, const PRECT lprc, DWORD dwStyle, DWORD dwExStyle, 
		                     const TCHAR *szCaption, HWND *hWnd) PURE;
	//显示一个模态窗体
	STDMETHOD (ShowModalWindow)(HWND hParent, const char *szWindowName, const TCHAR *szCaption, 
		           const int X, const int Y, const int nWidth, const int nHeight, int *nModalResult) PURE;
	//清除定制窗体消息
	STDMETHOD (ClearOrderAllMessage)() PURE;
	//定制窗体消息 
	STDMETHOD (OrderWindowMessage)(const char *szWndName, HWND hWnd, UINT uMsg, ICoreEvent *pCore) PURE;
	//删除窗体消息定制
	STDMETHOD (DeleteWindowMessage)(const char *szWndName, UINT uMsg, ICoreEvent *pCore) PURE;
	//获取窗体HANDLE 
	STDMETHOD (GetWindowHWNDByName)(const char *szWndName, HWND *hWnd) PURE;
	//发送窗体消息
	STDMETHOD (SendMessageToWindow)(const char *szWndName, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *hr) PURE;
	//进行窗体主循环
	STDMETHOD (UIManagerRun)() PURE;
	//设置窗体底色
	STDMETHOD (BlendSkinStyle)(COLORREF cr) PURE;
	//加入窗体底纹
	STDMETHOD (AlphaBackImage)(const char *szFileName) PURE;
	//设置控件内容
	STDMETHOD (GetControlText)(HWND hWnd, const TCHAR *szCtrlName, TCHAR *szText, int *nSize) PURE;
	//获取控件内容
	STDMETHOD (SetControlText)(HWND hWnd, const TCHAR *szCtrlName, const TCHAR *szText) PURE;
};

//登陆接口（ICoreLogin）
interface __declspec(uuid("80AB8198-5DC5-464B-9282-500161B76A6F")) ICoreLogin :public IUnknown
{
	//position from config
	//显示登陆窗口 
	//@bAutoLogin - 是否自动开始登陆
	STDMETHOD (ShowLogonWindow)(const char *szInitUserName, const char *szInitUserPwd, BOOL bAutoLogin) PURE;
	//开始登陆服务器
	STDMETHOD (LogonSvr)(const char *szUserName, const char *szUserPwd, const char *szDomain, 
		                 const char *szPresence, const char *szPresenceMemo) PURE;
	//取消登陆
	STDMETHOD (CancelLogon)() PURE;
};

//联系人接口（IContacts）
interface __declspec(uuid("08C1DB13-FA4D-476F-8625-3218855B9DEE")) IContacts :public IUnknown
{
	//加载联系人
	STDMETHOD (LoadContacts)() PURE;
	//获取联系人缓存文件名称
	STDMETHOD (GetCacheContactsFileName)(IAnsiString *strFileName) PURE;
	//获取用户真实姓名
	//@szUserName -- 用户名 szDomain 为空时  用户名@分割域名
	//@szDomain --- 登陆域名
	STDMETHOD (GetRealNameById)(const char *szUserName, const char *szDomain, IAnsiString *szName) PURE;
	//获取用户实时状态信息
	STDMETHOD (GetContactUserInfo)(const char *szUserName, IInstantUserInfo *pInfo) PURE;
	//设置用户实时状态信息
	//@szParam
	//@szValue -- 值
	STDMETHOD (SetContactUserInfo)(const char *szUserName, const char *szParam, const char *szValue) PURE;
	//获取用户状态信息
	STDMETHOD (GetContactUserValue)(const char *szUserName, const char *szParam, IAnsiString *szValue) PURE;
	//xml <i u="user@doamin"/><i u="user2@doamin"/>....
	STDMETHOD (AddOrderUserList)(const char *szXml) PURE;
	//订制内存中用户状态，主要是用户离线后再上线
	STDMETHOD (OrderAllStatusFromSvr)() PURE;
	//获取个性头像 是否从服务器上更新
	STDMETHOD (GetContactHead)(const char *szUserName, IAnsiString *strFileName, BOOL bRefresh) PURE;
	//上传头像
	STDMETHOD (UploadHead)(const char *szFileName) PURE;
	//删除用户状态订制信息 <i u="user@doamin"/><i u="user2@doamin"/>
	STDMETHOD (DeleteOrderUserList)(const char *szXml) PURE;
	//dept list ,隔开
	STDMETHOD (GetDeptListByUserName)(const char *szUserName, const char *szDomain, IAnsiString *strDeptList) PURE;
	//获取子部门列表
	STDMETHOD (GetChildListByDeptId)(const char *szDeptId, IUserList *pUserList, int nType) PURE;
	// bSubDeptId, 是否包含子部门 nType 类型，是否为外部联系人
	STDMETHOD (GetUserListByDeptId)(const char *szDeptId, IUserList *pUserList, BOOL bSubDeptId, int nType) PURE;
	//获取部门全路径
	STDMETHOD (GetUserDeptPath)(const char *szUserName, const char *szDomain, IAnsiString *strDeptPath) PURE;
	//绘制联系人至UI
	STDMETHOD (DrawContactToUI)(HWND hWnd, const TCHAR *szTreeName, const char *szUserName, void *pParentNode,
		       BOOL bOrder, BOOL bInitPresence, int nType) PURE;
	//展开节点至UI
	STDMETHOD (ExpandTreeNodeToUI)(HWND hWnd, const TCHAR *szTreeName, void *pParentNode,
		const int nPid) PURE;
	//获取手机号码
	STDMETHOD (GetPhoneByName)(const char *szUserName, IAnsiString *strPhone) PURE;
	//获取传真号码
	STDMETHOD (GetFaxByName)(const char *szUserName, IAnsiString *strFax) PURE;
	//跟据传真号获取用户名
	STDMETHOD (GetUserNameByFax)(const char *szFax, IAnsiString *strUserName, IAnsiString *strRealName) PURE;
	//获取分机号
	STDMETHOD (GetCellPhoneByName)(const char *szUserName, IAnsiString *strCellPhone) PURE;
	//获取部门路径
	STDMETHOD (GetDeptPathNameByUserName)(const char *szUserName, IAnsiString *strPathName) PURE;
	//获取邮箱
	STDMETHOD (GetMailByUserName)(const char *szUserName, IAnsiString *strMail) PURE;
	//获取用户某项数据
	STDMETHOD (GetUserValueByParam)(const char *szUserName, const char *szParam, IAnsiString *strValue) PURE;
	//跟据电话号码获取用户名及真实姓名
	STDMETHOD (GetUserNameByNameOrPhone)(const char *szInput, IAnsiString *strUserName, IAnsiString *strRealName) PURE;
	//查询某人的电话号码和真实姓名，猜测用户名
	STDMETHOD (GetPhoneByRealName)(const char *szRealName2, const char *szRealName3, IAnsiString *szRealName, IAnsiString *strPhone) PURE;
	//显示搜索结果框
	STDMETHOD (ShowHelpEditWindow)(ICoreEvent *pOwner, const char *szText, int x, int y, int w, int h) PURE;
	//隐藏搜索结果框
	STDMETHOD (HideHelpEditWindow)() PURE;
	//显示搜索框
	STDMETHOD (ShowSearchFrame)(int x, int y, int w, int h) PURE;
	//bActive 是否活动状态 
	//bEnded 是否输入完毕
	STDMETHOD (EditHelpSearchActive)(HWND hWndFrom, const char *szText, BOOL bActived, BOOL bEnded) PURE;
	//编辑框的虚拟键
	STDMETHOD (EditVirtualKeyUp)(WORD wKey) PURE;
	//加入一个扩展用户组
	STDMETHOD (AddExtractDept)(const char *szId, const char *szDeptName, const char *szDispSeq, 
		                       const char *szParentId, int nType) PURE;
	//加入一个扩展用户
	STDMETHOD (AddExtractUser)(const char *szId, const char *szUserName, const char *szRealName, 
		const char *szDeptId, const char *szMobile, const char *szTel, const char *szEmail,
		const char *szFax, int nType) PURE;
	//删除扩展用户组别
	STDMETHOD (DeleteExtractDept)(const char *szId, int nType) PURE;
	//删除扩展用户
	STDMETHOD (DeleteExtractUser)(const char *szId, int nType) PURE;
	//是否存在扩展用户
	STDMETHOD_(int, IsExistsExtraUsers)(const char *szUserName, int nType) PURE;
	//装载常用联系人菜单
	STDMETHOD (LoadMenuFromExtractDept)(HWND hWnd, const TCHAR *szParentMenu, const char *szParentId, int nType) PURE;
	//是否为认证联系人
	STDMETHOD_(BOOL, IsAuthContact)(const char *szUserName) PURE; 
	//获取本人权限列表
	STDMETHOD (GetRoleList)(const char *szUserName, IAnsiString *RoleList) PURE;
};

//消息列表
interface __declspec(uuid("B8B2E959-C643-4BCA-914D-87017FA49040")) IMessageList :public IUnknown
{
	//加入一条消息
	STDMETHOD (AddMsg)(const int nMsgId, const char *szType, const char *szFromName,
		               const char *szToName, const char *szTime, const char *szMsg) PURE;
	//加入一条裸消息
	STDMETHOD (AddRawMsg)(const int nMsgId, const char *szRawMsg) PURE;
	//获取消息条数
	STDMETHOD_(DWORD, GetCount)() PURE;
	//清除所有消息
	STDMETHOD (Clear)() PURE;
	//获取一条消息
	STDMETHOD (GetMsg)(const int nIdx, int *nMsgId, IAnsiString *szType, IAnsiString *strFromName,
		               IAnsiString *strToName, IAnsiString *strTime, IAnsiString *strMsg) PURE;
	//获取一条裸消息
	STDMETHOD (GetRawMsg)(const int nIdx, int *nMsgId, IAnsiString *strRawMsg) PURE;
};

//消息管理接口（IMsgMgr）
interface __declspec(uuid("35F3DD63-2D92-4CC0-8169-C00E2AD00F62")) IMsgMgr :public IUnknown
{
	//init 
	STDMETHOD (InitMsgMgr)(const char *szFileName, const char *szUserName) PURE;
	//保存聊天消息
	STDMETHOD (SaveMsg)(const char *szType, const char *szFromName, const char *szToName,
	                 const char *szTime, const char *szMsg, const char *szBody) PURE;
	//get message 
	STDMETHOD (GetMsg)(const char *szType, const char *szFromName, const int nPage, 
	                const int nPageCount, IMessageList *pList) PURE;
	//获取消息
	STDMETHOD (GetMsgById)(const char *szType, const int nMsgId, IMessageList *pList) PURE;
	//获取Raw消息
	STDMETHOD (GetRawMsg)(const char *szType, const char *szFromName, const int nPage, 
	                const int nPageCount, IMessageList *pList) PURE;
	//获取消息条数
	STDMETHOD_ (int, GetMsgCount)(const char *szType, const char *szFromName) PURE;
	//nMsg != 0 delete one message
	//nMsg == 0 delete all message from == szFromName and type == szType
	STDMETHOD (ClearMsg)(const int nMsgId, const char *szType, const char *szFromName) PURE;
	//获取查询消息条件
	STDMETHOD_ (int, GetSearchMsgCount)(const char *szType, const char *szKey, const char *szFromName) PURE;
	//查询历史消息
	STDMETHOD (SearchMsg)(const char *szKey, const char *szType, const char *szFromName,
	                   const int nPage, const int nPageCount, IMessageList *pList) PURE;
	//查询历史消息
	STDMETHOD (SearchRawMsg)(const char *szKey, const char *szType, const char *szFromName,
	                   const int nPage, const int nPageCount, IMessageList *pList) PURE;
	//保存讨论组信息
	STDMETHOD (SaveGroupInfo)(const char *szGrpId, const char *szGrpDspName, const char *szCreator) PURE;
	//获取保存的讨论组列表
	STDMETHOD (GetGroups)(IUserList *pList) PURE;
	//获取短信发送者列表
	STDMETHOD (GetSmsUserList)(IUserList *pList) PURE;
};

//聊天接口（IChatFrame）
interface __declspec(uuid("8E09D17D-D1D2-47F1-BAC4-D4522CFF4A7F")) IChatFrame :public IUnknown
{
	//显示聊天窗口
	STDMETHOD_ (HWND, ShowChatFrame)(HWND hWndFrom, const char *szUserName, const char *szUTF8DspName) PURE;
	//发送文件请求，当szFileName --为NULL，将弹出文件选择窗口
	STDMETHOD (SendFileToPeer)(const char *szUserName, const char *szFileName) PURE;
	//发送远程协助请求
	STDMETHOD (SendRmcRequest)(const char *szUserName) PURE;
	//发送视频请求
	STDMETHOD (SendVideoRequest)(const char *szUserName) PURE;
	//发送音频请求
	STDMETHOD (SendAudioRequest)(const char *szUserName) PURE;
	//显示聊天消息
	STDMETHOD (ShowChatMessage)(HWND hOwner, const char *szMsg) PURE;
	//显示窗口提示消息
	STDMETHOD (ShowChatTipMsg)(HWND hOwner, const TCHAR *szMsg) PURE;
	//视频连接成功
	STDMETHOD (VideoConnected)(LPARAM lParam, WPARAM wParam) PURE;
	//根据窗口Handle 获取用户名
	STDMETHOD (GetUserNameByHWND)(HWND hOwner, IAnsiString *strUserName) PURE;
	//strMsgType ==== "tip"-- 提示
	STDMETHOD (ParserP2PProtocol)(const char *szContent, const int nContentSize, IAnsiString *strDispName,
	                          IAnsiString *strDspTime, IAnsiString *strDspText, IFontStyle *fs, IAnsiString *strMsgType,
							  BOOL *bSelf) PURE;
};

//讨论组接口（IGroupFrame）
interface __declspec(uuid("69EFE8BC-9A6E-4BA6-A157-CBDE881A1D2A")) IGroupFrame :public IUnknown
{
	//显示讨论组窗口
	STDMETHOD (ShowGroupFrame)(const char *szGrpId, const char *szUTF8DspName) PURE;
	//绘制讨论组成员
	STDMETHOD (DrawGroupToUI)(HWND hWnd, const char *szGrpId) PURE;
	//显示讨论组消息
	STDMETHOD (ShowGroupMessage)(const char *szGrpId, const char *szMsg) PURE;
	//发送文件至群
	STDMETHOD (SendFileToGroup)(const char *szGrpId, const char *szFileName) PURE;
	//发送短信至群
	STDMETHOD (SendSMSToGroup)(const char *szGrpId, const char *szSMSText) PURE;
	//发送邮件至讨论组
	STDMETHOD (SendMailToGroup)(const char *szGrpId) PURE;
	//显示讨论组提示消息
	STDMETHOD (ShowGroupTipMsg)(HWND hOwner, TCHAR *szMsg) PURE;
	//根据窗口HANDLE 获取讨论组ID
	STDMETHOD (GetGroupIdByHWND)(HWND hOwner, IAnsiString *strGroupId) PURE;
	//根据讨论组ID获取名称 
	STDMETHOD (GetGroupNameById)(const char *szGrpId, IAnsiString *strGrpName) PURE;
	//解析讨论组协议
	STDMETHOD (ParserGroupProtocol)(const char *szContent, const int nContentSize, IAnsiString *strDispName,  IAnsiString *strUserList,
	                          IAnsiString *strDspTime, IAnsiString *strDspText, IFontStyle *fs, IAnsiString *strMsgType,
							  BOOL *bSelf) PURE; 
	//szGrpId 存在则修改
	STDMETHOD (CreateGroup)(const char *szGrpId, const char *szGrpName, IUserList *pUser) PURE;
	//修改分组名称
	STDMETHOD (UpdateGroupName)(const char *szGrpId, const char *szNewGrpName) PURE;
	//退出分组
	STDMETHOD (ExitGroupById)(const char *szGrpId) PURE;
	//删除分组
	STDMETHOD (DeleteGroupById)(const char *szGrpId) PURE; 
};

//配置接口（IConfigure）
interface __declspec(uuid("C991C7E1-0F8D-467C-986B-48F9BD7D2CA8")) IConfigure :public IUnknown
{
	//公共配置 or 私有配置
	STDMETHOD (InitCfgFileName)(const char *szFileName, const char *szPersonName, BOOL bCommon) PURE;
	//获取配置参数
	STDMETHOD (GetParamValue)(BOOL bCommon, const char *szSection, const char *szParamName, 
		        IAnsiString *szValue) PURE;
	//设置配置参数
	STDMETHOD (SetParamValue)(BOOL bCommon, const char *szSection, 
		             const char *szParamName, const char *szValue) PURE;
	//设置服务器参数
	STDMETHOD (SetSvrParam)(const char *szParam, const char *szValue) PURE;
	//设置用户登陆信息
	//
	STDMETHOD (SetUserLoginInfo)(const char *szUserName, const char *szUserPwd, const char *szUserDomain,
		               BOOL bSavePwd, const char *szStatus, const char *szLoginSvrHost, const int nLoginSvrPort) PURE;

	//设置登陆用户真实姓名
	STDMETHOD (SetUserRealName)(const char *szUserName, const char *szUserDomain, const char *szUTF8RealName) PURE;
	//获取用户登陆登陆信息
	STDMETHOD (GetUserInfoByRealName)(const char *szName, IAnsiString *szUserName, IAnsiString *szDomain,
		                          IAnsiString *szPwd, IAnsiString *szPic, IAnsiString *strRealName, IAnsiString *szStatus, 
								  IAnsiString *szSvrHost, IAnsiString *szPort) PURE;
	//获取用户登陆名称
	STDMETHOD (GetUserNameByRealName)(const char *szName, IAnsiString *szUserName) PURE;
	//szUserInfos XML
	//<?xml version="1.0" encoding="gb2312"?>
	//<users>
	//  <item username="username" password="password" savepwd="y" realname="realname" userpic="12" loginsvrhost="www.nowhelp.cn"
	//            loginsvrport="9902" logindomain="gocom"/>
	//  <item ...>
	//</users>
	STDMETHOD (GetUserLoginUserList)(IAnsiString *szUserInfos) PURE;
	//关键字检查
	STDMETHOD (CheckKeyWord)(const char *UTF8Chars) PURE;
	//nId != 0 ==> modify
	STDMETHOD_ (int, AddReplyMessage)(int nId, int nType, const char *szReply) PURE;
	//获取自动回复消息
	STDMETHOD (GetReplyMessage)(int nType, IMessageList *strReplys) PURE;
	//删除自动回复数据
	STDMETHOD (DelReplyMessage)(int nId, int nType) PURE;
	//获取自动回复字符
	STDMETHOD (GetAutoReplyMessage)(IAnsiString *strMsg) PURE;
	//获取最近联系人列表
	STDMETHOD (GetRecentlyList)(IUserList *List) PURE;
	//加入一个最近联系人数据
	STDMETHOD (AddRecentlyList)(const char *szUserName, const char *szDispName) PURE;
	//从最近联系人中删除用户
	//szUserName 为空时删除所有用户
	STDMETHOD (DelUserFromRecently)(const char *szUserName) PURE;
	//获取用户字体
	STDMETHOD (GetChatFontStyle)(IFontStyle *Style) PURE;
	//获取皮肤定义
	STDMETHOD (GetSkinXml)(const char *szDefaultName, IAnsiString *szXmlString) PURE;
	//获取存取路径
	STDMETHOD (GetPath)(int nPathId, IAnsiString *szPath) PURE;
	//获取服务器地址
	STDMETHOD (GetServerAddr)(int nServerId, IAnsiString *szSvrAddr) PURE;
	//播放声音
	STDMETHOD (PlayMsgSound)(const char *szType, const char *szUserName, BOOL bLoop) PURE;
	//关注状态改变用户
	STDMETHOD (AddContactOnlineTip)(const char *szUserName) PURE;
	//删除关注状态改变用户
	STDMETHOD (DelContactOnlineTip)(const char *szUserName) PURE;
	///获取状态关注用户列表
	STDMETHOD (GetContactOnlineTipUsers)(IUserList *List) PURE;
	//测试用户上线是否提示
	STDMETHOD_(BOOL, IsContactOnlineTip)(const char *szUserName) PURE;
	//
	STDMETHOD (AddWidgetTab)(const char *szTabId, const char *szTabDspName) PURE;
	//
	STDMETHOD (AddWidgetItem)(const char *szTabId, const char *szItemName, const char *szItemDspName,
		                      const char *szItemUrl, const int nImageId, const char *szItemTip) PURE;
	//返回XML字符串 <widgets><tab id="016FD35A-4DA2-44F8-86EA-C845DDD5F953" name="标签1"/><tab.../></widgets>
	STDMETHOD (GetWidgetTabs)(IAnsiString *strTabs) PURE;
	//返回XML字符串 <widgets><item tabid="016FD35A-4DA2-44F8-86EA-C845DDD5F953" id="id1" name="GoCom"
	//  caption="统一通信平台" url="gocom://" imageid="25" tip="统一通信平台"/><item... /></widgets>
	STDMETHOD (GetWidgetItems)(IAnsiString *strTabs) PURE;
	//
	STDMETHOD (DeleteWidgetTab)(const char *szTabId) PURE;
	//
	STDMETHOD (DeleteWidgetItem)(const char *szItemUrl) PURE;
};

//任务栏消息接口（ITrayMsg）
interface __declspec(uuid("016FDF5A-4DA2-44F8-86EA-C845DDD5F955")) ITrayMsg :public IUnknown
{
	//初始化系统托盘
	STDMETHOD (InitTrayMsg)(HINSTANCE hInstance, HICON hDefaultIcon, const char *szTip) PURE;
	//更新登陆状态
	STDMETHOD (RefreshPresence)(const char *szPresence, const char *szMemo) PURE;
	//加入一个托盘动画图标
	STDMETHOD (AddAnimateIcon)(HICON hAnimIcon) PURE;
	//开始托盘动画
	STDMETHOD (StartAnimate)(const char *szTip) PURE;
	//停止托盘动画
	STDMETHOD (StopAnimate)() PURE;
	//显示托盘图标
	STDMETHOD (ShowTrayIcon)() PURE;
	//隐藏托盘图标
	STDMETHOD (HideTrayIcon)() PURE;
	//显示托盘信息窗口
	STDMETHOD (ShowTipInfo)(const TCHAR *szImage, const TCHAR *szTipText, 
		                    const TCHAR *szCaption, const TCHAR *szUrl, BOOL bUserClosed) PURE;
	STDMETHOD (ShowTipPanel)(const char *szCaption, const char *szContent) PURE;
};

//IConfigureUI
interface __declspec(uuid("2BEEF816-A4E7-4CB0-BB6F-BC910623511C")) IConfigureUI :public IUnknown
{
	//导航至配置窗口
	STDMETHOD (Navegate2Frame)(const LPRECT lprc, const char *szItem) PURE; 
	//显示用户信息
	STDMETHOD (ViewUserInfo)(const char *szUserName) PURE;
};

//主界面插件（IMainFrame）
interface __declspec(uuid("FC18AF99-4A8D-4C73-B8FB-3A40239CE8DC")) IMainFrame :public IUnknown
{
	//弹出窗口示到最前面
	STDMETHOD (BringToFront)() PURE; 
	//显示主窗口
	STDMETHOD (ShowMainFrame)() PURE;
	//初始化 只创建不显示
	STDMETHOD (InitMainFrame)() PURE;
	//显示联系人
	STDMETHOD (ShowContacts)() PURE;
	//更新用户个性签名
	//@szUserName -- 用户名
	//@szUTF8Label -- 用户个性签名 ＵＴＦ８字符
	STDMETHOD (UpdateUserLabel)(const char *szUserName,  const char *szUTF8Label) PURE;
	//更新用户状态
	STDMETHOD (UpdateUserPresence)(const char *szUserName, const char *szPresence, BOOL bSort) PURE;
	//显示和更新最近联系人
	STDMETHOD (ShowRecentlyUser)(const char *szUserName, const char *szDispName) PURE;
	//获取主窗口HANDLE
	STDMETHOD_(HWND, GetSafeWnd)() PURE;
	//更新HotKey
	STDMETHOD (UpdateHotKey)(int nType) PURE;
};
 

//联系人选择面板
interface __declspec(uuid("B3586B3F-77D8-45F0-AED6-1583AA6F4F9E")) IContactPanel :public IUnknown
{
	//显示联系人选择面板
	//@hParent -- 父窗体
	//@szCaption -- 窗口标题
	//@lprc -- 窗口位置及大小
	//@bModal --是否模态显示
	//@UserList -- 已选择用户
	STDMETHOD (ShowPanel)(HWND hParent, const TCHAR *szCaption, LPRECT lprc, BOOL bModal, IUserList *pInitList) PURE; 
	//获取选择的用户列表
	STDMETHOD (GetSelContact)(IUserList *pList) PURE;
};

//短信发送
interface __declspec(uuid("2071FEF9-3EDF-48DF-9137-FB06C0A56E72")) ISMSFrame :public IUnknown
{
	//显示短信发送窗口
	STDMETHOD (ShowSMSFrame)(LPRECT lprc, IUserList *pList) PURE; 
	//szPeerNumbers 以分号;隔开
	STDMETHOD (SendSMS)(const char *szPeerNumbers, const char *szContent, const char *szSign) PURE;
	//
	STDMETHOD (ShowSmsView)(const char *szUserName, const char *szType) PURE;
	//
	STDMETHOD (ShowFaxFrame)(LPRECT lprc, IUserList *pList) PURE;
	//解析短信协议
	
	STDMETHOD (ParserSMSProtocol)(const char *szContent, IAnsiString *strSender, IAnsiString *strReceiver,
		IAnsiString *strTime, IAnsiString *strGuid, IAnsiString *strSign, IAnsiString *strText) PURE;
};

//广播消息
interface __declspec(uuid("F82ECE9D-FCE6-4221-86D9-FC09394928A0")) IBCFrame :public IUnknown
{
	//nTyle = 1 发送文字广播 2 发送短信 3 群发文件
	STDMETHOD (ShowBCFrame)(const TCHAR *szCaption, LPRECT lprc, int nStyle, IUserList *pUsers) PURE;
};

//业务消息中心
interface __declspec(uuid("7C030435-5A43-4D83-B989-CF756AED502D")) IBMCFrame :public IUnknown
{
	STDMETHOD (ShowBMCFrame)(LPRECT lprc) PURE;
};

//MINI 资料卡
interface __declspec(uuid("18FBBD8D-B8C6-455B-BAA1-1E6FFFD4DF5D")) IMiniCard :public IUnknown
{
	//nAlign = 1 left
	//       = 2 right
	//       = 3 top
	//       = 4 bottom
	STDMETHOD (ShowMiniCard)(const char *szUserName, int x, int y, int nAlign) PURE;
}; 

//表情管理
interface __declspec(uuid("8A1A5AF6-39BA-40D9-9903-E687EFF0627B")) IEmotionFrame :public IUnknown
{
	//显示表情文件显示窗口
	STDMETHOD (ShowEmotionFrame)(ICoreEvent *pOwner, HWND hOwner, int x, int y) PURE;
	//获取系统表情文件名称
	STDMETHOD (GetSysEmotion)(const char *szTag, IAnsiString *strFileName) PURE;
	//获取默认的表情文件名称
	//@szStyle -- 表情类型
	STDMETHOD (GetDefaultEmotion)(const char *szStyle, IAnsiString *strFileName) PURE;
	//获取自定文件表情文件名称
	//@szTag -- 表情Tag
	//@strFileName -- 返回的表情文件名称 
	STDMETHOD (GetCustomEmotion)(const char *szTag, IAnsiString *strFileName) PURE;
};

//常用联系人
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
	//是否已经存在于常用联系人当中
	STDMETHOD_(int, IsExistsFreContact)(const char *szUserName) PURE;
	//导入本地用户记录
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