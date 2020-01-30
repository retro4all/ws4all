#define DEF_QUICK
//
// [Slow down]
// (1) [DMA is wasted if the updating process is slightly palette]
// (2)
//

////////////////////////////////////////////////////////////////////////////////
//
// Wonderswan emulator
//
////////////////////////////////////////////////////////////////////////////////
#include "hal.h"

#include "ws.h"
#include "nec.h"
#include "necintrf.h"
#include "initval.h"

//#include "misc.h"



#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PSG_VOLUME       80  // 120 -> 80 = 64 + 16
#define VOICE_VOLUME     80  // 100 -> 80 = 64 + 16
#define NOISE_VOLUME     380 // 400 -> 380 ~= 256 + 128

#define BGFG_CLIP_MASK   0x30
#define BGFG_CLIP_SIDE   0x10

#define BGFG_CLIP_ENABLE 0x20
#define BGFG_CLIP_OUT    0x30
#define BGFG_CLIP_IN     0x20

#define VBLANK_BEGIN_LINE     145
#define END_DRAW_LINE		  144
#define SCANLINE_MAX          159

#define FBPTR(y) (screen8 + ((y+48) << 8) + ((y+48) << 6) + 48) 	// screen8 + (y+48) * 320 + 48
#define FBPTRTEMP(y) (fbtemp8 + ((y+48) << 8) + ((y+48) << 6) + 48) // fbtemp8 + (y+48) * 320 + 48

#define FILTER_NONE   0
#define FILTER_LINEAR 1

#define MESSAGE_DELAY 200

//-------------------------------------------------------------------
// TILE CACHE (Datos temporales de video)
//-------------------------------------------------------------------
typedef struct
{
	//u32 pal_flag;
	u8 wsc_n[0x10000];		// Cache de tiles en posicion normal
	u8 wsc_h[0x10000];		// Cache de tiles con Flip horizontal
	u32 modify[1024];		// Bytes "dirty" de los tiles

	u8 palette[256][4];		// Paleta de colores (WSC)
	u8 mpalette[8][3];		// Paleta de colores (WSM)
	u8 shades[16][3];		// Pool de grises (WSM)

	u32 rotated;
	u32 vertical;
	u32 fullscreen;
	u32 idxBlack;
	u32 idxWhite;

	// [Used to speed]
	u32 trans[1024][8];		// Indica si la fila seleccionada es transparente
	u8 scanline_num[160];

} SYSTEM_CACHE_T;

//-------------------------------------------------------------------
// WonderSwan Structure (Contexto de WonderSwan)
//-------------------------------------------------------------------
typedef struct
{
	u8 tag_iram[4];   // State Tag
	u8 iRam[0x10000];

	u8 tag_ioram[4];  // State Tag

	union
	{
		u8 ioRam[0x100];
		struct
		{
			u8 disp_ctl;
			u8 bg_color;
			u8 line;
			u8 line_cmp;
			u8 spr_base;
			u8 spr_cnt;
			u8 tile_ram;
			window fg, spr;
			scroll bg_scr, fg_scr;
			u8 lcd;
			u8 lcd_icon;
		};
	};

	u8 tag_spr[4];    // State Tag
	u8 spTableCnt;
	u32 spTable[128];

	u8 videoMode;

	u8 tag_snd[4];    // State Tag
	// Audio Work Parameter
	s32 noise_k;
	s32 noise_v;
	s32 noise_r;
	s32 noise_c;
	s32 sweep_pitch;
	s32 sweep_step;
	s32 sweep_value;
	s32 sweep_upd;
	s32 sweep_count;
	s32 snd_n[4];
	s32 snd_k[4];

	// [Calculate and store the following sizes in bulk to save these]
	u8 tag_eE2P[4];    // State Tag
	u8 romE2P[0x800];   // E2P(16kb=2048Byte) in ROM Cart

	u8 tag_eRAM[4];    // State Tag
	u8 romRam[0x40000]; // RAM( 2Mb=256KByte) in ROM Cart

} WONDERSWAN_T;


static void drawLine(u32 beg, u32 end);
static u32 ws_reset(void);
static void ws_set_colour_scheme(t_option * options);
static void ws_gpu_changeVideoMode(u8 value);
static void ws_gpu_clearCache(void);
static u32 check_keys();

// internal values
static WONDERSWAN_T* pWS = 0;
static SYSTEM_CACHE_T* pWC = 0;
static BYTE* ws_RomAddr = 0;
static DWORD ws_RomSize = 0;

// export functions
void cpu_writeport(DWORD port, BYTE value);
void cpu_writemem20(DWORD addr, BYTE value);
static void swan_sound(u32 ch, u32 do_cursor);
static void ws_voice_dma(void);

// export values
static u32 ws_key;
u32 rtcDataRegisterReadCount = 0;
u8* pWsRomMap[0x10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static u32 bgBase = 0;
static u32 fgBase = 0;

// [Temporary area]
static u32 sramAddressMask;
static u32 romAddressMask;
static u32 eEepromAddressMask;

//static s32 waveTable[SWAN_SOUND_CHANNELS][WAVE_SAMPLES];

static u8 * fbtemp8;

/*
#define VOLUME_R(ch)     (pWS->ioRam[0x88+ch]&15)
#define VOLUME_L(ch)    ((pWS->ioRam[0x88+ch]>>4)&15)
#define GET_PITCH(ch)   ((((s32)pWS->ioRam[0x81+((ch)<<1)]&7)<<8)|((s32)pWS->ioRam[0x80+((ch)<<1)]))
*/

// ********* Puck ****************
//u32 frames = 0;

#ifdef DEBUG
u32 frames_rendered = 0;
u32 frames_displayed = 0;
u32 last_frames_rendered = 0;
u32 last_frames_displayed = 0;
static u32 showBGBG = 1;
static u32 showBGFG = 1;
static u32 showSptr = 1;
/*
static u32 psgEnabled = 1;
static u32 sweepEnabled = 1;
static u32 noiseEnabled = 1;
static u32 voiceEnabled = 1;
*/
#else
static u32 frames_rendered = 0;
static u32 frames_displayed = 0;
static u32 last_frames_rendered = 0;
static u32 last_frames_displayed = 0;
#endif

s16 * soundbuffer;
s16 lastSampleL, lastSampleR;
//int master_volume = 100;

//u32 FrameSkip = 0;
//u32 SkipCnt = 1;
u32 volume = 100;
//u32 scheme = 0;
//u32 fullscreen = 0;
static u32 stateSlot = 0;
static u32 showFPS = 0;
static char sfps[10];
static u32 fpsX;
static u32 fpsY;
static u32 showMessage;
static char sMessage[50];
static u32 messageX;
static u32 messageY;
// ********************************


//------------------------------------------------------------------------------
//- OSWAN
//------------------------------------------------------------------------------

/* Conversiones:
 * IO -> pWS->ioRam
 * Page -> pWsRomMap
 * IRAM -> pWS->iRam
 */

#define BUFSIZEN    0x10000
#define BPSWAV      12000 // WSÌHblankª12KHz
#define BUFSIZEW    64
#define BUFCOUNT    30

unsigned long WaveMap;
SOUND Ch[4];
int VoiceOn;
SWEEP Swp;
NOISE Noise;
//int Sound[7] = {1, 1, 1, 1, 1, 1, 1};
int WsWaveVol = 20;

static unsigned char PData[4][32];
static unsigned char PDataN[8][BUFSIZEN];
static unsigned int RandData[BUFSIZEN];

static void (*soundFill)(int waveR, int waveL);

static unsigned int apuMrand(unsigned int Degree)
{
    typedef struct {
        unsigned int N;
        int InputBit;
        int Mask;
    } POLYNOMIAL;

    static POLYNOMIAL TblMask[]=
    {
        { 2,BIT(2) ,BIT(0)|BIT(1)},
        { 3,BIT(3) ,BIT(0)|BIT(1)},
        { 4,BIT(4) ,BIT(0)|BIT(1)},
        { 5,BIT(5) ,BIT(0)|BIT(2)},
        { 6,BIT(6) ,BIT(0)|BIT(1)},
        { 7,BIT(7) ,BIT(0)|BIT(1)},
        { 8,BIT(8) ,BIT(0)|BIT(2)|BIT(3)|BIT(4)},
        { 9,BIT(9) ,BIT(0)|BIT(4)},
        {10,BIT(10),BIT(0)|BIT(3)},
        {11,BIT(11),BIT(0)|BIT(2)},
        {12,BIT(12),BIT(0)|BIT(1)|BIT(4)|BIT(6)},
        {13,BIT(13),BIT(0)|BIT(1)|BIT(3)|BIT(4)},
        {14,BIT(14),BIT(0)|BIT(1)|BIT(4)|BIT(5)},
        {15,BIT(15),BIT(0)|BIT(1)},
        { 0,      0,      0},
    };
    static POLYNOMIAL *pTbl = TblMask;
    static int ShiftReg = BIT(2)-1;
    int XorReg = 0;
    int Masked;

    if(pTbl->N != Degree) {
        pTbl = TblMask;
        while(pTbl->N) {
            if(pTbl->N == Degree) {
                break;
            }
            pTbl++;
        }
        if(!pTbl->N) {
            pTbl--;
        }
        ShiftReg &= pTbl->InputBit-1;
        if(!ShiftReg) {
            ShiftReg = pTbl->InputBit-1;
        }
    }
    Masked = ShiftReg & pTbl->Mask;
    while(Masked) {
        XorReg ^= Masked & 0x01;
        Masked >>= 1;
    }
    if(XorReg) {
        ShiftReg |= pTbl->InputBit;
    }
    else {
        ShiftReg &= ~pTbl->InputBit;
    }
    ShiftReg >>= 1;
    return ShiftReg;
}

static int apuInit(void)
{
    int i, j;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 32; j++) {
            PData[i][j] = 8;
        }
    }
    for (i = 0; i < 8; i++) {
        for (j = 0; j < BUFSIZEN; j++) {
            PDataN[i][j] = ((apuMrand(15 - i) & 1) ? 15 : 0);
        }
    }

    for (i = 0; i < BUFSIZEN; i++) {
        RandData[i] = apuMrand(15);
    }

    return 0;
}

static void apuSetPData(int addr, unsigned char val)
{
    int i, j;

    i = (addr & 0x30) >> 4;
    j = (addr & 0x0F) << 1;
    PData[i][j]=(unsigned char)(val & 0x0F);
    PData[i][j + 1]=(unsigned char)((val & 0xF0)>>4);
}

static unsigned char apuVoice(void)
{
    static int index = 0, b = 0;
    unsigned char v;

    if ((SDMACTL & 0x98) == 0x98) { // Hyper voice
        v = pWsRomMap[SDMASB + b][SDMASA + index++];
        if ((SDMASA + index) == 0)
        {
            b++;
        }
        if (v < 0x80) {
            v += 0x80;
        }
        else {
            v -= 0x80;
        }
        if (SDMACNT <= index) {
            index = 0;
            b = 0;
        }
        return v;
    }
    else if ((SDMACTL & 0x88) == 0x80) { // DMA start
        pWS->ioRam[0x89] = pWsRomMap[SDMASB + b][SDMASA + index++];
        if ((SDMASA + index) == 0)
        {
            b++;
        }
        if (SDMACNT <= index) {
            SDMACTL &= 0x7F; // DMA end
            SDMACNT = 0;
            index = 0;
            b = 0;
        }
    }
    return ((VoiceOn /*&& Sound[4]*/) ? pWS->ioRam[0x89] : 0x80);
}

static void apuSweep(void)
{
    if ((Swp.step) && Swp.on) { // sweep on
        if (Swp.cnt < 0) {
            Swp.cnt = Swp.time;
            Ch[2].freq += Swp.step;
            Ch[2].freq &= 0x7ff;
        }
        Swp.cnt--;
    }
}

static WORD apuShiftReg(void)
{
    static int nPos = 0;
    // Noise counter
    if (++nPos >= BUFSIZEN) {
        nPos = 0;
    }
    return RandData[nPos];
}

static void apuWaveSet(void)
{
    static  int point[] = {0, 0, 0, 0};
    static  int preindex[] = {0, 0, 0, 0};
    int     channel, index;
    short   value, lVol[4], rVol[4];
    short   LL, RR, vVol;

    apuSweep();
    for (channel = 0; channel < 4; channel++) {
        lVol[channel] = 0;
        rVol[channel] = 0;
        if (Ch[channel].on) {
            if (channel == 1 && VoiceOn /*&& Sound[4]*/) {
                continue;
            }
			/*
            else if (channel == 2 && Swp.on && !Sound[5]) {
                continue;
            }
			*/
            else if (channel == 3 && Noise.on /*&& Sound[6]*/){
                index = (3072000 / BPSWAV) * point[3] / (2048 - Ch[3].freq);
                if ((index %= BUFSIZEN) == 0 && preindex[3]) {
                    point[3] = 0;
                }
                value = (short)PDataN[Noise.pattern][index] - 8;
            }
			/*
            else if (Sound[channel] == 0) {
                continue;
            }
			*/
            else {
                index = (3072000 / BPSWAV) * point[channel] / (2048 - Ch[channel].freq);
                if ((index %= 32) == 0 && preindex[channel]) {
                    point[channel] = 0;
                }
                value = (short)PData[channel][index] - 8;
            }
            preindex[channel] = index;
            point[channel]++;
            lVol[channel] = value * Ch[channel].volL; // -8*15=-120, 7*15=105
            rVol[channel] = value * Ch[channel].volR;
        }
    }
    vVol = ((short)apuVoice() - 0x80);
    // mix 16bits wave -32768 ` +32767 32768/120 = 273
    LL = (lVol[0] + lVol[1] + lVol[2] + lVol[3] + vVol) * WsWaveVol;
    RR = (rVol[0] + rVol[1] + rVol[2] + rVol[3] + vVol) * WsWaveVol;

    soundFill(LL, RR);
}

static void soundFill12k(int waveR, int waveL)
{	
	*soundbuffer++ = waveR;
	*soundbuffer++ = waveL;
}

static void soundFill24k(int waveR, int waveL)
{	
	*soundbuffer++ = waveR;
	*soundbuffer++ = waveL;
	*soundbuffer++ = waveR;
	*soundbuffer++ = waveL;
}

static void soundFill48k(int waveR, int waveL)
{	
	*soundbuffer++ = waveR;
	*soundbuffer++ = waveL;
	*soundbuffer++ = waveR;
	*soundbuffer++ = waveL;
	*soundbuffer++ = waveR;
	*soundbuffer++ = waveL;
	*soundbuffer++ = waveR;
	*soundbuffer++ = waveL;
}

static void soundFillLinear24k(int waveR, int waveL)
{	
	s16 avgR, avgL;
	
	avgR = (waveR - lastSampleR) >> 1;
	avgL = (waveL - lastSampleL) >> 1;
	
	*soundbuffer++ = avgR;
	*soundbuffer++ = avgL;
	*soundbuffer++ = waveR;
	*soundbuffer++ = waveL;
	
	lastSampleR = waveR;
	lastSampleL = waveL;
}

static void soundFillLinear48k(int waveR, int waveL)
{	
	s16 avgR, avgL;
	
	avgR = (waveR - lastSampleR) >> 2;
	avgL = (waveL - lastSampleL) >> 2;
	
	/*
	lastSampleR += avgR;
	lastSampleL += avgL;
	*soundbuffer++ = lastSampleR;
	*soundbuffer++ = lastSampleL;
	
	lastSampleR += avgR;
	lastSampleL += avgL;
	*soundbuffer++ = lastSampleR;
	*soundbuffer++ = lastSampleL;

	lastSampleR += avgR;
	lastSampleL += avgL;
	*soundbuffer++ = lastSampleR;
	*soundbuffer++ = lastSampleL;
	*/
	
	*soundbuffer++ = lastSampleR + avgR;
	*soundbuffer++ = lastSampleL + avgL;
	
	*soundbuffer++ = lastSampleR + (avgR << 1);
	*soundbuffer++ = lastSampleL + (avgL << 1);
	
	*soundbuffer++ = lastSampleR + (avgR << 1) + avgR;
	*soundbuffer++ = lastSampleL + (avgL << 1) + avgR;
	
	lastSampleR = waveR;
	lastSampleL = waveL;		
	*soundbuffer++ = waveR;
	*soundbuffer++ = waveL;
}


//------------------------------------------------------------------------------
//-
//------------------------------------------------------------------------------
static u32 ws_reset(void)
{
	if (pWS && pWC)
	{
		u32 i;

		ws_key = 0;
		memset(pWS->iRam, 0, sizeof(pWS->iRam));
		//ws_set_colour_scheme(0);  /* default scheme setup */

		pWS->spTableCnt = 0;

		for (i = 0; i < 0x0c9; ++i)
		{
			cpu_writeport(i, initialIoValue[i]);
		}

		rtcDataRegisterReadCount = 0;

		/* ws_gpu_init */
		ws_gpu_clearCache();
		ws_gpu_changeVideoMode(2);

		nec_reset(NULL);
		nec_set_reg(NEC_SP, 0x2000);
	}

	return 1;
}


//------------------------------------------------------------------------------
//-
//------------------------------------------------------------------------------
void ws_set_colour_scheme(t_option * options)
{
	if (pWS)
	{
		u32 i;

		for (i = 0; i < 16; ++i)
		{
			pWC->shades[i][0] = options->shades[i][0];
			pWC->shades[i][1] = options->shades[i][1];
			pWC->shades[i][2] = options->shades[i][2];
		}	
		
		/*
		u32 r, g, b, i, R, G, B;

		if (options->scheme != -1) // Paletas prefijadas
		{
			switch (options->scheme)
			{
				case 0:
					r = 100;
					g = 100;
					b = 100;
					break;// white
				case 1:
					r = 60;
					g = 61;
					b =  0;
					break;// amber
				case 2:
					r = 20;
					g = 90;
					b = 20;
					break;// green
				case 3:
					r =  0;
					g = 61;
					b = 60;
					break;// blue
			}

			for (i = 0; i < 16; ++i)
			{
				R = (((15 - i) * r) / 100);
				G = (((15 - i) * g) / 100);
				B = (((15 - i) * b) / 100);
				pWC->shades[i][0] = R << 4;
				pWC->shades[i][1] = G << 4;
				pWC->shades[i][2] = B << 4;
			}
		}
		else // Paletas personalizadas
		{
			for (i = 0; i < 16; ++i)
			{
				pWC->shades[i][0] = options->shades[i][0];
				pWC->shades[i][1] = options->shades[i][1];
				pWC->shades[i][2] = options->shades[i][2];
			}		
		}
		*/
	}
}

//------------------------------------------------------------------------------
// - [There was a problem handling the video mode.]
// - [0: 4,5,6,7 there is a transparent color palette, I / O using a palette]
// - [4: 4,5,6,7 there is a transparent color palette, MEM using a palette]
// - [6: The color palette transparency exists in all different ways and produce tiles 7]
// - [7: The color palette present in complete transparency, and how to generate six different tile]
//------------------------------------------------------------------------------
static void ws_gpu_changeVideoMode(u8 value)
{
	u32 i;
	value = (value >> 5) & 7;

	switch (value)
	{
		case 7:
			pWS->videoMode = 7;
			break;
		case 6:
			pWS->videoMode = 6;
			break;
		case 4:
			pWS->videoMode = 4;
			break;
		default:
			pWS->videoMode = 0;
			break;
	}

	if (pWS->videoMode)
	{
		pWC->idxBlack = 0;
		pWC->idxWhite = 16;
	}
	else
	{
		pWC->idxBlack = 4;
		pWC->idxWhite = 20;
	}
		
	// Rellena la pantalla de color transparente
	for (i = 0; i < NUM_FBUFFERS; i++)
	{
		memset (screen8, pWC->idxBlack, 320 * 240);
		videoFlip(0);
	}	

	// cache clear [title because the garbage was not SAGA]
	ws_gpu_clearCache();
}

//------------------------------------------------------------------------------
// [Clear the palette and Baguru here.]
// [Character problems disappear when the Terrors 2]
// [Core_memset (pWC, 0, sizeof (* pWC));]
// [Had to do and because]
//------------------------------------------------------------------------------
static void ws_gpu_clearCache(void)
{
	//pWC->pal_flag = -1;
	memset(pWC->wsc_n, 0, sizeof(*pWC->wsc_n));
	memset(pWC->wsc_h, 0, sizeof(*pWC->wsc_h));
	memset(pWC->modify, -1, sizeof(pWC->modify));
}

//------------------------------------------------------------------------------
//- Get Palette Pointer (COLOR / MONO)
//------------------------------------------------------------------------------
static void ws_gpu_palette(u32 pal_idx)
{
	/*
	// 0 : [Palette is not being updated]
	if ( (pWC->pal_flag & (1 << pal_idx)) )
	{
		pWC->pal_flag &= ~(1 << pal_idx);

		if (pWS->videoMode == 0)
		{
	*/
			u32 p = 0x20 + (pal_idx << 1);

			u32 idx1 = (pal_idx << 4) + 0;
			u32 idx2 = (pWS->ioRam[p+0]   ) & 7;
			pWC->palette[idx1][0] = pWC->mpalette[idx2][0];
			pWC->palette[idx1][1] = pWC->mpalette[idx2][1];
			pWC->palette[idx1][2] = pWC->mpalette[idx2][2];
			pWC->palette[idx1][3] = 1;

			idx1 = (pal_idx << 4) + 1;
			idx2 = (pWS->ioRam[p+0] >> 4) & 7;
			pWC->palette[idx1][0] = pWC->mpalette[idx2][0];
			pWC->palette[idx1][1] = pWC->mpalette[idx2][1];
			pWC->palette[idx1][2] = pWC->mpalette[idx2][2];
			pWC->palette[idx1][3] = 1;

			idx1 = (pal_idx << 4) + 2;
			idx2 = (pWS->ioRam[p+1]   ) & 7;
			pWC->palette[idx1][0] = pWC->mpalette[idx2][0];
			pWC->palette[idx1][1] = pWC->mpalette[idx2][1];
			pWC->palette[idx1][2] = pWC->mpalette[idx2][2];
			pWC->palette[idx1][3] = 1;

			idx1 = (pal_idx << 4) + 3;
			idx2 = (pWS->ioRam[p+1] >> 4) & 7;
			pWC->palette[idx1][0] = pWC->mpalette[idx2][0];
			pWC->palette[idx1][1] = pWC->mpalette[idx2][1];
			pWC->palette[idx1][2] = pWC->mpalette[idx2][2];
			pWC->palette[idx1][3] = 1;
		/*
		}
	}
	*/
}

//------------------------------------------------------------------------------
//-
// (tInfo&0x1ff, offsetY, tInfo&0x8000, tInfo&0x4000, tInfo&0x2000);
//------------------------------------------------------------------------------
/*
 * Retorna: 0 si el tile es transparente
 *          Un puntero a la cache si no es transparente:
 *              - pWC->wsc_n -> No hay flip horizontal
 *              - pWC->wsc_h -> Si hay flip horizontal
*/
static u8* getTileRow(u32 tInfo, u32 line)
{
#define PHN(h,n) pH[(h)] = pN[(n)]
	u32 i, tL, tLP;
	u32 tileIndex = tInfo & 0x01ff;
	u32 vFlip     = tInfo & 0x8000;
	u32 hFlip     = tInfo & 0x4000;
	//u32 bank      = (tInfo & 0x2000) ? 1 : 0;
	u32 bank      = tInfo & 0x2000;

	if (pWS->videoMode >= 4 && bank)
	{
		tileIndex += 512; // Al hacer luego <<5 cambia al banco 1.
	}

	if (pWC->modify[tileIndex])
	{
		u8* pN = &pWC->wsc_n[tileIndex<<6];
		u8* pH = &pWC->wsc_h[tileIndex<<6];

		pWC->modify[tileIndex] = 0;

		switch ( pWS->videoMode )
		{
			case 7:   // 1pixel = 4bit
			{
				u32 *tIRP = (u32*)&pWS->iRam[0x4000+(tileIndex<<5)];

				for (i = 0; i < 8; ++i)
				{
					tL = *tIRP++;
					PHN(7, 0) = (tL >> 4) & 0x0f;
					PHN(6, 1) = (tL    ) & 0x0f;
					PHN(5, 2) = (tL >> 12) & 0x0f;
					PHN(4, 3) = (tL >> 8) & 0x0f;
					PHN(3, 4) = (tL >> 20) & 0x0f;
					PHN(2, 5) = (tL >> 16) & 0x0f;
					PHN(1, 6) = (tL >> 28)     ;
					PHN(0, 7) = (tL >> 24) & 0x0f;
					pN += 8;
					pH += 8;
					pWC->trans[tileIndex][i] = tL;
				}
			}
			break;

			case 6:   // 1pixel = 4bit
			{
				u32 *tIRP = (u32*)&pWS->iRam[0x4000+(tileIndex<<5)];

				for (i = 0; i < 8; ++i)
				{
					tL = *tIRP++;
					tLP = (tL >> 7) & 0x01010101;
					PHN(7, 0) = tLP | (tLP >> 7) | (tLP >> 14) | (tLP >> 21);
					tLP = (tL >> 6) & 0x01010101;
					PHN(6, 1) = tLP | (tLP >> 7) | (tLP >> 14) | (tLP >> 21);
					tLP = (tL >> 5) & 0x01010101;
					PHN(5, 2) = tLP | (tLP >> 7) | (tLP >> 14) | (tLP >> 21);
					tLP = (tL >> 4) & 0x01010101;
					PHN(4, 3) = tLP | (tLP >> 7) | (tLP >> 14) | (tLP >> 21);
					tLP = (tL >> 3) & 0x01010101;
					PHN(3, 4) = tLP | (tLP >> 7) | (tLP >> 14) | (tLP >> 21);
					tLP = (tL >> 2) & 0x01010101;
					PHN(2, 5) = tLP | (tLP >> 7) | (tLP >> 14) | (tLP >> 21);
					tLP = (tL >> 1) & 0x01010101;
					PHN(1, 6) = tLP | (tLP >> 7) | (tLP >> 14) | (tLP >> 21);
					tLP = (tL >> 0) & 0x01010101;
					PHN(0, 7) = tLP | (tLP >> 7) | (tLP >> 14) | (tLP >> 21);
					pN += 8;
					pH += 8;
					pWC->trans[tileIndex][i] = tL;
				}
			}
			break;

			default:    // 1pixel = 2bit
			{
				u16 *tIRP = (u16*)&pWS->iRam[0x2000+(tileIndex<<4)];

				for (i = 0; i < 8; ++i)
				{
					tL = *tIRP++;
					tLP = (tL >> 7) & 0x0101;
					PHN(7, 0) = tLP | (tLP >> 7);
					tLP = (tL >> 6) & 0x0101;
					PHN(6, 1) = tLP | (tLP >> 7);
					tLP = (tL >> 5) & 0x0101;
					PHN(5, 2) = tLP | (tLP >> 7);
					tLP = (tL >> 4) & 0x0101;
					PHN(4, 3) = tLP | (tLP >> 7);
					tLP = (tL >> 3) & 0x0101;
					PHN(3, 4) = tLP | (tLP >> 7);
					tLP = (tL >> 2) & 0x0101;
					PHN(2, 5) = tLP | (tLP >> 7);
					tLP = (tL >> 1) & 0x0101;
					PHN(1, 6) = tLP | (tLP >> 7);
					tLP = (tL >> 0) & 0x0101;
					PHN(0, 7) = tLP | (tLP >> 7);
					pN += 8;
					pH += 8;
					pWC->trans[tileIndex][i] = tL;
				}
			}
			break;
		}
	}

	if (vFlip) line = 7 - line;

	if (pWC->trans[tileIndex][line] == 0) // Fila de tile transparente
	{
		return(NULL);
	}

	if (hFlip) return (&pWC->wsc_h[(tileIndex<<6)+(line<<3)]);
	else       return (&pWC->wsc_n[(tileIndex<<6)+(line<<3)]);
#undef PHN
}

//------------------------------------------------------------------------------
//-
//------------------------------------------------------------------------------
BYTE cpu_readport(BYTE port)
{
	switch (port)
	{
		case 0x02:
			return pWC->scanline_num[pWS->line];

		case 0x92: /* Noise Counter Shift Register */
		case 0x93: /* [make appropriate changes in channel 4 is enabled] */

			if (pWS->ioRam[0x90] & 0x08) // near random value
			{
				pWS->ioRam[port] += ((port & 1) << 1) + 1;
			}

			break;


			//-----------------------------------------------------------------------------
			/*    case 0x00: case 0x01:            case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
			      case 0x08: case 0x09: case 0x0A: case 0x0B: case 0x0C: case 0x0D: case 0x0E: case 0x0F:
			      case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
			      case 0x18: case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E: case 0x1F:
			      case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
			      case 0x28: case 0x29: case 0x2A: case 0x2B: case 0x2C: case 0x2D: case 0x2E: case 0x2F:
			      case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
			      case 0x38: case 0x39: case 0x3A: case 0x3B: case 0x3C: case 0x3D: case 0x3E: case 0x3F:
			      case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
			      case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4E: case 0x4F:
			      case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
			      case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5E: case 0x5F:
			      case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
			      case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6E: case 0x6F:
			      case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
			      case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F:
			      case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
			      case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D: case 0x8E: case 0x8F:
			      case 0x90: case 0x91: case 0x94: case 0x95: case 0x96: case 0x97:
			      case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9E: case 0x9F:
			      case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5: case 0xA6: case 0xA7: case 0xA8: case 0xA9: break;
			      case 0xAB: case 0xAC: case 0xAD: case 0xAE: case 0xAF: case 0xB0: case 0xB1: case 0xB2: break;
			      case 0xB4: case 0xB5: case 0xB6: case 0xB7: case 0xB8: case 0xB9: break;
			      case 0xBC: case 0xBD: case 0xBE: case 0xBF:// break;
			      case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC6: case 0xC7: case 0xC9: case 0xCC: case 0xCD: case 0xCE: case 0xCF:
			      case 0xD0: case 0xD1: case 0xD2: case 0xD3: case 0xD4: case 0xD5: case 0xD6: case 0xD7:
			      case 0xD8: case 0xD9: case 0xDA: case 0xDB: case 0xDC: case 0xDD: case 0xDE: case 0xDF:
			      case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5: case 0xE6: case 0xE7:
			      case 0xE8: case 0xE9: case 0xEA: case 0xEB: case 0xEC: case 0xED: case 0xEE: case 0xEF:
			        break;
			*/ //-----------------------------------------------------------------------------
		case 0xA0:
			return 0x87;
		case 0xAA:
			return 0xff;
		case 0xB3:

			if (pWS->ioRam[0xb3] < 0x80) return 0;

			if (pWS->ioRam[0xb3] < 0xc0) return 0x84;

			return 0xc4;

		case 0xBA: /* eeprom even byte read */
		{
			u32 w1 = ((((u16)pWS->ioRam[0xbd]) << 8) | ((u16)pWS->ioRam[0xbc]));
			w1 = (w1 << 1) & 0x3ff;
			return internalEeprom[w1];
		}

		case 0xBB: /* eeprom odd byte read */
		{
			u32 w1 = ((((u16)pWS->ioRam[0xbd]) << 8) | ((u16)pWS->ioRam[0xbc]));
			w1 = ((w1 << 1) + 1) & 0x3ff;
			return internalEeprom[w1];
		}

		case 0xC4:

			if (eEepromAddressMask)
			{
				u32 w1 = (((((WORD)pWS->ioRam[0xc7]) << 8) | ((WORD)pWS->ioRam[0xc6])) << 1) & (eEepromAddressMask);
				return pWS->romE2P[w1];
			}

			return 0xff;

		case 0xC5:

			if (eEepromAddressMask)
			{
				u32 w1 = (((((WORD)pWS->ioRam[0xc7]) << 8) | ((WORD)pWS->ioRam[0xc6])) << 1) & (eEepromAddressMask);
				return pWS->romE2P[w1+1];
			}

			return 0xff;

		case 0xC8:

			if (eEepromAddressMask)
			{
				// ack eeprom write
				if (pWS->ioRam[0xc8] & 0x20)
					return pWS->ioRam[0xc8] | 2;

				// ack eeprom read
				if (pWS->ioRam[0xc8] & 0x10)
					return pWS->ioRam[0xc8] | 1;

				// else ack both
				return pWS->ioRam[0xc8] | 3;
			}

		case 0xCA:
			return pWS->ioRam[0xca] | 0x80;

		case 0xCB:  // RTC data register

			if ( pWS->ioRam[0xca] == 0x15 )
			{
				switch (rtcDataRegisterReadCount)
				{
					case 0:
						++rtcDataRegisterReadCount;
						return 0; /* BCD : year + 2000 */
					case 1:
						++rtcDataRegisterReadCount;
						return 0; /* BCD : month       */
					case 2:
						++rtcDataRegisterReadCount;
						return 0; /* BCD : day         */
					case 3:
						++rtcDataRegisterReadCount;
						return 0; /* BCD : day of week */
					case 4:
						++rtcDataRegisterReadCount;
						return 0; /* BCD : hour        */
					case 5:
						++rtcDataRegisterReadCount;
						return 0; /* BCD : min         */
					case 6:
						rtcDataRegisterReadCount = 0;
						return 0; /* BCD : sec         */
				}
			}
			else
			{
				return (pWS->ioRam[0xcb] | 0x80);
			}

			break;

		case 0xF0:
		case 0xF1:
		case 0xF2:
		case 0xF3:
		case 0xF4:
		case 0xF5:
		case 0xF6:
		case 0xF7:
		case 0xF8:
		case 0xF9:
		case 0xFA:
		case 0xFB:
		case 0xFC:
		case 0xFD:
		case 0xFE:
		case 0xFF:
			return 0xd1; // [F0-FF used as the working]
	}

	return pWS->ioRam[port];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void cpu_writeport(DWORD port, BYTE value)
{
	u32 w1, ix;

	// PreSet Working
	switch (port)
	{
		case 0x02:
			return; /* READ ONLY */
		case 0x52: // Audio DMA
		case 0x90:
			Ch[0].on = value & 0x01;
			Ch[1].on = value & 0x02;
			Ch[2].on = value & 0x04;
			Ch[3].on = value & 0x08;
			VoiceOn   = value & 0x20;
			Swp.on    = value & 0x40;
			Noise.on  = value & 0x80;
			break;
		/*
		case 0x94: // Main Volume Control
			swan_sound(0, ((u32)pWS->line * 243925) >> 16);
			swan_sound(1, ((u32)pWS->line * 243925) >> 16);
			swan_sound(2, ((u32)pWS->line * 243925) >> 16);
			swan_sound(3, ((u32)pWS->line * 243925) >> 16);
			break;
		*/

		case 0x80:
		case 0x81:
			Ch[0].freq = *(unsigned short*)(pWS->ioRam + 0x80);
			break;
		case 0x88:
		    Ch[0].volL = (value >> 4) & 0x0F;
			Ch[0].volR = value & 0x0F;
			break;
			/*
			swan_sound(0, ((u32)pWS->line * 243925) >> 16);
			break;
		*/
		case 0x82:
		case 0x83:
			Ch[1].freq = *(unsigned short*)(pWS->ioRam + 0x82);
			break;		
		
		case 0x89:
		    Ch[1].volL = (value >> 4) & 0x0F;
			Ch[1].volR = value & 0x0F;
			break;		
		/*
			swan_sound(1, ((u32)pWS->line * 243925) >> 16);
			break;
		*/
		case 0x8c: /* Audio 3 Sweep value */
			Swp.step = (signed char)value;
			break;		
		case 0x8d: /* Audio 3 Sweep step  */
			Swp.time = (value + 1) << 5;
			break;		
			//pWS->sweep_upd = 1;
		case 0x84:
		case 0x85:
			Ch[2].freq = *(unsigned short*)(pWS->ioRam + 0x84);
			break;
		
		case 0x8a:
		    Ch[2].volL = (value >> 4) & 0x0F;
			Ch[2].volR = value & 0x0F;
			break;
		/*
			swan_sound(2, ((u32)pWS->line * 243925) >> 16);
			break;
		*/
		case 0x86:
		case 0x87:
			Ch[3].freq = *(unsigned short*)(pWS->ioRam + 0x86);
			break;		
		case 0x8b:
		    Ch[3].volL = (value >> 4) & 0x0F;
			Ch[3].volR = value & 0x0F;
			break;		
		case 0x8e: // Audio 4 Noise Control
			Noise.pattern = value & 0x07;
			break;
			/*
			swan_sound(3, ((u32)pWS->line * 243925) >> 16);
			break;
			*/
		case 0x8f:
			WaveMap = value << 6;
			u32 i;
			for (i = 0; i < 64; i++) {
				apuSetPData(WaveMap + i, pWS->iRam[WaveMap + i]);
			}
			break;		

		case 0x4e: /* DMA address <read only> */
		case 0x4f: /* DMA address <read only> */
			break;

		case 0xb6:

			if (value)
			{
				pWS->ioRam[0xb6] &= ~value;
			}

			return;
	}


	pWS->ioRam[port] = value;

	//-----------------------------------------------------------------------
	// Post Working
	//-----------------------------------------------------------------------
	switch (port)
	{
			/*
			      case 0x00: // Display Control
			      case 0x01: // Background Color
			      case 0x02: // Current Line
			      case 0x03: // Line Compare (for Interrupt)
			      case 0x04: //
			      case 0x05:
			      case 0x06:
			        break;
			*/
		case 0x07:
			bgBase = ((u32)(pWS->ioRam[7] & 0x0f)) << 11;
			fgBase = ((u32)(pWS->ioRam[7] & 0xf0)) <<  7;
			break;

		case 0x08:
		case 0x09:
		case 0x0A:
		case 0x0B:
		case 0x0C:
		case 0x0D:
		case 0x0E:
		case 0x0F:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			break;

		case 0xa4:
		case 0xa5: // HBLANK
		case 0xa6:
		case 0xa7: // VBLANK
			pWS->ioRam[port|0xf0] = value; // backup to unused area
			break;

		case 0xa8:
		case 0xa9:
			break;

		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
			pWC->mpalette[((port-0x1c)<<1)+0][0] = pWC->shades[15 & value][0];
			pWC->mpalette[((port-0x1c)<<1)+0][1] = pWC->shades[15 & value][1];
			pWC->mpalette[((port-0x1c)<<1)+0][2] = pWC->shades[15 & value][2];
			pWC->mpalette[((port-0x1c)<<1)+1][0] = pWC->shades[value >> 4][0];
			pWC->mpalette[((port-0x1c)<<1)+1][1] = pWC->shades[value >> 4][1];
			pWC->mpalette[((port-0x1c)<<1)+1][2] = pWC->shades[value >> 4][2];
			//pWC->pal_flag = -1;
			break;

		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
		case 0x28:
		case 0x29:
		case 0x2a:
		case 0x2b:
		case 0x2c:
		case 0x2d:
		case 0x2e:
		case 0x2f:
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:
		case 0x38:
		case 0x39:
		case 0x3a:
		case 0x3b:
		case 0x3c:
		case 0x3d:
		case 0x3e:
		case 0x3f:
			//pWC->pal_flag |= (1 << ((port - 0x20) >> 1));
			break;

		case 0x42:
		case 0x43:
			pWS->ioRam[port] &= 0x0f;
			break;

		case 0x48:	// DMA
			// bit 7 set to start dma transfer
			if (value & 0x80)
			{
				u32 dma_start = (((DWORD)pWS->ioRam[0x41]) << 8) | (((DWORD)pWS->ioRam[0x40])) | (((DWORD)pWS->ioRam[0x42]) << 16);
				u32 dma_end   = (((DWORD)pWS->ioRam[0x45]) << 8) | (((DWORD)pWS->ioRam[0x44])) | (((DWORD)pWS->ioRam[0x43]) << 16);
				u32 dma_size  = (((DWORD)pWS->ioRam[0x47]) << 8) | (((DWORD)pWS->ioRam[0x46]));

				dma_end &= 0x000fffff; /* naruto bugfix */

				for (ix = 0; ix < dma_size; ++ix)
				{
					cpu_writemem20(dma_end, cpu_readmem20(dma_start));
					++dma_end;
					++dma_start;
				}

				pWS->ioRam[0x47] = 0;
				pWS->ioRam[0x46] = 0;
				pWS->ioRam[0x41] = (BYTE)(dma_start >> 8);
				pWS->ioRam[0x40] = (BYTE)(dma_start & 0xff);
				pWS->ioRam[0x45] = (BYTE)(dma_end >> 8);
				pWS->ioRam[0x44] = (BYTE)(dma_end & 0xff);
				pWS->ioRam[0x48] = 0;
			}

			break;
			/*
			      case 0x4a: // sound DMA source address
			      case 0x4b:
			      case 0x4c: // DMA source memory segment bank
			      case 0x4d:
			      case 0x4e: // DMA Transfer size (in bytes)
			      case 0x4f: // ^^^
			      case 0x50:
			      case 0x51:
			*/
		case 0x52: /* bit  7 = 1  -> DMA start */
			ws_voice_dma();
			break;

		case 0x60:
			ws_gpu_changeVideoMode(value);
			return;
			/*
			      case 0x80: // Audio 1 Freq : low
			      case 0x81: // ^^^          : high
			      case 0x82: // Audio 2 freq : low
			      case 0x83: // ^^^          : high
			      case 0x84: // Audio 3 Freq : low
			      case 0x85: // ^^^          : high
			      case 0x86: // Audio 4 freq : low
			      case 0x87: // ^^^          : high
			      case 0x88: // Audio 1 volume
			      case 0x89: // Audio 2 volume
			      case 0x8a: // Audio 3 volume
			      case 0x8b: // Audio 4 volume
			      case 0x8c: // Audio 3 Sweep Value
			      case 0x8d: // Audio 3 Sweep Step
			        break;
			*/
		case 0x8e: /* Audio 4 Noise Control*/
			// Counter Enable
			pWS->noise_c = (value & 7) << 9;

			// FF1 Noise fix
			if (value & 0x10)
			{
				pWS->noise_k = 0;
				pWS->noise_r = 0;
				pWS->noise_v = 0x51f631e4;
			}

			break;

		case 0x91: /* Audio Output    */
			pWS->ioRam[0x91] |= 0x80; // stereo status
			break;

#if 0
		case 0x8f: // Sample Location
			break;
		case 0x90: /* Audio Control   */
			break;

		case 0x92: /* Noise Counter Shift Register : low?  */
		case 0x93: /* ^^^                          : high? */
			//			value = value;
			break;

		case 0x94: /* Volume 4bit */
			//        ws_audio_port_write(port,value);
			break;
#endif

		case 0xb5: // Controls

			switch (value & 0xf0)
			{
				case 0x10:
					value = 0x10 | (0x0f & (ws_key >> 8));
					break; // read vertical
				case 0x20:
					value = 0x20 | (0x0f & (ws_key >> 4));
					break; // read horizontal
				case 0x40:
					value = 0x40 | (0x0f & (ws_key)   );
					break; // read buttons
				default:
					value &= 0xf0;
					break;
			}

			pWS->ioRam[0xb5] = value;

			break;

		case 0xba:
			w1 = (((((WORD)pWS->ioRam[0xbd]) << 8) | ((WORD)pWS->ioRam[0xbc])));
			w1 = (w1 << 1) & 0x3ff;
			internalEeprom[w1] = value;
			return;

		case 0xbb:
			w1 = (((((WORD)pWS->ioRam[0xbd]) << 8) | ((WORD)pWS->ioRam[0xbc])));
			w1 = ((w1 << 1) + 1) & 0x3ff;
			internalEeprom[w1] = value;
			return;

		case 0xbe: // EEPROM

			if (value & 0x20)
			{
				value |= 0x02;
			}
			else if (value & 0x10)
			{
				value |= 0x01;
			}
			else
			{
				value |= 0x03;
			}

			pWS->ioRam[0xbe] = value;
			break;

		case 0xc0:
		{
			u32 romBank, bank;

			for (bank = 4; bank < 16; ++bank)
			{
				romBank = (256 - (((value & 0xf) << 4) | (bank & 0xf)));
				pWsRomMap[bank] = &ws_RomAddr[ws_RomSize-(romBank<<16)];
			}

			// Read Port [so that the same process must have done at the Write Port]
			pWS->ioRam[0xc0] = (pWS->ioRam[0xc0] & 0x0f) | 0x20;
		}
		break;

		case 0xc1: // SRAM Bank Change

			switch (sramAddressMask)
			{
				case 0x1ffff:
					pWsRomMap[0x01] = &pWS->romRam[(value&1)<<16];
					break;
				case 0x3ffff:
					pWsRomMap[0x01] = &pWS->romRam[(value&3)<<16];
					break;
				default:
					pWsRomMap[0x01] = &pWS->romRam[0];
					break;
			}

			break;

		case 0xc2:
			pWsRomMap[0x02] = &ws_RomAddr[ ((value&((ws_RomSize>>16)-1))<<16) ];
			break;

		case 0xc3:
			pWsRomMap[0x03] = &ws_RomAddr[ ((value&((ws_RomSize>>16)-1))<<16) ];
			break;

		case 0xc4:
			w1 = (((((WORD)pWS->ioRam[0xc7]) << 8) | ((WORD)pWS->ioRam[0xc6])) << 1)&eEepromAddressMask;
			pWS->romE2P[w1] = value;
			return;

		case 0xc5:
			w1 = (((((WORD)pWS->ioRam[0xc7]) << 8) | ((WORD)pWS->ioRam[0xc6])) << 1)&eEepromAddressMask;
			pWS->romE2P[w1+1] = value;
			return;

		case 0xca:

			if (value == 0x15)
			{
				rtcDataRegisterReadCount = 0;
			}

			break;

		default:
			break;
	}
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void cpu_writeport2(DWORD port, WORD value)
{
	cpu_writeport(port  , value);
	cpu_writeport(port + 1, value >> 8);
}



//------------------------------------------------------------------------------
// 0x4000-0x8000 : TILE BANK 0
// 0x8000-0xC000 : TILE BANK 1
//------------------------------------------------------------------------------
void cpu_writemem20(DWORD addr, BYTE value)
{
	u16 offset;
	u32 bank = addr >> 16;

	/* 0 - RAM - 16 KB (WS) / 64 KB (WSC) internal RAM */
	if (bank == 0)
	{
		offset = addr;
		
		if(!((addr - WaveMap) & 0xFFC0))
		{
			apuSetPData(addr & 0x003F, value);
			pWS->iRam[addr & 0xFFFF] = value;
		}
		
		if (pWS->iRam[offset] != value)
		{

			if (offset < 0x02000);
			else if (offset < 0x04000)
			{
				pWC->modify[((offset>>4)&0x1ff)] = 1;
			}
			else if (offset < 0x0C000)
			{
				pWC->modify[(offset-0x4000)>>5] = 1;
			}
			else

				//			if(offset<0x08000) { pWC->modify[((offset>>5)&0x1ff)+0x000]=1; } else
				//			if(offset<0x0C000) { pWC->modify[((offset>>5)&0x1ff)+0x200]=1; } else
				if (offset < 0x0FE00);
				else
				{
					u16 c, adr, pal;
					pWS->iRam[offset] = value;
					adr = offset & 0x0fffe;
					pal = (adr - 0xfe00) >> 1;
					c = ((u16)(pWS->iRam[adr+1] << 8) | pWS->iRam[adr]);

					u32 r, g, b;
					r = (c >> 8) & 0xf;
					g = (c >> 4) & 0xf;
					b = c & 0xf;

					r = r << 1;
					g = g << 1;
					b = b << 1;

					if (r & 0xFFFFFFE0) r = 31;

					if (g & 0xFFFFFFE0) g = 31;

					if (b & 0xFFFFFFE0) b = 31;

					pWC->palette[pal][0] = r << 3;
					pWC->palette[pal][1] = g << 3;
					pWC->palette[pal][2] = b << 3;
					pWC->palette[pal][3] = 1;
				}

			pWS->iRam[offset] = value;
		}
	}
	else if (bank == 1)
	{
		offset = addr;
		*(pWsRomMap[0x01] + offset) = value;
	}

	// other banks are read-only
}

//------------------------------------------------------------------------------
// Audio DMA work
//------------------------------------------------------------------------------
static void ws_voice_dma(void)
{
	if (pWS->ioRam[0x52] & 0x80)
	{
		u32 adr = (u32)((u32)pWS->ioRam[0x4c] << 16) | ((u32)pWS->ioRam[0x4b] << 8) | pWS->ioRam[0x4a];
		u32 len = (u32)((u32)pWS->ioRam[0x4f] << 8 ) | ((u32)pWS->ioRam[0x4e]);

		pWS->ioRam[0x90] |= 0x22;

		cpu_writeport(0x89, cpu_readmem20(adr));

		++adr;
		--len;

		/* Sound DMA[in the bank if it stopped over] */
		if ( (adr >> 16) != pWS->ioRam[0x4c] )
		{
			--adr;
			len = 0;
		}

#if 1 /* [oswan implementation] */

		if (len < 32)
		{
			len = 0;
		}

#endif

		//      pWS->ioRam[0x4C]=(u8)((adr>>16)&0xFF); /* Bank [is prohibited beyond] */
		pWS->ioRam[0x4B] = (u8)((adr >> 8) & 0xFF);
		pWS->ioRam[0x4A] = (u8)(adr & 0xFF);
		pWS->ioRam[0x4F] = (u8)((len >> 8) & 0xFF);
		pWS->ioRam[0x4E] = (u8)(len & 0xFF);

		if (len == 0)
		{
			pWS->ioRam[0x52] &= ~0x80;
#if 0 // Terrors2 [not fix the sound]
			pWS->ioRam[0x90] &= ~0x22;
#endif
		}
	}
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

u32 saveState(void)
{
	char name[0x100];
	FILE *fd = NULL;
	sprintf (name, "saves/%s.st%u", cart.game_name, stateSlot);
#ifdef DEBUG
	printf ("Saving in: %s\n", name);
#endif
	fd = fopen(name, "wb");

	if (!fd) return 0;

	u32 size, len = 0;
	void* pCpu = nec_getRegPtr(&len);

	memcpy(pWS->tag_iram, "*RAM", 4);
	memcpy(pWS->tag_ioram, "*I/O", 4);
	memcpy(pWS->tag_spr,  "*SPR", 4);
	memcpy(pWS->tag_snd,  "*SND", 4);
	memcpy(pWS->tag_eE2P, "@E2P", 4);
	memcpy(pWS->tag_eRAM, "@RAM", 4);

	size = sizeof(*pWS)
	       - sizeof(pWS->romE2P) - sizeof(pWS->tag_eE2P)
	       - sizeof(pWS->romRam) - sizeof(pWS->tag_eRAM);

	fwrite(pCpu, 1, len, fd);
	fwrite(pWS, 1, size, fd);

	if (sramAddressMask)
	{
		fwrite(pWS->tag_eRAM, 1, sramAddressMask + 1 + sizeof(pWS->tag_eRAM), fd);
	}
	else if (eEepromAddressMask)
	{
		fwrite(pWS->tag_eE2P, 1, eEepromAddressMask + 1 + sizeof(pWS->tag_eE2P), fd);
	}

	fclose(fd);
	sync();
	
	sprintf (sMessage, "Slot %d Saved!", stateSlot);
	showMessage = MESSAGE_DELAY;

	return 1;
}

/*
static int ws_save(int fd)
{
	int size, len = 0;
	void* pCpu = nec_getRegPtr(&len);

	memcpy(pWS->tag_iram, "*RAM", 4);
	memcpy(pWS->tag_ioram, "*I/O", 4);
	memcpy(pWS->tag_spr,  "*SPR", 4);
	memcpy(pWS->tag_snd,  "*SND", 4);
	memcpy(pWS->tag_eE2P, "@E2P", 4);
	memcpy(pWS->tag_eRAM, "@RAM", 4);

	size = sizeof(*pWS)
	       - sizeof(pWS->romE2P) - sizeof(pWS->tag_eE2P)
	       - sizeof(pWS->romRam) - sizeof(pWS->tag_eRAM);

	fwrite(pCpu, 1, len, fd);
	fwrite(pWS, 1, size, fd);

	if (sramAddressMask)
	{
		fwrite(pWS->tag_eRAM, 1, sramAddressMask + 1 + sizeof(pWS->tag_eRAM), fd);
	}
	else if (eEepromAddressMask)
	{
		fwrite(pWS->tag_eE2P, 1, eEepromAddressMask + 1 + sizeof(pWS->tag_eE2P), fd);
	}

	return 1;
}
*/

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
u32 loadState(void)
{
	char name[0x100];
	FILE *fd = NULL;
	sprintf (name, "saves/%s.st%u", cart.game_name, stateSlot);
#ifdef DEBUG
	printf ("Loading from: %s\n", name);
#endif
	fd = fopen(name, "rb");

	if (!fd) return 0;

	u32 i, size, len = 0;
	void *pCpu = nec_getRegPtr(&len);

	size = sizeof(*pWS)
	       - sizeof(pWS->romE2P) - sizeof(pWS->tag_eE2P)
	       - sizeof(pWS->romRam) - sizeof(pWS->tag_eRAM);

	fread(pCpu, 1, len, fd);
	fread(pWS, 1, size, fd);

	if (sramAddressMask)
	{
		fread(pWS->tag_eRAM, 1, sramAddressMask + 1, fd);
	}
	else if (eEepromAddressMask)
	{
		fread(pWS->tag_eE2P, 1, eEepromAddressMask + 1, fd);
	}

	// Tile Cache Clear
	ws_gpu_clearCache();
	//memset(waveTable, 0, sizeof(waveTable));

	// ROM Map Update
	cpu_writeport(0xc0, pWS->ioRam[0xc0]);
	cpu_writeport(0xc2, pWS->ioRam[0xc2]);
	cpu_writeport(0xc3, pWS->ioRam[0xc3]);

	// Update Color Palette
	if ( pWS->videoMode >= 4 )
	{
		byte v;

		for (i = 0xfe00; i < 0x10000; ++i)
		{
			v = cpu_readmem20(i);
			cpu_writemem20(i, 0);
			cpu_writemem20(i, v);
		}
	}
	else
	{
		for (i = 0x1c; i <= 0x1f; ++i)
		{
			cpu_writeport(i, pWS->ioRam[i]);
		}
	}

	fclose(fd);
	
	sprintf (sMessage, "Slot %d Loaded!", stateSlot);
	showMessage = MESSAGE_DELAY;

	return 1;
}

/*
static int ws_load(int fd)
{
	int i, size, len = 0;
	void *pCpu = nec_getRegPtr(&len);

	size = sizeof(*pWS)
	       - sizeof(pWS->romE2P) - sizeof(pWS->tag_eE2P)
	       - sizeof(pWS->romRam) - sizeof(pWS->tag_eRAM);

	fread(pCpu, 1, len, fd);
	fread(pWS, 1, size, fd);

	if (sramAddressMask)
	{
		fread(pWS->tag_eRAM, 1, sramAddressMask + 1, fd);
	}
	else if (eEepromAddressMask)
	{
		fread(pWS->tag_eE2P, 1, eEepromAddressMask + 1, fd);
	}

	// Tile Cache Clear
	ws_gpu_clearCache();
	//memset(waveTable, 0, sizeof(waveTable));

	// ROM Map Update
	cpu_writeport(0xc0, pWS->ioRam[0xc0]);
	cpu_writeport(0xc2, pWS->ioRam[0xc2]);
	cpu_writeport(0xc3, pWS->ioRam[0xc3]);

	// Update Color Palette
	if ( pWS->videoMode >= 4 )
	{
		byte v;

		for (i = 0xfe00; i < 0x10000; i++)
		{
			v = cpu_readmem20(i);
			cpu_writemem20(i, 0);
			cpu_writemem20(i, v);
		}
	}
	else
	{
		for (i = 0x1c; i <= 0x1f; i++)
		{
			cpu_writeport(i, pWS->ioRam[i]);
		}
	}

	return 1;
}
*/

static void saveConfig(void)
{
	char work[512];
	FILE *fd = NULL;
	
	// EEPROM(Internal)
	sprintf (work, "saves/%s", SWAN_INTERNAL_EEP);
#ifdef DEBUG
	printf ("Saving internal EEPROM to: %s\n", work);
#endif

	if (fd = fopen(work, "wb"))
	{
		fwrite(internalEeprom, 1, sizeof(internalEeprom), fd);
		fclose(fd);
	}
	
	// ROM [SRAM or EEPROM]
	sprintf (work, "saves/%s.srm", cart.game_name);
	if (sramAddressMask)
	{
#ifdef DEBUG
		printf ("Saving SRAM to: %s\n", work);
#endif	
		if (fd = fopen(work, "wb"))
		{
			fwrite(pWS->romRam, 1, sramAddressMask + 1, fd);
			fclose(fd);
		}
	}
	else if (eEepromAddressMask)
	{
#ifdef DEBUG
		printf ("Saving EEPROM to: %s\n", work);
#endif	
		if (fd = fopen(work, "wb"))
		{
			fwrite(pWS->romE2P, 1, eEepromAddressMask + 1, fd);
			fclose(fd);
		}
	}
}	

static void loadConfig(void)
{
	char work[512];
	FILE *fd = NULL;
	
	// EEPROM(Internal)
	sprintf (work, "saves/%s", SWAN_INTERNAL_EEP);
#ifdef DEBUG
	printf ("Loading internal EEPROM from: %s\n", work);
#endif

	if (fd = fopen(work, "rb"))
	{
		fread(internalEeprom, 1, sizeof(internalEeprom), fd);
		fclose(fd);
	}
	
	// ROM [SRAM or EEPROM]
	sprintf (work, "saves/%s.srm", cart.game_name);
	if (sramAddressMask)
	{
#ifdef DEBUG
		printf ("Loading SRAM from: %s\n", work);
#endif	
		if (fd = fopen(work, "rb"))
		{
			fread(pWS->romRam, 1, sramAddressMask + 1, fd);
			fclose(fd);
		}
	}
	else if (eEepromAddressMask)
	{
#ifdef DEBUG
		printf ("Loading EEPROM from: %s\n", work);
#endif	
		if (fd = fopen(work, "rb"))
		{
			fread(pWS->romE2P, 1, eEepromAddressMask + 1, fd);
			fclose(fd);
		}
	}
}

//------------------------------------------------------------------------------
//- Background fill
//------------------------------------------------------------------------------
inline static void drawLineFillBg(u8 * pFb, u32 beg, u32 end, u8 bgc)
{
	//u32 i, line;
	//u8* pFb;

	//for (line = beg; line < end; line++)
	for (; beg < end; ++beg, pFb += 320)
	{
		//pFb = FBPTR(beg);

		/*
		for (i = 0; i < 224; i++)
		{
			*pFb++ = bgc;
		}
		*/
		memset(pFb, bgc, 224);
		//memset32(pFb, bgc, 224 / 4);
	}
}

//------------------------------------------------------------------------------
// - [Tile (attribute background.)]
// - [So that transparent BG BG is in the first draw must fill in color.]
// - [From the paint to draw more solid transparent BG = BG BG rewritten to draw and color.]
//------------------------------------------------------------------------------
static void drawLineBg(u8 * pFb, u32 beg, u32 end, u8 bgc)
{
	u8 * pFb2;
	u8 * px;
	u32 curTile;
	u32 tInfo;
	u16 *pTileRam;
	u8* wsTR;

	//u8 pa0;
	u32 index;

	//u16 mInfo = (pWS->videoMode > 4) ? 1 : 0;
	u32 mInfo = pWS->videoMode;
	u32 scrX = pWS->ioRam[0x10];

	u32 scrY = (pWS->ioRam[0x11] + beg) & 0xff;
	register u32 ctemp;

	px = pFb + 224;
	pFb -= (scrX & 7);

	for (; beg < end; ++beg, pFb += 320, px += 320)
	{
		curTile = (scrX >> 3);
		pTileRam = (u16*)(pWS->iRam + bgBase + ((scrY & 0xf8) << 3));

		pFb2 = pFb;

		while (pFb2 < px)
		{
			tInfo = pTileRam[(curTile++)&0x1f];

			if (mInfo == 0) ws_gpu_palette(PAL_TILE(tInfo));
			index = (PAL_TILE(tInfo)) << 4;

			if ((mInfo & 0x6) | (tInfo & 0x800)) // El color 0 de cada paleta es transparente
			{
				if ( (wsTR = getTileRow(tInfo, scrY & 7)) ) // Fila de tile NO transparente
				{
					REPx8
					(
						ctemp = *wsTR++;
						*pFb2++ = (ctemp) ? index + ctemp : bgc
					);
				}
				else // Fila de tile transparente
				{
					REPx8( *pFb2++ = bgc );
				}
			}
			else // NO hay colores transparentes en la paleta (WSM)
			{
				if ( (wsTR = getTileRow(tInfo, scrY & 7)) ) // Fila de tile NO transparente
				{
					REPx8( *pFb2++ = index + (*wsTR++) );
				}
				else // Fila de tile con color = 0
				{
					REPx8( *pFb2++ = index );
				}
			}
		}

		scrY = (scrY + 1) & 0xff;
	}
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
static void tile_draw_trans(u8* pFb, u32 tInfo, u32 offsetY)
{
	u8 * wsTR = getTileRow(tInfo, offsetY);
	u32 mInfo = pWS->videoMode;

	if (mInfo == 0) ws_gpu_palette(PAL_TILE(tInfo));

	u32 index = (PAL_TILE(tInfo)) << 4;

	if ((mInfo & 0x6) | (tInfo & 0x800)) // Hay transparencia (WSC/WSM)
	{
		if ( wsTR ) // El color NO es transparente
		{
			REPx8(

			    if (*wsTR)
				{
					//*pFb = BGFGMASK(wsPA[*wsTR]);
					*pFb = index + (*wsTR);
				};
				++pFb; ++wsTR;
			);
		}
	}
	else // Sin transparencia (WSM)
	{
		if ( wsTR ) // La fila del tile tiene color
		{
			REPx8( *pFb++ = index + (*wsTR++) );
		}
		else // La fila del tile es transparente
		{
			REPx8( *pFb++ = index );
		}
	}
}

//------------------------------------------------------------------------------
//
// Foreground TILE draw (with window clip(inside/outside))
//
// [No window: normal display]
// [In window: divided into three types]
// [(1) hidden outside the window]
// [(2) all in the window display]
// [(1) at the border less window]
//------------------------------------------------------------------------------
static void drawLineFg(u8 * pFb, u32 beg, u32 end)
{
	s32 px;
	//u32 line;
	u16* ws_fgScrollRamBase;
	u8 *pFb2;
	//PIXEL_FORMAT *wsPA;
	u32 bgWin_y0 = pWS->ioRam[0x09];
	u32 bgWin_y1 = pWS->ioRam[0x0b];
	u32 scrX     = pWS->ioRam[0x12]; // scrollX FG Layer
	s32 offsetX  = scrX & 0x07;
	u32 tInfo;
	u8* wsTR;
	u32 bgWin_x0 = pWS->ioRam[0x08];
	u32 bgWin_x1 = pWS->ioRam[0x0a];
	//u8  side, fWin, wMode;
	u32  side, fWin, wMode;
	int scrY, curTile, offsetY;

	//pWC->scroll |= scrX;

	fWin = pWS->ioRam[0x00] & BGFG_CLIP_MASK;
	side = (pWS->ioRam[0x00] & BGFG_CLIP_SIDE) ? 1 : 0;

	//printf ("Inicio -> Begin: %u - End: %u - pFb: %u\n", beg, end, pFb-screen8);

	//
	// [X is really necessary check-axis clipping]
	//
	if ( BGFG_CLIP_ENABLE & fWin )
	{
		if ( (bgWin_x0 == bgWin_x1) || (bgWin_x0 == 0 && bgWin_x1 >= 224) )
		{
			bgWin_x0 = 0;
			bgWin_x1 = 224;
		}

		// [Inside view]
		if ( BGFG_CLIP_IN == fWin )
		{
			if (beg < bgWin_y0)
			{
				pFb += (bgWin_y0 - beg) * 320;
				beg = bgWin_y0;
			}

			if (end > bgWin_y1) end = bgWin_y1+1;
		}
	}



	//printf ("Ventana -> Begin: %u - End: %u - pFb: %u\n", beg, end, pFb-screen8);
	for (; beg < end; ++beg, pFb += 320)
	{
		//printf("Line: %u - pFb: %u\n", line, pFb-screen8);
		wMode = fWin;

		//
		// [If Kurippingumodo ON, Y-axis clipping examine the situation]
		//
		if ( BGFG_CLIP_ENABLE & wMode )  // CLIPPING : ON
		{

			// YCLIPPING CHECK
			if (side == 0) /* SHOW INSIDE */
			{
				if ( !BETWEEN(beg, bgWin_y0, bgWin_y1) )
				{
					// [window hidden inside the outer axis Y]
					continue;
				}
				/*
				else
				{
					// [Inner window inside Y ? X axis is the axis through a window treatment]
				}
				*/
			}
			else          /* SHOW OUTSIDE */
			{
				if ( !BETWEEN(beg, bgWin_y0, bgWin_y1) )
				{
					// [window windowless outer lateral axis Y]
					wMode = 0; // [Y axis is outside the window because no window]
				}
				/*
				else
				{
					// [Outside window, Y ? X is an inner shaft axis through the clipping process]
				}
				*/
			}
		}

		// Setup Values
		//pFb = FBPTR(line) - offsetX;
		pFb2 = pFb - offsetX;
		scrY    = ((s32)pWS->ioRam[0x13] + beg) & 0xff;
		curTile = (scrX >> 3);
		offsetY = scrY & 0x07;

		//if (offsetY) printf ("ScrollY: %d\n", offsetY);

		ws_fgScrollRamBase = (u16*)(pWS->iRam + fgBase + ((scrY & 0xfff8) << 3));

		//
		// Disable Window
		//
		if ((wMode & 0x20) == 0)
		{
			for (px = -offsetX; px < 224; px += 8)
			{
				tInfo = ws_fgScrollRamBase[curTile&0x1f];
				++curTile;
				tile_draw_trans(pFb2, tInfo, offsetY);
				pFb2 += 8;
			}
		}
		else   // foreground layer displayed with Window
		{
			u32 fx1, fx2;

			// [The window handle is actually related to]
			// [X is limited to plain tiles up to 2 axes]
			//
			// [[In] drawing the drawing pattern] [No 2]
			// [Clip [right] [clip] [clip left] left three patterns]

			for (px = -offsetX; px < 224; ++curTile)
			//for (; px < 217; curTile++)
			{
				fx1 = side ^ BETWEEN(px  , bgWin_x0, bgWin_x1);
				fx2 = side ^ BETWEEN(px + 7, bgWin_x0, bgWin_x1);

				// [Hide all areas]
				if ( !fx1 && !fx2 )
				{
					pFb2 += 8;
					px += 8;
					continue;
				}

				tInfo = ws_fgScrollRamBase[curTile&0x1f];
				wsTR = getTileRow(tInfo, offsetY);
				//wsPA = ws_gpu_palette(PAL_TILE(tInfo));
				u32 mInfo = pWS->videoMode;
				if (mInfo == 0) ws_gpu_palette(PAL_TILE(tInfo));

				u32 index = (PAL_TILE(tInfo)) << 4; // [Puck]

				// [Total display area]
				if ( fx1 && fx2 )
				{
					tile_draw_trans(pFb2, tInfo, offsetY);
					pFb2 += 8;
					px += 8;
				}
				// [Part display area]
				else
				{
					if ((mInfo & 0x6) | (tInfo & 0x800))
					{
						// [Transparent decision]
						if (wsTR)  // [In Decision]
						{
							REPx8(

							    if (*wsTR)
								{
									if ( (side ^ BETWEEN(px, bgWin_x0, bgWin_x1)) )
									{
										//*pFb = BGWNMASK(wsPA[*wsTR]);
										*pFb2 = index + (*wsTR);
									}
									else
									{
										/* BackBG [without, can fill in here] */
									}
								}
								++wsTR;
								++pFb2;
								++px;
							);
						}
						else      // [clear confirmation]
						{
							pFb2 += 8;
							px += 8;
						}
					}
					else
					{
						// [Fill]
						if (wsTR)  // [In a transparent decision]
						{
							REPx8(

							    if ( side ^ BETWEEN(px, bgWin_x0, bgWin_x1) )
								{
								//*pFb = BGWNMASK(wsPA[*wsTR]);
									*pFb2 = index + (*wsTR);
								}
								++wsTR;
								++pFb2;
								++px;
							);
						}
						else     // [fixed palette 0]
						{
							REPx8(

							    if ( side ^ BETWEEN(px, bgWin_x0, bgWin_x1) )
								{
									//*pFb = BGWNMASK(wsPA[0]);
									*pFb2 = index;
								}
								++pFb2;
								++px;
							);
						}
					}
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
static void drawLineSprites(u8 * pFb, u32 beg, u32 end, u32 priority)
{
	u32 i;
	s32 j, x, y;
	u8 * wsTR;
	//PIXEL_FORMAT * wsPA;
	//u8 * pFb;
	u32* pSpRam, spr, t, p;
	u32 outside = 0;
	u32 xbet, ybet;
	//u32 line;

	u32 spWin_x0 = pWS->ioRam[0x0c];
	u32 spWin_y0 = pWS->ioRam[0x0d];
	u32 spWin_x1 = pWS->ioRam[0x0e];
	u32 spWin_y1 = pWS->ioRam[0x0f];
	//byte spWin   = pWS->ioRam[0x00] & 0x08; // Sprite Window Enable?
	u32 spWin   = pWS->ioRam[0x00] & 0x08; // Sprite Window Enable?

	if ( spWin_x0 == spWin_x1)
	{
		//spWin &= ~0x08;
		spWin = 0;
	}

	// [No window = (0,0) - (223,143) in terms of drawing the inside of the window]
	if (!spWin)
	{
		spWin_x0 = 0;
		spWin_y0 = 0;
		spWin_x1 = 224;
		spWin_y1 = 144;
	}

	//for (line = beg; line < end; line++, pFb += 320)
	for (; beg < end; beg++, pFb += 320)
	{
		//pFb = FBPTR(line);

		/* [Y coordinates are within a flag that indicates] */
		ybet = BETWEEN(beg, spWin_y0, spWin_y1);

		pSpRam = &pWS->spTable[pWS->spTableCnt-1];

		//for (i = 0; i < pWS->spTableCnt; i++)
		for (i = pWS->spTableCnt; i != 0; i--)
		{
			spr = *pSpRam--;

			if ((spr & 0x2000) != priority) continue;

			x = (spr >> 24) & 0x0ff;
			y = (spr >> 16) & 0x0ff;
			t =  spr      & 0x1ff;
			p = ((spr & 0xe00) >> 9) + 8;

			// cygne [try to adjust what's]
			if (y > (144 - 1 + 7))
			{
				y = (signed char)((unsigned char)y);
			}

			if ((y + 8) <= (s32)beg) continue;

			if ( y   > (s32)beg) continue;

			if (x >= (224 - 1 + 7))
			{
				x = (signed char)((unsigned char)x);
			}

			if (x <= -8) continue;

			if ((y >= 144) || (x >= 224))  continue;

			outside = (spr & 0x1000) && (spWin);

			if ( (wsTR = getTileRow(spr&~0x2000, (beg - y) & 0x07)) )
			{
				//wsPA = ws_gpu_palette(p);
				u32 mInfo = pWS->videoMode;
				if (mInfo == 0) ws_gpu_palette(p);
				u32 index = (p) << 4;

				if ((mInfo & 0x6) | (p & 0x04))
				{
					// [(See conditions)]
					// if(!outside &&  BETWEEN) true
					// if(outside  && !BETWEEN) true
					// [You better use the XOR]
					for (j = x; j < (x + 8); ++j, ++wsTR)
					{
						if (*wsTR)
						{
							xbet = BETWEEN(j, spWin_x0, spWin_x1);

							if ( outside ^ (xbet && ybet) )
							{
								//pFb[j] = (pFb[j] & DRAW_MASK) | SPMASK(wsPA[*wsTR]);
								pFb[j] = index + (*wsTR);
							}
						}
					}
				}
				else
				{
					for (j = x; j < (x + 8); ++j, ++wsTR)
					{
						xbet = BETWEEN(j, spWin_x0, spWin_x1);

						if ( outside ^ (xbet && ybet) )
						{
							//pFb[j] = SPMASK(wsPA[*wsTR]);
							pFb[j] = index + (*wsTR);
						}
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// [Part of it could increase the speed]
// [(1) BackBG = OFF, BackFG = ON in the case, BackFill BackFG not call the Fill color to transparent]
// [(2) BackFG pixel fill> BackBG to optimize the case of pixel fill]
//
//
// #define IS_BGBG()     (pWS->ioRam[0x00]&0x01)
// #define IS_BGFG()     (pWS->ioRam[0x00]&0x02)
// #define IS_SPRT()     (pWS->ioRam[0x00]&0x04)
//
//-----------------------------------------------------------------------------
static void drawLine(u32 beg, u32 end)
{
	// Scanline Video Processing
	u8 bg;
	u8 * pFb;

	//printf("Start drawLine -> beg: %u - end: %u\n", beg, end);

	if (pWS->videoMode)
	{
		//bg = pWC->system_c_pal[pWS->ioRam[1]];
		//bg = pWS->ioRam[1];
		u8 bgc = pWS->ioRam[1];
		pWC->palette[240][0] = pWC->palette[bgc][0];
		pWC->palette[240][1] = pWC->palette[bgc][1];
		pWC->palette[240][2] = pWC->palette[bgc][2];
		pWC->palette[240][3] = 1;
		bg = 240;
	}
	else
	{
		//bg = pWC->system_m_pal[pWS->ioRam[1] & 7];
		u8 bgc = pWS->ioRam[1] & 7;
		pWC->palette[255][0] = pWC->palette[bgc][0];
		pWC->palette[255][1] = pWC->palette[bgc][1];
		pWC->palette[255][2] = pWC->palette[bgc][2];
		pWC->palette[255][3] = 1;
		bg = 255;
	}

	//if (bg % 16 == 0) printf ("Background color: %.3u -> R: %.3u, G: %.3u, B: %.3u\n", bg, pWC->palette[bg][0], pWC->palette[bg][1], pWC->palette[bg][2]);

	if (!pWC->rotated && !pWC->fullscreen) pFb = FBPTR(beg);
	else pFb = FBPTRTEMP(beg);

#ifdef DEBUG
	if (showBGBG)
	{
#endif		
		if (IS_BGBG()) drawLineBg(pFb, beg, end, bg);
		else          drawLineFillBg(pFb, beg, end, bg);
#ifdef DEBUG		
	}
	else
		drawLineFillBg(pFb, beg, end, bg);
#endif		

#ifdef DEBUG
	if (showSptr)
#endif	
		if (IS_SPRT()) drawLineSprites(pFb, beg, end, MIDDLE_SPRITE);

#ifdef DEBUG
	if (showBGFG)
#endif	
		if (IS_BGFG()) drawLineFg(pFb, beg, end);

#ifdef DEBUG
	if (showSptr)
#endif	
		if (IS_SPRT()) drawLineSprites(pFb, beg, end, TOP_SPRITE);


	if (pWC->rotated)
	{
		u32 i;
		for (; beg < end; ++beg, pFb += 320)
		{
			for (i = 0; i < 224; ++i)
			{
				screen8[(((232<<8)+(232<<6))+beg+88)-((i<<8)+(i<<6))] = pFb[i];
			}
		}

		return;
	}
	
	if (pWC->fullscreen)
	{
		u32 i;
		for (; beg < end; ++beg, pFb += 320)
		{
			unsigned char * p = &screen8[320 * (beg + beg / 2 + 12)];
			
			int i;
			for (i = 5; i < 218; i++)
			{
				*p++ = pFb[i];
				if (i % 2 == 0) *p++ = pFb[i];
				
			}

			if ((beg + 1) % 2 == 0)
			{
				p++;
				unsigned char * p2 = &screen8[320 * (beg + beg / 2 + 12)]; 
				memcpy (p, p2, 320);
			}
		}
		
		return;
	}

	if (pWS->ioRam[0x12] | pWS->ioRam[0x10]) // Oculta las columnas excedentes
	{
		//hideExtraColumns(beg, end, bg);
		//u8 * pFb;
		//u32 line;
		for (; beg < end; ++beg)
		{
			//pFb = FBPTR(line) - 8;
			memset(pFb - 8, 0, 8);
			//pFb += 232;
			memset(pFb + 224, 0, 8);
			pFb += 320;
		}
	}
}

//------------------------------------------------------------------------------
// Wonderswan Interrupt flag check
// [CPU interrupt handling achieved even when close to the actual execution was carried out ? lifting the interrupt]
//------------------------------------------------------------------------------
u32 ws_int_check(void)
{
	u32 intr = pWS->ioRam[0xb6] & pWS->ioRam[0xb2];
	u32 mask = 8;

	//#if 1
	//	pWS->ioRam[0xb6]&=0xf0;
	//	intr            &=0xf0;
	//#endif

	if (intr & 0xf0)
	{
		if (intr & INT_HBLANK) mask = 7; // HBLANK Timer
		else if (intr & INT_VBLANK) mask = 6; // VBLANK begin
		else if (intr & INT_VTIMER) mask = 5; // VBLANK end
		else if (intr & INT_SCLINE) mask = 4; // SCANLINE
		else
			return 0;

		/*
		if (mask < 8)
		{
			if ( nec_int((pWS->ioRam[0xb0] + mask) << 2) )
			{
				//				pWS->ioRam[0xb6]&=~(1<<mask);
			}

			return 1;
		}
		*/
		nec_int((pWS->ioRam[0xb0] + mask) << 2);

		return 1;
	}

	return 0;
}

//-----------------------------------------------------------------------------
//
// Interrupt flag update
//
//-----------------------------------------------------------------------------
static void ws_int_work(void)
{
	//-----------------------------------------------------
	// SPRITE TABLE FETCH
	//-----------------------------------------------------
	if ( //SPRITE_UPDATE_LINE
	    VBLANK_BEGIN_LINE == pWS->line )
	{
		u32 base = (((u32)pWS->ioRam[4] & 0x3f) << 9) + (pWS->ioRam[5] << 2);

		if ( (pWS->spTableCnt = pWS->ioRam[6]) > 0x80)
		{
			pWS->ioRam[6] = 0x80;
		}

		// [Puck] core_memcpyX4(pWS->spTable, (u32*)&pWS->iRam[base], pWS->spTableCnt * 4);
#ifdef ARM
		memcpy32(pWS->spTable, (u32*)&pWS->iRam[base], pWS->spTableCnt);
#else
		memcpy(pWS->spTable, (u32*)&pWS->iRam[base], pWS->spTableCnt << 2);
#endif
		//
	}

	//-----------------------------------------------------
	// HBLANK TIMER
	//-----------------------------------------------------
	if (HBLANK_TIMER())
	{
		if (pWS->ioRam[0xa4] || pWS->ioRam[0xa5])
		{
			if ( pWS->ioRam[0xa4] == 0 )
			{
				pWS->ioRam[0xa5]--;    // 0xa5 [is absolutely non-zero]
				pWS->ioRam[0xa4] = 255;
			}
			else
			{
				if ((--pWS->ioRam[0xa4]) == 0)
				{
					if (pWS->ioRam[0xa5] == 0)
					{
						pWS->ioRam[0xb6] |= (pWS->ioRam[0xb2] & INT_HBLANK);

						if (HBLANK_AUTO())
						{
							pWS->ioRam[0xa4] = pWS->ioRam[IO_BACKUP_A4];
							pWS->ioRam[0xa5] = pWS->ioRam[IO_BACKUP_A5];
						}
					}
				}
			}
		}
		/*
		else
		{
			//			pWS->ioRam[0xb6]|=(pWS->ioRam[0xb2]&INT_HBLANK);
		}
		*/
	}

	//-----------------------------------------------------
	// VBLANK BEGIN
	//-----------------------------------------------------
	if ( VBLANK_BEGIN_LINE == pWS->line )
	{
		pWS->ioRam[0xb6] |= pWS->ioRam[0xb2] & INT_VBLANK;

		if ( !(++pWS->ioRam[0xaa]) && !(++pWS->ioRam[0xab]) && !(++pWS->ioRam[0xac]) )
		{
			++pWS->ioRam[0xad];
		}
	}

	//-----------------------------------------------------
	// VBLANK TIMER
	//-----------------------------------------------------
	if ( SCANLINE_MAX == pWS->line )
	{
		if (VBLANK_TIMER())
		{
			// [If the counter is set counting process]
			if ( pWS->ioRam[0xa6] || pWS->ioRam[0xa7])
			{
				if ( pWS->ioRam[0xa6] == 0 )
				{
					pWS->ioRam[0xa7]--;
					pWS->ioRam[0xa6] = 255;
				}
				else
				{
					if ((--pWS->ioRam[0xa6]) == 0)
					{
						if (pWS->ioRam[0xa7] == 0)
						{
							pWS->ioRam[0xb6] |= (pWS->ioRam[0xb2] & INT_VTIMER);

							if (VBLANK_AUTO())
							{
								pWS->ioRam[0xa6] = pWS->ioRam[IO_BACKUP_A6];
								pWS->ioRam[0xa7] = pWS->ioRam[IO_BACKUP_A7];
							}
						}
					}
				}
			}
			// [If not set interrupt counter is not counting]
			else
			{
				pWS->ioRam[0xb6] |= (pWS->ioRam[0xb2] & INT_VTIMER);
			}
		}
	}

	//-----------------------------------------------------
	// SCANLINE INT
	//-----------------------------------------------------
	if (pWS->line == pWS->line_cmp)
	{
		pWS->ioRam[0xb6] |= pWS->ioRam[0xb2] & INT_SCLINE;
	}
}


//------------------------------------------------------------------------------
// 2006.10.10 : [Do not change it and found the following experimental conditions required]
// SCANLINE       = 0 to 159 (total 160)
// VBLANK Timing  = 145
//
// FF1 [VBLANK Timing in VBLANK Timer is then set to work.]
// [ 2 Senkaiden VBLANK Timing is essential that it is operating at 145.]
//
// [<2 Senkaiden>]
// SCANLINE=0 to 158, VBLANK Timing=144 [in the process of going]
// SCANLINE=0 to 158, VBLANK Timing=145 [normal processing.]
// SCANLINE=144 [I like running the 144-based check processing]
// VBLANK Timing [of doubt that 145 is considered.]
//
// <FF1>
// SCANLINE=0 to 158, VBLANK Timing=144 [normal operation]
// SCANLINE=0 to 158, VBLANK Timing=145 [shift at 145 dots per image.]
// [2 Senkaiden required because VBLANK Timing = 145, FF1 they shifted one dot at 145.]
// [The one dot shifted one dot shifted VBlank Timer so because the behavior of]
// [SCANLINE minute if you change just one dot in the number of OK!]
// [SCANLINE = 0 to mean that that was a 159 OK.]
//
//------------------------------------------------------------------------------
//u32 tiempoAnterior = 0, tiempoActual = 0, ticks = 0, globalticks = 0;
//u32 totalFrames = 0, drawnFrames = 0;

static void renderFrame(u32 skipVideo)
{
#ifdef DEF_QUICK
	int iflag = 0, beg_line = 0;
#endif

	if (skipVideo)
	{
		pWS->line = 0;

		//-----------------------------------------------------
		// No Drawing Area
		//-----------------------------------------------------
		for (; pWS->line <= SCANLINE_MAX; pWS->line++)
		{
			ws_voice_dma();
			ws_int_work();
			ws_int_check();
			nec_execute(256);
			apuWaveSet();
			NCSR = apuShiftReg();			
		}
		return;
	}
	
	
	pWS->line = 0;

	//-----------------------------------------------------
	// Drawing Area
	//-----------------------------------------------------
	for (; pWS->line < END_DRAW_LINE; pWS->line++)
	{
		ws_voice_dma();
		ws_int_work();

#ifdef DEF_QUICK

		if ((iflag = ws_int_check()))
		{
			drawLine(beg_line, pWS->line + 1);
			beg_line = pWS->line + 1;
		}

#else
		drawLine(pWS->line, pWS->line + 1);
#endif
		nec_execute(256);
		apuWaveSet();
		NCSR = apuShiftReg();
	}

#ifdef DEF_QUICK
	if (!iflag)
	{
		drawLine(beg_line, pWS->line);
	}
#endif

	//-----------------------------------------------------
	// No Drawing Area
	//-----------------------------------------------------
	for (; pWS->line <= SCANLINE_MAX; pWS->line++)
	{
		ws_voice_dma();
		ws_int_work();
		ws_int_check();
		nec_execute(256);
		apuWaveSet();
		NCSR = apuShiftReg();
	}
}

static void updateVideo()
{
	s32 i;
	//for (i = 0; i < 256; i++)
	for (i = 255; i >= 0; --i)
	{
		if (pWC->palette[i][3])
		{
			//printf ("%d -> Color sucio de paleta con index %d: %d, %d, %d\n", timerRead(), i,
			//pWC->palette[i][0], pWC->palette[i][1], pWC->palette[i][2]);
			setPaletteColor(pWC->palette[i][0], pWC->palette[i][1], pWC->palette[i][2], i);
			pWC->palette[i][3] = 0;
		}
	}
	setPaletteColor(255, 255, 255, pWC->idxWhite);
	setPaletteColor(0, 0, 0, pWC->idxBlack);
	
	/*
	setPaletteColor(255, 255, 255, 16);
	setPaletteColor(0, 0, 0, 0);
	
	for (i = 32; i < 240; i += 16)
		setPaletteColor(255, 0, 0, i);
	setPaletteColor(0, 255, 0, 240);
	//setPaletteColor(244, 18, 231, 0);
	*/

	updatePalette();

	if (showFPS)
	{
		//if (frames_rendered % 75 == 0)
		if ((frames_rendered & 0x3F) == 0) // Cada 64 frames
		{
			int fps = ((frames_displayed-last_frames_displayed)*100) / (frames_rendered-last_frames_rendered);
			last_frames_displayed = frames_displayed;
			last_frames_rendered = frames_rendered;
			sprintf (sfps, "FPS: %.d%%", fps);
		}
#if 0
		printf ("%s\n", sfps);
#endif
		//printText(screen8, 192, 48, sfps, pWC->idxWhite);
		printText(screen8, fpsX, fpsY, sfps, pWC->idxWhite);
	}
	
	if (showMessage)
	{
		showMessage--;
		printText(screen8, messageX, messageY, sMessage, pWC->idxWhite);
	}

	videoFlip(0);
}

static s32 SegAim()
{
	s32 aim = CurrentSoundBank;

	aim--;

	if (aim < 0) aim += 8;

	return aim;
}

u32 SWAN_Loop(void)
{
	u32 i;

	//static u32 last_frames_rendered = 0;
	//static u32 last_frames_displayed = 0;
	//static u32 tiempoAnterior = 0;
	//u32 tiempoActual = 0;
	//u32 ticks = 0;
	
	u32 running = 1;
	u32 done = 0, aim = 0;
		
	/* Main emulation loop */
	done = SegAim();
	//short * soundbuffer;
	
	while (running)
	{
		for (i = 0; i < 10; i++)
		{
			aim = SegAim();

			//printf ("Done: %d - Aim: %d\n", done, aim);
#ifdef PROFILE
			aim = 1;
			done = 0;
#endif
			if (done != aim)
			//if (1)
			{
				soundbuffer = pOutput[done];
				lastSampleL = lastSampleR = 0;
				done++;

				if (done >= 8) done = 0;

				if (done == aim)
				//if (1)
				{
					renderFrame(0);
					updateVideo();
					frames_displayed++;
				}
				else renderFrame(1);

				//running = gp2x_update_inputs();
				running = !((ws_key = check_keys()) & (1 << 31));
				//sound_mixer (soundbuffer, snd.sample_count);

				frames_rendered++;
			}

			if (done == aim) break;
		}

		done = aim;

#ifdef PROFILE
		if (frames_rendered >= 4500) running = 0;
#endif
	}
	
	return 0;
}


//------------------------------------------------------------------------------
//
// [ROM running wants to show how the rotation]
//
//------------------------------------------------------------------------------
int isWonderSwanRotate(void)
{
	if (ws_RomAddr)
	{
		return ws_RomAddr[ws_RomSize-4] & 1;
	}

	return 0;
}

//------------------------------------------------------------------------------
// [ROM functions to be performed only once after the load]
// [This function describes the processing of the ROM,]
// [Reset processing required to implement different functions]
//------------------------------------------------------------------------------
static int ws_init(u8 * pRom, u32 nRom)
{
	ws_romHeaderStruct* pRomHeader = 0;

	memset(pWS, 0, sizeof(WONDERSWAN_T));

	memset(pWsRomMap, 0, sizeof(pWsRomMap));
	pWsRomMap[0] = pWS->iRam;
	pWsRomMap[1] = pWS->romRam;

	romAddressMask = nRom - 1;
	sramAddressMask = eEepromAddressMask = 0;

	pRomHeader = (ws_romHeaderStruct*)&pRom[nRom-10];

	// SRAM
	switch (pRomHeader->eepromSize)
	{
		case 0x01:
			sramAddressMask    = 0x01fff;
			break; //  64kbit =  8KB (SRAM)
		case 0x02:
			sramAddressMask    = 0x07fff;
			break; //  256kbit= 32KB
		case 0x03:
			sramAddressMask    = 0x1ffff;
			break; // 1024kbit=128KB
		case 0x04:
			sramAddressMask    = 0x3ffff;
			break; // 2048kbit=256KB
		case 0x10:
			eEepromAddressMask = 0x0007f;
			break; //    1kbit= 128B
		case 0x20:
			eEepromAddressMask = 0x007ff;
			break; //   16kbit=2048B
		case 0x50:
			eEepromAddressMask = 0x000ff;
			break; //    8kbit=1024B
		case 0x00:
		default:
			break;
	}

	// Detective Conan
	if ((pRom[nRom-10] == 0x01) && (pRom[nRom-8] == 0x27))
	{
		// WS cpu is using cache/pipeline or
		//   there's protected ROM bank where
		//   pointing CS
		pRom[0xfffe8] = (char)0xea;
		pRom[0xfffe9] = (char)0x00;
		pRom[0xfffea] = (char)0x00;
		pRom[0xfffeb] = (char)0x00;
		pRom[0xfffec] = (char)0x20;
	}

	return 1;
}

//------------------------------------------------------------------------------
//- Checks inputs
//------------------------------------------------------------------------------

unsigned long last_key = 0;
static u32 check_keys ()
{
	u32 key = 0;
	u32 pad;

	pad = joyRead(0);

	if (!pWC->rotated)
	{	
		if (pWC->vertical)
		{
			if (pad & option.inputs[0].maps[MAP_XU])	key |= 1 << WSC_YU;
			
			if (pad & option.inputs[0].maps[MAP_XD])	key |= 1 << WSC_YD;
			
			if (pad & option.inputs[0].maps[MAP_XL])	key |= 1 << WSC_YL;
			
			if (pad & option.inputs[0].maps[MAP_XR])	key |= 1 << WSC_YR;
			
			if (pad & option.inputs[0].maps[MAP_YU])	key |= 1 << WSC_XL;
			
			if (pad & option.inputs[0].maps[MAP_YD])	key |= 1 << WSC_XU;
			
			if (pad & option.inputs[0].maps[MAP_YL])	key |= 1 << WSC_A;
			
			if (pad & option.inputs[0].maps[MAP_YR]) 	key |= 1 << WSC_B;
			
			if (pad & option.inputs[0].maps[MAP_A])		key |= 1 << WSC_XR;
			
			if (pad & option.inputs[0].maps[MAP_B])		key |= 1 << WSC_XD;
			
			if (pad & option.inputs[0].maps[MAP_START])	key |= 1 << WSC_S;
		}
		else // Horizontal
		{
			if (pad & option.inputs[0].maps[MAP_XU])	key |= 1 << WSC_XU;
			
			if (pad & option.inputs[0].maps[MAP_XD])	key |= 1 << WSC_XD;
			
			if (pad & option.inputs[0].maps[MAP_XL])	key |= 1 << WSC_XL;
			
			if (pad & option.inputs[0].maps[MAP_XR])	key |= 1 << WSC_XR;
			
			if (pad & option.inputs[0].maps[MAP_YU])	key |= 1 << WSC_YU;
			
			if (pad & option.inputs[0].maps[MAP_YD])	key |= 1 << WSC_YD;
			
			if (pad & option.inputs[0].maps[MAP_YL])	key |= 1 << WSC_YL;
			
			if (pad & option.inputs[0].maps[MAP_YR]) 	key |= 1 << WSC_YR;
			
			if (pad & option.inputs[0].maps[MAP_A])		key |= 1 << WSC_A;
			
			if (pad & option.inputs[0].maps[MAP_B])		key |= 1 << WSC_B;
			
			if (pad & option.inputs[0].maps[MAP_START])	key |= 1 << WSC_S;
		
			/*
			if (pad & MACH_UP)    key |= 1 << WSC_XU;

			if (pad & MACH_DOWN)  key |= 1 << WSC_XD;

			if (pad & MACH_LEFT)  key |= 1 << WSC_XL;

			if (pad & MACH_RIGHT) key |= 1 << WSC_XR;


			if (pad & MACH_B2)     key |= 1 << WSC_A;

			if (pad & MACH_B3)     key |= 1 << WSC_B;
			*/
		}
	}
	else // Rotado
	{
		if (pad & option.inputs[0].maps[MAP_XU])	key |= 1 << WSC_YR;
		
		if (pad & option.inputs[0].maps[MAP_XD])	key |= 1 << WSC_YL;
		
		if (pad & option.inputs[0].maps[MAP_XL])	key |= 1 << WSC_YU;
		
		if (pad & option.inputs[0].maps[MAP_XR])	key |= 1 << WSC_YD;
		
		if (pad & option.inputs[0].maps[MAP_YU])	key |= 1 << WSC_XU;
		
		if (pad & option.inputs[0].maps[MAP_YD])	key |= 1 << WSC_XR;
		
		if (pad & option.inputs[0].maps[MAP_YL])	key |= 1 << WSC_A;
		
		if (pad & option.inputs[0].maps[MAP_YR]) 	key |= 1 << WSC_B;
		
		if (pad & option.inputs[0].maps[MAP_A])		key |= 1 << WSC_XD;
		
		if (pad & option.inputs[0].maps[MAP_B])		key |= 1 << WSC_XL;
		
		if (pad & option.inputs[0].maps[MAP_START])	key |= 1 << WSC_S;

		/*
		if (pad & option.inputs[0].maps[MAP_XU])	key |= 1 << WSC_YR;
		
		if (pad & option.inputs[0].maps[MAP_XD])	key |= 1 << WSC_YL;
		
		if (pad & option.inputs[0].maps[MAP_XL])	key |= 1 << WSC_YU;
		
		if (pad & option.inputs[0].maps[MAP_XR])	key |= 1 << WSC_YD;
		
		if (pad & option.inputs[0].maps[MAP_YU])	key |= 1 << WSC_XR;
		
		if (pad & option.inputs[0].maps[MAP_YD])	key |= 1 << WSC_XL;
		
		if (pad & option.inputs[0].maps[MAP_YL])	key |= 1 << WSC_XU;
		
		if (pad & option.inputs[0].maps[MAP_YR]) 	key |= 1 << WSC_XD;
		
		if (pad & option.inputs[0].maps[MAP_A])		key |= 1 << WSC_A;
		
		if (pad & option.inputs[0].maps[MAP_B])		key |= 1 << WSC_B;
		
		if (pad & option.inputs[0].maps[MAP_START])	key |= 1 << WSC_S;
		*/		 

	}
		
	//if (pad & MACH_START) key |= 1 << WSC_S;	
		
	if (pad & MACH_EXIT)   key |= 1 << 31;

	if (pad & MACH_VOLDOWN) if (volume > 0)
	{
	    volume--;
	    soundVolume(volume, volume);
	}

	if (pad & MACH_VOLUP) if (volume < 100)
	{
	    volume++;
	    soundVolume(volume, volume);
	}

	if (pad & MACH_SAVE) if (pad != last_key) saveState();

	if (pad & MACH_LOAD) if (pad != last_key) loadState();

	if (pad & MACH_SLOTD) if (pad != last_key)
	{
		stateSlot = (stateSlot == 0) ? 9 : stateSlot - 1;
		sprintf (sMessage, "Slot %d", stateSlot);
		showMessage = MESSAGE_DELAY;
#ifdef DEBUG
		printf ("Slot %u\n", stateSlot);
#endif
	}

	if (pad & MACH_SLOTU) if (pad != last_key)
	{
		stateSlot = (stateSlot + 1) % 10;
		sprintf (sMessage, "Slot %d", stateSlot);
		showMessage = MESSAGE_DELAY;
#ifdef DEBUG
		printf ("Slot %d\n", stateSlot);
#endif
	}

	if (pad & MACH_SHOWFPS) if (pad != last_key) showFPS ^= 1;

#ifdef DEBUG
	if (pad & MACH_LAYER1) if (pad != last_key)
	{
		showBGBG ^= 1;
		printf ("Background layer: %u\n", showBGBG);
	}

	if (pad & MACH_LAYER2) if (pad != last_key)
	{
		showBGFG ^= 1;
		printf ("Foreground layer: %u\n", showBGFG);
	}

	if (pad & MACH_LAYER3) if (pad != last_key)
	{
		showSptr ^= 1;
		printf ("Sprites layer: %u\n", showSptr);
	}

/*
	if (pad & MACH_AUDIO1) if (pad != last_key)
	{
		noiseEnabled ^= 1;
		printf ("Noise channel: %u\n", noiseEnabled);
	}

	if (pad & MACH_AUDIO2) if (pad != last_key)
	{
		psgEnabled ^= 1;
		printf ("PSG channel: %u\n", psgEnabled);
	}

	if (pad & MACH_AUDIO3) if (pad != last_key)
	{
		sweepEnabled ^= 1;
		printf ("Sweep channel: %u\n", sweepEnabled);
	}

	if (pad & MACH_AUDIO4) if (pad != last_key)
	{
		voiceEnabled ^= 1;
		printf ("Voice channel: %u\n", voiceEnabled);
	}			
*/
		
#endif

	last_key = pad;

	return key;
}


//------------------------------------------------------------------------------
//-
//------------------------------------------------------------------------------
u32 SWAN_Init(u32 nRomSize, u8 * pRomAddr, t_option * options)
{
	u32 i;
	
	ws_RomAddr = pRomAddr;
	ws_RomSize = nRomSize;
	ws_key = 0;

	// TODO: Prueba de paletas personalizadas [BORRAR]
	if (options->scheme == -1)
	{
		srand (time(NULL));
		for (i = 0; i < 16; i++)
		{
			options->shades[i][0] = rand() % 256;
			options->shades[i][1] = rand() % 256;
			options->shades[i][2] = rand() % 256;
		}
	}
	
	if ( (pWS = (WONDERSWAN_T*)malloc(sizeof(WONDERSWAN_T))) &&
	        (pWC = (SYSTEM_CACHE_T*)malloc(sizeof(SYSTEM_CACHE_T))) )
	{
		// SCANLINE NUMBER TABLE
		{
			for (i = 0; i < SCANLINE_MAX; ++i)
			{
				pWC->scanline_num[i] = i;
			}

			pWC->scanline_num[SCANLINE_MAX] = 158;
		}
		
		if (options->sndrate <= 12000)
		{
			soundFill = soundFill12k;
		}
		else if (options->sndrate <= 24000)
		{
			if (options->sndfilter == FILTER_LINEAR)
				soundFill = soundFillLinear24k;
			else
				soundFill = soundFill24k;
		}
		else // 48k
		{
			if (options->sndfilter == FILTER_LINEAR)
				soundFill = soundFillLinear48k;
			else
				soundFill = soundFill48k;
		}		
		
		pWC->vertical = isWonderSwanRotate();
		pWC->rotated = pWC->vertical && options->autorotate;
#ifdef DEBUG
		printf ("Rotated screen: %u\n", pWC->rotated);
#endif

		pWC->fullscreen = options->fullscreen && !pWC->rotated;

		if (pWC->rotated || pWC->fullscreen)
		{
			free(fbtemp8);
			fbtemp8 = (u8 *) malloc(320*240);
		}		
		
		if (pWC->fullscreen)
		{
			fpsX = 240;
			fpsY = 15;
			messageX = 20;
			messageY = 15;
		}
		else if (pWC->rotated)
		{
			fpsX = 88;
			fpsY = 10;
			messageX = 88;
			messageY = 222;
		}
		else
		{
			fpsX = 192;
			fpsY = 48;
			messageX = 48;
			messageY = 48;
		}
		
		for (i = 0; i < 256; i++)
		{
#ifdef DEBUG
			// Pone todos los colores a rosa para detectar fallos de paleta
			/*
			pWC->palette[i][0] = 244;
			pWC->palette[i][1] = 18;
			pWC->palette[i][2] = 231;
			*/
#endif			
			pWC->palette[i][3] = 1;
		}	
		
		/*
		for (i = 0; i < 4; i++)
		{
			memset (screen8, 0, 320 * 240);
			videoFlip(0);
		}
		*/
		
#ifdef GP2X
		// Establece pantalla completa por hardware
		if (pWC->fullscreen && !pWC->rotated) gp2x_video_RGB_setscaling(224, 168);
#endif

		// Establece el marco en caso de haberlo
		s32 use_marquee = strcmp (option.marquee, "marquees/!none.png");
		
		if (!option.fullscreen && use_marquee)
		{	
			setBackLayer(1, 16);
			t_img_rect image;
			loadPNG(option.marquee, &image, 16, 1);
			memcpy (back8, image.data, 320*240*2);
			videoFlip(1);
			memcpy (back8, image.data, 320*240*2);
			videoFlip(1);    
		}		

		if (ws_init(pRomAddr, nRomSize))
		{
			ws_reset();
			//SWAN_SoundInit();
			ws_set_colour_scheme(options);
			
			apuInit();

			//last_frames_rendered = 0;
			//last_frames_displayed = 0;

			/* Carga EEPROM/SRAM */
			loadConfig();
			
			soundPause(0);
			
			/*
			// READ : EEPROM(Internal)
			//[Puck] HAL_Cfg_Load(SWAN_INTERNAL_EEP, internalEeprom, sizeof(internalEeprom));

			// ROM [SRAM or EEPROM]
			if (sramAddressMask)
			{
				//[Puck] HAL_Mem_Load(pWS->romRam, sramAddressMask + 1);
			}
			else if (eEepromAddressMask)
			{
				//[Puck] HAL_Mem_Load(pWS->romE2P, eEepromAddressMask + 1);
			}
			*/

			return 1;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
u32 SWAN_Exit(void)
{
	/* Salva EEPROM/SRAM */
	saveConfig();
	
	/*
	// WRITE : EEPROM(Internal)
	// [Puck] HAL_Cfg_Save(SWAN_INTERNAL_EEP, internalEeprom, sizeof(internalEeprom));

	// ROM [SRAM or EEPROM]
	if (sramAddressMask)
	{
		//[Puck] HAL_Mem_Save(pWS->romRam, sramAddressMask + 1);
	}
	else if (eEepromAddressMask)
	{
		//[Puck] HAL_Mem_Save(pWS->romE2P, eEepromAddressMask + 1);
	}
	*/

	ws_RomAddr = 0;
	ws_RomSize = 0;

	free(pWS);
	free(pWC);
	free(fbtemp8);

	pWS = 0;
	pWC = 0;
	fbtemp8 = 0;
	
	memset (back8, 0, 320 * 240 * 2);
	videoFlip(1);
	memset (back8, 0, 320 * 240 * 2);
	videoFlip(1);
	
	setBackLayer(0, 0);
	
	soundPause(1);
	soundDeInit();

	return 1;
}



