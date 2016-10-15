#include <Commonlib/DebugLog.h>
#include <FileTransfer/filetransfer.h>
#include <rmc/remotemachinecontrol.h>
//#include <Media/VideoSDL.h>
#include <Commonlib/systemutils.h>
#include <SmartSkin/smartskin.h>
#include <Core/common.h>
#include "../P2Svr/P2Svr.h"
#include "ChatFrameImpl.h"
#include "../econtacts/econtactsimpl.h"
#include "../IMCommonLib/InterfaceAnsiString.h"
#include "../IMCommonLib/InterfaceFontStyle.h"
#include "../IMCommonLib/InterfaceUserList.h"
#include "../IMCommonLib/MessageList.h"
#include <ShellAPI.h>

#pragma warning(disable:4996)

#define DEFAULT_TRANSFER_TIMEOUT  30000

#define MIN_SHAKE_INTERVAL   10000  //闪动最小间隔

#pragma warning(disable:4996)

std::map<CStdString_, int> g_FontList;

void CChatFrameImpl::TerminatedTransFileByHWND(HWND hOwner, BOOL bAnsy)
{
	std::vector<int> List;
	m_TransFileList.GetOwnerWindowList(hOwner, List);
	while (!List.empty())
	{
		CTransferFileInfo Info;
		if (m_TransFileList.GetFileInfoById(List.back(), Info))
		{
			std::string strXml;
			if (Info.m_bSender)
				strXml = "<trs type=\"filecancel\" from=\"";
			else
				strXml = "<trs type=\"filedecline\" from=\"";
			strXml += m_strUserName;
			strXml += "\" to=\"";
			strXml += Info.m_strPeerName;
			strXml += "\" senderfileid=\"";
			char szFileId[20] = {0};
			if (Info.m_nPeerFileId > 0)
				strXml += ::itoa(Info.m_nPeerFileId, szFileId, 10);
			else
				strXml += ::itoa(Info.m_nLocalFileId, szFileId, 10);
			strXml += "\"/>";
			if (m_pCore)
				m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (int) strXml.size(), 0);
			::CancelFileTrans(List.back());	
			RemoveTransFile(List.back(), NULL, bAnsy); 
		}
		List.pop_back();
	} 
}

BOOL CChatFrameImpl::CanClosed(HWND hWnd)
{
	BOOL bCan = TRUE;
	std::map<HWND, CUserChatFrame *>::iterator it = m_ChatFrameList.find(hWnd);
	if (it != m_ChatFrameList.end())
	{
		if ((m_rmcInfo.m_hOwner == hWnd) 
			&& (m_rmcChlId > 0))
		{
			int n = ::SkinMessageBox(hWnd, L"正在进行远程协助，是否关闭当前远程协助", L"提示", 2/*MBI_OKCANCEL*/);
			if (n == IDOK)
			{
				std::string strXml = "<trs type=\"rtoCancelPleaseControlMe\" from=\"";
				strXml += m_strUserName;
				strXml += "\" to=\"";
				strXml += m_rmcInfo.m_strPeerName;
				strXml += "\" channelid=\"";
				char szTmp[16] = {0};
				strXml += ::itoa(m_rmcInfo.m_nChlId, szTmp, 10);
				strXml += "\"/>";
				if (m_pCore)
					m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (int) strXml.size(), 0);
				ClearRmcChannel();
			} else
				bCan = FALSE;
		}
		int nCount = m_TransFileList.HasOwnerWindow(hWnd);
		if (nCount > 0)
		{
			TCHAR szTmp[MAX_PATH] = {0};
			::wsprintf(szTmp, L"有 %d 个文件尚未发送完毕，是否要关闭", nCount);
			int n = ::SkinMessageBox(hWnd,szTmp, L"提示", 2/*MBI_OKCANCEL*/);
			if (n == IDOK)
			{
				TerminatedTransFileByHWND(hWnd, FALSE);
			} else
				bCan = FALSE;
		} //end if (nCount > 0)
	} //end if (it != m_ChatFrameList.end()
	return bCan;
}

void CChatFrameImpl::ExtractWindowWidth(HWND hWnd, const TCHAR *szName, int nExtractW)
{
	RECT rc = {0};
	::GetWindowRect(hWnd, &rc);
	TCHAR szValue[32] = {0};
	int szValueSize = 31;
	if (::SkinGetControlAttr(hWnd, szName, L"width", szValue, &szValueSize))
	{
		int NewW = ::_wtoi(szValue);
		NewW += nExtractW;
		memset(szValue, 0, 32);
		::_itow(NewW, szValue, 10);
		::SkinSetControlAttr(hWnd, szName, L"width", szValue);
	}
	::MoveWindow(hWnd, rc.left, rc.top, rc.right - rc.left + nExtractW, rc.bottom - rc.top, TRUE);
}

BOOL CChatFrameImpl::SelectFileAndSend(HWND hWnd)
{
	CStringList_ FileList;
	if (CSystemUtils::OpenFileDialog((HINSTANCE)::GetModuleHandle(NULL), hWnd, "选择要发送的文件", "所有文件(*.*)|*.*", 
		                NULL, FileList, FALSE, FALSE))
	{
		std::string strFileName;
		if (!FileList.empty())
			strFileName = FileList.back(); 
		if ((!strFileName.empty()) && CSystemUtils::FileIsExists(strFileName.c_str()))
		{
			return SendFileToPeer(hWnd, strFileName.c_str());							
		} //end if ((!strFileName.empty()) ...
	} //end if (CSystemUtils::OpenFileDialog(...
	return FALSE;
}

BOOL CChatFrameImpl::SendRtoRequest(HWND hWnd)
{
	if (m_rmcChlId == 0)
	{
		//<trs type="RtoRequest" from="admin@gocom" fromname="admin" szkey="a" to="wuxiaozhong@gocom" locip="192.168.1.101" locport="9911" remotip="123.116.122.148" remotport="9911" channelid="3"/>" 
		//
		CUserChatFrame *pFrame = GetChatFrameByHWND(hWnd);
		if (pFrame)
		{
			IContacts *pContact = NULL;
			CInterfaceAnsiString strStatus;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
			{
				CInstantUserInfo Info;
				if (SUCCEEDED(pContact->GetContactUserInfo(pFrame->GetUserName(), &Info)))
				{
					Info.GetUserStatus(&strStatus);
				}
				pContact->Release();
			}
			if ((strStatus.GetSize() == 0) || ::stricmp(strStatus.GetData(), "offline") == 0)
			{
				ShowChatTipMsg(hWnd, L"对方处于离线状态，无法进行远程协助");
				return FALSE;
			}
			if (m_pCore)
			{
				char szKey[MAX_PATH] = {0};
				DWORD nKeySize = MAX_PATH - 1;
				char szLocalIp[20] = {0};
				char szInternetIp[20] = {0};
				WORD wLocalPort = 0, wInternetPort = 0;
				m_rmcChlId = ::RmcStartByControl(m_strTransferIp.c_str(), ::atoi(m_strTransferPort.c_str()), 0, szKey, 
					&nKeySize, this, 30000);
				m_rmcInfo.m_bRequest = TRUE;
				m_rmcInfo.m_strKey = szKey;
				m_rmcInfo.m_hOwner = hWnd;
				m_rmcInfo.m_nChlId = m_rmcChlId;
				
				::RmcGetViewPortInfo(szLocalIp, &wLocalPort, szInternetIp, &wInternetPort);

				
				//
				CStdString_ strTip = _T("您给\"");
				char szTmp[16] = {0};
				std::string strXml = "<trs type=\"RtoRequest\" from=\"";
				strXml += m_strUserName;
				strXml += "\" fromname=\"";
				char szTmpName[MAX_PATH] = {0};
				CStringConversion::UTF8ToString(m_strRealName.c_str(), szTmpName, MAX_PATH - 1);
				TCHAR szwTmpName[128] = {0};
				if (pFrame)
				{
					CStringConversion::UTF8ToWideChar(pFrame->GetDspName(), szwTmpName, MAX_PATH - 1);	
					strTip += szwTmpName;
				}
				strTip += _T("\" 发送了一个远程协助请求，等待对方回应 "); 
	           // wsprintf(szwTmpName, L"\"\n \t <取消,%d>",
				//	            ((m_rmcChlId << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_RMC_REFUSE); 
				//strTip += szwTmpName;
				::SkinRichEditInsertTip(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, NULL, 0, strTip.GetData());

				strXml += szTmpName;
				strXml += "\" szkey=\"";
				strXml += szKey;
				strXml += "\" to=\""; 
				if (pFrame)
				{
					strXml += pFrame->GetUserName();
					m_rmcInfo.m_strPeerName = pFrame->GetUserName();
				}
				strXml += "\" locip=\"";
				strXml += szLocalIp;
				strXml += "\" locport=\"";
				strXml += ::itoa(wLocalPort, szTmp, 10);
				strXml += "\" remotip=\"";
				strXml += szInternetIp;
				strXml += "\" remotport=\"";
				memset(szTmp, 0, 16);
				strXml += ::itoa(wInternetPort, szTmp, 10);
				strXml += "\" channelid=\"";
				memset(szTmp, 0, 16);
				strXml += ::itoa(m_rmcChlId, szTmp, 10);
				strXml += "\"/>";
				if (SUCCEEDED(m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (int) strXml.size(), 0)))
				{
					DoChatCommand(hWnd, CHAT_COMMAND_TAB2RMC_REQUEST);
					return TRUE;
				}
			} //end if (pFrame)
		} //end if (m_pCore)
	} else
		::SkinMessageBox(hWnd, L"远程控制正在进行中，同时只能启动一个连接", L"提示", MB_OK);
	return FALSE;
}

BOOL CChatFrameImpl::SendVideoRequest(HWND hWnd)
{
	if (m_VideoChlId == 0)
	{
//		if (!::VideoCheckDevice())
//			::SkinRichEditInsertTip(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, NULL, 0, L"系统中没有找到视频摄像头设备");
		if (m_pCore)
		{
			DoChatCommand(hWnd, CHAT_COMMAND_TAB2AV);
	        //ExtractWindowWidth(hWnd, L"TagFrame", MEDIA_EXTRACT_WIDTH);	 
			CStdString_ strTip = _T("您给\"");
			TCHAR szwTmp[MAX_PATH] = {0};
			CUserChatFrame *pFrame = GetChatFrameByHWND(hWnd);
			char szKey[MAX_PATH] = {0};
			DWORD nKeySize = MAX_PATH - 1;
			char szLocalIp[20] = {0};
			char szInternetIp[20] = {0};
			WORD wLocalPort = 0, wInternetPort = 0;
			//RECT rcPreview = {300, 460, 460, 580}, rcPlayer = {200, 100, 520, 440 };
			RECT rcPreview = {0}, rcPlayer = {0};
			::SkinGetControlRect(hWnd, L"SelfVideo", &rcPreview);
			::SkinGetControlRect(hWnd, L"PeerVideo", &rcPlayer);
//			m_VideoChlId = ::VideoReady(hWnd, &rcPreview, hWnd, &rcPlayer, m_strTransferIp.c_str(), ::atoi(m_strTransferPort.c_str()),
//				szKey, &nKeySize, this, 10000);
			m_VideoInfo.m_strKey = szKey;
			m_VideoInfo.m_hOwner = hWnd;
			m_VideoInfo.m_nChlId = m_VideoChlId;
			CStringConversion::UTF8ToWideChar(pFrame->GetDspName(), szwTmp, MAX_PATH - 1);	
			strTip += szwTmp;
			strTip += _T("\" 发送视频聊天请求 \""); 	
			
			wsprintf(szwTmp, L"\"\n \t <取消,%d>",
				            ((m_VideoChlId << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_V_REFUSE); 
			strTip += szwTmp;
			::SkinRichEditInsertTip(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, NULL, 0, strTip.GetData());

//			::VideoGetViewPort(szLocalIp, &wLocalPort, szInternetIp, &wInternetPort);
			//
			char szTmp[16] = {0};
			std::string strXml = "<trs type=\"VideoRequest\" from=\"";
			strXml += m_strUserName;
			strXml += "\" fromname=\"";
			char szTmpName[MAX_PATH] = {0};
			CStringConversion::UTF8ToString(m_strRealName.c_str(), szTmpName, MAX_PATH - 1);
			strXml += szTmpName;
			strXml += "\" szkey=\"";
			strXml += szKey;
			strXml += "\" to=\"";
			
			if (pFrame)
			{
				strXml += pFrame->GetUserName();
				m_VideoInfo.m_strPeerName = pFrame->GetUserName();
			}
			strXml += "\" locip=\"";
			strXml += szLocalIp;
			strXml += "\" locport=\"";
			strXml += ::itoa(wLocalPort, szTmp, 10);
			strXml += "\" remotip=\"";
			strXml += szInternetIp;
			strXml += "\" remotport=\"";
			memset(szTmp, 0, 16);
			strXml += ::itoa(wInternetPort, szTmp, 10);
			strXml += "\" channelid=\"";
			memset(szTmp, 0, 16);
			strXml += ::itoa(m_VideoChlId, szTmp, 10);
			strXml += "\"/>";
			return SUCCEEDED(m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (int) strXml.size(), 0));
		} //end if (m_pCore)
	} else
	{
		ShowChatTipMsg(hWnd, L"同时只能启动一个视频通话");
	}//end if (m_VideoChlId == 0)
	return FALSE;
}

BOOL CChatFrameImpl::DoEmotionClick(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	CUserChatFrame *pChat = GetChatFrameByHWND(hWnd);
	if (pChat && pChat->CheckIsOldVersion())
	{
		::SkinRichEditInsertTip(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, NULL, 0, L"对方客户端版本不支持此操作，此消息只有您能看到");
	} else
	{
		char *szFileName = (char *)wParam;
		char *szTagName = (char *)lParam;
		::SkinInsertImageToRichEdit(hWnd, UI_CHARFRAME_INPUT_EDIT_NAME, szFileName, szTagName, -1);
		return TRUE;
	}
	return FALSE;
}

BOOL CChatFrameImpl::DoSendPicture(HWND hWnd)
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
			return ::SkinREInsertOlePicture(hWnd, UI_CHARFRAME_INPUT_EDIT_NAME, strFileName.c_str()); 						
		} //end if ((!strFileName.empty()) ...
	} //end if (CSystemUtils::OpenFileDialog(...
	return FALSE;
}

 
BOOL CChatFrameImpl::FileRecvEvent(HWND hOwner, const char *szFileFlag)
{ 
	CTransferFileInfo FileInfo;
	if (m_TransFileList.GetFileInfoByFlag(szFileFlag, FileInfo))
	{
		char szLocalFileName[MAX_PATH] = {0};
		CInterfaceAnsiString strDefaultPath;
		IConfigure *pCfg = NULL;
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			pCfg->GetPath(PATH_LOCAL_RECV_PATH, &strDefaultPath);
			pCfg->Release();
		}
		strcpy(szLocalFileName, strDefaultPath.GetData());
	    strcat(szLocalFileName, FileInfo.m_strDspName.c_str());
		BOOL bRecv = TRUE;
		if (CSystemUtils::FileIsExists(szLocalFileName))
		{
			bRecv = (::SkinMessageBox(hOwner, L"文件已经存在，是否覆盖？", L"提示", 2) == IDOK);
		}
		if (bRecv)
		{
			return FileRecvToLocal(hOwner, FileInfo, szLocalFileName);
		} else
		{
			return FileSaveAsEvent(hOwner, szFileFlag);
		}
	}
	return FALSE;
}
	
//
BOOL CChatFrameImpl::FileRecvToLocal(HWND hOwner, CTransferFileInfo &FileInfo, const char *szLocalFileName)
{
	char szInternetIp[32] = {0};
	char szIntranetIp[32] = {0};
	WORD wInternetPort = 0;
	WORD wIntranetPort = 0;
    
	//
	CStdString_ strName = L"fr_";
	strName += FileInfo.m_strProFlag;
	::SkinSetControlVisible(hOwner, strName, FALSE);
	
	strName = L"fs_";
	strName += FileInfo.m_strProFlag;
	::SkinSetControlVisible(hOwner, strName, FALSE);
	if (FileInfo.m_RemoteName.empty())
	{
		if (::GetTransPortInfo(szInternetIp, &wInternetPort, szIntranetIp, &wIntranetPort) == 0)
		{ 
			int nLocalFileId = ::ReceiveFileFromUser(m_strTransferIp.c_str(), ::atoi(m_strTransferPort.c_str()), szLocalFileName,
								 FileInfo.m_dwFileSize, FALSE, FileInfo.m_strPeerIntranetIp.c_str(), FileInfo.m_wPeerIntranetPort,
								 FileInfo.m_strPeerInternetIp.c_str(), FileInfo.m_wPeerInternetPort, FileInfo.m_nPeerFileId,
								 FileInfo.m_strFileTag.c_str(), this);
			m_TransFileList.ChangeId(FileInfo.m_nLocalFileId, nLocalFileId, szLocalFileName); 
		    ChangeAllFileSetVisible(hOwner);
			return TRUE;		 
		} else
		{
			ShowChatTipMsg(hOwner, L"获取在线文件服务器失败，建议要求对方发送离线文件");
		}
	} else //离线文件
	{
		std::string strUrl = FileInfo.m_OfflineSvr;
		if (strUrl.empty())
		{
			CInterfaceAnsiString strTmp;
			IConfigure *pCfg = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
			{
				if (SUCCEEDED(pCfg->GetServerAddr(HTTP_SVR_URL_HTTP, &strTmp)))
				{
					strUrl = strTmp.GetData();
				}
			    pCfg->Release();
			}  //end if (SUCCEEDED(m_pCore->
		} //end if (strUrl.empty()
		strUrl += "/offlinefile/";
		strUrl += FileInfo.m_RemoteName;
		m_TransFileList.ChangeId(FileInfo.m_nLocalFileId, FileInfo.m_nLocalFileId, szLocalFileName);	 
		CCustomPicItem *pItem = new CCustomPicItem();
		pItem->m_hOwner =  GetHWNDByUserName(FileInfo.m_strPeerName.c_str());
		pItem->m_strFlag = FileInfo.m_strFileTag;
		pItem->m_pOverlapped = this;
		pItem->m_nFileId = FileInfo.m_nLocalFileId;
		pItem->m_strLocalFileName = szLocalFileName;  
		pItem->m_strPeerName = FileInfo.m_strPeerName; 
		pItem->m_strUrl = strUrl;
		ChangeAllFileSetVisible(hOwner);
		if (m_CustomPics.AddItem(pItem))
		{	
			::P2SvrAddDlTask(strUrl.c_str(), szLocalFileName, FILE_TYPE_NORMAL, 
								pItem, HttpDlCallBack, FALSE);
			return TRUE;
		} else
			delete pItem;  
	}
	return FALSE;
}

BOOL CChatFrameImpl::FileSendOfflineEvent(HWND hOwner, const char *szFileFlag)
{
	CTransferFileInfo FileInfo;
	if (m_TransFileList.GetFileInfoByFlag(szFileFlag, FileInfo))
	{
		std::string strXml;
		char szFileId[16] = {0};
		::itoa(FileInfo.m_nLocalFileId, szFileId, 10);
		//<trs type="filecancel" from="wuxiaozhong@gocom" to="admin@gocom" senderfileid="3"/>
		strXml = "<trs type=\"filecancel\" from=\"";
		strXml += m_strUserName;
		strXml += "\" to=\"";
		strXml += FileInfo.m_strPeerName;
		strXml += "\" senderfileid=\"";
		strXml += szFileId;
		strXml += "\"/>";
		::CancelFileTrans(FileInfo.m_nLocalFileId);
		if (m_pCore)
			m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (int) strXml.size(), 0);
		strXml.empty();  
		//取消本地发送
		CancelFileTrans(FileInfo.m_nLocalFileId);
		m_TransFileList.DeleteFileInfo(FileInfo.m_nLocalFileId);
		std::string strFileName = FileInfo.m_strLocalFileName;
		//离线上传
		UploadLocalFileToServer(hOwner, strFileName.c_str()); 
		//
		TCHAR szwTmp[MAX_PATH] = {0};
		TCHAR szwFileName[MAX_PATH] = {0};
		CStringConversion::StringToWideChar(strFileName.c_str(), szwFileName, MAX_PATH - 1);
		wsprintf(szwTmp, L"文件 %s 转为离线方式发送", szwFileName);
		ShowChatTipMsg(hOwner, szwTmp);
		//从界面上删除
		TCHAR *szwFlag = new TCHAR[64];
		memset(szwFlag, 0, sizeof(TCHAR) * 64);
		CStringConversion::StringToWideChar(szFileFlag, szwFlag, MAX_PATH - 1);
		::PostMessage(GetMainFrameHWND(), WM_CHAT_RMFILE_PRO, WPARAM(hOwner), LPARAM(szwFlag));	
		return TRUE;
	}
	return FALSE;
}

BOOL CChatFrameImpl::FileCancelEvent(HWND hOwner, const char *szFileFlag)
{
	CTransferFileInfo FileInfo;
	if (m_TransFileList.GetFileInfoByFlag(szFileFlag, FileInfo))
	{
		char szFileId[16] = {0};
		std::string strXml;
		if (FileInfo.m_bSender)
		{
			 //<trs type="filecancel" from="wuxiaozhong@gocom" to="admin@gocom" senderfileid="3"/> 
		     ::itoa(FileInfo.m_nLocalFileId, szFileId, 10);
			 strXml = "<trs type=\"filecancel\" from=\"";
			 strXml += m_strUserName;
			 strXml += "\" to=\"";
			 strXml += FileInfo.m_strPeerName;
			 strXml += "\" senderfileid=\"";
			 strXml += szFileId;
			 strXml += "\"/>";
			 ::CancelFileTrans(FileInfo.m_nLocalFileId);
			 RemoveTransFileProgre(FileInfo.m_nLocalFileId, "发送文件 %s 被取消", FALSE); 
		} else
		{
			//<trs type="filedecline" from="admin@gocom" to="wuxiaozhong@gocom" senderfileid="1"/>
			::itoa(FileInfo.m_nPeerFileId, szFileId, 10);
			strXml = "<trs type=\"filedecline\" from=\"";
			strXml += m_strUserName;
			strXml += "\" to=\"";
			strXml += FileInfo.m_strPeerName;
			strXml += "\" senderfileid=\""; 
			strXml += szFileId;
			strXml += "\"/>"; 
			::CancelFileTrans(FileInfo.m_nLocalFileId);
			RemoveTransFileProgre(FileInfo.m_nLocalFileId, "拒绝接收文件 %s", FALSE);
		}
		ChangeAllFileSetVisible(hOwner);
		if (!strXml.empty())
		{
			return SUCCEEDED(m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG)(strXml.size()), 0));
		}
	}
	return FALSE;
}

BOOL CChatFrameImpl::FileSaveAsEvent(HWND hOwner, const char *szFileFlag)
{
	CTransferFileInfo FileInfo;
	if (m_TransFileList.GetFileInfoByFlag(szFileFlag, FileInfo))
	{
		char szLocalFileName[MAX_PATH] = {0};
		CStringList_ FileList;
		BOOL bSucc = FALSE;
		while (TRUE)
		{
			char szTmpFile[MAX_PATH] = {0};
			strncpy(szTmpFile, FileInfo.m_strDspName.c_str(), MAX_PATH - 1);
			if (CSystemUtils::OpenFileDialog(NULL, FileInfo.hOwner, "另存文件为", "所有文件|*.*", szTmpFile,
					            FileList, FALSE, TRUE))
			{
				if (!FileList.empty())
				{ 
					memset(szLocalFileName, 0, MAX_PATH);
					strncpy(szLocalFileName, FileList.back().c_str(), MAX_PATH - 1);
					char szExTmp[MAX_PATH] = {0};
					CSystemUtils::ExtractFileExtName(szLocalFileName, szExTmp, MAX_PATH - 1);
					if (strlen(szExTmp) == 0)
					{
						CSystemUtils::ExtractFileExtName(FileInfo.m_strDspName.c_str(), szExTmp, MAX_PATH - 1);
						::strcat(szLocalFileName, ".");
						::strcat(szLocalFileName, szExTmp);
					}
					if (CSystemUtils::FileIsExists(szLocalFileName))
					{
						if (::SkinMessageBox(hOwner, L"文件已经存在，是否覆盖？", L"提示", 2) == IDOK)
						{
							bSucc = TRUE;
							break;
						} else
						{
							continue;
						}
					} else
					{
						bSucc = TRUE;
						break;
					}
				} else
				{
					return FALSE;
				} //end if (!FileList.empty())
			} else
			{
				return FALSE;
			} //else if (CSystemUtils::OpenFileDialog(NULL,
		} //end while(...
		if (bSucc)
			return FileRecvToLocal(hOwner, FileInfo, szLocalFileName);
	} //end if m_TransFileList...
	return FALSE;
}

//Event
LRESULT CChatFrameImpl::DoClickEvent(HWND hWnd, const char *szCtrlName, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = -1;
    if (::stricmp(szCtrlName, "sendchatmsg") == 0)
	{
		SendMessageToPeer(hWnd);	
		lr = 0;
	} else if (::stricmp(szCtrlName, "filetransfer") == 0)
	{
		SelectFileAndSend(hWnd);
		lr = 0;
	} else if (::stricmp(szCtrlName, "CutScreen") == 0)
	{
		CutScreen(hWnd, FALSE);
		lr = 0;
	} else if (::stricmp(szCtrlName, "rto") == 0)
	{
		SendRtoRequest(hWnd);
		lr = 0;
	} else if (::stricmp(szCtrlName, "emotion") == 0)
	{ 
		POINT pt = {0};
		::GetCursorPos(&pt);
	    IEmotionFrame *pFrame = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IEmotionFrame), (void **)&pFrame)))
		{
			pFrame->ShowEmotionFrame((ICoreEvent *) this, hWnd, pt.x, pt.y);
			pFrame->Release();
		}
	} else if (::stricmp(szCtrlName, "emotionpanel") == 0)
	{
		DoEmotionClick(hWnd, wParam, lParam);
		::SkinSetControlFocus(hWnd, UI_CHARFRAME_INPUT_EDIT_NAME, TRUE);
		//
	} else if (::stricmp(szCtrlName, "shake") == 0)
	{
		//<msg type="p2p" from="%s" to="%s"><shake times="%d" /></msg>
		if ((GetTickCount() - m_dwLastShake) > MIN_SHAKE_INTERVAL)
		{
			CUserChatFrame *pFrame = GetChatFrameByHWND(hWnd);
			if (pFrame)
			{
				std::string strXml = "<msg type=\"p2p\" from=\"";
				strXml += m_strUserName;
				strXml += "\" to=\"";
				strXml += pFrame->GetUserName();
				strXml += "\"><shake times=\"1\"/></msg>";
				if (m_pCore)
					m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG)strXml.size(), 0);
			}
			::SkinRichEditInsertTip(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, NULL, 0, L"您给对方发送了一个闪屏震动");
		} else
		{
			::SkinRichEditInsertTip(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, NULL, 0, L"闪动过于频繁");
		}
		m_dwLastShake = ::GetTickCount();
	} else if (::stricmp(szCtrlName, "picture") == 0)
	{
		DoSendPicture(hWnd);
	} else if (::stricmp(szCtrlName, "imagesetok") == 0)
	{
#define RMC_IMAGE_ENCODE_TYPE_NONE   0  //没有经过编码的裸数据
#define RMC_IMAGE_ENCODE_TYPE_JPG    1  //JPG编码图片
#define RMC_IMAGE_ENCODE_TYPE_AC     2  //AC编码
#define RMC_IMAGE_ENCODE_TYPE_ZLIB   3  //Zlib压缩
#define RMC_IMAGE_ENCODE_TYPE_ACZLIB 4  //先AC 再 zlib压缩
#define RMC_IMAGE_ENCODE_TYPE_256    5  //先转成256色再zlib
		if (m_rmcChlId > 0)
		{
			int nOpenClip = ::SkinGetCheckBoxStatus(hWnd, L"cbOpenClip");
			int nShowCursor = ::SkinGetCheckBoxStatus(hWnd, L"cbShowRemoteMouse");
			BYTE nCodec = RMC_IMAGE_ENCODE_TYPE_JPG;
			if (::SkinGetRadioChecked(hWnd, L"zlibcodec"))
				nCodec = RMC_IMAGE_ENCODE_TYPE_ZLIB;
			else if (::SkinGetRadioChecked(hWnd, L"codec256"))
				nCodec = RMC_IMAGE_ENCODE_TYPE_256;
			//
			::RmcSetImageEncodeType(m_rmcChlId, nCodec);
			if (nOpenClip > 0)
				::RmcEnableClipboard(TRUE);
			else
				::RmcEnableClipboard(FALSE);
			if (nShowCursor > 0)
				::RmcShowRemoteMouseCursor(m_rmcChlId, TRUE);
			else
				::RmcShowRemoteMouseCursor(m_rmcChlId, FALSE);
		}
		::SkinCloseWindow(hWnd);
	} else if (::stricmp(szCtrlName, "imagesetcancel") == 0)
	{
		::SkinCloseWindow(hWnd);
	} else if (::stricmp(szCtrlName, "cancel") == 0) 
	{ 
		::SkinCloseWindow(hWnd);
    } else if (::stricmp(szCtrlName, "disconnectrmc") == 0)
	{ 
		::SkinRichEditInsertTip(m_rmcInfo.m_hOwner, UI_CHATFRAME_DISPLAY_EDIT_NAME, NULL, 0, L"您断开了与对方的远程协助");
		std::string strXml = "<trs type=\"rtoCancelStartResponse\" from=\"";
		strXml +=  m_strUserName;
		strXml += "\" to=\"";
		strXml += m_rmcInfo.m_strPeerName;
		strXml += "\" channelid=\"";
		char szChlId[16] = {0};
		::itoa(m_rmcInfo.m_nChlId, szChlId, 10);
		strXml += szChlId;
		strXml += "\"/>";
		m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG)strXml.size(), 0); 
		ClearRmcChannel();
	} else if (::stricmp(szCtrlName, "rmcfullscreen") == 0)
	{
		if (!m_rmcInfo.m_bRequest)
			RmcFullScreen();
	} else if (::stricmp(szCtrlName, "btnCancelRmc") == 0)
	{
		DoRefuseControl(hWnd);
	} else if (::stricmp(szCtrlName, "btnRefuselRmc") == 0)
	{
		DoRefuseControl(hWnd);
	} else if (::stricmp(szCtrlName, "btnRequestCtrl") == 0)
	{
		if (m_rmcChlId > 0)
			::RmcRequestByControl(m_rmcChlId, 0xFFFFFFFF);
	} else if (::stricmp(szCtrlName, "btnAcceptRmc") == 0)
	{
		DoAcceptControl(hWnd);
	} else if (::stricmp(szCtrlName, "cancelfull") == 0)
	{
		if ((m_rmcInfo.m_nChlId > 0) && (m_rmcInfo.m_hOwner != NULL))
		{
			RECT rc = {0};
			::SkinGetControlRect(m_rmcInfo.m_hOwner, L"rmcShowWindow", &rc);
			::RmcSetNewPlayerHandle(m_rmcChlId, m_rmcInfo.m_hOwner);
			::RmcAdjustPlayRect(m_rmcChlId, &rc);
			CloseRmcFullWindow();
		}
	} else if (::stricmp(szCtrlName, "videobtn") == 0)
	{
		SendVideoRequest(hWnd); 
	} else if (::stricmp(szCtrlName, "rmcsetbtn") == 0)
	{
		SetRmcParam(hWnd);		
	} else if (::stricmp(szCtrlName, "allrecv") == 0)
	{
		AllFileRecv(hWnd);
	} else if (::stricmp(szCtrlName, "allsaveas") == 0)
	{
		AllFileSaveAs(hWnd);
	} else if (::stricmp(szCtrlName, "allrefuse") == 0)
	{
		AllFileRefuse(hWnd);
	} else if (::stricmp(szCtrlName, "fontset") == 0)
	{
		BOOL bVisible = !::SkinGetControlVisible(hWnd, L"FontSetting");
		::SkinSetControlVisible(hWnd, L"FontSetting", bVisible);
		if (bVisible)
			::SkinSetControlAttr(hWnd, L"fontset", L"down", L"true");
		else
			::SkinSetControlAttr(hWnd, L"fontset", L"down", L"false");
	} else if (::stricmp(szCtrlName, "receipt") == 0)
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
	} else if (::stricmp(szCtrlName, "fontbold") == 0)
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
			} //end if (::SkinGetControlAttr(hWnd,
			pCfg->Release();
		}		
	} else if (::stricmp(szCtrlName, "fontitalic") == 0)
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
	} else if (::stricmp(szCtrlName, "fontunderline") == 0)
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
	} else if (::stricmp(szCtrlName, "fontcolor") == 0)
	{
		IConfigure *pCfg = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			CInterfaceAnsiString strCrTmp;
			COLORREF cr = 0;
			if (SUCCEEDED(pCfg->GetParamValue(FALSE, "ChatFont", "FontColor", &strCrTmp)))
				cr = ::atoi(strCrTmp.GetData());
			if (CSystemUtils::OpenColorDialog(NULL, hWnd, cr))
			{ 
				char szValue[16] = {0};
				::itoa(cr, szValue, 10);
				pCfg->SetParamValue(FALSE, "ChatFont", "FontColor",  szValue);
				RefreshInputChatFont(hWnd, pCfg); 
			}
			pCfg->Release();
		}
	} else if (::stricmp(szCtrlName, "PeerHeader") == 0)
	{
		IMiniCard *pCard = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMiniCard), (void **)&pCard)))
		{
			CInterfaceAnsiString strUserName;
			if (SUCCEEDED(GetUserNameByHWND(hWnd, &strUserName)))
			{
				POINT pt = {0};
				::GetCursorPos(&pt);
				RECT rc = {0};
				::GetWindowRect(hWnd, &rc);

				pCard->ShowMiniCard(strUserName.GetData(), rc.right + 2, pt.y, 2);
			}
			pCard->Release();
		}
	} else if (::stricmp(szCtrlName, "SelfHeader") == 0)
	{
		IConfigureUI *pUI = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigureUI), (void **)&pUI)))
		{
			pUI->Navegate2Frame(NULL, "personpage");
			pUI->Release();
		}
	} else if (::stricmp(szCtrlName, "acceptbutton") == 0)
	{
		//
		TCHAR szTmp[MAX_PATH] = {0};
		int nSize = MAX_PATH - 1;
		if (::SkinGetControlTextByName(hWnd, L"peerusername", szTmp, &nSize))
		{
			char szUserName[MAX_PATH] = {0};
			CStringConversion::WideCharToString(szTmp, szUserName, MAX_PATH - 1);
			SendReceiptToPeer(szUserName, "对方接受了您的会话请求", "acceptchat");
			IFreContacts *pContact = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IFreContacts), (void **)&pContact)))
			{
				pContact->AddFreContactUser("0", szUserName, NULL, NULL, "1");
				pContact->Release();
			}
		}
		::SkinCloseWindow(hWnd);
		lr = 0;
	} else if (::stricmp(szCtrlName, "declinebutton") == 0)
	{
		TCHAR szTmp[MAX_PATH] = {0};
		int nSize = MAX_PATH - 1;
		if (::SkinGetControlTextByName(hWnd, L"peerusername", szTmp, &nSize))
		{
			char szUserName[MAX_PATH] = {0};
			CStringConversion::WideCharToString(szTmp, szUserName, MAX_PATH - 1);
			SendReceiptToPeer(szUserName, "对方拒绝了您的会话请求", "declinechat");
		}
		::SkinCloseWindow(hWnd);
		lr = 0;
	} else if (::stricmp(szCtrlName, "sendsms") == 0)
	{
		//
		SendSmsByHWND(hWnd);
	} else if (::stricmp(szCtrlName, "closetip") == 0)
	{
		::SkinSetControlVisible(hWnd, L"offlinetip", FALSE);
	}
	return lr;
}


void CChatFrameImpl::AllFileSaveToPath(HWND hWnd, const char *szPath)
{
	std::vector<int> flList;
	m_TransFileList.GetPendingRecvFileList(hWnd, flList);
	char szRecvPath[MAX_PATH] = {0};
	CSystemUtils::IncludePathDelimiter(szPath, szRecvPath, MAX_PATH - 1);
	int nFileId = 0;
	CTransferFileInfo FileInfo;
	std::string strFileName; 
	while (!flList.empty())
	{
		nFileId = flList.back();
		if (m_TransFileList.GetFileInfoById(nFileId, FileInfo))
		{
			strFileName = szRecvPath;
			strFileName += FileInfo.m_strDspName;
			FileRecvToLocal(hWnd, FileInfo, strFileName.c_str());
		} //end if (..
		flList.pop_back();
	} //end while
	ChangeAllFileSetVisible(hWnd);
}

void CChatFrameImpl::AllFileRecv(HWND hWnd)
{
	CInterfaceAnsiString strDefaultPath;
	IConfigure *pCfg = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		pCfg->GetPath(PATH_LOCAL_RECV_PATH, &strDefaultPath);
		pCfg->Release();
	}
	AllFileSaveToPath(hWnd, strDefaultPath.GetData());
}

void CChatFrameImpl::AllFileSaveAs(HWND hWnd)
{
	char szPath[MAX_PATH] = {0};
	if (CSystemUtils::SelectFolder(hWnd, szPath))
	{
		AllFileSaveToPath(hWnd, szPath);
	} 
}

void CChatFrameImpl::AllFileRefuse(HWND hWnd)
{
	std::vector<int> flList;
	m_TransFileList.GetPendingRecvFileList(hWnd, flList); 
	CTransferFileInfo FileInfo; 
	std::string strXml; 
	char szFileId[16] = {0};
	while (!flList.empty())
	{
		if (m_TransFileList.GetFileInfoById(flList.back(), FileInfo))
		{
		   //<trs type="filedecline" from="admin@gocom" to="wuxiaozhong@gocom" senderfileid="1"/>
			strXml = "<trs type=\"filedecline\" from=\"";
			strXml += m_strUserName;
			strXml += "\" to=\"";
			strXml += FileInfo.m_strPeerName;
			strXml += "\" senderfileid=\"";
			memset(szFileId, 0, 16);
			::itoa(FileInfo.m_nPeerFileId, szFileId, 10);
			strXml += szFileId;
			strXml += "\"/>"; 
			RemoveTransFileProgre(FileInfo.m_nLocalFileId, "拒绝接收文件 %s", FALSE);
			m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG)(strXml.size()), 0);
		} 
		flList.pop_back();
	} 
	ChangeAllFileSetVisible(hWnd);
}

void CChatFrameImpl::SetRmcParam(HWND hWnd)
{
	IUIManager *pUI = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
	{
		HWND h = NULL;
		RECT rc = {0};
		RECT rcScreen = {0};
		static int RMC_SET_MODAL_WINDOW_WIDTH  = 360;
		static int RMC_SET_MODAL_WINDOW_HEIGHT = 300;
		CSystemUtils::GetScreenRect(&rcScreen);
		rc.left = (rcScreen.right - RMC_SET_MODAL_WINDOW_WIDTH) / 2;
		rc.right = rc.left + RMC_SET_MODAL_WINDOW_WIDTH;
		rc.top = (rcScreen.bottom - RMC_SET_MODAL_WINDOW_HEIGHT) / 2;
		rc.bottom = rc.top + RMC_SET_MODAL_WINDOW_HEIGHT;
		if (SUCCEEDED(pUI->CreateUIWindow(hWnd, "rmcimagesetwindow", &rc, WS_POPUP, NULL, L"远程协助设置", &h)))
		{
			if (::RmcGetClipboardEnabled())
				::SkinSetCheckBoxStatus(h, L"cbOpenClip", 1);
			else
				::SkinSetCheckBoxStatus(h, L"cbOpenClip", 0); 
			if (::RmcIsShowRemoteMouseCursor(m_rmcChlId))
				::SkinSetCheckBoxStatus(h, L"cbShowRemoteMouse", 1);
			else
				::SkinSetCheckBoxStatus(h, L"cbShowRemoteMouse", 0);
			BYTE byteCodec = ::RmcGetImageEncodeType(m_rmcChlId);
			switch(byteCodec)
			{
			case RMC_IMAGE_ENCODE_TYPE_JPG:
				::SkinSetRadioChecked(h, L"jpgcodec", TRUE);
				break;
			case RMC_IMAGE_ENCODE_TYPE_ZLIB:
				::SkinSetRadioChecked(h, L"zlibcodec", TRUE);
				break;
			case RMC_IMAGE_ENCODE_TYPE_256:
				::SkinSetRadioChecked(h, L"codec256", TRUE);
				break;
			} 
			//
			::SkinShowModal(h);
		}
		pUI->Release();
	}
}

void CChatFrameImpl::RmcFullScreen()
{
	if (m_rmcInfo.m_nChlId > 0)
	{ 	
		RECT rcScreen = {0};
		CSystemUtils::GetScreenRect(&rcScreen);
		IUIManager *pUI = NULL;
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
		{ 
			//
			static int RMC_SET_WINDOW_WIDHT = 240; 
			m_rmcInfo.m_rcSet.left = (rcScreen.right - RMC_SET_WINDOW_WIDHT) / 2; 
			m_rmcInfo.m_rcSet.right = m_rmcInfo.m_rcSet.left + RMC_SET_WINDOW_WIDHT;
			m_rmcInfo.m_rcSet.bottom = 26;
			m_rmcInfo.m_rcSet.top = 0;
			pUI->CreateUIWindow(NULL, "rmcsetwindow", &m_rmcInfo.m_rcSet, WS_POPUP, WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
				L"", &m_rmcInfo.m_hSetWnd);
			pUI->Release();
		} //end if (m_pCore.. 
		::RmcFullScreen(m_rmcChlId, m_rmcInfo.m_hSetWnd, &m_rmcInfo.m_rcSet); 
		RECT rc = {0, 0, m_rmcInfo.m_nWidth, m_rmcInfo.m_nHeight};
		::RmcAdjustPlayRect(m_rmcChlId, &rcScreen);
	} // end if (m_rmcInfo.m_nChlId > 0
}

BOOL CChatFrameImpl::SendFileToPeer(HWND hWnd, const char *szLocalFileName)
{
//
	CUserChatFrame *pFrame = GetChatFrameByHWND(hWnd);
	if (!pFrame)
		return FALSE;
	if (m_TransFileList.CheckIsTrans(pFrame->GetUserName(), szLocalFileName))
	{
		ShowChatTipMsg(hWnd, L"文件正在传送中");
		return FALSE;
	}
	if (m_pCore)
	{
		if (m_pCore->CanAllowAction(USER_ROLE_SEND_FILE) != S_OK)
		{
			ShowChatTipMsg(hWnd, L"没有发送文件权限");
			return FALSE;
		}
	}
	char szLocalIp[32] = {0};
	char szInternetIp[32] = {0};
	WORD wLocalPort = 0, wInternetPort = 0;
	if (::GetTransPortInfo(szLocalIp, &wLocalPort, szInternetIp, &wInternetPort) == 0)
	{ 
		DWORD dwFileSize = (DWORD) CSystemUtils::GetFileSize(szLocalFileName);
		int nMaxFileSize = 0;
		IConfigure *pCfg = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			CInterfaceAnsiString strTmp;
			if (SUCCEEDED(m_pCore->GetSvrParams("online.maxfilesize", &strTmp, FALSE)))
			{
				nMaxFileSize = ::atoi(strTmp.GetData());
			}
			pCfg->Release();
		}
		if ((nMaxFileSize != 0) && (dwFileSize > nMaxFileSize))
		{
			TCHAR szwTmp[MAX_PATH] = {0};
			::wsprintf(szwTmp, L"文件太大，单个在线文件大小不能超过 %dK", nMaxFileSize / 1024);
			ShowChatTipMsg(hWnd, szwTmp);
			return FALSE;
		}
		char szDspName[MAX_PATH] = {0};
		char szTag[MAX_PATH] = {0};
		char szFileId[32] = {0};
		int nTagSize = MAX_PATH - 1;
		::GetFileTagByName(szLocalFileName, szTag, &nTagSize);
		char szFileSize[32] = {0};
		char szTmp[32] = {0};
		::itoa(dwFileSize, szFileSize, 10);					
		CSystemUtils::ExtractFileName(szLocalFileName, szDspName, MAX_PATH - 1);
		int nFileId = ::TransFileReady(m_strTransferIp.c_str(), ::atoi(m_strTransferPort.c_str()), szLocalFileName,
			dwFileSize, this, DEFAULT_TRANSFER_TIMEOUT);
		//add progress
		TCHAR szFlag[MAX_PATH] = {0};
		TCHAR szwTmp[MAX_PATH] = {0};
		int nFlagSize = MAX_PATH - 1;
		::SkinAddChildControl(hWnd, L"fileprogress", m_strFileTransSkinXml.c_str(), szFlag, &nFlagSize, 999999);
		m_TransFileList.AddFileInfo(pFrame->GetUserName(), szDspName, szLocalFileName, szFlag, 
			                            szTag, "", "0", "", "0", "0", nFileId, szFileSize, hWnd, TRUE);
		DoChatCommand(hWnd, CHAT_COMMAND_TAB2FILE_PRO);
		memset(szwTmp, 0, MAX_PATH * sizeof(TCHAR));
		CStringConversion::StringToWideChar(szDspName, szwTmp, MAX_PATH - 1);
		::SkinSetControlAttr(hWnd, szFlag, L"filename", szwTmp);
		memset(szwTmp, 0, MAX_PATH * sizeof(TCHAR));
		CStringConversion::StringToWideChar(szFileSize, szwTmp, MAX_PATH - 1);
		::SkinSetControlAttr(hWnd, szFlag, L"filesize", szwTmp);
		::SkinSetControlAttr(hWnd, szFlag, L"currfilesize", L"0"); 
		::SkinSetControlAttr(hWnd, szFlag, L"progrestyle", L"send");
		::SkinUpdateControlUI(hWnd, szFlag);
	    // <trs type="FileRequest" from="wuxiaozhong@gocom" fromname="邬晓忠" 
                //   filename="Default.rdp" filetag="30f79afad4318774e447dc2db96936e0" filesize="2000"
                //   to="admin@gocom" locip="192.168.1.100" locport="4800" remotip="123.116.125.172"
                //   remotport="4800" senderfileid="4"/>
		::itoa(nFileId, szFileId, 10);
		std::string strXml;
		strXml = "<trs type=\"FileRequest\" from=\"";
		strXml += m_strUserName;
		strXml += "\" fromname=\"";
		strXml += pFrame->GetDspName();
		strXml += "\" filename=\"";
		strXml += szDspName;
		strXml += "\" filetag=\"";
		strXml += szTag;
		strXml += "\" filesize=\"";
		strXml += szFileSize;
		strXml += "\" to=\"";
		strXml += pFrame->GetUserName();
		strXml += "\" locip=\"";
		strXml += szLocalIp;
		strXml += "\" locport=\"";
		memset(szTmp, 0, 32);
		strXml += ::itoa(wLocalPort, szTmp, 10);
		strXml += "\" remotip=\"";
		memset(szTmp, 0, 32);
		strXml += szInternetIp;
		strXml += "\" remotport=\"";
		strXml += ::itoa(wInternetPort, szTmp, 10);
		strXml += "\" senderfileid=\"";
		strXml += szFileId;
		strXml += "\"/>";
		if (m_pCore)
		{ 
			m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (int) strXml.size(), 0);
			//draw to ui
			CStdString_ strTip = _T("您给\"");
			TCHAR szTmp[MAX_PATH] = {0};
			CStringConversion::UTF8ToWideChar(pFrame->GetDspName(), szTmp, MAX_PATH - 1);	
			strTip += szTmp;
			strTip += _T("\" 发送文件 \"");
			memset(szTmp, 0, MAX_PATH * sizeof(TCHAR));
			CStringConversion::StringToWideChar(szDspName, szTmp, MAX_PATH - 1);
			strTip += szTmp;
			strTip += _T("\" 请求,");						
			
			/*wsprintf(szTmp, L"\"\n \t <取消,%d>,<发送离线文件,%d>",
				            ((nFileId << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_CANCEL, 
				            ((nFileId << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_OFFLINE);
			strTip + szTmp;
			::SkinRichEditInsertTip(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, NULL, 0, strTip.GetData());*/
			return TRUE;
		} //end if (m_pCore)
	}  else
	{
		::SkinRichEditInsertTip(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, NULL, 0, L"获取文件服务器失败，将采用离线方式传送文件");
		UploadLocalFileToServer(hWnd, szLocalFileName);
		//
	}//end if (::GetTransPortInfo(...	
	return FALSE;
}

//
LRESULT CChatFrameImpl::DoLayoutEvent(HWND hWnd, const char *szCtrlName, WPARAM wParam, LPARAM)
{
	if (::stricmp(szCtrlName, "SelfVideo") == 0)
	{
		if ((m_VideoChlId > 0) && (m_VideoInfo.m_hOwner == hWnd))
		{
			RECT rc = {0};
			::SkinGetControlRect(hWnd, L"SelfVideo", &rc);
//			::VideoSetPreviewRect(&rc); 
		}
	} else if (stricmp(szCtrlName, "PeerVideo") == 0)
	{
		if ((m_VideoChlId > 0) && (m_VideoInfo.m_hOwner == hWnd))
		{
			RECT rc = {0};
			::SkinGetControlRect(hWnd, L"PeerVideo", &rc);
		//	::VideoSetPlayerRect(m_VideoChlId, &rc);
			PRINTDEBUGLOG(dtInfo, "adjust peer video rect(left:%d top:%d right:%d bottom:%d)",
				rc.left, rc.top, rc.right, rc.bottom);
		}
	} else if (stricmp(szCtrlName, "rmcShowWindow") == 0)
	{
		if ((m_rmcChlId > 0) && (m_rmcInfo.m_hOwner == hWnd))
		{
			RECT rc = {0};
			::SkinGetControlRect(hWnd, L"rmcShowWindow", &rc);
			::RmcAdjustPlayRect(m_rmcChlId, &rc);
		}
	} else if (stricmp(szCtrlName, "rmcWorkArea") == 0)
	{
		if ((m_rmcChlId > 0) && (m_rmcInfo.m_hFullScreen != NULL))
		{
			RECT rc = {0};
			::SkinGetControlRect(m_rmcInfo.m_hFullScreen, L"rmcWorkArea", &rc);
			::RmcAdjustPlayRect(m_rmcInfo.m_nChlId, &rc);
		}
	}
	return -1;
}

//
LRESULT CChatFrameImpl::DoItemSelectEvent(HWND hWnd, const char *szCtrlName, WPARAM wParam, LPARAM lParam)
{
	if ((::stricmp(szCtrlName, "cbFontName") == 0) && (!m_bInitFrame))
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
	} else if ((::stricmp(szCtrlName, "cbFontSize") == 0) && (!m_bInitFrame))
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

void CChatFrameImpl::CloseRmcFullWindow()
{
	if (m_rmcInfo.m_hSetWnd != NULL)
	{
		::SkinCloseWindow(m_rmcInfo.m_hSetWnd);
		m_rmcInfo.m_hSetWnd = NULL;
	}
	if (m_rmcInfo.m_hFullScreen != NULL)
	{
		::SkinCloseWindow(m_rmcInfo.m_hFullScreen);
		m_rmcInfo.m_hFullScreen = NULL;
	}

}

HWND CChatFrameImpl::OpenChatFrameByCollegeTree(HWND hWnd)
{
	void *pSelNode = NULL;
	void *pSelData = NULL;
	TCHAR szName[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	CTreeNodeType tnType;
	if (::SkinGetSelectTreeNode(hWnd, UI_COMPANY_TREE_NAME_WCHAR, szName, &nSize, &pSelNode, &tnType, &pSelData))
	{
		if (tnType == TREENODE_TYPE_LEAF)
		{
			if (pSelData)
			{
				LPORG_TREE_NODE_DATA pData = (LPORG_TREE_NODE_DATA) pSelData;
				ShowChatFrame(hWnd, pData->szUserName, pData->szDisplayName);
				return GetHWNDByUserName(pData->szUserName);
				//
			} //end if (pSelData)
		} //end if (tnType == TREENODE_TYPE_LEAF)
	} //end if (::GetSelectTreeNode(hWnd..	
	return NULL;
}


LRESULT CChatFrameImpl::DoDblClkEvent(HWND hWnd, const char *szCtrlName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szCtrlName, UI_COMPANY_TREE_NAME_STR) == 0)
	{
		OpenChatFrameByCollegeTree(hWnd);
		return -1;
	} //end if (::stricmp(szName, "colleaguetree")
	return 0;
}
int CALLBACK DetailsEnumProc (const ENUMLOGFONTEX *lpelfe, const NEWTEXTMETRICEX *lpntme, unsigned long FontType, LPARAM lParam) 
{ 
	HWND h = reinterpret_cast<HWND> (lParam);
	if ((FontType == TRUETYPE_FONTTYPE)
		&& (lpelfe->elfFullName[0] != L'@'))
	{
		g_FontList[lpelfe->elfFullName] = 0;
	}
		
	return 1;
}


LRESULT CChatFrameImpl::DoAfterInitEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "chatwindow") == 0)
	{
		//init font name
		if (g_FontList.empty())
		{
			HDC dc = ::GetDC(::GetDesktopWindow());
			LOGFONT lf;
			memset (&lf, 0, sizeof(lf));
			lf.lfCharSet	= DEFAULT_CHARSET;
			EnumFontFamiliesEx(dc, &lf, (FONTENUMPROC)DetailsEnumProc, reinterpret_cast<LPARAM>(hWnd), 0);
			::ReleaseDC(::GetDesktopWindow(), dc);	 
		}
		//
		std::map<CStdString_, int>::iterator it;
		for (it = g_FontList.begin(); it != g_FontList.end(); it ++)
		{
			::SkinSetDropdownItemString(hWnd, L"cbFontName", -1,  it->first, NULL);
		}
		//init font size
		TCHAR szTmp[8] = {0};
		for (int i = 8; i < 24; i ++)
		{
			::_itow(i, szTmp, 10);
			::SkinSetDropdownItemString(hWnd, L"cbFontSize", 9999, szTmp, NULL);
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
			::SkinSetControlTextByName(hWnd, L"cbFontName", szValue);
			memset(szValue, 0, sizeof(TCHAR) * 32);
			::_ltow(FontStyle.GetSize(), szValue, 10);
			::SkinSetControlTextByName(hWnd, L"cbFontSize", szValue);
			if (FontStyle.GetBold())
				::SkinSetControlAttr(hWnd, L"fontbold", L"down", L"true");
			else
				::SkinSetControlAttr(hWnd, L"fontbold", L"down", L"false");
			if (FontStyle.GetItalic())
				::SkinSetControlAttr(hWnd, L"fontitalic", L"down", L"true");
			else
				::SkinSetControlAttr(hWnd, L"fontitalic", L"down", L"false");
			if (FontStyle.GetUnderline())
				::SkinSetControlAttr(hWnd, L"fontunderline", L"down", L"true");
			else
				::SkinSetControlAttr(hWnd, L"fontunderline", L"down", L"false");
			pCfg->Release();
		}
		IUIManager *pUI = NULL;
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
		{  				
			pUI->OrderWindowMessage("chatwindow", hWnd, WM_KEYDOWN, (ICoreEvent *) this); 
			pUI->Release();
		}
	} else if (::stricmp(szName, "mainwindow") == 0)
	{
		IUIManager *pUI = NULL;
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
		{  				
			pUI->OrderWindowMessage("mainwindow", NULL, WM_OPENCHATFRAME, (ICoreEvent *) this);
			pUI->OrderWindowMessage("mainwindow", NULL, WM_VIDEO_CONNECTED, (ICoreEvent *) this);
			pUI->OrderWindowMessage("mainwindow", NULL, WM_SHOWCHATMESSAGE, (ICoreEvent *) this);
		    pUI->OrderWindowMessage("mainwindow", NULL, WM_SHOWCHATTIPMSG, (ICoreEvent *) this);
			pUI->OrderWindowMessage("mainwindow", NULL, WM_CHAT_UPDL_PROGR, (ICoreEvent *) this);
			pUI->OrderWindowMessage("mainwindow", NULL, WM_CHAT_RMFILE_PRO, (ICoreEvent *) this); 
			pUI->OrderWindowMessage("mainwindow", NULL, WM_CHAT_APPEND_PRO, (ICoreEvent *) this);
			pUI->OrderWindowMessage("mainwindow", NULL, WM_CHAT_COMMAND, (ICoreEvent *) this);
			pUI->OrderWindowMessage("mainwindow", NULL, WM_SHOW_FILE_LINK, (ICoreEvent *) this);
			pUI->Release();
		}
	} //
	return -1;
}

//
LRESULT CChatFrameImpl::DoMenuClickEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "cutmenu") == 0)
	{
		if (wParam == 20001) //截屏菜单ID
		{
			CutScreen(hWnd, FALSE);
		} else if (wParam == 20002) //隐藏再截屏
		{
			CutScreen(hWnd, TRUE);
		}
	} else if (::stricmp(szName, "chatshortcutmenu") == 0)
	{
		if (wParam == 100)
		{
			COLORREF cr;
			if (CSystemUtils::OpenColorDialog(NULL, hWnd, cr))
			{
				if (m_pCore)
				{
					IUIManager *pUI = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
					{
						if (SUCCEEDED(pUI->BlendSkinStyle(cr)))
						{
							IConfigure *pCfg = NULL;
							if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
							{
								char szTmp[16] = {0};
								::itoa(cr, szTmp, 10);
								pCfg->SetParamValue(FALSE, "Skin", "background", szTmp);
								pCfg->Release();
							} //end if (SUCCEEDED(m_pCore->...
						} //end if (SUCCEEDED(pUI->...
						pUI->Release();
					} //end if (SUCCEEDED(m_pCore->
				} //end if (m_pCore)
			} //end if (CSystemUtils::OpenColorDialog(NULL... 
		} else if (wParam == 101) //智能合并
		{
			IConfigure *pCfg = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
			{
				BOOL bChecked = (BOOL) lParam;
				if (bChecked)
				{
					::SkinSetControlAttr(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, L"mergemsg", L"true");
					pCfg->SetParamValue(FALSE, "normal", "aimsg", "true");
				} else
				{
					::SkinSetControlAttr(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, L"mergemsg", L"false");
					pCfg->SetParamValue(FALSE, "normal", "aimsg", "false");
				}
				pCfg->Release();
				::SkinUpdateControlUI(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME);
			}
		} else if (wParam == 102) //透明背景
		{
			IConfigure *pCfg = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
			{
				BOOL bChecked = (BOOL) lParam;
				if (bChecked)
				{
					::SkinSetControlAttr(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, L"transparent", L"true");
					CInterfaceAnsiString strValue;
					if (SUCCEEDED(pCfg->GetParamValue(FALSE, "normal", "transimagefile", &strValue)))
					{
						TCHAR szTmp[MAX_PATH] = {0};
						CStringConversion::StringToWideChar(strValue.GetData(), szTmp, MAX_PATH - 1);
						::SkinSetControlAttr(hWnd, L"chatdisplaycanvs", L"imagefile", szTmp);
					}
					pCfg->SetParamValue(FALSE, "normal", "msgtransparent", "true");
				} else
				{
				    ::SkinSetControlAttr(hWnd, L"chatdisplaycanvs", L"imagefile", NULL);
					::SkinSetControlAttr(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, L"transparent", L"false");
					pCfg->SetParamValue(FALSE, "normal", "msgtransparent", "false");
				}
				pCfg->Release();
				::SkinUpdateControlUI(hWnd, L"chatdisplaycanvs");
			}
		}
	} else if (::stricmp(szName, "treeleaf") == 0)
	{
		switch(wParam)
		{
		case 20:  //发送消息
			{
				OpenChatFrameByCollegeTree(hWnd);
				break;
			}
		case 4: //发送文件
			{
				HWND h = OpenChatFrameByCollegeTree(hWnd);
				if (h != NULL)
				{
					SelectFileAndSend(h);
				}
				//
				break;
			}
		case 6: //远程协助
			{
				HWND h = OpenChatFrameByCollegeTree(hWnd);
				if (h != NULL)
					SendRtoRequest(h);
				//
				break;
			}
		case 7: //视频会话
			{
				HWND h = OpenChatFrameByCollegeTree(hWnd);
				if (h != NULL)
					SendVideoRequest(h);
				break;
			}
		case 8: //语音会话
			{
				OpenChatFrameByCollegeTree(hWnd);
				break;
			}
		case 30: //发送邮件
			{
				SendMailToPeerByTreeMenu(hWnd);
				break;
			}
		}
	} else if (::stricmp(szName, "sendmenu") == 0)
	{
		switch(wParam)
		{
		case 30001:
			 m_bEnterSend = TRUE;
			 break;
		case 30002:
			 m_bEnterSend = FALSE;
			 break;
		}
		IConfigure *pCfg = NULL;
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			if (m_bEnterSend)
			{
				pCfg->SetParamValue(FALSE, "hotkey", "entersendmsg", "true");
				pCfg->SetParamValue(FALSE, "hotkey", "ctrlentersendmsg", "false");
			} else
			{
				pCfg->SetParamValue(FALSE, "hotkey", "ctrlentersendmsg", "true");
				pCfg->SetParamValue(FALSE, "hotkey", "entersendmsg", "false");
			}
			pCfg->Release();
		}
	} else if (::stricmp(szName, "phrasemenu") == 0) //短语
	{
		TCHAR szwCaption[MAX_PATH] = {0};
		int nSize = MAX_PATH - 1;
		if (::SkinMenuGetItemCaption(hWnd, L"phrasemenu", wParam, szwCaption, &nSize))
		{
			::SkinSetControlAttr(hWnd, UI_CHARFRAME_INPUT_EDIT_NAME, L"text", szwCaption);
			SendMessageToPeer(hWnd);
		}
	} else if (::stricmp(szName, "RichEditReadOnlyMenu") == 0) //显示框的右键菜单
	{
		switch(wParam)
		{
		case 20: //复制
			::SkinRichEditCommand(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, "copy", NULL);
			break;
		case 21: //全选
			::SkinRichEditCommand(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, "selectall", NULL);
			break;
		case 26:
			::SkinRichEditCommand(hWnd, UI_CHATFRAME_DISPLAY_EDIT_NAME, "clear", NULL);
			break;
		}
	} else if (::stricmp(szName, "RichEditPopMenu") == 0) //输入框的右键菜单
	{
		switch(wParam)
		{
		case 20: //复制
			::SkinRichEditCommand(hWnd, UI_CHARFRAME_INPUT_EDIT_NAME, "copy", NULL);
			break;
		case 22: //剪切
			::SkinRichEditCommand(hWnd, UI_CHARFRAME_INPUT_EDIT_NAME, "cut", NULL);
			break;
		case 23: //粘贴
			::SkinRichEditCommand(hWnd, UI_CHARFRAME_INPUT_EDIT_NAME, "paste", NULL);
			break;
		case 25: //全选
			::SkinRichEditCommand(hWnd, UI_CHARFRAME_INPUT_EDIT_NAME, "selectall", NULL);
			break;
		}
	}
	return 0;
}

BOOL CChatFrameImpl::SendMailToPeerByTreeMenu(HWND hWnd)
{
	void *pSelNode = NULL;
	LPORG_TREE_NODE_DATA pSelData = NULL;
	TCHAR szName[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	CTreeNodeType tnType;
	CInterfaceAnsiString strMail;
	if (::SkinGetSelectTreeNode(hWnd, UI_COMPANY_TREE_NAME_WCHAR, szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
	{
		if (tnType == TREENODE_TYPE_LEAF)
		{
			if (pSelData)
			{ 
				IContacts *pContact = NULL;
				if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
				{
					pContact->GetMailByUserName(pSelData->szUserName, &strMail);
					pContact->Release();
				}
				//
			} //end if (pSelData)
		} //end if (tnType == TREENODE_TYPE_LEAF)
	} //end if (::GetSelectTreeNode(hWnd..	
	if (strMail.GetSize() > 0)
	{
		std::string strOpenString = "mailto:";
		strOpenString += strMail.GetData();
		TCHAR szTmp[MAX_PATH] = {0};
		CStringConversion::StringToWideChar(strOpenString.c_str(), szTmp, MAX_PATH - 1);
		::ShellExecute(NULL, L"open", szTmp, NULL, NULL, SW_SHOW);
		return TRUE;
	} else
	{
		::SkinMessageBox(hWnd, L"对方没有预留邮箱帐号，请与对方确认邮箱帐号后再发送邮件", L"提示", MB_OK);
	}
	return FALSE;
}

//远程协助相关 ==> 接收对方的远程协助请求
BOOL CChatFrameImpl::DoAcceptControl(HWND hOwner)
{
	std::string strXml;
	strXml = "<trs type=\"rtoAcceptStartResponse\" from=\"";
	strXml += m_strUserName;
	strXml += "\" to=\"";
	strXml += m_rmcInfo.m_strPeerName;
	strXml += "\" channelid=\"";
	char szTmp[16] = {0};
	strXml += ::itoa(m_rmcChlId, szTmp, 10);
	strXml += "\"/>";
	RECT rc = {0, 0, 0, 0};
	m_rmcInfo.m_hOwner =  GetHWNDByUserName(m_rmcInfo.m_strPeerName.c_str());
	m_rmcChlId = ::RmcViewController(m_strTransferIp.c_str(), ::atoi(m_strTransferPort.c_str()),
					 m_rmcInfo.m_nChlId, m_rmcInfo.m_hOwner, &rc, m_rmcInfo.m_strKey.c_str(), m_rmcInfo.m_strPeerInternetIp.c_str(),
					 m_rmcInfo.m_wPeerInternetPort, m_rmcInfo.m_strPeerIntranetIp.c_str(), 
					 m_rmcInfo.m_wPeerIntranetPort, this);
	CancelCustomLink(m_rmcInfo.m_hOwner, m_rmcInfo.m_nChlId, CUSTOM_LINK_FLAG_RMC_ACCEPT | CUSTOM_LINK_FLAG_RMC_REFUSE);
	return SUCCEEDED(m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG)strXml.size(), 0));
}

//远程协助相关 ==>拒绝对方的远程协助请求
BOOL CChatFrameImpl::DoRefuseControl(HWND hOwner)
{
	//<trs type="rtoCancelPleaseControlMe" from="admin@gocom" to="wuxiaozhong@GoCom" channelid="4"/>
	std::string strXml = "<trs type=\"rtoCancelPleaseControlMe\" from=\"";
	strXml += m_strUserName;
	strXml += "\" to=\"";
	strXml += m_rmcInfo.m_strPeerName;
	strXml += "\" channelid=\"";
	char szTmp[16] = {0};
	strXml += ::itoa(m_rmcInfo.m_nChlId, szTmp, 10);
	strXml += "\"/>";
	CancelCustomLink(m_rmcInfo.m_hOwner, m_rmcInfo.m_nChlId, CUSTOM_LINK_FLAG_RMC_ACCEPT | CUSTOM_LINK_FLAG_RMC_REFUSE);
	ClearRmcChannel();
	return SUCCEEDED(m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG)strXml.size(), 0));
}

void CChatFrameImpl::SendCancelVideoProtocol(const char *szPeerName, const char *szChlId, const char *szReason)
{
	if (m_pCore)
	{
		std::string strXml = "<trs type=\"VideoCancel\" from=\"";
		strXml += m_strUserName;
		strXml += "\" to=\"";
		strXml += szPeerName;
		strXml += "\" channelid=\"";
		if (szChlId)
			strXml += szChlId;
		else
			strXml += "0";
		strXml += "\" reason=\"";
		if (szReason)
			strXml += szReason;
		strXml += "\"/>"; 
		m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG) strXml.size(), 0);
	}
}

//
void CChatFrameImpl::DoRefuseVideoRequest(HWND hOwner)
{
	if (m_pCore)
	{
		DoChatCommand(m_VideoInfo.m_hOwner, CHAT_COMMAND_TAB2INFO);
		char szTmp[16] = {0}; 
		::itoa(m_VideoInfo.m_nChlId, szTmp, 10);
		SendCancelVideoProtocol(m_VideoInfo.m_strPeerName.c_str(), szTmp, NULL);
		CancelCustomLink(m_VideoInfo.m_hOwner, m_VideoInfo.m_nChlId, CUSTOM_LINK_FLAG_V_ACCEPT | CUSTOM_LINK_FLAG_V_REFUSE);
		m_VideoChlId = 0;
		memset(&m_VideoInfo, 0, sizeof(m_VideoInfo));
//		::VideoStopCapture();
	}
}

BOOL CChatFrameImpl::DoCustomLink(HWND hWnd, DWORD dwFlag)
{

	int nCmd = dwFlag & 0x0000FFFF;
	int nFileId = ((dwFlag >> CUSTOM_LINK_FLAG_OFFSET) & 0x0000FFFF);
	
	PRINTDEBUGLOG(dtInfo, "custom link, FileId:%d Flag:%d", nFileId, nCmd);
	char szLocalFileName[MAX_PATH] = {0};
	char szFileId[32] = {0};
	::itoa(nFileId, szFileId, 10);
	CTransferFileInfo FileInfo;
	std::string strXml;
	char szInternetIp[32] = {0};
	char szIntranetIp[32] = {0};
	WORD wInternetPort = 0;
	WORD wIntranetPort = 0;
	if (nCmd == CUSTOM_LINK_FLAG_RMC_ACCEPT)
	{
		DoAcceptControl(hWnd);
	} else if (nCmd == CUSTOM_LINK_FLAG_RMC_REFUSE)
	{
		DoRefuseControl(hWnd);
	} else if (nCmd == CUSTOM_LINK_FLAG_RMC_CTRL)
	{
		if (m_rmcChlId > 0)
			::RmcRequestByControl(m_rmcChlId, 0xFFFFFFFF);
	} else if (nCmd == CUSTOM_LINK_FLAG_V_ACCEPT)
	{  
		strXml = "<trs type=\"VideoAccept\" from=\"";
		strXml += m_strUserName;
		strXml += "\" to=\"";
		strXml += m_VideoInfo.m_strPeerName;
		strXml += "\" channelid=\"";
		char szTmp[16] = {0};
		strXml += ::itoa(m_VideoChlId, szTmp, 10);
		strXml += "\"/>";
		HWND h =  GetHWNDByUserName(m_VideoInfo.m_strPeerName.c_str());
		if (h != NULL)
		{
//			if (!::VideoCheckDevice())
//				::SkinRichEditInsertTip(h, UI_CHATFRAME_DISPLAY_EDIT_NAME, NULL, 0, L"没有找到视频摄像头");
			m_VideoInfo.m_hOwner = h; 
			char szKey[MAX_PATH] = {0};
			DWORD nKeySize = MAX_PATH - 1;
			char szLocalIp[20] = {0};
			char szInternetIp[20] = {0};
			WORD wLocalPort = 0, wInternetPort = 0;
			//RECT rcPreview = {300, 460, 460, 580}, rcPlayer = {200, 100, 520, 440 };
			RECT rcPreview = {0}, rcPlayer = {0};
			::SkinGetControlRect(h, L"SelfVideo", &rcPreview);
			::SkinGetControlRect(h, L"PeerVideo", &rcPlayer);
//			::VideoView(h, &rcPreview, h, &rcPlayer, m_strTransferIp.c_str(), ::atoi(m_strTransferPort.c_str()),
//				m_VideoChlId, m_VideoInfo.m_strKey.c_str(), m_VideoInfo.m_strPeerIntranetIp.c_str(),
//				m_VideoInfo.m_wPeerIntranetPort, m_VideoInfo.m_strPeerInternetIp.c_str(),
//				m_VideoInfo.m_wPeerInternetPort, this); 
			CancelCustomLink(m_VideoInfo.m_hOwner, m_VideoInfo.m_nChlId, CUSTOM_LINK_FLAG_V_ACCEPT | CUSTOM_LINK_FLAG_V_REFUSE);
		}		
	} else if (nCmd == CUSTOM_LINK_FLAG_V_REFUSE)
	{
		if (m_VideoChlId > 0)
		{ 
			//<trs type="VideoCancel" from="admin@gocom" to="wuxiaozhong@GoCom" channelid="4"/>
			DoRefuseVideoRequest(hWnd);
		}
	} else if (GetTransferFileInfoById(nFileId, FileInfo))
	{
		if (FileInfo.m_nPeerFileId > 0)
			::itoa(FileInfo.m_nPeerFileId, szFileId, 10);
		else
			::itoa(FileInfo.m_nLocalFileId, szFileId, 10);
		switch(nCmd)
		{
		case CUSTOM_LINK_FLAG_SAVEAS:
			{
				if (!FileInfo.m_strProFlag.IsEmpty())
				{
					char szTmp[MAX_PATH] = {0};
					CStringConversion::WideCharToString(FileInfo.m_strProFlag.GetData(), szTmp, MAX_PATH - 1);
				    FileSaveAsEvent(hWnd, szTmp);
				}
				break;
			} 
		case CUSTOM_LINK_FLAG_RECV:
			{
				if (!FileInfo.m_strProFlag.IsEmpty())
				{
					char szTmp[MAX_PATH] = {0};
					CStringConversion::WideCharToString(FileInfo.m_strProFlag.GetData(), szTmp, MAX_PATH - 1);
					FileRecvEvent(hWnd, szTmp);
				}
				break;
			}
		case CUSTOM_LINK_FLAG_CANCEL:
			{
				 if (!FileInfo.m_strProFlag.IsEmpty())
				{
					char szTmp[MAX_PATH] = {0};
					CStringConversion::WideCharToString(FileInfo.m_strProFlag.GetData(), szTmp, MAX_PATH - 1);
					FileCancelEvent(hWnd, szTmp);
				}
				break;
			}
		case CUSTOM_LINK_FLAG_REFUSE:
			{
				if (!FileInfo.m_strProFlag.IsEmpty())
				{
					char szTmp[MAX_PATH] = {0};
					CStringConversion::WideCharToString(FileInfo.m_strProFlag.GetData(), szTmp, MAX_PATH - 1);
					FileCancelEvent(hWnd, szTmp);
				}
				break;
			}
		case CUSTOM_LINK_FLAG_OFFLINE:
			{
				if (!FileInfo.m_strProFlag.IsEmpty())
				{
					char szTmp[MAX_PATH] = {0};
					CStringConversion::WideCharToString(FileInfo.m_strProFlag.GetData(), szTmp, MAX_PATH - 1);
					FileSendOfflineEvent(hWnd, szTmp);
				}
				break;
			}
		}
	}
	if ((!strXml.empty()) && m_pCore)
		return m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (int) strXml.size(), 0);
	return FALSE;
}

LRESULT CChatFrameImpl::DoWindowPosChanged(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if ((m_rmcChlId > 0) && (m_rmcInfo.m_hOwner == hWnd))
	{
		//RECT rc = {0};
		//if (::SmartSkinGetControlRect(hWnd, L"rmcShowWindow", &rc))
		//	::AdjustPlayRect(m_rmcInfo.m_nChlId, &rc);
	}
	return -1;
}

//
LRESULT CChatFrameImpl::DoAfterInitMenu(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{

	return -1;
}

LRESULT CChatFrameImpl::DoLinkEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "peermail") == 0)
	{
		TCHAR szTmp[MAX_PATH] = {0};
		int nSize = MAX_PATH - 1;
		if (::SkinGetControlTextByName(hWnd, L"peermail", szTmp, &nSize))
		{
			BOOL bHasDot = FALSE, bHasAt = FALSE;
			TCHAR *p = szTmp;
			while ((*p) != _T('\0'))
			{
				if (*p == _T('@'))
					bHasAt = TRUE;
				else if (*p == _T('.'))
				{
					if (bHasAt == TRUE)
						bHasDot = TRUE;
				}
				if (bHasAt && bHasDot)
					break;
				p ++; 
			}
			if (bHasAt && bHasDot)
			{
				CStdString_ strOpen = L"mailto:";
				strOpen += szTmp;
				::ShellExecute(NULL, L"open", strOpen.GetData(), NULL, NULL, SW_SHOW);
			} else
			{
				::SkinMessageBox(hWnd, L"非法的邮箱地址", L"提示", MB_OK);
			}
		}
	} else if (::stricmp(szName, "peerphone") == 0)
	{
		SendSmsByHWND(hWnd);
	} else if (::strnicmp(szName, "fr_", 3) == 0)
	{
		//file recv
		char szFlag[128] = {0};
		strcpy(szFlag, szName + 3);
		FileRecvEvent(hWnd, szFlag);
	} else if (::strnicmp(szName, "fo_", 3) == 0)
	{
		//send offline 
		char szFlag[128] = {0};
		strcpy(szFlag, szName + 3);
		FileSendOfflineEvent(hWnd, szFlag);
	} else if (::strnicmp(szName, "fs_", 3) == 0)
	{
		//file save as 
		char szFlag[128] = {0};
		strcpy(szFlag, szName + 3);
		FileSaveAsEvent(hWnd, szFlag);
	} else if (::strnicmp(szName, "fc_", 3) == 0)
	{
		//file cancel 
		char szFlag[128] = {0};
		strcpy(szFlag, szName + 3);
		FileCancelEvent(hWnd, szFlag);
	}
	return -1;
}

void CChatFrameImpl::SendSmsByHWND(HWND hWnd)
{
	//
	TCHAR szTmp[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	if (::SkinGetControlTextByName(hWnd, L"peerphone", szTmp, &nSize))
	{
		BOOL bValid = FALSE;
		if (::lstrlen(szTmp) == 11)
		{
			bValid = TRUE;
		}
		if (bValid)
		{
			ISMSFrame *pFrame = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(ISMSFrame), (void **)&pFrame)))
			{
				CInterfaceAnsiString strUserName;
				if (SUCCEEDED(GetUserNameByHWND(hWnd, &strUserName)))
				{
				    CInterfaceUserList ulList;
					LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
					memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
					strcpy(pData->szUserName, strUserName.GetData());
					ulList.AddUserInfo(pData, FALSE, TRUE);
					pFrame->ShowSMSFrame(NULL, &ulList);
				}
				pFrame->Release();
			}
		} else
			::SkinMessageBox(hWnd, L"非法的手机号码", L"提示", MB_OK);
	}
}

//
LRESULT CChatFrameImpl::DoInitMenuPopup(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "phrasemenu") == 0)
	{
		CUserChatFrame *pChat = GetChatFrameByHWND(hWnd);
		if (pChat && (!pChat->m_bInitAckMenu))
		{
			IConfigure *pCfg = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
			{
				CMessageList mlList;
				if (SUCCEEDED(pCfg->GetReplyMessage(1, &mlList)))
				{
					int nId = 0;
					CInterfaceAnsiString strMsg;
					TCHAR szwTemp[256] = {0};
					for (int i = 0; i < mlList.GetCount(); i ++)
					{
						mlList.GetRawMsg(i, &nId, &strMsg);
						memset(szwTemp, 0, sizeof(TCHAR) * 256);
						CStringConversion::StringToWideChar(strMsg.GetData(), szwTemp, 255);
						::SkinMenuAppendItem(hWnd, L"phrasemenu", 0, szwTemp, nId);
					}
				}
				pCfg->Release(); 
			}
			pChat->m_bInitAckMenu = TRUE;
		}
	}
	return -1;
}

BOOL CChatFrameImpl::RECallBack(HWND hWnd, DWORD dwEvent, char *szFileName, DWORD *dwFileNameSize,
			  char *szFileFlag, DWORD *dwFileFlagSize, DWORD dwFlag)
{
	switch(dwEvent)
	{
		case RICHEDIT_EVENT_SENDFILE:
			 SendFileToPeer(hWnd, szFileName);
			 break;
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
					} else if (SUCCEEDED(pFrame->GetCustomEmotion(szFileFlag, &strFileName)))
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
				
				return TRUE;
			}
		case RICHEDIT_EVENT_GETTIPPIC:
			break;
		case RICHEDIT_EVENT_CUSTOMLINKCLICK:
			{
				//
				if (dwFlag == 0) //取消自动回复
				{
					CUserChatFrame * pFrame = GetChatFrameByHWND(hWnd);
					if (pFrame)
					{
						pFrame->m_bShowAutoReply = FALSE; 
					}   
				} else if (dwFlag == 0xFFFFFFFF) //回执
				{
					//回执信息
					std::map<HWND, CUserChatFrame *>::iterator it = m_ChatFrameList.find(hWnd);
					if (it != m_ChatFrameList.end())
					{	
						SendReceiptToPeer(it->second->GetUserName(), szFileName, "ack");
					}
				} else
				{
					/*    0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31     |
					      |---ID FLAG-----|---------------------FILE  ID---------------------------------------     |*/
					DoCustomLink(hWnd, dwFlag);
				}
			}
			break;
	}
	return FALSE;
}


#pragma warning(default:4996)
