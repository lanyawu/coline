#include <commonlib/stringutils.h>
#include "SmartPaintManager.h"
#include "SmartUIResource.h"
#include "SmartTipWnd.h"

CMenuUI *LoadMenuUI(const char *szMenu)
{
	LPCONTROLNODE lpRoot = CSmartUIResource::Instance()->CreateMenuNodes(szMenu);
	if(lpRoot)
	{
		CDialogBuilder dlgBuilder;
		CMenuUI *pMenuUI = dynamic_cast<CMenuUI *>(dlgBuilder.CreateFromNode(lpRoot));
		return pMenuUI;
	}
	return NULL;
}


void ReleaseMenuUI(CMenuUI** pMenu)
{
	if((pMenu != NULL) && (*pMenu != NULL))
	{
		delete *pMenu;
		*pMenu = NULL;
	}
}

CSmartPaintManager::CSmartPaintManager(void)
{
}

CSmartPaintManager::~CSmartPaintManager(void)
{
}

DWORD CSmartPaintManager::GetHintWindowBkgImageId()
{
	return CSmartUIResource::Instance()->GetHintWindowBkgImageId();
}

HWND CSmartPaintManager::HintWindow(HWND hParent)
{
	//CSmartTipWnd *pWnd = new CSmartTipWnd();
	//pWnd->Create(hParent, L"HintWindow",  WS_POPUP,  WS_EX_TOPMOST, 0, 0, 1, 1);
	//return pWnd->GetHWND();
	return CPaintManagerUI::HintWindow(hParent);
}

void CSmartPaintManager::DoShiftBackground(HDC hdc, RECT rc)
{
	/*int r, g, b;
	CSmartUIResource::Instance()->GetBlendColorValue(r, g, b);
	if ((r != 0) || (g != 0) || (b!= 0))
		CBlueRenderEngineUI::DoShiftBitmap(hdc, this, r, g, b, rc);*/
}

void CSmartPaintManager::DoAlphaTopForm(HDC hdc, const RECT &rcPaint)
{
	 //
}

CPaintManagerUI *CSmartPaintManager::CreateInstance()
{
	return new CSmartPaintManager();
}

BOOL CSmartPaintManager::GetImage(DWORD dwImageId, LPUI_IMAGE_ITEM *pImage) const
{
	return CSmartUIResource::Instance()->GetImageById(dwImageId, pImage);
}


CMenuUI *CSmartPaintManager::LoadMenuUI(LPCWSTR lpszMenu)  
{
	CMenuUI *pMenu = NULL; 
	if(::lstrlen(lpszMenu) > 0)
	{ 
		std::map<CStdString, CMenuUI *>::iterator it = m_MenuList.find(lpszMenu);
		if (it != m_MenuList.end())
		{
			pMenu = it->second;
		} else
		{
			char szValue[512] = {0};
			CStringConversion::WideCharToString(lpszMenu, szValue, 512);
			pMenu = LoadMenuUI(szValue);
			m_MenuList.insert(std::pair<CStdString, CMenuUI *>(lpszMenu, pMenu)); 
		}
	}
	return pMenu;
}


CMenuUI *CSmartPaintManager::LoadMenuUI(const char *szMenuName)  
{
	LPCONTROLNODE lpRoot = CSmartUIResource::Instance()->CreateMenuNodes(szMenuName);
	if(lpRoot)
	{
		CDialogBuilder dlgBuilder;
		CMenuUI* pMenuUI = dynamic_cast<CMenuUI *>(dlgBuilder.CreateFromNode(lpRoot));
		return pMenuUI;
	}
	return NULL;
}


void CSmartPaintManager::ReleaseMenuUI(CMenuUI **pMenu) 
{
	if((pMenu != NULL) && (*pMenu != NULL))
	{
		std::map<CStdString, CMenuUI *>::iterator it;
		for (it = m_MenuList.begin(); it != m_MenuList.end(); it ++)
		{
			if (it->second == (*pMenu))
			{
				m_MenuList.erase(it);
				break;
			}
		} //end
		delete *pMenu;
		*pMenu = NULL;
	}
}

UINT CSmartPaintManager::GetMenuCheckImage() const
{
	return CSmartUIResource::Instance()->GetMenuCheckImageId();
}



void CSmartPaintManager::GetScrollBarImage(UINT &uPrior, UINT &uMid, UINT &uNext, BOOL bVert) const
{
	CSmartUIResource::Instance()->GetScrollBarImage(uPrior, uMid, uNext, bVert);
}

DWORD CSmartPaintManager::GetMaxBtnImageId()
{
	return CSmartUIResource::Instance()->GetMaxBtnImageId();
}

DWORD CSmartPaintManager::GetMinBtnImageId()
{
	return CSmartUIResource::Instance()->GetMinBtnImageId();
}

DWORD CSmartPaintManager::GetRestoreBtnImageId()
{
	return CSmartUIResource::Instance()->GetRestoreBtnImageId();
}

DWORD CSmartPaintManager::GetCloseBtnImageId()
{
	return CSmartUIResource::Instance()->GetCloseBtnImageId();
}

DWORD CSmartPaintManager::GetFormBkgImageId()
{
	return CSmartUIResource::Instance()->GetFormBkgImageId();
}

DWORD CSmartPaintManager::GetMessageDlgBkgImageId()
{
	return CSmartUIResource::Instance()->GetMessageDlgBkgImageId();
}

DWORD CSmartPaintManager::GetMsgBoxSuccImageId()
{
	return CSmartUIResource::Instance()->GetMsgBoxSuccImageId();
}

DWORD CSmartPaintManager::GetMsgBoxQuestionImageId()
{
	return CSmartUIResource::Instance()->GetMsgBoxQuestionImageId();
}

DWORD CSmartPaintManager::GetMsgBoxInfoImageId()
{
	return CSmartUIResource::Instance()->GetMsgBoxInfoImageId();
}

DWORD CSmartPaintManager::GetMsgBoxErrorImageId()
{
	return CSmartUIResource::Instance()->GetMsgBoxErrorImageId();
}

int  CSmartPaintManager::GetGraphicLinkImageId(LPCTSTR lpszLink)
{
	return CSmartUIResource::Instance()->GetLinkImageIdByLink(lpszLink);
}