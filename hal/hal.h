#ifndef __MACHINEDEP_H__
#define __MACHINEDEP_H__	

#define TRANS 242

#define BIT(number) (1<<(number))
enum {
	MACH_B1 	= BIT(1),
	MACH_B2 	= BIT(2),
	MACH_B3 	= BIT(3),
	MACH_B4 	= BIT(4),
	MACH_B5 	= BIT(5),
	MACH_B6 	= BIT(6),
	MACH_START 	= BIT(7),
	MACH_SELECT = BIT(8),
	MACH_LEFT 	= BIT(9),
	MACH_RIGHT 	= BIT(10),
	MACH_UP 	= BIT(11),
	MACH_DOWN 	= BIT(12),
	MACH_VOLUP 	= BIT(13),
	MACH_VOLDOWN= BIT(14),
	MACH_EXIT	= BIT(15),
	MACH_SAVE	= BIT(16),
	MACH_LOAD	= BIT(17),
	MACH_SLOTD	= BIT(18),
	MACH_SLOTU	= BIT(19),
	MACH_SHOWFPS= BIT(20),
	MACH_BATTERY= BIT(21),
	MACH_LAYER1 = BIT(22),
	MACH_LAYER2 = BIT(23),
	MACH_LAYER3 = BIT(24),
	MACH_AUDIO1 = BIT(25),
	MACH_AUDIO2 = BIT(26),
	MACH_AUDIO3 = BIT(27),
	MACH_AUDIO4 = BIT(28)
};

typedef struct 
{
	//struct usbjoy * joy;
	int joytype;
	int maps[12];
} t_input;

typedef struct
{ 
	int x,y,w,h; 
	void *data; 
} t_img_rect;

unsigned char * screen8;
unsigned char * back8;
//unsigned short * screen16;

char * audiobuffer;

int audioLen;
int audioBufferSize;

extern volatile int CurrentSoundBank;
extern volatile short *pOutput[];

void machineInit (int bpp, int rate, int bits, int stereo);

void machineDeInit (void);

void setMachineClock (int speed);

void setBackLayer (int enabled, int bpp);

void setPaletteColor (char r, char g, char b, unsigned char index);

void setPalette (char palette[256][3]);

void updatePalette (void);

void drawRect (int x1, int y1, int x2, int y2, char color);

void drawFillRect (int x1, int y1, int x2, int y2, char color);

void drawSprite(char* image, int srcX, int srcY, int dstX, int dstY, int width, int height, int imgw, int imgh );

void drawTransSprite (char* image, int srcX, int srcY, int dstX, int dstY, int width, int height, int imgw, int imgh, char trans);

void drawText (char * image, int x, int y, char * texto, int center);

int loadPNG (char filename[], t_img_rect * image, int bitdepth, int solid);

void videoFlip (int layer);

unsigned long joyRead (int joystick);

void timerDelay (unsigned long delay);

unsigned long timerRead (void);

void timerProfile (void);

void soundInit (int rate, int bits, int stereo, int Hz);

void soundDeInit (void);

void soundVolume (int left, int right);

void soundPause (int pause);

void printText (unsigned char *screen, int x, int y, char *text, int color);

#endif /* __MACHINEDEP_H__ */
