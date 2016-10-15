#ifndef __SMARTMESSAGEBOX_H___
#define __SMARTMESSAGEBOX_H___

#include "SmartWindow.h"

const TCHAR MSGBOX_ICON_DISPLAY[] = _T("displayicon");//提示图标
const TCHAR MSGBOX_TEXT_DISPLAY[] = _T("displaymessage");//提示消息文本
const TCHAR MSGBOX_ICONTEXT_PADDING[] = _T("icontextpadding");//提示图标、文本间隔区域
const TCHAR MSGBOX_PADDING[] = _T("buttonpadding");//上面两个按钮之间的间隔
const TCHAR MSGBOX_TITLE_DISPLAY[] = _T("title"); //标题窗口
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
