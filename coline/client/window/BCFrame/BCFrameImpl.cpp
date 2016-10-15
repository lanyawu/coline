#include <time.h>
#include <Commonlib/systemutils.h>
#include <Commonlib/DebugLog.h>
#include <FileTransfer/filetransfer.h>
#include "BCFrameImpl.h"
#include "../IMCommonLib/InterfaceAnsiString.h"
#include "../IMCommonLib/XmlNodeTranslate.h"
#include <Core/treecallbackfun.h>
#include <Core/common.h>
#include "../P2Svr/P2Svr.h"
#include "../IMCommonLib/InterfaceFontStyle.h"
#include <ShellAPI.h>

#pragma warning(disable:4996)

int CALLBACK CutImage(HWND hParent, BOOL bHideParent);
 
int CALLBACK DetailsEnumProc (const ENUMLOGFONTEX *lpelfe, const NEWTEXTMETRICEX *lpntme, unsigned long FontType, LPARAM lParam) 
{ 
	HWND h = reinterpret_cast<HWND> (lParam);
	if ((FontType == TRUETYPE_FONTTYPE)
		&& (lpelfe->elfFullName[0] != L'@'))
		::SkinSetDropdownItemString(h, L"cbFontName", -1, lpelfe->elfFullName, NULL);
	return 1;
}

BOOL CALLBACK RichEditCallBack(HWND hWnd, DWORD dwEvent, char *szFileName, DWORD *dwFileNameSize,
			  char *szFileFlag, DWORD *dwFileFlagSize, DWORD dwFlag, void *pOverlapped)
{
	if (pOverlapped)
	{
		CBCFrameImpl *pThis = (CBCFrameImpl *) pOverlapped;
		return pThis->RECallBack(hWnd, dwEvent, szFileName, dwFileNameSize, szFileFlag, dwFileFlagSize, dwFlag);
	}
	return FALSE;
}

CBCFrameImpl::CBCFrameImpl(void):
              m_pCore(NULL),
			  m_hWnd(NULL)
{
	m_strFileTransSkinXml = "<Control xsi:type=\"FileProgressBar\" progressimage=\"27\" bkgndimage=\"28\"  name=\"filename\" filename=\"文件2\" filesize=\"224123413\" currfilesize=\"0\" height=\"60\"/>";
}


CBCFrameImpl::~CBCFrameImpl(void)
{
	if (m_hWnd)
		::SkinCloseWindow(m_hWnd);
	m_hWnd = NULL;
	if (m_pCore)
		m_pCore->Release();
	m_pCore = NULL;
}

//IUnknown
STDMETHODIMP CBCFrameImpl::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IBCFrame)))
	{
		*ppv = (IBCFrame *) this;
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

HRESULT CBCFrameImpl::DoMenuCommand(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "treegroup") == 0)
	{
		switch(wParam)
		{
		case 80001:
			 DoCreateGroupMenu(L"发送广播消息",  1); 
			 break;
		case 80002:
			 DoCreateGroupMenu(L"群发文件", 3);
			 break;
		case 70002: //群发短信
			 DoCreateGroupMenu(L"群发短信", 2);
			 break;
		}
	} if (::stricmp(szName, "cutmenu") == 0)
	{
		if (wParam == 20001) //截屏菜单ID
		{
			CutScreen(hWnd, FALSE);
		} else if (wParam == 20002) //隐藏再截屏
		{
			CutScreen(hWnd, TRUE);
		}
	}
	return -1;
}

BOOL CBCFrameImpl::CutScreen(HWND hWnd, BOOL bHide)
{
	if (CutImage(hWnd, bHide) == IDOK)
	{
		return ::SkinRichEditCommand(hWnd, L"bctext", "paste", NULL);
	}
	return FALSE;
}

void CBCFrameImpl::SaveMessage(const char *szType, TiXmlElement *pNode)
{
	IMsgMgr *pMsgMgr = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMsgMgr), (void **)&pMsgMgr)))
	{
		TiXmlString strXml;
		pNode->SaveToString(strXml, 0);
		TiXmlElement *pBody = pNode->FirstChildElement("body");
		std::string strText;
		if (pBody)
		{
			if (pBody->GetText())
			{
				strText = pBody->GetText();
			}
		}
		pMsgMgr->SaveMsg(szType, pNode->Attribute("from"), pNode->Attribute("to"), pNode->Attribute("datetime"),
			strXml.c_str(), strText.c_str());
		pMsgMgr->Release();
	}
}

BOOL CBCFrameImpl::SendFilesToUsers(HWND hWnd)
{
	if (m_SelList.GetUserCount() <= 0)
	{
		::SkinMessageBox(hWnd, L"请先选择要发送的人员", L"提示", MB_OK);
		return FALSE;
	}
	if (m_TransFileList.GetCount() > 0)
	{
		::SkinSetControlEnable(hWnd, L"btnSend", FALSE);
		IConfigure *pCfg = NULL;
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			CInterfaceAnsiString strUrl;
			if (SUCCEEDED(pCfg->GetServerAddr(HTTP_SVR_URL_OFFLINE_FILE, &strUrl)))
			{
				std::vector<int> FileList;
				m_TransFileList.GetOwnerWindowList(m_hWnd, FileList);
				if (!FileList.empty())
				{
					CTransferFileInfo Info;
					int nFileId;
					while (!FileList.empty())
					{
						nFileId = FileList.back();
						if (m_TransFileList.GetFileInfoById(nFileId, Info))
						{
							CCustomPicItem *pItem = new CCustomPicItem();
							pItem->m_hOwner = hWnd;
							pItem->m_strFlag = Info.m_strFileTag;
							pItem->m_pOverlapped = this;
							pItem->m_strLocalFileName = Info.m_strLocalFileName;  
							pItem->m_strPeerName = Info.m_strPeerName; 
							pItem->m_strUrl = strUrl.GetData();
							pItem->m_nFileSize = Info.m_dwFileSize;
							pItem->m_nFileId = nFileId;
							std::string strParam = "filename=";
							strParam += Info.m_strFileTag;
							strParam += ";username=";
							strParam += m_strUserName;
							if (m_CustomPics.AddItem(pItem))
							{ 

								::P2SvrPostFile(strUrl.GetData(), Info.m_strLocalFileName.c_str(), strParam.c_str(), FILE_TYPE_NORMAL, 
									pItem, HttpUpCallBack, FALSE);
							} else
								delete pItem;
						}
						FileList.pop_back();
					} //end while (
				} //end if (m_TransFileList...
			} //end if (SUCCEEDED(pCfg->...
			pCfg->Release();
			return TRUE;
		} //end if (m_pCore->
	} else
	{
		::SkinMessageBox(m_hWnd, L"请先选择要群发的文件", L"提示", MB_OK);
	}
		//
	return TRUE;
}

void CALLBACK HttpUpCallBack(int nErrorCode, int nType, WPARAM wParam, LPARAM lParam, void *pOverlapped)
{
	switch(nErrorCode)
	{
	case ERROR_CODE_COMPLETE:
		{
			if (nType == FILE_TYPE_CUSTOMPIC)
			{
				//custom picture download notify
				//// <trs type="custompic" from="wuxiaozhong@gocom"  to="admin@gocom" 
				//   filename="30f79afad4318774e447dc2db96936e0.gif" fileserver="http://imbs.smartdot.com.cn:9910" 
				CCustomPicItem *pItem = (CCustomPicItem *)pOverlapped;				
				CBCFrameImpl *pThis = (CBCFrameImpl *)pItem->m_pOverlapped;
				std::string strXml = "<trs type=\"custompic\" from=\"";
				strXml += pThis->m_strUserName;
				strXml += "\" to=\"";
				strXml += pItem->m_strPeerName;
				strXml += "\" filename=\"";
				char szTmp[MAX_PATH] = {0};
				CSystemUtils::ExtractFileName(pItem->m_strLocalFileName.c_str(), szTmp, MAX_PATH - 1);
				strXml += szTmp;
				strXml += "\" fileserver=\"";
				//url = "http://imbs.smartdot.com.cn:9910/upcustompic.php"
				int nPos = pItem->m_strUrl.find_last_of(":");
				std::string strSvr;
				if (nPos != std::string::npos)
				{
					nPos = pItem->m_strUrl.find("/", nPos);
					if (nPos != std::string::npos)
					{
						strSvr = pItem->m_strUrl.substr(0, nPos);
					} //end if (nPos != ...
				} //end if (nPos != std::...
				if (!strSvr.empty())
					strXml += strSvr;
				else
					strXml += pItem->m_strUrl;
				strXml += "\"/>";
				pThis->BroadcastFileMsg(strXml.c_str()); 
				pThis->m_CustomPics.DeleteItem(pItem);
			} else if (nType == FILE_TYPE_NORMAL)
			{
				//<msg type="offlinefile" from="admin@gocom" to="wuxiaozhong@gocom" name="apss.dll.mui" filesize="3072" url="A90C62138DFB74BEA86244E2432D133EF.mui
				CCustomPicItem *pItem = (CCustomPicItem *)pOverlapped;				
				CBCFrameImpl *pThis = (CBCFrameImpl *)pItem->m_pOverlapped;
				std::string strXml = "<msg type=\"offlinefile\" from=\"";
				strXml += pThis->m_strUserName;
				strXml += "\" to=\"";
				strXml += pItem->m_strPeerName;
				strXml += "\" name=\"";
				char szTmp[MAX_PATH] = {0};
				CSystemUtils::ExtractFileName(pItem->m_strLocalFileName.c_str(), szTmp, MAX_PATH - 1);
				strXml += szTmp;
				strXml += "\" filesize=\"";
				memset(szTmp, 0, MAX_PATH);
				::itoa(pItem->m_nFileSize, szTmp, 10);
				strXml += szTmp;
				strXml += "\" url=\"";
				strXml += pItem->m_strFlag;
				memset(szTmp, 0, MAX_PATH);
				CSystemUtils::ExtractFileExtName(pItem->m_strLocalFileName.c_str(), szTmp, MAX_PATH - 1);
				strXml += ".";  //szTmp 扩展名不带 .
				strXml += szTmp;
				strXml += "\" fileserver=\"";
				//url = "http://imbs.smartdot.com.cn:9910/upcustompic.php"
				int nPos = pItem->m_strUrl.find_last_of(":");
				std::string strSvr;
				if (nPos != std::string::npos)
				{
					nPos = pItem->m_strUrl.find("/", nPos);
					if (nPos != std::string::npos)
					{
						strSvr = pItem->m_strUrl.substr(0, nPos);
					} //end if (nPos != ...
				} //end if (nPos != std::...
				if (!strSvr.empty())
					strXml += strSvr;
				else
					strXml += pItem->m_strUrl;
				strXml += "\"/>";

				//广播
				pThis->BroadcastFileMsg(strXml.c_str()); 

				/*CStdString_ strTip = L"文件 ";
				TCHAR szwTmp[MAX_PATH] = {0};
				CStringConversion::StringToWideChar(pItem->m_strLocalFileName.c_str(), szwTmp, MAX_PATH - 1);
				strTip += szwTmp;
				strTip += L"  发送完毕";*/
				//pThis->AnsycShowTip(pItem->m_hOwner, strTip.GetData());
				CTransferFileInfo Info;
				if (pThis->m_TransFileList.GetFileInfoById(pItem->m_nFileId, Info))
				{
					pThis->m_TransFileList.DeleteFileInfo(pItem->m_nFileId);
					TCHAR *szProFlag = new TCHAR[Info.m_strProFlag.GetLength() + 1];
					memset(szProFlag, 0, sizeof(TCHAR) * (Info.m_strProFlag.GetLength() + 1));
					lstrcpy(szProFlag, Info.m_strProFlag.GetData());
					::PostMessage(pThis->m_hWnd, WM_BCRM_FILEPRO, WPARAM(Info.hOwner), LPARAM(szProFlag));
				}
				
				pThis->m_CustomPics.DeleteItem(pItem);
			}//end if (wParam ==
			break;
		} //end case Error
	case ERROR_CODE_PROGRESS:
		{
			if (nType == FILE_TYPE_NORMAL)
			{
				//显示进度				
				CCustomPicItem *pItem = (CCustomPicItem *)pOverlapped;				
				CBCFrameImpl *pThis = (CBCFrameImpl *)pItem->m_pOverlapped;
				::PostMessage(pThis->m_hWnd, WM_BCSW_FILEPRO, pItem->m_nFileId, lParam); 
			}
			break;
		}
	} //end switch(..
}

BOOL CBCFrameImpl::UploadLocalFileToServer(HWND hWnd, const char *szLocalFileName)
{ 
	if (m_TransFileList.CheckIsTrans(m_strUserName.c_str(), szLocalFileName))
	{
		//AnsycShowTip(hWnd, L"文件正在传送中");
		return FALSE;
	}
	char szDspName[MAX_PATH] = {0};
	char szTag[MAX_PATH] = {0};
	char szFileId[32] = {0};
	int nTagSize = MAX_PATH - 1;
	::GetFileTagByName(szLocalFileName, szTag, &nTagSize);
	char szFileSize[32] = {0};
	char szTmp[32] = {0};
	DWORD dwFileSize = (DWORD) CSystemUtils::GetFileSize(szLocalFileName);
	::itoa(dwFileSize, szFileSize, 10);					
	CSystemUtils::ExtractFileName(szLocalFileName, szDspName, MAX_PATH - 1);
	TCHAR szFlag[MAX_PATH] = {0};
	TCHAR szwTmp[MAX_PATH] = {0};
	int nFlagSize = MAX_PATH - 1;
	::SkinAddChildControl(hWnd, L"fileprogress", m_strFileTransSkinXml.c_str(), szFlag, &nFlagSize, 999999);
	int nFileId = m_TransFileList.AddFileInfo(m_strUserName.c_str(), szDspName, szLocalFileName, szFlag, 
		                            szTag, "", "0", "", "0", "0", 0, szFileSize, hWnd, TRUE);
//
	IConfigure *pCfg = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		CInterfaceAnsiString strUrl;
		if (SUCCEEDED(pCfg->GetServerAddr(HTTP_SVR_URL_OFFLINE_FILE, &strUrl)))
		{
			CCustomPicItem *pItem = new CCustomPicItem();
			pItem->m_hOwner = hWnd;
			pItem->m_strFlag = szTag;
			pItem->m_pOverlapped = this;
			pItem->m_strLocalFileName = szLocalFileName;  
			pItem->m_strPeerName = m_strUserName; 
			pItem->m_strUrl = strUrl.GetData();
			pItem->m_nFileSize = dwFileSize;
			pItem->m_nFileId = nFileId;
			std::string strParam = "filename=";
			strParam += szTag;
			strParam += ";username=";
			strParam += m_strUserName;
			if (m_CustomPics.AddItem(pItem))
			{				
				::P2SvrPostFile(strUrl.GetData(), szLocalFileName, strParam.c_str(), FILE_TYPE_NORMAL, 
					pItem, HttpUpCallBack, FALSE);
			} else
				delete pItem;
		}
		pCfg->Release();
		return TRUE;
	}
	return FALSE;
}

const char *CBCFrameImpl::GetImagePath()
{
	if (m_strImagePath.empty())
	{
		IConfigure *pCfg = NULL;
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			CInterfaceAnsiString strPath;
			if (SUCCEEDED(pCfg->GetPath(PATH_LOCAL_CUSTOM_PICTURE, &strPath)))
				m_strImagePath = strPath.GetData(); 
			pCfg->Release();
		} //end if (m_pCore && SUCCEEDED(..
	} //end if (m_strImagePath...
	return m_strImagePath.c_str();
}

BOOL CBCFrameImpl::UploadCustomPicToServer(HWND hWnd, const char *szFlag)
{
	BOOL bSucc = FALSE;
	IEmotionFrame *pFrame = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IEmotionFrame), (void **)&pFrame)))
	{
		CInterfaceAnsiString strFileName;
		if (SUCCEEDED(pFrame->GetSysEmotion(szFlag, &strFileName)))
			bSucc = TRUE;
		pFrame->Release();
	}
	if (bSucc)
		return TRUE;
	
	char szFileName[MAX_PATH] = {0};
	sprintf(szFileName, "%s%s.gif", GetImagePath(), szFlag);
	//
	IConfigure *pCfg = NULL;
	if ((CSystemUtils::FileIsExists(szFileName)) && m_pCore 
		&& SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		CInterfaceAnsiString strUrl;
		if (SUCCEEDED(pCfg->GetServerAddr(HTTP_SVR_URL_CUSTOM_PICTURE, &strUrl)))
		{
			CCustomPicItem *pItem = new CCustomPicItem();
			pItem->m_hOwner = hWnd;
			pItem->m_strFlag = szFlag;
			pItem->m_pOverlapped = this;
			pItem->m_strLocalFileName = szFileName;  
			pItem->m_strPeerName = m_strUserName; 
			pItem->m_strUrl = strUrl.GetData();
			if (m_CustomPics.AddItem(pItem))
			{				
				::P2SvrPostFile(strUrl.GetData(), szFileName, NULL, FILE_TYPE_CUSTOMPIC, 
					pItem, HttpUpCallBack, FALSE);
			} else
				delete pItem;
		}
		pCfg->Release();
		return TRUE;
	}
	return FALSE;
}

BOOL CBCFrameImpl::SendOleResourceToPeer(HWND hWnd)
{
	//
	char *pOle = NULL;
	if (::SkinGetRichEditOleFlag(hWnd,  L"bctext", &pOle) && pOle)
	{ 
		int nIdx = 0;
		while (TRUE)
		{
			char szTmp[128] = {0};
			if (!CSystemUtils::GetStringBySep(pOle, szTmp, ';', nIdx))
				break; 
			if (::strlen(szTmp) > 0)
				UploadCustomPicToServer(hWnd, szTmp);
			nIdx ++;
		}
		free(pOle);
	}
	return TRUE;
}

void CBCFrameImpl::BroadcastFileMsg(const char *szFileMsg)
{
	CGuardLock::COwnerLock guard(m_PendLock);
	TiXmlDocument xmldoc;
	if (xmldoc.Load(szFileMsg, ::strlen(szFileMsg)))
	{
		TiXmlElement *pNode = xmldoc.FirstChildElement();
		if (pNode)
		{
			std::vector<LPORG_TREE_NODE_DATA> TmpList;
			LPORG_TREE_NODE_DATA pData;
			while (SUCCEEDED(m_SelList.PopBackUserInfo(&pData)))
			{
				pNode->SetAttribute("to", pData->szUserName);
				TiXmlString strXml;
				xmldoc.SaveToString(strXml, 0);
				//send to peer
				m_pCore->SendRawMessage((BYTE *)strXml.c_str(), strXml.size(), 0);
				//
				//SaveMessage("p2p", xmldoc.FirstChildElement());
				TmpList.push_back(pData);
				PRINTDEBUGLOG(dtInfo, "broadcast file to user:%s", pData->szUserName);
			}
			while (!TmpList.empty())
			{
				pData = TmpList.back();
				TmpList.pop_back();
				m_SelList.AddUserInfo(pData, FALSE, TRUE);
			} //end while(!..
		} //end if (pNode)
	} //end if (xmldoc..
}

BOOL CBCFrameImpl::SendSms(HWND hWnd)
{
	char *p = ::SkinGetRichEditOleText(hWnd, L"bctext", 0);
	if (p)
	{
		std::string strNumber;
		std::vector<LPORG_TREE_NODE_DATA> TmpList;
		LPORG_TREE_NODE_DATA pData;
		CInterfaceAnsiString strPhone;
		std::map<std::string, std::string>::iterator it;
		while (SUCCEEDED(m_SelList.PopBackUserInfo(&pData)))
		{
			it = m_PhoneTable.find(pData->szUserName);
			if (it != m_PhoneTable.end())
			{ 
				strNumber += it->second;
				strNumber += ";";
			} 
			TmpList.push_back(pData);
		}
		while (!TmpList.empty())
		{
			pData = TmpList.back();
			TmpList.pop_back();
			m_SelList.AddUserInfo(pData, FALSE, TRUE);
		}
		if (!strNumber.empty())
		{
			ISMSFrame *pFrame = NULL;
			if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(ISMSFrame), (void **)&pFrame)))
			{
				if (SUCCEEDED(pFrame->SendSMS(strNumber.c_str(), p, NULL)))
					::SkinMessageBox(m_hWnd, L"群发短信成功", L"提示", MB_OK);
				else
					::SkinMessageBox(m_hWnd, L"群发短信失败", L"提示", MB_OK);
				pFrame->Release();
			} else
					::SkinMessageBox(m_hWnd, L"群发短信失败", L"提示", MB_OK);
		} else
			::SkinMessageBox(m_hWnd, L"接收短信用户不能为空", L"警告", MB_OK);
	} else
		::SkinMessageBox(m_hWnd, L"发送短信内空不能为空", L"警告", MB_OK);
	return FALSE;
}

BOOL CheckInputChars(const char *p)
{
	if (p)
	{
		int nSize = ::strlen(p); 
		const char *p1 = p;
		while (*p1 == ' ')
		{
			p1 ++;
			nSize --;
		}
		if (nSize > 1)
			return TRUE;
	}
	return FALSE;
}

static const char BROADCAST_MESSAGE_XML_FORMAT[] = "<msg type=\"p2p\"><font/><body></body></msg>";
BOOL CBCFrameImpl::SendMessageToPeer(HWND hWnd)
{
	if (m_SelList.GetUserCount() <= 0)
	{
		::SkinMessageBox(hWnd, L"请先选择要发送的人员", L"提示", MB_OK);
		return FALSE;
	}
	std::string strUserName;
	if (m_pCore)
	{
		CInterfaceAnsiString szName;
		m_pCore->GetUserName((IAnsiString *)&szName);
		strUserName = szName.GetData();
		m_pCore->GetUserDomain((IAnsiString *)&szName);
		strUserName += "@";
		strUserName += szName.GetData();
	}
	//
	CCharFontStyle cf = {0};
	if (::SkinGetCurrentRichEditFont(hWnd, L"bctext", &cf))
	{
		char *p = ::SkinGetRichEditOleText(hWnd, L"bctext", 0);
		if (CheckInputChars(p))
		{			 
			char szTime[64] = {0};
			CSystemUtils::DateTimeToStr((DWORD)::time(NULL), szTime);
			TiXmlDocument xmlDoc;
			if (xmlDoc.Load(BROADCAST_MESSAGE_XML_FORMAT, ::strlen(BROADCAST_MESSAGE_XML_FORMAT)))
			{
				TCHAR szValue[32] = {0};
				int nSize = 31;
				::SkinGetControlAttr(hWnd, L"receipt", L"down", szValue, &nSize);
				char szReceipt[32] = {0};
				CStringConversion::WideCharToString(szValue, szReceipt, 31);
				TiXmlElement *pNode = xmlDoc.FirstChildElement();
				pNode->SetAttribute("from", strUserName.c_str());
				pNode->SetAttribute("Receipt", szReceipt);
				pNode->SetAttribute("datetime", szTime);
				//Font
				TiXmlElement *pFont = pNode->FirstChildElement("font");
				if (pFont)
				{
					CXmlNodeTranslate::FontStyleToXmlNode(cf, pFont);
				}
				TiXmlElement *pBody = pNode->FirstChildElement("body");
				if (pBody)
				{
					TiXmlText pText(p);
					pBody->InsertEndChild(pText);
				}
				std::vector<LPORG_TREE_NODE_DATA> TmpList;
				LPORG_TREE_NODE_DATA pData;
				while (SUCCEEDED(m_SelList.PopBackUserInfo(&pData)))
				{
					pNode->SetAttribute("to", pData->szUserName);
					TiXmlString strXml;
					xmlDoc.SaveToString(strXml, 0);
					//send to peer
					m_pCore->SendRawMessage((BYTE *)strXml.c_str(), strXml.size(), 0);
						//
					SaveMessage("p2p", xmlDoc.FirstChildElement());
					TmpList.push_back(pData);
				}
				while (!TmpList.empty())
				{
					pData = TmpList.back();
					TmpList.pop_back();
					m_SelList.AddUserInfo(pData, FALSE, TRUE);
				}
				SendOleResourceToPeer(hWnd);
				//清除输入框
				::SkinRichEditCommand(hWnd, L"bctext", "clear", NULL); 
			} else
			{
				PRINTDEBUGLOG(dtInfo, "Load Base P2p Xml Failed");
			} //end else if (xmlDoc.Load(... 
		} //end if (p)
	} //end if (::GetCurrentRichEditFont(...
	return FALSE;
}

void CBCFrameImpl::DoCreateGroupMenu(const TCHAR *szCaption, int nStyle)
{
	IMainFrame *pFrame = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMainFrame), (void **)&pFrame)))
	{
		void *pSelNode = NULL;
		LPORG_TREE_NODE_DATA pSelData = NULL;
		TCHAR szName[MAX_PATH] = {0};
		int nSize = MAX_PATH - 1;
		CTreeNodeType tnType;
		if (::SkinGetSelectTreeNode(pFrame->GetSafeWnd(), L"colleaguetree", szName, 
			&nSize, &pSelNode, &tnType, (void **)&pSelData))
		{
			if (tnType == TREENODE_TYPE_GROUP)
			{
				char szUserList[8192] = {0};
				int nSize = 8191;
				if (::SkinGetNodeChildUserList(pFrame->GetSafeWnd(), pSelNode, szUserList, &nSize, 
			                      FALSE))
				{
					CInterfaceUserList UserList; 
					if (nSize > 0)
					{
						TiXmlDocument xml;
						if (xml.Load(szUserList, nSize))
						{
							TiXmlElement *pNode = xml.FirstChildElement(); 
							while (pNode)
							{
								LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
								memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
								strcpy(pData->szUserName, pNode->Attribute("u"));
								UserList.AddUserInfo(pData, FALSE, TRUE);
								pNode = pNode->NextSiblingElement();
							}
						} //end if (xml.Load(...
					} else //节点未展开
					{
						IContacts *pContact = NULL;
						if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
						{
							char szDeptId[16] = {0};
							::itoa(pSelData->id, szDeptId, 10);
							pContact->GetChildListByDeptId(szDeptId, &UserList, 0);
							pContact->Release();
						}
					} 
					if (nStyle == 2) //短信
					{
						ISMSFrame *pSMSFrame = NULL;
						if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(ISMSFrame), (void **)&pSMSFrame)))
						{
							pSMSFrame->ShowSMSFrame(NULL, &UserList);
							pSMSFrame->Release();
						}
					} else
					{
						ShowBCFrame(szCaption, NULL, nStyle, &UserList);
					}
				} // end if (::SkinGetNodeChildUserList(..
			} //end if (tnType..
		} //end if (::SkinGetSelectTreeNode(..
		pFrame->Release();
	} //end if (m_pCore && ...
}

void CBCFrameImpl::BroadCastMessage()
{
	//
	switch(m_nStyle)
	{
	case 1: //群发消息
		{
			SendMessageToPeer(m_hWnd);
			break;
		}
	case 2: //群发短信
		{ 
			SendSms(m_hWnd);
			break;
		}
	case 3: //群发文件
		{ 
			SendFilesToUsers(m_hWnd);
			break;
		} //end case
	} //end switch(..
}

HRESULT CBCFrameImpl::DoClickEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "contacttree") == 0)
	{
		//
		void *pSelNode = NULL;
		LPORG_TREE_NODE_DATA pSelData = NULL;
		TCHAR szName[MAX_PATH] = {0};
		int nSize = MAX_PATH - 1;
		CTreeNodeType tnType;
		if (::SkinGetSelectTreeNode(hWnd, L"contacttree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
		{
			if (tnType == TREENODE_TYPE_GROUP)
			{
				if (pSelData && pSelData->bOpened == 0)
				{
					IContacts *pContact = NULL;
					if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
					{
						pContact->ExpandTreeNodeToUI(hWnd, L"contacttree", pSelNode, pSelData->id);
						pSelData->bOpened = TRUE;
						pContact->Release();
					} 
				} // end if (pSelData ...					 
			} else
			{
				//
			} //end else if (tnType == ...
		} //end if (::GetSelectTreeNode(hWnd...
	} else if (::stricmp(szName, "emotion") == 0)
	{ 
		POINT pt = {0};
		::GetCursorPos(&pt);
	    IEmotionFrame *pFrame = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IEmotionFrame), (void **)&pFrame)))
		{
			pFrame->ShowEmotionFrame((ICoreEvent *) this, hWnd, pt.x, pt.y);
			pFrame->Release();
		}
	} else if (::stricmp(szName, "emotionpanel") == 0)
	{
		DoEmotionClick(hWnd, wParam, lParam);
		::SkinSetControlFocus(hWnd, L"bcText", TRUE);
		//
	} else if (::stricmp(szName, "picture") == 0)
	{
		DoSendPicture(hWnd);
	} else if (::stricmp(szName, "CutScreen") == 0)
	{
		CutScreen(hWnd, FALSE);
	} else if (::stricmp(szName, "receipt") == 0)
	{
		TCHAR szValue[32] = {0};
		int nSize = 32;
		if (::SkinGetControlAttr(hWnd, L"receipt", L"down", szValue, &nSize))
		{
			if (::lstrcmpi(szValue, L"true") == 0)
			{
				::SkinSetControlAttr(hWnd, L"receipt", L"down", L"false");
			} else
				::SkinSetControlAttr(hWnd, L"receipt", L"down", L"true");
		} //end if (::SkinGetControlAttr		
	} else if (::stricmp(szName, "fontset") == 0)
	{
		BOOL bVisible = !::SkinGetControlVisible(hWnd, L"FontSetting");
		::SkinSetControlVisible(hWnd, L"FontSetting", bVisible);
		if (bVisible)
			::SkinSetControlAttr(hWnd, L"fontset", L"down", L"true");
		else
			::SkinSetControlAttr(hWnd, L"fontset", L"down", L"false");
	} else if (::stricmp(szName, "fontbold") == 0)
	{
		IConfigure *pCfg = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			TCHAR szValue[16] = {0};
			int nSize = 15;
			if (::SkinGetControlAttr(hWnd, L"fontbold", L"down", szValue, &nSize))
			{
				if (_tcsicmp(szValue, L"true") == 0)
				{
					pCfg->SetParamValue(FALSE, "ChatFont", "fontbold", "false");
					::SkinSetControlAttr(hWnd, L"fontbold", L"down", L"false");
				} else
				{
					pCfg->SetParamValue(FALSE, "ChatFont", "fontbold", "true");
					::SkinSetControlAttr(hWnd, L"fontbold", L"down", L"true");
				}
				RefreshInputChatFont(hWnd, pCfg);
			}
			pCfg->Release();
		}		
	} else if (::stricmp(szName, "fontitalic") == 0)
	{
		IConfigure *pCfg = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			TCHAR szValue[16] = {0};
			int nSize = 15;
			if (::SkinGetControlAttr(hWnd, L"fontitalic", L"down", szValue, &nSize))
			{
				if (_tcsicmp(szValue, L"true") == 0)
				{
					pCfg->SetParamValue(FALSE, "ChatFont", "fontitalic", "false");
					::SkinSetControlAttr(hWnd, L"fontitalic", L"down", L"false");
				} else
				{
					pCfg->SetParamValue(FALSE, "ChatFont", "fontitalic", "true");
					::SkinSetControlAttr(hWnd, L"fontitalic", L"down", L"true");
				}
				RefreshInputChatFont(hWnd, pCfg);
			}
			pCfg->Release();
		}	
	} else if (::stricmp(szName, "fontunderline") == 0)
	{
		IConfigure *pCfg = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			TCHAR szValue[16] = {0};
			int nSize = 15;
			if (::SkinGetControlAttr(hWnd, L"fontunderline", L"down", szValue, &nSize))
			{
				if (_tcsicmp(szValue, L"true") == 0)
				{
					pCfg->SetParamValue(FALSE, "ChatFont", "fontunderline", "false");
					::SkinSetControlAttr(hWnd, L"fontunderline", L"down", L"false");
				} else
				{
					pCfg->SetParamValue(FALSE, "ChatFont", "fontunderline", "true");
					::SkinSetControlAttr(hWnd, L"fontunderline", L"down", L"true");
				}
				RefreshInputChatFont(hWnd, pCfg);
			}
			pCfg->Release();
		}	
	} else if (::stricmp(szName, "fontcolor") == 0)
	{
		COLORREF cr;
		if (CSystemUtils::OpenColorDialog(NULL, hWnd, cr))
		{
			IConfigure *pCfg = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
			{
				char szValue[16] = {0};
				::itoa(cr, szValue, 10);
				pCfg->SetParamValue(FALSE, "ChatFont", "FontColor",  szValue);
				RefreshInputChatFont(hWnd, pCfg);
				pCfg->Release();
			}	
		}
	} else if (::stricmp(szName, "btnSend") == 0)
	{
		BroadCastMessage();
	} else if (::stricmp(szName, "btnCancel") == 0)
	{
		::SkinCloseWindow(hWnd);
	} else if (::stricmp(szName, "selAllUsers") == 0)
	{
		::SkinTreeSelectAll(hWnd, L"selcontact", NULL, TRUE);
	} else if (::stricmp(szName, "unselUsers") == 0)
	{
		::SkinTreeUnSelected(hWnd, L"selcontact", NULL, TRUE);
	} else if (::stricmp(szName, "delselusers") == 0)
	{
		DelTreeSelectedUsers(hWnd);
	} else if (::stricmp(szName, "addfile") == 0)
	{
		CStringList_ FileList;
		if (CSystemUtils::OpenFileDialog(NULL, hWnd, "选择要发送的文件", "所有文件(*.*)|*.*", 
			                NULL, FileList, FALSE, FALSE))
		{
			std::string strFileName;
			if (!FileList.empty())
				strFileName = FileList.back();
			if ((!strFileName.empty()) && CSystemUtils::FileIsExists(strFileName.c_str()))
			{
				BroadcastFile(strFileName.c_str()); 			
			} //end if ((!strFileName.empty()) ...
		}
	} else if (::stricmp(szName, "delallfile") == 0)
	{
		DeleteAllSelFile();
	} else if (::stricmp(szName, "upfile") == 0)
	{
		//群发文件
		ShowBCFrame(L"群发文件", NULL, 3, NULL);
		 
	} else if (::stricmp(szName, "sendsmg") == 0)
	{  
		ShowBCFrame(L"群发消息", NULL, 1, NULL); 
	} 
	return -1;
}

BOOL CBCFrameImpl::DoSendPicture(HWND hWnd)
{
	CStringList_ FileList;
 	if (CSystemUtils::OpenFileDialog(NULL, hWnd, "选择要发送的文件", "所有图片文件(*.*)|*.bmp;*.gif;*.jpg;*.png", 
		                NULL, FileList, FALSE, FALSE))
	{
		std::string strFileName;
		if (!FileList.empty())
			strFileName = FileList.back();
		if ((!strFileName.empty()) && CSystemUtils::FileIsExists(strFileName.c_str()))
		{
			return ::SkinREInsertOlePicture(hWnd, L"bctext", strFileName.c_str()); 						
		} //end if ((!strFileName.empty()) ...
	} //end if (CSystemUtils::OpenFileDialog(...
	return FALSE;
}

void CBCFrameImpl::DeleteAllSelFile()
{
	std::vector<int> FileList;
	m_TransFileList.GetOwnerWindowList(m_hWnd, FileList);
	if (!FileList.empty())
	{
		CTransferFileInfo Info;
		int nFileId;
		while (!FileList.empty())
		{
			nFileId = FileList.back();
			if (m_TransFileList.GetFileInfoById(nFileId, Info))
			{
				::SkinRemoveChildControl(m_hWnd, L"pendingfilelist", Info.m_strProFlag);
			}
			FileList.pop_back();
		}
	}
	m_TransFileList.Clear();
	::SkinSetControlTextByName(m_hWnd, L"selfilecount", L"0");
}

void CBCFrameImpl::DelTreeSelectedUsers(HWND hWnd)
{
	int nSize = 0;
	::SkinTreeGetSelectedUsers(hWnd, L"selcontact", NULL, &nSize);
	if (nSize > 0)
	{
		char *szUsers = new char[nSize + 1];
		memset(szUsers, 0, nSize + 1);
		if (SkinTreeGetSelectedUsers(hWnd, L"selcontact", szUsers, &nSize))
		{
			TiXmlDocument xmldoc;
			if (xmldoc.Load(szUsers, nSize))
			{
				TiXmlElement *pNode = xmldoc.FirstChildElement();
				while (pNode)
				{
					if (pNode->Attribute("u"))
						m_SelList.DeleteUserInfo(pNode->Attribute("u"));
					pNode = pNode->NextSiblingElement();
				} // end while (
			} //end if (xmldoc..
			::SkinTreeDelSelected(hWnd, L"selcontact", TRUE);
		} //end if (SkinTreeGetSelected
	} //end if (nSize > 0)
}

void CBCFrameImpl::RefreshInputChatFont(HWND hWnd, IConfigure *pCfg)
{
	CInterfaceFontStyle FontStyle;
	if (SUCCEEDED(pCfg->GetChatFontStyle((IFontStyle *)&FontStyle)))
	{
		CCharFontStyle fs = {0};
		CXmlNodeTranslate::StringFontToStyle(&FontStyle, fs);
		::SkinNotifyEvent(hWnd, L"bctext", L"setfont", 0, LPARAM(&fs));
	}
}

void CBCFrameImpl::DoEmotionClick(HWND hWnd, WPARAM wParam, LPARAM lParam)
{ 
	char *szFileName = (char *)wParam;
	char *szTagName = (char *)lParam;
	::SkinInsertImageToRichEdit(hWnd, L"bctext", szFileName, szTagName, -1); 
}

BOOL CBCFrameImpl::DoAdjustSelNode(IContacts *pContact, HWND hWnd, const char *szUserName, const TCHAR *szDspName, BOOL bAdd)
{

	if ((m_nStyle == 2) && bAdd) //短信
	{
		CInterfaceAnsiString strPhone; 
		if (SUCCEEDED(pContact->GetPhoneByName(szUserName, &strPhone)) && (strPhone.GetSize() == 11))  //11位的手机号
			m_PhoneTable.insert(std::pair<std::string, std::string>(szUserName, strPhone.GetData()));
		else
		{
			CStdString_ strTip = L"没有找到此用户手机号\n用户名:";
			strTip += szDspName;
			::SkinMessageBox(hWnd, strTip, L"提示", MB_OK);
			return FALSE;
		} //end else if (SUCCEEDED(
	} //end if (m_nStyle == 2) 

	LPORG_TREE_NODE_DATA pCopyData = new ORG_TREE_NODE_DATA();

	memset(pCopyData, 0, sizeof(ORG_TREE_NODE_DATA)); 
	strcpy(pCopyData->szUserName, szUserName);

	if (bAdd)
	{
		if (::stricmp(szUserName, m_strUserName.c_str()) != 0)
		{
			LPORG_TREE_NODE_DATA pTmp = new ORG_TREE_NODE_DATA();
			memcpy(pTmp, pCopyData, sizeof(ORG_TREE_NODE_DATA));
			if (!m_SelList.AddUserInfo(pTmp, FALSE, TRUE))
				delete pTmp;
		}
	} else
	{
		m_SelList.DeleteUserInfo(szUserName);
	}
 
	if (::SkinAdjustTreeNode(hWnd, L"selcontact", NULL, szDspName, TREENODE_TYPE_LEAF, pCopyData, bAdd, TRUE) == NULL)
	{
		delete pCopyData;
	} //end if (::SkinAdjustTreeNode(
	::SkinUpdateControlUI(hWnd, L"selcontact");
	::SkinExpandTree(hWnd, L"selcontact", NULL, TRUE, FALSE);
	return TRUE;
}

void CBCFrameImpl::DoContactTreeChanged(HWND hWnd)
{
	IContacts *pContact = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
	{
        //
		void *pSelNode = NULL;
		LPORG_TREE_NODE_DATA pSelData = NULL;
		TCHAR szName[MAX_PATH] = {0};
		int nSize = MAX_PATH - 1;
		CTreeNodeType tnType;
		if (::SkinGetSelectTreeNode(hWnd, L"contacttree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
		{
			if (tnType == TREENODE_TYPE_LEAF)
			{
				int nStatus = ::SkinGetTreeNodeStatus(pSelNode);
				BOOL bAdd = TRUE;
				if (nStatus == 0)
					bAdd = FALSE; 
				DoAdjustSelNode(pContact, hWnd, pSelData->szUserName, szName, bAdd);
			}else //加入组织结构节点
			{
				int nStatus = ::SkinGetTreeNodeStatus(pSelNode);
				BOOL bAdd = TRUE;
				if (nStatus == 0)
					bAdd = FALSE;
				CInterfaceUserList ulList;
				char szId[32] = {0};
				itoa(pSelData->id, szId, 10);
				int nType = ::atoi(pSelData->szUserName);
				if (SUCCEEDED(pContact->GetUserListByDeptId(szId, &ulList, FALSE, nType)))
				{
					LPORG_TREE_NODE_DATA pData;
					while (SUCCEEDED(ulList.PopBackUserInfo(&pData)))
					{
						if (::stricmp(pData->szUserName, m_strUserName.c_str()) != 0)
						{
							memset(szName, 0, sizeof(TCHAR) * MAX_PATH);
							if (pData->szDisplayName)
								CStringConversion::UTF8ToWideChar(pData->szDisplayName, szName, MAX_PATH - 1);
							else
								CStringConversion::StringToWideChar(pData->szUserName, szName, MAX_PATH - 1);
							//
							if (bAdd)
								m_SelList.AddUserInfo(pData, TRUE, TRUE);
							else
								m_SelList.DeleteUserInfo(pData->szUserName);
							 
							
							DoAdjustSelNode(pContact, hWnd, pData->szUserName, szName, bAdd); 
							if (pData->szDisplayName)
								delete []pData->szDisplayName;
							delete pData;
							 
						} else
						{
							if (pData->szDisplayName)
								delete []pData->szDisplayName;
							delete pData;
						}
					} //end while (SUCCEEDED(
				} //end if (SUCCEEDED(..
				::SkinUpdateControlUI(hWnd, L"selcontact");
			} //end else //加入组织结构节点
		} //end if (::SkinGetSelectTreeNode(..
		pContact->Release();
	} //end if (m_pCore ..
}

HRESULT CBCFrameImpl::DoStatusChanged(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "contacttree") == 0)
	{
		DoContactTreeChanged(hWnd);
	}
	return -1;
}

HRESULT CBCFrameImpl::DoItemSelectEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM)
{
	if ((::stricmp(szName, "cbFontName") == 0) && (!m_bInitFrame))
	{
		TCHAR szTmp[64] = {0};
		int nSize = 63;
		::SkinGetControlTextByName(hWnd, L"cbFontName", szTmp, &nSize);
		char szValue[31] = {0};
		CStringConversion::WideCharToString(szTmp, szValue, 31);
		IConfigure *pCfg = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			pCfg->SetParamValue(FALSE, "ChatFont", "FontName",  szValue);
			RefreshInputChatFont(hWnd, pCfg);
			pCfg->Release();
		}	
	} else if ((::stricmp(szName, "cbFontSize") == 0) && (!m_bInitFrame))
	{
		TCHAR szTmp[64] = {0};
		int nSize = 63;
		::SkinGetControlTextByName(hWnd, L"cbFontSize", szTmp, &nSize);
		char szValue[31] = {0};
		CStringConversion::WideCharToString(szTmp, szValue, 31);
		IConfigure *pCfg = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			pCfg->SetParamValue(FALSE, "ChatFont", "FontSize",  szValue);
			RefreshInputChatFont(hWnd, pCfg);
			pCfg->Release();
		}
	}
	return -1;
}

//ICoreEvent
STDMETHODIMP CBCFrameImpl::DoCoreEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
	                     LPARAM lParam, HRESULT *hResult)
{
	if (::stricmp(szType, "menucommand") == 0)
	{
		*hResult = DoMenuCommand(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "itemselect") == 0)
	{
		*hResult = DoItemSelectEvent(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "click") == 0)
	{
		*hResult = DoClickEvent(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "killfocus") == 0)
	{
		if (::stricmp(szName, "searchedit") == 0)
		{
			IContacts *pContact = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
			{
				pContact->HideHelpEditWindow();
				pContact->Release();
			}
		} //end if (::stricmp(szName, ...
	} else if (::stricmp(szType, "editchanged") == 0)
	{
		if (::stricmp(szName, "searchedit") == 0)
		{
			TCHAR szwText[64] = {0};
			int nSize = 63;
			if (::SkinGetControlTextByName(hWnd, L"searchedit", szwText, &nSize))
			{
				char szText[64] = {0};
				CStringConversion::WideCharToString(szwText, szText, 63);
				char szDest[64] = {0};
				CStringConversion::Trim(szText, szDest);
				RECT rc = {0};
				::SkinGetControlRect(hWnd, L"searchedit", &rc);
				POINT pt = {rc.left, rc.bottom};
				::ClientToScreen(hWnd, &pt);
				IContacts *pContact = NULL;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
				{
					pContact->ShowHelpEditWindow((ICoreEvent *) this, szDest, pt.x, pt.y, rc.right - rc.left, 100);
					pContact->Release();
				}
			} //end if (::
		} //end 
	}  else if (::stricmp(szType, "keydown") == 0)
	{
		if (::stricmp(szName, "searchedit") == 0)
		{
			IContacts *pContact = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
			{
				switch(wParam)
				{
					case VK_RETURN:
					{
						TCHAR szTmp[128] = {0};
						int nSize = 127;				
						char szRealName[128] = {0};
						if (::SkinGetControlTextByName(m_hWnd, L"searchedit", szTmp, &nSize))
						{
							CStringConversion::WideCharToString(szTmp, szRealName, 127);
						} 
						pContact->EditHelpSearchActive(hWnd, szRealName, TRUE, FALSE);
						break;
					}
					case VK_ESCAPE:
					{
						pContact->EditHelpSearchActive(hWnd, NULL, FALSE, FALSE);
						break;
					} 
					case VK_UP:
					case VK_DOWN:
					case VK_PRIOR:
					case VK_NEXT:
					case VK_HOME:
					case VK_END:
					{
						pContact->EditVirtualKeyUp(wParam);
						*hResult = 0;
						break;
					}
				} //end switch(.. 
			} 
		} else if (wParam == VK_ESCAPE)
			::SkinCloseWindow(m_hWnd); //end if (::stricmp(szName...
	} else if (::stricmp(szType, "itemactivate") == 0)
	{
		if (::stricmp(szName, "resultlist") == 0)
		{
			::SkinSetControlTextByName(m_hWnd, L"searchedit", (TCHAR *)wParam);
			IContacts *pContact = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
			{
				DoAdjustSelNode(pContact, m_hWnd, (char *)lParam, (TCHAR *)wParam, TRUE);
				pContact->Release();
			}
		} //
	} else  if (::stricmp(szType, "statuschange") == 0)
	{
		*hResult = DoStatusChanged(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "afterinit") == 0)
	{
		if (::stricmp(szName, "BCFrame") == 0)
		{
			::SkinSetGetKeyFun(hWnd, L"contacttree", GetTreeNodeKey);
			::SkinSetFreeNodeDataCallBack(hWnd, L"contacttree", FreeTreeNodeData);
			::SkinSetGetKeyFun(hWnd, L"selcontact", GetTreeNodeKey);
			::SkinSetFreeNodeDataCallBack(hWnd, L"selcontact", FreeTreeNodeData);
		}
	} else if (::stricmp(szType, "link") == 0)
	{
		if (::strnicmp(szName, "fc_", 3) == 0)
		{
			//file recv
			char szFlag[128] = {0};
			strcpy(szFlag, szName + 3);
			CTransferFileInfo FileInfo;
			if (m_TransFileList.GetFileInfoByFlag(szFlag, FileInfo))
			{
				TCHAR *szwProFlag = new TCHAR[FileInfo.m_strProFlag.GetLength() + 1];
				memset(szwProFlag, 0, sizeof(TCHAR) * (FileInfo.m_strProFlag.GetLength() + 1));
				::lstrcpy(szwProFlag, FileInfo.m_strProFlag.GetData());
				::PostMessage(hWnd, WM_BCRM_FILEPRO, WPARAM(hWnd), LPARAM(szwProFlag));
				m_TransFileList.DeleteFileInfo(FileInfo.m_nLocalFileId);
			} //end if (m_TransFileList...
		} //end if (strnicmp(
	} else if (::stricmp(szType, "enterkeydown") == 0)
	{
		SHORT sCtrl = ::GetKeyState(VK_CONTROL) & 0xF000;
		if ((sCtrl != 0) && (!m_bEnterSend))
		{
			BroadCastMessage();
			*hResult = 0;
		} else if ((sCtrl == 0) && m_bEnterSend)
		{
			BroadCastMessage();
		    *hResult = 0;
		}
	} 
	return E_NOTIMPL;
}

STDMETHODIMP CBCFrameImpl::DoBroadcastMessage(const char *szFromWndName, HWND hWnd,
	                                         const char *szType, const char *szContent, void *pData)
{
	return E_NOTIMPL;
}

STDMETHODIMP CBCFrameImpl::SetCoreFrameWork(ICoreFrameWork *pCore)
{
	if (m_pCore)
		m_pCore->Release();
	m_pCore = pCore;
	if (m_pCore)
	{
		m_pCore->AddRef();
		//order
		m_pCore->AddOrderEvent((ICoreEvent *) this, "MainWindow", "treegroup", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "MainWindow", "upfile", "click");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "MainWindow", "sendsmg", "click");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "BCFrame", NULL, NULL);
	}
	return S_OK;
}

STDMETHODIMP CBCFrameImpl::GetSkinXmlString(IAnsiString *szXmlString)
{
	HRESULT hr = E_FAIL;
	IConfigure *pCfg = NULL; 
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{ 
		hr = pCfg->GetSkinXml("BCFrame.xml",szXmlString); 
		pCfg->Release();
	}
	return hr;   
}

//
STDMETHODIMP CBCFrameImpl::CoreFrameWorkError(int nErrorNo, const char *szErrorMsg)
{
	switch(nErrorNo)
	{
		case CORE_ERROR_LOGOUT:
			{
				if ((m_hWnd != NULL) && (::IsWindow(m_hWnd)))
					::SkinCloseWindow(m_hWnd);
				break;
			}
	}
	return S_OK;
}
 
void CBCFrameImpl::BroadcastFile(const char *szFileName)
{
	if (CSystemUtils::FileIsExists(szFileName))
	{
		if (m_TransFileList.CheckIsTrans("peername", szFileName))
		{
			::SkinMessageBox(m_hWnd, L"文件已在待发送列表中", L"提示", MB_OK);
			return ;
		}
		TCHAR szFlag[MAX_PATH] = {0};
		int nFlagSize = MAX_PATH - 1;
		char szDspName[MAX_PATH] = {0};
		CSystemUtils::ExtractFileName(szFileName, szDspName, MAX_PATH - 1);
		char szFileSize[32] = {0}; 
	    DWORD dwFileSize = (DWORD) CSystemUtils::GetFileSize(szFileName);
	    ::itoa(dwFileSize, szFileSize, 10);	
		//
		if (::SkinAddChildControl(m_hWnd, L"pendingfilelist", m_strFileTransSkinXml.c_str(), szFlag, &nFlagSize, 999999))
		{
			int nFileId = m_TransFileList.AddFileInfo("peername", szDspName, szFileName, szFlag, 
			                            "0", "", "0", "", "0", "0", 0, szFileSize, m_hWnd, TRUE);
			TCHAR szwTmp[MAX_PATH] = {0};

			memset(szwTmp, 0, MAX_PATH * sizeof(TCHAR));
			CStringConversion::StringToWideChar(szDspName, szwTmp, MAX_PATH - 1);
			::SkinSetControlAttr(m_hWnd, szFlag, L"filename", szwTmp);
			memset(szwTmp, 0, MAX_PATH * sizeof(TCHAR));
			CStringConversion::StringToWideChar(szFileSize, szwTmp, MAX_PATH - 1);
			::SkinSetControlAttr(m_hWnd, szFlag, L"filesize", szwTmp);
			::SkinSetControlAttr(m_hWnd, szFlag, L"currfilesize", L"0");
			::SkinSetControlAttr(m_hWnd, szFlag, L"progrestyle", L"offline");
		    ::SkinUpdateControlUI(m_hWnd, szFlag);
		}
		TCHAR szFileCount[16] = {0};
		::_ltow(m_TransFileList.GetCount(), szFileCount, 10);
		::SkinSetControlTextByName(m_hWnd, L"selfilecount", szFileCount);
	}
}

LRESULT CBCFrameImpl::OnWMDropFiles(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (m_nStyle == 3) //群发文件
	{
		HDROP hDrop = (HDROP)wParam;
		if (hDrop == NULL)
			return - 1;

		TCHAR szFile[MAX_PATH]; 
		char szFileName[MAX_PATH] = {0};
		int iFileCount = ::DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
		if (iFileCount <= 5)
		{
			for (int i = 0; i < iFileCount; ++i)
			{
				memset(szFile, 0, MAX_PATH * sizeof(TCHAR));
				if (::DragQueryFile(hDrop, i, szFile, MAX_PATH))
				{
					memset(szFileName, 0, MAX_PATH);
					CStringConversion::WideCharToString(szFile, szFileName, MAX_PATH - 1);
					BroadcastFile(szFileName); 
				} //end if (::DragQueryFile(..
			} //end for (int i
		} else
		{
			::SkinMessageBox(hWnd, L"本应用程序最多只支持同时拖曳5个文件", L"提示", MB_OK); 
		}
		::DragFinish(hDrop);
	}
	return 0;
}

//
STDMETHODIMP CBCFrameImpl::DoWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes)
{
	if (uMsg == WM_DROPFILES)
	{
		OnWMDropFiles(hWnd, wParam, lParam);
	} else if (uMsg == WM_BCSW_FILEPRO)
	{
		CTransferFileInfo Info;
		if (m_TransFileList.GetFileInfoById(wParam, Info))
		{ 
			TCHAR szTmp[20] = {0};
			::_itow(lParam, szTmp, 10);
			::SkinSetControlAttr(m_hWnd, Info.m_strProFlag.GetData(), L"currfilesize", szTmp);						 
		}
	} else if (uMsg == WM_BCRM_FILEPRO)
	{
		HWND h = (HWND)wParam;
		TCHAR *szFlag = (TCHAR *)lParam;
		::SkinRemoveChildControl(h, L"pendingfilelist", szFlag);
		delete []szFlag;
		TCHAR szFileCount[16] = {0};
		::_ltow(m_TransFileList.GetCount(), szFileCount, 10);
		::SkinSetControlTextByName(m_hWnd, L"selfilecount", szFileCount);
		if (m_TransFileList.GetCount() == 0)
			::SkinSetControlEnable(hWnd, L"btnSend", TRUE);
	}
	return E_NOTIMPL;
}

//IProtocolParser
STDMETHODIMP CBCFrameImpl::DoRecvProtocol(const BYTE *pData, const LONG lSize)
{
	return E_NOTIMPL;
}

STDMETHODIMP CBCFrameImpl::DoPresenceChange(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder)
{
	return E_NOTIMPL;
}


//IBCFrame
STDMETHODIMP CBCFrameImpl::ShowBCFrame(const TCHAR *szCaption, LPRECT lprc, int nStyle, IUserList *pUsers)
{
	HRESULT hr = S_OK;
	if (m_strUserName.empty())
	{
		CInterfaceAnsiString strTmp;
		if (SUCCEEDED(m_pCore->GetUserName(&strTmp)))
		{
			m_strUserName = strTmp.GetData();
			if (SUCCEEDED(m_pCore->GetUserDomain(&strTmp)))
			{
				m_strUserName += "@";
				m_strUserName += strTmp.GetData();
			} //end if (SUCCEEDED(m_pCore->GetUserDomain(..
		} //end if (SUCCEEDED(m_pCore->GetUserName(..
	} //end if (m_strUserName...

	m_SelList.Clear();
	m_nStyle = nStyle;
	if (m_hWnd && ::IsWindow(m_hWnd))
	{
		if (::ShowWindow(m_hWnd, SW_SHOW))
		{
			::SetWindowText(m_hWnd, szCaption);
			IContacts *pContact = NULL;
			if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
			{
				TCHAR szText[MAX_PATH];
				if (pUsers)
				{
					LPORG_TREE_NODE_DATA pData;
					while (SUCCEEDED(pUsers->PopFrontUserInfo(&pData)))
					{
						if (::stricmp(pData->szUserName, m_strUserName.c_str()) != 0)
						{
							memset(szText, 0, sizeof(TCHAR) * MAX_PATH - 1);
							if (pData->szDisplayName)
								CStringConversion::UTF8ToWideChar(pData->szDisplayName, szText, MAX_PATH - 1);
							else
							{
								CInterfaceAnsiString szName;
								if (SUCCEEDED(pContact->GetRealNameById(pData->szUserName, NULL, &szName)))
								{
									CStringConversion::UTF8ToWideChar(szName.GetData(), szText, MAX_PATH - 1);
								} else
								{
									CStringConversion::StringToWideChar(pData->szUserName, szText, 63);
								}
							}
							m_SelList.AddUserInfo(pData, TRUE, TRUE);
							::SkinAddTreeChildNode(m_hWnd, L"selcontact", pData->id, NULL, szText, TREENODE_TYPE_LEAF, 
								pData, NULL, NULL, NULL);
						} else
						{
							if (pData->szDisplayName)
								delete []pData->szDisplayName;
							delete pData;
						}
					}
				}
				::SkinExpandTree(m_hWnd, L"selcontact", NULL, TRUE, TRUE);
				pContact->Release();
			}
			CSystemUtils::BringToFront(m_hWnd); 
			hr = S_OK;
		} //
	} else
	{
		if (m_hWnd)
			::SkinCloseWindow(m_hWnd);
		m_hWnd = NULL; 
		m_bInitFrame = TRUE;
		if (m_pCore)
		{
			IUIManager *pUI = NULL;
			IConfigure *pCfg = NULL;		
			hr = m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg);
			if (SUCCEEDED(hr) && pCfg)
			{
				RECT rc = {100, 100, 700, 730};
				if (lprc)
					rc = *lprc;
				else
				{
					RECT rcSave = {0};
					CInterfaceAnsiString strPos;
					if (SUCCEEDED(pCfg->GetParamValue(FALSE, "Position", "BCFrame", (IAnsiString *)&strPos)))
					{
						CSystemUtils::StringToRect(&rcSave, strPos.GetData());
					}
					if (!::IsRectEmpty(&rcSave))
						rc = rcSave;
				}
				hr = m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI);
				if (SUCCEEDED(hr) && pUI)
				{
					pUI->CreateUIWindow(NULL, "BCFrame", &rc, WS_POPUP | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
					                WS_EX_ACCEPTFILES, szCaption, &m_hWnd);				
					if (::IsWindow(m_hWnd))
					{
						 ::ShowWindow(m_hWnd, SW_SHOW);		
						 ::SkinSetControlAttr(m_hWnd, L"logo", L"image", L"701");
			             pUI->OrderWindowMessage("BCFrame", m_hWnd, WM_DROPFILES, (ICoreEvent *) this);
						 pUI->OrderWindowMessage("BCFrame", m_hWnd, WM_BCRM_FILEPRO, (ICoreEvent *) this);
						 pUI->OrderWindowMessage("BCFrame", m_hWnd, WM_BCSW_FILEPRO, (ICoreEvent *) this);
					}
					//pUI->OrderWindowMessage("ContactPanel", NULL, WM_DESTROY, (ICoreEvent *) this);
					// 
					::SkinSetRichEditCallBack(m_hWnd, L"bctext", RichEditCallBack, this);
 					LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
					memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
					pData->bOpened = TRUE;
					pData->id = -1;
				    void *pSaveNode = ::SkinAddTreeChildNode(m_hWnd, L"contacttree", pData->id, NULL,  L"联系人", 
			               TREENODE_TYPE_GROUP, pData, NULL, NULL, NULL);
					IContacts *pContact = NULL;
					if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
					{
						pContact->DrawContactToUI(m_hWnd, L"contacttree", NULL, pSaveNode, FALSE, FALSE, 0);
						::SkinSetControlAttr(m_hWnd, L"contacttree", L"showcheckstatus", L"true");
						::SkinSetControlAttr(m_hWnd, L"selcontact", L"showcheckstatus", L"true");
						::SkinExpandTree(m_hWnd, L"selcontact", NULL, TRUE, TRUE);
                        TCHAR szText[MAX_PATH];
						if (pUsers)
						{
							while (SUCCEEDED(pUsers->PopFrontUserInfo(&pData)))
							{
								if (stricmp(pData->szUserName, m_strUserName.c_str()) != 0)
								{
									memset(szText, 0, sizeof(TCHAR) * MAX_PATH - 1);
									if (pData->szDisplayName)
										CStringConversion::UTF8ToWideChar(pData->szDisplayName, szText, MAX_PATH - 1);
									else
									{
										CInterfaceAnsiString szName;
										if (SUCCEEDED(pContact->GetRealNameById(pData->szUserName, NULL, &szName)))
										{
											CStringConversion::UTF8ToWideChar(szName.GetData(), szText, MAX_PATH - 1);
										} else
										{
											CStringConversion::StringToWideChar(pData->szUserName, szText, 63);
										}
									}
									m_SelList.AddUserInfo(pData, TRUE, TRUE);
									::SkinAddTreeChildNode(m_hWnd, L"selcontact", pData->id, NULL, szText, 
										TREENODE_TYPE_LEAF, pData, NULL, NULL, NULL);
								} else
								{
									if (pData->szDisplayName)
										delete []pData->szDisplayName;
									delete pData;
								}
							} //end while (SUCCEEDED..
						} // end if (pUsers)
						::SkinExpandTree(m_hWnd, L"selcontact", NULL, TRUE, TRUE);
						pContact->Release();
					} 
								//init font name
					HDC dc = ::GetDC(::GetDesktopWindow());
					LOGFONT lf;
					memset (&lf, 0, sizeof(lf));
					lf.lfCharSet	= DEFAULT_CHARSET;
					EnumFontFamiliesEx(dc, &lf, (FONTENUMPROC)DetailsEnumProc, reinterpret_cast<LPARAM>(m_hWnd), 0);
					::ReleaseDC(::GetDesktopWindow(), dc);	 
					//init font size
					TCHAR szTmp[8] = {0};
					for (int i = 8; i < 24; i ++)
					{
						::_itow(i, szTmp, 10);
						::SkinSetDropdownItemString(m_hWnd, L"cbFontSize", 9999, szTmp, NULL);
					}
					IConfigure *pCfg = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
					{
						CInterfaceFontStyle FontStyle;
						pCfg->GetChatFontStyle((IFontStyle *)&FontStyle);
						CInterfaceAnsiString strTmp;
						TCHAR szValue[32] = {0};
						FontStyle.GetName((IAnsiString *)&strTmp);
						CStringConversion::StringToWideChar(strTmp.GetData(), szValue, 31);
						::SkinSetControlTextByName(m_hWnd, L"cbFontName", szValue);
						memset(szValue, 0, sizeof(TCHAR) * 32);
						::_ltow(FontStyle.GetSize(), szValue, 10);
						::SkinSetControlTextByName(m_hWnd, L"cbFontSize", szValue);
						if (FontStyle.GetBold())
							::SkinSetControlAttr(m_hWnd, L"fontbold", L"down", L"true");
						else
							::SkinSetControlAttr(m_hWnd, L"fontbold", L"down", L"false");
						if (FontStyle.GetItalic())
							::SkinSetControlAttr(m_hWnd, L"fontitalic", L"down", L"true");
						else
							::SkinSetControlAttr(m_hWnd, L"fontitalic", L"down", L"false");
						if (FontStyle.GetUnderline())
							::SkinSetControlAttr(m_hWnd, L"fontunderline", L"down", L"true");
						else
							::SkinSetControlAttr(m_hWnd, L"fontunderline", L"down", L"false");
						pCfg->Release();
					}
					CInterfaceAnsiString strValue;
					m_bEnterSend = TRUE;
					if (SUCCEEDED(pCfg->GetParamValue(FALSE, "hotkey", "ctrlentersendmsg", &strValue)))
					{
						if (::stricmp(strValue.GetData(), "true") == 0)
							m_bEnterSend = FALSE;
					}
					pUI->Release();
					pUI = NULL;
				}
				pCfg->Release();
				pCfg = NULL;
				hr = S_OK;
			} //end if (SUCCEEDED(hr)... 
		} //end if (m_pCore) 
		m_bInitFrame = FALSE;
	}
	if (SUCCEEDED(hr))
	{
		switch(m_nStyle)
		{
		case 1: //群发消息
			::SkinSetControlVisible(m_hWnd, L"messagepanel", TRUE);
			::SkinSetControlVisible(m_hWnd, L"sendfilelist", FALSE);
			::SkinSetControlVisible(m_hWnd, L"midtoolbar", TRUE); 
			break;
		case 2: //群发短信
			::SkinSetControlVisible(m_hWnd, L"messagepanel", TRUE);
			::SkinSetControlVisible(m_hWnd, L"sendfilelist", FALSE);
			::SkinSetControlVisible(m_hWnd, L"midtoolbar", FALSE);
			::SkinSetControlVisible(m_hWnd, L"FontSetting", FALSE);
			break;
		case 3: //文件发送
		    ::SkinSetControlVisible(m_hWnd, L"messagepanel", FALSE);
			::SkinSetControlVisible(m_hWnd, L"sendfilelist", TRUE);
			break;
		}
	}
	return hr;
}

//
BOOL CBCFrameImpl::RECallBack(HWND hWnd, DWORD dwEvent, char *szFileName, DWORD *dwFileNameSize,
			  char *szFileFlag, DWORD *dwFileFlagSize, DWORD dwFlag)
{
	switch(dwEvent)
	{
		case RICHEDIT_EVENT_SENDFILE:
			 return FALSE;
		case RICHEDIT_EVENT_GETFILEBYTAG:
			 sprintf(szFileName, "%s%s.gif", GetImagePath(), szFileFlag);
			 return TRUE;
		case RICHEDIT_EVENT_GETCUSTOMPIC:
			{
				IEmotionFrame *pFrame = NULL;
				BOOL bSucc = FALSE;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IEmotionFrame), (void **)&pFrame)))
				{
					CInterfaceAnsiString strFileName;
					if (SUCCEEDED(pFrame->GetSysEmotion(szFileFlag, &strFileName)))
					{
						strcpy(szFileName, strFileName.GetData());
						bSucc = TRUE;
					} else
					{ 
						sprintf(szFileName, "%s%s.gif", GetImagePath(), szFileFlag);
						if (!CSystemUtils::FileIsExists(szFileName))
						{
							if (SUCCEEDED(pFrame->GetDefaultEmotion("sending", &strFileName)))
								strcpy(szFileName, strFileName.GetData());
						} //end if (!CSystemUtils::					  
					} //end else if (
					pFrame->Release();
				}
				
				return bSucc;
			}
		case RICHEDIT_EVENT_GETTIPPIC:
			break;
		case RICHEDIT_EVENT_CUSTOMLINKCLICK:
			{
				 
			}
			break;
	}
	return FALSE;
}

#pragma warning(default:4996)
