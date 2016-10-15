#include "SmartMessageBox.h"

const TCHAR MESSAGE_STD_BTN_CANCEL[] = _T("cancel");
const TCHAR MESSAGE_STD_BTN_OK[] = _T("ok");
//宽高比3：2比较好
const int TEXT_WIDTH_MIN = 120;
const int TEXT_WIDTH_MAX = 900;
const int TEXT_HEIGHT_MIN = 80;
const int TEXT_HEIGHT_MAX = 600;

const int WND_HORZ_PADDING = 31;
const int WND_VERT_CTRL_PADDING = 60;
const int WND_VERT_TEXT_PADDING_1 = 40;
const int WND_VERT_TEXT_PADDING_2 = 80;



CSmartMessageBox::CSmartMessageBox(void):
                  CSmartWindow(NULL, "MessageBox")
{

}

CSmartMessageBox::~CSmartMessageBox(void)
{

}

 
std::string CSmartMessageBox::GetWindowName() const
{
	return WNDNAME_MESSAGEBOX;
}

void CSmartMessageBox::Notify(TNotifyUI& msg)
{
	if (msg.sType == _T("click"))
	{
		if (msg.pSender->GetName() == MESSAGE_STD_BTN_OK)
		{
			m_nModalResult = IDOK;
			Close();
			msg.bHandled = TRUE;
		} else if (msg.pSender->GetName() == MESSAGE_STD_BTN_CANCEL)
		{
			m_nModalResult = IDCANCEL;
			Close();
			msg.bHandled = TRUE;
		}
	}
	if (!msg.bHandled)
		CSmartWindow::Notify(msg);
}

void CSmartMessageBox::OnFinalMessage(HWND hWnd)
{
}

UINT CSmartMessageBox::GetClassStyle() const
{
	return CS_DBLCLKS;
}

UINT CSmartMessageBox::DoModal()
{
	Center();
	return ShowModal();
}

BOOL CSmartMessageBox::CreateMessageBox(HWND hParent, LPCTSTR szContent, LPCTSTR szCaption, UINT uStyle)
{
	m_hParent = hParent;
	if(szCaption != NULL)
	{
		m_strCaption = szCaption;
	}
	if(szContent != NULL)
	{
		m_strContent = szContent;
	}
	m_uStyle = uStyle;

	BOOL bRes = Create(hParent, NULL, WS_POPUP, 0, CRect(0,0,0,0), NULL);
	if(bRes)
	{
		SIZE sz = EstimateWindowSize();
		::SetWindowPos(m_hWnd, HWND_TOP, 0, 0, 200,200, 0);
	}
	return bRes;
}

BOOL CSmartMessageBox::Init()
{ 
	CSmartWindow::Init();
	//hide the minimize and maximize box
	HideMinBtn();
	HideMaxRestoreBtn();
	//adjust image canvas
	StretchFixed sf;
	sf.SetFixed( 10 );
	sf.m_iTopLeftWidth = 80;
	sf.m_iTopHeight = 40;
	SetBkgndImage(m_paintMgr.GetMessageDlgBkgImageId(), sf, TRUE );
	//Set caption and content text 
	if (m_pTitle)
		m_pTitle->SetText(m_strCaption);
	m_pTextPanel = dynamic_cast<CTextPanelUI *>(FindControl(MSGBOX_TEXT_DISPLAY));
	if (m_pTextPanel)
		m_pTextPanel->SetText(m_strContent);

	CControlUI *pCtrl = NULL;
	//Adjust the buttons and icon according to the message box style
	if((m_uStyle & MBI_OKCANCEL) != 0) 
	{ 
		//ok and cancel button
		//load and set icon
	} else
	{
		//default m_nStyle == MBS_OK, ok button only
		pCtrl = FindControl(MESSAGE_STD_BTN_CANCEL);
		if(pCtrl)
			pCtrl->SetVisible(false);
	}

	m_pImage = dynamic_cast<CImagePanelUI *>(FindControl(MSGBOX_ICON_DISPLAY));
	if((m_uStyle & MBI_SUCCESS) != 0)
	{
		m_pImage->SetImage(m_paintMgr.GetMsgBoxSuccImageId());
	} else if((m_uStyle & MBI_QUESTION) != 0)
	{
		m_pImage->SetImage(m_paintMgr.GetMsgBoxQuestionImageId());	
	} else if((m_uStyle & MBI_INFORMATION) != 0)
	{
		m_pImage->SetImage(m_paintMgr.GetMsgBoxInfoImageId());
	} else if((m_uStyle & MBI_ERROR ) != 0)
	{
		m_pImage->SetImage(m_paintMgr.GetMsgBoxErrorImageId());
	} else
	{ 
		//default donot display any icon
		m_pImage->SetImage(IMGID_INVALID_);
		pCtrl = FindControl(MSGBOX_ICONTEXT_PADDING);
		pCtrl->SetVisible(false);
	}
	return TRUE;
}


SIZE CSmartMessageBox::EstimateWindowSize()
{
	int cxWnd = TEXT_WIDTH_MIN;
	int cyWnd = TEXT_HEIGHT_MIN;
	//determine the window size by text, images and so on
	SIZE szAvail = {9999, 9999};
	SIZE szImage = m_pImage->EstimateSize(szAvail);

	//WND_HORZ_PADDING : 横向上各个控件的间距总和
	szAvail.cx = TEXT_WIDTH_MIN;
	SIZE szText = m_pTextPanel->EstimateSize(szAvail);
	if(szText.cy > TEXT_HEIGHT_MIN)
	{ 
		//多行，且在120x80的区域内已经容纳不下
		szAvail.cx = min(TEXT_WIDTH_MIN * (szText.cy / TEXT_HEIGHT_MIN + 1), TEXT_WIDTH_MAX);
		szText = m_pTextPanel->EstimateSize(szAvail);
		cxWnd = WND_HORZ_PADDING + szImage.cx + szText.cx;
		cyWnd = min(cxWnd * 2 / 3, szText.cy + WND_VERT_CTRL_PADDING + WND_VERT_TEXT_PADDING_2);
		//程序运行到这儿，textpanel未必能容纳所有文字，不过文字
		//数量已经不小，不再计算
	} else
	{
		cxWnd = WND_HORZ_PADDING + szImage.cx + TEXT_WIDTH_MIN;
		cyWnd = max(cxWnd * 2 / 3, szText.cy + WND_VERT_CTRL_PADDING + WND_VERT_TEXT_PADDING_1);
	}
	
	return CSize(cxWnd, cyWnd);
}

void CSmartMessageBox::Center()
{
	CRect rcBox;
	::GetWindowRect(m_hWnd, &rcBox);

	int cxBox = rcBox.right - rcBox.left;
	int cyBox = rcBox.bottom - rcBox.top;

	CRect rcParent;
	::GetWindowRect(m_hParent, &rcParent);

    // Find messagebox's upper left based on rcParent
    int xLeft = (rcParent.left + rcParent.right) / 2 - cxBox / 2;
    int yTop = (rcParent.top + rcParent.bottom) / 2 - cyBox / 2;

	CRect rcArea;
	::SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcArea, NULL);
	// The dialog is outside the screen, move it inside
	if(xLeft < rcArea.left ) 
		xLeft = rcArea.left;
	else if(xLeft + cxBox > rcArea.right) 
		xLeft = rcArea.right - cxBox;
	if(yTop < rcArea.top)
		yTop = rcArea.top;
	else if(yTop + cyBox > rcArea.bottom) 
		yTop = rcArea.bottom - cyBox;

	::MoveWindow(m_hWnd, xLeft, yTop, cxBox, cyBox, FALSE);
}