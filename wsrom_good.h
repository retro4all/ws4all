#ifndef _WSROM_GOOD_H_
#define _WSROM_GOOD_H_

#define WS_DATROMS (295)

int get_crc (char * filename);

int WS_DAT_LookFor(int CRC32);

char * WS_DAT_getname(int no);

#endif /* _WSROM_GOOD_H_ */
