#pragma once

#include <UILib/UIContainer.h>
#include <UILib/UIPanel.h> 
/////////////////////////////////////////////////////////////////////////////
//
//
class CProgressBarUI : public CControlUI
{
public:
	CProgressBarUI();
	~CProgressBarUI();

	LPCTSTR GetClass() const;
	UINT GetControlFlags() const;

	SIZE EstimateSize(SIZE szAvailable);
	void DoPaint(HDC hDC, const RECT& rcPaint);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

	//operation
	int SetSize( const SIZE& szNew );//control's size
	SIZE GetSize() const;
	int GetExtension() const {
		return m_iMax;
	}
	int GetCurStep() const{
		return m_iCurPos;
	}
	int SetExtension( int iExt );//base from 0
	int SetPosition(int iPos);
	int Progress( int iStep );
	void SetBkgndImage( UINT nImageID );
	void SetProgressImage( UINT nProgressImage );
private:
	SIZE m_szFixed;
	int m_iMax;
	int m_iCurPos;

	int m_iBkgndImageID;//背景图片
	int m_iStepImageID;//
};


/////////////////////////////////////////////////////////////////////////////
//
//
class CLabelPanelUI;
class CFileProgressBarUI : public CVerticalLayoutUI
{
public:
	CFileProgressBarUI();

	LPCTSTR GetClass() const;
	UINT GetControlFlags() const;

	void DoPaint(HDC hDC, const RECT& rcPaint);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	SIZE EstimateSize( SIZE szAvail );
	void SetBorderColor( COLORREF clrBorder );
	//operations
	//nFileSize : nFileSize bytes
	void SetFileInfo( const CStdString& sFileName, UINT nFileSize );
	//iStep当前文件已经传输的bytes
	int Progress( int iStep );
	//speed kb/s
	bool TransSpeed( double speed );
	bool EnableDisplaySpeed( bool bEnable );
	CProgressBarUI& ProgressBar() const;

private:
	CStdString GetSizeString(UINT nSize);
private:
	BOOL m_bShowFileIcon;
	CNormalImagePanelUI *m_pImage;
	CLabelPanelUI *m_labelFileName;
	CProgressBarUI *m_progressBar;
	CLabelPanelUI *m_labelTransSpeed;
	CLabelPanelUI *m_labelCurrentSize;
	CLabelPanelUI *m_labelSlash;
	CLabelPanelUI *m_labelFileSize;
	CTextPanelUI  *m_pCancel;
	CTextPanelUI  *m_pSaveAs;
	CTextPanelUI  *m_pSendOffline;
	CTextPanelUI  *m_pRecv;
};