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
	BOOL SetFixedCorner( UINT );//拉伸时铆钉的corner尺寸,4个corner享有相同的尺寸
	UINT GetFixedCorner() const { return m_nStretchFixed; }
	BOOL SetStretchMode( UINT );
	UINT GetStretchMode() const { return m_nStretchMode; }
    void SetGray(BOOL bIsGray);
	BOOL SetFloatImage(const char *szFileName, BOOL bIsTransParent = FALSE);//设置button图片上的浮动图片
	BOOL SetFloatImage(int nImageId);
	BOOL SetFloatImageShrink( const RECT& rc );//浮动图片缩进的尺寸
protected:
	UINT m_uImageId;
	BOOL m_bTransparent;
private: 
	UINT m_ImageCx;
	UINT m_ImageCy;
	UINT m_nFloatImageId;
	UINT m_nStretchMode;
	UINT m_nStretchFixed;//拉伸的铆钉尺寸，对于SM_FIXED4CORNERS来讲，4个角尺寸相同
							//对于SM_HORIZONTAL来讲，左右两边铆钉数相同，其余类似
	IImageInterface *m_buttonGraph;//按钮图片，只按照m_nImgID或m_pButtonGraph一个绘制
	IImageInterface  *m_pFloatImg; //浮动图片
	RECT m_rcFloatImageShrink;//button支持上面浮动一张图片，此属性是浮动
							//图片4周向内缩的距离
	BOOL m_bIsGray;
	enum
	{
		DRAW_TEXT_NONE,
		DRAW_TEXT_VERTICAL, //垂直方向
		DRAW_TEXT_HORIZONTAL //水平方向 
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
	int m_iGroupID;//所在组的组id
	int m_iIndex;//在组中的索引
	CControlGroup* m_pOwner;//组
};
#endif // !defined(AFX_UIBUTTON_H__20060218_72F5_1B46_6300_0080AD509054__INCLUDED_)

