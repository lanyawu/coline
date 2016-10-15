#ifndef __SMARTTIPWND_H__
#define __SMARTTIPWND_H__

#include <uilib/uilib.h>
#include "SmartPaintManager.h"
#include "SmartWindow.h"

class CSmartTipWnd :public CSmartWindow
{
public:
	CSmartTipWnd(void);
	~CSmartTipWnd(void);
public:
    //INotifyUI overridable
	void Notify(TNotifyUI& msg);

	
	void SetParent(CWindowWnd *pParent);
	virtual void SetText(LPCTSTR szTitle);
	virtual std::string GetWindowName();
protected:
 	//CWindowWnd overridable
	LPCTSTR GetWindowClassName() const;
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual BOOL Init();
protected:
	virtual BOOL OnTrackActivate(WPARAM wParam, LPARAM lParam);
private:
	CRichEditUI *m_edtShow;
};

#endif

