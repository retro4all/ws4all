/*  cpuctrl for GP2X
    Copyright (C) 2005  Hermes/PS2Reality

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
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/


#include <sys/mman.h>
#include "cpuctrl.h"


/* system registers */
static struct
{
	unsigned short SYSCLKENREG, SYSCSETREG, FPLLVSETREG, DUALINT920, DUALINT940, DUALCTRL940;
}
system_reg;

static unsigned short dispclockdiv;

static volatile unsigned short *MEM_REG;

#define SYS_CLK_FREQ 7372800


void cpuctrl_init(void)
{
	extern volatile unsigned short *gp2x_memregs; /* from minimal library rlyeh */
	MEM_REG = &gp2x_memregs[0];
	system_reg.SYSCSETREG = MEM_REG[0x91c>>1];
	system_reg.FPLLVSETREG = MEM_REG[0x912>>1];
	system_reg.SYSCLKENREG = MEM_REG[0x904>>1];
	system_reg.DUALINT920 = MEM_REG[0x3B40>>1];
	system_reg.DUALINT940 = MEM_REG[0x3B42>>1];
	system_reg.DUALCTRL940 = MEM_REG[0x3B48>>1];
	dispclockdiv = MEM_REG[0x924>>1];
}


void cpuctrl_deinit(void)
{
	MEM_REG[0x91c>>1] = system_reg.SYSCSETREG;
	MEM_REG[0x910>>1] = system_reg.FPLLVSETREG;
	MEM_REG[0x3B40>>1] = system_reg.DUALINT920;
	MEM_REG[0x3B42>>1] = system_reg.DUALINT940;
	MEM_REG[0x3B48>>1] = system_reg.DUALCTRL940;
	MEM_REG[0x904>>1] = system_reg.SYSCLKENREG;
	MEM_REG[0x924>>1] = dispclockdiv;
}


void set_display_clock_div(unsigned div)
{
	div = ((div & 63) | 64) << 8;
	MEM_REG[0x924>>1] = (MEM_REG[0x924>>1] & ~(255 << 8)) | div;
}


void set_FCLK(unsigned MHZ)
{
	unsigned v;
	unsigned mdiv, pdiv = 3, scale = 0;
	MHZ *= 1000000;
	mdiv = (MHZ * pdiv) / SYS_CLK_FREQ;
	mdiv = ((mdiv - 8) << 8) & 0xff00;
	pdiv = ((pdiv - 2) << 2) & 0xfc;
	scale &= 3;
	v = mdiv | pdiv | scale;
	MEM_REG[0x910>>1] = v;
}


void set_920_Div(unsigned short div)
{
	unsigned short v;
	v = MEM_REG[0x91c>>1] & (~0x3);
	MEM_REG[0x91c>>1] = (div & 0x7) | v;
}


void set_DCLK_Div( unsigned short div )
{
	unsigned short v;
	v = (unsigned short)( MEM_REG[0x91c>>1] & (~(0x7 << 6)) );
	MEM_REG[0x91c>>1] = ((div & 0x7) << 6) | v;
}


void Disable_940(void)
{
	MEM_REG[0x3B42>>1];
	MEM_REG[0x3B42>>1] = 0;
	MEM_REG[0x3B46>>1] = 0xffff;
	MEM_REG[0x3B48>>1] |= (1 << 7);
	MEM_REG[0x904>>1] &= 0xfffe;
}

void gp2x_video_wait_vsync(void)
{
	MEM_REG[0x2846>>1] = (MEM_REG[0x2846>>1] | 0x20) & ~2;

	while (!(MEM_REG[0x2846>>1] & 2));
}
