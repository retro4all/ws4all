
#ifdef WIN32
#pragma warning(disable:4244)
#pragma warning(disable:4018)
#pragma warning(disable:4761)
#pragma warning(disable:4005)
#endif

#ifndef __COM_TYPES_H__
#define __COM_TYPES_H__

#define LSB_FIRST             /* Little Endian */
//#define PIXEL_FORMAT   u8    /* Pixel Format  */

typedef   signed char  int8;
typedef   signed short int16;
typedef   signed long  int32;
typedef unsigned char  uint8, byte, BYTE;
typedef unsigned short uint16, word, WORD;
typedef unsigned long  uint32, dword, DWORD;

#define s8  signed char
#define u8  unsigned char
#define s16 signed short
#define u16 unsigned short
#define s32 signed int
#define u32 unsigned int

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

#define MAX_PATH    512

// Hardware types
/*
enum
{
	HW_PCE  = 0,
	HW_NES  = 1,
	HW_GBC  = 2,
	HW_WSC  = 3,
	HW_NGP  = 4,
	HW_SMS  = 5,
	HW_LNX  = 6,
	HW_NULL = 999
} ;
*/


/* frame buffer format */
/*
typedef struct
{
	u32 width;   // FB width
	u32 height;  // FB height
	u32 bpp;     // Bit Per Pixel (=sizeof(PIXEL_FORMAT))

	u32 pic_x;   // x (in FB)
	u32 pic_y;   // y (in FB)

	u32 pic_w;   // width (in FB)
	u32 pic_h;   // height(in FB)


	u32 rotate;  // rotate flag

	PIXEL_FORMAT *fb;     // 32bit pointer
	PIXEL_FORMAT *fb_tmp; // work for user

	u32 hardware;   // hardware type

} FBFORMAT;
*/

/*
typedef struct
{
	char  szExt[4];
	int   nExtId;
} FILE_EXTENTION;
*/

//-----------------------------------------------------------------------------
// support extensions
//-----------------------------------------------------------------------------
/*
enum
{
	EXT_NULL,
	EXT_UNKNOWN,
	EXT_ALL,

	EXT_GB,  // GameBoy
	EXT_GBC,
	EXT_SGB,

	EXT_PCE, // PC-Engine
	EXT_SGX,
	EXT_TOC,

	EXT_WS,  // WonderSwan
	EXT_WSC,

	EXT_NPC, // NEOGEO Pocket
	EXT_NGP,
	EXT_NGC,

	EXT_NES, // NES

	EXT_SMS, // MasterSystem
	EXT_GG , // GameGear

	EXT_LNX,

	EXT_MP3,
	EXT_TXT,
	EXT_ZIP,
};
*/

//-----------------------------------------------------------------------------
// Handler Table
//-----------------------------------------------------------------------------
/*
typedef struct
{
	int ext;
	int (*pINIT)(int, byte*);
	int (*pLOOP)(void);
	int (*pEXIT)(void);
	int (*pRESET)(void);

	// FOR STATE HANDLER
	int (*pLOAD)(int);
	int (*pSAVE)(int);
	char szExt[4];
} CORE_HANDLER;
*/

//#include "cstring.h"

#define numof(array)             (sizeof((array))/sizeof((array)[0]))

#endif // __COM_TYPES_H__


