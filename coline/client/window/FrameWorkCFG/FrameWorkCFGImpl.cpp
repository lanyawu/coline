#include <string>
#include <time.h>
#include <Commonlib/stringutils.h>
#include <Commonlib/systemutils.h>
#include <Commonlib/DebugLog.h>
#include "../IMCommonLib/InterfaceAnsiString.h"
#include "FrameWorkCFGImpl.h"
#include <Core/common.h>
#include <fstream>

#define WIDGET_SECTION_NAME  "widgettab"

#define DEFAULT_RECENTLY_COUNT 10

#define FRAMEWORKCFG_DB_KEY  "c1F2W@"

#define CREATE_CFG_TABLE_SQL "create table params(id INTEGER PRIMARY KEY, \
                              Section VARCHAR(64) COLLATE NOCASE, ParamName VARCHAR(128) COLLATE NOCASE,\
                              ParamValue VARCHAR(512)  COLLATE NOCASE);\
                              create index section_idx on params(Section);\
                              create index paramname_idx on params(ParamName);"

#define CREATE_USER_TABLE_SQL "create table userlist(id INTEGER PRIMARY KEY, username VARCHAR(128)  COLLATE NOCASE,\
                               userpwd VARCHAR(128)  COLLATE NOCASE, userrealname VARCHAR(128) COLLATE NOCASE,\
                               userpic VARCHAR(128)  COLLATE NOCASE, savepwd VARCHAR(1), stamp INTEGER,\
                               status VARCHAR(32), loginsvrhost VARCHAR(128)  COLLATE NOCASE, loginsvrport INTEGER,\
                               userdomain VARCHAR(64) COLLATE NOCASE);"
//回复表
#define CREATE_REPLY_TABLE_SQL "create table reply(id INTEGER PRIMARY KEY, type INTEGER, data VARCHAR(256) COLLATE NOCASE);"

#define CREATE_RECENTLY_TABLE_SQL "create table recently(id INTEGER PRIMAY KEY, username VARCHAR(256) COLLATE NOCASE, realname VARCHAR(256),\
                                   stamp INTEGER);"

#define CREATE_CONTACT_TIP_TABLE_SQL "create table contacttip(username VARCHAR(256) COLLATE NOCASE PRIMARY KEY);"

//快捷方式
#define CREATE_WIDGETS_TABLE_SQL "create table widgets(id INTEGER PRIMARY KEY, tagsname VARCHAR(128) COLLATE NOCASE,\
                                  widgetname VARCHAR(128) COLLATE NOCASE, widgetcaption VARCHAR(256) COLLATE NOCASE,\
                                  widgeturl VARCHAR(256), imageid INTEGER, widgettip VARCHAR(256));"
//初始化目录定义
#define CUSTOM_PICTURE_PATH       "CustomImages\\"   //自定义图片目录
#define LOCAL_USER_HEAD_PATH      "UserHead\\"       //个人头像目录
#define LOCAL_RECV_DEFAULT_PATH   "RecvFile\\"       //文件默认接收目录
#define LOCAL_CACHE_DEFAULT_PATH  "Cache\\"         //缓存默认目录
#define LOCAL_CUSTOM_EMOTION_PATH "Emotion\\"       //个人自定义表情目录  

#pragma warning(disable:4996)

CFrameWorkCFGImpl::CFrameWorkCFGImpl(void):
                   m_pCommon(NULL),
				   m_pPerson(NULL)
{
	//
}


CFrameWorkCFGImpl::~CFrameWorkCFGImpl(void)
{
	if (m_pCommon)
		delete m_pCommon;
	m_pCommon = NULL;
	if (m_pPerson)
		delete m_pPerson;
	m_pPerson = NULL;
}

//IUnknown
STDMETHODIMP CFrameWorkCFGImpl::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IConfigure)))
	{
		*ppv = (IConfigure *) this;
		_AddRef();
		return S_OK;
	} 
	return E_NOINTERFACE;
}

//
STDMETHODIMP CFrameWorkCFGImpl::InitCfgFileName(const char *szFileName, const char *szPersonName, BOOL bCommon)
{
	BOOL bSucc = FALSE;
	if (bCommon)
	{
		if (m_pCommon)
			delete m_pCommon;
		std::string strKey = "common";
		strKey += FRAMEWORKCFG_DB_KEY;
		m_pCommon = new CSqliteDBOP(szFileName, strKey.c_str());
		bSucc = CheckDBOPValid(m_pCommon, NULL, bCommon);
	} else
	{
		if (m_pPerson)
			delete m_pPerson;
		std::string strKey;
		if (szPersonName) 
			strKey = szPersonName;
		strKey += FRAMEWORKCFG_DB_KEY;
		char szPersonPath[MAX_PATH] = {0};
		CSystemUtils::ExtractFilePath(szFileName, szPersonPath, MAX_PATH - 1);
		m_strPersonPath = szPersonPath;
		m_pPerson = new CSqliteDBOP(szFileName, strKey.c_str());
		bSucc = CheckDBOPValid(m_pPerson, szPersonName, bCommon);
		if (bSucc)
		{
			InitChatFont();
			InitContactOnlineTipList(); //初始化上线提示人员列表
		}
	}
	if (bSucc)
		return S_OK;
	else
		return E_FAIL;
}
 

//window启动项
#define REGISTER_START_RUN_KEY "Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define REGISTER_START_RUN_NAME "CoLine"

void CFrameWorkCFGImpl::CheckAutoStart()
{
	BOOL bWrite = TRUE;
	CInterfaceAnsiString strTmp;
	if (SUCCEEDED(GetParamValue(FALSE, "normal", "checkautostart", &strTmp)))
	{
		bWrite = FALSE;
	}
	if (bWrite)
	{
		char szFileName[MAX_PATH] = {0};
		if (CSystemUtils::GetApplicationFileName(szFileName, MAX_PATH))
		{
			CSystemUtils::WriteRegisterKey(HKEY_CURRENT_USER, REGISTER_START_RUN_KEY, REGISTER_START_RUN_NAME, szFileName);
		}
		SetParamValue(FALSE, "normal", "checkautostart", "true");
	}
}

//获取自动回复字符
STDMETHODIMP CFrameWorkCFGImpl::GetAutoReplyMessage(IAnsiString *strMsg)
{
	CInterfaceAnsiString strState;
	if (SUCCEEDED(GetParamValue(FALSE, "replysetting", "start", &strState)))
	{
		if (::stricmp(strState.GetData(), "true") == 0)
		{
			return GetParamValue(FALSE, "replysetting", "autoreply", strMsg);
		}
	}
	return E_FAIL;
}

void CFrameWorkCFGImpl::InitDefaultPersonCfg(const char *szPersonName)
{
	//default sound file 
	static const char SOUND_ITEM_NAME[8][32] = {"system", "friend", "group", "online", "video", "audio", "shake", "sms"};
	static const char SOUND_ITEM_FILENAME_LIST[8][32] = {"system.wav", "friend.wav", "group.wav", "online.wav",
		                        "video.wav", "audio.wav", "shake.wav", "sms.wav"};
	char szAppFileName[MAX_PATH] = {0};
	char szAppPath[MAX_PATH] = {0};
	CSystemUtils::GetApplicationFileName(szAppFileName, MAX_PATH - 1);
	CSystemUtils::ExtractFilePath(szAppFileName, szAppPath, MAX_PATH - 1);
	strcat(szAppPath, "sound\\"); 
	std::string strFileName;
	for (int i = 0; i < 8; i ++)
	{
		strFileName = szAppPath;
		strFileName += SOUND_ITEM_FILENAME_LIST[i];
		SetParamValue(FALSE, "sound", SOUND_ITEM_NAME[i], strFileName.c_str());
	}

	//初始化默认接收文件存储目录
	memset(szAppPath, 0, MAX_PATH);
	CSystemUtils::GetMyDocumentPath(szAppPath, MAX_PATH);
	strcat(szAppPath, "\\CoLine\\");
	strcat(szAppPath, szPersonName);
	strcat(szAppPath, "\\recvfile\\"); 
	CSystemUtils::ForceDirectories(szAppPath);
	SetParamValue(FALSE, "filetransfer", "defaultpath", szAppPath);
	//
	SetParamValue(FALSE, "person", "timetoaway", "5");
	//
}

BOOL CFrameWorkCFGImpl::CheckUserIsExists(const char *szUserName, const char *szUserDomain)
{
	BOOL bSucc = FALSE;
	std::string strSql = "select id from userlist where username='";
	strSql += szUserName;
	strSql += "' and userdomain='";
	strSql += szUserDomain;
	strSql += "'";
	char **szResult = NULL;
	int nRow = 0, nCol = 0;
	if (m_pCommon)
	{
		CGuardLock::COwnerLock guard(m_DbLock);
		bSucc = m_pCommon->Open(strSql.c_str(), &szResult, nRow, nCol);
		if (bSucc && (nRow == 0))
			bSucc = FALSE;
	}
	return bSucc;
}

STDMETHODIMP CFrameWorkCFGImpl::SetUserRealName(const char *szUserName, const char *szUserDomain, const char *szUTF8RealName)
{	
	if (CheckUserIsExists(szUserName, szUserDomain))
	{
		std::string strSql = "update userlist set userrealname='";
		strSql += szUTF8RealName;
		strSql += "' where username='";
		strSql += szUserName;
		strSql += "' and userdomain='";
		strSql += szUserDomain;
		strSql += "'";
		if (m_pCommon)
		{
			CGuardLock::COwnerLock guard(m_DbLock);
			if (m_pCommon->Execute(strSql.c_str()))
				return S_OK;
		}
	}
	return E_FAIL;
}

STDMETHODIMP CFrameWorkCFGImpl::SetUserLoginInfo(const char *szUserName, const char *szUserPwd, const char *szUserDomain,
		               BOOL bSavePwd, const char *szStatus, const char *szLoginSvrHost, const int nLoginSvrPort)
{

	std::string strSql;
	char szSvrPort[16] = {0};
	::itoa(nLoginSvrPort, szSvrPort, 10);
	char szStamp[16] = {0};
	::itoa((int)::time(NULL), szStamp, 10);
	if (CheckUserIsExists(szUserName, szUserDomain))
	{
		if (szLoginSvrHost == NULL)
		{
			strSql = "delete from userlist  where username='";
			strSql += szUserName;
			strSql += "' and userdomain='";
			strSql += szUserDomain;
			strSql += "'";
		} else
		{
			strSql = "update userlist set userpwd='";
			if (szUserPwd)
				strSql += szUserPwd; 
			strSql += "', savepwd='";
			if (bSavePwd)
				strSql += "y',loginsvrhost='";
			else
				strSql += "n',loginsvrhost='";
			strSql += szLoginSvrHost;
			strSql += "',loginsvrport=";
		    strSql += szSvrPort;
			strSql += ", stamp=";
			strSql += szStamp;
			strSql += ", status='";
			strSql += szStatus;
			strSql += "' where username='";
			strSql += szUserName;
			strSql += "' and userdomain='";
			strSql += szUserDomain;
			strSql += "'";
		}
	} else
	{
		if (szLoginSvrHost != NULL)
		{
			strSql = "insert into userlist(username,userrealname,userdomain,userpwd,savepwd,loginsvrhost,loginsvrport,stamp, status) values('";
			strSql += szUserName;
			strSql += "','";
			strSql += szUserName;
			strSql += "','";
			strSql += szUserDomain;
			strSql += "','";
			if (szUserPwd)
				strSql += szUserPwd;
			if (bSavePwd)
				strSql += "','y','";
			else
				strSql += "','n','";
			strSql += szLoginSvrHost;
			strSql += "','";
			strSql += szSvrPort;
			strSql += "','";
			strSql += szStamp;
			strSql += "','";
			strSql += szStatus;
			strSql += "')";
		}
	}
	BOOL bSucc = FALSE;
	if (m_pCommon && (!strSql.empty()))
	{
		CGuardLock::COwnerLock guard(m_DbLock);
		if (m_pCommon->Execute(strSql.c_str()))
		{
			CheckAutoStart();
			return S_OK;
		}
	}
	return E_FAIL;
}

STDMETHODIMP CFrameWorkCFGImpl::SetUserName(const char *szUserName)
{
	if (szUserName)
	{
		m_strUserName = szUserName;
	} else
	{
		m_strUserName = "";
	}
	return S_OK;
}

STDMETHODIMP CFrameWorkCFGImpl::GetUserNameByRealName(const char *szName, IAnsiString *szUserName)
{
	if (m_pCommon)
	{
		std::string strSql = "select username  from userlist \
						  where username='";
		strSql += szName;
		strSql += "' or userrealname='";
		char szUTF8[MAX_PATH] = {0};
		CStringConversion::StringToUTF8(szName, szUTF8, MAX_PATH - 1);
		strSql += szUTF8;
		strSql += "'";
		char **szResult;
		int nRow = 0, nCol = 0;
		BOOL bSucc = FALSE;
		CGuardLock::COwnerLock gaurd(m_DbLock);
		if (m_pCommon->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			if (nRow > 0)
			{
				szUserName->SetString(szResult[1]); 
				bSucc = TRUE;
			}
		}
		CSqliteDBOP::Free_Result(szResult);
		if (bSucc)
			return S_OK;
	}
	return E_FAIL;
}

STDMETHODIMP CFrameWorkCFGImpl::GetUserInfoByRealName(const char *szName, IAnsiString *szUserName, IAnsiString *szDomain,
		                          IAnsiString *szPwd, IAnsiString *szPic, IAnsiString *strRealName, IAnsiString *strStatus,
								  IAnsiString *szSvrHost, IAnsiString *szPort)
{
	if (m_pCommon)
	{
		std::string strSql = "select username, userrealname,userpwd,userdomain,loginsvrhost,loginsvrport,userpic, status from userlist \
						  where username='";
		strSql += szName;
		strSql += "' or userrealname='";
		char szUTF8[MAX_PATH] = {0};
		CStringConversion::StringToUTF8(szName, szUTF8, MAX_PATH - 1);
		strSql += szUTF8;
		strSql += "'";
		char **szResult;
		int nRow = 0, nCol = 0;
		BOOL bSucc = FALSE;
		CGuardLock::COwnerLock guard(m_DbLock);
		if (m_pCommon->Open(strSql.c_str(), &szResult, nRow, nCol))
		{ 
			if (nRow > 0)
			{
				szUserName->SetString(szResult[nCol]);
				strRealName->SetString(szResult[nCol + 1]);
				szPwd->SetString(szResult[nCol + 2]);
				szDomain->SetString(szResult[nCol + 3]);
				szSvrHost->SetString(szResult[nCol + 4]);
				szPort->SetString(szResult[nCol + 5]);
				szPic->SetString(szResult[nCol + 6]);
				strStatus->SetString(szResult[nCol + 7]);
				bSucc = TRUE;
			}
		}
		CSqliteDBOP::Free_Result(szResult);
		if (bSucc)
			return S_OK;
	}
	return E_FAIL;
}

void CFrameWorkCFGImpl::InitChatFont()
{
	CInterfaceAnsiString strTmp;
	if (SUCCEEDED(GetParamValue(FALSE, "ChatFont", "FontName", (IAnsiString *) &strTmp)))
	{
		m_ChatFont.SetName(strTmp.GetData());
	}
	if (SUCCEEDED(GetParamValue(FALSE, "ChatFont", "FontSize", (IAnsiString *) &strTmp)))
	{
		m_ChatFont.SetSize(::atoi(strTmp.GetData()));
	}
	if (SUCCEEDED(GetParamValue(FALSE, "ChatFont", "FontColor", (IAnsiString *) &strTmp)))
	{
		m_ChatFont.SetColor(::atoi(strTmp.GetData()));
	}
	if (SUCCEEDED(GetParamValue(FALSE, "ChatFont", "FontBold", (IAnsiString *) &strTmp)))
	{
		m_ChatFont.SetBold(::stricmp(strTmp.GetData(), "true") == 0);
	}
	if (SUCCEEDED(GetParamValue(FALSE, "ChatFont", "FontItalic", (IAnsiString *) &strTmp)))
	{
		m_ChatFont.SetItalic(::stricmp(strTmp.GetData(), "true") == 0);
	}
	if (SUCCEEDED(GetParamValue(FALSE, "ChatFont", "FontUnderline", (IAnsiString *) &strTmp)))
	{
		m_ChatFont.SetUnderline(::stricmp(strTmp.GetData(), "true") == 0);
	}
}

STDMETHODIMP CFrameWorkCFGImpl::GetChatFontStyle(IFontStyle *Style)
{
	m_ChatFont.CopyTo(Style);
	return S_OK;
}

//关键字检查
STDMETHODIMP CFrameWorkCFGImpl::CheckKeyWord(const char *UTF8Chars)
{
	std::vector<std::string>::iterator it;
	for (it = m_KeyWords.begin(); it != m_KeyWords.end(); it ++)
	{
		if (strstr(UTF8Chars, it->c_str()) != NULL)
			return E_FAIL;
	}
	return S_OK;
}


STDMETHODIMP CFrameWorkCFGImpl::GetUserLoginUserList(IAnsiString *szUserInfos)
{
	std::string strXml ="<?xml version=\"1.0\" encoding=\"gb2312\"?><users>";
	char **szResult;
	int nRow = 0, nCol = 0;
	if (m_pCommon)
	{
		if (m_pCommon->Open("select username,userpwd,savepwd,userdomain,loginsvrhost,loginsvrport,\
							userpic,userrealname from userlist order by stamp desc", &szResult, nRow, nCol))
		{
			//<?xml version="1.0" encoding="gb2312"?>
	        //<users>
	        //  <item name="username" password="password" savepwd="y" realname="realname" userpic="12" loginsvrhost="www.nowhelp.cn"
	        //            loginsvrport="9902" logindomain="gocom"/>
	        //  <item ...>
	        //</users>
			static const int SELECT_ROW_COUNT = 8;
			for (int i = 1; i <= nRow; i ++)
			{
				strXml += "<item username=\"";
				if (szResult[i * SELECT_ROW_COUNT])
					strXml += szResult[i * SELECT_ROW_COUNT];
				strXml += "\" password=\"";
				if (szResult[i * SELECT_ROW_COUNT + 1])
					strXml += szResult[i * SELECT_ROW_COUNT + 1];
				strXml += "\" savepwd=\"";
				if (szResult[i * SELECT_ROW_COUNT + 2])
					strXml += szResult[i * SELECT_ROW_COUNT + 2];
				strXml += "\" realname=\"";
				if (szResult[i * SELECT_ROW_COUNT + 7])
					strXml += szResult[i * SELECT_ROW_COUNT + 7];
				strXml += "\" userpic=\"";
				if (szResult[i * SELECT_ROW_COUNT + 6])
					strXml += szResult[i * SELECT_ROW_COUNT + 6];
				strXml += "\" loginsvrhost=\"";
				if (szResult[i * SELECT_ROW_COUNT + 4])
					strXml += szResult[i * SELECT_ROW_COUNT + 4];
				strXml += "\" loginsvrport=\"";
				if (szResult[i * SELECT_ROW_COUNT + 5])
					strXml += szResult[i * SELECT_ROW_COUNT + 5];
				strXml += "\" logindomain=\"";
				if (szResult[i * SELECT_ROW_COUNT + 3])
					strXml += szResult[i * SELECT_ROW_COUNT + 3];
				strXml += "\"/>";
			}
		}
		CSqliteDBOP::Free_Result(szResult);
	} 
	strXml += "</users>";
	szUserInfos->SetString(strXml.c_str());
	return S_OK;
}


BOOL CFrameWorkCFGImpl::CheckDBOPValid(CSqliteDBOP *pDB, const char *szKey, BOOL bCommon)
{
	CGuardLock::COwnerLock guard(m_DbLock);
	BOOL bSucc = TRUE;
	if (pDB)
	{
		if (!pDB->TableIsExists("params"))
		{
			if (!pDB->Execute(CREATE_CFG_TABLE_SQL))
				bSucc = FALSE;
			else
			{ 
				if (!bCommon)
					InitDefaultPersonCfg(szKey); //初始化个人默认参数
			}
		}
		if (bCommon)
		{
			if (!pDB->TableIsExists("userlist"))
			{
				if (!pDB->Execute(CREATE_USER_TABLE_SQL))
					bSucc = FALSE;
			}
		} else
		{
			if (!pDB->TableIsExists("reply"))
			{
				if (!pDB->Execute(CREATE_REPLY_TABLE_SQL))
					bSucc = FALSE;
				else
				{
					InitDefaultReply();
				}
			}
			if (!pDB->TableIsExists("recently"))
			{
				if (!pDB->Execute(CREATE_RECENTLY_TABLE_SQL))
					bSucc = FALSE;
			}
			if (!pDB->TableIsExists("contacttip"))
			{
				if (!pDB->Execute(CREATE_CONTACT_TIP_TABLE_SQL))
					bSucc = FALSE;
			}
			if (!pDB->TableIsExists("widgets"))
			{
				if (!pDB->Execute(CREATE_WIDGETS_TABLE_SQL))
					bSucc = FALSE;
			}
			//end if (!pDB->
		}//end if (bCommon) 
	} //end if (pDB)
	return bSucc;
}

//初始化回复
void CFrameWorkCFGImpl::InitDefaultReply()
{
	AddReplyMessage(0, 1, "哦");
	AddReplyMessage(0, 1, "是吗？");
	//
	AddReplyMessage(0, 2, "现在有事不在，一会再和你联系");
	AddReplyMessage(0, 2, "工作中，请勿打扰");
	AddReplyMessage(0, 2, "吃饭去了");
	SetParamValue(FALSE, "replysetting", "autoreply", "现在有事不在，一会再和你联系");
}

void CFrameWorkCFGImpl::InitContactOnlineTipList()
{
	static char SELECT_CONTACT_TIP_SQL[] = "select username from contacttip";
	char **szResult = NULL;
	int nCol, nRow; 
	CGuardLock::COwnerLock guard(m_DbLock);
	if (m_pPerson->Open(SELECT_CONTACT_TIP_SQL, &szResult, nRow, nCol))
	{
		CGuardLock::COwnerLock guard(m_OnlineTipLock);
		for (int i = 1; i <= nRow; i ++)
		{
			m_ContactOnlineTipList.insert(std::pair<CAnsiString_, int>(szResult[i * nCol], 0));
		} 
	}
	CSqliteDBOP::Free_Result(szResult);
}

//
STDMETHODIMP CFrameWorkCFGImpl::AddContactOnlineTip(const char *szUserName)
{
	std::string strSql = "insert into contacttip(username) values('";
	strSql += szUserName;
	strSql += "');";
	CGuardLock::COwnerLock guard(m_DbLock);
	if (m_pPerson->Execute(strSql.c_str()))
	{
		CGuardLock::COwnerLock guard(m_OnlineTipLock);
		m_ContactOnlineTipList.insert(std::pair<CAnsiString_, int>(szUserName, 0));
		return S_OK;
	}
	return E_FAIL;
}

//
STDMETHODIMP CFrameWorkCFGImpl::DelContactOnlineTip(const char *szUserName)
{
	std::string strSql = "delete from contacttip where username='";
	strSql += szUserName;
	strSql += "'";
	CGuardLock::COwnerLock guard(m_DbLock);
	if (m_pPerson->Execute(strSql.c_str()))
	{
		CGuardLock::COwnerLock guard(m_OnlineTipLock);
		std::map<CAnsiString_, int>::iterator it = m_ContactOnlineTipList.find(szUserName);
		if (it != m_ContactOnlineTipList.end())
			m_ContactOnlineTipList.erase(it);
		return S_OK;
	}
	return E_FAIL;
}

//测试用户上线是否提示
STDMETHODIMP_(BOOL) CFrameWorkCFGImpl::IsContactOnlineTip(const char *szUserName)
{
	CInterfaceAnsiString strValue;
	if (SUCCEEDED(GetParamValue(FALSE, "onlinetip", "allcontactclose", &strValue)))
	{
		if (::stricmp(strValue.GetData(), "true") == 0)
			return FALSE;
	}
	if (SUCCEEDED(GetParamValue(FALSE, "onlinetip", "exceptcontact", &strValue)))
	{
		if (::stricmp(strValue.GetData(), "true") == 0)
		{
			CGuardLock::COwnerLock guard(m_OnlineTipLock);
			std::map<CAnsiString_, int>::iterator it = m_ContactOnlineTipList.find(szUserName);
			if (it == m_ContactOnlineTipList.end())
				return FALSE;
		} 
	}
	/*if (SUCCEEDED(GetParamValue(FALSE, "onlinetip", "allcontacttip", &strValue)))
	{
		if (::stricmp(strValue.GetData(), "true") == 0)
			return TRUE;
	}*/
	return TRUE;
}

///
STDMETHODIMP CFrameWorkCFGImpl::GetContactOnlineTipUsers(IUserList *List) 
{
	if (!m_ContactOnlineTipList.empty())
	{
		CGuardLock::COwnerLock guard(m_OnlineTipLock);
		std::map<CAnsiString_, int>::iterator it;
		for (it = m_ContactOnlineTipList.begin(); it != m_ContactOnlineTipList.end(); it ++)
		{
			LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
			strncpy(pData->szUserName, it->first.c_str(), 63);
			List->AddUserInfo(pData, FALSE);			 
		} 
		return S_OK; 
	}
	return E_FAIL;
}

//nId != 0 ==> modify
STDMETHODIMP_(int) CFrameWorkCFGImpl::AddReplyMessage(int nId, int nType, const char *szReply)
{
	if (m_pPerson)
	{
		int nResult = nId;
		std::string strSql;
		if (nId == 0)
		{
			strSql = "insert into reply(type,data) values(";
			char szTmp[16] = {0};
			strSql += ::itoa(nType, szTmp, 10);
			strSql += ",'";
			int nLen = ::strlen(szReply)  * 2;
			char *strSafeSql = new char[nLen + 1];
			memset(strSafeSql, 0, nLen + 1);
			strSql += CSqliteDBOP::StrToSqlStr(szReply, strSafeSql);
			delete []strSafeSql;
			strSql += "')";
		} else
		{
			strSql = "update reply set data='"; 
			int nLen = ::strlen(szReply)  * 2;
			char *strSafeSql = new char[nLen + 1];
			memset(strSafeSql, 0, nLen + 1);
			strSql += CSqliteDBOP::StrToSqlStr(szReply, strSafeSql);
			delete []strSafeSql;
			strSql += "' where id=";
			char szTmp[16] = {0};
			strSql += ::itoa(nId, szTmp, 10);
			strSql += " and type=";
			strSql += ::itoa(nType, szTmp, 10);
		} 
		CGuardLock::COwnerLock guard(m_DbLock);
		if (m_pPerson->Execute(strSql.c_str()))
		{
			if (nResult == 0)
				nResult = m_pPerson->LastInsertRowId();
			return nResult;
		}
	}
	return 0;
}

STDMETHODIMP CFrameWorkCFGImpl::DelReplyMessage(int nId, int nType)
{
	std::string strSql = "delete from reply where type=";
	char szTmp[16] = {0};
	strSql += ::itoa(nType, szTmp, 10);
	strSql += " and id=";
	strSql += ::itoa(nId, szTmp, 10);
	CGuardLock::COwnerLock guard(m_DbLock);
	if (m_pPerson && m_pPerson->Execute(strSql.c_str()))
		return S_OK;
	return E_FAIL;
}

//
STDMETHODIMP CFrameWorkCFGImpl::GetReplyMessage(int nType, IMessageList *strReplys)
{
	std::string strSql = "select id,data from reply where type=";
	char szTmp[16] = {0};
	strSql += ::itoa(nType, szTmp, 10);
	char **szResult = NULL;
	int nRow, nCol;
	HRESULT hr = E_FAIL;
	CGuardLock::COwnerLock guard(m_DbLock);
	if (m_pPerson && m_pPerson->Open(strSql.c_str(), &szResult, nRow, nCol))
	{
		for (int i = 1; i <= nRow; i ++)
		{
			if (szResult[i * nCol] && szResult[i * nCol + 1])
			{
				strReplys->AddRawMsg(::atoi(szResult[i * nCol]), szResult[i * nCol + 1]);
			}
		}
		CSqliteDBOP::Free_Result(szResult);
		hr = S_OK;
	}
	return hr;
}

BOOL CFrameWorkCFGImpl::CheckParamIsExists(BOOL bCommon, const char *szSection, const char *szParamName)
{
	BOOL bExists = FALSE;
	std::string strSql = "select id from params where section='";
	strSql += szSection;
	strSql += "' and paramname='";
	strSql += szParamName;
	strSql += "'";
	char **szResult = NULL;
	int nRow = 0, nCol = 0;
	CGuardLock::COwnerLock guard(m_DbLock);
	if (bCommon)
	{
		if (m_pCommon)
			bExists = m_pCommon->Open(strSql.c_str(), &szResult, nRow, nCol);
	} else
	{
		if (m_pPerson)
			bExists = m_pPerson->Open(strSql.c_str(), &szResult, nRow, nCol);
	}
	if (bExists)
	{
		if (nRow < 1)
			bExists = FALSE;
	}
	if (szResult)
		CSqliteDBOP::Free_Result(szResult);
	return bExists;
}

STDMETHODIMP CFrameWorkCFGImpl::SetParamValue(BOOL bCommon, const char *szSection, 
	                               const char *szParamName, const char *szValue)
{
	BOOL bNeedRefresh = TRUE;
	if (::stricmp(szSection, "ChatFont") == 0)
	{
		if (::stricmp(szParamName, "FontName") == 0)
		{
			m_ChatFont.SetName(szValue);
		} 
		if (::stricmp(szParamName, "FontSize") == 0)
		{
			m_ChatFont.SetSize(::atoi(szValue));
		} 
		if (::stricmp(szParamName, "FontColor") == 0)
		{
			m_ChatFont.SetColor(::atoi(szValue));
		} 	 
		if (::stricmp(szParamName, "FontBold") == 0)
		{
			m_ChatFont.SetBold(stricmp(szValue, "true") == 0);
		} 	 
 		if (::stricmp(szParamName, "FontItalic") == 0)
		{
			m_ChatFont.SetItalic(stricmp(szValue, "true") == 0);
		} 	
 		if (::stricmp(szParamName, "FontUnderline") == 0)
		{
			m_ChatFont.SetUnderline(stricmp(szValue, "true") == 0);
		} 	  
	} 

	std::string strTmp = szSection;
	strTmp += "_";
	strTmp += szParamName;
	std::map<CAnsiString_, std::string>::iterator it;
	m_DbLock.Lock();
	if (bCommon)
	{
		it = m_CommonParams.find(strTmp.c_str());
		if (it != m_CommonParams.end())
		{
			if (::stricmp(it->second.c_str(), szValue) == 0)
				bNeedRefresh = FALSE;
			else
				m_CommonParams[strTmp.c_str()] = szValue;
		} //end if (it != 			
	} else
	{
		it = m_PersonParams.find(strTmp.c_str());
		if (it != m_PersonParams.end())
		{
			if (::stricmp(it->second.c_str(), szValue) == 0)
				bNeedRefresh = FALSE;
			else
				m_PersonParams[strTmp.c_str()] = szValue;
		} 
	}
	m_DbLock.UnLock(); 

	if (!bNeedRefresh)
		return S_OK;
	BOOL bSucc = FALSE;
	std::string strSql;
	if (CheckParamIsExists(bCommon, szSection, szParamName))
	{
		strSql = "update params set paramvalue='";
		strSql += szValue;
		strSql += "' where section='";
		strSql += szSection;
		strSql += "' and ParamName='";
		strSql += szParamName;
		strSql += "'";		
	} else
	{
		strSql = "insert into params(section,paramname,paramvalue) values('";
		strSql += szSection;
		strSql += "','";
		strSql += szParamName;
		strSql += "','";
		strSql += szValue;
		strSql += "')";		
	}

	CGuardLock::COwnerLock guard(m_DbLock);
	if (bCommon)
	{
		if (m_pCommon)
			bSucc = m_pCommon->Execute(strSql.c_str());
	} else
	{
		if ((::stricmp(szSection, "filetransfer") == 0) 
			&& (::stricmp(szParamName, "defaultpath") == 0))
		{
			CSystemUtils::ForceDirectories(szValue);
			m_strRcvFilePath = szValue;
			char szPath[MAX_PATH] = {0};
			CSystemUtils::IncludePathDelimiter(m_strRcvFilePath.c_str(), szPath, MAX_PATH - 1);
			m_strRcvFilePath = szPath;
		}
		if (m_pPerson)
			bSucc = m_pPerson->Execute(strSql.c_str());
	}
	if (bSucc)
		return S_OK;
	else
		return E_FAIL;
}

STDMETHODIMP CFrameWorkCFGImpl::GetParamValue(BOOL bCommon, const char *szSection, const char *szParamName, 
		        IAnsiString *szValue)
{
	BOOL bSucc = FALSE;
	std::string strTmp = szSection;
	strTmp += "_";
	strTmp += szParamName;
	//
	{
		CGuardLock::COwnerLock guard(m_DbLock);
		std::map<CAnsiString_, std::string>::iterator it;
		if (bCommon)
		{
			it = m_CommonParams.find(strTmp.c_str());
			if (it != m_CommonParams.end())
			{
				szValue->SetString(it->second.c_str());
				return S_OK;
			}
		} else
		{
			it = m_PersonParams.find(strTmp.c_str());
			if (it != m_PersonParams.end())
			{
				szValue->SetString(it->second.c_str());
				return S_OK;
			}
		}
	}
	std::string strSql = "select ParamValue from params where Section='";
	strSql += szSection;
	strSql += "' and ParamName='";
	strSql += szParamName;
	strSql += "'";
	char **szResult = NULL;
	int nRow, nCol;
	CGuardLock::COwnerLock guard(m_DbLock);
	if (bCommon)
	{
		if (m_pCommon)
		{
			bSucc = m_pCommon->Open(strSql.c_str(), &szResult, nRow, nCol);
		}
	} else
	{
		if (m_pPerson)
		{
			bSucc = m_pPerson->Open(strSql.c_str(), &szResult, nRow, nCol);
		}
	}
	if (bSucc && szResult)
	{
		bSucc = FALSE;
		if (nRow > 0) 
		{
			if (szResult[1] && (::strlen(szResult[1]) > 0))
			{  
				if (bCommon)
					m_CommonParams[strTmp.c_str()] = szResult[1];
				else
					m_PersonParams[strTmp.c_str()] = szResult[1]; 
				szValue->SetString(szResult[1]);
				bSucc = TRUE;
			} //end if (szResult[1])
		} //end if (nRow > 0)
	} //end if (bSucc &&
	CSqliteDBOP::Free_Result(szResult);
	if (bSucc)
		return S_OK;
	else
		return E_FAIL;
}
//
STDMETHODIMP CFrameWorkCFGImpl::GetSkinXml(const char *szDefaultName, IAnsiString *szXmlString)
{
	std::string strFileName;
    std::string strSkinFileName;
	BOOL bSucc = FALSE;
	CInterfaceAnsiString strFilePath;
	if (SUCCEEDED(GetPath(PATH_LOCAL_SKIN, &strFilePath)))
	{
		strFileName = strFilePath.GetData();
		strSkinFileName = strFilePath.GetData();
		strSkinFileName += "default.skin";
		strFileName += szDefaultName;
	}
	if (CSystemUtils::FileIsExists(strSkinFileName.c_str()))
	{
		CSqliteDBOP skin(strSkinFileName.c_str(), NULL);
		std::string strSql = "select windowtext from skinwindow where name='";
		strSql += szDefaultName;
		strSql += "'";
		char **szResult = NULL;
		int nRow, nCol;
		if (skin.Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			if ((nRow > 0) && szResult[1] != NULL)
			{
				szXmlString->SetString(szResult[1]);
				bSucc = TRUE;
			}
			CSqliteDBOP::Free_Result(szResult);
		}
	} 

	if ((!bSucc) && !strFileName.empty() && CSystemUtils::FileIsExists(strFileName.c_str()))
	{
		TCHAR szwFileName[MAX_PATH] = {0};
		CStringConversion::StringToWideChar(strFileName.c_str(), szwFileName, MAX_PATH);
		std::ifstream ifs(szwFileName, std::ios::in | std::ios::binary);
		if (ifs.is_open() && szXmlString)
		{
			ifs.seekg(0, std::ios::end);
			DWORD dwSize = ifs.tellg();
			char *p = new char[dwSize + 1];
			ifs.seekg(0, std::ios::beg);
			ifs.read(p, dwSize);
			p[dwSize] = '\0';
			bSucc = TRUE;
			szXmlString->SetString(p);
			delete []p;
			ifs.close();
		}
	}
	if (bSucc)
		return S_OK;
	else
		return E_FAIL; 
}

//
STDMETHODIMP CFrameWorkCFGImpl::GetRecentlyList(IUserList *List)
{
	HRESULT hr = E_FAIL;
	if (m_pPerson)
	{
		int nRecentlyCount = DEFAULT_RECENTLY_COUNT;
		CInterfaceAnsiString szValue;
		if (SUCCEEDED(GetParamValue(FALSE, "normal", "recentlycount", (IAnsiString *)&szValue)))
		{
			nRecentlyCount = ::atoi(szValue.GetData());
			if (nRecentlyCount <= 0)
				nRecentlyCount = DEFAULT_RECENTLY_COUNT;
		}
		std::string strSql = "select username,realname,stamp from recently order by stamp desc limit ";
		char szTmp[16] = {0};
		strSql += ::itoa(nRecentlyCount, szTmp, 10);
		char **szResult = NULL;
		int nRow, nCol;
		CGuardLock::COwnerLock guard(m_DbLock);
		if (m_pPerson->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			for (int i = 1; i <= nRow; i ++)
			{
				if (szResult[i * nCol] && szResult[i * nCol + 1])
				{
					LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA;
					memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
					strncpy(pData->szUserName, szResult[i * nCol], 63);
					int nLen = ::strlen(szResult[i * nCol + 1]);
					pData->szDisplayName = new char[nLen + 1];
					pData->nStamp = ::atoi(szResult[i * nCol + 2]);
					memset(pData->szDisplayName, 0, nLen + 1);
					strcpy(pData->szDisplayName, szResult[i * nCol + 1]);
					List->AddUserInfo(pData, FALSE);
				} //end if (szResult[i * nCol] && szResult[i * nCol + 1])
			} //
			hr = S_OK;
		}
		CSqliteDBOP::Free_Result(szResult);
	}
	return hr;
}

//播放声音
STDMETHODIMP CFrameWorkCFGImpl::PlayMsgSound(const char *szType, const char *szUserName, BOOL bLoop)
{
	CInterfaceAnsiString strValue;
	GetParamValue(FALSE, "sound", "play", &strValue);
	if ((strValue.GetSize() <= 0) || (::stricmp(strValue.GetData(), "true") == 0))
	{
		if (SUCCEEDED(GetParamValue(FALSE, "sound", szType, &strValue)))
		{
			CSystemUtils::PlaySoundFile(strValue.GetData(), bLoop);
			return S_OK;
		}
	}
	return E_FAIL;
}

STDMETHODIMP CFrameWorkCFGImpl::SetSvrParam(const char *szParam, const char *szValue)
{
	if (szParam && szValue)
	{
		if (::stricmp(szParam, "servertime") == 0)
		{
			//
		} else if (::stricmp(szParam, "filter") == 0)
		{ 
			const char *p = szValue;
			const char *pStart = p;
			while (*p != '\0') 
			{
				if (*p == ',')
				{
					int nSize = p - pStart;
					if (nSize > 1)
					{
						char *pTmp = new char[nSize + 1];
						memmove(pTmp, pStart, nSize);
						pTmp[nSize] = '\0';
						m_KeyWords.push_back(pTmp);
						delete []pTmp;
					} 
					p ++;
					pStart = p;
				} else 
					p ++;
			}
			int nSize = p - pStart;
			if (nSize > 1)
			{
				char *pTmp = new char[nSize + 1];
				memmove(pTmp, pStart, nSize);
				pTmp[nSize] = '\0';
				m_KeyWords.push_back(pTmp);
				delete []pTmp;
			} 
		}
		if (::stricmp(szValue, "127.0.0.1") == 0)  //替换IP值
		{
			std::string strSvrIp = m_SvrParamList["serverip"];
			m_SvrParamList[szParam] = strSvrIp;
		} else
			m_SvrParamList[szParam] = szValue;
		return S_OK;
	}
	return E_FAIL;
}

//获取存取路径
STDMETHODIMP CFrameWorkCFGImpl::GetPath(int nPathId, IAnsiString *szPath)
{
	if (szPath)
	{
		switch(nPathId)
		{
		case PATH_LOCAL_CUSTOM_PICTURE:
			{
				if (m_strImagePath.empty())
				{ 
					m_strImagePath = m_strPersonPath;
					m_strImagePath += CUSTOM_PICTURE_PATH;
					CSystemUtils::ForceDirectories(m_strImagePath.c_str());  
				} //end if (m_strImagePath...
				if (!m_strImagePath.empty())
				{
					szPath->SetString(m_strImagePath.c_str());
					return S_OK;
				} 
				break;
			}
		case PATH_LOCAL_PERSON:
			{
				if (!m_strPersonPath.empty())
				{
					szPath->SetString(m_strPersonPath.c_str());
					return S_OK;
				}
				break;
			}
		case PATH_LOCAL_USER_HEAD:
			{
				if (m_strUserHeadPath.empty())
				{ 
					m_strUserHeadPath = m_strPersonPath;
					m_strUserHeadPath += LOCAL_USER_HEAD_PATH;
					CSystemUtils::ForceDirectories(m_strUserHeadPath.c_str());  
				} //end if (m_strImagePath...
				if (!m_strUserHeadPath.empty())
				{
					szPath->SetString(m_strUserHeadPath.c_str());
					return S_OK;
				} 
				break;
			}
		case PATH_LOCAL_RECV_PATH:
			{
				if (m_strRcvFilePath.empty())
				{
					CInterfaceAnsiString strPath;
					if (SUCCEEDED(GetParamValue(FALSE, "filetransfer", "defaultpath", (IAnsiString *) &strPath)))
					{
						char szPath[MAX_PATH] = {0};
						CSystemUtils::IncludePathDelimiter(strPath.GetData(), szPath, MAX_PATH - 1);
						m_strRcvFilePath = szPath;
					} else
					{
						m_strRcvFilePath = m_strPersonPath;
						m_strRcvFilePath += LOCAL_RECV_DEFAULT_PATH;
						SetParamValue(FALSE, "filetransfer", "defaultpath", m_strRcvFilePath.c_str());
					}
					if (!m_strRcvFilePath.empty())
					{
						CSystemUtils::ForceDirectories(m_strRcvFilePath.c_str());
					}
				}
				if (!m_strRcvFilePath.empty())
				{
					szPath->SetString(m_strRcvFilePath.c_str());
					return S_OK;
				}
				break;
			}
		case PATH_LOCAL_CACHE_PATH:
			{
				if (m_strCachePath.empty())
				{
					CInterfaceAnsiString strPath;
					if (SUCCEEDED(GetParamValue(FALSE, "filetransfer", "cachepath", (IAnsiString *) &strPath)))
					{
						char szPath[MAX_PATH] = {0};
						CSystemUtils::IncludePathDelimiter(strPath.GetData(), szPath, MAX_PATH - 1);
						m_strCachePath = szPath;
					} else
					{
						m_strCachePath = m_strPersonPath;
						m_strCachePath += LOCAL_CACHE_DEFAULT_PATH;
						SetParamValue(FALSE, "filetransfer", "cachepath", m_strCachePath.c_str());
					}
					if (!m_strCachePath.empty())
					{
						CSystemUtils::ForceDirectories(m_strCachePath.c_str());
					}
				}
				if (!m_strCachePath.empty())
				{
					szPath->SetString(m_strCachePath.c_str());
					return S_OK;
				}
				break;
			} //end case 
		case PATH_LOCAL_SKIN:
			{
				CInterfaceAnsiString strFilePath;
				if (FAILED(GetParamValue(FALSE, "Path", "SkinPath", (IAnsiString *)&szPath)))
				{
					char szAppName[MAX_PATH] = {0};
					char szAppPath[MAX_PATH] = {0};
					CSystemUtils::GetApplicationFileName(szAppName, MAX_PATH - 1);
					CSystemUtils::ExtractFilePath(szAppName, szAppPath, MAX_PATH - 1);
					szPath->SetString(szAppPath);
					szPath->AppendString("Skin\\");
				} 
				return S_OK;
				break;
			}
		case PATH_CUSTOM_EMOTION:
			{
				if (m_strCustomEmotionPath.empty())
				{ 
					m_strCustomEmotionPath = m_strPersonPath;
					m_strCustomEmotionPath += LOCAL_CUSTOM_EMOTION_PATH;
					CSystemUtils::ForceDirectories(m_strCustomEmotionPath.c_str());  
				} //end if (m_strCustomEmotionPath...
				if (!m_strCustomEmotionPath.empty())
				{
					szPath->SetString(m_strCustomEmotionPath.c_str());
					return S_OK;
				} 
				break;
			}
		} //end switch(..
	} //end if (szPath)
	return E_FAIL;
}

//获取服务器地址
STDMETHODIMP CFrameWorkCFGImpl::GetServerAddr(int nServerId, IAnsiString *szSvrAddr)
{
	if (szSvrAddr)
	{
		switch(nServerId)
		{
		case HTTP_SVR_URL_CUSTOM_PICTURE:
			{
				szSvrAddr->SetString(m_SvrParamList["httpip"].c_str());
				szSvrAddr->AppendString(":");
				szSvrAddr->AppendString(m_SvrParamList["httpport"].c_str());
				szSvrAddr->AppendString("/upcustompic.php");
				break;
			}
		case HTTP_SVR_URL_OFFLINE_FILE:
			{
				szSvrAddr->SetString(m_SvrParamList["httpip"].c_str());
				szSvrAddr->AppendString(":");
				szSvrAddr->AppendString(m_SvrParamList["httpport"].c_str());
				szSvrAddr->AppendString("/upofflinefile.php");
				break;
			}
		case HTTP_SVR_URL_USER_HEAD:
			{
				szSvrAddr->SetString(m_SvrParamList["httpip"].c_str());
				szSvrAddr->AppendString(":");
				szSvrAddr->AppendString(m_SvrParamList["httpport"].c_str());
				szSvrAddr->AppendString("/uppic.php");
				break;
			}
		case HTTP_SVR_URL_HTTP:
			{
				szSvrAddr->SetString(m_SvrParamList["httpip"].c_str());
				szSvrAddr->AppendString(":");
				szSvrAddr->AppendString(m_SvrParamList["httpport"].c_str());
				break;
			}
		case HTTP_SVR_URL_FAX:
			{
				szSvrAddr->SetString(m_SvrParamList["httpip"].c_str());
				szSvrAddr->AppendString(":");
				szSvrAddr->AppendString(m_SvrParamList["httpport"].c_str());
				szSvrAddr->AppendString("/Fax/fileupload.do");
				break;
			}
		case HTTP_SVR_MAIL_URL:
			{
				if (m_SvrParamList["mailurl"].length() > 0) 
				{
					szSvrAddr->SetString(m_SvrParamList["mailurl"].c_str());
				}
				break;
			}
		default:
			return E_FAIL;
		}
		return S_OK;
	}
	return E_FAIL;
}

//从最近联系人中删除用户
//szUserName 为空时删除所有用户
STDMETHODIMP CFrameWorkCFGImpl::DelUserFromRecently(const char *szUserName)
{
	if (m_pPerson)
	{
		std::string strSql = "delete from recently";
		if (szUserName)
		{
			strSql += " where username='";
		    strSql += szUserName;
		    strSql += "'";
		}
 
		CGuardLock::COwnerLock guard(m_DbLock);
		if (m_pPerson->Execute(strSql.c_str()))
		{
			return S_OK;
		} //end if (m_pPerson->Execute(..
	} //end if (m_pPerson)
	return E_FAIL;
}

//
STDMETHODIMP CFrameWorkCFGImpl::AddRecentlyList(const char *szUserName, const char *szDispName)
{
	if (m_pPerson)
	{
		std::string strSql = "select id from recently where username='";
		strSql += szUserName;
		strSql += "'";
		char **szResult = NULL;
		int nRow, nCol;
		BOOL bUpdate = FALSE;
		CGuardLock::COwnerLock guard(m_DbLock);
		if (m_pPerson->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			if (nRow > 0)
				bUpdate = TRUE;
		}
		CSqliteDBOP::Free_Result(szResult);
		time_t t = time(NULL);
		char szTmp[16] = {0};
		::itoa((int) t, szTmp, 10);
		strSql.clear();
		if (bUpdate)
		{
			strSql = "update recently set stamp=";
			strSql += szTmp;
			strSql += ",realname='";
			strSql += szDispName;
			strSql += "' where username='";
			strSql += szUserName;
			strSql += "'";
		} else
		{
			strSql = "insert into recently(username,realname,stamp) values('";
			strSql += szUserName;
			strSql += "','";
			strSql += szDispName;
			strSql += "',";
			strSql += szTmp;
			strSql += ")";
		} 
		if (m_pPerson->Execute(strSql.c_str()))
			return S_OK;
	}
	return E_FAIL;
}

//
STDMETHODIMP CFrameWorkCFGImpl::AddWidgetTab(const char *szTabId, const char *szTabDspName)
{
	return SetParamValue(FALSE, WIDGET_SECTION_NAME, szTabId, szTabDspName); 
}

//
STDMETHODIMP CFrameWorkCFGImpl::AddWidgetItem(const char *szTabId, const char *szItemName, const char *szItemDspName,
	                      const char *szItemUrl, const int nImageId, const char *szItemTip)
{
	//create table widgets(id INTEGER PRIMARY KEY, tagsname VARCHAR(128) COLLATE NOCASE,\
    //  widgetname VARCHAR(128) COLLATE NOCASE, widgetcaption VARCHAR(256) COLLATE NOCASE,\
   //   widgeturl VARCHAR(256), imageid INTEGER, widgettip VARCHAR(256);"
	if (m_pPerson)
	{
		char **szResult = NULL;
		int nRow, nCol;
		BOOL bExists = FALSE;
		std::string strSql = "select id from widgets where widgeturl='";
		int nUrlLen = ::strlen(szItemUrl) * 2;
		char *szTmpUrl = new char[nUrlLen + 1];
		memset(szTmpUrl, 0, nUrlLen + 1);
		strSql += CSqliteDBOP::StrToSqlStr(szItemUrl, szTmpUrl);
		strSql += "'";
		if (m_pPerson->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			if (nRow > 0)
				bExists = TRUE;
		}
		CSqliteDBOP::Free_Result(szResult);

		//insert or update
		if (bExists)
		{
			//update ...
		} else
		{
			//insert into
			char szImageId[16] = {0};
			::itoa(nImageId, szImageId, 10);
			strSql = "insert into widgets(tagsname,widgetname,widgetcaption,widgeturl,imageid,widgettip) values('";
			strSql += szTabId;
			strSql += "','";
			strSql += szItemName;
			strSql += "','";
			strSql += szItemDspName;
			strSql += "','";
			strSql += szItemUrl;
			strSql += "','";
			strSql += szImageId;
			strSql += "','";
			strSql += szItemTip;
			strSql += "')";
		}
		delete []szTmpUrl;
		return m_pPerson->Execute(strSql.c_str());
	}
	return E_FAIL;
}

//返回XML字符串 <widgets><tab id="016FD35A-4DA2-44F8-86EA-C845DDD5F953" name="标签1"/><tab.../></widgets>
STDMETHODIMP CFrameWorkCFGImpl::GetWidgetTabs(IAnsiString *strTabs)
{
	if (m_pPerson)
	{
		std::string strSql = "select ParamName,ParamValue from params where Section='"; 
		strSql += WIDGET_SECTION_NAME;
		strSql += "'";
		char **szResult = NULL;
		int nRow, nCol;
		strTabs->SetString("<widgets>");
		if (m_pPerson->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			for (int i = 1; i <= nRow; i ++)
			{
				if (szResult[i * nCol] && szResult[i * nCol + 1])
				{
					strTabs->AppendString("<tab id=\"");
					strTabs->AppendString(szResult[i * nCol]);
					strTabs->AppendString("\" name=\"");
					strTabs->AppendString(szResult[i * nCol + 1]);
					strTabs->AppendString("\"/>");
				} //end if (szResult[i * nCol
			} //end for (int i
		}  //end if (m_pPerson->
		strTabs->AppendString("</widgets>");
		CSqliteDBOP::Free_Result(szResult);
		return S_OK;
	} //end if (m_pPerson)
	return E_FAIL;
}

//返回XML字符串 <widgets><item tabid="016FD35A-4DA2-44F8-86EA-C845DDD5F953" id="id1" name="GoCom"
//  caption="统一通信平台" url="gocom://" imageid="25" tip="统一通信平台"/><item... /></widgets>
STDMETHODIMP CFrameWorkCFGImpl::GetWidgetItems(IAnsiString *strTabs)
{
	if (m_pPerson)
	{
		std::string strSql = "select tagsname,widgetname,widgetcaption,widgeturl,imageid,widgettip from widgets"; 
		char **szResult = NULL;
		int nRow, nCol;
		strTabs->SetString("<widgets>");
		if (m_pPerson->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			for (int i = 1; i <= nRow; i ++)
			{
				if (szResult[i * nCol] && szResult[i * nCol + 1])
				{
					strTabs->AppendString("<item tabid=\"");
					strTabs->AppendString(szResult[i * nCol]);
					strTabs->AppendString("\" name=\"");
					strTabs->AppendString(szResult[i * nCol + 1]);
					if (szResult[i * nCol + 2])
					{
					    strTabs->AppendString("\" caption=\"");
						strTabs->AppendString(szResult[i * nCol + 2]);
					}
					if (szResult[i * nCol + 3])
					{
					    strTabs->AppendString("\" url=\"");
						strTabs->AppendString(szResult[i * nCol + 3]);
					}
					if (szResult[i * nCol + 4])
					{
					    strTabs->AppendString("\" imageid=\"");
						strTabs->AppendString(szResult[i * nCol + 4]);
					}
					if (szResult[i * nCol + 5])
					{
					    strTabs->AppendString("\" tip=\"");
						strTabs->AppendString(szResult[i * nCol + 5]);
					}
					strTabs->AppendString("\"/>");
				} //end if (szResult[i * nCol
			} //end for (int i
		}  //end if (m_pPerson->
		strTabs->AppendString("</widgets>");
		CSqliteDBOP::Free_Result(szResult);
		return S_OK;
	} //end if (m_pPerson)
	return E_FAIL;
}

//
STDMETHODIMP CFrameWorkCFGImpl::DeleteWidgetTab(const char *szTabId)
{
	if (m_pPerson)
	{
		std::string strSql = "delete from params where (Section='";
		strSql += WIDGET_SECTION_NAME;
		strSql += "' and ParamName='";
		strSql += szTabId;
		strSql += "')";
		if (m_pPerson->Execute(strSql.c_str()))
			return S_OK;
	}
	return E_FAIL;
}

//
STDMETHODIMP CFrameWorkCFGImpl::DeleteWidgetItem(const char *szItemUrl)
{
	if (m_pPerson)
	{
		std::string strSql = "delete from widgets where widgeturl='";
		int nSize = ::strlen(szItemUrl) * 2;
		char *szTmpUrl = new char[nSize + 1];
		memset(szTmpUrl, 0, nSize + 1);
		strSql += CSqliteDBOP::StrToSqlStr(szItemUrl, szTmpUrl);
		strSql += "'";
		delete []szTmpUrl;
		if (m_pPerson->Execute(strSql.c_str()))
			return S_OK;
	}
	return E_FAIL;
}

#pragma warning(default:4996)
