
#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "hal.h"
#include "types.h"

#define MAX_CHEATS 20

enum { MAP_START = 1, MAP_A, MAP_B, MAP_XU, MAP_XR, MAP_XD, MAP_XL, MAP_YU, MAP_YR, MAP_YD, MAP_YL };

typedef struct
{
	char description[256];
	int address;
	int value;
	int enabled;
} t_cheat;

typedef struct
{
  int sndrate;
  int sndfilter;
  int territory;
  int display;
  int fm_enable;
  int codies;
  int speed;
  int fullscreen;
  u32 autorotate;
  char marquee[256];
  int players;
  float z80speedmod;
  int scheme;
  u8 shades[16][3];
  t_input inputs[2];
}t_option;

typedef struct
{
	/*
  uint8 *rom;
  uint8 pages;
  uint32 crc;
  uint32 sram_crc;
  int mapper;
  uint8 sram[0x8000];
  uint8 fcr[4];
  */
	char game_name[0x100];
	char file_name[0x100];
} t_cart;

/* Global data */
t_option option;
t_cart cart;

/* Function prototypes*/
void do_config(int arc, char **argv);
void parse_args(int argc, char **argv);
int parse_file(const char *filename, int *argc, char **argv);
void set_option_defaults(void);

#endif /* _CONFIG_H_ */
