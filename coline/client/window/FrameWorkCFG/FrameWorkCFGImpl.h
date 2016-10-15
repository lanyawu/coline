#ifndef __FRAMEWORKCFGIMPL_H___
#define __FRAMEWORKCFGIMPL_H___
#include <map>
#include <vector>
#include <ComBase.h>
#include <Commonlib/stringutils.h>
#include <Commonlib/GuardLock.h>
#include <Core/CoreInterface.h>
#include <Commonlib/SqliteDBOP.h>
#include "../IMCommonLib/InterfaceFontStyle.h"

class CFrameWorkCFGImpl: public CComBase<>, 
	                     public InterfaceImpl<IConfigure> 
{
public:
	CFrameWorkCFGImpl(void);
	~CFrameWorkCFGImpl(void);
public:
	//IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppv);
	//公共配置 or 私有配置
	//IConfigure
	STDMETHOD (InitCfgFileName)(const char *szFileName, const char *szPersonName, BOOL bCommon);
	STDMETHOD (GetParamValue)(BOOL bCommon, const char *szSection, const char *szParamName, 
		        IAnsiString *szValue);
	//关键字检查
	STDMETHOD (CheckKeyWord)(const char *UTF8Chars);

	STDMETHOD (SetParamValue)(BOOL bCommon, const char *szSection,
		          const char *szParamName, const char *szValue);
	STDMETHOD (SetSvrParam)(const char *szParam, const char *szValue);
	STDMETHOD (SetUserLoginInfo)(const char *szUserName, const char *szUserPwd, const char *szUserDomain,
		               BOOL bSavePwd, const char *szStatus, const char *szLoginSvrHost, const int nLoginSvrPort);
	STDMETHOD (GetUserLoginUserList)(IAnsiString *szUserInfos);
	STDMETHOD (SetUserRealName)(const char *szUserName, const char *szUserDomain, const char *szUTF8RealName);
	STDMETHOD (GetUserInfoByRealName)(const char *szName, IAnsiString *szUserName, IAnsiString *szDomain,
		                          IAnsiString *szPwd, IAnsiString *szPic, IAnsiString *strRealName, IAnsiString *strStatus, 
								  IAnsiString *szSvrHost, IAnsiString *szPort);
	STDMETHOD (GetUserNameByRealName)(const char *szName, IAnsiString *szUserName);
	//
	STDMETHOD (GetChatFontStyle)(IFontStyle *Style);
	//
	STDMETHOD (GetSkinXml)(const char *szDefaultName, IAnsiString *szXmlString);
	//
	STDMETHOD (SetUserName)(const char *szUserName);
	//获取存取路径
	STDMETHOD (GetPath)(int nPathId, IAnsiString *szPath);
	//获取服务器地址
	STDMETHOD (GetServerAddr)(int nServerId, IAnsiString *szSvrAddr);
	//nId != 0 ==> modify
	STDMETHOD_ (int, AddReplyMessage)(int nId, int nType, const char *szReply);
	//
	STDMETHOD (GetReplyMessage)(int nType, IMessageList *strReplys);
	//
	STDMETHOD (DelReplyMessage)(int nId, int nType);
	//获取自动回复字符
	STDMETHOD (GetAutoReplyMessage)(IAnsiString *strMsg);
	//
	STDMETHOD (GetRecentlyList)(IUserList *List);
	//
	STDMETHOD (AddRecentlyList)(const char *szUserName, const char *szDispName);
	//从最近联系人中删除用户
	//szUserName 为空时删除所有用户
	STDMETHOD (DelUserFromRecently)(const char *szUserName);
	//
	//播放声音
	STDMETHOD (PlayMsgSound)(const char *szType, const char *szUserName, BOOL bLoop);
	//
	STDMETHOD (AddContactOnlineTip)(const char *szUserName);
	//
	STDMETHOD (DelContactOnlineTip)(const char *szUserName);
	///
	STDMETHOD (GetContactOnlineTipUsers)(IUserList *List);
	//测试用户上线是否提示
	STDMETHOD_(BOOL, IsContactOnlineTip)(const char *szUserName);
	//
	//
	STDMETHOD (AddWidgetTab)(const char *szTabId, const char *szTabDspName);
	//
	STDMETHOD (AddWidgetItem)(const char *szTabId, const char *szItemName, const char *szItemDspName,
		                      const char *szItemUrl, const int nImageId, const char *szItemTip);
	//返回XML字符串 <widgets><tab id="016FD35A-4DA2-44F8-86EA-C845DDD5F953" name="标签1"/><tab.../></widgets>
	STDMETHOD (GetWidgetTabs)(IAnsiString *strTabs);
	//返回XML字符串 <widgets><tab id="016FD35A-4DA2-44F8-86EA-C845DDD5F953><item id="id1" name="GoCom"
	//  caption="统一通信平台" url="gocom://" imageid="25" tip="统一通信平台"/><item... /></tab></widgets>
	STDMETHOD (GetWidgetItems)(IAnsiString *strTabs);
    //
	STDMETHOD (DeleteWidgetTab)(const char *szTabId);
	//
	STDMETHOD (DeleteWidgetItem)(const char *szItemUrl);
private:
	BOOL CheckDBOPValid(CSqliteDBOP *pDB, const char *szKey, BOOL bCommon);
	BOOL CheckParamIsExists(BOOL bCommon, const char *szSection, const char *szParamName);
	BOOL CheckUserIsExists(const char *szUserName, const char *szUserDomain);
	HRESULT DoMenuCommand(const char *szName, WPARAM wParam, LPARAM lParam);
	void InitChatFont();
	void InitDefaultPersonCfg(const char *szPersonName);
	void InitContactOnlineTipList(); 
	void InitDefaultReply();
	void CheckAutoStart();
private:
	CSqliteDBOP *m_pCommon;
	CSqliteDBOP *m_pPerson;
	CInterfaceFontStyle m_ChatFont;
	//path
	std::string m_strPersonPath;
	std::string m_strImagePath;
	std::string m_strUserHeadPath;
	std::string m_strRcvFilePath;
	std::string m_strCachePath;
	std::string m_strCustomEmotionPath;

	std::string m_strUserName; 
	CGuardLock m_DbLock;
	std::map<CAnsiString_, std::string> m_SvrParamList;
	std::map<CAnsiString_, std::string> m_PersonParams;
	std::map<CAnsiString_, std::string> m_CommonParams;
	std::vector<std::string> m_KeyWords;

	//
	CGuardLock m_OnlineTipLock;
	std::map<CAnsiString_, int> m_ContactOnlineTipList;

	//
	DWORD m_dwSvrTime;
	DWORD m_dwLocalTime;
};

#endif
