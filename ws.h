#ifndef __WS_H__
#define __WS_H__

#define SWAN_INTERNAL_EEP "swan.eep"
#define SWAN_CONFIG_SRM   "swan.srm"

#include "types.h"
#include "config.h"

/*
enum {
	WS_SYSTEM_MONO			= 0,
	WS_SYSTEM_COLOR			= 1,
	WS_SYSTEM_AUTODETECT    = 2
};

enum {
	COLOUR_SCHEME_DEFAULT	= 0,
	COLOUR_SCHEME_AMBER		= 1,
	COLOUR_SCHEME_GREEN		= 2
};


enum { // KEYMAP
	WS_UP      = 0x1, WS_R_LEFT  = 0x1,
	WS_DOWN    = 0x4, WS_R_RIGHT = 0x4,
	WS_RIGHT   = 0x2, WS_R_UP    = 0x2,
	WS_LEFT    = 0x8, WS_R_DOWN  = 0x8,
	WS_START   = 0x2,
	WS_BT_A    = 0x4,
	WS_BT_B    = 0x8,
};

*/

#ifdef GP2X
	#define NUM_FBUFFERS 4
#else
	#define NUM_FBUFFERS 2
#endif

enum { WSC_S = 1, WSC_A, WSC_B, WSC_XU, WSC_XR, WSC_XD, WSC_XL, WSC_YU, WSC_YR, WSC_YD, WSC_YL };

void* nec_getRegPtr(int* pLen);
void  nec_reset(void*);
void  nec_setPipeline(int clk);
int   nec_getIF(void);

u32 SWAN_Loop(void);
u32 SWAN_Init(u32 nRomSize, u8 * pRomAddr, t_option * options);
u32 SWAN_Exit(void);

typedef struct ws_romHeaderStruct
{
	u8	developperId;
	u8	minimumSupportSystem;
	u8	cartId;
	u8  undef_3;
	u8	romSize;
	u8	eepromSize;
	u8	additionnalCapabilities;
	u8	realtimeClock;
	u16	checksum;
} ws_romHeaderStruct;

typedef struct _window
{
	u8 x0, y0, x1, y1;
} window;

typedef struct _scroll
{
	u8 x, y;
} scroll;


//-------------------------------------------------------------------
// 変更のないマクロ
//-------------------------------------------------------------------
#define REPx8(a) a;a;a;a;a;a;a;a;
//#define BETWEEN(j,min,max) ((min<=j) && (j<=max))
#define BETWEEN(j,min,max) ((unsigned)(j-min) < (max-min+1))  //((min<=j) && (j<=max))

#define INT_HBLANK        0x80
#define INT_VBLANK        0x40
#define INT_VTIMER        0x20
#define INT_SCLINE        0x10

#define HBLANK_TIMER()   (pWS->ioRam[0xa2]&0x01)
#define HBLANK_AUTO()    (pWS->ioRam[0xa2]&0x02)
#define VBLANK_TIMER()   (pWS->ioRam[0xa2]&0x04)
#define VBLANK_AUTO()    (pWS->ioRam[0xa2]&0x08)

#define IS_BGBG()     (pWS->ioRam[0x00]&0x01)
#define IS_BGFG()     (pWS->ioRam[0x00]&0x02)
#define IS_SPRT()     (pWS->ioRam[0x00]&0x04)

#define TOP_SPRITE		0x2000
#define MIDDLE_SPRITE	0x0

/*
#define BGCOLORMOD(a) (a)
#define BGBGMOD(a)    (a)
#define BGFGMOD(a)    (a)
#define BGWNMOD(a)    (a)
#define SPMOD(a)      (a)
*/

//#define DRAW_MASK       0x8000
//#define BGC_MASK(col)  (BGCOLORMOD(col))
//#define BGBGMASK(col)  (BGBGMOD(col))
//#define BGFGMASK(col)  (BGFGMOD(col)|DRAW_MASK)
//#define BGFGMASK(col)  (col|DRAW_MASK)
//#define BGWNMASK(col)  (BGWNMOD(col)|DRAW_MASK)
//#define BGWNMASK(col)  (col|DRAW_MASK)
//#define SPMASK(col)    (SPMOD(col))

#define CPU_CLOCK          30720000
#define PAL_TILE(tInfo)    (((tInfo)>>9)&0x0f)

#define IO_BACKUP_A4 0xf4
#define IO_BACKUP_A5 0xf5
#define IO_BACKUP_A6 0xf6
#define IO_BACKUP_A7 0xf7

#define SOUND_SamplesPerFrame 588 /* 44100/75=588 */
#define SWAN_SOUND_CHANNELS   4
#define WAVE_SAMPLES          32

#define IO_ROM_BANK_BASE_SELECTOR	0xC0

//-------------------------------------------------------------------
// OSWAN
//-------------------------------------------------------------------
typedef struct sound {
    int on;
    int freq;
    int volL;
    int volR;
} SOUND;
typedef struct sweep {
    int on;
    int time;
    int step;
    int cnt;
} SWEEP;
typedef struct noise {
    int on;
    int pattern;
} NOISE;

extern unsigned long WaveMap;
extern SOUND Ch[4];
extern int VoiceOn;
extern SWEEP Swp;
extern NOISE Noise;
//extern unsigned char PData[4][32];
extern int Sound[7];
extern int WsWaveVol;

/*
int apuWaveCreate(void);
void apuWaveRelease(void);
void apuWaveClear(void);
int apuInit(void);
void apuEnd(void);
unsigned int apuMrand(unsigned int Degree);
void apuSetPData(int addr, unsigned char val);
unsigned char apuVoice(void);
void apuSweep(void);
WORD apuShiftReg(void);
void apuWaveSet(void);
void apuStartupSound(void);
*/

//---------------------------------------------------------------------------
// I/O
//---------------------------------------------------------------------------
#define DSPCTL  pWS->ioRam[0x00]        //
#define BORDER  pWS->ioRam[0x01]        //
#define RSTRL   pWS->ioRam[0x02]        // 
#define RSTRLC  pWS->ioRam[0x03]        // 
#define SPRTAB  pWS->ioRam[0x04]        // 
#define SPRBGN  pWS->ioRam[0x05]        // 
#define SPRCNT  pWS->ioRam[0x06]        // 
#define SCRMAP  pWS->ioRam[0x07]        // 
#define SCR2WL  pWS->ioRam[0x08]        // foreground X
#define SCR2WT  pWS->ioRam[0x09]        // foreground Y
#define SCR2WR  pWS->ioRam[0x0A]        // foreground X
#define SCR2WB  pWS->ioRam[0x0B]        // foreground Y
#define SPRWL   pWS->ioRam[0x0C]        // X
#define SPRWT   pWS->ioRam[0x0D]        // Y
#define SPRWR   pWS->ioRam[0x0E]        // X
#define SPRWB   pWS->ioRam[0x0F]        // Y
#define SCR1X   pWS->ioRam[0x10]        // 
#define SCR1Y   pWS->ioRam[0x11]        // 
#define SCR2X   pWS->ioRam[0x12]        // foreground X
#define SCR2Y   pWS->ioRam[0x13]        // foreground Y
#define LCDSLP  pWS->ioRam[0x14]        // LCD
#define LCDSEG  pWS->ioRam[0x15]        // segment

#define COL     (BYTE*)(pWS->ioRam+0x1C)    // 
#define PAL     (BYTE*)(pWS->ioRam+0x20)    // 

#define DMASRC  (*(DWORD*)(pWS->ioRam+0x40))    //IO[]が4バイト境界にあることが必要
#define DMADST  (*(WORD*)(pWS->ioRam+0x44)) //
#define DMACNT  (*(WORD*)(pWS->ioRam+0x46)) //

#define SDMASA  (*(WORD*)(pWS->ioRam+0x4A)) //
#define SDMASB  pWS->ioRam[0x4C]        // 
#define SDMACNT (*(WORD*)(pWS->ioRam+0x4E)) //
#define SDMACTL pWS->ioRam[0x52]        // 

#define COLCTL  pWS->ioRam[0x60]        // 

#define SNDP    pWS->ioRam[0x80]        // 
#define SNDV    pWS->ioRam[0x88]        // 
#define SNDSWP  pWS->ioRam[0x8C]        // 
#define SWPSTP  pWS->ioRam[0x8D]        // 
#define NSCTL   pWS->ioRam[0x8E]        // 
#define WAVDTP  pWS->ioRam[0x8F]        // WAVE
#define SNDMOD  pWS->ioRam[0x90]        // 
#define SNDOUT  pWS->ioRam[0x91]        // 
#define NCSR    (*(WORD*)(pWS->ioRam+0x92))  //

#define HWARCH  pWS->ioRam[0xA0]        // 

#define TIMCTL  pWS->ioRam[0xA2]        // 
#define HPRE    (*(WORD*)(pWS->ioRam+0xA4))  //
#define VPRE    (*(WORD*)(pWS->ioRam+0xA6))  //
#define HCNT    (*(WORD*)(pWS->ioRam+0xA8))  //
#define VCNTL   (*(WORD*)(pWS->ioRam+0xAA))  //  L
#define VCNTH   (*(WORD*)(pWS->ioRam+0xAC))  //  H

#define IRQBSE  pWS->ioRam[0xB0]        // 
#define COMDT   pWS->ioRam[0xB1]        // 
#define IRQENA  pWS->ioRam[0xB2]        // 
#define COMCTL  pWS->ioRam[0xB3]        // 
#define KEYCTL  pWS->ioRam[0xB5]        // 
#define IRQACK  pWS->ioRam[0xB6]        // 

#define EEPDTL  pWS->ioRam[0xBA]        // 
#define EEPDTH  pWS->ioRam[0xBA]        // 
#define EEPADL  pWS->ioRam[0xBA]        // 
#define EEPADH  pWS->ioRam[0xBA]        // 
#define EEPCTL  pWS->ioRam[0xBA]        // 

#define BNK1SEL pWS->ioRam[0xC1]        // 
#define BNK2SEL pWS->ioRam[0xC2]        // 
#define BNK3SEL pWS->ioRam[0xC3]        // 

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
#define TXD_IFLAG 0x01
#define KEY_IFLAG 0x02
#define CRT_IFLAG 0x04
#define RXD_IFLAG 0x08
#define RST_IFLAG 0x10
#define VTM_IFLAG 0x20
#define VBB_IFLAG 0x40
#define HTM_IFLAG 0x80

#endif /*__WS_H__*/
