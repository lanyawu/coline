#include "UserChatFrame.h"

#define SHAKE_TIMER_EVENT_ID  0x8743F2
#define SHAKE_TIMES           16
#define SHKE_OFFSET_DISTANT   8

CUserChatFrame::CUserChatFrame(const char *szUserName, HWND hWnd):
                m_hWnd(hWnd),
				m_strUserName(szUserName),
				m_bOldVersion(TRUE),
				m_bInitAckMenu(FALSE),
				m_uShakeTimer(NULL),
				m_bShowAutoReply(TRUE),
				m_nShakeTimes(0)
{
	//
}


CUserChatFrame::~CUserChatFrame(void)
{
 
}

BOOL CUserChatFrame::IsUserNameFrame(const char *szUserName)
{
	if ((!m_strUserName.IsEmpty()) && szUserName)
		return (m_strUserName == szUserName);
	return FALSE;
}

const char *CUserChatFrame::GetUserName()
{
	return m_strUserName.c_str();
}

void CUserChatFrame::SetDispName(const char *szUTF8DspName)
{
	if (szUTF8DspName)
		m_strUTF8DspName = szUTF8DspName;
}

const char *CUserChatFrame::GetDspName()
{
	return m_strUTF8DspName.c_str();
}

BOOL CUserChatFrame::CheckIsOldVersion()
{
	return FALSE;
	//return m_bOldVersion;
}

void CUserChatFrame::StartShake()
{
	if ((m_hWnd != NULL) && (::IsWindow(m_hWnd)))
	{
		if (m_uShakeTimer == NULL)
		{
			m_uShakeTimer = ::SetTimer(m_hWnd, SHAKE_TIMER_EVENT_ID, 30, NULL);
			m_nShakeTimes = SHAKE_TIMES;

			::GetWindowRect(m_hWnd, &m_rcSrcWindow);
		}
	}
}

void CUserChatFrame::Shake()
{
	if ((m_uShakeTimer != NULL) && (m_nShakeTimes >= 0))
	{		
		int X = m_rcSrcWindow.left, Y = m_rcSrcWindow.top;
		if (m_nShakeTimes > 0)
		{
			int n = m_nShakeTimes % 4;
			switch(n)
			{ 
			case 3:
				 X += SHKE_OFFSET_DISTANT;
				 break; 
			case 2:
				 Y -= SHKE_OFFSET_DISTANT;
				 break; 
			case 1:
				 X -= SHKE_OFFSET_DISTANT;
				 break; 
			case 0:
				 Y += SHKE_OFFSET_DISTANT;
				 break; 
			}
		} else
		{		
			::KillTimer(m_hWnd, m_uShakeTimer);
			m_uShakeTimer = 0;
		}
		::MoveWindow(m_hWnd, X, Y, m_rcSrcWindow.right - m_rcSrcWindow.left, m_rcSrcWindow.bottom - m_rcSrcWindow.top, TRUE);
		m_nShakeTimes --;
	}
}

void CUserChatFrame::SetIsOldVersion(BOOL bOld)
{
	m_bOldVersion = bOld;
}
