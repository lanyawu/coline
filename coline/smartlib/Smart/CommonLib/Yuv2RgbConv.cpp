////////////////////////////////////////////////////////////////////////////
//
//
//    Project     : VideoNet version 1.1.
//    Description : Peer to Peer Video Conferencing over the LAN.
//	  Author      :	Nagareshwar Y Talekar ( nsry2002@yahoo.co.in)
//    Date        : 15-6-2004.
//
//
//    File description : 
//    Name    : convert.cpp
//    Details : Conversion routine from RGB24 to YUV420 & YUV420 to RGB24.
//
/////////////////////////////////////////////////////////////////////////////


#include <Commonlib/Yuv2RgbConv.h>
 
inline int max(int x, int y)
{
	if (x > y)
		return x;
	else
		return y;
}

inline int min(int x, int y)
{
	if (x < y)
		return x;
	else
		return y;
}

//
// Table used for RGB to YUV420 conversion
//
 

//
//  Convert from  RGB24 to YUV420
//
int ConvertRGB2YUV(int w,int h, const unsigned char *rgb,unsigned char *yuv)
{

int i, j;
unsigned char*bufY, *bufU, *bufV;
const unsigned char *bufRGB;
bufY = yuv;
bufV = yuv + w * h;
bufU = bufV + (w * h* 1/4);
 
unsigned char y, u, v, r, g, b;
    unsigned int ylen = w * h;
unsigned int ulen = (w * h)/4;
unsigned int vlen = (w * h)/4;
for (j = 0; j<h;j++)
{
   bufRGB = rgb + w * (h - 1 - j) * 3 ;
   for (i = 0;i<w;i++)
   {
    int pos = w * i + j;
    r = *(bufRGB++);
    g = *(bufRGB++);
    b = *(bufRGB++);

   y = (unsigned char)( ( 66 * r + 129 * g + 25 * b + 128) >> 8) + 16 ;          
     u = (unsigned char)( ( -38 * r - 74 * g + 112 * b + 128) >> 8) + 128 ;          
        v = (unsigned char)( ( 112 * r - 94 * g - 18 * b + 128) >> 8) + 128 ;
   *(bufY++) = max( 0, min(y, 255 ));
   
    if (j%2==0&&i%2 ==0)
    {
     if (u>255)
     {
      u=255;
     }
     if (u<0)
     {
      u = 0;
     }
     *(bufU++) =u;
//存u分量
    }
    else
    {
     //存v分量
     if (i%2==0)
     {
      if (v>255)
      {
       v = 255;
      }
      if (v<0)
      {
       v = 0;
      }
      *(bufV++) =v;
    
     }
    }

   }

}
 
return true;
}







 

static unsigned __int64 mmw_mult_Y     = 0x2568256825682568;
static unsigned __int64 mmw_mult_U_G   = 0xf36ef36ef36ef36e;
static unsigned __int64 mmw_mult_U_B   = 0x40cf40cf40cf40cf;
static unsigned __int64 mmw_mult_V_R   = 0x3343334333433343;
static unsigned __int64 mmw_mult_V_G   = 0xe5e2e5e2e5e2e5e2;

static unsigned __int64 mmb_0x10       = 0x1010101010101010;
static unsigned __int64 mmw_0x0080     = 0x0080008000800080;
static unsigned __int64 mmw_0x00ff     = 0x00ff00ff00ff00ff;
static unsigned __int64 mmw_cut_red    = 0x7c007c007c007c00;
static unsigned __int64 mmw_cut_green = 0x03e003e003e003e0;
static unsigned __int64 mmw_cut_blue   = 0x001f001f001f001f;
#define MAXIMUM_Y_WIDTH  800

//
//  Convert from YUV420 to RGB24
//
void ConvertYUV2RGB(unsigned char *srcy, int stride_y,  unsigned char *srcu,unsigned char *srcv, int stride_uv,
	                unsigned char *dst_ori, int width,int height, int stride_out)
{
	int y, horiz_count;
	unsigned char *puc_out_remembered;
	//int stride_out = width_y * 3;
    if (height > 0) 
	{
		//we are flipping our output upside-down 
		srcy      += (height    - 1) * stride_y ;
		srcu      += (height/2 - 1) * stride_uv;
		srcv      += (height/2 - 1) * stride_uv;
		stride_y   = -stride_y;
		stride_uv = -stride_uv;
	} else
		height = -height;
	horiz_count = -(width >> 3);
	for (y=0; y<height; y++)
	{
		if (y == height-1) 
		{
			//this is the last output line - we need to be careful not to overrun the end of this line
			unsigned char temp_buff[3* MAXIMUM_Y_WIDTH + 1];
			puc_out_remembered = dst_ori;
			dst_ori = temp_buff; //write the RGB to a temporary store
		}
		_asm 
		{
		    push eax
		    push ebx
		    push ecx
		    push edx
		    push edi
		    mov eax, dst_ori       
		    mov ebx, srcy       
		    mov ecx, srcu       
		    mov edx, srcv
		    mov edi, horiz_count
		    horiz_loop:
		    movd mm2, [ecx]
		    pxor mm7, mm7
		    movd mm3, [edx]
		    punpcklbw mm2, mm7       
		    movq mm0, [ebx]          
		    punpcklbw mm3, mm7       
		    movq mm1, mmw_0x00ff     
		    psubusb mm0, mmb_0x10    
		    psubw mm2, mmw_0x0080    
		    pand mm1, mm0            
		    psubw mm3, mmw_0x0080    
		    psllw mm1, 3             
		    psrlw mm0, 8             
		    psllw mm2, 3             
		    pmulhw mm1, mmw_mult_Y   
		    psllw mm0, 3             
		    psllw mm3, 3             
		    movq mm5, mm3            
		    pmulhw mm5, mmw_mult_V_R
		    movq mm4, mm2            
		    pmulhw mm0, mmw_mult_Y   
		    movq mm7, mm1            
		    pmulhw mm2, mmw_mult_U_G
		    paddsw mm7, mm5
		    pmulhw mm3, mmw_mult_V_G
		    packuswb mm7, mm7
		    pmulhw mm4, mmw_mult_U_B
		    paddsw mm5, mm0      
		    packuswb mm5, mm5
		    paddsw mm2, mm3          
		    movq mm3, mm1            
		    movq mm6, mm1            
		    paddsw mm3, mm4
		    paddsw mm6, mm2
		    punpcklbw mm7, mm5
		    paddsw mm2, mm0
		    packuswb mm6, mm6
		    packuswb mm2, mm2
		    packuswb mm3, mm3
		    paddsw mm4, mm0
		    packuswb mm4, mm4
		    punpcklbw mm6, mm2
		    punpcklbw mm3, mm4
		    // 32-bit shuffle.
		    pxor mm0, mm0
		    movq mm1, mm6
		    punpcklbw mm1, mm0
		    movq mm0, mm3
		    punpcklbw mm0, mm7
		    movq mm2, mm0
		    punpcklbw mm0, mm1
		    punpckhbw mm2, mm1
		    // 24-bit shuffle and sav
		    movd    [eax], mm0
		    psrlq mm0, 32
		    movd   3[eax], mm0
		    movd   6[eax], mm2

		    psrlq mm2, 32            
		    movd   9[eax], mm2        
		    // 32-bit shuffle.
		    pxor mm0, mm0            
		    movq mm1, mm6            
		    punpckhbw mm1, mm0       
		    movq mm0, mm3            
		    punpckhbw mm0, mm7       
		    movq mm2, mm0            
		    punpcklbw mm0, mm1       
		    punpckhbw mm2, mm1       
		    // 24-bit shuffle and sav
		    movd 12[eax], mm0        
		    psrlq mm0, 32            
		    movd 15[eax], mm0        
		    add ebx, 8               
		    movd 18[eax], mm2        
		    psrlq mm2, 32            
		    add ecx, 4               
		    add edx, 4               
		    movd 21[eax], mm2        
		    add eax, 24              
		    inc edi
		    jne horiz_loop
		    pop edi
		    pop edx
		    pop ecx
		    pop ebx
		    pop eax
		    emms
		}
		
		if (y == height - 1)
		{
		    //last line of output - we have used the temp_buff and need to copy
		    int x = 3 * width ;                   //interation counter
		    unsigned char *ps = dst_ori;                 // source pointer (temporary line store)
		    unsigned char *pd = puc_out_remembered;      // dest pointer
		    while (x--) *(pd++) = *(ps++);           // copy the line
		}
		srcy  += stride_y;
		if (y%2) 
		{
			srcu    += stride_uv;
			srcv    += stride_uv;
		}
		dst_ori += stride_out;
	}
}
