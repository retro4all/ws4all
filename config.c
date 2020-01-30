#include "config.h"


/* Options structure */
extern t_option option;
extern int tweak;

void do_config(int argc, char **argv)
{
	set_option_defaults();

	parse_args(argc, argv);
}


/* Parse configuration file */
/*
int parse_file(const char *filename, int *argc, char **argv)
{
    char token[0x100];
    FILE *handle = NULL;

    *argc = 0;

    handle = fopen(filename, "r");
    if(!handle) return (0);

    fscanf(handle, "%s", &token[0]);
    while(!(feof(handle)))
    {
        int size = strlen(token) + 1;
        argv[*argc] = malloc(size);
        if(!argv[*argc]) return (0);
        strcpy(argv[*argc], token);
        *argc += 1;
        fscanf(handle, "%s", &token[0]);
    }

    if(handle) fclose(handle);
    return (1);
}
*/

void set_option_defaults(void)
{
	option.players      =   1;
	option.sndrate      =   48000;
	option.sndfilter	=	0;
#ifdef GP2X
	option.speed        =   200;
#elif WIZ
	option.speed        =   533;
#endif
	option.fullscreen   =   0;
	option.autorotate   =   1;
	option.scheme 		=   0;
	option.z80speedmod  =   1;
	sprintf (option.marquee, "marquees/!none.png");
	
	option.inputs[0].maps[MAP_XU] = MACH_UP;
	option.inputs[0].maps[MAP_XD] = MACH_DOWN;
	option.inputs[0].maps[MAP_XL] = MACH_LEFT;
	option.inputs[0].maps[MAP_XR] = MACH_RIGHT;
	option.inputs[0].maps[MAP_YU] = MACH_B1;
	option.inputs[0].maps[MAP_YD] = MACH_B4;
	option.inputs[0].maps[MAP_YL] = MACH_B5;
	option.inputs[0].maps[MAP_YR] = MACH_B6;
	option.inputs[0].maps[MAP_A] = MACH_B2;
	option.inputs[0].maps[MAP_B] = MACH_B3;
	option.inputs[0].maps[MAP_START] = MACH_START;
	
	u32 i;
	for (i = 0; i < 16; ++i)
	{
		option.shades[i][0] = (15 - i) << 4;
		option.shades[i][1] = (15 - i) << 4;
		option.shades[i][2] = (15 - i) << 4;
	}	
}

/*
      printf(" -tweak       \t force 256x192 or 160x144 tweaked display.\n");
      printf(" -vsync       \t wait for vertical sync before blitting.\n");
      printf(" -sndrate <n> \t specify sound rate. (8000, 11025, 22050, 44100)\n");
      printf(" -jp          \t use Japanese console type.\n");
      printf(" -fm          \t required to enable YM2413 sound.\n");
      printf(" -codies      \t force Codemasters mapper.\n");
*/

/* Parse argument list */
void parse_args(int argc, char **argv)
{
	int i;

	for (i = 2; i < argc; i++)
	{
		int left = argc - i - 1;

		if (strcasecmp(argv[i], "--sndrate") == 0 && left)
		{
			option.sndrate = atoi(argv[i+1]);
		}
		
		if (strcasecmp(argv[i], "--sndfilter") == 0 && left)
		{
			option.sndfilter = atoi(argv[i+1]);
		}		

		else if (strcasecmp(argv[i], "--speed") == 0)
		{
			if (++i < argc)
			{
				option.speed = atoi(argv[i]);
			}
		}

		else if (strcasecmp(argv[i], "--fullscreen") == 0)
		{
			option.fullscreen = 1;
		}
		
		else if (strcasecmp(argv[i], "--autorotate") == 0)
		{
			if (++i < argc)
			{
				option.autorotate = atoi(argv[i]);
			}
		}		

		else if (strcasecmp(argv[i], "--scheme") == 0)
		{
			if (++i < argc)
			{
				option.scheme = atoi(argv[i]);
			}
		}

#ifdef GP2X
		else if (strcasecmp(argv[i], "--tweak") == 0)
		{
			tweak = 1;
		}
#endif
	}
}


