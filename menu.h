/*
typedef struct {
  int fm;
  int japan;
  int usesram;
  int fskip;
  int aratio;
  int frecuency;
  char game_name[0x100];
  char marquee[256];
  int speed;
  int country;
  int machine_fps;
  int map_up;
  int map_down;
  int map_left;
  int map_right;
  int map_b1;
  int map_b2;
  int map_pause;
}game_cfg;
*/

typedef struct
{
	char game_name[80];
	char file_name[80];
	int type;
	int crc;
} troms;

int menu (void);

