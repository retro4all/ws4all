#include "hal.h"
#include "wiz_lib.h"
#include "minimal_image.h"

/*
#include <SDL.h>

SDL_Surface * screen;
SDL_Surface * vbuf;
SDL_Color palette [256];
*/
//volatile short *pOutput[8];
//volatile int CurrentSoundBank=0;

void machineInit (int bpp, int rate, int bits, int stereo)
{
	wiz_init (bpp, rate, bits, stereo);
	
	screen8 = fb0_8bit;
	back8 = fb1_8bit;
}

void machineDeInit (void)
{
	wiz_deinit();
}

void setMachineClock (int speed)
{
	wiz_set_clock(speed);
}

void setBackLayer (int enabled, int bpp)
{
	wiz_enable_back_layer(enabled, bpp);
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

			if (color != (char) TRANS) screen8[(dstY+j)*320+dstX+i] = color;
		}
	}
}

void drawRect (int x1, int y1, int x2, int y2, char color)
{
	int i, j;

	x2 += x1;
	y2 += y1;

	j = y1 * 320;
	for (i = x1; i < x2; i++)
		screen8[j + i] = color;
		
	j = (y2 - 1) * 320;
	for (i = x1; i < x2; i++)
		screen8[j + i] = color;	
	
	for (j = y1 + 1; j < y2 - 1; j++)
	{
		screen8[j * 320 + x1] = color;
		screen8[j * 320 + x2 - 1] = color;
	}
}

void drawFillRect (int x1, int y1, int x2, int y2, char color)
{
	int i, j;

	for (j = y1; j < y1 + y2; j++)
	{
		for (i = x1; i < x1 + x2; i++)
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

int loadPNG (char filename[], t_img_rect * image, int bitdepth, int solid)
{
	return gp2x_loadPNG(filename, image, bitdepth, solid);
}

void setPaletteColor (char r, char g, char b, unsigned char index)
{
	wiz_video_color8(index, r, g, b);
}

void setPalette (char palette[256][3])
{
	int i;
	
	for (i = 0; i < 256; i++)
	{
		wiz_video_color8 (i, palette[i][0], palette[i][1], palette[i][2]);
	}

	wiz_video_setpalette();
}

void updatePalette (void)
{
	wiz_video_setpalette();
}

void videoFlip (int layer)
{
	wiz_video_flip(layer);
	
	if (layer == 0) screen8 = fb0_8bit;
	if (layer == 1) back8 = fb1_8bit;
}

unsigned long joyRead (int joystick)
{
	unsigned long joy = 0;
	unsigned int pad = wiz_joystick_read (0);
	
	if (pad & WIZ_UP) joy |= MACH_UP;
	if (pad & WIZ_DOWN) joy |= MACH_DOWN;
	if (pad & WIZ_LEFT) joy |= MACH_LEFT;
	if (pad & WIZ_RIGHT) joy |= MACH_RIGHT;
	if (pad & WIZ_L) if (pad & WIZ_R)
	{
		joy |= MACH_EXIT;
	}
	if (pad & WIZ_MENU) joy |= MACH_START;
	if (pad & WIZ_SELECT) joy |= MACH_SELECT;
	if (pad & WIZ_A) joy |= MACH_B1;
	if (pad & WIZ_B) joy |= MACH_B2;
	if (pad & WIZ_X) joy |= MACH_B3;
	if (pad & WIZ_Y) joy |= MACH_B4;
	if (pad & WIZ_L) joy |= MACH_B5;
	if (pad & WIZ_R) joy |= MACH_B6;
	
	if (pad & WIZ_SELECT) if (pad & WIZ_Y) joy |= MACH_SHOWFPS;
	
	if (pad & WIZ_SELECT) if (pad & WIZ_R) joy |= MACH_LOAD;
	if (pad & WIZ_SELECT) if (pad & WIZ_L) joy |= MACH_SAVE;
	if (pad & WIZ_SELECT) if (pad & WIZ_VOLDOWN) joy |= MACH_SLOTD;
	if (pad & WIZ_SELECT) if (pad & WIZ_VOLUP) joy |= MACH_SLOTU;
	
	if (pad & WIZ_L) if (pad & WIZ_A) joy |= MACH_LAYER1;
	if (pad & WIZ_L) if (pad & WIZ_Y) joy |= MACH_LAYER2;
	if (pad & WIZ_L) if (pad & WIZ_B) joy |= MACH_LAYER3;
	
	if (pad & WIZ_R) if (pad & WIZ_A) joy |= MACH_AUDIO1;
	if (pad & WIZ_R) if (pad & WIZ_B) joy |= MACH_AUDIO2;
	if (pad & WIZ_R) if (pad & WIZ_X) joy |= MACH_AUDIO3;
	if (pad & WIZ_R) if (pad & WIZ_Y) joy |= MACH_AUDIO4;
	
	return joy;
}

void timerDelay (unsigned long delay)
{
	wiz_timer_delay(delay * 1000);
}

unsigned long timerRead (void)
{
	return (wiz_timer_read() / 1000);
}

void timerProfile (void)
{
	wiz_timer_profile();
}


void soundInit (int rate, int bits, int stereo, int Hz)
{
	wiz_sound_init (rate, bits, stereo, Hz);
	
	wiz_sound_volume(100, 100);
	/*
	SDL_AudioSpec as_desired, as_obtained;

	as_desired.freq = rate; 		// Frecuencia de muestreo
	as_desired.format = AUDIO_S16SYS;	// 16 bits con signo
	as_desired.channels = stereo + 1;		// Dos canales (estereo)
	as_desired.samples = 1024; 		// Tamano de muestras
	as_desired.callback = sound_callback;// Funcion callback
	
	if (audiobuffer != NULL)
		free(audiobuffer);
		
	audioBufferSize = as_desired.samples * as_desired.channels * 8 * sizeof(short);
	
	audiobuffer = (char *) malloc (audioBufferSize);
	
	as_desired.userdata = (unsigned char *) audiobuffer;
	
	audioLen = 0;
	
	printf ("Rate: %d, Bits: %d, Stereo: %d, Hz: %d, BufferSize: %d\n", rate, bits, stereo, Hz, audioBufferSize);

	if (SDL_OpenAudio (&as_desired, &as_obtained) == -1)
	{
		printf ("ERROR: No se pudo abrir el modo de audio: %s.\n", SDL_GetError() );
		return;
	}

	if (as_desired.samples != as_obtained.samples)
	{
		printf ("ERROR: La tarjeta de sonido no soporta el tipo de muestras especificadas.\n");
		return;
	}
	
	printf ("Buffer Samples: %d - Buffer Bytes: %d\n", as_obtained.samples, as_obtained.size);
	printf ("Rate: %d - Channels: %d\n", as_obtained.freq, as_obtained.channels);
			
	*/
}

void soundDeInit (void)
{
	wiz_sound_stop();
}

void soundVolume (int left, int right)
{
	wiz_sound_volume(left, right);
}

void soundPause (int pause)
{
	wiz_sound_pause(pause);
	/*
	printf ("Pausa: %d\n", pause);
	SDL_PauseAudio(pause);
	*/
}

void printText (unsigned char *screen, int x, int y, char *text, int color)
{
	wiz_text (screen, x, y, text, color);
}
