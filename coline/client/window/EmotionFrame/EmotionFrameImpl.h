#ifndef __EMOTIONFRAME_IMPL_H____
#define __EMOTIONFRAME_IMPL_H____

#include <ComBase.h>
#include <Commonlib/stringutils.h>
#include <smartskin/smartskin.h>
#include <Core/CoreInterface.h> 
#include <xml/tinyxml.h>
#include <map>

class CEmotionFrameImpl: public CComBase<>,
	                     public InterfaceImpl<ICoreEvent>,
						 public InterfaceImpl<IEmotionFrame>
{
public:
	CEmotionFrameImpl(void);
	~CEmotionFrameImpl(void);
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
	//IEmotionFrame
	STDMETHOD (ShowEmotionFrame)(ICoreEvent *pOwner, HWND hOwner, int x, int y);
	//
	STDMETHOD (GetSysEmotion)(const char *szTag, IAnsiString *strFileName);
	//
	STDMETHOD (GetDefaultEmotion)(const char *szStyle, IAnsiString *strFileName);
	//
	STDMETHOD (GetCustomEmotion)(const char *szTag, IAnsiString *strFileName);
private:
	void InitEmotion();
	BOOL GetEmotionXmlFile(std::string &strFileName, BOOL bSysEmotion);
	//��ʾ���� nPage ��ʼҳ bSysEmotion �Ƿ���ϵͳ����
	BOOL DisplayEmotion(int nPage, BOOL bSysEmotion);
	//
	void AddCustomEmotionEvent(HWND hWnd, const char *szFileName);
	//
	BOOL AddEmotion(HWND hWnd);
	//
	void BrowserPicture(HWND hWnd);
	//
	BOOL AddEmotion2Xml(const char *szFileName, const char *szMd5, const char *szShortCut, const char *szComment);
	//
	void RefrehPageTip();
	//
	HRESULT DoMenuCommand(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam);
private:
	ICoreFrameWork *m_pCore;
	ICoreEvent *m_pOwnerEvent;
	HWND m_pOwnerWnd;
	BOOL m_bSysTab;
	std::map<CAnsiString_, std::string> m_EmotionTables;
	std::map<CAnsiString_, std::string> m_CusEmotionTables;
	std::string strDefaultSendFileName;
	std::string strDefaultErrorFileName;
	HWND m_hWnd;
	//
	int m_nSysPage;
	int m_nCustomPage;
	int m_nTotalSysCount; //�����������ҳ
	int m_nTotalCustomCount; //�����������ҳ
	TiXmlDocument *m_SysEmotionDoc;
	TiXmlDocument *m_CusEmotionDoc;
	std::string m_strSysEmotionPath;
	std::string m_strCusEmotionPath;
};

#endif
