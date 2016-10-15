#ifndef __COLORTRANSLATE_H___
#define __COLORTRANSLATE_H___

#include <commonlib/types.h>

#define COLORS_ITEM_COUNT 4096

typedef struct CColorFreqItem
{
	int nColor;
	int nTimes;
}COLOR_FREQ_ITEM, *LPCOLOR_FREQ_ITEM;

#pragma pack(push)
#pragma pack(1)
//
typedef struct CRGBColorItem
{
	BYTE b;
	BYTE g;
	BYTE r;
	BYTE f;
}RGB_COLOR_ITEM, *LPRGB_COLOR_ITEM;

#pragma pack(pop)

class COMMONLIB_API CColorTranslate
{
public:
	CColorTranslate(void);
	~CColorTranslate(void);
public:
	//��ʼ��һ����ɫ��
	void  InitTranslate(const char *pImageData, const WORD wSrcWidth, const WORD wSrcHeight, const WORD wByteCount);
	//ת����256��ɫ
	BOOL  Translate256(const char *pImageData, const WORD wSrcWidth, const WORD wSrcHeight, const WORD wByteCount,
		               const RECT *prc, char *pOutImage, DWORD &dwOutSize);
	//ת����ԭ����ɫ
	static BOOL TranslateClipColor(const char *pImageData, const WORD wDstWidth, const WORD wDstHeight, const WORD wByteCount, 
	               char *pOutImageData, int *pClrMap);
	//����һ��pal
	static HPALETTE CreatePaletteFromTable(LPCOLOR_FREQ_ITEM pColors); 

	//��ȡ��ɫMAP��
    BOOL GetColorMap(RGBQUAD *pClrMap, DWORD &dwSize);
private:
	//ͳ��ͼ��ʹ��Ƶ��
    BOOL StatistColorFreq(const char *pImageData, const WORD wWidth, const WORD wHeight, const WORD wByteCount);
	void ClearNoUserColors(); //ȥ����ʹ�õ���ɫ
	void QuickSort(int nLow, int nHigh);
	void Sort();
	//������ɫ
	void AdjustColor();
	//��ȡϵͳ��ɫ��
	void GetSystemColorTables();
private:
	COLOR_FREQ_ITEM m_FreqItems[COLORS_ITEM_COUNT]; //Ϊ��¼��ɫʹ��Ƶ�ʵ�����
	BYTE m_byteClrTable[COLORS_ITEM_COUNT]; //Ϊ��¼��ɫ����ֵ������ 
	RGBQUAD m_clrMap[256]; //
	DWORD m_dwClrNumber; //�ܵ���ɫ��
};

#endif
