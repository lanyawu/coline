#ifndef __SMARTMESSAGEBOX_H___
#define __SMARTMESSAGEBOX_H___

#include "SmartWindow.h"

const TCHAR MSGBOX_ICON_DISPLAY[] = _T("displayicon");//��ʾͼ��
const TCHAR MSGBOX_TEXT_DISPLAY[] = _T("displaymessage");//��ʾ��Ϣ�ı�
const TCHAR MSGBOX_ICONTEXT_PADDING[] = _T("icontextpadding");//��ʾͼ�ꡢ�ı��������
const TCHAR MSGBOX_PADDING[] = _T("buttonpadding");//����������ť֮��ļ��
const TCHAR MSGBOX_TITLE_DISPLAY[] = _T("title"); //���ⴰ��
class CSmartMessageBox :public CSmartWindow
{
public:
	CSmartMessageBox(void);
	~CSmartMessageBox(void);

public:
	BOOL CreateMessageBox(HWND hParent, LPCTSTR szContent, LPCTSTR szCaption, UINT uStyle);
	UINT DoModal();
	//INotifyUI overridable
protected:
	//CStandardWindow overridable
	std::string GetWindowName() const;
	BOOL Init();

	//CWindowWnd overridable
	UINT GetClassStyle() const;
	void OnFinalMessage(HWND hWnd);
	void Notify(TNotifyUI& msg);
private:
	SIZE EstimateWindowSize();
	void Center();
private:
	CTextPanelUI *m_pTextPanel;
	CImagePanelUI *m_pImage;
	CStdString m_strCaption;
	CStdString m_strContent;
	UINT m_uStyle;

	HWND m_hParent;
};

#endif
