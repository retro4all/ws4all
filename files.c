/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "unzip.h"


int check_zip(const char* filename)
{
	int zip = 0;
	static char SIG[] = { 0x50, 0x4B, 0x03, 0x04 };
	FILE* fp = fopen(filename, "rb");

	if (fp)
	{
		zip = fgetc(fp) == SIG[0] &&
		      fgetc(fp) == SIG[1] &&
		      fgetc(fp) == SIG[2] &&
		      fgetc(fp) == SIG[3];
		fclose(fp);
	}

	return zip;
}


static int load_rom_zip(const char* filename, unsigned char **base_rom, int * romSize)
{
	int size;
	char romfile[0x100];
	unz_file_info fi;
	unzFile zf = unzOpen(filename);

	if (!zf) return 0;

	if (unzGoToFirstFile(zf) != UNZ_OK)
		return 0;

	if (unzGetCurrentFileInfo(zf, &fi, romfile, sizeof(romfile), NULL, 0, NULL, 0) != UNZ_OK)
		return 0;

	size = fi.uncompressed_size;

	if (size < 0x70000) return 0;

	printf("size=%d %s\n", size, romfile);

	*base_rom = (unsigned char*) malloc(size);

	if (!*base_rom) return 0;

	if (unzOpenCurrentFile(zf) != UNZ_OK)
		return 0;

	unzReadCurrentFile(zf, *base_rom, size);

	unzCloseCurrentFile(zf);
	unzClose(zf);

	*romSize = size;

	return 1;
}


static int load_rom_normal(const char* filename, unsigned char **base_rom, int * romSize)
{
	int size;
	FILE *fd = fopen(filename, "rb");

	if (!fd) return 0;

	/* Seek to end of file, and get size */
	fseek(fd, 0, SEEK_END);
	size = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	/* Don't load games smaller than 512k */
	if (size < 0x70000) 
	{
		fclose(fd);
		return 0;
	}

	*base_rom = (unsigned char*) malloc(size);

	if (!*base_rom) 
	{
		fclose(fd);
		return 0;
	}

	fread(*base_rom, size, 1, fd);

	fclose(fd);

	*romSize = size;

	return 1;
}

//unsigned char *base_rom;
//u32 romSize;

/* Load a game file */

int load_rom(const char *filename, unsigned char **base_rom, int * romSize)
{

	if (check_zip(filename))
		return load_rom_zip(filename, base_rom, romSize);

	return load_rom_normal(filename, base_rom, romSize);

}
