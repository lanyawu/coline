#include "TrayMsgSysIcon.h"


CTrayMsgSysIcon::CTrayMsgSysIcon(HINSTANCE hInstance, ITrayMsgSysIconApp *pApp):
                 CTraySysIcon(hInstance),
				 m_pApp(pApp)
{
}


CTrayMsgSysIcon::~CTrayMsgSysIcon(void)
{
	//
}

//�������
void CTrayMsgSysIcon::OnLButtonClick(int nShiftState,int nX, int nY)
{
	if (m_pApp)
		m_pApp->OnLButtonClick(nShiftState, nX, nY);
}

//�Ҽ�����
void CTrayMsgSysIcon::OnRButtonClick(int nShiftState,int nX, int nY)
{
	if (m_pApp)
		m_pApp->OnRButtonClick(nShiftState, nX, nY);
}

//���˫��
void CTrayMsgSysIcon::OnLButtonDblClick(int nShiftState,int nX, int nY)
{
	if (m_pApp)
		m_pApp->OnLButtonDblClick(nShiftState, nX, nY);
}

//
void CTrayMsgSysIcon::OnBalloonShow()
{
	if (m_pApp)
		m_pApp->OnBalloonShow(0, 0, 0);
}

//
void CTrayMsgSysIcon::OnBalloonHide()
{
	if (m_pApp)
		m_pApp->OnBalloonHide(0, 0, 0);
}

//�Ҽ�˫��
void CTrayMsgSysIcon::OnRButtonDblClick(int nShiftState, int nX, int nY)
{
	if (m_pApp)
		m_pApp->OnRButtonDblClick(nShiftState, nX, nY);
}

//
void CTrayMsgSysIcon::OnToolTipShow(BOOL bActived)
{
	if (m_pApp)
		m_pApp->OnToolTipShow(bActived);
}
