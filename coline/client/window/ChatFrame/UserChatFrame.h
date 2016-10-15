#ifndef __USER_CHAT_FRAME_H___
#define __USER_CHAT_FRAME_H___

#include <commonlib/stringutils.h>

class CUserChatFrame
{
public:
	CUserChatFrame(const char *szUserName, HWND hWnd);
	~CUserChatFrame(void);
public: 
	BOOL IsUserNameFrame(const char *szUserName);
	void SetDispName(const char *szUTF8DspName);
	const char *GetUserName();
	const char *GetDspName();
	BOOL CheckIsOldVersion();
	void SetIsOldVersion(BOOL bOld);
	void StartShake();
	void Shake();

	BOOL m_bInitAckMenu;
	BOOL m_bShowAutoReply;
private:
	CAnsiString_ m_strUserName;
	std::string m_strUTF8DspName;
	HWND m_hWnd;

	//
	BOOL m_bOldVersion;

	//
	UINT_PTR m_uShakeTimer;
	int m_nShakeTimes;
	RECT m_rcSrcWindow;
};

#endif
