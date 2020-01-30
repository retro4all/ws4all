#include "hal.h"
#include <SDL.h>

SDL_Surface * screen;
SDL_Surface * vbuf;
SDL_Color palette [256];

volatile short *pOutput[8];
volatile int CurrentSoundBank=0;

int ciclos = 0;

void machineInit (int bpp, int rate, int bits, int stereo)
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO);
	screen = SDL_SetVideoMode(320, 240, bpp, SDL_SWSURFACE|SDL_HWPALETTE);
	
	SDL_JoystickOpen(0);
	SDL_ShowCursor(SDL_DISABLE);

	screen8 = screen->pixels;
}

void machineDeInit (void)
{
	SDL_FreeSurface(screen);
	printf ("Saliendo\n");
	SDL_Quit();
}

void setMachineClock (int speed)
{
	return;
}

void setBackLayer (int enabled, int bpp)
{
	return;
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

int loadPNG (char filename[], t_img_rect * image, int bitdepth, int solid)
{
	return 1;
}

void setPaletteColor (char r, char g, char b, unsigned char index)
{
	/*
	SDL_Color color;

	color.r = r;
	color.g = g;
	color.b = b;
	
	SDL_SetPalette(screen, SDL_LOGPAL | SDL_PHYSPAL, &color, index, 1);
	*/
	palette[index].r = r;
	palette[index].g = g;
	palette[index].b = b;
}

void setPalette (char palette[256][3])
{
	SDL_Color colors[256];
	int i;

	for (i = 0; i < 256; i++)
	{
		colors[i].r = palette[i][0];
		colors[i].g = palette[i][1];
		colors[i].b = palette[i][2];
	}

	SDL_SetPalette(screen, SDL_LOGPAL | SDL_PHYSPAL, colors, 0, 256);
}

void updatePalette (void)
{
	SDL_SetPalette(screen, SDL_LOGPAL|SDL_PHYSPAL, palette, 0, 256);
}

void videoFlip (int layer)
{
	SDL_Flip(screen);
}

unsigned long joyRead (int joystick)
{
	SDL_Event event;
	unsigned long joy = 0;
	unsigned char * keys = SDL_GetKeyState(NULL);

    while (SDL_PollEvent (&event))
    {
		if (event.type == SDL_QUIT)
		{
			SDL_Quit();
		}
    }
	
	if (keys[SDLK_UP]) joy |= MACH_UP;
	if (keys[SDLK_DOWN]) joy |= MACH_DOWN;
	if (keys[SDLK_LEFT]) joy |= MACH_LEFT;
	if (keys[SDLK_RIGHT]) joy |= MACH_RIGHT;
	if (keys[SDLK_ESCAPE]) 
	{
		joy |= MACH_START;
		joy |= MACH_EXIT;
	}
	if (keys[SDLK_SPACE]) joy |= MACH_SELECT;
	if (keys[SDLK_RETURN]) joy |= MACH_START;
	if (keys[SDLK_a]) joy |= MACH_B1;
	if (keys[SDLK_s]) joy |= MACH_B2;
	if (keys[SDLK_x]) joy |= MACH_B3;
	if (keys[SDLK_y]) joy |= MACH_B4;
	if (keys[SDLK_l]) joy |= MACH_B5;
	if (keys[SDLK_r]) joy |= MACH_B6;
	if (keys[SDLK_n]) joy |= MACH_SAVE;
	if (keys[SDLK_m]) joy |= MACH_LOAD;
	if (keys[SDLK_o]) joy |= MACH_SLOTD;
	if (keys[SDLK_p]) joy |= MACH_SLOTU;
	if (keys[SDLK_f]) joy |= MACH_SHOWFPS;
	
	if (keys[SDLK_F1]) joy |= MACH_LAYER1;
	if (keys[SDLK_F2]) joy |= MACH_LAYER2;
	if (keys[SDLK_F3]) joy |= MACH_LAYER3;
	
	//printf ("SDLK_a: %d, SDLK_b: %d\n", SDLK_a, SDLK_b);

	return joy;
}

void timerDelay (unsigned long delay)
{
	SDL_Delay(delay);
}

unsigned long timerRead (void)
{
	return SDL_GetTicks();
}

/*
static void *gp2x_sound_play(void)
{
  while(!gp2x_exit)     {
  	if(!gp2x_sound_pausei) {
		//Timer++;
		CurrentSoundBank++;

		if (CurrentSoundBank >= 8) CurrentSoundBank = 0;
		
		write(gp2x_dev[3], (void *)pOutput[CurrentSoundBank], gp2x_sound_buffer[1]);
		ioctl(gp2x_dev[3], SOUND_PCM_SYNC, 0); 
	}
   } 
   return NULL;
}
*/

/*
static void sound_callback (void *userdata, Uint8 *stream, int len)
{
	//printf ("Len: %d - Ticks: %u\n", len, SDL_GetTicks());
	
	CurrentSoundBank++;

	if (CurrentSoundBank >= 8) CurrentSoundBank = 0;
	
	memset (stream, 0, len);
	
	//ciclos++;
}
*/

static void sound_callback (void *user, Uint8 * buffer, int len)
{
	//printf ("Sound callback: AudioLen: %d - Len: %d\n", audioLen, len);
	/* Check whether or not there is enough generated sound data */
	if (audioLen < (Uint32)len)
	{
		/* Send all the already generated sound data */
		memcpy(buffer, user, audioLen);

		/* Set the remaining audio buffer to zero to avoid undesired noises */
		memset(buffer + audioLen, 0, len - audioLen);

		/* Reset the user buffer length and over time */
		audioLen = 0;
		//gens_rt.over_time = 0;
	}
	else
	{
		/* Send the sound data required by the hardware */
		memcpy (buffer, user, len);

		/* Update the user buffer data length */
		audioLen -= len;

		/* Set the over time depending upon remaining buffer length */
		//gens_rt.over_time = (audio_len > (Uint32)len)?1:0;

		/* Copy the rest of the created buffer to the beginning */
		/* to be fetched the next time the callback is called */
 		memcpy (user, (unsigned char *) user + len, audioLen);
	}
	//ciclos++;
}

void mix_audio(void* unused, Uint8 *stream8, int len)
{
	memset(stream8, 0, len);
}

void soundInit (int rate, int bits, int stereo, int Hz)
{	
	SDL_AudioSpec as_desired, as_obtained;

	as_desired.freq = rate; 				// Frecuencia de muestreo
	as_desired.format = AUDIO_S16SYS;			// 16 bits con signo
	as_desired.channels = stereo + 1;		// Dos canales (estereo)
	as_desired.samples = 1024; 				// Tamano de muestras
	as_desired.callback = mix_audio;		// Funcion callback
	
	/*
	if (audiobuffer != NULL)
		free(audiobuffer);
		
	audioBufferSize = as_desired.samples * as_desired.channels * 8 * sizeof(short);
	
	audiobuffer = (char *) malloc (audioBufferSize);
	
	as_desired.userdata = (unsigned char *) audiobuffer;
	*/
	as_desired.userdata = 0;
	
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
	
	soundPause(0);
}

void soundDeInit (void)
{
	soundPause(1);
	SDL_CloseAudio();
}

void soundVolume (int left, int right)
{
	
}

void soundPause (int pause)
{
	printf ("Pausa: %d\n", pause);
	SDL_PauseAudio(pause);
}

void printText (unsigned char *screen, int x, int y, char *text, int color)
{
	
}
