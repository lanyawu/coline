#ifndef __GIFIMAGE_H__
#define __GIFIMAGE_H__

#include <fstream>
#include <CommonLib/Types.h>

/* Various error codes used by decoder */
#define OUT_OF_MEMORY     -10
#define BAD_CODE_SIZE     -20
#define READ_ERROR        -1
#define WRITE_ERROR       -2
#define OPEN_ERROR        -3
#define CREATE_ERROR      -4
#define MAX_CODES         4095
#define GIFBUFTAM         16383
#define TRANSPARENCY_CODE 0xF9

//LZW GIF Image compression
#define MAXBITSCODES    12
#define HSIZE  5003     /* 80% occupancy */
#define MAXCODE(n_bits) (((code_int) 1 << (n_bits)) - 1)
#define HashTabOf(i)    htab[i]
#define CodeTabOf(i)    codetab[i]

//GIF相关定义
#pragma pack(push)
#pragma pack(1)

typedef struct TAG_rgbColor
{
	BYTE r, g, b;
}RGB_COLOR;

typedef struct TAG_GifHeader{
	char szHeader[6];
	WORD wSrcWidth;
	WORD wSrcHeight;
	char cFlds;
	char cIndx;
	char cPxasrat;
}GIF_HEADER, *LPGIF_HEADER;

typedef struct TAG_TabColor
{
	short wColRes; //色彩资源
	short wColTbl; //全局颜色表大小
	RGB_COLOR Paleta[256]; //模板
}TAB_COLOR, *LPTAB_COLOR;

typedef struct TAG_Image_Desc //图像描述
{
	WORD wLeft;
	WORD wTop;
	WORD wWidth;
	WORD wHeight;
	BYTE bytePf;
}IMAGE_DESC, *LPIMAGE_DESC; 

typedef struct TAG_GifInfo
{
	BYTE byteFlag;  /*res:3|dispmeth:3|userinputflag:1|transpcolflag:1*/
	WORD wDelayTime;
	BYTE byteTransIndex; //透明色
}GIF_INFO;


typedef struct tag_RLE{
	int rl_pixel;
	int rl_basecode;
	int rl_count;
	int rl_table_pixel;
	int rl_table_max;
	int just_cleared;
	int out_bits;
	int out_bits_init;
	int out_count;
	int out_bump;
	int out_bump_init;
	int out_clear;
	int out_clear_init;
	int max_ocodes;
	int code_clear;
	int code_eof;
	unsigned int obuf;
	int obits;
	unsigned char oblock[256];
	int oblen;
} struct_RLE;

#pragma pack(pop)

//GIF编解码类
class COMMONLIB_API CGifImage
{
public:
	CGifImage(void);
	~CGifImage(void);
private:
	BOOL Decode(std::ifstream &ifs);
	BOOL DecodeExtension(std::ifstream &ifs);
	long Seek_Next_Image(std::ifstream &ifs, long lPos);
	int  Get_Num_Frame(std::ifstream &ifs, LPTAB_COLOR lpTabColor, LPGIF_HEADER lpHeader);

	//lzw压缩算法
	void compressRLE( int init_bits, std::ofstream *ofs);
	void rle_clear(struct_RLE* rle);
	void rle_flush(struct_RLE* rle);
	void rle_flush_withtable(int count, struct_RLE* rle);
	void rle_flush_clearorrep(int count, struct_RLE* rle);
	void rle_flush_fromclear(int count,struct_RLE* rle);
	void rle_output_plain(int c,struct_RLE* rle);
	void rle_reset_out_clear(struct_RLE* rle);
	unsigned int rle_compute_triangle_count(unsigned int count, unsigned int nrepcodes);
	unsigned int rle_isqrt(unsigned int x);
	void rle_write_block(struct_RLE* rle);
	void rle_block_out(unsigned char c, struct_RLE* rle);
	void rle_block_flush(struct_RLE* rle);
	void rle_output(int val, struct_RLE* rle);
	void rle_output_flush(struct_RLE* rle);
private:
	//图像私有变量
	int m_nFrameCount; //图像有多少帧
	BITMAPINFOHEADER m_PicHeader;

	//
	int m_ibf;
	int m_interlaced;
	int m_iheight;
	int m_istep;
	int m_iypos;
	int m_ipass;
	int m_ibfmax;

	//lzw 压缩算法相关变量
	std::ofstream *m_pOutFile;
	int m_Init_bits;
};

#endif