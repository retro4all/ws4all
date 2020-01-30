#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <math.h>

#include "hal.h"

#include "ws.h"

#include "splash.h"

#include "files.h"
#include "types.h"
#include "config.h"

//extern t_option option;

#ifdef DEBUG
extern int frames_rendered;
extern int frames_displayed;
#endif

extern int romx, romy, romindex;

extern char romspath[256];
extern char exe_path[256];
extern char current_path[256];

int master_volume = 100;

int main(int argc, char *argv[])
{
	int commandline = 0;
	int exit = 0;

	unsigned char *base_rom = NULL;
	int romSize;

	printf("WS4ALL %s\n", VERSION);
	printf("(C) Israel Lopez Fernandez (Puck2099)\n");
	printf("Based on e[mulator] (C) e (T.Kawamorita)\n\n");

	int i;

	machineInit(8, 44100, 16, 1);
	
	/* Draws logo screen */
	
	drawSprite (splash, 0, 0, 0, 0, 320, 240, 320, 240);
	setPalette (splash_pal);
	videoFlip(0);
	drawSprite (splash, 0, 0, 0, 0, 320, 240, 320, 240);
	videoFlip(0);	
	timerDelay(1500);
	
	
	if (argc < 2)
	{
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

	romy = 0;
	romindex = 0;
menu:

	if (!commandline) exit = menu();

	if (exit) goto exitemu;

	/* Attempt to load game off commandline */
#ifdef DEBUG
	printf ("File name: %s\n", cart.file_name);
	printf ("Game name: %s\n", cart.game_name);
#endif
	
	int load_ok;
	free (base_rom);
	load_ok = load_rom(cart.file_name, &base_rom, &romSize);

	if (!load_ok)
	{
#ifdef DEBUG		
		printf("ERROR: File %s is not valid.\n", argv[1]);
#endif		
		if (commandline) goto exitemu;
		else goto menu;
	}
	
#ifdef DEBUG
	printf("\nOptions:\n");
	printf("Speed: %d Mhz\n", option.speed);
	printf("Color scheme: %d\n", option.scheme);
	printf("Fullscreen: %d\n", option.fullscreen);
	printf("Sound rate: %d\n", option.sndrate);
	printf("Sound filter: %d\n", option.sndfilter);	
#endif
			
	setMachineClock(option.speed);
	soundInit (option.sndrate, 16, 1, 75);

#if 0
	if (mmu)
	{
		if (hack_the_mmu() == 0)
		{
			printf("Patching MMU ... OK!\n");
		}
		else
		{
			printf("Patching MMU ... FAILED :(\n");
		}
	}
#endif

	SWAN_Init(romSize, base_rom, &option);
	//SWAN_Reset();

#ifdef DEBUG
	frames_rendered = 0;
	frames_displayed = 0;
	unsigned long initial_ticks = timerRead();	
#endif

	SWAN_Loop();

#ifdef DEBUG
	extern unsigned int samplesTotales;
	float playtime = (float)(timerRead() - initial_ticks) / (float)1000;
	float avgfps = (float)frames_rendered / playtime;

	printf("[INFO] Total Sound Samples = %u\n", samplesTotales);	
	printf("[INFO] Frames rendered =  %d (%d%%)\n", frames_rendered, (int)(avgfps * 100 / 75));
	printf("[INFO] Frames displayed = %d (%d%%)\n", frames_displayed, (frames_displayed * 100) / frames_rendered);
	printf("[INFO] Average Frameskip = %.2f\n", ((float)frames_rendered / frames_displayed) - 1);
	printf("[INFO] Average FPS = %.2f (%d%%).\n", avgfps, (int)(avgfps * 100 / 75));
	printf("[INFO] Play time = %.2f sec.\n\n", playtime);
#endif
	
	SWAN_Exit();
	
	if (!commandline) goto menu;

exitemu:
	free (base_rom);
	machineDeInit();

#ifdef DEBUG
	printf ("Exiting...\n");
#endif	

	return(0);
}


