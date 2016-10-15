#include <Commonlib/DebugLog.h>
#include <Commonlib/systemutils.h>
#include <Commonlib/stringutils.h>
#include <FileTransfer/filetransfer.h>
#include <rmc/remotemachinecontrol.h>
#include <time.h>
//#include <Media/VideoSDL.h>
#include <SmartSkin/smartskin.h>
#include <Core/common.h>
#include "../P2Svr/P2Svr.h"
#include "ChatFrameImpl.h"
#include "../IMCommonLib/InterfaceAnsiString.h"

#pragma warning(disable:4996)

#define DEFAULT_P2P_START_PORT  4800
#define DEFAULT_P2P_TRY_TIMES   200

#define FT_ERROR_PROGRESS         0  //表示传送进度
#define FT_ERROR_CONNECT          1  //连接
#define FT_ERROR_TRANSFER         2  //传输结果
#define RMC_WORK_STATUS           3  //远程控制工作状态
#define RMC_CHANGE_SIZE           4  //远程端SIZE改变

//连接状态
#define CONNECT_STATUS_SUCC       0 //连接成功
#define CONNECT_STATUS_CONNECTING 1 //正在连接
#define CONNECT_STATUS_FAIL       2 //连接失败

//连接方式定义
#define CONNECT_TYPE_INTRANET     0  //局域网方式连接
#define CONNECT_TYPE_INTERNET     1  //广域网方式连接
#define CONNECT_TYPE_RESERVECON   2  //反向连接
#define CONNECT_TYPE_SERVERTRANS  3  //服务器中转

//传输结果类型定义 参数 lParam  参数wParam 为文件名称 直接转换为(char *)
#define TRANSFER_TYPE_SUCC        0  //传输成功
#define TRANSFER_TYPE_FAIL        1  //传输失败
#define TRANSFER_TYPE_CANCEL      2  //被对方中止传输
#define TRANSFER_OPENFILE_FAIL    3  //打开文件失败
#define TRANSFER_WRITE_FAIL       4  //写入文件出现错误，此错误有可能是磁盘已满
#define TRANSFER_FILELEN_ERROR    5  //文件长度出错
#define TRANSFER_SETFILEPOS_FAIL  6  //断点续传文件长度出错
#define TRANSFER_COMPLETE_TIMEOUT 7  //传输完成超时

//工作方式定义
#define RMC_WORK_STATUS_INVALID      1  //非法的用户
#define RMC_WORK_STATUS_WORKING      2  //进入工作状态
#define RMC_WORK_STATUS_CANCEL       3  //被对方取消
#define RMC_WORK_STATUS_RECV_TIMEOUT 4  //接收数据超时
#define RMC_WORK_STATUS_SEND_TIMEOUT 5  //发送数据超时

//远程监控编码方式
#define RMC_IMAGE_ENCODE_TYPE_NONE   0  //没有经过编码的裸数据
#define RMC_IMAGE_ENCODE_TYPE_JPG    1  //JPG编码图片
#define RMC_IMAGE_ENCODE_TYPE_AC     2  //AC编码
#define RMC_IMAGE_ENCODE_TYPE_ZLIB   3  //Zlib压缩
#define RMC_IMAGE_ENCODE_TYPE_ACZLIB 4  //先AC 再 zlib压缩
#define RMC_IMAGE_ENCODE_TYPE_256    5  //先转成256色再zlib

void CALLBACK FileTransNotifyCallBack(const DWORD dwErrorNo, const DWORD dwFileId, const LPARAM lParam, 
												 const WPARAM wParam, void *pOverlapped)
{
	if (pOverlapped)
	{
		CChatFrameImpl *pThis = (CChatFrameImpl *)pOverlapped;
		pThis->TransFileProgress(dwErrorNo, dwFileId, lParam, wParam);
	} else
	{
		PRINTDEBUGLOG(dtInfo, "FileTransNotify ID:%d  FileId:%d", dwErrorNo, dwFileId);
	}
}

void CALLBACK RmcCallBack(const DWORD dwErrorNo, const DWORD dwChlId, const WPARAM wParam, 
												 const LPARAM lParam, void *pOverlapped)
{
	if (pOverlapped)
	{
		CChatFrameImpl *pThis = (CChatFrameImpl *)pOverlapped;
		pThis->RcmCallback(dwErrorNo, dwChlId, lParam, wParam);
	} else
	{
		PRINTDEBUGLOG(dtInfo, "RmcCallBack ID:%d  FileId:%d", dwErrorNo, dwChlId);
	}
}


void CALLBACK VideoCallBack(const DWORD dwErrorNo, const DWORD dwChlId, const WPARAM wParam,
	                                               const LPARAM lParam, void *pOverlapped)
{
	if ((dwErrorNo == 1) && (lParam == 0))
	{
		CChatFrameImpl *pThis = (CChatFrameImpl *) pOverlapped;		 
		pThis->VideoNotify(dwErrorNo, dwChlId, lParam, wParam);
	}
}


void CChatFrameImpl::VideoNotify(const DWORD dwErrorNo, const DWORD dwChlId, const LPARAM lParam, const WPARAM wParam)
{
	IUIManager *pUI = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
	{
		 
		HRESULT hr = 0;
		pUI->SendMessageToWindow(UI_MAIN_WINDOW_NAME, WM_VIDEO_CONNECTED, 
			      0, 0, &hr);
		 
		pUI->Release();
	} //end if (m_pCore && ...	 
}


void CChatFrameImpl::RcmCallback(const DWORD dwErrorNo, const DWORD dwRmcId, const WPARAM wParam, const LPARAM lParam)
{
	switch(dwErrorNo)
	{
	case FT_ERROR_CONNECT: //连接进度
		{
			switch(wParam)
			{
			case CONNECT_TYPE_INTRANET: //局域网方式连接
			case CONNECT_TYPE_INTERNET:      //广域网方式连接
			case CONNECT_TYPE_RESERVECON:    //反向连接
			case CONNECT_TYPE_SERVERTRANS:   //服务器中转
				break;
			}
			switch(lParam)
			{
			case CONNECT_STATUS_SUCC: //连接成功
				 {				  
					 if (m_rmcChlId > 0)
					 {
						 AnsycShowTip(m_rmcInfo.m_hOwner, L"远程协助连接建立成功");
						 m_rmcInfo.m_bConnected = TRUE;
						 if (m_rmcInfo.m_bRequest)
						 {
							 ::PostMessage(GetMainFrameHWND(), WM_CHAT_COMMAND, (WPARAM)m_rmcInfo.m_hOwner,
								 (LPARAM) CHAT_COMMAND_TAB2RMC_SWCTRL);
						 } else
						 { 							 
							 ::PostMessage(GetMainFrameHWND(), WM_CHAT_COMMAND, (WPARAM)m_rmcInfo.m_hOwner,
								 (LPARAM) CHAT_COMMAND_TAB2RMC_SHOW);					 
						 }
					 } else
					 {
						 PRINTDEBUGLOG(dtInfo, "rmc connect succ, but channel id is zero");
					 }
				 }
				 break;
			case CONNECT_STATUS_CONNECTING: //正在连接
				 break;
			case CONNECT_STATUS_FAIL: //连接失败
				 if (wParam == CONNECT_TYPE_SERVERTRANS)
				 {
					 //彻底失败
					 if (m_rmcChlId > 0)
					 {
						 AnsycShowTip(m_rmcInfo.m_hOwner, L"远程协助连接建立失败，请重试");
						 ::PostMessage(GetMainFrameHWND(), WM_CHAT_COMMAND, (WPARAM)m_rmcInfo.m_hOwner, (LPARAM)CHAT_COMMAND_CLEAR_RMC_CHL);						 
					 }
				 }
				 break;
			}
			break;
		}
	case RMC_WORK_STATUS: //远程控件工作状态
		{
			switch (wParam)
			{
				case RMC_WORK_STATUS_INVALID     : //非法的用户
					 break;
				case RMC_WORK_STATUS_WORKING     : //进入工作状态
					 break;
				case RMC_WORK_STATUS_CANCEL      : //被对方取消 
					 if (m_rmcChlId > 0)
					 {
						 AnsycShowTip(m_rmcInfo.m_hOwner, L"对方取消了与您的远程协助");
						 ::PostMessage(GetMainFrameHWND(), WM_CHAT_COMMAND, (WPARAM)m_rmcInfo.m_hOwner, (LPARAM)CHAT_COMMAND_CLEAR_RMC_CHL);
					 }
					 break;
				case RMC_WORK_STATUS_RECV_TIMEOUT: //接收数据超时
					 if (m_rmcChlId > 0)
					 {
						 AnsycShowTip(m_rmcInfo.m_hOwner, L"远程协助过程中出现接收数据超时");
						 ::PostMessage(GetMainFrameHWND(), WM_CHAT_COMMAND, (WPARAM)m_rmcInfo.m_hOwner, (LPARAM)CHAT_COMMAND_CLEAR_RMC_CHL);
					 }
					 break;
				case RMC_WORK_STATUS_SEND_TIMEOUT: //发送数据超时
					 if (m_rmcChlId > 0)
					 {
						 AnsycShowTip(m_rmcInfo.m_hOwner, L"远程协助过程中出现发送数据超时");
						 ::PostMessage(GetMainFrameHWND(), WM_CHAT_COMMAND, (WPARAM)m_rmcInfo.m_hOwner, (LPARAM)CHAT_COMMAND_CLEAR_RMC_CHL);
					 }
					 break;
			} 
			break;
		}

	case RMC_CHANGE_SIZE:   //远程端SIZE改变
		{
			//lParam :宽 参数wParam：高
			if (m_rmcChlId > 0)
			{
				m_rmcInfo.m_nWidth = (int) lParam;
				m_rmcInfo.m_nHeight = (int) wParam;
				if (m_rmcInfo.m_hFullScreen != NULL)
				{
					RECT rc = {0, 0, m_rmcInfo.m_nWidth, m_rmcInfo.m_nHeight};

					::RmcAdjustPlayRect(m_rmcChlId,  &rc);
				}
			}
			break;
		}    
       
	} //end switch(nErrorNo)
}

void CChatFrameImpl::TransFileProgress(const DWORD dwErrorNo, const DWORD dwFileId, const LPARAM lParam, 
												 const WPARAM wParam)
{
	
	if (dwErrorNo == FT_ERROR_PROGRESS)
	{

		::PostMessage(GetMainFrameHWND(), WM_CHAT_UPDL_PROGR, dwFileId, lParam);
		 //m_hMainFrame
	} else
	{
		CTransferFileInfo Info;
		if (m_TransFileList.GetFileInfoById(dwFileId, Info))
		{
			switch(dwErrorNo)
			{
				case FT_ERROR_CONNECT:
					{
						break;
					}
				case FT_ERROR_TRANSFER:
					{
						if (lParam == TRANSFER_TYPE_SUCC)
						{ 
							if (Info.m_bSender)
							{						 
							    CStdString_ strTip = L"文件 ";
							    TCHAR szTmp[MAX_PATH] = {0};
								CStringConversion::StringToWideChar(Info.m_strDspName.c_str(), szTmp, MAX_PATH - 1);
							    strTip += szTmp;
								strTip += L"  发送成功"; 
							    AnsycShowTip(Info.hOwner, strTip.GetData());
							} else
							{
								std::string strTip = "文件 %%FILE%% 接收成功";  
								AnsyShowFileLink(Info.hOwner, strTip.c_str(), Info.m_strLocalFileName.c_str());
							}
							char szTime[64] = {0};
							CSystemUtils::DateTimeToStr((DWORD)::time(NULL), szTime);
							std::string strMsg, strBody;
							strBody = "文件";
							strBody += Info.m_strLocalFileName.c_str();
							if (Info.m_bSender)
								strBody += " 发送成功";
							else
								strBody += " 接收成功";
							strMsg = "<tip datetime=\"";
							strMsg += szTime;
							strMsg += "\">";
							strMsg += strBody;
							strMsg += "</tip>";
							SaveP2PMsg("p2p", Info.m_strPeerName.c_str(),  m_strUserName.c_str(), szTime, 
								strMsg.c_str(), strBody.c_str());
							RemoveTransFile(dwFileId, NULL, TRUE);
						} else if (lParam == TRANSFER_TYPE_FAIL)
						{
							CStdString_ strTip = L"文件 ";
							TCHAR szTmp[MAX_PATH] = {0};
							CStringConversion::StringToWideChar(Info.m_strDspName.c_str(), szTmp, MAX_PATH - 1);
							strTip += szTmp;
							if (Info.m_bSender)
							{
								strTip += L"  发送失败"; 
							} else
								strTip += L"  接收失败";
							AnsycShowTip(Info.hOwner, strTip.GetData());
							RemoveTransFile(dwFileId, NULL, TRUE);
						} else if (lParam == TRANSFER_TYPE_CANCEL)
						{
							RemoveTransFile(dwFileId, NULL, TRUE);
						}
						break;
					} //end case FT_ERROR_TRANSFER
			} // end switch(dwErrorNo)
		} // end if (m_TransFileList..
	} //end if 
}

//do protocol
//
BOOL CChatFrameImpl::DoSystemProtocol(const char *szType, TiXmlElement *pNode)
{
	BOOL bDid = FALSE;
	if (::stricmp(szType, "login") == 0)
	{
		const char *szResult = pNode->Attribute("result");
		if (szResult && (::stricmp(szResult, "ok") == 0))
		{
			//
			CInterfaceAnsiString strHost;
			CInterfaceAnsiString strPort;
			if (m_pCore && SUCCEEDED(m_pCore->GetSvrParams("p2pserver", (IAnsiString *)&strHost, TRUE))
				&& SUCCEEDED(m_pCore->GetSvrParams("p2pport", (IAnsiString *)&strPort, TRUE)))
			{
				CInterfaceAnsiString strCachePath;
				IConfigure *pCfg = NULL;
				if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
				{
					pCfg->GetPath(PATH_LOCAL_CACHE_PATH, &strCachePath); 
					pCfg->Release();
				}
				m_strTransferIp.clear();
				m_strTransferPort = strPort.GetData();
				CSystemUtils::GetIpByHostName(strHost.GetData(), m_strTransferIp);
				//创造文件传输接口
				if (::CreateTransPort(m_strTransferIp.c_str(), ::atoi(m_strTransferPort.c_str()), 
					   FileTransNotifyCallBack, strCachePath.GetData(), DEFAULT_P2P_START_PORT, DEFAULT_P2P_TRY_TIMES) != 0)
				{
					PRINTDEBUGLOG(dtInfo, "CreateTransPort Failed");
				} 
				//获取服务器参数
				CInterfaceAnsiString strTmp;
				//创建远程协助接口
				if (::RmcCreateViewPort(m_strTransferIp.c_str(), ::atoi(m_strTransferPort.c_str()),
					strCachePath.GetData(), RmcCallBack, DEFAULT_P2P_START_PORT + 1, DEFAULT_P2P_TRY_TIMES) != 0)
				{
					PRINTDEBUGLOG(dtInfo, "Create View Port Failed");
				}

				//创建视频传输接口
//				if (!::VideoInitPort(m_strTransferIp.c_str(), ::atoi(m_strTransferPort.c_str()),
//					strCachePath.GetData(), VideoCallBack, DEFAULT_P2P_START_PORT + 2, DEFAULT_P2P_TRY_TIMES))
//				{
//					PRINTDEBUGLOG(dtInfo, "Create Video Port Failed");
//				}

				//获取文件传输大小限制参数 
				m_pCore->GetSvrParams("online.maxfilesize", &strTmp, TRUE);
				m_pCore->GetSvrParams("offline.maxfilesize", &strTmp, TRUE);
			}
		}
	}
	return bDid;
}

BOOL CChatFrameImpl::SaveP2PMsg(const char *szType, const char *szFromName, const char *szToName,
		                 const char *szTime, const char *szMsg, const char *szBody)
{
	IMsgMgr *pMsgMgr = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMsgMgr), (void **)&pMsgMgr)))
	{
		BOOL bSucc = SUCCEEDED(pMsgMgr->SaveMsg(szType, szFromName, szToName, szTime, szMsg, szBody));
		
		pMsgMgr->Release();
        return bSucc;
	}
	return FALSE;
}

void CChatFrameImpl::SaveMessage(const char *szType, TiXmlElement *pNode)
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
	const char *szReceipt = pNode->Attribute("Receipt");
	if (szReceipt && (::stricmp(szReceipt, "ack") == 0))
	{
		std::string strTip = "<tip datetime=\">";
		if (pNode->Attribute("datetime"))
			strTip += pNode->Attribute("datetime");
		strTip += "\">对方阅读了你发送的信息";
		strTip += strText;
		strTip += "</tip>";
		SaveP2PMsg("p2p",  pNode->Attribute("from"), pNode->Attribute("to"), pNode->Attribute("datetime"),
			strTip.c_str(), strText.c_str());
	} else
		SaveP2PMsg(szType, 	pNode->Attribute("from"), pNode->Attribute("to"), pNode->Attribute("datetime"),
			strXml.c_str(), strText.c_str());
}

void CChatFrameImpl::PlayTipSound(const char *szType, const char *szUserName, BOOL bLoop)
{
	IConfigure *pCfg = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		pCfg->PlayMsgSound(szType, szUserName, bLoop);
		pCfg->Release();
	}
}

BOOL CChatFrameImpl::DoMessageProtocol(const char *szType, TiXmlElement *pNode)
{
	BOOL bDid = FALSE;
	if (::stricmp(szType, "p2p") == 0)
	{
		const char *szUserName = pNode->Attribute("from");
		if (szUserName)
		{
			
		    TiXmlElement *pChild = pNode->FirstChildElement();
			if (pChild && ::stricmp(pChild->Value(), "shake") == 0)
			{
				//<msg type="p2p" from="%s" to="%s"><shake times="%d" /></msg>
				HWND h = GetHWNDByUserName(szUserName);
				if (h == NULL)
				{	
					//打开窗体
					IUIManager *pUI = NULL;
					if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
					{
						HRESULT hr;
						pUI->SendMessageToWindow(UI_MAIN_WINDOW_NAME, WM_OPENCHATFRAME, 
								  0, LPARAM(szUserName), &hr);
						pUI->Release();
					}
					h = GetHWNDByUserName(szUserName);
				} 
				if (h)
				{
					CUserChatFrame *pFrame = GetChatFrameByHWND(h); 
					if (pFrame)
						pFrame->StartShake();
					//CSystemUtils::
					PlayTipSound("shake", szUserName, FALSE);
					AnsycShowTip(h, L"您接收到一个闪屏振动");
				} 
			} else if (pChild && ::stricmp(pChild->Value(), "verison") == 0)
			{
				//<msg type="p2p" from="%s" to="%s"><version mode="request or ack" /></msg>
				const char *szMode = pChild->Attribute("mode");
				if (szMode && ::stricmp(szMode, "request") == 0) //ack
				{
					std::string strXml = "<msg type=\"p2p\" from=\"";
					strXml += m_strUserName;
					strXml += "\" to=\"";
					strXml += szUserName;
					strXml += "\"><version mode=\"ack\"/></msg>";
					m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG)strXml.size(), 0);
				} else
				{
					std::map<HWND, CUserChatFrame *>::iterator it;
					for (it = m_ChatFrameList.begin(); it != m_ChatFrameList.end(); it ++)
					{
						if (it->second->IsUserNameFrame(szUserName))
						   it->second->SetIsOldVersion(FALSE);
					}
				}
				//回应
			} else if (pChild && (::stricmp(pChild->Value(), "autoreply") == 0))
			{ 
				//<msg type="p2p" from="%s" to="%s"><autoreply text="reply message" /></msg> 
				HWND h = GetHWNDByUserName(szUserName);
				if (h && (pChild->Attribute("text") != NULL))
				{   
					CUserChatFrame *pFrame = GetChatFrameByHWND(h);
					if (pFrame && pFrame->m_bShowAutoReply)
					{
						TCHAR szwTmp[MAX_PATH] = {0};
						CStringConversion::StringToWideChar(pChild->Attribute("text"), szwTmp, MAX_PATH - 1);
						CStdString_ strTip = L"收到对方的自动回复“";
						strTip += szwTmp;
						strTip += L"”<不再提示,0>";
						AnsycShowTip(h, strTip.GetData());
					} //end if (pFrame...
				} //end if (h
			} else
			{
				SaveMessage(szType, pNode);
				HWND h = GetHWNDByUserName(szUserName); 
				BOOL bRequest = FALSE;
				if (pNode->Attribute("Receipt"))
				{
					if (::stricmp(pNode->Attribute("Receipt"), "request") == 0)
						bRequest = TRUE;
				}
				if (h || bRequest)
				{ 
					IUIManager *pUI = NULL;
					if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
					{
						TiXmlString strXml;
						pNode->SaveToString(strXml, 0);
						HRESULT hr = 0;
						pUI->SendMessageToWindow(UI_MAIN_WINDOW_NAME, WM_SHOWCHATMESSAGE, 
							      WPARAM(h), LPARAM(strXml.c_str()), &hr);
						 
						pUI->Release();
					} //end if (m_pCore && ...
					bDid = TRUE;
				}  
			} //end if 
		} //end if (szUserName)
	} else if (::stricmp(szType, "offlinefile") == 0)
	{
		//<msg type="offlinefile" from="admin@gocom" to="wuxiaozhong@gocom" name="apss.dll.mui" filesize="3072"
		// url="A90C62138DFB74BEA86244E2432D133EF.mui" fileserver="http://im.smartdot.com.cn:9910"/>
		const char *szFromName = pNode->Attribute("from");
		if (szFromName)
		{
			PlayTipSound("friend", szFromName, FALSE);
			HWND h = GetHWNDByUserName(szFromName);
			if (h == NULL)
			{ 
				IUIManager *pUI = NULL;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
				{
					HRESULT hr;
					pUI->SendMessageToWindow(UI_MAIN_WINDOW_NAME, WM_OPENCHATFRAME, 
						      0, LPARAM(szFromName), &hr);
					h = GetHWNDByUserName(szFromName);
					pUI->Release();
				} //end if (m_pCore && ...
			} else 
			{
				if (m_pCore)
				{
					//m_pCore->StartTrayIcon("P2P", "收到离线文件", NULL);
				}
			}//end if (h == NULL

			if (h != NULL)
			{
				//add progress
				if (!m_TransFileList.CheckTransFileIsExists(pNode->Attribute("from"), pNode->Attribute("name")))
				{
					PRINTDEBUGLOG(dtInfo, "recv offline file, PeerName:%s  name:%s", pNode->Attribute("from"), pNode->Attribute("name"));
					CTransferFileInfo Info;
					Info.hOwner = h;
					Info.m_strDspName = pNode->Attribute("name");
					Info.m_dwFileSize = ::atoi(pNode->Attribute("filesize"));
					Info.m_bSender = FALSE; 
					::SendMessage(GetMainFrameHWND(), WM_CHAT_APPEND_PRO, 0, (LPARAM)&Info);
					int nFileId = m_TransFileList.AddFileInfo(pNode->Attribute("from"), pNode->Attribute("name"), NULL, Info.m_strProFlag.GetData(), 
						pNode->Attribute("filetag"), "", "0", "", "0", "0", 0,
						pNode->Attribute("filesize"), h);
					if (pNode->Attribute("fileserver"))
						m_TransFileList.SetOfflineSvr(nFileId, pNode->Attribute("fileserver"));
					if (pNode->Attribute("url"))
						m_TransFileList.SetRemoteName(nFileId, pNode->Attribute("url"));
					/*CStdString_ strTip = _T("收到\"");
					TCHAR szTmp[MAX_PATH] = {0};
					CStringConversion::UTF8ToWideChar(GetRealNameByHWND(h), szTmp, MAX_PATH - 1);	
					strTip += szTmp;
					strTip += _T("\" 发送的离线文件 \"");
					memset(szTmp, 0, MAX_PATH * sizeof(TCHAR));
					CStringConversion::StringToWideChar(pNode->Attribute("name"), szTmp, MAX_PATH - 1);
					strTip += szTmp; 
					
					wsprintf(szTmp, L"\"\n \t\t <接收,%d>,<另存为,%d>,<拒绝,%d>", 
						            ((nFileId << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_RECV,
						            ((nFileId << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_SAVEAS, 
	 								((nFileId << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_REFUSE);
					strTip += szTmp;
					AnsycShowTip(h, strTip.GetData());*/
				} //end if (
			} else
			{
				PRINTDEBUGLOG(dtInfo, "Open Chat Frame Failed in recv file");
			}
		} //end if (szFromName)
	}		//end if (::stricmp(szType...
	return bDid;
}

//
BOOL CChatFrameImpl::DoTransferProtocol(const char *szType, TiXmlElement *pNode)
{
	if (::stricmp(szType, "FileRequest") == 0)
	{
		// <trs type="FileRequest" from="wuxiaozhong@gocom" fromname="邬晓忠" 
		//   filename="Default.rdp" filetag="30f79afad4318774e447dc2db96936e0" filesize="2000"
		//   to="admin@gocom" locip="192.168.1.100" locport="4800" remotip="123.116.125.172"
		//   remotport="4800" senderfileid="4"/>
		//
		const char *szFromName = pNode->Attribute("from");
		if (szFromName)
		{
			PlayTipSound("friend", "szFromName", FALSE);
			HWND h = GetHWNDByUserName(szFromName);
			if (h == NULL)
			{
				IUIManager *pUI = NULL;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
				{
					HRESULT hr;
					pUI->SendMessageToWindow(UI_MAIN_WINDOW_NAME, WM_OPENCHATFRAME, 
						      0, LPARAM(szFromName), &hr);
					h = GetHWNDByUserName(szFromName);
					pUI->Release();
				} //end if (m_pCore && ...
			} else 
			{
				/*if (m_pCore)
				{
					m_pCore->StartTrayIcon("P2P", "收到新的消息", NULL);
				}*/
			}//end if (h == NULL

			if (h != NULL)
			{
				//add progress
				CTransferFileInfo Info;
				Info.hOwner = h;
				Info.m_strDspName = pNode->Attribute("filename");
				Info.m_bSender = FALSE;
				Info.m_dwFileSize = ::atoi(pNode->Attribute("filesize"));
				::SendMessage(GetMainFrameHWND(), WM_CHAT_APPEND_PRO, 0, (LPARAM)&Info); 
				int nFileId = m_TransFileList.AddFileInfo(pNode->Attribute("from"), pNode->Attribute("filename"), NULL, Info.m_strProFlag.GetData(), 
					pNode->Attribute("filetag"), pNode->Attribute("locip"), pNode->Attribute("locport"),
					pNode->Attribute("remotip"), pNode->Attribute("remotport"), pNode->Attribute("senderfileid"), 0,
					pNode->Attribute("filesize"), h);

				//
				int nPeerFileId = ::atoi(pNode->Attribute("senderfileid"));
				/*CStdString_ strTip = _T("收到\"");
				TCHAR szTmp[MAX_PATH] = {0};
				CStringConversion::UTF8ToWideChar(GetRealNameByHWND(h), szTmp, MAX_PATH - 1);	
				strTip += szTmp;
				strTip += _T("\" 发送的文件 \"");
				memset(szTmp, 0, MAX_PATH * sizeof(TCHAR));
				CStringConversion::StringToWideChar(pNode->Attribute("filename"), szTmp, MAX_PATH - 1); 
				strTip += szTmp;  

				wsprintf(szTmp, L"\"\n \t\t <接收,%d>,<另存为,%d>,<拒绝,%d>", 
					            ((nFileId << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_RECV,
					            ((nFileId << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_SAVEAS, 
 								((nFileId << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_REFUSE);
				strTip += szTmp;
				AnsycShowTip(h, strTip.GetData());*/
			} else
			{
				PRINTDEBUGLOG(dtInfo, "Open Chat Frame Failed in recv file");
			}
		} //end if (szFromName)
	} else if (::stricmp(szType, "filecancel") == 0)
	{
		//<trs type="filecancel" from="wuxiaozhong@gocom" to="admin@gocom" senderfileid="3"/>
		int nFileId = ::atoi(pNode->Attribute("senderfileid"));
		RemoveTransFile(nFileId, "对方取消了文件 %s 的传送请求", TRUE, pNode->Attribute("from"), TRUE);
	} else if (::stricmp(szType, "filedecline") == 0)
	{
		//<trs type="filedecline" from="admin@gocom" to="wuxiaozhong@gocom" senderfileid="1"/>
		int nFileId = ::atoi(pNode->Attribute("senderfileid"));
		RemoveTransFile(nFileId, "对方拒绝了文件 %s 的传送请求", TRUE);
	} else if (::stricmp(szType, "filestop") == 0)
	{
		//<trs type="filestop" from="admin@gocom" to="wuxiaozhong@gocom" senderfileid="1" senderid="wuxiaozhong@gocom"/>
		int nFileId = ::atoi(pNode->Attribute("senderfileid"));
		RemoveTransFile(nFileId, "对方中止了文件 %s 的传送请求", TRUE);
	} else if (::stricmp(szType, "rtoCancelPleaseControlMe") == 0) //取消远程控制，
	{
		//<trs type="rtoCancelPleaseControlMe" from="admin@gocom" to="wuxiaozhong@GoCom" channelid="4"/>		
		if (pNode->Attribute("channelid"))
		{
			DWORD dwChlId = ::atoi(pNode->Attribute("channelid"));

			CStdString_ strTip;
			if (m_rmcInfo.m_bRequest)
			{
				strTip = L"对方拒绝了您的远程协助请求";
			} else
				strTip = L"对方取消了与您的远程协助请求";
			AnsycShowTip(m_rmcInfo.m_hOwner, strTip.GetData()); 
			::PostMessage(GetMainFrameHWND(), WM_CHAT_COMMAND, (WPARAM)m_rmcInfo.m_hOwner, (LPARAM)CHAT_COMMAND_CLEAR_RMC_CHL);
		}
	} else if (::stricmp(szType, "RtoRequest") == 0) //请求远程控制
	{
		//<trs type="RtoRequest" from="wuxiaozhong@gocom" fromname="邬晓忠" szkey="a" to="admin@gocom" locip="192.168.1.100" locport="9911" remotip="123.116.118.30" remotport="9911" channelid="2"/>
		const char *szFromName = pNode->Attribute("from");
		if (szFromName)
		{
			PlayTipSound("friend", szFromName, FALSE);
			HWND h = GetHWNDByUserName(szFromName);
			if (h == NULL)
			{
				IUIManager *pUI = NULL;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
				{
					HRESULT hr;
					pUI->SendMessageToWindow(UI_MAIN_WINDOW_NAME, WM_OPENCHATFRAME, 
						      0, LPARAM(szFromName), &hr);
					h = GetHWNDByUserName(szFromName);
					pUI->Release();
				} //end if (m_pCore && ...
			} else 
			{
				/*if (m_pCore)
				{
					m_pCore->StartTrayIcon("P2P", "收到远程控制请求", NULL);
				}*/
			}//end if (h == NULL

			if (h != NULL)
			{
				if (pNode->Attribute("remotip"))
					m_rmcInfo.m_strPeerInternetIp = pNode->Attribute("remotip");
				else
					m_rmcInfo.m_strPeerInternetIp = "";
				if (pNode->Attribute("locip"))
					m_rmcInfo.m_strPeerIntranetIp = pNode->Attribute("locip");
				else
					m_rmcInfo.m_strPeerIntranetIp = "";
				if (pNode->Attribute("szkey"))
					m_rmcInfo.m_strKey = pNode->Attribute("szkey");
				else
					m_rmcInfo.m_strKey = "";
				m_rmcInfo.m_strPeerName = szFromName;
				m_rmcInfo.m_nChlId = ::atoi(pNode->Attribute("channelid"));
				m_rmcInfo.m_wPeerInternetPort = ::atoi(pNode->Attribute("remotport"));
				m_rmcInfo.m_wPeerIntranetPort = ::atoi(pNode->Attribute("locport")); 
				m_rmcInfo.m_hOwner = h;
				m_rmcChlId = m_rmcInfo.m_nChlId;
				CStdString_ strTip = _T("收到\"");
				TCHAR szTmp[MAX_PATH] = {0};
				CStringConversion::UTF8ToWideChar(GetRealNameByHWND(h), szTmp, MAX_PATH - 1);	
				strTip += szTmp;
				strTip += _T("\" 的远程协助请求 \"");
 
				//wsprintf(szTmp, L"\"\n \t\t <接受,%d>,<拒绝,%d>", 
				//	            ((m_rmcChlId << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_RMC_ACCEPT, 
				//				((m_rmcChlId << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_RMC_REFUSE);
				//strTip += szTmp;
				AnsycShowTip(h, strTip.GetData());	
				::PostMessage(GetMainFrameHWND(), WM_CHAT_COMMAND, (WPARAM)h, (LPARAM)CHAT_COMMAND_TAB2RMC_CONTROL);
			} else
			{
				PRINTDEBUGLOG(dtInfo, "Open Chat Frame Failed in recv file");
			}
		} //end 
	} else if (::stricmp(szType, "rtoCancelStartResponse") == 0) //取消请求
	{
		//<trs type="rtoCancelStartResponse" from="wuxiaozhong@gocom" to="admin@gocom" channelid="2"/>
		if ((m_rmcInfo.m_nChlId > 0) && ::stricmp(pNode->Attribute("from"), m_rmcInfo.m_strPeerName.c_str()) == 0)
		{
			/**/
			AnsycShowTip(m_rmcInfo.m_hOwner, L"对方取消了与您的远程协助请求");
			::PostMessage(GetMainFrameHWND(), WM_CHAT_COMMAND, (WPARAM)m_rmcInfo.m_hOwner, (LPARAM)CHAT_COMMAND_CLEAR_RMC_CHL);
		}
	} else if (::stricmp(szType, "rtoAcceptStartResponse") == 0) //接受远程请求
	{
		//<trs type="rtoAcceptStartResponse" from="wuxiaozhong@gocom" to="admin@GoCom" channelid="2"/>
		// ack <trs type="rtoPleaseControlMe" from="admin@gocom" to="wuxiaozhong@gocom" channelid="3"/>
		if (::stricmp(pNode->Attribute("from"), m_rmcInfo.m_strPeerName.c_str()) == 0)
		{
			std::string strXml = "<trs type=\"rtoPleaseControlMe\" from=\"";
			strXml += m_strUserName;
			strXml += "\" to=\"";
			strXml += m_rmcInfo.m_strPeerName;
			strXml += "\" channelid=\"";
			char szTmp[20] = {0};
			::itoa(m_rmcInfo.m_nChlId, szTmp, 10);
			strXml += szTmp;
			strXml += "\"/>";
			if (m_pCore)
				m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (int) strXml.size(), 0);
		}
	} else if (::stricmp(szType, "VideoRequest") == 0) //视频聊天请求
	{
		//<trs type="VideoRequest" from="wuxiaozhong@gocom" fromname="邬晓忠" szkey="a" to="admin@gocom" locip="192.168.1.100" locport="9911" remotip="123.116.118.30" remotport="9911" channelid="2"/>
		const char *szFromName = pNode->Attribute("from");
		if (szFromName)
		{
			if (m_VideoChlId == 0)
			{
				PlayTipSound("video", szFromName, TRUE);
				HWND h = GetHWNDByUserName(szFromName);
				if (h == NULL)
				{
					IUIManager *pUI = NULL;
					if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
					{
						HRESULT hr;
						pUI->SendMessageToWindow(UI_MAIN_WINDOW_NAME, WM_OPENCHATFRAME, 
							      0, LPARAM(szFromName), &hr);
						h = GetHWNDByUserName(szFromName);
						pUI->Release();
					} //end if (m_pCore && ...*/
				} else 
				{
					/*if (m_pCore)
					{
						m_pCore->StartTrayIcon("P2P", "收到视频聊天请求", NULL);
					}*/
				}//end if (h == NULL

				if (h != NULL)
				{					
				    ::PostMessage(GetMainFrameHWND(), WM_CHAT_COMMAND, (WPARAM)h, (LPARAM)CHAT_COMMAND_TAB2AV);	
					if (pNode->Attribute("remotip"))
						m_VideoInfo.m_strPeerInternetIp = pNode->Attribute("remotip");
					else
						m_VideoInfo.m_strPeerInternetIp = "";
					if (pNode->Attribute("locip"))
						m_VideoInfo.m_strPeerIntranetIp = pNode->Attribute("locip");
					else
						m_VideoInfo.m_strPeerIntranetIp = "";
					if (pNode->Attribute("szkey"))
						m_VideoInfo.m_strKey = pNode->Attribute("szkey");
					else
						m_VideoInfo.m_strKey = "";
					m_VideoInfo.m_strPeerName = szFromName;
					m_VideoInfo.m_nChlId = ::atoi(pNode->Attribute("channelid"));
					m_VideoInfo.m_wPeerInternetPort = ::atoi(pNode->Attribute("remotport"));
					m_VideoInfo.m_wPeerIntranetPort = ::atoi(pNode->Attribute("locport")); 
					m_VideoInfo.m_hOwner = h;
					m_VideoChlId = m_VideoInfo.m_nChlId;
					CStdString_ strTip = _T("收到\"");
					TCHAR szTmp[MAX_PATH] = {0};
					CStringConversion::UTF8ToWideChar(GetRealNameByHWND(h), szTmp, MAX_PATH - 1);	
					strTip += szTmp;
					strTip += _T("\" 的视频聊天请求 \"");
	 
					wsprintf(szTmp, L"\"\n \t\t <接受,%d>,<拒绝,%d>", 
						            ((m_VideoChlId << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_V_ACCEPT, 
									((m_VideoChlId << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_V_REFUSE);
					strTip += szTmp;
					AnsycShowTip(h, strTip.GetData());				
				} else
				{
					PRINTDEBUGLOG(dtInfo, "Open Chat Frame Failed in recv file");
				}
			} else
			{
				//
				SendCancelVideoProtocol(szFromName, pNode->Attribute("channelid"), "正在进行视频通信");
			}//end if (m_VideoChlId > 0)
		} //end if (szFromName)
	} else if (::stricmp(szType, "VideoCancel") == 0) //取消视频请求
	{
		if (m_VideoChlId > 0)
		{ 	
			const char *szReason = pNode->Attribute("reason");
			if (szReason && ::strlen(szReason) > 0)
			{
				TCHAR szwTmp[MAX_PATH] = {0};
				TCHAR szwReason[256] = {0};
				CStringConversion::StringToWideChar(szReason, szwReason, 255);
				wsprintf(szwTmp, L"对方由于\"%s\", 取消了与您的视频通话", szwReason);
				AnsycShowTip(m_VideoInfo.m_hOwner, szwTmp);
			} else
			{
				AnsycShowTip(m_VideoInfo.m_hOwner, L"对方取消了与您的视频通话");
			}
			::PostMessage(GetMainFrameHWND(), WM_CHAT_COMMAND, (WPARAM)m_VideoInfo.m_hOwner, (LPARAM)CHAT_COMMAND_TAB2INFO);
			CancelCustomLink(m_VideoInfo.m_hOwner, m_VideoInfo.m_nChlId, CUSTOM_LINK_FLAG_V_ACCEPT | CUSTOM_LINK_FLAG_V_REFUSE);			 
//			::VideoStopCapture();
			memset(&m_VideoInfo, 0, sizeof(m_VideoInfo));
			m_VideoChlId = 0;	
		}
	} else if (::stricmp(szType, "custompic") == 0) //自定义图片下载
	{
		// <trs type="custompic" from="wuxiaozhong@gocom"  to="admin@gocom" 
		//   filename="30f79afad4318774e447dc2db96936e0.gif" fileserver="http://imbs.smartdot.com.cn:9910" 
		DoCustomPicProtocol(pNode);
	}
	return FALSE;
}

BOOL CChatFrameImpl::DoCustomPicProtocol(TiXmlElement *pNode)
{
	std::string strFlag;
	const char *szFromName = pNode->Attribute("from");
	const char *szSvrAddr = pNode->Attribute("fileserver");
	const char *szSrcFile = pNode->Attribute("filename");
	if (szFromName && szSvrAddr && szSrcFile)
	{
		strFlag = szSrcFile;
		int nPos = strFlag.find(".");
		if (nPos != std::string::npos)
			strFlag = strFlag.substr(0, nPos);
		char szFileName[MAX_PATH] = {0};
		sprintf(szFileName, "%s%s.gif", GetImagePath(), strFlag.c_str());
		//
		std::string strUrl = szSvrAddr;
		strUrl += "/custompics/";
		strUrl += szSrcFile;
		if (!CSystemUtils::FileIsExists(szFileName))
		{ 
			CCustomPicItem *pItem = new CCustomPicItem();
			pItem->m_hOwner =  GetHWNDByUserName(szFromName);
			pItem->m_strFlag = strFlag;
			pItem->m_pOverlapped = this;
			pItem->m_strLocalFileName = szFileName;  
			pItem->m_strPeerName = szFromName; 
			pItem->m_strUrl = strUrl;
			PRINTDEBUGLOG(dtInfo, "Recv Custom picture:%s", szFileName);
			if (m_CustomPics.AddItem(pItem))
			{	
				::P2SvrAddDlTask(strUrl.c_str(), szFileName, FILE_TYPE_CUSTOMPIC, 
					 pItem, HttpDlCallBack, FALSE);
			} else
				delete pItem; 
			return TRUE;
		} else
		{
			//
		}
	}
	return FALSE;
}

void CALLBACK HttpDlCallBack(int nErrorCode, int nType, WPARAM wParam, LPARAM lParam, void *pOverlapped)
{
	switch(nErrorCode)
	{
	case ERROR_CODE_COMPLETE:
		{
			if (nType == FILE_TYPE_CUSTOMPIC)
			{
				CCustomPicItem *pItem = (CCustomPicItem *)pOverlapped;
				CChatFrameImpl *pThis = (CChatFrameImpl *)pItem->m_pOverlapped;
				HWND h = pThis->GetHWNDByUserName(pItem->m_strPeerName.c_str());
				if (h)
				{
					::SendMessage(h, WM_CHAT_REPLACE_PIC, (WPARAM)pItem->m_strLocalFileName.c_str(),
						(LPARAM)pItem->m_strFlag.c_str()); 
				}
				PRINTDEBUGLOG(dtInfo, "recv custom picture:%s", pItem->m_strLocalFileName.c_str());
				pThis->m_CustomPics.DeleteItem(pItem);
			} else if (nType == FILE_TYPE_NORMAL)
			{
				CCustomPicItem *pItem = (CCustomPicItem *)pOverlapped;
				CChatFrameImpl *pThis = (CChatFrameImpl *)pItem->m_pOverlapped;
				HWND h = pThis->GetHWNDByUserName(pItem->m_strPeerName.c_str());
				if (h)
				{
					std::string strTip = "文件 %%FILE%% 接收完毕";
					pThis->AnsyShowFileLink(h, strTip.c_str(), pItem->m_strLocalFileName.c_str()); 
					char szTime[64] = {0};
					CSystemUtils::DateTimeToStr((DWORD)::time(NULL), szTime);
					std::string strMsg, strBody;
					strBody = "离线文件";
					strBody += pItem->m_strLocalFileName.c_str();
					strBody += " 接收成功";
					strMsg = "<tip datetime=\"";
					strMsg += szTime;
					strMsg += "\">";
					strMsg += strBody;
					strMsg += "</tip>";
					pThis->SaveP2PMsg("p2p", pItem->m_strPeerName.c_str(), pThis->m_strUserName.c_str(), szTime, 
						strMsg.c_str(), strBody.c_str());
					
					pThis->RemoveTransFile(pItem->m_nFileId, NULL, TRUE);
				}
				pThis->m_CustomPics.DeleteItem(pItem);
			}
			//end if (wParam == FILE_TYPE_CUSTOMPIC)
			break;
		} //end case ERROR_CODE_COMPLETE
 	case ERROR_CODE_PROGRESS: //进度
		{
			if (nType == FILE_TYPE_NORMAL)
			{
				//显示进度				
				CCustomPicItem *pItem = (CCustomPicItem *)pOverlapped;				
				CChatFrameImpl *pThis = (CChatFrameImpl *)pItem->m_pOverlapped;
				::PostMessage(pThis->GetMainFrameHWND(), WM_CHAT_UPDL_PROGR, pItem->m_nFileId, lParam);
			}
			break; 
		}
	} //end switch(nErrorCode)
}

void CChatFrameImpl::ClearRmcChannel()
{	
	if (m_rmcInfo.m_hFullScreen != NULL)
		CloseRmcFullWindow();
	CancelCustomLink(m_rmcInfo.m_hOwner, m_rmcChlId, CUSTOM_LINK_FLAG_RMC_ACCEPT | CUSTOM_LINK_FLAG_RMC_REFUSE
				        | CUSTOM_LINK_FLAG_RMC_CTRL);
	::RmcCloseControl(m_rmcChlId, m_rmcInfo.m_strKey.c_str());
	if (m_TransFileList.HasOwnerWindow(m_rmcInfo.m_hOwner))
	{
		DoChatCommand(m_rmcInfo.m_hOwner, CHAT_COMMAND_TAB2FILE_PRO);
	} else
		DoChatCommand(m_rmcInfo.m_hOwner, CHAT_COMMAND_TAB2INFO);
	memset(&m_rmcInfo, 0, sizeof(m_rmcInfo));
	m_rmcChlId = 0;
	
}


void CChatFrameImpl::RemoveTransFileProgre(int nFileId, const char *szTip, BOOL bPost)
{
	CTransferFileInfo Info;  
	if (m_TransFileList.GetFileInfoById(nFileId, Info))
	{
		CancelFileTrans(Info.m_nLocalFileId);
		m_TransFileList.DeleteFileInfo(Info.m_nLocalFileId);
		TCHAR *szwTmp = new TCHAR[Info.m_strProFlag.GetLength() + 1];
		memset(szwTmp, 0, sizeof(TCHAR) * (Info.m_strProFlag.GetLength() + 1));
		lstrcpy(szwTmp, Info.m_strProFlag.GetData());
		//if (bPost)
		::PostMessage(GetMainFrameHWND(), WM_CHAT_RMFILE_PRO, WPARAM(Info.hOwner), LPARAM(szwTmp));	
		//else
		//	RMFileProgress(WPARAM(Info.hOwner), LPARAM(szwTmp));

		if (szTip && (!Info.m_strDspName.empty()))
		{
			char szTmp[512] = {0};
			TCHAR szwTmp[512] = {0};			
			sprintf(szTmp, szTip, Info.m_strDspName.c_str());
			CStringConversion::StringToWideChar(szTmp, szwTmp, MAX_PATH - 1); 
			ShowChatTipMsg(Info.hOwner, szwTmp);
		} //end if (.. 
	}
}

void CChatFrameImpl::RemoveTransFile(int nFileId, const char *szTip, BOOL bAnsy, const char *szPeerName, BOOL bByPeer)
{
	CTransferFileInfo Info; 
	BOOL bSucc = FALSE;
	if (bByPeer)
	{
		bSucc = m_TransFileList.GetFileInfoByPeerId(nFileId, szPeerName, Info);
	} else
		bSucc = m_TransFileList.GetFileInfoById(nFileId, Info);
	if (bSucc)
	{
		m_TransFileList.DeleteFileInfo(Info.m_nLocalFileId);
		if (bAnsy)
			::SendMessage(GetMainFrameHWND(), WM_CHAT_RMFILE_PRO, WPARAM(&Info), 0);		
		else
		{
			::SkinRemoveChildControl(Info.hOwner, L"fileprogress", Info.m_strProFlag.GetData());
			
		    CancelCustomLink(Info.hOwner, Info.m_nLocalFileId, CUSTOM_LINK_FLAG_RECV | CUSTOM_LINK_FLAG_SAVEAS
				              | CUSTOM_LINK_FLAG_CANCEL | CUSTOM_LINK_FLAG_REFUSE | CUSTOM_LINK_FLAG_OFFLINE);
			if (!m_TransFileList.HasOwnerWindow(Info.hOwner))
			{
				::SkinSetControlAttr(Info.hOwner, L"chattabtip", L"currentpage", L"infotab");;
			}
		}
		if (szTip && (!Info.m_strDspName.empty()))
		{
			char szTmp[512] = {0};
			TCHAR szwTmp[512] = {0};			
			sprintf(szTmp, szTip, Info.m_strDspName.c_str());
			CStringConversion::StringToWideChar(szTmp, szwTmp, MAX_PATH - 1);
			if (bAnsy)
				AnsycShowTip(Info.hOwner, szwTmp);
			else
				ShowChatTipMsg(Info.hOwner, szwTmp);
		} //end if (..
	}
}

#pragma warning(default:4996)
