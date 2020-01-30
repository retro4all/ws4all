#ifndef MINIMAL_IMAGE_ADDON
#define MINIMAL_IMAGE_ADDON

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "minimal.h"

#include <string.h>
#include <stdarg.h>

#define PNG_DEBUG 3
#include "png.h"
#define SCREENWIDTH 320
#define TRANSPARENTCOLOUR 0

// Macro for 16 bit 565 colour from RGB
// #define gp2x_video_RGB_color16(R,G,B)    ((((R)&0xF8)<<8)|(((G)&(0xFC)<<2)|(((B)&0xF8)>>3)))

/*
 * png loading function from code by:
 * Guillaume Cottenceau (gc at mandrakesoft.com)
 *
 * Copyright 2002 MandrakeSoft
 *
 * This software may be freely redistributed under the terms of the GNU
 * public license.
 *
 */

typedef struct tagBitmapInfo
{
  unsigned short int infosize;
  unsigned short int width;
  unsigned short int width2;
  unsigned short int height;
  unsigned short int height2;
  unsigned short int colourPlanes; //must be 1
  unsigned short int bpp; //specifies the number of bit per pixel
  unsigned short int compression; //specifies the number of bit per pixel
  unsigned short int compression2; // compression type
  unsigned short int imageSize;  //size of image in bytes
  unsigned short int imageSize2;  //size of image in bytes
  unsigned short int xmetric;  //number of pixels per meter in x axis
  unsigned short int xmetric2;  //number of pixels per meter in x axis
  unsigned short int ymetric;  //number of pixels per meter in y axis
  unsigned short int ymetric2;  //number of pixels per meter in y axis
  unsigned short int coloursUsed;  //number of colors used by the bitmap
  unsigned short int coloursUsed2;  //number of colors used by the bitmap
  unsigned short int importantColours;  //number of colors that are important... whatever.
  unsigned short int importantColours2;  //number of colors that are important... whatever.
}BitmapInfo;

typedef struct tagBitmapHeader
{
  unsigned short int filetype;  //specifies the file type
  unsigned short int fileSize;  //  Filesize is actually fileSize + (fileSize2 << 16)
  unsigned short int fileSize2;
  unsigned short int Whatever;  // 0
  unsigned short int andEverAmen;  // 0
  unsigned short int offsetBytes;  //species the offset in bytes from the header to the bitmap
  unsigned short int morebollox;  //More bollox
}BitmapHeader;

extern int gp2x_loadPNG(char filename[], gp2x_rect *gp2ximage, int bitdepth, int solid);
extern int gp2x_loadBMP(char filename[], gp2x_rect *gp2ximage, int bitdepth, int solid, unsigned char *palette);
extern void gp2x_blitanimrect(gp2x_rect *blit, int framex, int framey, int frameh, int framew);
extern void gp2x_blitwholerect15(gp2x_rect *r);
extern void gp2x_spritemapblit(gp2x_rect *blit, int x, int y, int cols, int rows, int frame);
extern unsigned short** gp2x_createspritemap(gp2x_rect *map, int cols, int rows);

#endif     //MINIMAL_IMAGE_ADDON
