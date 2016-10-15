#if !defined(AFX_BLUEUI_H__20050424_6E62_6B42_BC3F_0080AD509054__INCLUDED_)
#define AFX_BLUEUI_H__20050424_6E62_6B42_BC3F_0080AD509054__INCLUDED_

#pragma once

#include <UILib/UIResource.h>
/////////////////////////////////////////////////////////////////////////////////////
//
typedef BOOL (WINAPI *PGradientFill)(HDC, PTRIVERTEX, ULONG, PVOID, ULONG, ULONG);

class  CRenderClip
{
public:
   ~CRenderClip();
   RECT rcItem;
   HDC hDC;
   HRGN hRgn;
   HRGN hOldRgn;
};

//图片拉伸时铆钉的部分
struct StretchFixed
{
public:
	StretchFixed( UINT nFixed = 0 );
	void SetFixed( UINT );//将所有的铆钉值设置为同一个
	UINT m_iTopLeftWidth;
	UINT m_iTopHeight;
	UINT m_iTopRightWidth;
	UINT m_iBotLeftWidth;
	UINT m_iCenterHeight;
	UINT m_iBotHeight;
	UINT m_iBotRightWidth;
	UINT m_iCenterLeftWidth;
	UINT m_iCenterRightWidth;
	COLORREF m_crTexture1;
	COLORREF m_crTexture2;
	int m_nTextureWidth;
	int m_nTextureImage;
};

enum StretchMode{
	SM_FIST_ = 0,
	SM_NORMALSTRETCH = 1,//普通拉伸，一次拉伸到目标dc的rect中
	SM_FIXED4CORNERS,//固定4个角，其余部分拉伸
	SM_HORIZONTAL,//固定左右两边，中间拉伸
	SM_VERTICAL,//固定上下两边，中间拉伸
	SM_NOSTRETCH,//不拉伸，保持图像尺寸
	SM_FIX4CVER,  //固定4角，并保持顶部
	SM_HORIZONTAL_CENTER,//固定左右两边，中间拉伸
	SM_VERTICAL_CENTER,//固定上下两边，中间拉伸
	SM_LAST_,
};

enum PAINT_ALIGN
{
	PA_TOPLEFT,     //左上
	PA_TOPRIGHT,    //右上
	PA_BOTTOMLEFT,  //左下
	PA_BOTTOMRIGHT, //右下
};

class  CBlueRenderEngineUI
{
public:
	static RGBQUAD RGBtoHSL(RGBQUAD lRGBColor);

    static RGBQUAD HSLtoRGB(RGBQUAD lHSLColor);
	//
	static COLORREF AdjustColor(COLORREF clr, const short H, const short S, const short L);
	//根据rcItem产生可绘制区域，并将其选入hDC
	static void GenerateClip(HDC hDC, RECT rcItem, CRenderClip& clip);
	//绘制线
	static void DoPaintLine(HDC hDC, CPaintManagerUI* pManager, RECT rc, UITYPE_COLOR Color);
	//绘制线
	static void DoPaintLine(HDC, const RECT& rc,  COLORREF clr);
	//
	static void DoPaintWidthLine(HDC, const RECT &rc, COLORREF clr, int nWidth, BOOL bTransparent);
	//draw texture
	static void DoPaintTexture(HDC hdc, const RECT &rc, COLORREF crTexture1, COLORREF crTexture2, int nTextureWidth);
	//
	static void DoPaintTexture(HDC hdc, const RECT &rc, CPaintManagerUI *pManager, int nImageId);
	//填充rc所示矩形
	static void DoFillRect(HDC hDC, CPaintManagerUI* pManager, RECT rc, UITYPE_COLOR Color, BOOL bTransparent);
	//填充rc所示矩形
	static void DoFillRect(HDC hDC, CPaintManagerUI* pManager, RECT rc, COLORREF clrFill, BOOL bTransparent);
	//绘制矩形
	static void DoPaintRectangle(HDC hDC, CPaintManagerUI* pManager, RECT rcItem, UITYPE_COLOR Border, UITYPE_COLOR Fill);
	//绘制矩形
	static void DoPaintRectangle(HDC hDC, const RECT& rc, COLORREF clrBorder, COLORREF clrFill);
	//绘制圆角矩形
	static void DoPaintRoundRect(HDC hDC, CPaintManagerUI* pManager, const RECT& rcItem,
	   UITYPE_COLOR border,  UITYPE_COLOR fill, const SIZE& corner);
	//绘制圆角矩形
	static void DoPaintRoundRect(HDC hDC, const RECT& rc, COLORREF clrBorder,
		COLORREF clrFill, const SIZE& corner);

	//绘制图片,将nImageID指定的图片拉伸到rc中.
	//如果nImageID包含多个子图片，iSubIndex指定
	//将要绘制的子图片序号，从0开始计数。
	static void DoPaintGraphic(HDC hDC,	CPaintManagerUI* pManager, const RECT& rc, UINT nImageID,
		int iSubIndex = -1,	const StretchFixed* = NULL,	UINT nStretchMode = SM_NORMALSTRETCH, BOOL bHole = FALSE);
	//绘制动态图片,双图叠加
	static void DoPaintGraphicPlus(HDC hDc, CPaintManagerUI *pManager, const RECT &rc, 
		                           UINT nImageId, UINT nPlusImageId, const StretchFixed * = NULL,
								   UINT nStretchMode = SM_NORMALSTRETCH);
	//绘制图片，将图片拉伸到rc中.
	//如果nImageID包含多个子图片，iSubIndex指定
	//将要绘制的子图片序号，从0开始计数。
	static void DoPaintGraphic(HDC hDC,	CPaintManagerUI* pManager, const RECT& rc,
		const UI_IMAGE_ITEM &image,	int iSubIndex = -1,	const StretchFixed* = NULL,
		UINT nStretchMode = SM_NORMALSTRETCH, BOOL bHole = FALSE);
	//只绘制可见部分 Align 表示靠左下 1，或者右下 2 
	static void DoPaintGraphic(HDC hDC, CPaintManagerUI *pManager, const RECT &rc,
		      UINT nImageId, PAINT_ALIGN Align);
	//绘制图片，特殊模式。该模式下源图片的四个角不被拉伸，
	//只拉伸其余部分,最终图片被拉伸到rc中。
	static void DoPaintGraphic(HDC hDC,	CPaintManagerUI* pManager, const RECT& rc, UINT nImage,
		const StretchFixed&, UINT nStretchMode = SM_NORMALSTRETCH);
	//绘制普通按钮。按钮边框、文字、效果等全部自绘。
	static void DoPaintButton(HDC hDC, CPaintManagerUI* pManager, RECT rc, LPCTSTR pstrText,
		const RECT& rcPadding, UINT uState, UINT uDrawStyle, UINT uImageId);
	//绘制双图像菜单
	static void DoPaintPlusMenuButton(HDC hDC, CPaintManagerUI *pManager, RECT rc, LPCTSTR pstrText,
		const SIZE &szPadding, UINT uState, UINT uDrawStyle, UINT uBkgImageId, UINT uImageId, UINT uSubIdx, UINT uArrowImage,
		BOOL bTransparent);
	//绘制按钮文字，不带边框
	static void DoPaintButtonText(HDC hDC, CPaintManagerUI *pManager, RECT rc, LPCTSTR pstrText,
		UINT uState, UINT uDrawStyle);
	//图片按钮.nImageID指定了按钮图片，图片应该包括4
	//个子图：按钮的正常状态、hot状态、按下状态和不可用
	//状态，且顺序如上所述。如果子图不等于4个，则效果
	//未知。
	static void DoPaintImageButton(HDC hDC, CPaintManagerUI* pManager, const RECT& rc, 
		UINT uState,  UINT nImageID, const StretchFixed* = NULL,
		UINT nStretchMode = SM_NORMALSTRETCH);
	//绘制tab页的标签按钮。和DoPaintImageButton类似，不过tab按钮的绘制
	//逻辑和ImageButton不同，而且子图的状态也不一样，选中、正常、hot和按下。
	static void DoPaintTabImageButton(HDC hDC, CPaintManagerUI* pManager, const RECT& rc,
		UINT uState, UINT nBkgImageId, UINT nImageID, const StretchFixed* pFixed = NULL,
		UINT nStretchMode = SM_NORMALSTRETCH);
	//选项框绘制，包括图片和文字（复选框、单选框）
	static void DoPaintOptionBox(HDC hDC, CPaintManagerUI* pManager, const RECT rcItem, 
		LPCTSTR pstrText, UINT uState, UINT uStyle, UINT nImageID);
	//绘制editbox边框
	static void DoPaintEditBox(HDC hDC, CPaintManagerUI* pManager, const RECT& rcItem, UINT uState);
	//绘制editbox的文本
	static void DoPaintEditText(HDC hDC, CPaintManagerUI* pManager, const RECT& rcItem, LPCTSTR pstrText, 
		COLORREF clrText, UINT uState, UINT uEditStyle);
	//绘制格式化的文本
	static int DoPaintQuickText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, 
		const CStdString& sText, UITYPE_COLOR iTextColor, UITYPE_FONT iFont, UINT uStyle);
	//绘制格式化文本
	static int DoPaintQuickText(HDC hDC, CPaintManagerUI* pManager,	RECT& rc, const CStdString& sText,
		COLORREF clrText, UITYPE_FONT iFont, UINT uStyle);
	//文本绘制
	static void DoPaintPrettyText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, LPCTSTR pstrText, 
		UITYPE_COLOR iTextColor, UITYPE_COLOR iBackColor, RECT* pLinks, int& nLinkRects, UINT uStyle);
	//文本绘制
	static void DoPaintPrettyText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, 
		LPCTSTR pstrText, COLORREF clrText, COLORREF clrBack, RECT* pLinks, 
		int& nLinkRects, UINT uStyle);
	//绘制树视图中节点图标（标示该节点展开或者缩起的图标）
	static void DoPaintGroupNode(HDC hDC, CPaintManagerUI* pManager, const RECT& rc,
		UINT nNodeType,	UINT nImageID,	BOOL bExpanded);
	//绘制树视图中节点图标（标示该节点展开或者缩起的图标）
	static void DoPaintGroupNode(HDC hDC, CPaintManagerUI* pManager, const RECT& rc,
		UINT nNodeType,	const LPUI_IMAGE_ITEM Item,	BOOL bExpanded);
	//绘制树视图中节点选取状态图标(0-未选 1-选择 2-灰选)
	static void DoPaintCheckStatus(HDC hDC,	CPaintManagerUI* pManager,	const RECT &rc, UINT nImageID,	int  nStatus);
	//属性页的tab区域，包括tab按钮，tab文字等
	static RECT DoPaintTabFolder(HDC hDC, CPaintManagerUI* pManager, const RECT& rc, 
		const LPCTSTR pstrText, UINT uState, UINT uTextStyle, UINT uTabAlign);
	//绘制控件的frame，目前不支持Dark颜色，而是四条边同色的普通border
	static void DoPaintFrame(HDC hDC, CPaintManagerUI* pManager, RECT rc, UITYPE_COLOR Light, UITYPE_COLOR Dark, 
		UITYPE_COLOR Background = UICOLOR__INVALID, UINT uStyle = 0);
 
    static void DoPaintPanel(HDC hDC,  CPaintManagerUI* pManager,  RECT rc);

	static void DoPaintToolbarButton(HDC hDC, CPaintManagerUI *pManager, RECT rc, LPCTSTR pstrText, 
		SIZE szPadding, UINT uState);
   
    static void DoPaintGradient(HDC hDC, CPaintManagerUI* pManager, RECT rc, COLORREF clrFirst, 
		COLORREF clrSecond, bool bVertical, int nSteps);
	static void DoPaintGradient(HDC hDC, CPaintManagerUI *pManager, POINT ptTriangle1, POINT ptTriangle2, 
		POINT ptTriangle3,  COLORREF crC1, COLORREF crC2, COLORREF crC3);
	static void DoPaintAlphaBitmap(HDC hDC, CPaintManagerUI* pManager, HBITMAP hBitmap, 
		RECT rc, BYTE iAlpha);
	static void DoShiftBitmap(HDC hDC, CPaintManagerUI *pManager, int r, int g, int b, RECT rc);
	static void GrayFrame(HDC hdc, int nLeft, int nTop, int nRight, int nBottom); //灰化区域
	static void DrawColor(HDC hDC, const RECT& rc, DWORD color);
	static void DoAnimateWindow(HWND hWnd, UINT uStyle, DWORD dwTime = 200);
	static HBITMAP GenerateAlphaBitmap(CPaintManagerUI* pManager, CControlUI* pControl, RECT rc, 
		UITYPE_COLOR Background);
private:
	static void CopyAndDraw(HDC hDC, CPaintManagerUI *, UI_IMAGE_ITEM &, const RECT& rcDest, 
		const RECT& rcCopy);
	static PGradientFill m_lpGradientFill;
};


#endif // !defined(AFX_BLUEUI_H__20050424_6E62_6B42_BC3F_0080AD509054__INCLUDED_)
