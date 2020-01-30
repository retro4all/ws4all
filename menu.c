#include <stdio.h>
#include <stdlib.h>

#include <ctype.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ezxml.h"

#include "menu.h"


/* Common files */
#include "font.h"
#include "fontdisable.h"
#include "bar.h"

/* Emulator dependent files */
#include "back.h"
#include "config.h"


#include "hal.h"

/* Options structure */
extern t_option option;
extern t_cart cart;
//extern t_cheat cheats[MAX_CHEATS];
//extern int cheatsEnabled;

t_cheat cheats[MAX_CHEATS];
int cheatsEnabled = 0;

//extern struct usbjoy * joys [4];
int numjoys = 1;

char romspath[256];
char exe_path[256];
char current_path[256];

int romx, romy, romindex;

#define GP2X_NONE 3221487616

// *****************************************************************************
// *******                   UPPERCASE                                   *******
// *****************************************************************************
const char * uppercase (const char * cadena)
{
	int  i = 0;
	char * cadenaout;
	cadenaout = (char *) malloc ((strlen(cadena) + 1) * sizeof(char));

	while ( cadena[i] )
	{
		cadenaout[i] = toupper(cadena[i]);
		i++;
	}

	cadenaout[i] = 0;
	return cadenaout;
}


// *****************************************************************************
// *******                     BUTTON2ASCII                              *******
// *****************************************************************************
char * button2ascii (int button)
{
	switch (button)
	{
		case MACH_B1:
			return "A";
		case MACH_B2:
			return "B";
		case MACH_B3:
			return "X";
		case MACH_B4:
			return "Y";
		case MACH_B5:
			return "L";
		case MACH_B6:
			return "R";
		case MACH_UP:
			return "UP";
		case MACH_DOWN:
			return "DOWN";
		case MACH_LEFT:
			return "LEFT";
		case MACH_RIGHT:
			return "RIGHT";
		case MACH_SELECT:
			return "SELECT";
		case MACH_START:
			return "MENU";
		default:
			return "";
	}
}

// *****************************************************************************
// *******                   SAVEBUTTON                                  *******
// *****************************************************************************
int savebutton (unsigned long pad)
{
	if (pad & MACH_B1) return MACH_B1;

	if (pad & MACH_B2) return MACH_B2;

	if (pad & MACH_B3) return MACH_B3;

	if (pad & MACH_B4) return MACH_B4;

	if (pad & MACH_B5) return MACH_B5;

	if (pad & MACH_B6) return MACH_B6;

	if (pad & MACH_UP) return MACH_UP;

	if (pad & MACH_DOWN) return MACH_DOWN;

	if (pad & MACH_LEFT) return MACH_LEFT;

	if (pad & MACH_RIGHT) return MACH_RIGHT;

	if (pad & MACH_SELECT) return MACH_SELECT;

	if (pad & MACH_START) return MACH_START;

	return 0;
}

/*
int savebutton2 (int joy)
{
	int i;
	int found = -1;

	while (found == -1)
	{
		joy_update (joys[joy]);

		for (i = 0; i < joy_buttons(joys[joy]); i++)
		{
			if (joy_getbutton(i, joys[joy]) == 1) found = i;
		}
	}

	return found;
}
* */


// *****************************************************************************
// *******                 REMAP_BUTTONS                                 *******
// *****************************************************************************
/*
void config_input ()
{
	int done = 0;
	int index = 1;
	unsigned long pad = 0;
	unsigned long last_key = 0;
	char name_inp1[50];
	char name_inp2[50];
	int cursorx = 16;
	int cursory = 75;
	int i;
	int joy1 = 0;
	int joy2 = -1;
	//gp2x_timer_delay(20);

	pad = last_key = joyRead(0);

	while (!done)
	{
		pad = joyRead(0);
		joy_update (joys[0]);

		if ((pad & MACH_SELECT) || (joy_getbutton(0, joys[0]) == 1)) done = 1;

		if (((pad & MACH_UP) && !(pad & MACH_LEFT) && !(pad & MACH_RIGHT)) || joy_getaxe(UP, joys[0]) == 1)
		{
			switch (cursory)
			{
				case 75:
					cursory = 115;
					cursorx = 96;
					break;
				case 85:
					cursory = 75;
					cursorx = 16;
					break;
				case 105:
					cursory = 85;
					cursorx = 96;
					break;
				case 115:
					cursory = 105;
					cursorx = 16;
					break;
			}
		}

		if (((pad & MACH_DOWN) && !(pad & MACH_LEFT) && !(pad & MACH_RIGHT)) || joy_getaxe(DOWN, joys[0]) == 1)
		{
			switch (cursory)
			{
				case 75:
					cursory = 85;
					cursorx = 96;
					break;
				case 85:
					cursory = 105;
					cursorx = 16;
					break;
				case 105:
					cursory = 115;
					cursorx = 96;
					break;
				case 115:
					cursory = 75;
					cursorx = 16;
					break;
			}
		}

		if (((pad & MACH_RIGHT) && !(pad & MACH_UP) && !(pad & MACH_DOWN)) || pad & MACH_B2 || joy_getaxe(RIGHT, joys[0]) == 1)
		{
			if (numjoys > 1)
			{
				int found = 0;

				if (cursory == 75)
				{
					while (!found)
					{
						joy1 = (joy1 + 1) % 5;

						if ((joy1 > 0) && (joys[joy1-1] != NULL) && (joy1 != joy2)) found = 1;
						else if ((joy1 == 0) && (joy1 != joy2)) found = 1;
					}
				}
				else
				{
					while (!found)
					{
						joy2 = (joy2 + 1) % 5;

						if ((joy2 > 0) && (joys[joy2-1] != NULL) && (joy2 != joy1)) found = 1;
						else if ((joy2 == 0) && (joy1 != joy2)) found = 1;
					}
				}
			}

			if (cursory == 85) remap_buttons (joy1, 0);

			if (cursory == 115 && joy2 != -1) remap_buttons (joy2, 1);
		}

		if (joy1 == 0) sprintf (name_inp1, "GP2X JOYSTICK");
		else sprintf (name_inp1, uppercase(joy_name(joys[joy1-1])));

		if (joy2 == 0) sprintf (name_inp2, "GP2X JOYSTICK");
		else if (joy2 > 0) sprintf (name_inp2, uppercase(joy_name(joys[joy2-1])));
		else sprintf (name_inp2, "NONE");

		drawSprite (back, 0, 0, 0, 0, 320, 240, 320, 240);

		drawText (font, 112, 45, "CONFIGURE INPUT", 1);
		drawText (font, 112, 53, "-------------", 1);

		drawText (font, 32, 75, "P1 INPUT:", 0);
		drawText (font, 112, 75, name_inp1, 0);
		drawText (font, 112, 85, "REMAP BUTTONS", 0);
		drawText (font, 32, 105, "P2 INPUT:", 0);
		drawText (font, 112, 105, name_inp2, 0);
		drawText (font, 112, 115, "REMAP BUTTONS", 0);

		drawText (font, cursorx, cursory, "->", 0);

		drawText (font, 112, 215, "-PUSH SELECT WHEN DONE-", 1);

		gp2x_video_RGB_flip(0);
		gp2x_timer_delay(50);
	}

	if (joy2 > -1) option.players = 2;

	if (joy1 == 0) option.inputs[0].joytype = INTJOY;
	else
	{
		option.inputs[0].joytype = USBJOY;
		option.inputs[0].joy = joys[joy1-1];
	}

	if (joy2 == 0) option.inputs[1].joytype = INTJOY;
	else
	{
		option.inputs[1].joytype = USBJOY;
		option.inputs[1].joy = joys[joy2-1];
	}
}


void remap_buttons (int joy, int p)
{
	int done = 0;
	int index = 1;
	unsigned long pad = 0;
	unsigned long last_key = 0;
	gp2x_timer_delay(20);

	if (joy == 0)
	{
		pad = last_key = joyRead(0);

		while (!done)
		{
			pad = joyRead(0);

			if ((pad != last_key) && (pad != GP2X_NONE))
			{
				switch (index)
				{
					case 1:
						option.inputs[p].maps[MAP_UP]    = savebutton (pad);
						index++;
						break;
					case 2:
						option.inputs[p].maps[MAP_DOWN]  = savebutton (pad);
						index++;
						break;
					case 3:
						option.inputs[p].maps[MAP_LEFT]  = savebutton (pad);
						index++;
						break;
					case 4:
						option.inputs[p].maps[MAP_RIGHT] = savebutton (pad);
						index++;
						break;
					case 5:
						option.inputs[p].maps[MAP_B1]    = savebutton (pad);
						index++;
						break;
					case 6:
						option.inputs[p].maps[MAP_B2]    = savebutton (pad);
						index++;
						break;
					case 7:
						option.inputs[p].maps[MAP_PAUSE] = savebutton (pad);
						index++;
						break;
				}
			}

			last_key = pad;

			drawSprite (back, 0, 132, 0, 132, 320, 75, 320, 240);

			if (index > 0)
			{
				drawText (font, 70, 135, "PUSH UP BUTTON:", 0);
				drawText (font, 220, 135, button2ascii(option.inputs[p].maps[MAP_UP]), 0);
			}

			if (index > 1)
			{
				drawText (font, 70, 145, "PUSH DOWN BUTTON:", 0);
				drawText (font, 220, 145, button2ascii(option.inputs[p].maps[MAP_DOWN]), 0);
			}

			if (index > 2)
			{
				drawText (font, 70, 155, "PUSH LEFT BUTTON:", 0);
				drawText (font, 220, 155, button2ascii(option.inputs[p].maps[MAP_LEFT]), 0);
			}

			if (index > 3)
			{
				drawText (font, 70, 165, "PUSH RIGHT BUTTON:", 0);
				drawText (font, 220, 165, button2ascii(option.inputs[p].maps[MAP_RIGHT]), 0);
			}

			if (index > 4)
			{
				drawText (font, 70, 175, "PUSH BUTTON 1:", 0);
				drawText (font, 220, 175, button2ascii(option.inputs[p].maps[MAP_B1]), 0);
			}

			if (index > 5)
			{
				drawText (font, 70, 185, "PUSH BUTTON 2:", 0);
				drawText (font, 220, 185,  button2ascii(option.inputs[p].maps[MAP_B2]), 0);
			}

			if (index > 6)
			{
				drawText (font, 70, 195, "PUSH PAUSE BUTTON:", 0);
				drawText (font, 220, 195,  button2ascii(option.inputs[p].maps[MAP_PAUSE]), 0);
			}

			if (index == 8) done = 1;

			gp2x_video_RGB_flip(0);
			gp2x_timer_delay(50);
		}
	}
	else
	{
		int joypause = 1;
		int i;
		char button [3];

		while (!done)
		{
			joy_update (joys[joy-1]);

			if (!joypause)
			{
				joypause = 1;

				switch (index)
				{
					case 1:
						option.inputs[p].maps[MAP_B1]     = savebutton2(joy - 1);
						index++;
						break;
					case 2:
						option.inputs[p].maps[MAP_B2]     = savebutton2(joy - 1);
						index++;
						break;
					case 3:
						option.inputs[p].maps[MAP_PAUSE]  = savebutton2(joy - 1);
						index++;
						break;
				}
			}
			else
			{
				for (i = 0; i < joy_buttons(joys[joy-1]); i++)
				{
					joypause = joy_getbutton(i, joys[joy-1]);
				}
			}

			drawSprite (back, 0, 132, 0, 132, 320, 75, 320, 240);

			if (index > 0)
			{
				drawText (font, 70, 175, "PUSH BUTTON 1:", 0);
				sprintf (button, "%d", option.inputs[p].maps[MAP_B1]);
				drawText (font, 220, 175, button, 0);
			}

			if (index > 1)
			{
				drawText (font, 70, 185, "PUSH BUTTON 2:", 0);
				sprintf (button, "%d", option.inputs[p].maps[MAP_B2]);
				drawText (font, 220, 185, button, 0);
			}

			if (index > 2)
			{
				drawText (font, 70, 195, "PUSH PAUSE BUTTON:", 0);
				sprintf (button, "%d", option.inputs[p].maps[MAP_PAUSE]);
				drawText (font, 220, 195, button, 0);
			}

			if (index == 4) done = 1;

			gp2x_video_RGB_flip(0);
			gp2x_timer_delay(50);
		}
	}
}
*/


void remap_buttons (char * back, char * font) 
{
	int done = 0;
	int index = 1;
	unsigned long pad = 0;
	unsigned long last_key = 0;
	timerDelay(20);
	pad = last_key = joyRead(0);
	while (!done) 
	{
		pad = joyRead(0);
		if ((pad != last_key) && (pad != 0)) 
		{
			switch (index) 
			{
				case 1:
					option.inputs[0].maps[MAP_XU]    = savebutton (pad); index++; 
					break;
				case 2:
					option.inputs[0].maps[MAP_XD]  = savebutton (pad); index++; 
					break;
				case 3:
					option.inputs[0].maps[MAP_XL]  = savebutton (pad); index++; 
					break;
				case 4:
					option.inputs[0].maps[MAP_XR] = savebutton (pad); index++; 
					break;
				case 5:
					option.inputs[0].maps[MAP_YU]    = savebutton (pad); index++; 
					break;
				case 6:
					option.inputs[0].maps[MAP_YD]    = savebutton (pad); index++;
					break;
				case 7:
					option.inputs[0].maps[MAP_YL] = savebutton (pad); index++; 
					break;
				case 8:
					option.inputs[0].maps[MAP_YR]    = savebutton (pad); index++; 
					break;
				case 9:
					option.inputs[0].maps[MAP_A]  = savebutton (pad); index++; 
					break;
				case 10:
					option.inputs[0].maps[MAP_B]  = savebutton (pad); index++; 
					break;
				case 11:
					option.inputs[0].maps[MAP_START] = savebutton (pad); index++; 
					break;					
			}
		}
		last_key = pad;

		drawSprite (back, 0, 0, 0, 0, 320, 240, 320, 240);

		drawText (font, 112, 45, "REMAP BUTTONS",1);
		drawText (font, 112, 53, "-------------",1);

		if (index > 0) {
		  drawText (font, 50, 75, "PUSH X UP BUTTON:",0);
		  drawText (font, 220, 75, button2ascii(option.inputs[0].maps[MAP_XU]),0);
		}
		if (index > 1) {
		  drawText (font, 50, 85, "PUSH X DOWN BUTTON:",0);
		  drawText (font, 220, 85, button2ascii(option.inputs[0].maps[MAP_XD]),0);
		}
		if (index > 2) {
		  drawText (font, 50, 95, "PUSH X LEFT BUTTON:",0);
		  drawText (font, 220, 95, button2ascii(option.inputs[0].maps[MAP_XL]),0);
		}
		if (index > 3) {
		  drawText (font, 50, 105, "PUSH X RIGHT BUTTON:",0);
		  drawText (font, 220, 105, button2ascii(option.inputs[0].maps[MAP_XR]),0);
		}
		if (index > 4) {
		  drawText (font, 50, 115, "PUSH Y UP BUTTON:",0);
		  drawText (font, 220, 115, button2ascii(option.inputs[0].maps[MAP_YU]),0);
		}
		if (index > 5) {
		  drawText (font, 50, 125, "PUSH Y DOWN BUTTON:",0);
		  drawText (font, 220, 125, button2ascii(option.inputs[0].maps[MAP_YD]),0);
		}
		if (index > 6) {
		  drawText (font, 50, 135, "PUSH Y LEFT BUTTON:",0);
		  drawText (font, 220, 135, button2ascii(option.inputs[0].maps[MAP_YL]),0);
		}
		if (index > 7) {
		  drawText (font, 50, 145, "PUSH Y RIGHT BUTTON:",0);
		  drawText (font, 220, 145, button2ascii(option.inputs[0].maps[MAP_YR]),0);
		}	
		if (index > 8) {
		  drawText (font, 50, 155, "PUSH A BUTTON:",0);
		  drawText (font, 220, 155, button2ascii(option.inputs[0].maps[MAP_A]),0);
		}
		if (index > 9) {
		  drawText (font, 50, 165, "PUSH B BUTTON:",0);
		  drawText (font, 220, 165,  button2ascii(option.inputs[0].maps[MAP_B]),0);
		}
		if (index > 10) {
		  drawText (font, 50, 175, "PUSH START BUTTON:",0);
		  drawText (font, 220, 175,  button2ascii(option.inputs[0].maps[MAP_START]),0);
		}

		if (index == 12) done = 1;
		videoFlip(0);
		timerDelay(100);
	}
}


void selectPalette (char * back, char * font) 
{
	int done = 0;
	unsigned long pad = 0;
	unsigned long last_key = 0;
	int n, i, j, k;
	struct dirent **namelist; // Vector de elementos del tipo dirent. Esta estructura almacena el nombre del fichero.
	char palettes[250][256];
	char sel_palette[256];
	char path[256];
	int npalettes = 0;
	int palettex = 0, palettey = 0, paletteindex = 0;
	t_img_rect image;
	int blackIndex = 0;
	FILE * fd;
	unsigned char paleta[16][3];
	int edicion = 0;

	#define PAL0 200
	
	memset(paleta, 0, sizeof(paleta));

	pad = last_key = joyRead(0);

	for (i = 0; i < 256; i++) path[i] = 0;

	strcpy(path, "marquees/!none.png");

	// Obtención de la lista de ficheros (n contiene el número de ficheros)
	n = scandir("palettes", &namelist, NULL, alphasort);

	for (i = 0; i < 250; i++)
	{
		for (j = 0; j < 256; j++)
		{
			palettes[i][j] = 0;
		}
	}

	// Recorrido por la lista
	i = 0;
	k = 0;

	while (i < n)
	{
		j = strlen(namelist[i]->d_name);

		if (strstr(namelist[i]->d_name, ".pal"))
		{
			npalettes++;
			strncat (palettes[k], namelist[i]->d_name, strlen(namelist[i]->d_name));
			k++;
		}

		i++;
	}

	//Libera memoria
	i = 0;

	while (i < n)
	{
		free(namelist[i]);
		i++;
	}

	while (!done)
	{
		pad = joyRead(0);

		if (pad & MACH_UP)
		{
			if (palettey > 0) palettey--;
			else
			{
				if (paletteindex > 0) paletteindex--;
			}
			
			strncat (sel_palette, palettes[palettey+paletteindex], strlen(palettes[palettey+paletteindex]));

			for (i = 0; i < 256; i++) path[i] = 0;

			sprintf(path, "palettes/%s", palettes[palettey+paletteindex]);
			
			// Carga paleta
			fd = fopen (path, "r");
			printf ("\nPaleta: %s\n", path);

			for (i = 0; i < 16; i++)
			{
				int r, g, b;
				//fscanf(fd, "%u,%u,%u\n", &paleta[i][0], &paleta[i][1], &paleta[i][2]);
				fscanf(fd, "%u,%u,%u\n", &r, &g, &b);
				paleta[i][0] = r;
				paleta[i][1] = g;
				paleta[i][2] = b;
				setPaletteColor (paleta[i][0], paleta[i][1], paleta[i][2], i + PAL0);
				printf ("Color %.2d -> R: %u, G: %u, B: %u\n", i, paleta[i][0], paleta[i][1], paleta[i][2]);
			}
			
			updatePalette();

			fclose(fd);

			//pause = 5;
		}

		if (pad & MACH_DOWN)
		{
			if (palettey < 6) palettey++;
			else
			{
				if (palettey + paletteindex + 1 < npalettes) paletteindex++;
			}
			
			strncat (sel_palette, palettes[palettey+paletteindex], strlen(palettes[palettey+paletteindex]));

			for (i = 0; i < 256; i++) path[i] = 0;

			sprintf(path, "palettes/%s", palettes[palettey+paletteindex]);
			
			// Carga paleta
			fd = fopen (path, "r");
			printf ("\nPaleta: %s\n", path);

			for (i = 0; i < 16; i++)
			{
				int r, g, b;
				//fscanf(fd, "%u,%u,%u\n", &paleta[i][0], &paleta[i][1], &paleta[i][2]);
				fscanf(fd, "%u,%u,%u\n", &r, &g, &b);
				paleta[i][0] = r;
				paleta[i][1] = g;
				paleta[i][2] = b;
				setPaletteColor (paleta[i][0], paleta[i][1], paleta[i][2], i + PAL0);
				printf ("Color %.2d -> R: %u, G: %u, B: %u\n", i, paleta[i][0], paleta[i][1], paleta[i][2]);
			}
			
			updatePalette();

			fclose(fd);

			//pause = 5;
		}

		//}

		//printf ("CICLO\n");
		if (pad & MACH_START)
		{
			done = 1;
		}

		if (pad & MACH_B5)
		{
			if (paletteindex > 6) paletteindex -= 7;
			else paletteindex = 0;
		}

		if (pad & MACH_B6)
		{
			if (npalettes - paletteindex > 13) paletteindex += 7;
			else if (npalettes >= 7) paletteindex = npalettes - 7;
		}

		/* Boton B -> Acepta paleta */
		if ((pad != last_key) && pad & MACH_B2)
		{
			/*
			strncat (sel_palette, palettes[palettey+paletteindex], strlen(palettes[palettey+paletteindex]));

			for (i = 0; i < 256; i++) path[i] = 0;

			sprintf(path, "palettes/%s", palettes[palettey+paletteindex]);
			*/
			
			for (i = 0; i < 16; i++)
			{
				option.shades[i][0] = paleta[i][0];
				option.shades[i][1] = paleta[i][1];
				option.shades[i][2] = paleta[i][2];
			}
			done = 1;
		}

		/* Boton Y -> Preview */
		if (pad & MACH_B4)
		{
			strncat (sel_palette, palettes[palettey+paletteindex], strlen(palettes[palettey+paletteindex]));

			for (i = 0; i < 256; i++) path[i] = 0;

			sprintf(path, "palettes/%s", palettes[palettey+paletteindex]);
			
			// Carga paleta
			fd = fopen (path, "r");
			printf ("\nPaleta: %s\n", path);

			for (i = 0; i < 16; i++)
			{
				int r, g, b;
				//fscanf(fd, "%u,%u,%u\n", &paleta[i][0], &paleta[i][1], &paleta[i][2]);
				fscanf(fd, "%u,%u,%u\n", &r, &g, &b);
				paleta[i][0] = r;
				paleta[i][1] = g;
				paleta[i][2] = b;
				setPaletteColor (paleta[i][0], paleta[i][1], paleta[i][2], i + PAL0);
				printf ("Color %.2d -> R: %u, G: %u, B: %u\n", i, paleta[i][0], paleta[i][1], paleta[i][2]);
			}
			
			updatePalette();

			fclose(fd);
	
			
			/*
			while (pad & MACH_B4)
			{
				pad = joyRead(0);
			}
			*/

		}

		//    if (pad & GP2X_PUSH) if (pad & MACH_B3) delete_rom (marqueey+marqueeindex, marquees);

		last_key = pad;

		drawSprite (back, 0, 0, 0, 0, 320, 240, 320, 240);
		
		// Pinta paletas
		//drawRect (int x1, int y1, int x2, int y2, char color);
		
		int filax, filay;
		for (filax = 35, filay = 69, i = 0; filax <= 35 + 224; filax += 32, i++)
		{
			drawRect(filax, filay, 26, 26, 244);
			drawFillRect(filax + 1, filay + 1, 24, 24, PAL0 + i);
			
			if (filax == 35 + 224 && filay == 69)
			{
				filax = 3;
				filay += 32;
			}
		}

		drawText (font, 112, 45, "SELECT PALETTE", 1);
		drawText (font, 112, 53, "------------", 1);

		int y = 135;

		if (edicion)
		{
		
		}
		else
		{
			for (i = paletteindex; i < paletteindex + 7; i++) // 6 lineas
			{
				drawText (font, 32, y, (char *) uppercase(palettes[i]), 0);
				y += 10;
				//printf("%s\n", palettes[i]);
			}
			
			//drawText (font, 112, 215, "-Y EDIT, X NEW, B SELECT-", 1);
			drawText (font, 112, 215, "-B SELECT-", 1);

			//drawText (font, palettex, palettey * 10 + 75, "->\0", 0);
			drawText (font, palettex, palettey * 10 + 135, "->\0", 0);
		}

		//if (pause) pause--;
		//    gp2x_video_waithsync();
		//gp2x_video_waitvsync();
		videoFlip(0);
		timerDelay(50);

	}
}

// *****************************************************************************
// *******                   DELETE_ROM                                  *******
// *****************************************************************************
/*
void delete_rom (int index, char roms[1200][256], char current_path[256]) {
  int i;
  char path[256];
  for (i=0; i<256; i++) path[i] = 0;
  sprintf(path, "%s/%s", current_path, roms[index]);
  // Borra rom roms[index]
  remove (path);
  sync();
#ifdef DEBUG
  printf ("Borrar rom: %s\n", path);
#endif
  i = index;
  while (roms[i][0] != 0) {
    strncpy (roms[i], roms[i+1], 256);
    i++;
  }
}
*/
void delete_rom (int index, troms ** roms, int * nroms)
{
	int i;
	char path[256];

	for (i = 0; i < 256; i++) path[i] = 0;

	sprintf(path, "%s/%s", current_path, (*roms)[index].file_name);
	// Borra rom roms[index]
	remove (path);
	sync();
#ifdef DEBUG
	printf ("Borrar rom: %s\n", path);
#endif

	//i = index;
	for (i = index; i < (*nroms) - 1; i++)
	{
		memcpy (*roms + i, *roms + i + 1, sizeof (troms));
	}

	memset (*roms + (*nroms) - 1, 0, sizeof (troms));
	(*nroms)--;
}

// *****************************************************************************
// *******                  SELECT_MARQUEE                               *******
// *****************************************************************************
//char * select_marquee (char * back, char * font, game_cfg * cfg) {
#ifdef USEMARQUEES
char * select_marquee (char * back, char * font)
{
	int done = 0;
	unsigned long pad = 0;
	unsigned long last_key = 0;
	int n, i, j, k;
	struct dirent **namelist; // Vector de elementos del tipo dirent. Esta estructura almacena el nombre del fichero.
	char marquees[250][256];
	char sel_marquee[256];
	char path[256];
	int nmarquees = 0;
	int marqueex = 0, marqueey = 0, marqueeindex = 0;
	t_img_rect image;
	int blackIndex = 0;

#ifdef GP2X
	unsigned long black = gp2x_video_YUV_color(0, 0, 0);
#elif WIZ
	unsigned short * back16 = (unsigned short *) back8;
#endif

	for (i = 0; i < 320 * 240; i++)
	{
#ifdef GP2X		
		gp2x_video_YUV[0].screen32[i] = black;
#elif WIZ
		back16[i] = 0;
#endif
	}

#ifdef GP2X
	gp2x_video_YUV_flip(0);
#elif WIZ
	videoFlip(1);
#endif

	pad = last_key = joyRead(0);

	for (i = 0; i < 256; i++) path[i] = 0;

	strcpy(path, "marquees/!none.png");

	// Obtención de la lista de ficheros (n contiene el número de ficheros)
	n = scandir("marquees", &namelist, NULL, alphasort);

	for (i = 0; i < 250; i++)
	{
		for (j = 0; j < 256; j++)
		{
			marquees[i][j] = 0;
		}
	}

	// Recorrido por la lista
	i = 0;
	k = 0;

	while (i < n)
	{
		j = strlen(namelist[i]->d_name);

		if (strstr(namelist[i]->d_name, ".png"))
		{
			nmarquees++;
			strncat (marquees[k], namelist[i]->d_name, strlen(namelist[i]->d_name));
			k++;
		}

		i++;
	}

	//Libera memoria
	i = 0;

	while (i < n)
	{
		free(namelist[i]);
		i++;
	}

	while (!done)
	{
		pad = joyRead(0);

		if (pad & MACH_UP)
		{
			if (marqueey > 0) marqueey--;
			else
			{
				if (marqueeindex > 0) marqueeindex--;
			}

			//pause = 5;
		}

		if (pad & MACH_DOWN)
		{
			if (marqueey < 12) marqueey++;
			else
			{
				if (marqueey + marqueeindex + 1 < nmarquees) marqueeindex++;
			}

			//pause = 5;
		}

		//}

		//printf ("CICLO\n");
		if (pad & MACH_START)
		{
			done = 1;
		}

		if (pad & MACH_B5)
		{
			if (marqueeindex > 12) marqueeindex -= 13;
			else marqueeindex = 0;
		}

		if (pad & MACH_B6)
		{
			if (nmarquees - marqueeindex > 25) marqueeindex += 13;
			else if (nmarquees >= 13) marqueeindex = nmarquees - 13;
		}

		if ((pad != last_key) && pad & MACH_B2)
		{
			strncat (sel_marquee, marquees[marqueey+marqueeindex], strlen(marquees[marqueey+marqueeindex]));

			for (i = 0; i < 256; i++) path[i] = 0;

			sprintf(path, "marquees/%s", marquees[marqueey+marqueeindex]);
			done = 1;
		}

		if (pad & MACH_B4)
		{
			strncat (sel_marquee, marquees[marqueey+marqueeindex], strlen(marquees[marqueey+marqueeindex]));

			for (i = 0; i < 256; i++) path[i] = 0;

			sprintf(path, "marquees/%s", marquees[marqueey+marqueeindex]);

#ifdef GP2X
			gp2x_video_RGB_setcolorkey(0, 0, 0);
			gp2x_video_RGB_setwindows(0x10, -1, -1, -1, 319, 239);
			gp2x_video_YUV_setparts(0, -1, -1, -1, 319, 239);
			gp2x_video_YUV_setscaling(0, 320, 240);
#elif WIZ
			setBackLayer(1, 16);
#endif

			printf ("Path: %s\n", path);
#ifdef GP2X
			gp2x_loadPNG(path, &image, 32, 1);
#elif WIZ
			loadPNG(path, &image, 16, 1);
#endif
			memset(screen8, blackIndex, 320 * 240);
			videoFlip(0);
#ifdef GP2X
			memcpy (gp2x_video_YUV[0].screen32, image.data, 320 * 240 * 4);
			gp2x_video_YUV_flip(0);
#elif WIZ
			memcpy (back8, image.data, 320*240*2);
			videoFlip(1);
			memcpy (back8, image.data, 320*240*2);
			videoFlip(1);
#endif

			while (pad & MACH_B4)
			{
				pad = joyRead(0);
			}

#ifdef WIZ
			back16 = (unsigned short *) back8;
#endif
			for (i = 0; i < 320 * 240; i++)
			{
#ifdef GP2X		
				gp2x_video_YUV[0].screen32[i] = black;
#elif WIZ
				back16[i] = 0;
#endif
			}
			
#ifdef GP2X
			gp2x_video_YUV_flip(0);
#elif WIZ
			videoFlip(1);
#endif			
		}

		//    if (pad & GP2X_PUSH) if (pad & MACH_B3) delete_rom (marqueey+marqueeindex, marquees);

		last_key = pad;

		drawSprite (back, 0, 0, 0, 0, 320, 240, 320, 240);

		drawText (font, 112, 45, "SELECT MARQUEE", 1);
		drawText (font, 112, 53, "------------", 1);

		int y = 75;

		for (i = marqueeindex; i < marqueeindex + 13; i++)
		{
			drawText (font, 32, y, (char *) uppercase(marquees[i]), 0);
			y += 10;
			//printf("%s\n", marquees[i]);
		}

		drawText (font, 112, 215, "-Y TO PREVIEW B TO SELECT-", 1);

		drawText (font, marqueex, marqueey * 10 + 75, "->\0", 0);

		//if (pause) pause--;
		//    gp2x_video_waithsync();
		//gp2x_video_waitvsync();
		videoFlip(0);
		timerDelay(50);

	}

	return path;
}
#endif

int selectCheats (char * back, char * font, int crc)
{
	int done = 0;
	unsigned long pad = 0;
	unsigned long last_key = 0;
	int n, i, j, k;

	char marquees[250][256];
	char sel_marquee[256];
	char path[256];
	int nCheats = 0;
	int cheatX = 18, cheatY = 0, cheatIndex = 0;

	pad = last_key = joyRead(0);

	memset (cheats, 0, MAX_CHEATS * sizeof(t_cheat));
	
	// Load the cheats' XML file
	ezxml_t parentCheats = ezxml_parse_file("data/cheats.xml"), game, cheat;
	const char *gamename;
	int gamecrc;
	
	for (game = ezxml_child(parentCheats, "game"); game; game = game->next)
	{
		gamename = ezxml_attr(game, "name");
		//gamecrc = ezxml_attr(game, "crc");
		sscanf(ezxml_attr(game, "crc"), "%x", &gamecrc);
		printf ("Game CRC: %x\n", gamecrc);
		if (crc == gamecrc)
		{			
			char * pEnd;
			int address = 0;
			int value = 0;
			for (cheat = ezxml_child(game, "cheat"); cheat; cheat = cheat->next)
			{
				//printf("%s, %s: %s\n", ezxml_child(cheat, "description")->txt, gamename,
				//	   ezxml_child(cheat, "code")->txt);
				address = strtol (ezxml_child(cheat, "code")->txt, &pEnd, 16);
				if (!address) continue;
				value = strtol (pEnd+1, NULL, 16);
				if (!value) continue;
				strncpy (cheats[nCheats].description, ezxml_child(cheat, "description")->txt, 20);
				cheats[nCheats].address = address;
				cheats[nCheats].value = value;
				printf ("%s -> Address: %d (%x) - Value: %d (%x)\n", cheats[nCheats].description, cheats[nCheats].address, 
						cheats[nCheats].address, cheats[nCheats].value, cheats[nCheats].value);
				nCheats++;
			}
			break;
		}
	}
	ezxml_free(parentCheats);
	
#ifdef DEBUG
	printf ("%d cheats found!\n", nCheats);
#endif
	

	while (!done)
	{
		pad = joyRead(0);

		if (pad & MACH_UP)
		{
			if (cheatY > 0) cheatY--;
			else
			{
				if (cheatIndex > 0) cheatIndex--;
			}
		}

		if (pad & MACH_DOWN)
		{
			if (cheatY < 12 && cheatY < nCheats -1) cheatY++;
			else
			{
				if (cheatY + cheatIndex + 1 < nCheats) cheatIndex++;
			}
		}

		if (pad & MACH_START)
		{
			return 0;
		}
		
		if (pad & MACH_SELECT)
		{
			done = 1;
		}

		if (pad & MACH_B5)
		{
			if (cheatIndex > 12) cheatIndex -= 13;
			else cheatIndex = 0;
		}

		if (pad & MACH_B6)
		{
			if (nCheats - cheatIndex > 25) cheatIndex += 13;
			else if (nCheats >= 13) cheatIndex = nCheats - 13;
		}

		if ((pad != last_key) && (pad & MACH_B2 || pad & MACH_RIGHT || pad & MACH_LEFT))
		{
			cheats[cheatY].enabled = ~cheats[cheatY].enabled;
		}

		last_key = pad;

		drawSprite (back, 0, 0, 0, 0, 320, 240, 320, 240);

		drawText (font, 112, 45, "SELECT CHEATS", 1);
		drawText (font, 112, 53, "-------------", 1);

		int y = 75;

		for (i = cheatIndex; i < cheatIndex + 13; i++)
		{
			if (cheats[i].address)
			{
				drawText (font, 32, y, (char *) uppercase(cheats[i].description), 0);
				drawText (font, 260, y, cheats[i].enabled ? "ON" : "OFF", 0);
				y += 10;
			}			
		}

		drawText (font, 112, 215, "-START TO RETURN SELECT TO CONTINUE-", 1);

		if (nCheats) drawText (font, cheatX, cheatY * 10 + 75, "->\0", 0);

		videoFlip(0);
		timerDelay(100);
	}

	for (i = 0; i < MAX_CHEATS; i++)
	{
		if (cheats[i].enabled)
			return 1;
	}
	
	return 0;
}

// *****************************************************************************
// *******                     GENERATE_CRC                              *******
// *****************************************************************************

void generate_crc (troms ** roms, int * nroms)
{
	char path[256];
	int i, ii, k, n;
	FILE *fp;
	struct dirent **namelist;
	// Obtención de la lista de ficheros (n contiene el número de ficheros)
	n = scandir(current_path, &namelist, NULL, alphasort);

	free(*roms);
	*roms = malloc (n * sizeof(troms));
	memset (*roms, 0, n * sizeof(troms));
	*nroms = 0;


	// Recorrido por la lista
	i = 0;
	k = 0;

	while (i < n)
	{
		if (namelist[i]->d_type & DT_DIR)
		{
			if (strcmp(namelist[i]->d_name, "."))
			{
#ifdef DEBUG
				printf("Directorio: %s\n", namelist[i]->d_name);
#endif
				(*nroms)++;
				//strncat (roms[k]->file_name, namelist[i]->d_name, strlen(namelist[i]->d_name));
				//strcpy (roms[k]->game_name, roms[k]->file_name);
				strncat ((*roms)[k].file_name, namelist[i]->d_name, strlen(namelist[i]->d_name));
				strcpy ((*roms)[k].game_name, (*roms)[k].file_name);

				if (!strcmp(namelist[i]->d_name, ".."))
				{
					(*roms)[k].type = 2;
				}
				else
				{
					(*roms)[k].type = 1;
				}

				k++;
			}
		}

		i++;
	}

	// Recorrido por la lista
	i = 0;

	while (i < n)
	{
		if (!(namelist[i]->d_type & DT_DIR))
		{
			if (strstr(namelist[i]->d_name, ".wsc") || strstr(namelist[i]->d_name, ".ws") || strstr(namelist[i]->d_name, ".zip"))
			{
#ifdef DEBUG
				printf("%s\n", namelist[i]->d_name);
#endif
				//printf ("File: %s - CRC: %x\n", namelist[i]->d_name, get_crc (namelist[i]->d_name));
				(*nroms)++;
				strncat ((*roms)[k].file_name, namelist[i]->d_name, strlen(namelist[i]->d_name));
				strcpy ((*roms)[k].game_name, (*roms)[k].file_name);
				(*roms)[k].type = 0;
				k++;
			}
		}

		i++;
	}

	//Libera memoria
	i = 0;

	while (i < n)
	{
		free(namelist[i]);
		i++;
	}


	//chdir (exe_path);


	//printf ("Saving CRC - %d Elements: %d bytes\n", *nroms, *nroms*sizeof(troms));
	float incr = (float)228 / ((*nroms) - 1);

	for (i = 1; i != *nroms; i++)
	{
		if ((*roms)[i].type == 0)
		{
			for (ii = 0; ii < 256; ii++) path[ii] = 0;

			sprintf (path, "%s/%s", current_path, (*roms)[i].file_name);
			(*roms)[i].crc = get_crc (path);
			//printf ("Rom seleccionada: %s - CRC: %x\n", sel_rom, crc);
			//printf ("Current path: %s\n", current_path);
			int romnum = WS_DAT_LookFor((*roms)[i].crc);

			if (romnum > -1)
			{
				sprintf ((*roms)[i].game_name, "%s", WS_DAT_getname(romnum));
				//printf ("%s\n", SMS_DAT_getname(romnum));
				//printf ("%s - %s\n", (*roms)[i].file_name, (*roms)[i].game_name);
				//printf ("ROM found on database\n");
			}
		}

		drawTransSprite (bar, 0, 0, 44, 225, 232, 8, 232, 8, 0);
		//drawRect (46, 227, 60, 230, 180);
		
		//drawRect (46, 227, (int)(46 + (float)i * incr), 230, 244);
		drawFillRect (46, 227, (int)((float)i * incr), 3, 244);

		//gp2x_video_RGB_flip(0);
		//gp2x_timer_delay(50);
		videoFlip(0);
		timerDelay(50);
	}

	for (ii = 0; ii < 256; ii++) path[ii] = 0;

	sprintf(path, "%s/roms.crc", current_path);
	fp = fopen (path, "w");
	if (fp)
	{
		fwrite(*roms, *nroms * sizeof(troms), 1, fp);
		fclose(fp);
		sync();
	}

}



// *****************************************************************************
// *******                         MENU                                  *******
// *****************************************************************************
int menu (void)
{
	FILE *fp;
	t_option default_config;
	int done = 0;
	int exit = 0;
	int cursorx;
	int cursory;
	int pause = 0;
	int n, i, j, k, ii;
	struct dirent **namelist; // Esta estructura almacena el nombre del fichero.
	troms * roms;
	troms * romstest;
	char sel_rom[256];
	char path[256];
	char temp_array[256];
	int nroms;
	int joypause1 = 0;
	int joypause2 = 0;

	unsigned long pad, last_key;

/*
	for (i = 0; i < 4; i++)
	{
		if (joys[i] != NULL) numjoys++;
	}
*/

#ifdef GP2X
	setMachineClock(120); // <- Cambiarlo?
	gp2x_video_RGB_setscaling(320, 240);
#endif
#ifdef WIZ
	setMachineClock(120);
#endif

	// Reset background
	drawSprite (back, 0, 0, 0, 0, 320, 240, 320, 240);
	videoFlip(0); //gp2x_video_RGB_flip(0);
	setPalette (back_pal);

#ifdef DEBUG
	printf ("\nSearching ROMs...\n");
#endif

	chdir (current_path);

scandir:
	/*
	romy = 0;
	romindex = 0;
	nroms = 0;
	*/

#ifdef DEBUG
	printf("Ruta: %s\n", getcwd (current_path, 256));
	printf("Prueba: %s\n", current_path);
#endif

	fp = fopen ("roms.crc", "r");

	if (fp != NULL)
	{
		int size;
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		roms = malloc(size);
		fread(roms, size, 1, fp);
		fclose(fp);
		nroms = size / sizeof(troms);
#ifdef DEBUG
		printf ("Loading CRC - %d Elements - %d bytes\n", nroms, size);
#endif
	}
	else
	{

		// Obtención de la lista de ficheros (n contiene el número de ficheros)
		n = scandir(current_path, &namelist, NULL, alphasort);

		roms = malloc (n * sizeof(troms));
		memset (roms, 0, n * sizeof(troms));
		nroms = 0;

		// Recorrido por la lista
		i = 0;
		k = 0;

		while (i < n)
		{
			if (namelist[i]->d_type & DT_DIR)
			{
				if (strcmp(namelist[i]->d_name, "."))
				{
#ifdef DEBUG
					printf("Directorio: %s\n", namelist[i]->d_name);
#endif
					nroms++;
					strncat (roms[k].file_name, namelist[i]->d_name, strlen(namelist[i]->d_name));
					strcpy (roms[k].game_name, roms[k].file_name);

					if (!strcmp(namelist[i]->d_name, ".."))
					{
						roms[k].type = 2;
					}
					else
					{
						roms[k].type = 1;
					}

					k++;
				}
			}

			i++;
		}

		// Recorrido por la lista
		i = 0;

		while (i < n)
		{
			if (!(namelist[i]->d_type & DT_DIR))
			{
				if (strstr(namelist[i]->d_name, ".wsc") || strstr(namelist[i]->d_name, ".ws") || strstr(namelist[i]->d_name, ".zip"))
				{
#ifdef DEBUG
					printf("%s\n", namelist[i]->d_name);
#endif
					//printf ("File: %s - CRC: %x\n", namelist[i]->d_name, get_crc (namelist[i]->d_name));
					nroms++;
					strncat (roms[k].file_name, namelist[i]->d_name, strlen(namelist[i]->d_name));
					strcpy (roms[k].game_name, roms[k].file_name);
					roms[k].type = 0;
					k++;
				}
			}

			i++;
		}

		//Libera memoria
		i = 0;

		while (i < n)
		{
			free(namelist[i]);
			i++;
		}
	}

	chdir (exe_path);


	// Load default config
	fp = fopen ("data/default.cfg", "r");

	if (fp != NULL)
	{
		fread(&default_config, sizeof(t_option), 1, fp);
		fclose(fp);
	}
	else
	{
		set_option_defaults();
		//gp2x_joystick_init(option.inputs);
		//joystickInit(option.inputs);
		memcpy (&default_config, &option, sizeof(t_option));
		fp = fopen ("data/default.cfg", "w");
		if (fp)
		{
			fwrite(&default_config, sizeof(t_option), 1, fp);
			fclose(fp);
		}
	}
	


	//printf ("Loading menu...\n");
	// ******************** FIRST BLOCK - SELECT ROM *********************
	//  romx = 16;
	//romy = 0;
	//romindex = 0;
block1:
	//pause = 20;
	pause = 5;

	// Inicializa sel_rom
	for (i = 0; i < 256; i++) sel_rom[i] = 0;

	pad = last_key = joyRead(0);

	while (!done)
	{
		pad = joyRead(0);
		//joy_update (joys[0]);

		// Scrolls one line up
		if (pad & MACH_UP )//|| joy_getaxe(UP, joys[0]) == 1)
		{
			if (romy > 0) romy--;
			else
			{
				if (romindex > 0) romindex--;
			}
		}

		// Scrolls one line down
		if (pad & MACH_DOWN )//|| joy_getaxe(DOWN, joys[0]) == 1)
		{
			if (roms[romy+romindex+1].file_name[0] != 0)
			{
				if (romy < 12) romy++;
				else
				{
					if (romy + romindex + 1 < nroms) romindex++;
				}
			}
		}

		// Scrolls one page up
		if (pad & MACH_B5 || (pad & MACH_LEFT && !(pad & MACH_UP) && !(pad & MACH_DOWN))
		        )//|| joy_getaxe(LEFT, joys[0]) == 1)
		{
			if (romindex > 12) romindex -= 13;
			else romindex = 0;
		}

		// Scrolls one page down
		if (pad & MACH_B6 || (pad & MACH_RIGHT && !(pad & MACH_UP) && !(pad & MACH_DOWN))
		        )//|| joy_getaxe(RIGHT, joys[0]) == 1)
		{
			if (nroms - romindex > 25) romindex += 13;
			else if (nroms >= 13) romindex = nroms - 13;
		}

		// Selects roms or change folder
		if (((pad != last_key) && (pad & MACH_B2)) )//|| joy_getbutton(0, joys[0]) == 1)
		{
			joypause1 = 1;

			if (roms[romy+romindex].type)   // Folder
			{
#ifdef DEBUG
				printf("*****\n");
				printf("Folder selected: %s\n", roms[romy+romindex].file_name);
				printf("Current dir: %s\n", current_path);
#endif
				strncat(current_path, "/", 1);
				strncat(current_path, roms[romy+romindex].file_name, strlen(roms[romy+romindex].file_name));
#ifdef DEBUG
				printf("Current dir modified: %s\n", current_path);
#endif
				chdir (current_path);

				if (roms[romy+romindex].type == 2)   // cd..
				{
					//chdir ("..");
#ifdef DEBUG
					printf("Ruta: %s\n", getcwd (current_path, 256));
#endif
				}

				romy = 0;
				romindex = 0;
				free (roms);
				goto scandir;
			}
			else
			{
				if (roms[romy+romindex].file_name[0] != 0)
				{
					strncat (sel_rom, roms[romy+romindex].file_name, strlen(roms[romy+romindex].file_name));
					done = 1;
				}
			}
		}

		// Limit joystick push event
		//if (!joy_getbutton(0, joys[0])) joypause1 = 0;

		//if (!joy_getbutton(1, joys[0])) joypause2 = 0;

		// Make CRC scan
		if ((pad & MACH_SELECT) && (pad != last_key))
		{
			generate_crc (&roms, &nroms);
			romy = 0;
			romindex = 0;
		}

		// Deletes current rom
		if (pad & MACH_B1) if ((pad != last_key) && (pad & MACH_B3))
		{
			delete_rom (romy + romindex, &roms, &nroms);
		}

		// Saves default rom path
		if (pad & MACH_B1) if ((pad != last_key) && (pad & MACH_B4))
		{
			for (ii = 0; ii < 256; ii++) temp_array[ii] = 0;

			sprintf(temp_array, "%s/ws4all.cfg", exe_path);
#ifdef DEBUG
			printf("%s\n", temp_array);
			printf("%s\n", current_path);
#endif
			fp = fopen (temp_array, "w");
			if (fp)
			{
				fwrite(&current_path, strlen(current_path), 1, fp);
				fclose(fp);
				sync();
			}
		}

		// Exits menu
		if (((pad != last_key) && (pad & MACH_START)) )//|| (joy_getbutton(1, joys[0]) == 1 && !joypause2))
		{
			return 1;
		}

		drawSprite (back, 0, 0, 0, 0, 320, 240, 320, 240);

		drawText (font, 112, 43, "SELECT A ROM", 1);
		drawText (font, 112, 51, "------------", 1);

		int y = 75;

		//printf ("Rom index: %d\n", romindex);
		for (i = romindex; (i < romindex + 13) && (i < nroms); i++)
		{
			if (roms[i].type)   // Folder
			{
				for (ii = 0; ii < 256; ii++) temp_array[ii] = 0;

				sprintf(temp_array, "<%s>", (char *) uppercase(roms[i].file_name));
				drawText (font, 24, y, temp_array, 0);
			}
			else   // ROM
			{
				drawText (font, 32, y, (char *) uppercase(roms[i].game_name), 0);
			}

			y += 10;
		}

		drawText (font, 112, 215, "-START TO EXIT B TO CONTINUE-", 1);

		if (roms[romy+romindex].type) drawText (font, 8, romy * 10 + 75, "->", 0);
		else drawText (font, 16, romy * 10 + 75, "->\0", 0);

		if (pause) pause--;

		last_key = pad;

		videoFlip(0);
		timerDelay (100);
	}

	// *******************************************************************

	// ****************** SECOND BLOCK - SELECT OPTIONS ******************
	for (i = 0; i < 256; i++) path[i] = 0;

	sprintf (path, "%s/%s", current_path, sel_rom);
	int crc = get_crc (path);
#ifdef DEBUG
	printf ("Rom seleccionada: %s - CRC: %x\n", sel_rom, crc);
	printf ("Current path: %s\n", current_path);
#endif
	int romnum = WS_DAT_LookFor(crc);
	//int romnum = -1;

	if (romnum > -1)
	{
		sprintf (cart.game_name, "%s", WS_DAT_getname(romnum));
		printf ("%s\n", WS_DAT_getname(romnum));
#ifdef DEBUG
		printf ("ROM found on database\n");
#endif
	}
	else
	{
		sprintf (cart.game_name, "%s", sel_rom);
		strcpy(strrchr(cart.game_name, '.'), "\0\0\0\0");
#ifdef DEBUG
		printf ("ROM not found on database\n");
#endif
	}

#ifdef DEBUG
	printf ("ROM Name: %s\n", cart.game_name);
#endif

	for (i = 0; i < 256; i++) path[i] = 0;

	sprintf(path, "data/%s.cfg", cart.game_name);
	fp = fopen (path, "r");

	if (fp != NULL)
	{
		fread(&option, sizeof(t_option), 1, fp);
		fclose(fp);
	}
	else
	{
		memcpy (&option, &default_config, sizeof(t_option));
	}

	char tspeed[8];
	char tz80[9];
	char taratio[8];
	char trotate[9];
	char tsndrate[8];
	char tscheme[8];
	char tsfilter[9];
	//int region = 2 * option.territory + option.display;

	cursorx = 54;
	cursory = 71;
	done = 0;
	pad = last_key = joyRead(0);

	while (!done && !exit)
	{
		pad = joyRead(0);
		//joy_update (joys[0]);

		//    pause = 0;
		if (!pause)
		{
			if (((pad & MACH_UP) && !(pad & MACH_LEFT) && !(pad & MACH_RIGHT))
			         )//||  joy_getaxe(UP, joys[0]) == 1)
			{
				switch (cursory)
				{
					case 71:
						cursory = 191;
						break;
					case 81:
						cursory = 71;
						break;
					case 101:
						cursory = 81;
						break;
					case 111:
						cursory = 101;
						break;
					case 131:
						cursory = 111;
						break;
					case 141:
						cursory = 131;
						break;
					case 161:
						cursory = 141;
						break;
					case 171:
						cursory = 161;
						break;
					case 181:
						cursory = 171;
						break;
					case 191:
						cursory = 181;
						break;
				}

				if (cursory == 161) cursorx = 84;
				else if (cursory == 171) cursorx = 72;
				else if (cursory == 181) cursorx = 92;
				else if (cursory == 191) cursorx = 88;
				else cursorx = 54;

				//pause = 7;
			}

			if (((pad & MACH_DOWN) && !(pad & MACH_LEFT) && !(pad & MACH_RIGHT))
			         )//||  joy_getaxe(DOWN, joys[0]) == 1)
			{
				switch (cursory)
				{
					case 71:
						cursory = 81;
						break;
					case 81:
						cursory = 101;
						break;
					case 101:
						cursory = 111;
						break;
					case 111:
						cursory = 131;
						break;
					case 131:
						cursory = 141;
						break;
					case 141:
						cursory = 161;
						break;
					case 161:
						cursory = 171;
						break;
					case 171:
						cursory = 181;
						break;
					case 181:
						cursory = 191;
						break;
					case 191:
						cursory = 71;
						break;
				}

				if (cursory == 161) cursorx = 84;
				else if (cursory == 171) cursorx = 72;
				else if (cursory == 181) cursorx = 92;
				else if (cursory == 191) cursorx = 88;
				else cursorx = 54;

				//pause = 7;
			}
		}

		if (((pad & MACH_SELECT) && (pad != last_key))  )//|| (joy_getbutton(0, joys[0]) == 1 && !joypause1))
		{
			done = 1;
		}

		if (!pause)
		{
			if (((pad & MACH_LEFT) && !(pad & MACH_UP) && !(pad & MACH_DOWN))
			         )//||  joy_getaxe(LEFT, joys[0]) == 1)
			{
				switch (cursory)
				{
					case 71: // CPU Speed

						switch (option.speed)
						{
							case 100:
#ifdef GP2X							
								option.speed = 250;
#endif								
								break;
							case 110:
								option.speed = 100;
								break;
							case 120:
								option.speed = 110;
								break;
							case 133:
								option.speed = 120;
								break;
							case 140:
								option.speed = 133;
								break;
							case 150:
								option.speed = 140;
								break;
							case 166:
								option.speed = 150;
								break;
							case 180:
								option.speed = 166;
								break;
							case 200:
								option.speed = 180;
								break;
							case 225:
								option.speed = 200;
								break;
							case 233:
								option.speed = 225;
								break;
							case 245:
								option.speed = 233;
								break;
							case 250:
								option.speed = 245;
								break;
							case 533:
								option.speed = 500;
								break;
							case 550:
								option.speed = 533;
								break;
#ifdef WIZ
							default:
								option.speed -= 25;
#endif
						}

						break;
					case 81: // V30 Speed

						//if (option.z80speedmod < 0.2) option.z80speedmod = 2;
						//else option.z80speedmod -= 0.1;

						break;
					case 101: // Aspect ratio

						if (option.fullscreen) option.fullscreen = 0;
						else option.fullscreen = 1;

						break;
					case 111: // Autorotate

						option.autorotate ^= 1;
						
						break;
					case 131: // Sample rate

						if (option.sndrate == 22050) option.sndrate = 48000;
						else if (option.sndrate == 44100) option.sndrate = 22050;
						else option.sndrate = 44100;

						break;
					case 141: // Sound filter

						if (option.sndfilter == 0) option.sndfilter = 1;
						else option.sndfilter--;

						break;
				}

				//pause = 15;
			}

			if ((((pad & MACH_RIGHT) && !(pad & MACH_UP) && !(pad & MACH_DOWN)) || (pad & MACH_B2 && (pad != last_key)))  )//||  joy_getaxe(RIGHT, joys[0]) == 1)
			{
				switch (cursory)
				{
					case 71: // CPU Speed

						switch (option.speed)
						{
							case 100:
								option.speed = 110;
								break;
							case 110:
								option.speed = 120;
								break;
							case 120:
								option.speed = 133;
								break;
							case 133:
								option.speed = 140;
								break;
							case 140:
								option.speed = 150;
								break;
							case 150:
								option.speed = 166;
								break;
							case 166:
								option.speed = 180;
								break;
							case 180:
								option.speed = 200;
								break;
							case 200:
								option.speed = 225;
								break;
							case 225:
								option.speed = 233;
								break;
							case 233:
								option.speed = 245;
								break;
							case 245:
								option.speed = 250;
								break;
							case 500:
								option.speed = 533;
								break;
							case 533:
								option.speed = 550;
								break;
#ifdef GP2X
							case 250:
								option.speed = 100;
								break;
#endif
#ifdef WIZ
							 default:
								option.speed += 25;
#endif
						}

						break;
					case 81: // V30 Speed

						//if (option.z80speedmod > 1.9) option.z80speedmod = 0.1;
						//else option.z80speedmod += 0.1;

						break;
					case 101: // Aspect Ratio

						if (option.fullscreen) option.fullscreen = 0;
						else option.fullscreen = 1;

						break;
					case 111: // Autorotate

						option.autorotate ^= 1;

						break;
					case 131: // Sample Rate

						if (option.sndrate == 22050) option.sndrate = 44100;
						else if (option.sndrate == 44100) option.sndrate = 48000;
						else option.sndrate = 22050;

						break;
					case 141: // Sound filter

						if (option.sndfilter == 1) option.sndfilter = 0;
						else option.sndfilter++;

						break;
					case 161: // Remap Buttons
						remap_buttons (back, font);
						//config_input();
						pad = joyRead(0);
						break;
					case 171: // Color scheme
						selectPalette(back, font);
						pad = joyRead(0);
						
						break;						
					case 181: // Select Cheats
						//cheatsEnabled = selectCheats(back, font, crc);
						break;
					case 191: // Select Marquee
#ifdef USEMARQUEES
						strcpy(option.marquee, select_marquee (back, font));
#endif
						break;
				}

				//pause = 15;
			}

			// Limit joystick push event
			//if (!joy_getbutton(0, joys[0])) joypause1 = 0;

			//if (!joy_getbutton(1, joys[0])) joypause2 = 0;

			if (pad & MACH_B1) if (pad & MACH_B4)
				{
					memcpy (&default_config, &option, sizeof(t_option));
					fp = fopen ("data/default.cfg", "w");
					if (fp)
					{
						fwrite(&default_config, sizeof(t_option), 1, fp);
						fclose(fp);
					}
				}

			if ((pad & MACH_START)  )//|| joy_getbutton(1, joys[0]) == 1)
			{
				joypause2 = 1;
				goto block1;
			}
		}

		drawSprite (back, 0, 0, 0, 0, 320, 240, 320, 240);

		drawText (font, 112, 43, (char *) uppercase(cart.game_name), 1);
		drawText (font, 112, 51, "------------\0", 1);

		drawText (font, cursorx, cursory, "->", 0);

		sprintf (tspeed, "%d MHZ", option.speed);

		sprintf (tz80, "X %.1f", option.z80speedmod);

		if (option.autorotate) sprintf (trotate, "YES");
		else sprintf (trotate, "NO");

		if (option.fullscreen) sprintf (taratio, "FULL");
		else sprintf (taratio, "1:1");

		sprintf (tsndrate, "%d", option.sndrate);

		/*
		if (option.fm_enable == 0) sprintf (tfm, "DISABLED");
		else if (option.fm_enable == 1) sprintf (tfm, "EMU2413");
		else sprintf (tfm, "YM2413");
		*/
		if (option.sndfilter == 0) sprintf(tsfilter, "NONE");
		else sprintf(tsfilter, "LINEAR");

		drawText (font, 70, 71, "CPU SPEED:", 0);
		drawText (font, 210, 71, tspeed, 0);

		drawText (fontdisable, 70, 81, "NEC V30 SPEED:", 0);
		drawText (fontdisable, 210, 81, tz80, 0);

		drawText (font, 70, 101, "ASPECT RATIO:", 0);
		drawText (font, 210, 101, taratio, 0);

		drawText (font, 70, 111, "AUTO ROTATION:", 0);
		drawText (font, 210, 111, trotate, 0);

		drawText (font, 70, 131, "SOUND S. RATE:", 0);
		drawText (font, 210, 131, tsndrate, 0);

		drawText (font, 70, 141, "SOUND FILTER:", 0);
		drawText (font, 210, 141, tsfilter, 0);

		drawText (font, 70, 161, "CONFIGURE INPUT", 1);
		drawText (font, 70, 171, "SELECT WSM PALETTE", 1);
		drawText (fontdisable, 70, 181, "SELECT CHEATS", 1);
		drawText (font, 70, 191, "SELECT MARQUEE", 1);
		//    drawText (fontdisable, 70, 185, "LOAD GAME",1);
		//drawText (fontdisable, 70, 195, "SAVE GAME",1);
		drawText (font, 70, 215, "-START TO RETURN SELECT TO PLAY-", 1);

		if (pause) pause--;

		last_key = pad;
		videoFlip(0);
		timerDelay(100);
	}
	

	// *******************************************************************
	if (!exit)
	{
		for (i = 0; i < 256; i++) path[i] = 0;

		sprintf(path, "%s/%s", current_path, sel_rom);
		strcpy(cart.file_name, path);

		if (numjoys == 1) option.players = 1;

		//option.inputs[1].joy = joys[0];
		//option.inputs[1].joytype = USBJOY;

		//for (i=0; i<320*240; i++) gp2x_video_RGB[0].screen16[i] = 0;
		/*
		memset (gp2x_video_RGB[0].screen8, 2, 320 * 240);
		gp2x_video_RGB_flip(0);
		memset (gp2x_video_RGB[0].screen8, 2, 320 * 240);
		*/
		//for (i=0; i<320*240; i++) gp2x_video_RGB[0].screen16[i] = 0;

		for (i = 0; i < 256; i++) path[i] = 0;

		sprintf(path, "data/%s.cfg", cart.game_name);
		fp = fopen (path, "w");
		if (fp)
		{
			fwrite(&option, sizeof(t_option), 1, fp);
			fclose(fp);
		}
	}

	return exit;

}
