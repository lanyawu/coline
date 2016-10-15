#include <commonlib/debuglog.h>
#include <commonlib/ColorTranslate.h>

CColorTranslate::CColorTranslate(void):
                 m_dwClrNumber(0)
{
	memset(&m_FreqItems, 0, sizeof(COLOR_FREQ_ITEM) * COLORS_ITEM_COUNT);
}

CColorTranslate::~CColorTranslate(void)
{
	//
}

// 快速排序，  将各种颜色  按使用的频率排序(Hight   --   Low)
void CColorTranslate::QuickSort(int nLow, int nHigh)
{
	int nLo, nHi, nMid;
	COLOR_FREQ_ITEM clrTemp;
	nLo = nLow;
	nHi = nHigh;
	nMid = m_FreqItems[(nLo + nHi) / 2].nTimes;
	do
	{
		while (m_FreqItems[nLo].nTimes > nMid)
			nLo ++;
		while (m_FreqItems[nHi].nTimes < nMid)
			nHi --;
		if (nLo <= nHi)
		{
			clrTemp = m_FreqItems[nLo];
			m_FreqItems[nLo] = m_FreqItems[nHi];
			m_FreqItems[nHi] = clrTemp;
			nLo ++;
			nHi --;
		}
	} while (nHi > nLo);
	if (nHi > nLow)
		QuickSort(nLow, nHi);
	if (nLo < nHigh)
		QuickSort(nLo, nHigh);
}
 
//排序
void CColorTranslate::Sort()
{
	QuickSort(0, m_dwClrNumber);
}

//统计图像使用频率
BOOL CColorTranslate::StatistColorFreq(const char *pImageData, const WORD wWidth, const WORD wHeight, const WORD wByteCount)
{
	if ((!pImageData) ||(wByteCount != 4))  //只支持32位色
		return FALSE;
	LPRGB_COLOR_ITEM pClr;
	int i, j;
	int nIdx;
	memset(&m_FreqItems, 0, sizeof(COLOR_FREQ_ITEM) * COLORS_ITEM_COUNT);
	DWORD dwBytePerLine = wWidth * wByteCount;
	for (i = 0; i < COLORS_ITEM_COUNT; i ++)
		m_FreqItems[i].nColor = i;
	for (i = 0; i < wHeight; i ++)
	{
		pClr = (LPRGB_COLOR_ITEM)(pImageData + dwBytePerLine * i);
		for (j = 0; j < wWidth; j ++)
		{
			//取 R、G、B三种颜色的前4位组成12位，共4096种颜色
			nIdx = ((pClr->r & 0xF0) << 4) + (pClr->g & 0xF0) + ((pClr->b & 0xF0) >> 4);
			m_FreqItems[nIdx].nTimes ++; //计算颜色的使用次数  
			pClr ++;
		}
	}
	return TRUE;
}
 
//创建一个pal
HPALETTE CColorTranslate::CreatePaletteFromTable(LPCOLOR_FREQ_ITEM pColors)
{
	char *p = new char[sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * 255];
	LPLOGPALETTE LogPal = (LPLOGPALETTE)(p);
	LogPal->palVersion = 0x300;
	LogPal->palNumEntries = 256;
	for (int i = 0; i <= 255; i ++)
	{
		LogPal->palPalEntry[i].peRed = ((pColors[i].nColor & 0xF00) >> 4) + 7;
		LogPal->palPalEntry[i].peGreen = (pColors[i].nColor & 0xF0) + 7;
		LogPal->palPalEntry[i].peBlue = ((pColors[i].nColor & 0xF) << 4) + 7;
		LogPal->palPalEntry[i].peFlags = 0;
	}
	HPALETTE Pal = ::CreatePalette(LogPal);
	delete []p;
	return Pal;
}

//获取系统颜色表
void CColorTranslate::GetSystemColorTables()
{
	PALETTEENTRY Pal[256] = {0};
	HDC h = ::GetDC(::GetDesktopWindow());
	if (::GetSystemPaletteEntries(h, 0, 256, Pal) == 256)
	{
		for (int i = 0; i < 256; i ++)
		{
			m_clrMap[i].rgbBlue = Pal[i].peBlue;
			m_clrMap[i].rgbGreen = Pal[i].peGreen;
			m_clrMap[i].rgbRed = Pal[i].peRed;
			m_clrMap[i].rgbReserved = Pal[i].peFlags;
			m_FreqItems[i].nColor = ((Pal[i].peRed & 0xF0) << 4);
			m_FreqItems[i].nColor += (Pal[i].peGreen & 0xF0);
			m_FreqItems[i].nColor += (Pal[i].peBlue & 0xF0) >> 4;
		}
	} else
		PRINTDEBUGLOG(dtInfo, "Get System Color Tables Failed");
	::ReleaseDC(::GetDesktopWindow(), h);
}

//去除不使用的颜色
void CColorTranslate::ClearNoUserColors()
{
	m_dwClrNumber = 0;
	for (int i = 0; i < COLORS_ITEM_COUNT; i ++)
	{
		if (m_FreqItems[i].nTimes > 0)
		{
			m_FreqItems[m_dwClrNumber].nColor = m_FreqItems[i].nColor;
			m_FreqItems[m_dwClrNumber].nTimes = m_FreqItems[i].nTimes;
			m_FreqItems[i].nTimes = 0;
			m_dwClrNumber ++;
		}
	}
	PRINTDEBUGLOG(dtInfo, "Use Color Count:%d", m_dwClrNumber);
}

//调整颜色,将不常用的颜色用常用的颜色代替 
void CColorTranslate::AdjustColor()
{
	int i, nError, nClr, j, nTemp;
	BYTE byteIdx;
	for (i = 0; i <= 255; i ++)
		m_byteClrTable[m_FreqItems[i].nColor] = i;
	for (i = 256; i < (int)m_dwClrNumber; i ++)
	{
		nError = 10000;
		byteIdx = 0;
		nClr = m_FreqItems[i].nColor;
		for (j = 0; j <= 255; j ++)
		{
			nTemp = abs(m_FreqItems[j].nColor) - nClr;
			if (nTemp < nError)
			{
				nError = nTemp;
				byteIdx = j;
			} // end if (nTemp...
		} //end for (j = 0...
		m_byteClrTable[m_FreqItems[i].nColor] = byteIdx;
	} //end for (i = 256 ...
	
	//初始化颜色表;
	for (i = 0; i < 256; i ++)
	{
		m_clrMap[i].rgbBlue = ((m_FreqItems[i].nColor & 0xF) << 4) + 7;
		m_clrMap[i].rgbGreen = (m_FreqItems[i].nColor & 0xF0) + 7;
		m_clrMap[i].rgbRed = ((m_FreqItems[i].nColor & 0xF00) >> 4) + 7;
		m_clrMap[i].rgbReserved = 0;
	}
}

  
//初始化一个调色板
void  CColorTranslate::InitTranslate(const char *pImageData, const WORD wWidth, const WORD wHeight, const WORD wByteCount)
{
	//统计颜色
	if (StatistColorFreq(pImageData, wWidth, wHeight, wByteCount))
	{
		//删除不用的颜色
		ClearNoUserColors();
		//颜色排序
		Sort();
		//调整颜色
		AdjustColor();
		//
		//GetSystemColorTables();
	}
}

//转换成256颜色
BOOL  CColorTranslate::Translate256(const char *pImageData, const WORD wSrcWidth, const WORD wSrcHeight, const WORD wByteCount,
	               const RECT *prc, char *pOutImage, DWORD &dwOutSize)
{
	if ((m_dwClrNumber == 0) || (wByteCount != 4))  //没有初始化颜色表 或者非32位色
		return FALSE; 
	DWORD dwBytePerLine = wSrcWidth * wByteCount;
	const char *pSrc = pImageData + prc->top * dwBytePerLine + wByteCount * prc->left;
	const char *pClr;
	BYTE *pDest = (BYTE *)pOutImage;
	dwOutSize = 0;
	int nIdx;
	for (LONG i = prc->top; i < prc->bottom; i ++)
	{
		pClr = pSrc;
		for (LONG j = prc->left; j < prc->right; j ++)
		{
			nIdx = (((LPRGB_COLOR_ITEM)pClr)->r & 0xF0) << 4;
			nIdx += (((LPRGB_COLOR_ITEM)pClr)->g & 0xF0);
			nIdx += ((((LPRGB_COLOR_ITEM)pClr)->b & 0xF0) >> 4);
			*pDest = m_byteClrTable[nIdx];
			pDest ++;
			dwOutSize ++;
			pClr += wByteCount;
		}
		pSrc += dwBytePerLine;
	}
	return TRUE;
}

//获取颜色MAP表
BOOL CColorTranslate::GetColorMap(RGBQUAD *pClrMap, DWORD &dwSize)
{
	if (dwSize >= sizeof(RGBQUAD) * 256)
	{
		dwSize = sizeof(RGBQUAD) * 256;
		if (m_dwClrNumber != 0) 
		{
			memmove(pClrMap, m_clrMap, dwSize);
		} else  //没有初始化颜色表
		{
			memset(pClrMap, 0, dwSize);
		}
		return TRUE;
	}
	return FALSE;
}

//转换成原来颜色
BOOL CColorTranslate::TranslateClipColor(const char *pImageData, const WORD wDstWidth, const WORD wDstHeight, const WORD wByteCount, 
	               char *pOutImageData, int *pClrMap)
{
	if ((!pClrMap) || (wByteCount != 4))
		return FALSE;
	DWORD dwBytePerLine = wDstWidth * wByteCount;
	char *pOut = pOutImageData;
	char *pClr = pOut;
	const BYTE *pSrc = (const BYTE *)pImageData;
	for (WORD i = 0; i < wDstHeight; i ++)
	{
		pClr = pOut;
		for (WORD j = 0; j < wDstWidth; j ++)
		{
			*((int *)pClr) = pClrMap[*pSrc];
			pSrc ++;
			pClr += wByteCount;
		}
		pOut += dwBytePerLine;
	}
	return TRUE;
}