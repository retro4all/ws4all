/***************************************************************************
 *            minimal_image
 *
 *  Sat Dec 31 07:50:58 2005
 *  Copyright  2005  bosteen
 *  Relies on minilib 0.B(included), libPNG, and the standard set.
 *  Compile with something like:
 *
 *  arm-linux-gcc minimal.c minimal_image.c Yourapp.c -oYourapp.gpe
 *  -lpng -lz -lm -lpthread -static -I/path/to/your/devkit/include
 *  -L/path/to/your/devkit/lib
 *
 *  It needs the four images that come with it. test1->4
 *
 *  Compile with '-O2 -fomit-frame-pointer -funroll-loops' for added speed.
 *  NB: The entire background is being software-blitted each frame to give
 *  something to do ;) Shows off the possible speed.
 *
 *  Creative Commons non-commercial, attribs licence applys
 *
 *  Email: bosteen@gmail.com
 *
 ****************************************************************************/



#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "minimal.h"
#include "minimal_image.h"
#include <string.h>
#include <stdarg.h>
#include <png.h>


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

int gp2x_loadPNG(char filename[], gp2x_rect *gp2ximage, int bitdepth, int solid)
{
  int y;
  int width, height;
  png_byte color_type;
  png_byte bit_depth;

  png_structp png_ptr;
  png_infop info_ptr;
  int number_of_passes;
  png_bytep * row_pointers;
  unsigned char header[8]; // 8 is the maximum size that can be checked

  /* open file and test for it being a png */
  FILE *fp = fopen(filename, "rb");
  if (!fp) {
    printf(" File %s could not be opened for reading\n", filename);
    return 1;
  }

  fread(header, 1, 8, fp);
  if (png_sig_cmp(header, 0, 8)) {
        printf(" File %s is not recognized as a PNG file\n", filename);
        return 1;
  }

  /* initialize stuff */
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (!png_ptr) {
    printf(" png_create_read_struct failed\n");
    return 1;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    printf(" png_create_info_struct failed\n");
    return 1;
  }

    if (setjmp(png_jmpbuf(png_ptr))) {
      printf(" Error during init_io\n");
      return 1;
    }

  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, 8);

  png_read_info(png_ptr, info_ptr);

  width = info_ptr->width;
  height = info_ptr->height;
  color_type = info_ptr->color_type;
  bit_depth = info_ptr->bit_depth;

  number_of_passes = png_set_interlace_handling(png_ptr);
  png_read_update_info(png_ptr, info_ptr);


  /* read file */
  if (setjmp(png_jmpbuf(png_ptr)))
    printf("[read_png_file] Error during read_image");

  row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
  for (y=0; y<height; y++)
    row_pointers[y] = (png_byte*) malloc(info_ptr->rowbytes);

  png_read_image(png_ptr, row_pointers);

  fclose(fp);

  // Now to load up the rect struct:
  gp2ximage->x = 0;
  gp2ximage->y = 0;
  // Use the width and height from the bitmap.      fileSize + (fileSize2 << 16)
  gp2ximage->w = width;
  gp2ximage->h = height;
  // Use TRANSPARENTCOLOUR as a transparent colour? then solid=0. I think.
  gp2ximage->solid = solid;

  int i,j;
  unsigned char R=0,G=0,B=0;
  unsigned short COLOR=0;
  switch(bitdepth) {
    case 8:
      // Convert RGBA png to 8 bit

      // Allocate 8bits (1 bytes) for each RGB triplet
      // gp2ximage->data = (unsigned char*)malloc(bitmapInfoHeader->imageSize / 3);
      // When I've worked out something useful for this, I'll add it...
      break;
    case 16:
      //convert RGBA to 16 bit 5515 format used by the minilib.
      // Allocate 16bits (2 bytes) for each RGBA quad
      gp2ximage->data = (unsigned short*)malloc(2*(width*height));

      // Run through the data and convert the BGR to 5515.
      switch(info_ptr->color_type) {
		//prueba
		case (PNG_COLOR_TYPE_PALETTE):
		  png_set_palette_to_rgb(png_ptr);
		  png_set_expand(png_ptr);
		  png_set_strip_16(png_ptr);
		  png_read_update_info(png_ptr, info_ptr);
		  for (j=0; j<height; j++) {
            //png_byte* row = row_pointers[j];
            for (i=0; i<width; i++) {
              R = *(row_pointers[j]+(i*3));
              G = *(row_pointers[j]+(i*3)+1);
              B = *(row_pointers[j]+(i*3)+2);
              COLOR = gp2x_video_RGB_color16(R,G,B);
              //png_byte* ptr = &(row[i*3]);
              ((unsigned short*)(*gp2ximage).data)[i+j*(gp2ximage->w)]=COLOR;
            }
          }
		  break;
		//prueba
		case (PNG_COLOR_TYPE_RGB):
          for (j=0; j<height; j++) {
            png_byte* row = row_pointers[j];
            for (i=0; i<width; i++) {
              png_byte* ptr = &(row[i*3]);
              ((unsigned short*)(*gp2ximage).data)[i+j*(gp2ximage->w)]=gp2x_video_RGB_color16(ptr[0], ptr[1], ptr[2]);
            }
          }
          break;
        case (PNG_COLOR_TYPE_RGBA):
          for (j=0; j<height; j++) {
            png_byte* row = row_pointers[j];
            for (i=0; i<width; i++) {
              png_byte* ptr = &(row[i*4]);
              if (ptr[3]==0) {
                ((unsigned short*)(*gp2ximage).data)[i+j*(gp2ximage->w)]=TRANSPARENTCOLOUR ;
              } else {
                ((unsigned short*)(*gp2ximage).data)[i+j*(gp2ximage->w)]=gp2x_video_RGB_color16(ptr[0], ptr[1], ptr[2]);
              }
            }
          }
          break;
      }
      break;
  case 32:
      //convert RGBA to 32 bit format used by the minilib.
      // Allocate 32bits (4 bytes) for each YUV quad
      //gp2ximage->data = (unsigned long*)malloc(4*(width*height));
    {
    unsigned long * ptr = (unsigned long*)malloc(4*(width*height));
    gp2ximage->data=ptr;

      // Run through the data and convert the BGR to 5515.
      switch(info_ptr->color_type) {
	//prueba
	/*
      case (PNG_COLOR_TYPE_PALETTE):
		  png_set_palette_to_rgb(png_ptr);
		  png_set_expand(png_ptr);
		  png_set_strip_16(png_ptr);
		  png_read_update_info(png_ptr, info_ptr);
		  for (j=0; j<height; j++) {
            //png_byte* row = row_pointers[j];
            for (i=0; i<width; i++) {
              R = *(row_pointers[j]+(i*3));
              G = *(row_pointers[j]+(i*3)+1);
              B = *(row_pointers[j]+(i*3)+2);
              COLOR = gp2x_video_RGB_color16(R,G,B);
              //png_byte* ptr = &(row[i*3]);
              ((unsigned short*)(*gp2ximage).data)[i+j*(gp2ximage->w)]=COLOR;
            }
          }
		  break;
	*/
		//prueba
      case (PNG_COLOR_TYPE_RGB):

	for (j=0; j<height; j++) {
	  for (i=0; i<width; i++) {
	    /*
	    R = *(row_pointers[j]+(i*3));
	    G = *(row_pointers[j]+(i*3)+1);
	    B = *(row_pointers[j]+(i*3)+2);
	    *ptr ++ = gp2x_video_YUV_color(R, G, B);
	    */
	    //*ptr ++ = gp2x_video_YUV_color( row_pointers[j][i*3+0], row_pointers[j][i*3+1], row_pointers[j][i*3+2] );
	    //	    *ptr ++ = gp2x_video_YUV_color( row_pointers[j][(i*2)*3+0], row_pointers[j][(i*2)*3+1], row_pointers[j][(i*2)*3+2] );
	    //*ptr ++ = gp2x_video_YUV_color( row_pointers[j][(i*3+0)*2], row_pointers[j][(i*3+1)*2], row_pointers[j][(i*3+2)*2] );
	    *ptr ++ = gp2x_video_YUV_color( row_pointers[j][i*3+0], row_pointers[j][i*3+1], row_pointers[j][i*3+2] );
	    //*ptr ++ = gp2x_video_YUV_color( row_pointers[j][i*4+0], row_pointers[j][i*4+1], row_pointers[j][i*4+2] );
	  }
	}
	/*
	for (j=0; j<height; j++) {
	  //png_byte* row = row_pointers[j];
	  for (i=0; i<width; i++) {
	    //png_byte* ptr = &(row[i*3]);
	    unsigned long *ptr = (unsigned long *)&gp2ximage->data[i+j*(gp2ximage->w)]; 
	    R = *(row_pointers[j]+(i*3));
	    G = *(row_pointers[j]+(i*3)+1);
	    B = *(row_pointers[j]+(i*3)+2);
	    //((unsigned long*)(*gp2ximage).data)[i+j*(gp2ximage->w)]= gp2x_video_YUV_color(R, G, B);
	    *ptr = gp2x_video_YUV_color(R,G,B);
	  }
	}
	*/
	break;
      case (PNG_COLOR_TYPE_RGBA):
	for (j=0; j<height; j++) {
	  png_byte* row = row_pointers[j];
	  for (i=0; i<width; i++) {
	    png_byte* ptr = &(row[i*4]);
	    if (ptr[3]==0) {
	      ((unsigned long*)(*gp2ximage).data)[i+j*(gp2ximage->w)]=TRANSPARENTCOLOUR ;
	    } else {
	      ((unsigned long*)(*gp2ximage).data)[i+j*(gp2ximage->w)]=gp2x_video_YUV_color(ptr[0], ptr[1], ptr[2]);
	    }
	  }
	}
	break;
      }
      break;
    }
    case 15:
    default:
      //convert RGBA to 16 bit 5515 format used by the minilib.
      // Allocate 16bits (2 bytes) for each RGBA quad
      gp2ximage->data = (unsigned short*)malloc(2*(width*height));

      // Run through the data and convert the BGR to 5515.

      switch(info_ptr->color_type) {
        case (PNG_COLOR_TYPE_RGB):
          for (j=0; j<height; j++) {
        png_byte* row = row_pointers[j];
            for (i=0; i<width; i++) {
              png_byte* ptr = &(row[i*3]);
              ((unsigned short*)(*gp2ximage).data)[i+j*(gp2ximage->w)]=gp2x_video_RGB_color15(ptr[0], ptr[1], ptr[2], 0);
            }
          }
          break;
        case (PNG_COLOR_TYPE_RGBA):
          for (j=0; j<height; j++) {
        png_byte* row = row_pointers[j];
            for (i=0; i<width; i++) {
              png_byte* ptr = &(row[i*4]);
              if (ptr[3]==0) {
                ((unsigned short*)(*gp2ximage).data)[i+j*(gp2ximage->w)]=TRANSPARENTCOLOUR ;
              } else {
                ((unsigned short*)(*gp2ximage).data)[i+j*(gp2ximage->w)]=gp2x_video_RGB_color15(ptr[0], ptr[1], ptr[2], 0);
              }
            }
          }
          break;
      }
      break;
  }

  // clean up pointers and shit.
  for (y=0; y<height; y++)
    free(row_pointers[y]);
  free(row_pointers);

  return 0;
}

// Returns 1 for error.
int gp2x_loadBMP(char filename[], gp2x_rect *gp2ximage, int bitdepth, int solid, unsigned char *paletteout)
{
  FILE *f; //our file pointer
  BitmapHeader bitmapFileHeader; //our bitmap file header
  BitmapInfo bitmapInfoHeader; //our bitmap fileinfo header
  unsigned char *bitmapImage;  //store image data
  unsigned long int actualImageSize;
  int imageIndex=0;  //image index counter

  unsigned char *palette;
  int numberOfColoursUsed;

  //open filename in read binary mode
  f = fopen(filename,"rb");
  if (f == NULL) {
    printf("Cocked up opening filename");
    return 1;
  }

  //read the bitmap file header
  fread(&bitmapFileHeader, sizeof(BitmapHeader),1,f);

  //check if it is a bitmap
  if (bitmapFileHeader.filetype !=0x4D42)  //BM
  {
    printf("Error. Not a bitmap file. First two bytes were: %d", bitmapFileHeader.filetype);
    fclose(f);
    return 1;  // Error. Not a bitmap file.
  }

  fread(&bitmapInfoHeader, sizeof(BitmapInfo),1,f);

  actualImageSize = bitmapInfoHeader.imageSize + (bitmapInfoHeader.imageSize2 << 16);

  //move file point to the beginning of bitmap data
  fseek(f, bitmapFileHeader.offsetBytes, SEEK_SET);

  //allocate enough memory for the bitmap image data
  bitmapImage = (unsigned char*)malloc(actualImageSize);
  //verify memory allocation
  if (!bitmapImage)
  {
    free(bitmapImage);
    fclose(f);
    printf("Error. No space allocated for bitmap.");
    printf("Image size is reportedly %lu.", actualImageSize);
    return 1;
  }

  //read in the bitmap image data
  fread(bitmapImage,actualImageSize,1,f);

   //Read in a palette from the bpp

  if (bitmapInfoHeader.bpp==8) {
     // Even though coloursUsed2 should be 0, the number of colours used is allocated 4 bytes so I'm gonna use em...
     numberOfColoursUsed = bitmapInfoHeader.coloursUsed + (bitmapInfoHeader.coloursUsed2<<16);
     palette = (unsigned char*)malloc(numberOfColoursUsed*4);
     fseek(f, 54, SEEK_SET);
     fread(palette, (numberOfColoursUsed*4),1,f);
   }

  //make sure bitmap image data was read
  if (bitmapImage == NULL)
  {
    printf("Error. No bitmap.");
    fclose(f);
    return 1;
  }

  //close file and return bitmap image data
  fclose(f);

  // Now to 'convert' it into a minilib friendly gp2x_rect
  // Start position is (0,0)
  gp2ximage->x = 0;
  gp2ximage->y = 0;
  // Use the width and height from the bitmap.      fileSize + (fileSize2 << 16)
  gp2ximage->w = (bitmapInfoHeader.width +(bitmapInfoHeader.width2 << 16));
  gp2ximage->h = (bitmapInfoHeader.height +(bitmapInfoHeader.height2 << 16));
  // Use TRANSPARENTCOLOUR as a transparent colour? then solid=0. I think.
  gp2ximage->solid = solid;

  int i,j, quadspacing;

  if (bitdepth==8) {
      // Convert bmp to 8 bit
      // Allocate 8bits (1 bytes) for each entry
      gp2ximage->data = (unsigned char*)malloc(actualImageSize);
      // Allocate space for the BGRr quad.
      switch(bitmapInfoHeader.bpp) {
        case 8:
          gp2ximage->data = (unsigned char*)malloc(actualImageSize);
          imageIndex = 0;
          switch ((gp2ximage->w)%4) {
            case 1:
              quadspacing = 3;
              break;
            case 2:
              quadspacing = 2;
              break;
            case 3:
              quadspacing = 1;
              break;
            case 0:
                                  // Fall through
            default:
              quadspacing = 0;
          }

                imageIndex = 0;
                for (j=(gp2ximage->h)-1; j>=0; j--) {
                  for (i=0; i<gp2ximage->w; i++) {
                    ((unsigned char*)(*gp2ximage).data)[i+j*(gp2ximage->w)]=bitmapImage[imageIndex];
                    ++imageIndex;
                  }
                  imageIndex+=quadspacing;
                }
                break;
        case 24:
          // Twiddle thumbs here....
          printf("Can't convert 24bpp bmp to 8bpp rect yet.");
          return 1;
          break;
      }

  } else if (bitdepth==16) {

      switch(bitmapInfoHeader.bpp) {
        case 8:

          gp2ximage->data = (unsigned short*)malloc(2*(actualImageSize));
          imageIndex = 0;
          switch ((gp2ximage->w)%4) {
            case 1:
              quadspacing = 3;
              break;
            case 2:
              quadspacing = 2;
              break;
            case 3:
              quadspacing = 1;
              break;
            case 0:
                                  // Fall through
            default:
              quadspacing = 0;
          }

          for (j=(gp2ximage->h)-1; j>=0; j--) {
            for (i=0; i<gp2ximage->w; i++) {
              unsigned char* ptr = &palette[bitmapImage[imageIndex]*4];
              ((unsigned short*)(*gp2ximage).data)[i+j*(gp2ximage->w)] = gp2x_video_RGB_color16(ptr[2], ptr[1], ptr[0]);
              ++imageIndex;
            }
            imageIndex+=quadspacing;
          }

          break;
        case 24:
          //convert BGR to 16 bit 565 format used by the minilib.

          gp2ximage->data = (unsigned short*)malloc(2*(actualImageSize / 3));

          // Run through the data and convert the BGR to 565.
                imageIndex = 0;
                switch ((3*gp2ximage->w)%4) {
                  case 1:
                    quadspacing = 3;
                    break;
                  case 2:
                    quadspacing = 2;
                    break;
                  case 3:
                    quadspacing = 1;
                    break;
                  case 0:
                                  // Fall through
                  default:
                    quadspacing = 0;
                }

                for (j=(gp2ximage->h)-1; j>=0; j--) {
                  for (i=0; i<gp2ximage->w; i++) {
                    ((unsigned short*)(*gp2ximage).data)[i+j*(gp2ximage->w)]=gp2x_video_RGB_color16(bitmapImage[imageIndex+2], bitmapImage[imageIndex+1],
                    bitmapImage[imageIndex]);
                    imageIndex+=3;
                  }
                  imageIndex+=quadspacing;
                }
                break;
      }
      } else if (bitdepth==15) {
      switch(bitmapInfoHeader.bpp) {
        case 8:
          // Twiddle thumbs here....
          // printf("Can't convert 8bpp bmp to 15bpp rect yet.");
          // return 1;


          gp2ximage->data = (unsigned short*)malloc(2*(actualImageSize));
          imageIndex = 0;
          switch ((gp2ximage->w)%4) {
            case 1:
              quadspacing = 3;
              break;
            case 2:
              quadspacing = 2;
              break;
            case 3:
              quadspacing = 1;
              break;
            case 0:
                                  // Fall through
            default:
              quadspacing = 0;
          }

          for (j=(gp2ximage->h)-1; j>=0; j--) {
            for (i=0; i<gp2ximage->w; i++) {
              unsigned char* ptr = &palette[bitmapImage[imageIndex]*4];
              ((unsigned short*)(*gp2ximage).data)[i+j*(gp2ximage->w)]=gp2x_video_RGB_color15(ptr[2], ptr[1],ptr[0],0);
              ++imageIndex;
            }
            imageIndex+=quadspacing;
          }

          break;
        case 24:
      //convert BGR to 16 bit 5515 format used by the minilib.
      // Allocate 16bits (2 bytes) for each RGB triplet
      gp2ximage->data = (unsigned short*)malloc(2*(actualImageSize / 3));

      // Run through the data and convert the BGR to 5515.
      imageIndex = 0;
      switch ((3*gp2ximage->w)%4) {
          case 1:
              quadspacing = 3;
          break;
          case 2:
              quadspacing = 2;
          break;
          case 3:
              quadspacing = 1;
          break;
          case 0:
              // Fall through
              default:
          quadspacing = 0;
      }

      for (j=(gp2ximage->h)-1; j>=0; j--) {
          for (i=0; i<gp2ximage->w; i++) {
            ((unsigned short*)(*gp2ximage).data)[i+j*(gp2ximage->w)]=gp2x_video_RGB_color15(bitmapImage[imageIndex+2], bitmapImage[imageIndex+1], bitmapImage[imageIndex], 0);
                  imageIndex+=3;
                  }
                  imageIndex+=quadspacing;
              }
      break;
  }
  }

  if (paletteout!=NULL && palette!=NULL) {
    paletteout=palette;
    palette=NULL;
  }

  free(bitmapImage);

  return 0;
}

// To be depreceated
void gp2x_blitwholerect15(gp2x_rect *r)
{
  // rlyeh's blit code
  // Changes: offset+=SCREENWIDTH-r->w; instead of the probable typo offset+=SCREENWIDTH-x;
  // Also uses the define'd TRANSPARENTCOLOUR instead of just black as the transparent colour key

  int x, y; unsigned short *data=r->data, *offset=&gp2x_video_RGB[0].screen16[r->x+r->y*SCREENWIDTH];

  y=r->h; if(r->solid)
      while(y--) { x=r->w; while(x--) *offset++=*data++; offset+=SCREENWIDTH-r->w; }
      else
        while(y--) { x=r->w; while(x--) { if(*data!=TRANSPARENTCOLOUR) *offset=*data; offset++, data++; }
        offset+=SCREENWIDTH-r->w; }
}

void gp2x_blitanimrect(gp2x_rect *blit, int framex, int framey, int frameh, int framew) {
  // Remember; This is way slow compared to gp2x_blitter_rect15
  // Also only 15/16 bit
  int i,j, index;
  index = (framex+(framey*blit->w));
    if ((blit->solid)==0) {
      j=blit->y;
        while (j<=(blit->y+frameh)) {
            for (i=blit->x; i<(blit->x+framew); i++) {
              if ((((unsigned short*)(*blit).data)[index])!=TRANSPARENTCOLOUR) {
                gp2x_video_RGB[0].screen16[i+j*SCREENWIDTH] = (((unsigned short*)(*blit).data))[index];
                }
                ++index;
            }
            index += (blit->w-framew);
            ++j;
        }
    } else {
          j=blit->y;
          while (j<=(blit->y+frameh)) {
            for (i=blit->x; i<(blit->x+framew); i++) {
              gp2x_video_RGB[0].screen16[i+j*SCREENWIDTH] = (((unsigned short*)(*blit).data))[index];
                ++index;
            }
            index += (blit->w-framew);
            ++j;
          }
    }
}

void gp2x_spritemapblit(gp2x_rect *blit, int x, int y, int cols, int rows, int frame) {
  // Function to take care of the framing of a rect, and to serve as a memory aide as to an access routine.
  // Rudimentary bounds checking on frameno, based on cols and rows
  int framex, framey, frameh, framew;
  if (frame>(rows*cols)) frame=frame%(rows*cols);
  framew = blit->w / cols;
  frameh = blit->h / rows;
  framex = framew*(frame%cols);
  framey = frameh*(frame/cols);
  blit->x = x;
  blit->y = y;
  gp2x_blitanimrect(blit, framex, framey, frameh, framew);
}

unsigned short** gp2x_createspritemap(gp2x_rect *map, int cols, int rows) {
  unsigned short **frames;
  int framenumber = 0;
  int framex, framey, framew, frameh, i,j, index, slicearea;

  framew = map->w / cols;
  frameh = map->h / rows;

  slicearea = framew*frameh;

  frames=(unsigned short**)malloc((cols*rows)*sizeof(unsigned short*));
  while(framenumber<(cols*rows)) {
    frames[framenumber] = (unsigned short*)malloc(2*(slicearea));
    framex = framew*(framenumber%cols);
    framey = frameh*(framenumber/cols);
    index = (framex+(framey*map->w));

    j=0;
    while (j<frameh) {
      for (i=0; i<framew; i++) {
        frames[framenumber][i+j*framew] = ((unsigned short*)(*map).data)[index];
        ++index;
      }
      index += (map->w-framew);
      ++j;
    }

    ++framenumber;
  }

  map->w = framew;
  map->h = frameh;

  free((unsigned short*)(*map).data);
  map->data = frames[0];

  return frames;
}
