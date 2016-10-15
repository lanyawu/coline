#include <map>
#include <Commonlib/DebugLog.h>
#include <Commonlib/stringutils.h>
#include <Commonlib/systemutils.h>
#include <Commonlib/pinyintrans.h>
#include <SmartSkin/smartskin.h>
#include <xml/tinyxml.h> 
#include <Crypto/crypto.h>
#include <NetLib/httpupdownload.h>
#include <Core/common.h>
#include "../IMCommonLib/InterfaceAnsiString.h"
#include "../IMCommonLib/InterfaceUserList.h"
#include "../P2Svr/P2Svr.h"

#include "EContactsImpl.h"

#define ORG_DOWNLOAD_PATH "/organize/"

#define ORG_LOCAL_FILE_NAME "newest.org"

#define KEY_SEARCH_PAGE_COUNT  10 //搜索结果每页显示

#define LIST_HEADER_HEIGHT 30
#define LIST_ITEM_HEIGHT   30
#define LIST_FOOTER_HEIGHT 30

#pragma warning(disable:4996)

#define PRINT_RUN_TIME_INTERVAL

const char CREATE_MEMORY_DB_DEPT_SQL[] = "create table dept(id INTEGER PRIMARY KEY, name VARCHAR(256), parentid INTEGER,\
										 nameidx VARCHAR(64) COLLATE NOCASE, dispseq INTEGER, depttype INTEGER);";
const char CREATE_MEMORY_DB_USER_SQL[] = "create table user(id INTEGER PRIMARY KEY AUTOINCREMENT, userid INTEGER, username VARCHAR(128) COLLATE NOCASE, deptid INTEGER, \
										 realname VARCHAR(128) COLLATE NOCASE, realnameindex VARCHAR(64) COLLATE NOCASE,\
										 fullpinyin VARCHAR(128) COLLATE NOCASE,deptname VARCHAR(256),mobile VARCHAR(20),fax VARCHAR(20),\
										 tel VARCHAR(20),email VARCHAR(128),usertype INTEGER);\
										 create index user_idx on user(username);";
const char UPDATE_MEMORY_AUTO_INCRE_SQL[]  = "insert into sqlite_sequence(name,seq) values('user','1000000')";

typedef struct 
{
	std::string strUserName;
	CEContactsImpl *pImpl;
}HEADER_HTTP_DL_ITEM, *LPHEADER_HTTP_DL_ITEM;

typedef struct 
{
	CStdString_ strId;
	std::string strFileName;
	CEContactsImpl *pImpl;
}IMAGE_LINK_HTTP_DL_ITEM, *LPIMAGE_LINK_HTTP_DL_ITEM;

void CALLBACK HeaderHttpDlCallBack(int nErrorCode, int nType,
		              WPARAM wParam, LPARAM lParam, void *pOverlapped)
{
	if (pOverlapped)
	{
		if (nErrorCode == ERROR_CODE_COMPLETE)
		{
			LPHEADER_HTTP_DL_ITEM pItem = (LPHEADER_HTTP_DL_ITEM)pOverlapped;
			if (pItem)
			{
				if (nType == FILE_TYPE_NORMAL)
				{ 
					if (pItem->pImpl && (lParam == 0))
					{
						char *pTmp = new char[128];
						memset(pTmp, 0, 128);
						strncpy(pTmp, pItem->strUserName.c_str(), pItem->strUserName.size());
						::PostMessage(pItem->pImpl->m_hUI, WM_USER_DL_HEADER, 0, (LPARAM)pTmp); ;
					} //end if (pItem->
				} //
				delete pItem;
			} //
		} //end if (nErrorCode == ERROR_CODE_COMPLETE)
	} //end if (pOverlapped)
}

void CALLBACK LinkImageHttpDlCallBack(int nErrorCode, int nType,
		              WPARAM wParam, LPARAM lParam, void *pOverlapped)
{
	if (pOverlapped)
	{
		if (nErrorCode == ERROR_CODE_COMPLETE)
		{
			LPIMAGE_LINK_HTTP_DL_ITEM pItem = (LPIMAGE_LINK_HTTP_DL_ITEM)pOverlapped;
			if (pItem)
			{
				if (nType == FILE_TYPE_NORMAL)
				{ 
					if (pItem->pImpl && (lParam == 0))
					{ 
						DWORD dwId = ::SkinAddLinkImage(pItem->strFileName.c_str(), 1, 0);
						pItem->pImpl->ChangeLinkImageId(pItem->strId.GetData(), dwId);
					} //end if (pItem->
				} //
				delete pItem;
			} //
		} //end if (nErrorCode == ERROR_CODE_COMPLETE)
	} //end if (pOverlapped)
}

DWORD CALLBACK GetLinkImageIdByLink(LPCTSTR pstrLink, LPVOID pOverlapped)
{
	CEContactsImpl *pContact = (CEContactsImpl *)pOverlapped;
	if (pContact)
		return pContact->GetImageIdByLink(pstrLink);
	return 0;
}

//http download callback
void CALLBACK EcontactsHttpCallBack(int nErrorCode, int nType, WPARAM wParam, LPARAM lParam, void *pOverlapped)
{
	if (nErrorCode == ERROR_CODE_COMPLETE)
	{
		if (nType == FILE_TYPE_ORGFILE)
		{
			CEContactsImpl *pContacts = (CEContactsImpl *)pOverlapped;
			if (pContacts && (lParam == 0))
			{
				pContacts->m_bDlOrgSucc = TRUE;
			}
		}
	} else if (nErrorCode == ERROR_CODE_PROGRESS)
	{
		if (nType == FILE_TYPE_ORGFILE)
		{
			CEContactsImpl *pContacts = (CEContactsImpl *) pOverlapped;
			if (pContacts)
			{
				ICoreLogin *pLogin = NULL;
				if (SUCCEEDED(pContacts->m_pCore->QueryInterface(__uuidof(ICoreLogin), (void **)&pLogin)))
				{
					ICoreEvent *pEvent = NULL;
					if (SUCCEEDED(pLogin->QueryInterface(__uuidof(ICoreEvent), (void **)&pEvent)))
					{
						char szTmp[32] = {0};
						int nTotal = (int) wParam;
						int nNow = (int) lParam;
						float f = (float) nNow / wParam;
						f *= 100;
						sprintf(szTmp, "%0.2f%%", f);
						pEvent->DoBroadcastMessage("downorg", NULL, "progress", szTmp, NULL);
						pEvent->Release();
					} //end if (SUCCEEDED(
					pLogin->Release();
				} //end if (SUCCEEDED(p
			} //end if (pContacts
		}//end if (nType == 
	}//end if (nErrorCode)
}

CEContactsImpl::CEContactsImpl(void):
                m_pCore(NULL),
				m_pOrgDb(NULL),
				m_hUI(NULL),
				m_pEditHelpOwner(NULL),
				m_iSearchIdx(0),
				m_nUserSign(0),
				m_hWndSearch(NULL),
				m_hEditHelp(NULL)
{
	m_pExtraDb = new CSqliteDBOP(":memory:", NULL);
	if (m_pExtraDb->Execute(CREATE_MEMORY_DB_DEPT_SQL)
		&& m_pExtraDb->Execute(CREATE_MEMORY_DB_USER_SQL))
	{
		m_pExtraDb->Execute(UPDATE_MEMORY_AUTO_INCRE_SQL);
		PRINTDEBUGLOG(dtInfo, "create contact memory db succ");
	} else
	{
		PRINTDEBUGLOG(dtInfo, "create contact memory db failed");
	}
}


CEContactsImpl::~CEContactsImpl(void)
{
	if (m_pEditHelpOwner)
		m_pEditHelpOwner->Release();
	if (m_hEditHelp)
	{
		::SkinCloseWindow(m_hEditHelp);
		m_hEditHelp = NULL;
	}
	if (m_hWndSearch)
	{
		::SkinCloseWindow(m_hWndSearch);
		m_hWndSearch = NULL;
	}
	if (m_pCore)
	{
		m_pCore->Release();
		m_pCore = NULL;
	}
	if (m_pOrgDb)
		delete m_pOrgDb;
	m_pOrgDb = NULL;
	if (m_pExtraDb)
		delete m_pExtraDb;
	m_pExtraDb = NULL;
	ClearUserInfos();
}

//IUnknown
STDMETHODIMP CEContactsImpl::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IContacts)))
	{
		*ppv = (IContacts *) this;
		_AddRef();
		return S_OK;
	} else if (IsEqualIID(riid, __uuidof(ICoreEvent)))
	{
		*ppv = (ICoreEvent *) this;
		_AddRef();
		return S_OK;
	} else if (IsEqualIID(riid, __uuidof(IProtocolParser)))
	{
		*ppv = (IProtocolParser *) this;
		_AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

//lpszLink === "url=http://www.smartdot.com.cn/pic.gif"
DWORD CEContactsImpl::GetImageIdByLink(LPCTSTR lpszLink)
{
	if (!lpszLink)
		return 0;
	{
		CGuardLock::COwnerLock guard(m_LinkLock);
		std::map<CStdString_, DWORD>::iterator it =  m_LinkList.find(lpszLink);
		if (it != m_LinkList.end())
			return it->second;
		m_LinkList.insert(std::pair<CStdString_, DWORD>(lpszLink, 0));
	}
	char szUrl[MAX_PATH] = {0};
	LPCTSTR p = lpszLink;
	while ((*p != L'\0') && (*p != L'='))
		p ++;
	if (*p == L'=')
		p ++;
	CStringConversion::WideCharToString(p, szUrl, MAX_PATH - 1);
	CInterfaceAnsiString strLocalFileName;
	IConfigure *pCfg = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		if (SUCCEEDED(pCfg->GetPath(PATH_LOCAL_CUSTOM_PICTURE, &strLocalFileName)))
		{
			char szGuid[128] = {0};
			int nSize = 127;
			char szExt[MAX_PATH] = {0};
			CSystemUtils::ExtractFileExtName(szUrl, szExt, MAX_PATH - 1);
			md5_encode(szUrl, ::strlen(szUrl), szGuid);
			strLocalFileName.AppendString(szGuid);
			strLocalFileName.AppendString(".");
			strLocalFileName.AppendString(szExt); 
			if (CSystemUtils::FileIsExists(strLocalFileName.GetData()))
			{
				DWORD dwId = ::SkinAddLinkImage(strLocalFileName.GetData(), 1, 0);
				ChangeLinkImageId(lpszLink, dwId);
			} else
			{
				LPIMAGE_LINK_HTTP_DL_ITEM pItem = new IMAGE_LINK_HTTP_DL_ITEM();
				pItem->strFileName = strLocalFileName.GetData();
				pItem->strId = lpszLink;
				pItem->pImpl = this;
				//
				::P2SvrAddDlTask(szUrl, strLocalFileName.GetData(), FILE_TYPE_NORMAL,
					pItem, LinkImageHttpDlCallBack, FALSE);
			}
		}
		pCfg->Release();
	} //end if  
	return 0;
}

//
void CEContactsImpl::ChangeLinkImageId(LPCTSTR lpszLink, DWORD dwId)
{
	CGuardLock::COwnerLock guard(m_LinkLock);
	std::map<CStdString_, DWORD>::iterator it =  m_LinkList.find(lpszLink);
	if (it != m_LinkList.end())
	{
		it->second = dwId;
	} 
}

//
BOOL CEContactsImpl::InitEditHelpWindow()
{
	BOOL b = FALSE;
	if (m_pCore)
	{ 
		IUIManager *pUI = NULL; 
		RECT rc = {1, 1, 300, 200}; 
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)) && pUI)
		{
			pUI->GetWindowHWNDByName("MainWindow", &m_hUI);
			
			if ((m_hEditHelp == NULL) || (!::IsWindow(m_hEditHelp)))
			{
				pUI->CreateUIWindow(NULL, "edithelpwindow", &rc, WS_POPUP ,
				                WS_EX_TOOLWINDOW | WS_EX_TOPMOST, L"", &m_hEditHelp);
				if (m_hEditHelp)
				{
					b = TRUE; 
				}
			}
			if ((m_hWndSearch == NULL) || (!::IsWindow(m_hWndSearch)))
			{
				pUI->CreateUIWindow(NULL, "searchwindow", &rc, WS_POPUP, 
					WS_EX_TOOLWINDOW | WS_EX_TOPMOST, L"", &m_hWndSearch);
				if (m_hWndSearch)
				{
					//::SkinSetWindowTransparent(m_hWndSearch, RGB(255, 255, 255), 
					//	253, LWA_ALPHA | LWA_COLORKEY);
					b = TRUE;
				}
			}
			pUI->Release();
			pUI = NULL;
		}
	} //end if (m_pCore)
	return b;
}

BOOL CEContactsImpl::ShowEditHelpWindow(int x, int y, int w, int h)
{
	BOOL b = FALSE;
	if (m_hEditHelp && ::IsWindow(m_hEditHelp))
	{
		b = TRUE;
	} else
	{
		if (m_hEditHelp)
			::SkinCloseWindow(m_hEditHelp);
		m_hEditHelp = NULL;
		b = InitEditHelpWindow();
	}
	//
	if (b && ::IsWindow(m_hEditHelp))
	{
		RECT rcScreen = {0};
		CSystemUtils::GetScreenRect(&rcScreen);
		if (!::AnimateWindow(m_hEditHelp, 200, AW_BLEND))
		{
			//if (m_hEditHelp)
			//   ::SkinCloseWindow(m_hEditHelp);
			//m_hEditHelp = NULL;
			//b = InitEditHelpWindow();
			::ShowWindow(m_hEditHelp, SW_SHOW);
		}
		//CSystemUtils::BringToFront(m_hEditHelp);
		if ((y + h) > rcScreen.bottom)
		{
			y -= h;
			y -= 30; //searchedit height
		}
		::MoveWindow(m_hEditHelp, x, y, w, h, TRUE);
		return TRUE;
	}
	return FALSE;
}

void CEContactsImpl::SearchList(const char *szKey, int nIdx, int &h)
{
	if ((szKey == NULL) || (::strlen(szKey) == 0) || (::strlen(szKey) > 32))
		return ;
	char szDest[64] = {0};
	char szTmp[64] = {0};
	CStringConversion::Trim(szKey, szTmp);
	CStringConversion::StringToUTF8(szTmp, szDest, 63);
	if (::strlen(szDest) > 0)
	{
		std::string strSql = "select id,realname,departmentname from user ";
		std::string strSqlWhere = "where realname like \"";
		strSqlWhere += szDest;
		strSqlWhere += "%%\" or fullpinyin like \"";
		strSqlWhere += szDest;
		strSqlWhere += "%%\" or realnameindex like \"";
		strSqlWhere += szDest;
		strSqlWhere += "%%\"";
		strSql += strSqlWhere;
		strSql += " limit ";
		char szTmp[16] = {0};
		int n = KEY_SEARCH_PAGE_COUNT * nIdx;
		strSql += ::itoa(n, szTmp, 10);
		strSql += ",";
		strSql += ::itoa(KEY_SEARCH_PAGE_COUNT, szTmp, 10);
		char **szResult = NULL;
		int nRow, nCol;
		m_strSearchKey = szKey;
		m_iSearchIdx = nIdx; 
		if (m_pOrgDb->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			std::string strTmp;
			int id;
			for (int i = 1; i <= nRow; i ++)
			{
				if (szResult[i * nCol] && szResult[i * nCol + 1])
				{
					id = ::atoi(szResult[i * nCol]);
					strTmp = szResult[i * nCol + 1];
					if (szResult[i * nCol + 2])
					{
						strTmp += "    ";
						strTmp += szResult[i * nCol + 2];
					}
					TCHAR *szTmp = new TCHAR[strTmp.size() + 1];
					memset(szTmp, 0, sizeof(TCHAR) * (strTmp.size() + 1));
					CStringConversion::UTF8ToWideChar(strTmp.c_str(), szTmp, strTmp.size());
					::SkinInsertListItem(m_hEditHelp, L"resultlist", szTmp, (void *)id, 999999);
					m_nSearchShownCount ++;
					delete []szTmp;
				} //end if (szResult
			}  //end for (
		} //end if (m_pOrgDb
		CSqliteDBOP::Free_Result(szResult);  
		szResult = NULL;
		if (nRow == KEY_SEARCH_PAGE_COUNT)
		{
			::SkinSetControlEnable(m_hEditHelp, L"moreresult", TRUE);
		} else
		{
			//再加入外部联系人
			strSql = "select id,realname from user ";
			strSql += strSqlWhere;
			if (m_pExtraDb->Open(strSql.c_str(), &szResult, nRow, nCol))
			{
				std::string strTmp;
				int id;
				for (int i = 1; i <= nRow; i ++)
				{
					if (szResult[i * nCol] && szResult[i * nCol + 1])
					{
						id = ::atoi(szResult[i * nCol]);
						strTmp = szResult[i * nCol + 1];
					    
						TCHAR *szTmp = new TCHAR[strTmp.size() + 32]; //strlen(外部联系人)
						memset(szTmp, 0, sizeof(TCHAR) * (strTmp.size() + 32));
						CStringConversion::UTF8ToWideChar(strTmp.c_str(), szTmp, strTmp.size());
						::lstrcat(szTmp, L"    外部联系人");
						::SkinInsertListItem(m_hEditHelp, L"resultlist", szTmp, (void *)id, 999999);
						m_nSearchShownCount ++;
						delete []szTmp;
					} //end if (szResult
				}  //end for (
			} 
			CSqliteDBOP::Free_Result(szResult);
			szResult = NULL;
			::SkinSetControlEnable(m_hEditHelp, L"moreresult", FALSE);
		}
		int nCount = ::SkinGetListCount(m_hEditHelp, L"resultlist");
		h = nCount * LIST_ITEM_HEIGHT + LIST_HEADER_HEIGHT + LIST_FOOTER_HEIGHT;
		if (nCount > 5)
		{
			h = 5 * LIST_ITEM_HEIGHT + LIST_HEADER_HEIGHT + LIST_FOOTER_HEIGHT;
		}

		if (nCount == 0)
		{
			h -= LIST_FOOTER_HEIGHT;
			::SkinSetControlTextByName(m_hEditHelp, L"title", L"没有搜索结果");
			::SkinSetControlVisible(m_hEditHelp, L"footer", FALSE);
		} else
		{
			if (nCount <= KEY_SEARCH_PAGE_COUNT)
				::SkinSetListSelItem(m_hEditHelp, L"resultlist", 0);
			if (nIdx == 0)
			{
				strSql = "select count(id) from user ";
				strSql += strSqlWhere;
				m_nSearchTotalCount = 0;
				if (m_pOrgDb->Open(strSql.c_str(), &szResult, nRow, nCol))
				{
					if ((nRow > 0) && (szResult[1]))
					{
						m_nSearchTotalCount = ::atoi(szResult[1]);
					}  
					CSqliteDBOP::Free_Result(szResult);	
				}
				//外部联系人
				if (m_pExtraDb->Open(strSql.c_str(), &szResult, nRow, nCol))
				{
					if ((nRow > 0) && (szResult[1]))
					{
						m_nSearchTotalCount += ::atoi(szResult[1]);
					}
					CSqliteDBOP::Free_Result(szResult);
				} //end if (m_pExtraDb->
			}
			if (m_nSearchTotalCount > 0)
			{
				TCHAR szwTmp[128] = {0};
				::wsprintf(szwTmp, L"找到 %d 个结果，已显示 %d", m_nSearchTotalCount, m_nSearchShownCount);
				::SkinSetControlTextByName(m_hEditHelp, L"title", szwTmp);
			} else
			{  
				::SkinSetControlTextByName(m_hEditHelp, L"title", L"请输入姓名、简拼或者全拼");
			}
			::SkinSetControlVisible(m_hEditHelp, L"footer", TRUE);
		}
	}
}

//获取用户某项数据
STDMETHODIMP CEContactsImpl::GetUserValueByParam(const char *szUserName, const char *szParam, IAnsiString *strValue)
{
	HRESULT hr = E_FAIL;
	std::string strName, strDomain;
	if (SepNameDomainByUserName(szUserName, strName, strDomain))
	{
		std::string strSql = "select ";
		strSql += szParam;
		strSql += " from user where lower(username)=lower('";
		strSql += strName;
		strSql += "')";
		char **szResult = NULL;
	    int nRow, nCol;
	    if (m_pOrgDb->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			if ((nRow > 0) && szResult[1])
			{
				strValue->SetString(szResult[1]);
				hr = S_OK;
			} //end if (szResult[1])
		} //end if (m_pOrgDb->...
		CSqliteDBOP::Free_Result(szResult); 
	} else  //外部联系人
	{
		std::string strSql = "select ";
		strSql += szParam;
		strSql += " from user where lower(username)=lower('";
		strSql += szUserName;
		strSql += "')";
		char **szResult = NULL;
		int nRow, nCol;
		if (m_pExtraDb->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			if ((nRow > 0) && szResult[1])
			{
				strValue->SetString(szResult[1]);
				hr = S_OK;
			} //end if (nRow > 0...
		} //end if (m_pExtraDb->
		CSqliteDBOP::Free_Result(szResult);
	}
	return hr;
}

//获取邮箱
STDMETHODIMP CEContactsImpl::GetMailByUserName(const char *szUserName, IAnsiString *strMail)
{
	HRESULT hr = E_FAIL;
	std::string strName, strDomain;
	if (SepNameDomainByUserName(szUserName, strName, strDomain))
	{
		std::string strSql = "select email from user where lower(username)=lower('";
		strSql += strName;
		strSql += "')";
		char **szResult = NULL;
	    int nRow, nCol;
	    if (m_pOrgDb->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			if ((nRow > 0) && szResult[1])
			{
				strMail->SetString(szResult[1]);
				hr = S_OK;
			} //end if (szResult[1])
		} //end if (m_pOrgDb->...
		CSqliteDBOP::Free_Result(szResult); 
	} else  //外部联系人
	{
		std::string strSql = "select email from user where lower(username)=lower('";
		strSql += szUserName;
		strSql += "')";
		char **szResult = NULL;
		int nRow, nCol;
		if (m_pExtraDb->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			if ((nRow > 0) && szResult[1])
			{
				strMail->SetString(szResult[1]);
				hr = S_OK;
			} //end if (nRow > 0...
		} //end if (m_pExtraDb->
		CSqliteDBOP::Free_Result(szResult);
	}
	return hr;
}

HRESULT CEContactsImpl::DoLinkEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "moreresult") == 0)
	{
		std::string strTmp = m_strSearchKey;
		int idx = m_iSearchIdx + 1;
		int h = 100;
		SearchList(strTmp.c_str(), idx, h);
		::SkinUpdateControlUI(m_hEditHelp, L"resultlist");
	}
	return -1;
}

 
//获取个性头像
STDMETHODIMP CEContactsImpl::GetContactHead(const char *szUserName, IAnsiString *strFileName, BOOL bRefresh)
{
	//
	HRESULT hr = E_FAIL;
	IConfigure *pCfg = NULL;
	std::string strUserName;
	if (szUserName)
		strUserName = szUserName;
	else
		strUserName = m_strUserName;
	CInterfaceAnsiString strLocalFileName;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		if (SUCCEEDED(pCfg->GetPath(PATH_LOCAL_USER_HEAD, &strLocalFileName)))
		{
			strLocalFileName.AppendString(strUserName.c_str());
			strLocalFileName.AppendString(".bmp");
			if (CSystemUtils::FileIsExists(strLocalFileName.GetData()))
			{
				strFileName->SetString(strLocalFileName.GetData());
				hr = S_OK;
			}  //end if (CSystemUtils::
		}//end if (SUCCEEDED(pCfg->
		pCfg->Release();
	} //end if (SUCCEEDED(m_pCore->
	if (bRefresh) //下载
	{
		CInterfaceAnsiString strHttpSvr;
		IConfigure *pCfg = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			pCfg->GetServerAddr(HTTP_SVR_URL_HTTP, &strHttpSvr);
			strHttpSvr.AppendString("/userpic/"); //userpic svr path
			strHttpSvr.AppendString(strUserName.c_str());
			strHttpSvr.AppendString(".head");
			LPHEADER_HTTP_DL_ITEM pItem = new HEADER_HTTP_DL_ITEM();
			pItem->strUserName = strUserName;
			pItem->pImpl = this;
			//
			::P2SvrAddDlTask(strHttpSvr.GetData(), strLocalFileName.GetData(), FILE_TYPE_NORMAL,
				pItem, HeaderHttpDlCallBack, FALSE);
			pCfg->Release();
		} //end if 
	} //end if (
	return hr; 
}

//上传头像
STDMETHODIMP CEContactsImpl::UploadHead(const char *szFileName)
{
	IConfigure *pCfg = NULL;
	HRESULT hr = E_FAIL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		CInterfaceAnsiString strUrl;
		if (SUCCEEDED(pCfg->GetServerAddr(HTTP_SVR_URL_USER_HEAD, &strUrl)))
		{
			std::string strParams;
			strParams = "username=";
			strParams += m_strUserName;
			::P2SvrPostFile(strUrl.GetData(), szFileName, strParams.c_str(), FILE_TYPE_NORMAL, this, NULL, TRUE);
			hr = S_OK;
		} //end if (pCfg->
		pCfg->Release();
	} //end if (SUCCEEDED(...
	return hr;
}

void CEContactsImpl::OpenFrameByRealName(HWND hWndFrom, const char *szName)
{
	char szUTF8[128] = {0};
	CStringConversion::StringToUTF8(szName, szUTF8, 127);
	std::string strSql = "select username,domain,realname from user where realname like\""; 
	strSql += szUTF8; 
	strSql += "%%\" or fullpinyin like \"";
	strSql += szUTF8;
	strSql += "%%\" or realnameindex like \"";
	strSql += szUTF8;
	strSql += "%%\""; 
	char **szResult = NULL;
	int nRow, nCol;
	if (m_pOrgDb->Open(strSql.c_str(), &szResult, nRow, nCol))
	{
		if (nRow > 0)
		{
			if (szResult[3] && szResult[4] && szResult[5])
			{
				std::string strUserName = szResult[3];
				strUserName += "@";
				strUserName += szResult[4];
				IChatFrame *pChat = NULL;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pChat)))
				{
					pChat->ShowChatFrame(hWndFrom, strUserName.c_str(), szResult[5]);
					pChat->Release();
				} //end if (m_pCore...
			} //end if (szResult[3]..
		} else 
		{
			m_pCore->BroadcastMessage("edithelp", m_hEditHelp, "search_key", szName, NULL);
		}//end if (nRow > 0)		
	}
	CSqliteDBOP::Free_Result(szResult);	
}

void CEContactsImpl::OpenFrameById(HWND hWndFrom, const int nId)
{
	std::string strSql = "select username,domain,realname from user where id=";
	char szTmp[16] = {0};
	strSql += ::itoa(nId, szTmp, 10);
	char **szResult = NULL;
	int nRow, nCol;
	if (m_pOrgDb->Open(strSql.c_str(), &szResult, nRow, nCol))
	{
		if (nRow > 0)
		{
			if (szResult[3] && szResult[4] && szResult[5])
			{
				std::string strUserName = szResult[3];
				strUserName += "@";
				strUserName += szResult[4];
				IChatFrame *pChat = NULL;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pChat)))
				{
					pChat->ShowChatFrame(hWndFrom, strUserName.c_str(), szResult[5]);
					pChat->Release();
				} //end if (m_pCore...
			} //end if (szResult[3]..
		} //end if (nRow > 0)		
	}
	CSqliteDBOP::Free_Result(szResult);	
}

BOOL CEContactsImpl::GetUserNameById(const int nId, std::string &strUserName, std::string &strRealName)
{
	BOOL bSucc = FALSE;
	std::string strSql = "select username,domain,realname from user where id=";
	char szTmp[16] = {0};
	strSql += ::itoa(nId, szTmp, 10);
	char **szResult = NULL;
	int nRow, nCol;
	if (m_pOrgDb->Open(strSql.c_str(), &szResult, nRow, nCol))
	{
		if (nRow > 0)
		{
			if (szResult[3] && szResult[4] && szResult[5])
			{
				strUserName = szResult[3];
				strUserName += "@";
				strUserName += szResult[4];
				//
				strRealName = szResult[5];
				bSucc = TRUE;
			} //end if (szResult[3]..
		} //end if (nRow > 0)		
	} //end if (m_pOrgDb->..
	CSqliteDBOP::Free_Result(szResult);	
	return bSucc;
}

HRESULT CEContactsImpl::DoItemActivate(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{ 
	HRESULT hr = -1;
	if (hWnd == m_hEditHelp)
	{
		if (::stricmp(szName, "resultlist") == 0)
		{
			int idx = ::SkinGetListSelItem(m_hEditHelp, L"resultlist");
			if (idx >= 0)
			{
				TCHAR szTmp[256] = {0};
				void *pData = NULL;
				if (::SkinGetListItemInfo(m_hEditHelp, L"resultlist", szTmp, &pData, idx))
				{
					std::string strUserName, strRealName;
					if (m_pEditHelpOwner && GetUserNameById((int) pData, strUserName, strRealName))
					{
						if (m_pEditHelpOwner == this)
						{
							::SkinSetControlTextByName(m_hWndSearch, L"searchedit", szTmp);
							IChatFrame *pChat = NULL;
							if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pChat)))
							{
								pChat->ShowChatFrame(m_hWndSearch, strUserName.c_str(), NULL);
								pChat->Release();
							}
						} else
						{
							memset(szTmp, 0, 256 * sizeof(TCHAR));
							CStringConversion::UTF8ToWideChar(strRealName.c_str(), szTmp, 255);
							m_pEditHelpOwner->DoCoreEvent(m_hEditHelp, "itemactivate", "resultlist",
								(WPARAM)szTmp, (LPARAM)strUserName.c_str(), &hr);
						}
					} //end if (m_pEditHelpOwner && ..		
					//::AnimateWindow(m_hEditHelp, 500, AW_HIDE);
					::ShowWindow(m_hEditHelp, SW_HIDE); 
				} //end if (::SkinGetListItemInfo(..
			} //end if (idx >= 0) 
			hr = S_OK;
		} //end if (::stricmp(szName..
	} //end if (hWnd == m_hEditHelp)
	return hr;
}
//
STDMETHODIMP CEContactsImpl::EditVirtualKeyUp(WORD wKey)
{
	if (::IsWindowVisible(m_hEditHelp))
	{
		if (::SkinListKeyDownEvent(m_hEditHelp, L"resultlist", wKey))
			return S_OK;
	}
	return E_FAIL;
}

//bActive 是否活动状态 
//bEnded 是否输入完毕
STDMETHODIMP CEContactsImpl::EditHelpSearchActive(HWND hWndFrom, const char *szText, BOOL bActived, BOOL bEnded)
{
	if (bActived)
	{
		if (bEnded)
		{
			OpenFrameByRealName(hWndFrom, szText);
		} else
		{
			if (::IsWindowVisible(m_hEditHelp))
			{
				DoItemActivate(m_hEditHelp, "resultlist", 0, 0);
			} else
			{
				OpenFrameByRealName(hWndFrom, szText);
			}
		}
	} else
	{
		::ShowWindow(m_hEditHelp, SW_HIDE); 
	}
	return S_OK;
}
 

 
STDMETHODIMP CEContactsImpl::HideHelpEditWindow()
{
	if (::GetForegroundWindow() == m_hEditHelp)
		return S_OK;
	if (m_pEditHelpOwner)
		m_pEditHelpOwner->Release();
	m_pEditHelpOwner = NULL;
	if (m_hEditHelp)
	{
		::ShowWindow(m_hEditHelp, SW_HIDE);
		return S_OK;
	}
	return E_FAIL;
}

//显示搜索框
STDMETHODIMP CEContactsImpl::ShowSearchFrame(int x, int y, int w, int h)
{
	if ((m_hWndSearch == NULL) || (!::IsWindow(m_hWndSearch)))
		InitEditHelpWindow();
	if ((m_hWndSearch != NULL) && (::IsWindow(m_hWndSearch)))
	{
		::MoveWindow(m_hWndSearch, x, y, w, h, FALSE); 
		::ShowWindow(m_hWndSearch, SW_SHOW);
		::SkinSetControlFocus(m_hWndSearch, L"searchedit", TRUE);
		return S_OK;
	}
	return E_FAIL;
}

//ICoreEvent
STDMETHODIMP CEContactsImpl::DoCoreEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
             LPARAM lParam, HRESULT *hResult)
{
	if (::stricmp(szType, "itemactivate") == 0)
	{
		*hResult = DoItemActivate(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "link") == 0)
	{
		*hResult = DoLinkEvent(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "afterinit") == 0)
	{
		if (::stricmp(szName, "mainwindow") == 0)
		{
			InitEditHelpWindow();
			::SkinSetLinkImageCallBack(GetLinkImageIdByLink, this);
		}
	} else if (::stricmp(szType, "killfocus") == 0)
	{
		if (::stricmp(szName, "searchedit") == 0)
		{ 
			HideHelpEditWindow(); 
		}
	} else if (::stricmp(szType, "editchanged") == 0)
	{
		if ((hWnd == m_hWndSearch) && (::stricmp(szName, "searchedit") == 0))
		{
			TCHAR szwText[64] = {0};
			int nSize = 63;
			if (::SkinGetControlTextByName(m_hWndSearch, L"searchedit", szwText, &nSize))
			{
				char szText[64] = {0};
				CStringConversion::WideCharToString(szwText, szText, 63);
				char szDest[64] = {0};
				CStringConversion::Trim(szText, szDest);
				RECT rc = {0};
				::SkinGetControlRect(hWnd, L"searchedit", &rc);
				POINT pt = {rc.left, rc.bottom};
				::ClientToScreen(hWnd, &pt); 
				ShowHelpEditWindow((ICoreEvent *) this, szDest, pt.x, pt.y, rc.right - rc.left, 100); 
			} //end if (::
		} //end 
	} else if (::stricmp(szType, "keydown") == 0)
	{
		if ((hWnd == m_hWndSearch) && (::stricmp(szName, "searchedit") == 0))
		{ 
			switch(wParam)
			{
				case VK_RETURN: 
					{
						TCHAR szTmp[128] = {0};
						int nSize = 127;				
						char szRealName[128] = {0};
						if (::SkinGetControlTextByName(m_hWndSearch, L"searchedit", szTmp, &nSize))
						{
							CStringConversion::WideCharToString(szTmp, szRealName, 127);
						} 
						EditHelpSearchActive(hWnd, szRealName, TRUE, FALSE);
						break;
				    } 
				case VK_ESCAPE:
				{
					EditHelpSearchActive(hWnd, NULL, FALSE, FALSE);
					break;
				}
				case VK_UP:
				case VK_DOWN:
				case VK_PRIOR:
				case VK_NEXT:
				case VK_HOME:
				case VK_END:
				{
					EditVirtualKeyUp(wParam);
					*hResult = 0;
					break;
				}
			}  
		} //end if (::stricmp(szName...
	} else if (::stricmp(szType, "click") == 0)
	{
		if ((hWnd == m_hWndSearch) && (::stricmp(szName, "closebutton") == 0))
		{
			::ShowWindow(m_hWndSearch, SW_HIDE);
		} //end if ((hWnd
	} //end else if (::stricmp(
	return E_FAIL;
}


STDMETHODIMP CEContactsImpl::ShowHelpEditWindow(ICoreEvent *pOwner, const char *szText, int x, int y, int w, int h)
{
	if (m_pEditHelpOwner != pOwner)
	{
		if (m_pEditHelpOwner)
			m_pEditHelpOwner->Release();
		m_pEditHelpOwner = pOwner;
		if (m_pEditHelpOwner)
			m_pEditHelpOwner->AddRef();
	}
	if (::strlen(szText) > 0)
	{		
		::SkinRemoveListItem(m_hEditHelp, L"resultlist", -1);
		int h1 = 100;	
		m_nSearchTotalCount = 0;
		m_nSearchShownCount = 0;
		SearchList(szText, 0, h1);
		ShowEditHelpWindow(x, y, w, h1); 
		return S_OK;
	}
		
	return E_FAIL;
}


//广播消息
STDMETHODIMP CEContactsImpl::DoBroadcastMessage(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData)
{
	return E_NOTIMPL;
}

STDMETHODIMP CEContactsImpl::GetContactUserValue(const char *szUserName, 
	                              const char *szParam, IAnsiString *szValue)
{
	if (szUserName)
	{
		CGuardLock::COwnerLock guard(m_InfoLock);
		std::map<CAnsiString_, CInstantUserInfo *>::iterator it = m_UserInfos.find(szUserName);
		if (it != m_UserInfos.end())
		{
			if (::stricmp(szParam, "presence") == 0)
				return it->second->GetUserStatus(szValue);
			else
				return it->second->GetUserInfo(szParam, szValue);
		} //end if (it != 
	} //end if (szUserName)
	return E_FAIL;
}

STDMETHODIMP CEContactsImpl::SetCoreFrameWork(ICoreFrameWork *pCore)
{
	if (m_pCore)
		m_pCore->Release();
	m_pCore = pCore;
	if (m_pCore)
	{
		m_pCore->AddRef();
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "mainwindow", "afterinit");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "edithelpwindow", "resultlist", "itemactivate");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "edithelpwindow", "moreresult", "link");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "searchwindow", NULL, NULL);
		//order protocol
		m_pCore->AddOrderProtocol((IProtocolParser *)this, "sys", "organize");
		m_pCore->AddOrderProtocol((IProtocolParser *)this, "sta", "order");
		m_pCore->AddOrderProtocol((IProtocolParser *)this, "sys", "sign");
		m_pCore->AddOrderProtocol((IProtocolParser *)this, "sys", "presence");
	}
	return E_NOTIMPL;
}

STDMETHODIMP CEContactsImpl::GetSkinXmlString(IAnsiString *szXmlString)
{
	HRESULT hr = E_FAIL;
	IConfigure *pCfg = NULL; 
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{ 
		hr = pCfg->GetSkinXml("companyorg.xml",szXmlString); 
		pCfg->Release();
	}
	return hr;   
}

void CEContactsImpl::SetAllUserStauts(const char *szStatus)
{
	CGuardLock::COwnerLock guard(m_InfoLock);
	std::map<CAnsiString_, CInstantUserInfo *>::iterator it;
	for (it = m_UserInfos.begin(); it != m_UserInfos.end(); it ++)
	{
		it->second->SetUserStatus(szStatus);
	} 
}

//
STDMETHODIMP CEContactsImpl::CoreFrameWorkError(int nErrorNo, const char *szErrorMsg)
{
	switch(nErrorNo)
	{
	case CORE_ERROR_SOCKET_CLOSED:
	case CORE_ERROR_KICKOUT:
		 SetAllUserStauts("offline");
		 break;
	case CORE_ERROR_LOGOUT:
		 ClearUserInfos();
		 m_strUserName.clear();
		 m_strOrgLocalName.clear();
		 m_strSearchKey.clear();
		 m_iSearchIdx = 0;
		 break;
	}
	return E_NOTIMPL;
}

STDMETHODIMP CEContactsImpl::DoWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes)
{
	switch(uMsg)
	{
		case WM_HTTPDLPROGRESS:
			{
				//wParam = Total Size   lParam = DlNow
				PRINTDEBUGLOG(dtInfo, "Recv HttpDlProgress");
				 			
				break;
			}
	}
	return E_FAIL;
}

//界面绘制
void CEContactsImpl::SignUpdateToUI(const char *szUID, const char *szUTF8Sign)
{ 
	//update to ui
	IMainFrame *pFrame = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMainFrame), (void **)&pFrame)))
	{
		pFrame->UpdateUserLabel(szUID, szUTF8Sign);
		pFrame->Release();
	} 
	if (m_hUI)
	{
		char *szUserName = new char[strlen(szUID) + 1];
		char *szSign = new char[strlen(szUTF8Sign) + 1];
		strcpy(szUserName, szUID);
		szUserName[strlen(szUID)] = '\0';
		strcpy(szSign, szUTF8Sign);
		szSign[strlen(szUTF8Sign)] = '\0';
		::PostMessage(m_hUI, WM_SIGNCHANGE, (WPARAM) szUserName, (LPARAM) szSign); 
	}
	 
}

BOOL CEContactsImpl::DoSignChange(TiXmlElement *pNode)
{
	//<sys type="sign" uid="user@domain" signver="2342" sign="签名"/>
	const char *szUID = pNode->Attribute("uid");
	const char *szVer = pNode->Attribute("signver");
	const char *szSign = pNode->Attribute("sign");
	if (szUID && szVer && szSign)
	{
		//update memory list
		SetUserInfo(szUID, "signver", szVer);		
		//write to local db

		//update to ui
		char szUTF8[MAX_PATH] = {0};
		CStringConversion::StringToUTF8(szSign, szUTF8, MAX_PATH - 1);
		SetUserInfo(szUID, "sign", szUTF8);
		SignUpdateToUI(szUID, szUTF8);	 
		return TRUE;
	}
	return FALSE;
}

//
BOOL CEContactsImpl::DoStatusChange(TiXmlElement *pNode)
{
	// "<sys type="presence" uid="user@doamin"  presence="online" memo="在线"/> 
	const char *szUID = pNode->Attribute("uid");
	const char *szPresence = pNode->Attribute("presence");
	const char *szMemo = pNode->Attribute("memo");
	if (szUID && szPresence)
	{ 
		SetUserStatus(szUID, szPresence, szMemo, FALSE);
		return TRUE;
	}
	return FALSE;
}


BOOL CEContactsImpl::DoStatusProtocol(TiXmlElement *pNode)
{
	/*  状态回应xml
			    <sta type="order" result="ok">
				   <i u="users3" presence="online" signver="2" sign="个性签名"/>
				   <i u="users4" presence="offline"/>
				</sta>
	*/ 
	const char *szResult = pNode->Attribute("result");
	if (szResult && ::stricmp(szResult, "ok") == 0)
	{
		TiXmlElement *pChild = pNode->FirstChildElement();
		IMainFrame *pFrame = NULL;
		if (m_pCore)
			m_pCore->QueryInterface(__uuidof(IMainFrame), (void **)&pFrame);
		while (pChild)
		{
			const char *szUserName = pChild->Attribute("u");
			const char *szStatus = pChild->Attribute("presence");
			const char *szSignver = pChild->Attribute("signver");
			const char *szSign = pChild->Attribute("sign");
			if (szUserName && szStatus)
			{
				SetUserStatus(szUserName, szStatus, NULL, TRUE);
				if (szSignver)
					SetUserInfo(szUserName, "signver", szSignver);
				if (szSign && pFrame)
				{					
					//update to ui
					char szUTF8[MAX_PATH] = {0};
					CStringConversion::StringToUTF8(szSign, szUTF8, MAX_PATH - 1);
					 
					SetUserInfo(szUserName, "sign", szUTF8);
					SignUpdateToUI(szUserName, szUTF8);
				}
			}
			pChild = pChild->NextSiblingElement();
		}
		if (pFrame)
			pFrame->Release();
		//update ui
		//::UpdateControlUI(m_hUI, UI_COMPANY_TREE_NAME);
		return TRUE;
	} else
	{
		PRINTDEBUGLOG(dtInfo, "sta ack protocol failed");
		return FALSE;
	} 
}

STDMETHODIMP CEContactsImpl::GetCacheContactsFileName(IAnsiString *strFileName)
{
	IConfigure *pCfg = NULL;
	std::string strTmp;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		CInterfaceAnsiString strCachePath;
		if (SUCCEEDED(pCfg->GetParamValue(FALSE, "Path", "cachePath", (IAnsiString *) &strCachePath)))
		{
			strTmp = strCachePath.GetData();								
		} else
		{
			if (SUCCEEDED(pCfg->GetParamValue(FALSE, "Path", "PersonPath",
				                             (IAnsiString *)&strCachePath)))
			{
				strTmp = strCachePath.GetData();
			} //end if (SUCCEEDED(..
		} //end else if (SUCCEEDED(...
		strTmp += ORG_LOCAL_FILE_NAME;
		pCfg->Release();
	}
	if (!strTmp.empty())
	{
		strFileName->SetString(strTmp.c_str());
		return S_OK;
	} else
		return E_FAIL;
}

//获取部门路径
STDMETHODIMP CEContactsImpl::GetDeptPathNameByUserName(const char *szUserName, IAnsiString *strPathName)
{
	HRESULT hr = E_FAIL;
	std::string strName, strDomain;
	if (!SepNameDomainByUserName(szUserName, strName, strDomain))
	{
		strName = szUserName;
	}
	if (m_pOrgDb)
	{
		std::string strSql = "select deptid from user where lower(username)=lower('";
		strSql += strName;
		strSql += "')";
		char **szResult = NULL;
		int nRow, nCol;
		std::string strDeptId;
		if (m_pOrgDb->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			if ((nRow > 0) && szResult[1])
			{
				strDeptId = szResult[1];
			}
		}
		CSqliteDBOP::Free_Result(szResult);
		szResult = NULL;
		if (!strDeptId.empty())
		{
			strSql = "select dtpall from dept where id=";
			strSql += strDeptId;
			if (m_pOrgDb->Open(strSql.c_str(), &szResult, nRow, nCol))
			{
				if ((nRow > 0) && szResult[1])
				{
					strPathName->SetString(szResult[1]);
					hr = S_OK;
				} //end if ((nRow > 0)
			} // end if (m_pOrgDb->
		} //end if (!strDeptId...
		CSqliteDBOP::Free_Result(szResult);
	}
	return hr;
}

//IProtocolParser
STDMETHODIMP CEContactsImpl::DoPresenceChange(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder)
{
	return E_NOTIMPL;
}

STDMETHODIMP CEContactsImpl::DoRecvProtocol(const BYTE *pData, const LONG lSize)
{
	TiXmlDocument xml;
	if (xml.Load((char *)pData, lSize))
	{
		TiXmlElement *pNode = xml.FirstChildElement();
		if (pNode)
		{
			const char *pValue = pNode->Value();
			const char *pType = pNode->Attribute("type");
			if (pValue && pType)
			{
				//下载联系人
				if (::stricmp(pValue, "sys") == 0)
				{
					if (::stricmp(pType, "organize") == 0)
					{
						CInterfaceAnsiString strTmp;
						if (SUCCEEDED(GetCacheContactsFileName((IAnsiString *)&strTmp)))
						{
							m_strOrgLocalName = strTmp.GetData();
						} else
							m_strOrgLocalName = "";
						m_bDlOrgSucc = TRUE;
						const char *szResult = pNode->Attribute("result");
						if (::stricmp(szResult, "no") == 0)
						{
							const char *szUrl = pNode->Attribute("url");
							if (szUrl)
							{
								DownloadOrg(szUrl);
								if (m_bDlOrgSucc)
								{
									m_bDlOrgSucc = CheckContactValid();
									IConfigure *pCfg = NULL;
									if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
									{
										pCfg->SetParamValue(FALSE, "contacts", "version", pNode->Attribute("version"));
										pCfg->Release();
									} //end if (m_pCore ...
								} //end if (m_bDlOrgSucc)
							}  else
							{
								PRINTDEBUGLOG(dtInfo, "get org url failed");
							}//end if (szUrl)
						} //end if (::stricmp(..
						char szRes[16] = {0};
						ClearMemoryUsers();
						if (m_bDlOrgSucc)
						{
							LoadContacts();
							strcpy(szRes, "true");
						} else
						{
							strcpy(szRes, "false");
						}
						//广播组织结构下载完成 //notify download complete
				        m_pCore->BroadcastMessage("contacts", NULL, "downorg", szRes, NULL);

					} else if (::stricmp(pType, "presence") == 0)
					{
						DoStatusChange(pNode);
					} else if (::stricmp(pType, "sign") == 0)
					{
						DoSignChange(pNode);
					} else
					{
						PRINTDEBUGLOG(dtInfo, "Invalid sys protocol in econtacts, type:%s", pType);
					}//end else if (::stricmp(pType, ..
				} else if (::stricmp(pValue, "sta") == 0)
				{
					if (::stricmp(pType, "order") == 0)
					{
						DoStatusProtocol(pNode);
					}  else
					{
						PRINTDEBUGLOG(dtInfo, "invalid order protocol in econtacts type:%s", pType);
					}//end if (::stricmp(pType...
				} else
				{
					PRINTDEBUGLOG(dtInfo, "EContacts Recv Protocol, Value:%s Type;%s", pValue, pType);
				} //end else if (::stricmp(pValue, "sys")...
			} //end if (pValue && pType)
			return S_OK;
		} //end if (pNode)
	} //end if (xml.Load((char *)pData...
	return E_FAIL;
}



STDMETHODIMP CEContactsImpl::GetRealNameById(const char *szUserName, const char *szDomain, IAnsiString *szName)
{
	if (!InitContactDb())
		return E_FAIL;
	std::string strTmp = szUserName;
	if (szDomain != NULL)
	{
		strTmp += '@';
		strTmp += szDomain;
	}
	if (SUCCEEDED(GetContactUserValue(strTmp.c_str(), "realname", szName)))
		return S_OK;

	std::string strName, strDomain;
	if (szDomain == NULL)
	{
		strName = szUserName;
		int nPos = strName.find('@');
		if (nPos != std::string::npos)
		{
			strDomain = strName.substr(nPos + 1);
			strName = strName.substr(0, nPos);
		}  
	} else
	{
		strName = szUserName;
		strDomain = szDomain;
	}
	std::string strSql = "select realname from user where lower(username)=lower('";
	strSql += strName;
	//strSql += "' and domain='";
	//strSql += strDomain;
	strSql += "')";
	char **szResult = NULL;
	int nCol, nRow;
	HRESULT hr = E_FAIL;
	if (m_pOrgDb->Open(strSql.c_str(), &szResult, nRow, nCol))
	{
		if ((nRow > 0) && (szResult[1]))
		{
			szName->SetString(szResult[1]); 			
			SetUserInfo(strTmp.c_str(), "realname", szResult[1]); 		 
			hr = S_OK;
		} //end if ((nRow > 0) &&
		CSqliteDBOP::Free_Result(szResult);
	} //end if (m_pOrgDb->Open(strSql.c_str()...
	//
	if (FAILED(hr))
	{
		if (m_pExtraDb)
		{
			szResult = NULL;
			//查找外部联系人
			strSql = "select realname from user where lower(username)=lower('";
			strSql += strTmp;
			strSql += "')";
			if (m_pExtraDb->Open(strSql.c_str(), &szResult, nRow, nCol))
			{
				if ((nRow > 0) && (szResult[1]))
				{
					szName->SetString(szResult[1]); 			
					SetUserInfo(strTmp.c_str(), "realname", szResult[1]); 		 
					hr = S_OK;
				}
				CSqliteDBOP::Free_Result(szResult);
			} //end if (m_pExtraDb->
		} //end if (m_pExtraDb)
	} //end if (FAILED(hr))
	return hr;
}

STDMETHODIMP CEContactsImpl::GetUserDeptPath(const char *szUserName, const char *szDomain, IAnsiString *strDeptPath)
{
	HRESULT hr = E_FAIL;
	std::string strName, strDomain;
	if (szDomain == NULL)
	{ 
		SepNameDomainByUserName(szUserName, strName, strDomain);
	} else
	{
		strName = szUserName;
		strDomain = szDomain;
	} 
	std::string strSql = "select departmentname from user where lower(username)=lower('";
	strSql += strName;
	strSql += "')";
	char **szResult = NULL;
	int nRow, nCol;
	std::string strDeptId;
	if (m_pOrgDb->Open(strSql.c_str(), &szResult, nRow, nCol))
	{
		if ((nRow > 0) && (szResult[1] != NULL))
		{
			strDeptPath->SetString(szResult[1]);
			hr = S_OK;
		}
	}
	CSqliteDBOP::Free_Result(szResult);
	return hr; 
}

//
STDMETHODIMP CEContactsImpl::GetUserNameByNameOrPhone(const char *szInput, IAnsiString *strUserName, IAnsiString *strRealName)
{
	if ((!szInput) || (!m_pOrgDb))
		return E_FAIL;
	HRESULT hr = E_FAIL;
	std::string strSql;
	if (CSystemUtils::IsMobileNumber(szInput))
	{
		strSql = "select username,domain,realname from user where mobile='";
		strSql += szInput;
		strSql += "'";
	} else
	{
		std::string strName, strDomain;
		if (SepNameDomainByUserName(szInput, strName, strDomain))
		{
			strSql = "select username,domain,realname from user where lower(username)=lower('";
			strSql += strName;
			strSql += "')";
		}
	}
	if (!strSql.empty())
	{
		char **szResult = NULL;
		int nRow, nCol;
		if (m_pOrgDb->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			if ((nRow > 0) && szResult[nCol] && szResult[nCol + 1])
			{
				strUserName->SetString(szResult[nCol]);
				strUserName->AppendString("@");
				strUserName->AppendString(szResult[nCol + 1]);
				if (szResult[nCol + 2])
					strRealName->SetString(szResult[nCol + 2]);
				hr = S_OK;
			}
		}
		CSqliteDBOP::Free_Result(szResult);
		szResult = NULL;
		if (FAILED(hr)) //暂时没找到
		{
			//从外部联系人找
			if (CSystemUtils::IsMobileNumber(szInput))
			{
				strSql = "select username,realname from user where mobile='";
				strSql += szInput;
				strSql += "'";
			} else
			{ 
				strSql = "select username,realname from user where lower(username)=lower('";
				strSql += szInput;
				strSql += "')"; 
			} 
			if (m_pExtraDb->Open(strSql.c_str(), &szResult, nRow, nCol))
			{
				if ((nRow > 0) && szResult[1])
				{
					strUserName->SetString(szResult[nCol]);
					if (szResult[nCol + 1])
						strRealName->SetString(szResult[nCol + 1]);
					hr = S_OK;
				} //end if ((nRow > 0) 
			    CSqliteDBOP::Free_Result(szResult);
			} //end if (m_pExtraDb->
		} //end if (FAILED(hr))
	}
	return hr;
}

//
STDMETHODIMP CEContactsImpl::GetPhoneByRealName(const char *szRealName2, const char *szRealName3, IAnsiString *szRealName, IAnsiString *strPhone)
{
	HRESULT hr = E_FAIL;
	if (szRealName2 && m_pOrgDb)
	{
		std::string strSql = "select realName, mobile from user where realname='";
		strSql += szRealName2;
		strSql += "'";
		if (szRealName3)
		{
			strSql += " or realname='";
			strSql += szRealName3;
			strSql += "'";
		}
		char **szResult = NULL;
		int nRow, nCol;
		if (m_pOrgDb->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			if ((nRow > 0) && szResult[2] && szResult[3])
			{
				szRealName->SetString(szResult[2]);
				strPhone->SetString(szResult[3]);
				hr = S_OK;
			} //end if (szResult[1])
		} //end if (m_pOrgDb->...
		CSqliteDBOP::Free_Result(szResult);
 
	}
	return hr;
}

//获取手机号码
STDMETHODIMP CEContactsImpl::GetPhoneByName(const char *szUserName, IAnsiString *strPhone)
{ 
	HRESULT hr = E_FAIL;
	std::string strName, strDomain;
	if (SepNameDomainByUserName(szUserName, strName, strDomain))
	{
		std::string strSql = "select mobile from user where lower(username)=lower('";
		strSql += strName;
		strSql += "')";
		char **szResult = NULL;
	    int nRow, nCol; 
	    if (m_pOrgDb->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			if ((nRow > 0) && szResult[1])
			{
				strPhone->SetString(szResult[1]);
				hr = S_OK;
			} //end if (szResult[1])
		} //end if (m_pOrgDb->...
		CSqliteDBOP::Free_Result(szResult); 
	} else //外部联系人 
	{
		std::string strSql = "select mobile from user where lower(username)=lower('";
		strSql += szUserName;
		strSql += "')";
		char **szResult = NULL;
		int nRow, nCol;
		if (m_pExtraDb->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			if ((nRow > 0) && szResult[1])
			{
				strPhone->SetString(szResult[1]);
				hr = S_OK;
			} //end if ((nRow > 0)
		} //end if (m_pExtraDb->
		CSqliteDBOP::Free_Result(szResult);
	}
	return hr;
}

//获取分机号
STDMETHODIMP CEContactsImpl::GetCellPhoneByName(const char *szUserName, IAnsiString *strCellPhone)
{
	std::string strName, strDomain;
	if (SepNameDomainByUserName(szUserName, strName, strDomain))
	{
		std::string strSql = "select cell from user where lower(username)=lower('";
		strSql += strName;
		strSql += "')";
		char **szResult = NULL;
	    int nRow, nCol;
	    std::string strCurrPid;
		HRESULT hr = E_FAIL;
	    if (m_pOrgDb->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			if ((nRow > 0) && szResult[1])
			{
				strCellPhone->SetString(szResult[1]);
				hr = S_OK;
			} //end if (szResult[1])
		} //end if (m_pOrgDb->...
		CSqliteDBOP::Free_Result(szResult);
		return hr;
	}
	return E_FAIL;
}

//dept list ,隔开
STDMETHODIMP CEContactsImpl::GetDeptListByUserName(const char *szUserName, const char *szDomain,
	                              IAnsiString *strDeptList)
{
	std::string strSql = "select deptid from user where lower(username)=lower('";
	strSql += szUserName;
	//strSql += "' and domain='";
	//strSql += szDomain;
	strSql += "') ";
	char **szResult = NULL;
	int nRow, nCol;
	std::string strCurrPid;
	if (m_pOrgDb->Open(strSql.c_str(), &szResult, nRow, nCol))
	{
		if ((nRow > 0) && szResult[1])
			strCurrPid = szResult[1];
	}
	CSqliteDBOP::Free_Result(szResult);
	std::string strDepts = strCurrPid;
	while (TRUE)
	{		
		//query parent id;
		strSql = "select parentid from dept where id='";
		strSql += strCurrPid;
		strSql += "'";
		if (m_pOrgDb->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			if ((nRow > 0) && szResult[1])
				strCurrPid = szResult[1];
			else 
				strCurrPid = "";
		} else
		{
			strCurrPid = "";
		}

		CSqliteDBOP::Free_Result(szResult);
		 
		if (strCurrPid == "")
			break;
		strDepts += ",";
		strDepts += strCurrPid;
	}
	if (!strDepts.empty())
	{
		strDeptList->SetString(strDepts.c_str());
		return S_OK;
	}
	return E_FAIL;
}

STDMETHODIMP CEContactsImpl::GetUserListByDeptId(const char *szDeptId, IUserList *pUserList, BOOL bSubDeptId, int nType) 
{
	if (bSubDeptId)
	{
		char **szResult = NULL; 
		std::string strSql = "select id from dept where parentid='";
		strSql += szDeptId;
		strSql += "'";
		int nRow, nCol;
		if (m_pOrgDb->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			for (int i = 1; i <= nRow; i ++)
			{
				if (szResult[i * nCol])
				{
					GetUsersByDeptId(szResult[i * nCol], pUserList, nType);
					GetUserListByDeptId(szResult[i * nCol], pUserList, TRUE, nType);
				}
			}
		}
		CSqliteDBOP::Free_Result(szResult);
		return S_OK;
	} else
	{
		if (GetUsersByDeptId(szDeptId, pUserList, nType))
			return S_OK;
	}
	return E_FAIL;
}

BOOL CEContactsImpl::GetDeptsByParentId(const char *szDeptId, IUserList *pList)
{
	char **szResult; 
	std::string strSql = "select id, show, name, roleid from dept where parentid='";
	strSql += szDeptId;
	strSql += "' order by dispseq asc";
	int nRow, nCol;
	int nDeptId = ::atoi(szDeptId);
	if (m_pOrgDb->Open(strSql.c_str(), &szResult, nRow, nCol))
	{
		for (int i = 1; i <= nRow; i ++)
		{				
			if (szResult[i * nCol])
			{
				if (!IsHideDept(szResult[i * nCol], szResult[i * nCol + 3]))
				{
					LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
					memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
					pData->id = ::atoi(szResult[i * nCol]);
					int nDisplayLen = ::strlen(szResult[i * nCol + 2]);
					if (nDisplayLen > 0)
					{
						pData->szDisplayName = new char[nDisplayLen + 1];
						memset(pData->szDisplayName, 0, nDisplayLen + 1);
						strcpy(pData->szDisplayName, szResult[i * nCol + 2]);
					}
					pData->pid = nDeptId;
					if (szResult[i * nCol + 1])
						pData->nDisplaySeq = ::atoi(szResult[i * nCol + 1]);
					pList->AddDeptInfo(pData, FALSE);
				} //end if (!IsHideDept(szResult[i * nCol
			} //end if (szResult[i * 2] && ...
		} //end for (int i = 1; i <= nRow...
	} //end if (m_pOrgDb->Open(strSql.c_str()...
	CSqliteDBOP::Free_Result(szResult);
	return TRUE;
}

	//
void CEContactsImpl::InitAuthList()
{
	m_RoleList.clear();
	char **szResult = NULL;
	int nRow, nCol; 
	if (m_pOrgDb->Open("select roleid,role from role", &szResult, nRow, nCol))
	{
		for (int i = 1; i <= nRow; i ++)
		{
			if (szResult[i * nCol + 1] == NULL)
			{
				m_RoleList.insert(std::pair<std::string, std::string>(szResult[i * nCol], "0"));
			} else
			{
				m_RoleList.insert(std::pair<std::string, std::string>(szResult[i * nCol], szResult[i * nCol + 1]));
			} //end else if (...
		} //end for (..
	} //end if (
}

void CEContactsImpl::InitDeptAuthList()
{
	m_DeptAuthList.clear();
	char **szResult = NULL;
	int nRow, nCol; 
	if (m_pOrgDb->Open("select deptid,auth_list from auth_list", &szResult, nRow, nCol))
	{
		for (int i = 1; i <= nRow; i ++)
		{
			m_DeptAuthList.insert(std::pair<std::string, std::string>(szResult[i * nCol], szResult[i * nCol + 1]));
		}
	}
}

//
BOOL CEContactsImpl::RoleIdHasHideDeptAuth(const char *szRoleId)
{
	if (strcmp(szRoleId, "0") == 0)
		return FALSE;
	std::map<std::string, std::string>::iterator it = m_RoleList.find(szRoleId);
	if (it != m_RoleList.end())
	{
		if (strcmp(it->second.c_str(), "0") == 0)
			return TRUE;
		if (it->second.find("admin/dept/hidden_dept") != std::string::npos)
			return TRUE;
	}
	return FALSE;
}

BOOL CEContactsImpl::IsHideDept(const char *szDeptId, const char *szRoleId)
{
	if ((szRoleId == NULL) || (!RoleIdHasHideDeptAuth(szRoleId)))
	{
		return FALSE;
	} else 
	{ 
		std::map<std::string, std::string>::iterator it = m_DeptAuthList.find(szDeptId);
		if (it != m_DeptAuthList.end())
		{
			const char *p = it->second.c_str();;
			int nIdx = 0;
			std::vector<std::string>::iterator deptit;
			for (deptit = m_DeptList.begin(); deptit != m_DeptList.end(); deptit ++)
			{
				if (strcmp(szDeptId, deptit->c_str()) == 0)
				{
					return FALSE;
				}
			}
			while (TRUE)
			{
				char szTmp[32] = {0};
				if (!CSystemUtils::GetStringBySep(p, szTmp, ',', nIdx))
				    break;
				for (deptit = m_DeptList.begin(); deptit != m_DeptList.end(); deptit ++)
				{
					if (strcmp(szTmp, deptit->c_str()) == 0)
					{
						return FALSE;
					}
				}
				nIdx ++;
			}
		} //end if (
		return TRUE;
	}
}

BOOL CEContactsImpl::GetUsersByDeptId(const char *szDeptId, IUserList *pList, int nType)
{
	if (nType == 0)
	{
		//select user 
		std::string strSql = "select id,show,domain,username,realname, cell from user where deptid='";
		strSql += szDeptId;
		strSql += "'";
		char **szResult = NULL;
		int nRow, nCol;
		int nCurrPid = ::atoi(szDeptId);
		if (m_pOrgDb->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			for (int i = 1; i <= nRow; i ++)
			{				
				if (szResult[i * nCol] && szResult[i * nCol + 2] && szResult[i * nCol + 3])
				{
					LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
				    memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
					pData->id = ::atoi(szResult[i * nCol]);
					strncpy(pData->szUserName, szResult[i * nCol + 3], MAX_USER_NAME_SIZE - 1);
					strcat(pData->szUserName, "@");
					strcat(pData->szUserName, szResult[i * nCol + 2]);
					int nDisplayLen = ::strlen(szResult[i * nCol + 4]);
					pData->szDisplayName = new char[nDisplayLen + 1];
					memset(pData->szDisplayName, 0, nDisplayLen + 1);
					strcpy(pData->szDisplayName, szResult[i * nCol + 4]);
					pData->pid = nCurrPid;
					if (szResult[i * nCol + 1] != NULL)
						pData->nDisplaySeq = ::atoi(szResult[i * nCol + 1]);
					if (szResult[i * nCol + 5])
						strncpy(pData->szCell, szResult[i * nCol + 5], NODE_DATA_CELL_PHONE_SIZE - 1);
					pList->AddUserInfo(pData, FALSE);
				} //end if (szResult[i * 2] && ...
			} //end for (int i = 1; i <= nRow...
		} //end if (m_pOrgDb->Open(strSql.c_str()....
		CSqliteDBOP::Free_Result(szResult); 
	} else if (m_pExtraDb)
	{
		//select user 
		std::string strSql = "select userid,username,realname,tel from user where deptid='";
		strSql += szDeptId;
		strSql += "'";
		char **szResult = NULL;
		int nRow, nCol;
		int nCurrPid = ::atoi(szDeptId);
		if (m_pExtraDb->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			for (int i = 1; i <= nRow; i ++)
			{				
				if (szResult[i * nCol] && szResult[i * nCol + 2] && szResult[i * nCol + 3])
				{
					LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
				    memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
					pData->id = ::atoi(szResult[i * nCol]);
					strncpy(pData->szUserName, szResult[i * nCol + 1], MAX_USER_NAME_SIZE - 1); 
					int nDisplayLen = ::strlen(szResult[i * nCol + 2]);
					pData->szDisplayName = new char[nDisplayLen + 1];
					memset(pData->szDisplayName, 0, nDisplayLen + 1);
					strcpy(pData->szDisplayName, szResult[i * nCol + 2]);
					pData->pid = nCurrPid; 
					if (szResult[i * nCol + 5])
						strncpy(pData->szCell, szResult[i * nCol + 3], NODE_DATA_CELL_PHONE_SIZE - 1);
					pList->AddUserInfo(pData, FALSE);
				} //end if (szResult[i * 2] && ...
			} //end for (int i = 1; i <= nRow...
		} //end if (m_pOrgDb->Open(strSql.c_str()....
		CSqliteDBOP::Free_Result(szResult); 
	}
	return TRUE;
}

//
STDMETHODIMP CEContactsImpl::GetChildListByDeptId(const char *szDeptId, IUserList *pUserList, int nType)
{
	if ((!m_pOrgDb) || (!pUserList) || (!szDeptId))
		return E_FAIL;
	pUserList->SetDeptId(szDeptId);
	GetUsersByDeptId(szDeptId, pUserList, nType);
	GetDeptsByParentId(szDeptId, pUserList);
	return S_OK;
}



BOOL CEContactsImpl::InitContactDb()
{
	m_pOrgDb = new CSqliteDBOP(m_strOrgLocalName.c_str(), "");
	return TRUE;
}

//检测组织结构文件是否合法
BOOL CEContactsImpl::CheckContactValid()
{
	//
	BOOL bRet = FALSE;
	CSqliteDBOP *p = new CSqliteDBOP(m_strOrgLocalName.c_str(), "");
	char **szResult = NULL;
	int nRow, nCol;
	if (p->Open("select count(id) from user", &szResult, nRow, nCol))
	{
		if (nRow > 0)
		{
			if (::atoi(szResult[1]) > 0)
				bRet = TRUE;
		}
	} //end if (p->Open(...
	CSqliteDBOP::Free_Result(szResult);
	delete p;
	return bRet;
}

//加载联系人至界面
STDMETHODIMP CEContactsImpl::LoadContacts()
{
	if (m_pOrgDb)
		delete m_pOrgDb;
	if (!InitContactDb())
		return E_FAIL;
	if (!m_pCore)
		return E_POINTER;
	InitDeptAuthList();
	InitAuthList();
	CInterfaceAnsiString strUserName;
	CInterfaceAnsiString strDomain, strRealName;
	m_pCore->GetUserName((IAnsiString *)&strUserName);
	m_pCore->GetUserDomain((IAnsiString *)&strDomain);

	//copy username
	m_strUserName = strUserName.GetData();
	m_strUserName += "@";
	m_strUserName += strDomain.GetData();
	m_nSelfCorpId = GetUserCorpId(m_strUserName.c_str(), m_nUserSign);
	return S_OK;
}

//get contact user info
STDMETHODIMP CEContactsImpl::GetContactUserInfo(const char *szUserName, IInstantUserInfo *pInfo)
{
	if (szUserName)
	{
		CGuardLock::COwnerLock guard(m_InfoLock);
		std::map<CAnsiString_, CInstantUserInfo *>::iterator it = m_UserInfos.find(szUserName);
		if (it != m_UserInfos.end())
		{
			it->second->AssignTo(pInfo);
			return S_OK;
		}
	}
	return E_FAIL;
}

//xml <i u="user@doamin"/><i u="user2@doamin"/>....
STDMETHODIMP CEContactsImpl::AddOrderUserList(const char *szXml)
{
	TiXmlDocument xml;
	if (xml.Load(szXml, (int) ::strlen(szXml)))
	{
		TiXmlElement *pNode = xml.FirstChildElement();
		if (OrderStatus(pNode))
			return S_OK;
	}
	return E_FAIL;
}

//xml <i u="user@doamin"/><i u="user2@doamin"/>
STDMETHODIMP CEContactsImpl::DeleteOrderUserList(const char *szXml)
{
	TiXmlDocument xml;
	if (xml.Load(szXml, (int) ::strlen(szXml)))
	{
		TiXmlElement *pNode = xml.FirstChildElement();
		if (DeleteStatus(pNode))
			return S_OK;
	}
	return E_FAIL;
}

void CEContactsImpl::SetUserStatus(const char *szUserName, const char *szStatus, const char *szMemo, BOOL bOrder)
{
	CGuardLock::COwnerLock guard(m_InfoLock);
	std::map<CAnsiString_, CInstantUserInfo *>::iterator it = m_UserInfos.find(szUserName);
    CInstantUserInfo *pInfo = NULL;
	if (it != m_UserInfos.end())
	{
		pInfo = it->second;
	} else
	{
		pInfo = new CInstantUserInfo();
		m_UserInfos.insert(std::pair<CAnsiString_, CInstantUserInfo *>(szUserName, pInfo));
		//
		CInterfaceAnsiString szValue;
  
		GetRealNameById(szUserName, NULL, &szValue);
		
	}
	if (pInfo->SetUserStatus(szStatus) == S_OK)
	{
		 if (m_pCore)
			 m_pCore->DoPresenceChanged(szUserName, szStatus, szMemo, bOrder);
	}	 
}

STDMETHODIMP CEContactsImpl::SetContactUserInfo(const char *szUserName, const char *szParam, const char *szValue)
{
	SetUserInfo(szUserName, szParam, szValue);
	return S_OK;
}

void CEContactsImpl::SetUserInfo(const char *szUserName, const char *szParam, const char *szValue)
{
	CGuardLock::COwnerLock guard(m_InfoLock);
	std::map<CAnsiString_, CInstantUserInfo *>::iterator it = m_UserInfos.find(szUserName);
	CInstantUserInfo *pInfo = NULL;
	if (it != m_UserInfos.end())
	{
		pInfo = it->second;
	} else
	{
		pInfo = new CInstantUserInfo();
		m_UserInfos.insert(std::pair<CAnsiString_, CInstantUserInfo *>(szUserName, pInfo));
		//
		CInterfaceAnsiString szValue;
		std::string strName, strDomain;
		if (szUserName)
		{
			strName = szUserName;
			int nPos = strName.find('@');
			if (nPos != std::string::npos)
			{
				strDomain = strName.substr(nPos + 1);
				strName = strName.substr(0, nPos);
			} 
			if (SUCCEEDED(GetRealNameById(strName.c_str(), strDomain.c_str(), &szValue)))
				pInfo->SetUserInfo("realname", szValue.GetData());
		}
	}
	pInfo->SetUserInfo(szParam, szValue);  
}

void CEContactsImpl::ClearUserInfos()
{
	CGuardLock::COwnerLock guard(m_InfoLock);
	std::map<CAnsiString_, CInstantUserInfo *>::iterator it;
	for (it = m_UserInfos.begin(); it != m_UserInfos.end(); it ++)
	{
		delete it->second;
	}
	m_UserInfos.clear();
}

BOOL CEContactsImpl::DownloadOrg(const char *szUrl)
{
	if (m_pCore)
	{
		HWND hLogon = NULL;
		IUIManager *pUI = NULL;
		m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI);
		if (pUI)
		{
			pUI->OrderWindowMessage("LogonWindow", NULL, WM_HTTPDLPROGRESS, (ICoreEvent *)this);
			pUI->GetWindowHWNDByName("LogonWindow", &hLogon);
			std::string strUrl;
 
			char szDomain[64] = {0};
            IConfigure *pCfg = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
			{
				CInterfaceAnsiString strOrgSvr, strOrgPort;
				if ((SUCCEEDED(pCfg->GetParamValue(TRUE, "Server", "orghost", &strOrgSvr)))
					&& (SUCCEEDED(pCfg->GetParamValue(TRUE, "Server", "orghostport", &strOrgPort))))
				{
					strUrl = strOrgSvr.GetData();
					strUrl += ":";
					strUrl += strOrgPort.GetData();
				} else
				{
					CInterfaceAnsiString strHttpSvr;
					if (SUCCEEDED(pCfg->GetServerAddr(HTTP_SVR_URL_HTTP, &strHttpSvr)))
					{
						strUrl = strHttpSvr.GetData();
					}
				}
				pCfg->Release();
			}
            CInterfaceAnsiString asTmp;
			int nSize = 63;
			m_pCore->GetSvrParams("logindomain", (IAnsiString *)&asTmp, FALSE);
			asTmp.GetString(szDomain, &nSize);
			if (!strUrl.empty())
			{
				strUrl += ORG_DOWNLOAD_PATH;
				strUrl += szDomain;
				strUrl += "/";
				strUrl += szUrl;

				m_bDlOrgSucc = FALSE;
#ifdef PRINT_RUN_TIME_INTERVAL
				DWORD dwStart= ::GetTickCount();
#endif
				//通知界面
				ICoreLogin *pLogin = NULL;
				if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(ICoreLogin), (void **)&pLogin)))
				{
					ICoreEvent *pEvent = NULL;
					if (SUCCEEDED(pLogin->QueryInterface(__uuidof(ICoreEvent), (void **)&pEvent)))
					{
						pEvent->DoBroadcastMessage("downorg", NULL, "progress", NULL, NULL);
						pEvent->Release();
					}
					pLogin->Release();
				} //end if (SUCCEEDED(p

				::P2SvrAddDlTask(strUrl.c_str(), m_strOrgLocalName.c_str(), FILE_TYPE_ORGFILE, this, EcontactsHttpCallBack, TRUE);
				PRINTDEBUGLOG(dtInfo, "Download contact url:%s", strUrl.c_str());
				//CHttpUpDownload::DownloadFile(strUrl.c_str(), m_strOrgLocalName.c_str(), hLogon);
#ifdef PRINT_RUN_TIME_INTERVAL
				DWORD  dwInterval = ::GetTickCount() - dwStart;
				PRINTDEBUGLOG(dtInfo, "Download Contacts List Time:%d ", dwInterval);
#endif							 
			} //end if (strlen(szSvrIp) >= sizeof("0.0.0.0"))			
			pUI->Release();
		} //end if (pUI
		return TRUE;
	} //end if (m_pCore)
	return FALSE;
}

BOOL CEContactsImpl::FreeTreeNodeData(CTreeNodeType nodeType, void **pData)
{
	if (pData && (*pData))
	{
		LPORG_TREE_NODE_DATA pNode = (LPORG_TREE_NODE_DATA)(*pData);
		delete pNode;
		*pData = NULL;
		return TRUE;
	}
	return FALSE;
}

BOOL CEContactsImpl::GetUserSignVer(const char *szUserName, std::string &strVer)
{
	strVer = "0";
	return TRUE;
}

STDMETHODIMP CEContactsImpl::OrderAllStatusFromSvr()
{
	if (m_pCore)
	{
		std::string strXml = "<sta type=\"order\"><add>";
		std::map<CAnsiString_, CInstantUserInfo *>::iterator it;
		std::string strVer;
		CGuardLock::COwnerLock gaurd(m_InfoLock);
		for (it = m_UserInfos.begin(); it != m_UserInfos.end(); it ++)
		{
			if (it->second->GetOrderRef() > 0)
			{
				strXml += "<i u=\"";
				strXml += it->first.c_str();
				strXml += "\" signver=\"";
				GetUserSignVer(it->first.c_str(), strVer);
				strXml += strVer;
				strXml += "\"/>"; 
			}
		}
		strXml += "</add></sta>"; 
		
		//分解一下xml
		if (strXml.size() > 4096)
		{
			TiXmlDocument paser;
			if (paser.Load(strXml.c_str(), strXml.size()))
			{
				TiXmlElement *pRoot = paser.RootElement();
				TiXmlElement *pChild = pRoot->FirstChildElement(); //sta
				if (pChild)
					pChild = pChild->FirstChildElement();
				if (pChild)
				{
					TiXmlString strTmp;
					std::string strNew;
					while(TRUE)
					{
						strNew = "<sta type=\"order\"><add>";
						while (pChild)
						{
							if (strNew.size() > 4096)
								break;
							strTmp.clear();
							pChild->SaveToString(strTmp, 0);
							strNew += strTmp.c_str();
							pChild = pChild->NextSiblingElement();
						}
						strNew += "</add></sta>";
						m_pCore->SendRawMessage((BYTE *)strNew.c_str(), (int) strNew.size(), 0);
						if (pChild == NULL)
							break;
					} //end while
				} //end if (pChild)
			} //end if (paser.
		} else
		{
			return (SUCCEEDED(m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (int) strXml.size(), 0)));
		} 
	}
	return E_FAIL;
}

BOOL CEContactsImpl::OrderStatus(TiXmlElement *pNode)
{ 
	std::string strXml;
	strXml = "<sta type=\"order\"><add>";
	TiXmlElement *pChild = pNode;
	CGuardLock::COwnerLock gurad(m_InfoLock);
	std::map<CAnsiString_, CInstantUserInfo *>::iterator it;
	std::string strVer;		
	CInstantUserInfo *pItem;
	BOOL bUsed = FALSE;
	while (pChild)
	{ 
		const char *szUserName = pChild->Attribute("u");
		if (szUserName)
		{
			it = m_UserInfos.find(szUserName);
			if (it != m_UserInfos.end())
			{
				pItem = it->second;
			} else
			{
				pItem = new CInstantUserInfo();
				m_UserInfos.insert(std::pair<CAnsiString_, CInstantUserInfo *>(szUserName, pItem));
				//
				CInterfaceAnsiString szValue;
				std::string strName, strDomain;
				if (szUserName)
				{
					strName = szUserName;
					int nPos = strName.find('@');
					if (nPos != std::string::npos)
					{
						strDomain = strName.substr(nPos + 1);
						strName = strName.substr(0, nPos);
					} 
					if (SUCCEEDED(GetRealNameById(strName.c_str(), strDomain.c_str(), &szValue)))
						pItem->SetUserInfo("realname", szValue.GetData());
				}
			}
			if (pItem->AddOrderRef() == 1)
			{
				strXml += "<i u=\"";
				strXml += szUserName;
				strXml += "\" signver=\"";
				GetUserSignVer(szUserName, strVer);
				strXml += strVer;
				strXml += "\"/>";
				bUsed = TRUE;
			} 	
		} //end if (szUserName);
		pChild = pChild->NextSiblingElement();
	} // end while(..
	strXml += "</add></sta>";
	if (m_pCore && bUsed)
	{
		//分解一下xml
		if (strXml.size() > 4096)
		{
			TiXmlDocument paser;
			if (paser.Load(strXml.c_str(), strXml.size()))
			{
				TiXmlElement *pRoot = paser.RootElement();
				TiXmlElement *pChild = pRoot->FirstChildElement(); //sta
				if (pChild)
					pChild = pChild->FirstChildElement();
				if (pChild)
				{
					TiXmlString strTmp;
					std::string strNew;
					while(TRUE)
					{
						strNew = "<sta type=\"order\"><add>";
						while (pChild)
						{
							if (strNew.size() > 4096)
								break;
							strTmp.clear();
							pChild->SaveToString(strTmp, 0);
							strNew += strTmp.c_str();
							pChild = pChild->NextSiblingElement();
						}
						strNew += "</add></sta>";
						m_pCore->SendRawMessage((BYTE *)strNew.c_str(), (int) strNew.size(), 0);
						if (pChild == NULL)
							break;
					} //end while
				} //end if (pChild)
			} //end if (paser.
		} else
		{
			return (SUCCEEDED(m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (int) strXml.size(), 0)));
		}
	}
	return FALSE;
}

BOOL CEContactsImpl::DeleteStatus(TiXmlElement *pNode)
{
	std::string strXml;
	strXml = "<sta type=\"order\"><delete>";
	TiXmlElement *pChild = pNode;
	CGuardLock::COwnerLock gurad(m_InfoLock);
	std::map<CAnsiString_, CInstantUserInfo *>::iterator it;
	std::string strVer;		
	CInstantUserInfo *pItem;
	BOOL bUsed = FALSE;
	while (pChild)
	{ 
		const char *szUserName = pChild->Attribute("u");
		if (szUserName)
		{
			it = m_UserInfos.find(szUserName);
			if (it != m_UserInfos.end())
			{
				pItem = it->second;
			} else
			{
				pItem = new CInstantUserInfo();
				m_UserInfos.insert(std::pair<CAnsiString_, CInstantUserInfo *>(szUserName, pItem));
			}
			if (pItem->ReleaseOrderRef() == 0)
			{
				strXml += "<i u=\"";
				strXml += szUserName; 
				strXml += "\"/>";
				bUsed = TRUE;
			} 
		} //end if (szUserName);
		pChild = pChild->NextSiblingElement();
	} // end while(..
	strXml += "</delete></sta>";
	if (m_pCore && bUsed)
	{
		//分解一下xml
		if (strXml.size() > 4096)
		{
			TiXmlDocument paser;
			if (paser.Load(strXml.c_str(), strXml.size()))
			{
				TiXmlElement *pRoot = paser.RootElement();
				TiXmlElement *pChild = pRoot->FirstChildElement(); //sta
				if (pChild)
					pChild = pChild->FirstChildElement();
				if (pChild)
				{
					TiXmlString strTmp;
					std::string strNew;
					while(TRUE)
					{
						strNew = "<sta type=\"order\"><delete>";
						while (pChild)
						{
							if (strNew.size() > 4096)
								break;
							strTmp.clear();
							pChild->SaveToString(strTmp, 0);
							strNew += strTmp.c_str();
							pChild = pChild->NextSiblingElement();
						}
						strNew += "</delete></sta>";
						m_pCore->SendRawMessage((BYTE *)strNew.c_str(), (int) strNew.size(), 0);
						if (pChild == NULL)
							break;
					} //end while
				} //end if (pChild)
			} //end if (paser.
		} else
		{
			return (SUCCEEDED(m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (int) strXml.size(), 0)));
		}
	}
	return FALSE;
}

//
void CEContactsImpl::ClearMemoryUsers()
{
	if (m_pExtraDb)
	{ 
		m_pExtraDb->Execute("delete from dept");
		m_pExtraDb->Execute("delete from user");
	}
}

//是否为认证联系人
STDMETHODIMP_(BOOL) CEContactsImpl::IsAuthContact(const char *szUserName)
{ 
	if (IsExistsExtraUsers(szUserName, 1) > 0)
		return TRUE;  
	int nSign = 0;
	int nCorpId = GetUserCorpId(szUserName, nSign);
	if (nCorpId != 0)
	{
		if (nCorpId != m_nSelfCorpId)
			return FALSE;
		else
		{
			if (nSign > m_nUserSign)
				return FALSE;
		}
	} //end if (
	return TRUE;
}

STDMETHODIMP CEContactsImpl::LoadMenuFromExtractDept(HWND hWnd, const TCHAR *szParentMenu, const char *szParentId, int nType)
{
	if (m_pExtraDb)
	{
		//插入部门
		std::string strSql = "select id,name,dispseq from dept where parentid=";
		strSql += szParentId;
		char **szResult = NULL;
		int nRow, nCol;
		UINT uMenuParentId = 2; //暂时，以后改进 //::atoi(szParentId);
		if (uMenuParentId != 0)
			uMenuParentId |= 0x80000000;
		std::map<std::string, std::string> TmpDeptList; //临时保存的部门列表，插入子节点时使用 
 		if (m_pExtraDb->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			TCHAR szTmp[MAX_PATH] = {0};
			UINT uMenuId = 0;
			for (int i = 1; i <= nRow; i ++)
			{
				if (::strlen(szResult[i * nCol + 1]) > 0)
				{
					memset(szTmp, 0, sizeof(TCHAR) * MAX_PATH);
					CStringConversion::UTF8ToWideChar(szResult[i * nCol + 1], szTmp, MAX_PATH - 1);
					uMenuId = ::atoi(szResult[i * nCol]);
					uMenuId |= 0x80000000;
					::SkinMenuAppendItem(hWnd, szParentMenu, uMenuParentId, szTmp, uMenuId);
					TmpDeptList.insert(std::pair<std::string, std::string>(szResult[i * nCol], szResult[i * nCol]));  
				} //end if (nDisplayLen > 0)
			} //end for (int i = 1;
			
		} //end if (m_pExtraDb->Open
		CSqliteDBOP::Free_Result(szResult);
		szResult = NULL;
		 

		//插入子部门
		std::map<std::string, std::string>::iterator it;
		for (it = TmpDeptList.begin(); it != TmpDeptList.end(); it ++)
		{
			//暂时不支持
			//LoadMenuFromExtractDept(hWnd, szParentMenu, it->second.c_str(), nType);
		} //end for (it = 
		return S_OK;
	} //end if (m_pExtraDb
	return E_FAIL;
}

void CEContactsImpl::LoadContactsByMemory(HWND hWnd, const TCHAR *szTreeName, void *pParentNode, const char *szParentId, 
	               BOOL bOrder, BOOL bInitPresence, int nType)
{
	if (m_pExtraDb)
	{
		//插入部门
		std::string strSql = "select id,name,dispseq from dept where parentid=";
		strSql += szParentId;
		char **szResult = NULL;
		int nRow, nCol;
		std::map<std::string, void *> TmpDeptList; //临时保存的部门列表，插入子节点时使用
		LPORG_TREE_NODE_DATA pData;
		char szType[16] = {0};
		::itoa(nType, szType, 10);
		if (m_pExtraDb->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			for (int i = 1; i <= nRow; i ++)
			{
				int nDisplayLen = ::strlen(szResult[i * nCol + 1]); 
				if (nDisplayLen > 0)
				{ 
					pData = new ORG_TREE_NODE_DATA();
					memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
					pData->id = ::atoi(szResult[i * nCol]);
					pData->szDisplayName = new char[nDisplayLen + 1];
					memset(pData->szDisplayName, 0, nDisplayLen + 1);
					strcpy(pData->szDisplayName, szResult[i * nCol + 1]);
					strcpy(pData->szUserName, szType);
					pData->pid = ::atoi(szParentId); 
					pData->nDisplaySeq = ::atoi(szResult[i * nCol + 2]);
					pData->bOpened = TRUE; 
					TCHAR *szDispText = new TCHAR[nDisplayLen + 1];
					memset(szDispText, 0, sizeof(TCHAR) * (nDisplayLen + 1));
					CStringConversion::UTF8ToWideChar(szResult[i * nCol + 1], szDispText, nDisplayLen);
					void *pNode = ::SkinAddTreeChildNode(hWnd, szTreeName, pData->id, pParentNode,  szDispText, 
				               TREENODE_TYPE_GROUP, pData, NULL, NULL, NULL);

					TmpDeptList.insert(std::pair<std::string, void *>(szResult[i * nCol], pNode));
					delete []szDispText;

				} //end if (nDisplayLen > 0)
			} //end for (int i = 1;
			
		} //end if (m_pExtraDb->Open
		CSqliteDBOP::Free_Result(szResult);
		szResult = NULL;
		//插入人员
		strSql = "select userid,username,realname from user where deptid=";
		strSql += szParentId;
		if (m_pExtraDb->Open(strSql.c_str(), &szResult, nRow, nCol))
		{ 
			std::string strXml;
			CInstantUserInfo Info;
			CInterfaceAnsiString strSign, strStatus;
			for (int i = 1; i <= nRow; i ++)
			{ 
				int nDisplayLen = ::strlen(szResult[i * nCol + 2]); 
				if (nDisplayLen > 0)
				{ 
					pData = new ORG_TREE_NODE_DATA();
					pData->id = ::atoi(szResult[i * nCol]);
					strncpy(pData->szUserName, szResult[i * nCol + 1], MAX_USER_NAME_SIZE - 1);
					pData->szDisplayName = new char[nDisplayLen + 1];
					memset(pData->szDisplayName, 0, nDisplayLen + 1);
					strcpy(pData->szDisplayName, szResult[i * nCol + 2]);
				 
					pData->pid = ::atoi(szParentId);  
					TCHAR *szDispText = new TCHAR[nDisplayLen + 1];
					memset(szDispText, 0, sizeof(TCHAR) * (nDisplayLen + 1));
					CStringConversion::UTF8ToWideChar(szResult[i * nCol + 2], szDispText, nDisplayLen);
					void *pNode = ::SkinAddTreeChildNode(hWnd, szTreeName, pData->id, pParentNode,  szDispText, 
				               TREENODE_TYPE_LEAF, pData, NULL, NULL, NULL);
					if (bInitPresence)
					{
						if (SUCCEEDED(GetContactUserInfo(pData->szUserName, (IInstantUserInfo *)&Info)))
						{
							if (SUCCEEDED(Info.GetUserStatus((IAnsiString *)&strStatus)) && (strStatus.GetSize() > 0))
								::SkinUpdateUserStatusToNode(hWnd, szTreeName, pData->szUserName, strStatus.GetData(), FALSE);
							if (SUCCEEDED(Info.GetUserInfo("sign", (IAnsiString *)&strSign)) && (strStatus.GetSize() > 0))
								::SkinUpdateUserLabelToNode(hWnd, szTreeName, pData->szUserName, strSign.GetData(), FALSE);
						}
					}
					delete []szDispText;
										
					//
					if (bOrder)
					{
						strXml += "<i u=\"";
						strXml += pData->szUserName;
						strXml += "\"/>";
					}
				} //end if (nDisplayLen > 0)
			} //end for (int i = 1;
			if (!strXml.empty())
				AddOrderUserList(strXml.c_str());
		} //end if (m_pExtraDb->Open(...
		CSqliteDBOP::Free_Result(szResult);

		//插入子部门
		std::map<std::string, void *>::iterator it;
		for (it = TmpDeptList.begin(); it != TmpDeptList.end(); it ++)
		{
			LoadContactsByMemory(hWnd, szTreeName, it->second, it->first.c_str(), bOrder, bInitPresence, nType);
		}
	}
}

void CEContactsImpl::LoadContactsByDepts(HWND hWnd, const TCHAR *szTreeName, void *pNode, const char *szDepts, BOOL bOrder)
{ 
	std::vector<std::string> vcDeptList;
	int nIdx = 0;
	while (TRUE)
	{
		char szTmp[32] = {0};
		if (!CSystemUtils::GetStringBySep(szDepts, szTmp, ',', nIdx))
			break;
		vcDeptList.push_back(szTmp);
		nIdx ++;
	}

	void *pParentNode = pNode;
	void *pSaveNode = NULL;
	int nSaveNodeId = 0;
	while (!vcDeptList.empty())
	{
		std::string strDeptId = vcDeptList.back();
		CInterfaceUserList UserList;
		
		vcDeptList.pop_back();
		if (SUCCEEDED(GetChildListByDeptId(strDeptId.c_str(), (IUserList *)&UserList, 0)))
		{
			if (!vcDeptList.empty()) 
				nSaveNodeId = ::atoi(vcDeptList.back().c_str());
			else
				nSaveNodeId = 0;
			if (!DataListDrawToUI(hWnd, szTreeName, (IUserList *)&UserList, pParentNode, nSaveNodeId,
				&pSaveNode, bOrder))
				break;
			pParentNode = pSaveNode;
		} //end 
	} //end while 
	//draw realname to ui
	::SkinExpandTree(hWnd, szTreeName, NULL, TRUE, TRUE);
	::SkinUpdateControlUI(hWnd, szTreeName);
}

BOOL CEContactsImpl::DataListDrawToUI(HWND hWnd, const TCHAR *szTreeName, IUserList  *pDataList, void *pParentNode, 
	                                  int nSaveNodeId, void **pSaveNode, BOOL bOrder)
{
	TCHAR szwDisplayText[512] = {0};
	LPORG_TREE_NODE_DATA pData = NULL;
	std::vector<LPORG_TREE_NODE_DATA>::iterator DataIt;
	while (SUCCEEDED(pDataList->PopFrontDeptInfo(&pData)))
	{ 

		memset(szwDisplayText, 0, sizeof(TCHAR) * 512);
		CStringConversion::UTF8ToWideChar(pData->szDisplayName, szwDisplayText, 511);
		if (pData->id == nSaveNodeId)
		{
			pData->bOpened = TRUE;
			if (pSaveNode)
				*pSaveNode = ::SkinAddTreeChildNode(hWnd, szTreeName, pData->id, pParentNode,  szwDisplayText, 
			               TREENODE_TYPE_GROUP, pData, NULL, NULL, NULL);
			else
				::SkinAddTreeChildNode(hWnd, szTreeName, pData->id, pParentNode,  szwDisplayText, 
			               TREENODE_TYPE_GROUP, pData, NULL, NULL, NULL);
		} else
			::SkinAddTreeChildNode(hWnd, szTreeName, pData->id, pParentNode,  szwDisplayText, 
			               TREENODE_TYPE_GROUP, pData, NULL, NULL, NULL);
	}
	std::string strXml;
	//add user node
	TCHAR szwExtra[NODE_DATA_CELL_PHONE_SIZE] = {0};
	TCHAR szImageFile[MAX_PATH] = {0};
	CInterfaceAnsiString strTmp;
	while (SUCCEEDED(pDataList->PopBackUserInfo(&pData)))
	{
		memset(szwDisplayText, 0, sizeof(TCHAR) * 512);
		memset(szwExtra, 0, sizeof(TCHAR) * NODE_DATA_CELL_PHONE_SIZE);
		CStringConversion::UTF8ToWideChar(pData->szDisplayName, szwDisplayText, 511);
		CStringConversion::StringToWideChar(pData->szCell, szwExtra, NODE_DATA_CELL_PHONE_SIZE - 1);
		if (SUCCEEDED(GetContactHead(pData->szUserName, &strTmp, FALSE)))
		{
			memset(szImageFile, 0, sizeof(TCHAR) * MAX_PATH);
			CStringConversion::StringToWideChar(strTmp.GetData(), szImageFile, MAX_PATH - 1);
			::SkinAddTreeChildNode(hWnd, szTreeName, pData->id, pParentNode, szwDisplayText,
			               TREENODE_TYPE_LEAF, pData, NULL, szImageFile, szwExtra);
		} else
			::SkinAddTreeChildNode(hWnd, szTreeName, pData->id, pParentNode, szwDisplayText,
			               TREENODE_TYPE_LEAF, pData, NULL, NULL, szwExtra);	
		if (bOrder)
		{
			strXml += "<i u=\"";
			strXml += pData->szUserName;
			strXml += "\"/>";
		}
	}
	if (!strXml.empty())
		AddOrderUserList(strXml.c_str());
	return TRUE;
}

//
STDMETHODIMP CEContactsImpl::DrawContactToUI(HWND hWnd, const TCHAR *szTreeName, const char *szUserName, void *pParentNode,
	                              BOOL bOrder, BOOL bInitPresence, int nType)
{
#ifdef PRINT_RUN_TIME_INTERVAL
	DWORD dwStart = ::GetTickCount();
#endif

	if (!m_pCore)
		return E_FAIL;
	std::string strName, strDomain;
	if (szUserName)
	{
		strName = szUserName;
		int nPos = strName.find('@');
		if (nPos != std::string::npos)
		{
			strDomain = strName.substr(nPos + 1);
			strName = strName.substr(0, nPos);
		} 
	}
	if (nType == 0)
	{
	    CInterfaceAnsiString strDepts;
		m_DeptList.clear();
		if ((!strName.empty()) && (!strDomain.empty()))
		{
			 GetDeptListByUserName(strName.c_str(), strDomain.c_str(),
			   (IAnsiString *)&strDepts);
			 const char *p = strDepts.GetData();
			 int nIdx = 0;
			 while (TRUE)
			 {
				 char szTmp[32] = {0};
				 if (!CSystemUtils::GetStringBySep(p, szTmp, ',', nIdx))
				     break;
				 m_DeptList.push_back(szTmp);
				 nIdx ++;
			 }
		}
		if (strDepts.GetSize() == 0)
			strDepts.SetString("0"); 
		LoadContactsByDepts(hWnd, szTreeName, pParentNode, strDepts.GetData(), bOrder);
	} else
	{
		LoadContactsByMemory(hWnd, szTreeName, pParentNode, "0", bOrder, bInitPresence, nType);
	}
#ifdef PRINT_RUN_TIME_INTERVAL
	PRINTDEBUGLOG(dtInfo, "Load Contacts Time:%d", GetTickCount() - dwStart);
#endif
	return S_OK;
	 
}

//
STDMETHODIMP CEContactsImpl::ExpandTreeNodeToUI(HWND hWnd, const TCHAR *szTreeName,
	                                         void *pParentNode, const int nPid)
{
	CInterfaceUserList UserList;
	char szTmp[16] = {0};
	::itoa(nPid, szTmp, 10);
	if (SUCCEEDED(GetChildListByDeptId(szTmp, (IUserList *)&UserList, 0)))
	{
		DataListDrawToUI(hWnd, szTreeName, &UserList, pParentNode, 0, NULL, FALSE);
		::SkinExpandTree(hWnd, szTreeName, pParentNode, TRUE, FALSE);
		::SkinUpdateControlUI(hWnd, szTreeName);
		return S_OK;;
	}
 
	return E_FAIL;
}
 
//
STDMETHODIMP CEContactsImpl::AddExtractDept(const char *szId, const char *szDeptName, const char *szDispSeq,
	                                        const char *szParentId, int nType)
{
	if (m_pExtraDb && szId && szDeptName && szDispSeq && szParentId)
	{
		std::string strSql = "insert into dept(id,name,dispseq,parentid,depttype,nameidx) values('";
		strSql += szId;
		strSql += "','";
		strSql += szDeptName;
		strSql += "','";
		strSql += szDispSeq;
		strSql += "','";
		strSql += szParentId;
		strSql += "','";
		char szType[16] = {0};
		::itoa(nType, szType, 10);
		strSql += szType;
		strSql += "','";
		int nSize = ::strlen(szDeptName) + 1;
		TCHAR *szText = new TCHAR[nSize];
		memset(szText, 0, sizeof(TCHAR) * nSize);
		CStringConversion::UTF8ToWideChar(szDeptName, szText, nSize - 1);
		std::string strPy;
		CPinyinTrans::GetPinyinByText(szText, &strPy, NULL);
		delete []szText;
		strSql += strPy;
		strSql += "')";
		if (m_pExtraDb->Execute(strSql.c_str()))
			return S_OK;
	}
	return E_FAIL;
}

//
STDMETHODIMP CEContactsImpl::AddExtractUser(const char *szId, const char *szUserName, 
	const char *szRealName, const char *szDeptId, const char *szMobile, const char *szTel,
	const char *szEmail, const char *szFax, int nType)
{
	if (m_pExtraDb && szUserName && szRealName && szDeptId && szId)
	{
		if (IsExistsExtraUsers(szUserName, nType))
		{
			//update
			std::string strSql = "update user set realname ='";
			strSql += szRealName;
			strSql += "',deptid='";
			strSql += szDeptId;
			strSql += "',realnameindex='";
			int nSize = ::strlen(szRealName) + 1;
			TCHAR *szText = new TCHAR[nSize];
			memset(szText, 0, sizeof(TCHAR) * nSize);
			CStringConversion::UTF8ToWideChar(szRealName, szText, nSize - 1);
			std::string strPy, strFullPy;
			CPinyinTrans::GetPinyinByText(szText, &strPy, &strFullPy);
			strSql += strPy;
			strSql += "', fullpinyin='";
			strSql += strFullPy;
			strSql += "',mobile='";
			if (szMobile)
				strSql += szMobile;
			strSql += "',tel='";
			if (szTel)
				strSql += szTel;
			strSql += "',email='";
			if (szEmail)
				strSql += szEmail;
			strSql += "',fax='";
			if (szFax)
				strSql += szFax;
			strSql += "' where username='";
			strSql += szUserName;
			strSql += "' and usertype='";
			char szTmp[16] = {0};
			::itoa(nType, szTmp, 10); 
			strSql += szTmp;
			strSql += "'";
			delete []szText;
			if (m_pExtraDb->Execute(strSql.c_str()))
				return S_OK;
		} else
		{
			//insert 
			std::string strSql = "insert into user(userid,username,realname,deptid,realnameindex,fullpinyin,mobile,tel,email,fax,usertype) values('";
			strSql += szId;
			strSql += "','";
			strSql += szUserName;
			strSql += "','";
			strSql += szRealName;
			strSql += "','";
			strSql += szDeptId;
			strSql += "','";
			int nSize = ::strlen(szRealName) + 1;
			TCHAR *szText = new TCHAR[nSize];
			memset(szText, 0, sizeof(TCHAR) * nSize);
			CStringConversion::UTF8ToWideChar(szRealName, szText, nSize - 1);
			std::string strPy, strFullPy;
			CPinyinTrans::GetPinyinByText(szText, &strPy, &strFullPy);
			strSql += strPy;
			strSql += "','";
			strSql += strFullPy;
			strSql += "','";
			if (szMobile)
				strSql += szMobile;
			strSql += "','";
			if (szTel)
				strSql += szTel;
			strSql += "','";
			if (szEmail)
				strSql += szEmail;
			strSql += "','";
			if (szFax)
				strSql += szFax;
			strSql += "',";
			char szTmp[16] = {0};
			::itoa(nType, szTmp, 10); 
			strSql += szTmp;
			strSql += ")";
			delete []szText;
			if (m_pExtraDb->Execute(strSql.c_str()))
				return S_OK;
		} //end else if (
	}
	return E_FAIL;
}

// 
STDMETHODIMP CEContactsImpl::DeleteExtractDept(const char *szId, int nType)
{
	if (m_pExtraDb)
	{
		std::string strSql = "delete from dept where id=";
		strSql += szId;
		strSql += " and depttype=";
		char szTmp[16] = {0};
		::itoa(nType, szTmp, 10);
		strSql += szTmp;
		if (m_pExtraDb->Execute(strSql.c_str()))
			return S_OK;
	}
	return E_FAIL;
}

//
STDMETHODIMP CEContactsImpl::DeleteExtractUser(const char *szId, int nType)
{
	if (m_pExtraDb)
	{
		std::string strSql = "delete from user where userid=";
		strSql += szId;
		strSql += " and usertype=";
		char szTmp[16] = {0};
		::itoa(nType, szTmp, 10);
		strSql += szTmp;
		if (m_pExtraDb->Execute(strSql.c_str()))
			return S_OK;
	}
	return E_FAIL;
}

STDMETHODIMP_(int) CEContactsImpl::IsExistsExtraUsers(const char *szUserName, int nType) 
{
	int nId = 0; 
	if (m_pExtraDb)
	{
		std::string strSql = "select userid from user where lower(username)=lower('";
		strSql += szUserName;
		strSql += "') and usertype=";
		char szTmp[16] = {0};
		::itoa(nType, szTmp, 10);
		strSql += szTmp;
		char **szResult = NULL;
		int nRow, nCol;
		if (m_pExtraDb->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			if ((nRow > 0) && (szResult[1]))
				nId = ::atoi(szResult[1]); 
		} 
		CSqliteDBOP::Free_Result(szResult);
	}
	return nId;
}

//获取本人权限列表
STDMETHODIMP CEContactsImpl::GetRoleList(const char *szUserName, IAnsiString *RoleList)
{
	int RuleId = 0;
	std::string szRule;

	std::string strSql = "select ruleid from user where lower(username)=lower('";
	strSql += szUserName;
	strSql += "')";
	char **szResult = NULL;
	int nRow, nCol; 
	if (m_pOrgDb->Open(strSql.c_str(), &szResult, nRow, nCol))
	{
		if ((nRow > 0) && szResult[1])
		{
				RuleId = ::atoi(szResult[1]);
				szRule = szResult[1];
		} //end if (szResult[1])
	} //end if (m_pOrgDb->...
	CSqliteDBOP::Free_Result(szResult); 

	if (RuleId != 0)
	{
		std::string strSql = "select role from role where roleid='";
		strSql += szRule;
		strSql += "'";
		char **szResult = NULL;
		int nRow, nCol;
		if (m_pOrgDb->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			if ((nRow > 0) && szResult[1])
			{
				RoleList->SetString(szResult[1]);
			} //end if ((nRow > 0)
		} //end if (m_pOrgDb
		CSqliteDBOP::Free_Result(szResult);
	}
	return S_OK;
}

//获取传真号码
STDMETHODIMP CEContactsImpl::GetFaxByName(const char *szUserName, IAnsiString *strFax)
{
	HRESULT hr = E_FAIL;
	std::string strName, strDomain;
	if (SepNameDomainByUserName(szUserName, strName, strDomain))
	{
		std::string strSql = "select fax from user where lower(username)=lower('";
		strSql += strName;
		strSql += "')";
		char **szResult = NULL;
	    int nRow, nCol; 
	    if (m_pOrgDb->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			if ((nRow > 0) && szResult[1])
			{
				strFax->SetString(szResult[1]);
				hr = S_OK;
			} //end if (szResult[1])
		} //end if (m_pOrgDb->...
		CSqliteDBOP::Free_Result(szResult); 
	} else //外部联系人 
	{
		std::string strSql = "select fax from user where lower(usename)=lower('";
		strSql += szUserName;
		strSql += "')";
		char **szResult = NULL;
		int nRow, nCol;
		if (m_pExtraDb->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			if ((nRow > 0) && szResult[1])
			{
				strFax->SetString(szResult[1]);
				hr = S_OK;
			} //end if ((nRow > 0)
		} //end if (m_pExtraDb->
		CSqliteDBOP::Free_Result(szResult);
	}
	return hr;
}

//跟据传真号获取用户名
STDMETHODIMP CEContactsImpl::GetUserNameByFax(const char *szFax, IAnsiString *strUserName, IAnsiString *strRealName)
{
	HRESULT hr = E_FAIL; 
	 
	std::string strSql = "select username,realname from user where fax='";
	strSql += szFax;
	strSql += "'";
	char **szResult = NULL;
    int nRow, nCol; 
    if (m_pOrgDb->Open(strSql.c_str(), &szResult, nRow, nCol))
	{
		if ((nRow > 0) && szResult[2] && szResult[3])
		{
			strUserName->SetString(szResult[2]);
			strRealName->SetString(szResult[3]);
			hr = S_OK;
		} //end if (szResult[1])
	} //end if (m_pOrgDb->...
	CSqliteDBOP::Free_Result(szResult); 
	if (SUCCEEDED(hr))
		return hr;
	 
	//外部联系人  
	strSql = "select username,realname from user where fax='";
	strSql += szFax;
	strSql += "'";
	szResult = NULL; 
	if (m_pExtraDb->Open(strSql.c_str(), &szResult, nRow, nCol))
	{
		if ((nRow > 0) && szResult[2] && szResult[3])
		{
			strUserName->SetString(szResult[2]);
			strRealName->SetString(szResult[3]);
			hr = S_OK;
		} //end if ((nRow > 0)
	} //end if (m_pExtraDb->
	CSqliteDBOP::Free_Result(szResult);
	 
	return hr;
}

int CEContactsImpl::GetUserCorpId(const char *szUserName, int &nSign)
{
	int nCorpId = 0;
	if (m_pOrgDb)
	{
		std::string strName, strDomain;
		SepNameDomainByUserName(szUserName, strName, strDomain);
		std::string strSql = "select secdeptid from user where lower(username)=lower('";
		strSql += strName;
		strSql += "')";
		char **szResult = NULL;
		int nCol, nRow;
		if (m_pOrgDb->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			if (nRow > 0)
				nCorpId = ::atoi(szResult[1]);
		}
		CSqliteDBOP::Free_Result(szResult);
		strSql = "select sign from user where lower(username)=lower('";
		strSql += strName;
		strSql += "')";
		if (m_pOrgDb->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			if (nRow > 0)
				nSign = ::atoi(szResult[1]);
		}
		CSqliteDBOP::Free_Result(szResult);
	}
	return nCorpId;
}

#pragma warning(default:4996)
