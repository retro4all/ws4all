#include "machinedep.h"
#include "minimal.h"

void machineInit (int bpp, int rate, int bits, int stereo)
{
	gp2x_init(1000, bpp, rate, bits, stereo, 60, 1);
	
	screen8 = gp2x_video_RGB[0].screen8;
	
	if (hack_the_mmu() == 0)
	{
		printf("Patching MMU ... OK!\n");
	}
	else
	{
		printf("Patching MMU ... FAILED :(\n");
	}
}

void machineDeInit (void)
{
	gp2x_deinit();
}

void setMachineClock (int speed)
{
	SetGP2XClock(speed);
}

void drawSprite(char* image, int srcX, int srcY, int dstX, int dstY, int width, int height, int imgw, int imgh )
{
	int i;
	int j;

	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
			screen8[(dstY+j)*320+dstX+i] = image[(srcY+j)*imgw+srcX+i];
		}
	}
}

void drawTransSprite (char* image, int srcX, int srcY, int dstX, int dstY, int width, int height, int imgw, int imgh, char trans)
{
	int i;
	int j;
	char color;

	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
			color =  image[(srcY+j)*imgw+srcX+i];

			if (color != trans) screen8[(dstY+j)*320+dstX+i] = color;
		}
	}
}

void drawRect (int x1, int y1, int x2, int y2, char color)
{
	int i, j;

	for (j = y1; j < y2; j++)
	{
		for (i = x1; i < x2; i++)
		{
			screen8[j*320+i] = color;
		}
	}
}

void drawText (char * image, int x, int y, char * texto, int center)
{
	int i = 0;
	int lenght;

	if (center)
	{
		lenght = strlen(texto);

		if (lenght < 34)
		{
			x = (320 - lenght * 8) / 2;
		}
		else x = 24;
	}

	while (texto[i] != 0 && i < 34)
	{
		if (texto[i] > 43 && texto[i] < 60)
		{
			drawTransSprite(image, (texto[i] - 44) * 8 + 32, 0, x, y, 8, 8, 416, 8, 0);
		}
		else if (texto[i] > 64 && texto[i] < 91)
		{
			drawTransSprite(image, (texto[i] - 65) * 8 + 160, 0, x, y, 8, 8, 416, 8, 0);
		}
		else
		{
			switch (texto[i])
			{
				case 32:
					break;
				case 33:
					drawTransSprite(image, 376, 0, x, y, 8, 8, 416, 8, 0);
					break;
				case 40:
				case 41:
					drawTransSprite(image, (texto[i] - 40) * 8 + 8, 0, x, y, 8, 8, 416, 8, 0);
					break;
				case 60:
					drawTransSprite(image, 408, 0, x, y, 8, 8, 416, 8, 0);
					break;
				case 62:
					drawTransSprite(image, 368, 0, x, y, 8, 8, 416, 8, 0);
					break;
				case 63:
					drawTransSprite(image, 384, 0, x, y, 8, 8, 416, 8, 0);
					break;
				case 91:
					drawTransSprite(image, 392, 0, x, y, 8, 8, 416, 8, 0);
					break;
				case 93:
					drawTransSprite(image, 400, 0, x, y, 8, 8, 416, 8, 0);
					break;
				default:
					break;
			}
		}

		x += 8;
		i++;
	}
}

void setPaletteColor (char r, char g, char b, unsigned char index)
{
	gp2x_video_RGB_color8(r, g, b, index);
}

void setPalette (char palette[256][3])
{
	int i;
	
	for (i = 0; i < 256; i++)
	{
		gp2x_video_RGB_color8(palette[i][0], palette[i][1], palette[i][2], i);
	}

	gp2x_video_RGB_setpalette();
}

void updatePalette (void)
{
	gp2x_video_RGB_setpalette();
}

void videoFlip (int layer)
{
	gp2x_video_RGB_flip(layer);
	
	if (layer == 0) screen8 = gp2x_video_RGB[0].screen8;
}

unsigned long joyRead (int joystick)
{
	unsigned long joy = 0;
	unsigned int pad = gp2x_joystick_read();
	
	if (pad & GP2X_UP) joy |= MACH_UP;
	if (pad & GP2X_DOWN) joy |= MACH_DOWN;
	if (pad & GP2X_LEFT) joy |= MACH_LEFT;
	if (pad & GP2X_RIGHT) joy |= MACH_RIGHT;
	if (pad & GP2X_L) if (pad & GP2X_R)
	{
		joy |= MACH_EXIT;
	}
	if (pad & GP2X_START) joy |= MACH_START;
	if (pad & GP2X_SELECT) joy |= MACH_SELECT;
	if (pad & GP2X_A) joy |= MACH_B1;
	if (pad & GP2X_B) joy |= MACH_B2;
	if (pad & GP2X_X) joy |= MACH_B3;
	if (pad & GP2X_Y) joy |= MACH_B4;
	if (pad & GP2X_L) joy |= MACH_B5;
	if (pad & GP2X_R) joy |= MACH_B6;
	
	if (pad & GP2X_START) if (pad & GP2X_Y) joy |= MACH_SHOWFPS;
	
	return joy;
}

void timerDelay (unsigned long delay)
{
	gp2x_timer_delay(delay);
}

unsigned long timerRead (void)
{
	return (gp2x_timer_read());
}

void timerProfile (void)
{
	//wiz_timer_profile();
}


void soundInit (int rate, int bits, int stereo, int Hz)
{
	gp2x_sound_init (rate, bits, stereo, Hz);
}

void soundVolume (int left, int right)
{
	gp2x_sound_volume(left, right);
}

void soundPause (int pause)
{
	gp2x_sound_pause(pause);
}

void printText (unsigned char *screen, int x, int y, char *text, int color)
{
	//wiz_text (screen, x, y, text, color);
}
