#include <Commonlib/DebugLog.h>
#include <Core/common.h>
#include "CoreFrameWorkImpl.h"
#include "../IMCommonLib/InterfaceAnsiString.h"

#pragma warning(disable:4996)
//
BOOL CCoreFrameWorkImpl::DoMessageProtocol(const char *szType, TiXmlElement *pNode)
{
	BOOL bDid = FALSE; //是否再次派发给订阅者
	if (::stricmp(szType, "offlinemsg") == 0)
	{
		m_OfflineDid = TRUE;
		TiXmlElement *pChild = pNode->FirstChildElement();
		while (pChild)
		{
			TiXmlString strXml;
			pChild->SaveToString(strXml, 0);
			OnRecvProtocol(strXml.c_str(), (int) strXml.size());
			pChild = pChild->NextSiblingElement();
		}
		std::string strXml = "<msg type=\"getofflinemsg_ok\"/>";
		SendRawMessage((BYTE *)strXml.c_str(), (int) strXml.size(), 0);
		CInterfaceAnsiString strTmp;
		GetSvrParams("homepage", &strTmp, TRUE);
		bDid = TRUE;
		m_OfflineDid = FALSE;
	} else if (::stricmp(szType, "p2p") == 0)
	{
		//<msg type="p2p" from="admin@gocom" to="wuxiaozhong@gocom" Receipt="" datetime="2010-09-28 15:09:49">
        //<font name="Arial" size="9pt" color="#000000" bold="false" underline="false" strikeout="false" italic="false"/>
        //<body>adfadfadf</body>
        //  </msg>
		//
		const char *szUserName = pNode->Attribute("from");
		if (szUserName)
		{
			TiXmlElement *pChild = pNode->FirstChildElement();
			BOOL bMsg = TRUE;
			if (pChild) 
			{
				if ((::stricmp(pChild->Value(), "shake") == 0) 
					|| (::stricmp(pChild->Value(), "version") == 0)
					|| (::stricmp(pChild->Value(), "autoreply") == 0))
					bMsg = FALSE;
			}
			if (bMsg && stricmp(m_strPresence.c_str(), "away") == 0)
			{
				//自动回复
				CInterfaceAnsiString strMsg;
				if (SUCCEEDED(m_pConfigure->GetAutoReplyMessage(&strMsg)))
				{
					if (strMsg.GetSize() > 0)
					{
						//<msg type="p2p" from="%s" to="%s"><autoreply text="reply message" /></msg>
						std::string strXml;
						strXml = "<msg type=\"p2p\" from=\"";
						//name@domain
						strXml += m_strUserName;
						strXml += "@";
						strXml += m_strDomain;
						strXml += "\" to=\"";
						strXml += szUserName;
						strXml += "\"><autoreply text=\"";
						strXml += strMsg.GetData();
						strXml += "\"/></msg>";
						SendRawMessage((BYTE *)strXml.c_str(), (LONG) strXml.size(), 0);
					} //end if (strMsg
				} //end if (SUCCEEDED
			} //end if (stricmp(...
			TiXmlString strXml;
			pNode->SaveToString(strXml, 0);
			if (!m_ProtoList.DoProtocol("msg", szType, strXml.c_str(), strXml.size())) //子协议没处理，则加入到待处理列表
			{
				if (bMsg)
				{
					if (m_pConfigure)
				        m_pConfigure->PlayMsgSound("friend", szUserName, FALSE);
					m_PendingList.AddItem(szUserName, szType, strXml.c_str());

					//根据配置是否弹窗
					CInterfaceAnsiString strTmp;
					strTmp.SetString("true");
					m_pConfigure->GetParamValue(FALSE, "person", "autopopup", &strTmp);

					if ((::stricmp(strTmp.GetData(), "true") == 0) && m_pUIManager && (!m_OfflineDid))
					{
						LRESULT hr;
						m_pUIManager->SendMessageToWindow(UI_MAIN_WINDOW_NAME, WM_OPENCHATFRAME, 0, LPARAM(szUserName), &hr);
					} else
					{
						StartTrayIcon("p2p", "收到新消息", NULL);
					}//end if (m_pUIManager)
				} //end if (bMsg)
			} //end if (!m_ProtoList.DoProtocol("msg",
		} //end if (szUserName)
		bDid = TRUE;
	} else 
	{
		PRINTDEBUGLOG(dtInfo, "invalid Message Protocol, Type:%s", szType);
	}
	return bDid;
}

//
BOOL CCoreFrameWorkImpl::DoGroupMessage(TiXmlElement *pNode)
{
	TiXmlString strXml;
	pNode->SaveToString(strXml, 0);
	if (!m_ProtoList.DoProtocol("grp", "msg", strXml.c_str(), strXml.size())) //子协议没处理，则加入到待处理列表
	{
		const char *szGrpId = pNode->Attribute("guid");
		if (m_pConfigure)
	        m_pConfigure->PlayMsgSound("friend", szGrpId, FALSE);
		m_PendingList.AddItem(szGrpId, "grp", strXml.c_str());

		//根据配置是否弹窗
		CInterfaceAnsiString strTmp;
		strTmp.SetString("true");
		m_pConfigure->GetParamValue(FALSE, "person", "autopopup", &strTmp);

		if ((::stricmp(strTmp.GetData(), "true") == 0) && m_pUIManager && (!m_OfflineDid))
		{
			LRESULT hr;
			m_pUIManager->SendMessageToWindow(UI_MAIN_WINDOW_NAME, WM_OPENGROUPFRAME, WPARAM(szGrpId), 0, &hr);
		} else
		{
			StartTrayIcon("grp", "收到新消息", NULL);
		}//end if (m_pUIManager)
	}
	return TRUE;
}

#pragma warning(default:4996)
