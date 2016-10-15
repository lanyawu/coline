#include "common.h"

#include <UILib/UIProgressBar.h>
#include <UILib/UILabel.h>
#include <UILib/UIPanel.h>
#include <CommonLib/StringUtils.h>
#include <Commonlib/systemutils.h>
/////////////////////////////////////////////////////////////////////////////
//
//
#pragma warning(disable:4996)

CProgressBarUI::CProgressBarUI(): 
                m_iMax(100),
				m_iCurPos(0),
				m_iBkgndImageID(IMGID_INVALID_),
				m_iStepImageID(IMGID_INVALID_)
{
	m_szFixed.cx = m_szFixed.cy = 0;
}

CProgressBarUI::~CProgressBarUI()
{

}

LPCTSTR CProgressBarUI::GetClass() const
{
	return _T("ProgressBarUI");
}

UINT CProgressBarUI::GetControlFlags() const
{
	return UIFLAG_TABSTOP;
}

SIZE CProgressBarUI::EstimateSize(SIZE szAvailable)
{
	return m_szFixed;
}

void CProgressBarUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	RECT rcTemp = {0};
	if (!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem)) 
		return;
	RECT rcBar = m_rcItem;//the bar
	if (m_iBkgndImageID != IMGID_INVALID_)
	{
		CBlueRenderEngineUI::DoPaintGraphic(hDC, m_pManager, rcBar, m_iBkgndImageID);
	} else
	{
		rcBar.right -= 3;
		//background image / border
		CBlueRenderEngineUI::DoPaintRectangle(hDC, rcBar, m_clrBorder, m_clrBkgnd);
	}
	//progress TBD how to draw
	ASSERT( m_iCurPos <= m_iMax );
	double proportion = (double)m_iCurPos / (double)m_iMax;
	int cx = (int)(proportion * (double)(rcBar.right - rcBar.left));
	RECT rcProgress = rcBar;
	rcProgress.right = rcProgress.left + cx;
	if (m_iStepImageID != IMGID_INVALID_)
	{
		CBlueRenderEngineUI::DoPaintGraphic(hDC, m_pManager, rcProgress, m_iStepImageID);
	} else
		CBlueRenderEngineUI::DoFillRect(hDC, m_pManager, rcProgress,m_clrBorder, m_bTransparent);
}

void CProgressBarUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if ( _tcsicmp(pstrName, _T("bkgndimage")) == 0)
	{
		SetBkgndImage(_ttoi(pstrValue));
	} else if (_tcsicmp(pstrName, _T("progressimage")) == 0)
	{
		SetProgressImage(_ttoi(pstrValue));
	} else if (_tcsicmp(pstrName, _T("height") ) == 0)
	{
		int cy = _ttoi(pstrValue);
		SetSize(CSize(m_szFixed.cx, cy));
	} else if (_tcsicmp(pstrName, _T("position")) == 0)
	{
		int iPos = _ttoi(pstrValue);
		SetPosition(iPos);
	} else if (_tcsicmp(pstrName, _T("max")) == 0)
	{
		int iMax = _ttoi(pstrValue);
		SetExtension(iMax);
	} else
	{
		CControlUI::SetAttribute(pstrName, pstrValue);
	}
}

int CProgressBarUI::SetExtension(int iExt)
{
	if (iExt > 0)
	{
		// 0 -- m_iMax
		m_iMax = iExt;
		m_iCurPos = max(0, min(m_iMax, m_iCurPos));
		Invalidate();
	}
	return 0;
}

int CProgressBarUI::SetPosition(int iPos)
{
	int nPos = max(0, min(m_iMax, iPos));
	if (m_iCurPos != nPos)
	{
		m_iCurPos = nPos;
		Invalidate();
	}
	return nPos;
}

int CProgressBarUI::Progress(int iStep)
{
	int iPos = m_iCurPos;
	//parameters validate
	int iNewPos = max(0, min(m_iMax, iStep + m_iCurPos));
	if (iNewPos != m_iCurPos)
	{
		m_iCurPos = iNewPos;
		Invalidate();
	}
	return iPos;
}

int CProgressBarUI::SetSize(const SIZE& szNew)
{
	if ((szNew.cx != m_szFixed.cx) || (szNew.cy != m_szFixed.cy))
	{
		m_szFixed = szNew;
		return true;
	}
	return false;
}


SIZE CProgressBarUI::GetSize() const
{ 
	return m_szFixed; 
}


void CProgressBarUI::SetBkgndImage(UINT nImageID)
{
	if (m_iBkgndImageID != nImageID)
	{
		m_iBkgndImageID = nImageID;
		Invalidate();
	}
}


void CProgressBarUI::SetProgressImage(UINT nProgressImage)
{
	if (m_iStepImageID != nProgressImage)
	{
		m_iStepImageID = nProgressImage;
		Invalidate();
	}
}

/////////////////////////////////////////////////////////////////////////////
//
// CFileProgressBarUI height minimum 60 
CFileProgressBarUI::CFileProgressBarUI():
                    m_bShowFileIcon(TRUE)
{
	SetBorder(false);

	//padding control
	CPaddingPanelUI* pPadding = new CPaddingPanelUI();
	CVerticalLayoutUI::Add(pPadding);

	//file name label
	CHorizontalLayoutUI *pHorz = new CHorizontalLayoutUI();
	pHorz->SetHeight(30);
	CVerticalLayoutUI::Add(pHorz);
	m_pImage = new CNormalImagePanelUI();
	m_pImage->SetWidth(30);
	m_pImage->SetAttribute(L"inset", L"1 1 1 1");
	pHorz->Add(m_pImage);
	m_labelFileName = new CLabelPanelUI;//estimate height, font height 14 TBD
	pHorz->Add(m_labelFileName);
	
	pPadding = new CPaddingPanelUI();
	pPadding->SetHeight(5);
	CVerticalLayoutUI::Add(pPadding);

	//progress bar
	m_progressBar = new CProgressBarUI();//height = 10
	m_progressBar->SetSize(CSize(0, 10));//height
	CVerticalLayoutUI::Add(m_progressBar);

	//file size displaying layout
	CHorizontalLayoutUI* pHL = new CHorizontalLayoutUI();
	pHL->SetHeight(15);//height 15
	CVerticalLayoutUI::Add(pHL);

	m_labelTransSpeed = new CLabelPanelUI();
	pHL->Add(m_labelTransSpeed);
	
	pPadding = new CPaddingPanelUI();
	pHL->Add(pPadding);

	m_labelCurrentSize = new CLabelPanelUI();
	m_labelCurrentSize->SetWidth(50);
	m_labelCurrentSize->SetTextStyle(m_labelCurrentSize->GetTextStyle() & ~DT_LEFT | DT_RIGHT);
	pHL->Add(m_labelCurrentSize);

	m_labelSlash = new CLabelPanelUI();
	m_labelSlash->SetText( _T("/"));
	m_labelSlash->SetVisible(false);
	pHL->Add(m_labelSlash);

	m_labelFileSize = new CLabelPanelUI();
	pHL->Add(m_labelFileSize);
	
	pPadding = new CPaddingPanelUI();
	pPadding->SetWidth(3);
	pHL->Add(pPadding);
	
	//button 
	CHorizontalLayoutUI* pbtnHL = new CHorizontalLayoutUI();
	pbtnHL->SetHeight(20);//height 15
	CVerticalLayoutUI::Add(pbtnHL);

	m_pRecv = new CTextPanelUI();
	m_pRecv->EnableLink(TRUE);
	m_pRecv->SetText(L"接收");
	m_pRecv->SetWidth(30);
	m_pRecv->SetTextColor(0xFF0000);
	pbtnHL->Add(m_pRecv);
	m_pRecv->SetVisible(false);

	pPadding = new CPaddingPanelUI();
	pPadding->SetWidth(3);
	pbtnHL->Add(pPadding);

	m_pSaveAs =  new CTextPanelUI();
	m_pSaveAs->EnableLink(TRUE);
	m_pSaveAs->SetText(L"另存为");
	m_pSaveAs->SetWidth(40);
	m_pSaveAs->SetTextColor(0xFF0000);
	pbtnHL->Add(m_pSaveAs);	
	m_pSaveAs->SetVisible(false);
	//
	pPadding = new CPaddingPanelUI(); 
	pbtnHL->Add(pPadding);

	m_pSendOffline = new CTextPanelUI();
	m_pSendOffline->EnableLink(TRUE);
	m_pSendOffline->SetText(L"发送离线文件");
	m_pSendOffline->SetWidth(80);
	m_pSendOffline->SetTextColor(0xFF0000);
	pbtnHL->Add(m_pSendOffline);	
	m_pSendOffline->SetVisible(false);

	pPadding = new CPaddingPanelUI();
	pPadding->SetWidth(5);
	pbtnHL->Add(pPadding);

	m_pCancel = new CTextPanelUI();
	m_pCancel->EnableLink(TRUE);
	m_pCancel->SetText(L"取消");
	m_pCancel->SetWidth(30);
	m_pCancel->SetTextColor(0xFF0000); 
	pbtnHL->Add(m_pCancel); 

	pPadding = new CPaddingPanelUI();
	pPadding->SetWidth(10);
	pbtnHL->Add(pPadding);

	//padding control
	pPadding = new CPaddingPanelUI();
	CVerticalLayoutUI::Add(pPadding);
}


LPCTSTR CFileProgressBarUI::GetClass() const
{
	return _T("FileProgressBarUI");
}

UINT CFileProgressBarUI::GetControlFlags() const
{
	return 0;//..
}

SIZE CFileProgressBarUI::EstimateSize(SIZE szAvail)
{
	SIZE sz = m_cxyFixed;
	sz.cy = max(sz.cy, 75); 
	return sz;
}

void CFileProgressBarUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcsicmp(pstrName, _T("bkgndimage") ) == 0)
	{
		m_progressBar->SetAttribute(pstrName, pstrValue);
	} else if ( _tcsicmp(pstrName, _T("progressimage")) == 0)
	{
		m_progressBar->SetAttribute(pstrName, pstrValue);
	} else if (_tcsicmp(pstrName, _T("filename")) == 0)
	{
	    m_labelFileName->SetText(pstrValue );
	    m_labelFileName->SetToolTip(pstrValue);
		if (m_pImage)
		{
			m_pImage->SetImageByFileName(pstrValue);	
		}
	} else if (_tcsicmp(pstrName, _T("filesize")) == 0)
	{
		m_progressBar->SetExtension(_ttoi(pstrValue));
		m_labelFileSize->SetText(GetSizeString(_ttoi(pstrValue)));
	} else if (_tcsicmp(pstrName, _T("currfilesize")) == 0)
	{
		Progress(_ttoi(pstrValue));
	} else if (_tcsicmp(pstrName, _T("showicon")) == 0)
	{
		if (_tcsicmp(pstrValue, L"true") == 0)
		{
			m_bShowFileIcon = TRUE;
			if (m_pImage)
				m_pImage->SetVisible(true);
		} else
		{
			m_bShowFileIcon = FALSE;
			if (m_pImage)
				m_pImage->SetVisible(false);
		}
	} else if (_tcsicmp(pstrName, _T("progrestyle"))== 0)
	{
		if (_tcsicmp(pstrValue, L"recv") == 0)
		{
			m_pRecv->SetVisible(true);
			m_pSaveAs->SetVisible(true);
			m_pCancel->SetText(L"拒绝");
			m_pSendOffline->SetVisible(false);
		} else if (_tcsicmp(pstrValue, L"send") == 0)
		{
			m_pSendOffline->SetVisible(true);
			m_pRecv->SetVisible(false);
			m_pSaveAs->SetVisible(false);
		} else
		{
			m_pRecv->SetVisible(false);
			m_pSaveAs->SetVisible(false);
			m_pSendOffline->SetVisible(false);
		}
	} else if (_tcsicmp(pstrName, _T("name")) == 0)
	{
		CStdString strValue = L"fc_";
		strValue += pstrValue; 
		m_pCancel->SetName(strValue);
		strValue = L"fr_";
		strValue += pstrValue;
		m_pRecv->SetName(strValue);
		strValue = L"fs_";
		strValue += pstrValue;
		m_pSaveAs->SetName(strValue);
		strValue = L"fo_";
		strValue += pstrValue;
		m_pSendOffline->SetName(strValue);
		CVerticalLayoutUI::SetAttribute(pstrName, pstrValue);
	} else
	{
		CVerticalLayoutUI::SetAttribute(pstrName, pstrValue);
	}
}

void CFileProgressBarUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	RECT rcTemp = {0};
	if (!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem)) 
		return;
	//border
	if (HasBorder())
	{
		CBlueRenderEngineUI::DoPaintRectangle(hDC, m_rcItem, 
			                                  m_clrBorder, m_clrBkgnd);
	}
	//当前传输字节数
	m_labelCurrentSize->SetVisible(m_progressBar->GetCurStep() > 0);
	m_labelSlash->SetVisible(m_progressBar->GetCurStep() > 0);
	CVerticalLayoutUI::DoPaint(hDC, rcPaint);
}

void CFileProgressBarUI::SetBorderColor(COLORREF clrBorder)
{
	if (m_progressBar)
	{
		m_progressBar->SetBorderColor(clrBorder);
	}
}

CStdString CFileProgressBarUI::GetSizeString(UINT nSize)
{
	double dbSize = 0;
	char tmp[32] = { 0 };
	if (nSize < 1024)
	{
		sprintf(tmp, "%dB", nSize);
	} else if (nSize < 1024 * 1024)
	{
		dbSize = (double)nSize / 1024;
		sprintf(tmp, "%.2fK", (double)nSize / 1024);
	} else 
	{
		sprintf(tmp, "%.2fM", (double)nSize / 1024 / 1024);
	} 
#ifdef UNICODE 
	TCHAR tcTmp[32] = { 0 };
	CStringConversion::StringToWideChar(tmp, tcTmp, 32);
	return tcTmp;
#else
	return tmp;
#endif
}

void CFileProgressBarUI::SetFileInfo(const CStdString& sFileName, UINT nFileSize)
{
	//nFileSize bytes
	m_labelFileName->SetText(sFileName );
	m_labelFileName->SetToolTip(sFileName);
	m_progressBar->SetExtension(nFileSize);
	m_labelFileSize->SetText(GetSizeString(nFileSize));
}

int CFileProgressBarUI::Progress(int iStep)
{
	int iSize = m_progressBar->GetCurStep();
	int iOldStep = m_progressBar->Progress(iStep - iSize);
	m_labelCurrentSize->SetText(GetSizeString(m_progressBar->GetCurStep()));
	return iOldStep;
}

bool CFileProgressBarUI::TransSpeed(double speed)
{
	char tmp[32] = { 0 };
	sprintf(tmp, "%.2fKB/S", speed);	
#ifdef UNICODE 
	TCHAR tcTmp[32] = { 0 };
	CStringConversion::StringToWideChar(tmp, tcTmp, 32);
	m_labelTransSpeed->SetText(tcTmp);
#else
	m_labelTransSpeed->SetText(tmp);
#endif
	return true;
}

bool CFileProgressBarUI::EnableDisplaySpeed(bool bEnable)
{
	bool bPre = m_labelTransSpeed->IsVisible();
	m_labelTransSpeed->SetVisible(bEnable);
	return bPre;
}

CProgressBarUI& CFileProgressBarUI::ProgressBar() const
{
	return *m_progressBar;
}

#pragma warning(default:4996)