#ifndef __SMARTPAINTMANAGER_H___
#define __SMARTPAINTMANAGER_H___
#include <commonlib/types.h>
#include <UILib/UIManager.h>
#include <UILib/UIResource.h>
#include <map>
class CSmartPaintManager :public CPaintManagerUI
{
public:
	CSmartPaintManager(void);
	~CSmartPaintManager(void);
public:
	//overridables
	virtual CPaintManagerUI *CreateInstance();
	virtual HWND HintWindow(HWND hParent);
	virtual BOOL GetImage(DWORD dwImageId, LPUI_IMAGE_ITEM *pImage) const;
    virtual UINT GetMenuCheckImage() const;
	virtual void GetScrollBarImage(UINT &uPrior, UINT &uMid, UINT &uNext, BOOL bVert) const;
	CMenuUI* LoadMenuUI(LPCWSTR lpszMenu ) ;
	CMenuUI* LoadMenuUI(const char *szMenuName) ;
	void ReleaseMenuUI(CMenuUI **pMenu) ;
    virtual int  GetGraphicLinkImageId(LPCTSTR lpszLink);
	//
	DWORD GetMaxBtnImageId();
	DWORD GetMinBtnImageId();
	DWORD GetRestoreBtnImageId();
	DWORD GetCloseBtnImageId();
	DWORD GetFormBkgImageId();
	DWORD GetMessageDlgBkgImageId();
	DWORD GetMsgBoxSuccImageId();
	DWORD GetMsgBoxQuestionImageId();
	DWORD GetMsgBoxInfoImageId();
	DWORD GetMsgBoxErrorImageId();

	//
	DWORD GetHintWindowBkgImageId();
protected:
	virtual void DoAlphaTopForm(HDC hdc, const RECT &rcPaint);
	virtual void DoShiftBackground(HDC hdc, RECT rc);
private:
	std::map<CStdString, CMenuUI *> m_MenuList;
};

#endif
