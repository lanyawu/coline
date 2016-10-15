#include <winsock2.h>
#include <CommonLib/GifImage.h>

CGifImage::CGifImage(void)
{
}

CGifImage::~CGifImage(void)
{
}

BOOL CGifImage::Decode(std::ifstream &ifs)
{
	GIF_HEADER Header;
	TAG_TabColor tbcColor;

	ifs.seekg(0, std::ios_base::beg); //移动文件头
    //读取头部结构
	ifs.read((char *)&Header, sizeof(GIF_HEADER));
	if (ifs.gcount() != (int)sizeof(GIF_HEADER))
		return FALSE;
	if (strncmp(Header.szHeader, "GIF8", 4) != 0) //非法格式
		return FALSE;

	//复制到图像
	m_PicHeader.biHeight = ntohs(Header.wSrcHeight);
	m_PicHeader.biWidth = ntohs(Header.wSrcWidth);
    

	tbcColor.wColTbl = (short)(1 << ((Header.cFlds & 0x07)+1));
	tbcColor.wColRes = (short)(((Header.cFlds & 0x70) >> 4) + 1);

	// assume that the image is a truecolor-gif if
	// 1) no global color map found
	// 2) (image.w, image.h) of the 1st image != (dscgif.scrwidth, dscgif.scrheight)
	long bTrueColor = 0;


	// Global colour map?
	if (Header.cFlds & 0x80)
		ifs.read((char *)tbcColor.Paleta, tbcColor.wColTbl * sizeof(RGB_COLOR));
	else 
		bTrueColor ++;	//first chance for a truecolor gif

	long first_transparent_index = 0;

	int iImage = 0;
	m_nFrameCount = Get_Num_Frame(ifs, &tbcColor, &Header);
    if (m_nFrameCount == 1)
		bTrueColor = 0;
 

	char ch;
	bool bPreviousWasNull = true;
	int  prevdispmeth = 0;


	for (BOOL bContinue = TRUE; bContinue; )
	{
		ifs.read((char *)&ch, sizeof(char));
		if (ifs.gcount() != 1)
			break;

	}	

}

int CGifImage::Get_Num_Frame(std::ifstream &ifs, LPTAB_COLOR lpTabColor, LPGIF_HEADER lpHeader)
{
	IMAGE_DESC image;

	long lPos = ifs.tellg();
	int nFrames = 0;
	TAB_COLOR tbcTemp; 
	memmove(&tbcTemp, lpTabColor, sizeof(TAB_COLOR));

	char ch;
	bool bPrev = true;
	BOOL bContinue = TRUE;
	for(; bContinue; )
	{
		ifs.read((char *)&ch, sizeof(char));
		if (ifs.gcount() != sizeof(char))
			break;
		if (bPrev || ch == 0)
		{
			switch(ch)
			{
			case '!': //扩展标志
				DecodeExtension(ifs);
				break;
			case ',': //图像
				{
					ifs.read((char *)&image, sizeof(IMAGE_DESC));
					image.wLeft = ntohs(image.wLeft);
					image.wTop = ntohs(image.wTop);
					image.wWidth = ntohs(image.wWidth);
					image.wHeight = ntohs(image.wHeight);
					if ((lpHeader->wSrcHeight == 0) && (lpHeader->wSrcWidth == 0))
					{
						lpHeader->wSrcHeight = image.wHeight;
						lpHeader->wSrcWidth = image.wWidth;
					}
					if (((image.wLeft + image.wWidth) > lpHeader->wSrcWidth) 
						|| ((image.wTop + image.wHeight) > lpHeader->wSrcHeight))
						break;
					nFrames ++;
					if (image.bytePf & 0x80)
					{
						tbcTemp.wColTbl = (short)(1 << ((image.bytePf & 0x07) + 1));
						ifs.read((char *)tbcTemp.Paleta, sizeof(RGB_COLOR) * tbcTemp.wColTbl);
					}
					int nBadCode = 0;
					m_ibf = GIFBUFTAM + 1;
					m_interlaced = image.bytePf & 0x40;
					m_iheight = image.wHeight;
					m_istep = 8;
					m_iypos = 0;
					m_ipass = 0;

					long lpos_start = ifs.tellg();
					//Decoder(ifs, 0, image.wWidth, nBadCode);
					if (nBadCode)
						Seek_Next_Image(ifs, lpos_start);
					else
						ifs.seekg(-(m_ibfmax - m_ibf - 1), std::ios_base::cur);
					break;
				}
			case ';':  //终止符
				bContinue = false;
				break;
			default:
				bPrev = (ch == 0);
				break;
			}
		}
	}
    
	ifs.seekg(lPos, std::ios_base::beg);
	return nFrames;
}

long CGifImage::Seek_Next_Image(std::ifstream &ifs, long lPos)
{
	ifs.seekg(lPos, std::ios_base::beg);
	char ch1,ch2;
	ch1 = ch2 = 0;
	while(true)
	{
		ifs.read(&ch2, sizeof(char));
		if (ifs.gcount() != sizeof(char)) 
			break;
		if ((ch1 == 0) && (ch2 == ',')){
			ifs.seekg(-1, std::ios_base::cur);
			return ifs.tellg();
		} else {
			ch1 = ch2;
		}
	}
	return -1;
}


//lzw压缩算法相关
void CGifImage::rle_clear(struct_RLE* rle)
{
	rle->out_bits = rle->out_bits_init;
	rle->out_bump = rle->out_bump_init;
	rle->out_clear = rle->out_clear_init;
	rle->out_count = 0;
	rle->rl_table_max = 0;
	rle->just_cleared = 1;
}

////////////////////////////////////////////////////////////////////////////////
void CGifImage::rle_flush(struct_RLE* rle)
{
	if (rle->rl_count == 1){
		rle_output_plain(rle->rl_pixel,rle);
		rle->rl_count = 0;
		return;
	}
	if (rle->just_cleared){
		rle_flush_fromclear(rle->rl_count,rle);
	} else if ((rle->rl_table_max < 2) || (rle->rl_table_pixel != rle->rl_pixel)) {
		rle_flush_clearorrep(rle->rl_count,rle);
	} else {
		rle_flush_withtable(rle->rl_count,rle);
	}
	rle->rl_count = 0;
}

////////////////////////////////////////////////////////////////////////////////
void CGifImage::rle_output_plain(int c,struct_RLE* rle)
{
	rle->just_cleared = 0;
	rle_output(c,rle);
	rle->out_count++;
	if (rle->out_count >= rle->out_bump){
		rle->out_bits ++;
		rle->out_bump += 1 << (rle->out_bits - 1);
	}
	if (rle->out_count >= rle->out_clear){
		rle_output(rle->code_clear,rle);
		rle_clear(rle);
	}
}

////////////////////////////////////////////////////////////////////////////////
void CGifImage::rle_flush_fromclear(int count,struct_RLE* rle)
{
	int n;

	rle->out_clear = rle->max_ocodes;
	rle->rl_table_pixel = rle->rl_pixel;
	n = 1;
	while (count > 0){
		if (n == 1){
			rle->rl_table_max = 1;
			rle_output_plain(rle->rl_pixel,rle);
			count --;
		} else if (count >= n){
			rle->rl_table_max = n;
			rle_output_plain(rle->rl_basecode+n-2,rle);
			count -= n;
		} else if (count == 1){
			rle->rl_table_max ++;
			rle_output_plain(rle->rl_pixel,rle);
			count = 0;
		} else {
			rle->rl_table_max ++;
			rle_output_plain(rle->rl_basecode+count-2,rle);
			count = 0;
		}
		if (rle->out_count == 0) n = 1; else n ++;
	}
	rle_reset_out_clear(rle);
}

////////////////////////////////////////////////////////////////////////////////
void CGifImage::rle_reset_out_clear(struct_RLE* rle)
{
	rle->out_clear = rle->out_clear_init;
	if (rle->out_count >= rle->out_clear){
		rle_output(rle->code_clear,rle);
		rle_clear(rle);
	}
}

////////////////////////////////////////////////////////////////////////////////
void CGifImage::rle_flush_withtable(int count, struct_RLE* rle)
{
	int repmax;
	int repleft;
	int leftover;

	repmax = count / rle->rl_table_max;
	leftover = count % rle->rl_table_max;
	repleft = (leftover ? 1 : 0);
	if (rle->out_count+repmax+repleft > rle->max_ocodes){
		repmax = rle->max_ocodes - rle->out_count;
		leftover = count - (repmax * rle->rl_table_max);
		repleft = 1 + rle_compute_triangle_count(leftover,rle->max_ocodes);
	}
	if (1+rle_compute_triangle_count(count,rle->max_ocodes) < (unsigned int)(repmax+repleft)){
		rle_output(rle->code_clear,rle);
		rle_clear(rle);
		rle_flush_fromclear(count,rle);
		return;
	}
	rle->out_clear = rle->max_ocodes;
	for (;repmax>0;repmax--) rle_output_plain(rle->rl_basecode+rle->rl_table_max-2,rle);
	if (leftover){
		if (rle->just_cleared){
			rle_flush_fromclear(leftover,rle);
		} else if (leftover == 1){
			rle_output_plain(rle->rl_pixel,rle);
		} else {
			rle_output_plain(rle->rl_basecode+leftover-2,rle);
		}
	}
	rle_reset_out_clear(rle);
}

////////////////////////////////////////////////////////////////////////////////
unsigned int CGifImage::rle_compute_triangle_count(unsigned int count, unsigned int nrepcodes)
{
	unsigned int perrep;
	unsigned int cost;

	cost = 0;
	perrep = (nrepcodes * (nrepcodes+1)) / 2;
	while (count >= perrep){
		cost += nrepcodes;
		count -= perrep;
	}
	if (count > 0){
		unsigned int n;
		n = rle_isqrt(count);
		while ((n*(n+1)) >= 2*count) n --;
		while ((n*(n+1)) < 2*count) n ++;
		cost += n;
	}
	return(cost);
}

////////////////////////////////////////////////////////////////////////////////
unsigned int CGifImage::rle_isqrt(unsigned int x)
{
	unsigned int r;
	unsigned int v;

	if (x < 2) return(x);
	for (v=x,r=1;v;v>>=2,r<<=1) ;
	for( ;; )
	{
		v = ((x / r) + r) / 2;
		if ((v == r) || (v == r+1)) return(r);
		r = v;
	}
}

////////////////////////////////////////////////////////////////////////////////
void CGifImage::rle_flush_clearorrep(int count, struct_RLE* rle)
{
	int withclr;
	withclr = 1 + rle_compute_triangle_count(count,rle->max_ocodes);
	if (withclr < count) {
		rle_output(rle->code_clear,rle);
		rle_clear(rle);
		rle_flush_fromclear(count,rle);
	} else {
		for (;count>0;count--) rle_output_plain(rle->rl_pixel,rle);
	}
}

////////////////////////////////////////////////////////////////////////////////
void CGifImage::rle_write_block(struct_RLE* rle)
{
	if (m_pOutFile)
	{
		m_pOutFile->write((char *)&rle->oblen, sizeof(BYTE));
		m_pOutFile->write((char *)&rle->oblock, rle->oblen);
		rle->oblen = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////
void CGifImage::rle_block_out(unsigned char c, struct_RLE* rle)
{
	rle->oblock[rle->oblen++] = c;
	if (rle->oblen >= 255) rle_write_block(rle);
}

////////////////////////////////////////////////////////////////////////////////
void CGifImage::rle_block_flush(struct_RLE* rle)
{
	if (rle->oblen > 0) rle_write_block(rle);
}

////////////////////////////////////////////////////////////////////////////////
void CGifImage::rle_output(int val, struct_RLE* rle)
{
	rle->obuf |= val << rle->obits;
	rle->obits += rle->out_bits;
	while (rle->obits >= 8){
		rle_block_out((unsigned char)(rle->obuf&0xff),rle);
		rle->obuf >>= 8;
		rle->obits -= 8;
	}
}
////////////////////////////////////////////////////////////////////////////////
void CGifImage::rle_output_flush(struct_RLE* rle)
{
	 if (rle->obits > 0) 
		 rle_block_out((unsigned char)(rle->obuf),rle);
	 rle_block_flush(rle);
}
////////////////////////////////////////////////////////////////////////////////
void CGifImage::compressRLE( int init_bits, std::ofstream *ofs)
{
	m_Init_bits = init_bits;
	m_pOutFile = ofs;

	struct_RLE rle;
	rle.code_clear = 1 << (init_bits - 1);
	rle.code_eof = rle.code_clear + 1;
	rle.rl_basecode = rle.code_eof + 1;
	rle.out_bump_init = (1 << (init_bits - 1)) - 1;
	rle.out_clear_init = (init_bits <= 3) ? 9 : (rle.out_bump_init-1);
	rle.out_bits_init = init_bits;
	rle.max_ocodes = (1 << MAXBITSCODES) - ((1 << (rle.out_bits_init - 1)) + 3);
	rle.rl_count = 0;
	rle_clear(&rle);
	rle.obuf = 0;
	rle.obits = 0;
	rle.oblen = 0;

	rle_output(rle.code_clear,&rle);

	int c;
	for( ;; )
	{
		//c = GifNextPixel();
		if ((rle.rl_count > 0) && (c != rle.rl_pixel)) rle_flush(&rle);
		if (c == EOF) break;
		if (rle.rl_pixel == c){
			rle.rl_count++;
		} else {
			rle.rl_pixel = c;
			rle.rl_count = 1;
		}
	}
	rle_output(rle.code_eof, &rle);
	rle_output_flush(&rle);
}