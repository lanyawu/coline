#ifndef __TRAYMSGSYSICON_H____
#define __TRAYMSGSYSICON_H____
#include <commonlib/traysysicon.h>

class ITrayMsgSysIconApp
{
public:
	virtual ~ITrayMsgSysIconApp() {};
	//�������
	virtual void OnLButtonClick(int nShiftState,int nX, int nY) = 0;
	//�Ҽ�����
	virtual void OnRButtonClick(int nShiftState,int nX, int nY) = 0;
	//���˫��
	virtual void OnLButtonDblClick(int nShiftState,int nX, int nY) = 0;
	//�Ҽ�˫��
	virtual void OnRButtonDblClick(int nShiftState, int nX, int nY) = 0;
	//
	virtual void OnBalloonShow(int nShiftState, int nX, int nY) = 0;
	//
	virtual void OnBalloonHide(int nShiftState, int nX, int nY) = 0;
	//
	virtual void OnToolTipShow(BOOL bActived) = 0;
};

class CTrayMsgSysIcon :public CTraySysIcon
{
public:
	CTrayMsgSysIcon(HINSTANCE hInstance, ITrayMsgSysIconApp *pApp);
	~CTrayMsgSysIcon(void);
public:
	//�������
	virtual void OnLButtonClick(int nShiftState,int nX, int nY);
	//�Ҽ�����
	virtual void OnRButtonClick(int nShiftState,int nX, int nY);
	//���˫��
	virtual void OnLButtonDblClick(int nShiftState,int nX, int nY);
	//�Ҽ�˫��
	virtual void OnRButtonDblClick(int nShiftState, int nX, int nY);
	//
	//
	virtual void OnBalloonShow();
	//
	virtual void OnBalloonHide();
	//
	virtual void OnToolTipShow(BOOL bActived);
private:
	ITrayMsgSysIconApp *m_pApp;
};

#endif
