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

//ͼƬ����ʱí���Ĳ���
struct StretchFixed
{
public:
	StretchFixed( UINT nFixed = 0 );
	void SetFixed( UINT );//�����е�í��ֵ����Ϊͬһ��
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
	SM_NORMALSTRETCH = 1,//��ͨ���죬һ�����쵽Ŀ��dc��rect��
	SM_FIXED4CORNERS,//�̶�4���ǣ����ಿ������
	SM_HORIZONTAL,//�̶��������ߣ��м�����
	SM_VERTICAL,//�̶��������ߣ��м�����
	SM_NOSTRETCH,//�����죬����ͼ��ߴ�
	SM_FIX4CVER,  //�̶�4�ǣ������ֶ���
	SM_HORIZONTAL_CENTER,//�̶��������ߣ��м�����
	SM_VERTICAL_CENTER,//�̶��������ߣ��м�����
	SM_LAST_,
};

enum PAINT_ALIGN
{
	PA_TOPLEFT,     //����
	PA_TOPRIGHT,    //����
	PA_BOTTOMLEFT,  //����
	PA_BOTTOMRIGHT, //����
};

class  CBlueRenderEngineUI
{
public:
	static RGBQUAD RGBtoHSL(RGBQUAD lRGBColor);

    static RGBQUAD HSLtoRGB(RGBQUAD lHSLColor);
	//
	static COLORREF AdjustColor(COLORREF clr, const short H, const short S, const short L);
	//����rcItem�����ɻ������򣬲�����ѡ��hDC
	static void GenerateClip(HDC hDC, RECT rcItem, CRenderClip& clip);
	//������
	static void DoPaintLine(HDC hDC, CPaintManagerUI* pManager, RECT rc, UITYPE_COLOR Color);
	//������
	static void DoPaintLine(HDC, const RECT& rc,  COLORREF clr);
	//
	static void DoPaintWidthLine(HDC, const RECT &rc, COLORREF clr, int nWidth, BOOL bTransparent);
	//draw texture
	static void DoPaintTexture(HDC hdc, const RECT &rc, COLORREF crTexture1, COLORREF crTexture2, int nTextureWidth);
	//
	static void DoPaintTexture(HDC hdc, const RECT &rc, CPaintManagerUI *pManager, int nImageId);
	//���rc��ʾ����
	static void DoFillRect(HDC hDC, CPaintManagerUI* pManager, RECT rc, UITYPE_COLOR Color, BOOL bTransparent);
	//���rc��ʾ����
	static void DoFillRect(HDC hDC, CPaintManagerUI* pManager, RECT rc, COLORREF clrFill, BOOL bTransparent);
	//���ƾ���
	static void DoPaintRectangle(HDC hDC, CPaintManagerUI* pManager, RECT rcItem, UITYPE_COLOR Border, UITYPE_COLOR Fill);
	//���ƾ���
	static void DoPaintRectangle(HDC hDC, const RECT& rc, COLORREF clrBorder, COLORREF clrFill);
	//����Բ�Ǿ���
	static void DoPaintRoundRect(HDC hDC, CPaintManagerUI* pManager, const RECT& rcItem,
	   UITYPE_COLOR border,  UITYPE_COLOR fill, const SIZE& corner);
	//����Բ�Ǿ���
	static void DoPaintRoundRect(HDC hDC, const RECT& rc, COLORREF clrBorder,
		COLORREF clrFill, const SIZE& corner);

	//����ͼƬ,��nImageIDָ����ͼƬ���쵽rc��.
	//���nImageID���������ͼƬ��iSubIndexָ��
	//��Ҫ���Ƶ���ͼƬ��ţ���0��ʼ������
	static void DoPaintGraphic(HDC hDC,	CPaintManagerUI* pManager, const RECT& rc, UINT nImageID,
		int iSubIndex = -1,	const StretchFixed* = NULL,	UINT nStretchMode = SM_NORMALSTRETCH, BOOL bHole = FALSE);
	//���ƶ�̬ͼƬ,˫ͼ����
	static void DoPaintGraphicPlus(HDC hDc, CPaintManagerUI *pManager, const RECT &rc, 
		                           UINT nImageId, UINT nPlusImageId, const StretchFixed * = NULL,
								   UINT nStretchMode = SM_NORMALSTRETCH);
	//����ͼƬ����ͼƬ���쵽rc��.
	//���nImageID���������ͼƬ��iSubIndexָ��
	//��Ҫ���Ƶ���ͼƬ��ţ���0��ʼ������
	static void DoPaintGraphic(HDC hDC,	CPaintManagerUI* pManager, const RECT& rc,
		const UI_IMAGE_ITEM &image,	int iSubIndex = -1,	const StretchFixed* = NULL,
		UINT nStretchMode = SM_NORMALSTRETCH, BOOL bHole = FALSE);
	//ֻ���ƿɼ����� Align ��ʾ������ 1���������� 2 
	static void DoPaintGraphic(HDC hDC, CPaintManagerUI *pManager, const RECT &rc,
		      UINT nImageId, PAINT_ALIGN Align);
	//����ͼƬ������ģʽ����ģʽ��ԴͼƬ���ĸ��ǲ������죬
	//ֻ�������ಿ��,����ͼƬ�����쵽rc�С�
	static void DoPaintGraphic(HDC hDC,	CPaintManagerUI* pManager, const RECT& rc, UINT nImage,
		const StretchFixed&, UINT nStretchMode = SM_NORMALSTRETCH);
	//������ͨ��ť����ť�߿����֡�Ч����ȫ���Ի档
	static void DoPaintButton(HDC hDC, CPaintManagerUI* pManager, RECT rc, LPCTSTR pstrText,
		const RECT& rcPadding, UINT uState, UINT uDrawStyle, UINT uImageId);
	//����˫ͼ��˵�
	static void DoPaintPlusMenuButton(HDC hDC, CPaintManagerUI *pManager, RECT rc, LPCTSTR pstrText,
		const SIZE &szPadding, UINT uState, UINT uDrawStyle, UINT uBkgImageId, UINT uImageId, UINT uSubIdx, UINT uArrowImage,
		BOOL bTransparent);
	//���ư�ť���֣������߿�
	static void DoPaintButtonText(HDC hDC, CPaintManagerUI *pManager, RECT rc, LPCTSTR pstrText,
		UINT uState, UINT uDrawStyle);
	//ͼƬ��ť.nImageIDָ���˰�ťͼƬ��ͼƬӦ�ð���4
	//����ͼ����ť������״̬��hot״̬������״̬�Ͳ�����
	//״̬����˳�����������������ͼ������4������Ч��
	//δ֪��
	static void DoPaintImageButton(HDC hDC, CPaintManagerUI* pManager, const RECT& rc, 
		UINT uState,  UINT nImageID, const StretchFixed* = NULL,
		UINT nStretchMode = SM_NORMALSTRETCH);
	//����tabҳ�ı�ǩ��ť����DoPaintImageButton���ƣ�����tab��ť�Ļ���
	//�߼���ImageButton��ͬ��������ͼ��״̬Ҳ��һ����ѡ�С�������hot�Ͱ��¡�
	static void DoPaintTabImageButton(HDC hDC, CPaintManagerUI* pManager, const RECT& rc,
		UINT uState, UINT nBkgImageId, UINT nImageID, const StretchFixed* pFixed = NULL,
		UINT nStretchMode = SM_NORMALSTRETCH);
	//ѡ�����ƣ�����ͼƬ�����֣���ѡ�򡢵�ѡ��
	static void DoPaintOptionBox(HDC hDC, CPaintManagerUI* pManager, const RECT rcItem, 
		LPCTSTR pstrText, UINT uState, UINT uStyle, UINT nImageID);
	//����editbox�߿�
	static void DoPaintEditBox(HDC hDC, CPaintManagerUI* pManager, const RECT& rcItem, UINT uState);
	//����editbox���ı�
	static void DoPaintEditText(HDC hDC, CPaintManagerUI* pManager, const RECT& rcItem, LPCTSTR pstrText, 
		COLORREF clrText, UINT uState, UINT uEditStyle);
	//���Ƹ�ʽ�����ı�
	static int DoPaintQuickText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, 
		const CStdString& sText, UITYPE_COLOR iTextColor, UITYPE_FONT iFont, UINT uStyle);
	//���Ƹ�ʽ���ı�
	static int DoPaintQuickText(HDC hDC, CPaintManagerUI* pManager,	RECT& rc, const CStdString& sText,
		COLORREF clrText, UITYPE_FONT iFont, UINT uStyle);
	//�ı�����
	static void DoPaintPrettyText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, LPCTSTR pstrText, 
		UITYPE_COLOR iTextColor, UITYPE_COLOR iBackColor, RECT* pLinks, int& nLinkRects, UINT uStyle);
	//�ı�����
	static void DoPaintPrettyText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, 
		LPCTSTR pstrText, COLORREF clrText, COLORREF clrBack, RECT* pLinks, 
		int& nLinkRects, UINT uStyle);
	//��������ͼ�нڵ�ͼ�꣨��ʾ�ýڵ�չ�����������ͼ�꣩
	static void DoPaintGroupNode(HDC hDC, CPaintManagerUI* pManager, const RECT& rc,
		UINT nNodeType,	UINT nImageID,	BOOL bExpanded);
	//��������ͼ�нڵ�ͼ�꣨��ʾ�ýڵ�չ�����������ͼ�꣩
	static void DoPaintGroupNode(HDC hDC, CPaintManagerUI* pManager, const RECT& rc,
		UINT nNodeType,	const LPUI_IMAGE_ITEM Item,	BOOL bExpanded);
	//��������ͼ�нڵ�ѡȡ״̬ͼ��(0-δѡ 1-ѡ�� 2-��ѡ)
	static void DoPaintCheckStatus(HDC hDC,	CPaintManagerUI* pManager,	const RECT &rc, UINT nImageID,	int  nStatus);
	//����ҳ��tab���򣬰���tab��ť��tab���ֵ�
	static RECT DoPaintTabFolder(HDC hDC, CPaintManagerUI* pManager, const RECT& rc, 
		const LPCTSTR pstrText, UINT uState, UINT uTextStyle, UINT uTabAlign);
	//���ƿؼ���frame��Ŀǰ��֧��Dark��ɫ������������ͬɫ����ͨborder
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
	static void GrayFrame(HDC hdc, int nLeft, int nTop, int nRight, int nBottom); //�һ�����
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
