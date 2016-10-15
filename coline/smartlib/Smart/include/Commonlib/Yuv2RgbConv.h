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
//    Name    : convert.h
//    Details : Conversion routine from RGB24 to YUV420 & YUV420 to RGB24.
//
/////////////////////////////////////////////////////////////////////////////

#if !defined _CONVERT_H
#define _CONVERT_H

#include<stdio.h> 
// Conversion from RGB24 to YUV420
void InitLookupTable();
int  ConvertRGB2YUV(int w,int h, const unsigned char *rgb, unsigned char *yuv);


// Conversion from YUV420 to RGB24
void InitConvertTable();
void ConvertYUV2RGB(unsigned char *srcy, int stride_y,  unsigned char *srcu,unsigned char *srcv, int stride_uv,
	                unsigned char *dst_ori, int width,int height, int stride_out);


#endif