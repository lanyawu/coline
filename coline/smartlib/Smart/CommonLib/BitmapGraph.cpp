#include <string.h>
#include <stdio.h>
#include <math.h>
#include <CommonLib/bitmapgraph.h>

#pragma warning(disable:4996)

CBitmapGraph::CBitmapGraph(void):
              m_Header(NULL),
              m_Data(NULL),
			  m_hBitmap(NULL),
              m_nDataSize(0)
{
    
}

CBitmapGraph::~CBitmapGraph(void)
{
    delete m_Header;
    delete []m_Data;
	if (m_hBitmap)
	{
		::DeleteObject(m_hBitmap);
		m_hBitmap = NULL;
	}
}

HBITMAP CBitmapGraph::GetBitmap()
{
	return m_hBitmap;
}

int CBitmapGraph::LoadFromFile(char *FileName)
{   
	if (NULL == FileName)
		return BITMAP_ERROR;
	FILE *fp = fopen(FileName, "rb");
	if (!fp) return false;
    do
	{
		unsigned int nFileSize;
		char buff[16];
		if (m_Header)
		{
			delete m_Header;
			m_Header = NULL;
		}
		if (m_Data)
		{
			delete []m_Data;
			m_Data = NULL;
		}
		if (m_hBitmap)
		{
			::DeleteObject(m_hBitmap);
			m_hBitmap = NULL;
		}
		fseek(fp, 0, SEEK_END);
		nFileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		if (fread(buff, 1, 2, fp) != 2) break; 
		if ((buff[0] != 'B') || (buff[1] != 'M') || (nFileSize < BITMAPMINSIZE)) break;
		unsigned int fSize, nOffset;
		if (fread(&fSize, 1, sizeof(int), fp) != sizeof(int)) break;
		fseek(fp, sizeof(int), SEEK_CUR);  //reserved
		if (fread(&nOffset, 1, sizeof(int), fp) != sizeof(int)) break;
        m_Header = new BITMAPINFOHEADER;
        if (fread(m_Header, 1, sizeof(BITMAPINFOHEADER), fp) != sizeof(BITMAPINFOHEADER)) break;
        if ((m_Header->biSize + 14) > nOffset)
            return BITMAP_ERROR;
        if (24 != m_Header->biBitCount)
            return BITMAP_NO_SUPPORT;
        m_nDataSize =  m_Header->biWidth * m_Header->biHeight * 3;
        m_Data = new char[m_nDataSize];
        fseek(fp, nOffset, SEEK_SET);
		if (fread(m_Data, 1, m_nDataSize, fp) != m_nDataSize) break;
		fclose(fp);
		m_hBitmap = ::CreateBitmap(m_Header->biWidth, m_Header->biHeight, m_Header->biPlanes, m_Header->biBitCount,
			m_Data);
		return BITMAP_NO_ERROR;
	} while(0);
	fclose(fp);
	return BITMAP_ERROR;

}

int CBitmapGraph::LoadFromBuff(char *lpBuff, DWORD dwSize)
{
	if ((NULL == lpBuff) || (dwSize == 0))
		return BITMAP_ERROR;
    do
	{
        DWORD nOffset = 0;
		int nBmpOffset; //Í¼ÏñµÄÆ«ÒÆÎ»ÖÃ
		char buff[16] = {0};
		if (m_Header)
		{
			delete m_Header;
			m_Header = NULL;
		}
		if (m_Data)
		{
			delete []m_Data;
			m_Data = NULL;
		}
		if (dwSize < BITMAPMINSIZE) 
			break; //ÍË³ö
 		memmove(buff, lpBuff + nOffset, 2); 
		nOffset += 2; //Æ«ÒÆ
		if ((buff[0] != 'B') || (buff[1] != 'M'))
			break;
		int nSize;
		memmove(&nSize, lpBuff + nOffset, sizeof(int));
        nOffset += sizeof(int);
		nOffset += sizeof(int); //±£Áô×Ö¶Î
		memmove(&nBmpOffset, lpBuff + nOffset, sizeof(int));
		nOffset += sizeof(int);
		m_Header = new BITMAPINFOHEADER;
        memmove(m_Header, lpBuff + nOffset, sizeof(BITMAPINFOHEADER));
        if ((m_Header->biSize + 14) > nBmpOffset)
            return BITMAP_ERROR;
        if (24 != m_Header->biBitCount)
            return BITMAP_NO_SUPPORT;
        m_nDataSize =  m_Header->biWidth * m_Header->biHeight * 3;
        
		if ((m_nDataSize + nBmpOffset) > dwSize)
			break;
		m_Data = new char[m_nDataSize];
		memmove(m_Data, lpBuff + nBmpOffset, m_nDataSize);
		m_hBitmap = ::CreateBitmap(m_Header->biWidth, m_Header->biHeight, m_Header->biPlanes, m_Header->biBitCount,
			m_Data);
		return BITMAP_NO_ERROR;
	} while(0);
	return BITMAP_ERROR;
}


void CBitmapGraph::DrawToDc(HDC dc, int x, int y, int Width, int Height)
{
    if (!IsEmpty())
    {
        ::StretchDIBits(dc, x, y, Width, Height, 0, 0, m_Header->biWidth, m_Header->biHeight, m_Data,
			            (BITMAPINFO *)m_Header, DIB_RGB_COLORS, SRCCOPY); 
    }
}

void CBitmapGraph::DrawIdxToDC(int idx, int nBmpHeight, HDC dc, int x, int y, int nWidth, int nHeight)
{
	if (!IsEmpty())
	{
		if (((idx  + 1)* nBmpHeight) <= m_Header->biHeight)
		{
			BITMAPINFO bmi = { 0 };
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = m_Header->biWidth;
			bmi.bmiHeader.biHeight = nBmpHeight;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 24;
			bmi.bmiHeader.biCompression = BI_RGB;
			bmi.bmiHeader.biSizeImage = m_Header->biWidth * nBmpHeight * 3;
			int nCount = m_Header->biHeight / nBmpHeight;
			char *p = m_Data + (nCount - idx - 1) * m_Header->biWidth * nBmpHeight * 3;
			::StretchDIBits(dc, x, y, nWidth, nHeight, 0, 0, bmi.bmiHeader.biWidth, bmi.bmiHeader.biHeight,
				p, &bmi, DIB_RGB_COLORS, SRCCOPY);
		}
	}
}

bool CBitmapGraph::IsEmpty()
{
    return (!(m_Header && m_Data));
}

bool CBitmapGraph::Relievo()
{
    if (!IsEmpty())
    {
        int nTmp;
        int i, j,  k;
        char *Temp = new char[m_nDataSize];
        for (i = m_Header->biHeight - 1; i >= 2; i --)
        {
            j = m_Header->biWidth - 1;
            while (j >= 2)
            {
                for(k = 0; k < 3; k ++)
                {
                    nTmp =  m_Data[(i * m_Header->biWidth + j) * 3 + k] - m_Data[((i -1) * m_Header->biWidth  + j -1) * 3 + k] + EFFECT_VALUE;
                    if (nTmp > 255) nTmp = 255;
                    if (nTmp < 0) nTmp = 0;
                    Temp[(i * m_Header->biWidth + j) * 3 + k] = nTmp;
                }
                j --;
            }
        }
        delete []m_Data;
        m_Data = Temp;
        return true;
    } else
        return false;
}

bool CBitmapGraph::Carve()
{
    if (!IsEmpty())
    {
        int nTmp;
        int i, j,  k;
        char *Temp = new char[m_nDataSize];
        for (i = 0; i < m_Header->biHeight - 1; i++)
        {
            j = 0;
            while (j < m_Header->biWidth - 1)
            {
                for(k = 0; k < 3; k ++)
                {
                    nTmp =  m_Data[(i * m_Header->biWidth + j) * 3 + k] - m_Data[((i +1) * m_Header->biWidth  + j + 1) * 3 + k] + EFFECT_VALUE;
                    if (nTmp > 255) nTmp = 255;
                    if (nTmp < 0) nTmp = 0;
                    Temp[(i * m_Header->biWidth + j) * 3 + k] = nTmp;
                }
                j ++;
            }
        }
        delete []m_Data;
        m_Data = Temp;
        return true;
    } else
        return false;
}

bool CBitmapGraph::Mosaic()
{
    return false;
}

int CBitmapGraph::GetWidth()
{
	if (m_Header)
		return m_Header->biWidth;
	else
		return 0;
}

int CBitmapGraph::GetHeight()
{
	if (m_Header)
		return m_Header->biHeight;
	else
		return 0;
}

void CBitmapGraph::AlphaPaste(char *SrcBuff, int nSrcWidth, int nSrcHeight, char *AlphaBuff, 
							  int x, int y, int nAlphaWidth, int nAlphaHeight, double fAlpha)
{
	if (SrcBuff && AlphaBuff)
	{
		int c1, c2;
        if  ((nSrcWidth >= (x + nAlphaWidth)) && (nSrcHeight >= (y + nAlphaHeight)))
		{
			for (int i = 0; i < nAlphaWidth; i ++)
			{
				for (int j = 0; j < nAlphaHeight; j ++)
				{
					for (int k = 0; k < 3; k ++)
					{
						c1 = SrcBuff[((nSrcHeight - (y + j)) * nSrcWidth + x + i) * 3 + k];
						c2 = AlphaBuff[((nAlphaHeight - j) * nAlphaWidth + i) * 3 + k];
						c2 = abs(c1 - c2);
						c2 = (int)(c1 * (1 - fAlpha) + c2 * fAlpha);
                        SrcBuff[((nSrcHeight - (y + j)) * nSrcWidth + x + i) * 3 + k] = MIN(0xFF, c2);
					}
				}
			}
		}
	}
}

void CBitmapGraph::Alpha(char *Buff, int nSrcWidth, int nSrcHeight, int x, int y, double fAlpha)
{
	if (!IsEmpty())
	{
		AlphaPaste(m_Data, m_Header->biWidth, m_Header->biHeight, Buff, x, y, nSrcWidth, nSrcHeight, fAlpha);
	}
}


bool CBitmapGraph::RefreshBitmap(const BITMAPINFOHEADER *Header, char *Data, unsigned int nDataSize)
{
    if (Header)
    {
        if (!m_Header)
            m_Header = new BITMAPINFOHEADER;
        memmove(m_Header, Header, sizeof(BITMAPINFOHEADER));
    }
    if (! m_Header)
        return false;
    if (m_Data && (m_nDataSize < nDataSize))
    {
        char *Temp = new char[nDataSize];
        delete [] m_Data;
        m_Data = Temp;
    }

    if (!m_Data)
        m_Data = new char[nDataSize];
    if (Data)
    {
        memmove(m_Data, Data, nDataSize);
        m_nDataSize = nDataSize;
        return true;
    }
    return false;
}

bool CBitmapGraph::SaveToFile(char *FileName)
{
    if (!IsEmpty())
    {
        FILE *fp = fopen(FileName, "wb");
        if (fp)
        {
            char *bmpFlag = "BM"; //±êÖ¾
            unsigned int fSize = 14 + sizeof(BITMAPINFOHEADER) + m_nDataSize;
            int Reserved = 0;
            unsigned int bitOffset = 14 + sizeof(BITMAPINFOHEADER);
            fwrite(bmpFlag, 1, 2, fp);
            fwrite(&fSize, 1, sizeof(unsigned int), fp);
            fwrite(&Reserved, 1, sizeof(int), fp);
            fwrite(&bitOffset, 1, sizeof(unsigned int), fp);
            fwrite(m_Header, 1, sizeof(BITMAPINFOHEADER), fp);
            fwrite(m_Data, 1, m_nDataSize, fp);
            fclose(fp);
            return true;
        }
        return false;
    }
    return false;
}

#pragma warning(default:4996)