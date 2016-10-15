#if !defined(AFX_UIBUTTON_H__20060218_72F5_1B46_6300_0080AD509054__INCLUDED_)
#define AFX_UIBUTTON_H__20060218_72F5_1B46_6300_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <CommonLib/SystemUtils.h>
#include <CommonLib/GdiPlusImage.h>
/////////////////////////////////////////////////////////////////////////////////////
//

class  CButtonUI : public CControlUI
{
public:
	CButtonUI();

	LPCTSTR GetClass() const;
	UINT GetControlFlags() const;

	bool Activate();
	void SetText(LPCTSTR pstrText);
	void Event(TEventUI& event);
	SIZE EstimateSize(SIZE szAvailable);
	void DoPaint(HDC hDC, const RECT& rcPaint);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	BOOL GetAttribute(LPCTSTR pstrName, TCHAR *szValue, int &nMaxValueSize); 
	void SetPadding(int cx, int cy);
	SIZE GetPadding() const { return m_szPadding; }
	void SetTextStyle( UINT uStyle );
	UINT GetTextStyle() const;
	UINT SetButtonState( UINT );
	UINT GetButtonState() const;

protected:
	virtual void OnEventButtonDown(TEventUI& e);
	virtual void OnEventDblclk(TEventUI& e);
	virtual void OnEventButtonUp(TEventUI& e);
	virtual void OnEventMouseMove(TEventUI& e);
	virtual void OnEventMouseEnter();
	virtual void OnEventMouseLeave();
protected:
	BOOL m_bDown;	
	int m_iBkgImageId;  
	SIZE m_szPadding;
	UINT m_uTextStyle;
	UINT m_uButtonState;
};

/////////////////////////////////////////////////////////////////////////////////////
//CImageButtonUI

class CImageButtonUI : public CButtonUI
{
public:
	CImageButtonUI();
    ~CImageButtonUI();
	LPCTSTR GetClass() const;

	SIZE EstimateSize(SIZE szAvailable);
    void DoPaint(HDC hDC, const RECT& rcPaint);
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	virtual void SetManager(CPaintManagerUI* pManager, CControlUI* pParent);
	BOOL SetImage( UINT );//button image
	BOOL SetImage(const char* imageFile, UINT uImage_Type );
	UINT GetImage() const { return m_uImageId; }
	BOOL SetFixedCorner( UINT );//����ʱí����corner�ߴ�,4��corner������ͬ�ĳߴ�
	UINT GetFixedCorner() const { return m_nStretchFixed; }
	BOOL SetStretchMode( UINT );
	UINT GetStretchMode() const { return m_nStretchMode; }
    void SetGray(BOOL bIsGray);
	BOOL SetFloatImage(const char *szFileName, BOOL bIsTransParent = FALSE);//����buttonͼƬ�ϵĸ���ͼƬ
	BOOL SetFloatImage(int nImageId);
	BOOL SetFloatImageShrink( const RECT& rc );//����ͼƬ�����ĳߴ�
protected:
	UINT m_uImageId;
	BOOL m_bTransparent;
private: 
	UINT m_ImageCx;
	UINT m_ImageCy;
	UINT m_nFloatImageId;
	UINT m_nStretchMode;
	UINT m_nStretchFixed;//�����í���ߴ磬����SM_FIXED4CORNERS������4���ǳߴ���ͬ
							//����SM_HORIZONTAL��������������í������ͬ����������
	IImageInterface *m_buttonGraph;//��ťͼƬ��ֻ����m_nImgID��m_pButtonGraphһ������
	IImageInterface  *m_pFloatImg; //����ͼƬ
	RECT m_rcFloatImageShrink;//button֧�����渡��һ��ͼƬ���������Ǹ���
							//ͼƬ4���������ľ���
	BOOL m_bIsGray;
	enum
	{
		DRAW_TEXT_NONE,
		DRAW_TEXT_VERTICAL, //��ֱ����
		DRAW_TEXT_HORIZONTAL //ˮƽ���� 
	}m_DrawStyle;
};

class CPlugImageButtonUI: public CButtonUI
{
public:
	CPlugImageButtonUI();
public:
	//
	SIZE EstimateSize(SIZE szAvailable);
    void DoPaint(HDC hDC, const RECT& rcPaint);
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
private:
	UINT m_nImageId;
	UINT m_nNormalImgId;
	UINT m_nHotImgId;
	UINT m_nPushedImgId;
	UINT m_nGrayImgId;
	int m_ImageCx;
	int m_ImageCy;
};

/////////////////////////////////////////////////////////////////////////////////////
//

class COptionUI : public CButtonUI
{
public:
	COptionUI();

	void SetImage( UINT nImageID );
	bool IsChecked() const;
	virtual void SetCheck(bool bSelected);

	SIZE EstimateSize(SIZE szAvailable);
	void DoPaint(HDC hDC, const RECT& rcPaint);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
   
	//operation
	void SetData(const int iData );
	int GetData() const;
public:
	bool m_bSelected;
private:
	int m_iImgID;
	int m_iData;
	
};

class CCheckBoxUI : public COptionUI
{
public:
	CCheckBoxUI();
	~CCheckBoxUI();

	LPCTSTR GetClass() const;
	bool Activate();
};

class CControlGroup;
class CRadioBoxUI : public COptionUI
{
public:
	CRadioBoxUI();
	~CRadioBoxUI();

	LPCTSTR GetClass() const;
	bool Activate();
	void SetCheck(bool bSelected);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	void Init();

	int GetGroupID() const;
	bool SetGroupID( int iGroup );

protected:
	void OnEventButtonDown(TEventUI& e);
	void OnEventButtonUp(TEventUI& e);

private:
	int m_iGroupID;//���������id
	int m_iIndex;//�����е�����
	CControlGroup* m_pOwner;//��
};
#endif // !defined(AFX_UIBUTTON_H__20060218_72F5_1B46_6300_0080AD509054__INCLUDED_)

