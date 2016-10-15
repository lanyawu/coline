#ifndef __SMSFRAME_IMPL_H____
#define __SMSFRAME_IMPL_H____
#define WIN32_LEAN_AND_MEAN	
#include <ComBase.h>
#include <map>
#include <string>
#include <vector>
#include <smartskin/smartskin.h>
#include <Core/CoreInterface.h>
#include <Commonlib/SqliteDBOP.h>
#include <Commonlib/GuardLock.h>
#include <xml/tinyxml.h>

#include "../IMCommonLib/InstantUserInfo.h"
#include "../IMCommonLib/InterfaceUserList.h"

typedef struct CSmsPendItem
{
	DWORD dwCount; //����ʱ��
	std::string strContent;  //��������
	std::string strReceiver; //�ռ���
	
}SMS_PEND_ITEM, *LPSMS_PEND_ITEM;

class CSmsBuffer
{
public:
	CSmsBuffer() {};
	~CSmsBuffer() {};
public:
	std::map<std::string, std::string> m_SmsNumbers;  //<guid, mobilenumber>
	std::string m_strContent;
	DWORD m_wSendTime;
	UINT_PTR m_Timer;
};

class CSMSFrameImpl: public CComBase<>,
	                 public InterfaceImpl<ICoreEvent>,
				     public InterfaceImpl<IProtocolParser>,
					 public InterfaceImpl<ISMSFrame> 
{
public:
	CSMSFrameImpl(void);
	~CSMSFrameImpl(void);
public:
	//IUnknown
	STDMETHOD (QueryInterface)(REFIID riid, LPVOID *ppv);
	//ICoreEvent
	STDMETHOD (DoCoreEvent)(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
		                     LPARAM lParam, HRESULT *hResult);
	STDMETHOD (SetCoreFrameWork)(ICoreFrameWork *pCore);
	STDMETHOD (GetSkinXmlString)(IAnsiString *szXmlString);
	//
	STDMETHOD (CoreFrameWorkError)(int nErrorNo, const char *szErrorMsg);
		//�㲥��Ϣ
	STDMETHOD (DoBroadcastMessage)(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData);
	//
	STDMETHOD (DoWindowMessage)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes);
	//IProtocolParser
	STDMETHOD (DoRecvProtocol)(const BYTE *pData, const LONG lSize);
	STDMETHOD (DoPresenceChange)(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder);
	//ISMSFrame
	STDMETHOD (ShowSMSFrame)(LPRECT lprc, IUserList *pList); 
	//szPeerNumbers �Էֺ�;����
	STDMETHOD (SendSMS)(const char *szPeerNumbers, const char *szContent, const char *szSign);
	//
	STDMETHOD (ShowSmsView)(const char *szUserName, const char *szType);
	//
	//
	STDMETHOD (ShowFaxFrame)(LPRECT lprc, IUserList *pList);
	//��������Э��
	STDMETHOD (ParserSMSProtocol)(const char *szContent, IAnsiString *strSender, IAnsiString *strReceiver,
		IAnsiString *strTime, IAnsiString *strGuid, IAnsiString *strSign, IAnsiString *strText);
private:
	//
	HRESULT DoMenuCommand(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
	//
	HRESULT DoClickEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);

	BOOL GetLastInputNumbers(HWND hWnd, std::string &strNumbers);
	//
	BOOL GetInputNumbers(HWND hWnd, std::string &strNumbers);
	//
	void SendSms();
	BOOL CheckNumbersValid(const char *szInput, std::string &strNumbers, std::string &strFailed);
	void DoSelReceiver(HWND hWnd);
	//
	BOOL GetPhoneInfoByUserName(const char *szUserName, std::string &strPhoneInfo, BOOL bFax = FALSE);
	//
	BOOL GetUserNameByPhonInfo(IContacts *pContact, const char *szPhoneInfo, std::string &strNumber,
		std::string &strUserName, std::string &strRealName);
	//
	BOOL ShowUserListToInputFrame(HWND hWnd, IUserList *pList, const char *szMobile);
	//ͳ���ַ���
	void StatInputCount(HWND hWnd);
	//ͳ�ƽ���������
	void StatReceiverCount(HWND hWnd);
	//
	BOOL InitSmsView(const char *szUserName, const char *szType);
	void DisplayHistoryMsg(int nPageCount);
	void DisplayMessageList(IMessageList *pList);
	//ɾ����ǰѡ��ļ�¼
	void DeleteCurrSmsMsg();
	//�鿴��ǰѡ��ļ�¼
	void ViewCurrSmsMsg();
	//
	void DoSendFaxFrame(HWND hWnd);
	//
	void SendFax(HWND hWnd);
	//
	void SendFaxFileToSvr(const char *szReceiverName, const char *szFileName, const char *szTitle);
	//
	void ShowSMSContent(const char *szSender, const char *szTime, const char *szContent);
	int  GetCurrSmsMsgId(int &nUISel);

	BOOL SenderGuidIsExists(const char *szGuid, BOOL bDelete);

 
	//
   static DWORD WINAPI CheckSmsStatusThread(LPVOID lpParam); //���״̬�߳�
private:
	HWND m_hMainWnd;
	ICoreFrameWork *m_pCore;
	HWND m_hWnd;

	//SMS VIEW 
	HWND m_hViewWnd;
	std::string m_strUserName;
	int m_nTotalPage;
	int m_nCurrPage;

	//
	std::string m_strShowType; //fax or sms
	//fax frame
	HWND m_hFaxWnd;
	//
	HWND m_hShowWnd;
	//
	CGuardLock m_smsPendLock;
    std::map<std::string, LPSMS_PEND_ITEM> m_SmsPendList;

	//
	CGuardLock m_SmsBufferLock;
	std::vector<CSmsBuffer *> m_SmsBuffer;
	HANDLE m_SignThread; //�߳��ź�
	HANDLE m_hThread;
	BOOL m_bTerminated;
};


#endif
