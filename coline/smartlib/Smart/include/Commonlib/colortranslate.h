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
	//初始化一个调色板
	void  InitTranslate(const char *pImageData, const WORD wSrcWidth, const WORD wSrcHeight, const WORD wByteCount);
	//转换成256颜色
	BOOL  Translate256(const char *pImageData, const WORD wSrcWidth, const WORD wSrcHeight, const WORD wByteCount,
		               const RECT *prc, char *pOutImage, DWORD &dwOutSize);
	//转换成原来颜色
	static BOOL TranslateClipColor(const char *pImageData, const WORD wDstWidth, const WORD wDstHeight, const WORD wByteCount, 
	               char *pOutImageData, int *pClrMap);
	//创建一个pal
	static HPALETTE CreatePaletteFromTable(LPCOLOR_FREQ_ITEM pColors); 

	//获取颜色MAP表
    BOOL GetColorMap(RGBQUAD *pClrMap, DWORD &dwSize);
private:
	//统计图像使用频率
    BOOL StatistColorFreq(const char *pImageData, const WORD wWidth, const WORD wHeight, const WORD wByteCount);
	void ClearNoUserColors(); //去除不使用的颜色
	void QuickSort(int nLow, int nHigh);
	void Sort();
	//调整颜色
	void AdjustColor();
	//获取系统颜色表
	void GetSystemColorTables();
private:
	COLOR_FREQ_ITEM m_FreqItems[COLORS_ITEM_COUNT]; //为记录颜色使用频率的数组
	BYTE m_byteClrTable[COLORS_ITEM_COUNT]; //为记录颜色索引值的数组 
	RGBQUAD m_clrMap[256]; //
	DWORD m_dwClrNumber; //总的颜色数
};

#endif
