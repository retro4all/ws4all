#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hal.h"

#include "ws.h"
#include "types.h"
#include "config.h"
#include "wsrom_good.h"
#include "files.h"

#ifdef DEBUG
extern int frames_rendered;
extern int frames_displayed;
#endif
//extern int last_frames_rendered;
//extern int last_frames_displayed;
//extern int FrameSkip;
//extern int gputime;
//extern int scheme;
//extern int fullscreen;

int main(int argc, char *argv[])
{
	int commandline = 0;
	int exit = 0;

	printf("WS4ALL %s\n", VERSION);
	printf("(C) Israel Lopez Fernandez (Puck2099)\n");
	printf("Based on e[mulator] (C) e (T.Kawamorita)\n\n");

	int i;

	u8 * base_rom = NULL;
	u32 romSize;

	machineInit(8, 44100, 16, 1);

	/* Draws logo screen */
	/*
	drawSprite (splash, 0, 0, 0, 0, 320, 240, 320, 240);
	setPalette (splash_pal);
	videoFlip(0);
	drawSprite (splash, 0, 0, 0, 0, 320, 240, 320, 240);
	videoFlip(0);
	timerDelay(1500);
	*/

	if (argc < 2)
	{
		/*
		FILE *fp;
		int i;
		getcwd (exe_path, 256);

		for (i = 0; i < 256; i++) romspath[i] = 0;

		fp = fopen ("ws4all.cfg", "r");

		if (fp != NULL)
		{
			fscanf(fp, "%s", romspath, &i);
			fclose(fp);
		}
		else
		{
			sprintf(romspath, "roms");
		}

		if (chdir(romspath) != 0) chdir("roms");

		getcwd (current_path, 256);
		*/
	}
	else
	{
		/* Do configuration */
		do_config(argc, argv);
		//gp2x_joystick_init(option.inputs);
		//joystickInit(option.inputs);
		/* Make copy of game filename */
		strcpy(cart.file_name, argv[1]);

		int crc = get_crc (cart.file_name);
		int romnum = WS_DAT_LookFor(crc);

		if (romnum > -1)
		{
			sprintf (cart.game_name, "%s", WS_DAT_getname(romnum));
#ifdef DEBUG
			printf ("%s\n", WS_DAT_getname(romnum));
			printf ("ROM found on database\n");
#endif
		}
		else
		{
			sprintf (cart.game_name, "%s", cart.file_name);
		}


		commandline = 1;
	}

	//romy = 0;
	//romindex = 0;
menu:

	//if (!commandline) exit = menu();

	//if (exit) goto exitemu;

	/* Attempt to load game off commandline */
#ifdef DEBUG
	printf ("File name: %s\n", cart.file_name);
	printf ("Game name: %s\n", cart.game_name);
#endif

	u32 load_ok;
	free (base_rom);
	load_ok = load_rom(cart.file_name, &base_rom, &romSize);

	if (!load_ok)
	{
#ifdef DEBUG
		printf("ERROR: File %s is not valid.\n", argv[1]);
#endif
		return 1;
	}

#ifdef DEBUG
	printf("\nOptions:\n");
	printf("Speed: %d Mhz\n", option.speed);
	printf("Color scheme: %d\n", option.scheme);
	printf("Fullscreen: %d\n", option.fullscreen);
	printf("Sound rate: %d\n", option.sndrate);
	printf("Sound filter: %d\n", option.sndfilter);
	//printf("Sound samples per frame: %d\n\n", soundSamplesPerFrame);
#endif

	setMachineClock(option.speed);
	soundInit (option.sndrate, 16, 1, 75);
	//soundInit (44100, 16, 1, 60);

	SWAN_Init(romSize, base_rom, option.sndrate, option.sndfilter);
	//SWAN_Reset();

#ifdef DEBUG
	unsigned long initial_ticks = timerRead();
	frames_rendered = 0;
	frames_displayed = 0;
#endif

	//gputime = 0;

	u32 done = 0;

	while ( !done )
	{
		if (SWAN_Loop() == 0) done = 1;
		printf ("Salida\n");
#ifdef DEBUG
#ifdef PROFILE
		//if (timerRead() - initial_ticks > 60000) done = 1;
		if (frames_rendered >= 4500) done = 1;
		//if (frames_rendered >= 36000) done = 1;
#endif
#endif
	}

	soundDeInit();

	SWAN_Exit();

#ifdef DEBUG
	extern unsigned int samplesTotales;

	printf ("Samples totales: %u\n", samplesTotales);
	float playtime = (float)(timerRead() - initial_ticks) / (float)1000;
	float avgfps = (float)frames_rendered / playtime;
	//float cpu = playtime - (float)gputime / 1000;
	//float gpu = (float)gputime / 1000;

	//printf("\n[INFO] ROM: %s\n", cart.file_name);
	//printf("[INFO] Speed: %d Mhz, Frameskip: auto\n", option.speed);
	printf("[INFO] Frames rendered = %d (%d%%)\n", frames_rendered, (int)(avgfps * 100 / 75));
	printf("[INFO] Frames displayed: %d (%d%%)\n", frames_displayed, (frames_displayed * 100) / frames_rendered);
	printf("[INFO] Average Frameskip = %.2f\n", ((float)frames_rendered / frames_displayed) - 1);
	printf("[INFO] Average FPS = %.2f (%d%%).\n", avgfps, (int)(avgfps * 100 / 75));
	//printf("[INFO] CPU time: %.2f sec.\n", cpu);
	//printf("[INFO] GPU time: %.2f ms * %d frames displayed = %.2f sec.\n", (float)gputime / frames_displayed, frames_displayed, gpu);
	//printf("[INFO] CPU: %.2f%% / GPU: %.2f%%\n", cpu * 100 / playtime, gpu * 100 / playtime);
	printf("[INFO] Play time = %.2f sec.\n\n", playtime);
#endif

	free (base_rom);
	machineDeInit();

	return 0;
 }
