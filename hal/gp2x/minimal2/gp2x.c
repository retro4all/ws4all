#include <stdlib.h>

#include <signal.h>
#include <setjmp.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <unistd.h>
#include <stropts.h>
#include <string.h>

#include "gp2x.h"

int hack_the_mmu(void)
{
	int mmufd = open("/dev/mmuhack", O_RDWR);
	volatile unsigned int *secbuf = (unsigned int *)malloc (204800);
	/*
		benchmark ((void*)gp2x_ram);
		benchmark ((void*)secbuf);
	*/

	if (mmufd < 0)
	{
		system("/sbin/insmod -f mmuhack.o");
		mmufd = open("/dev/mmuhack", O_RDWR);
	}

	if (mmufd < 0) return 1;

	close(mmufd);

	/*
		benchmark ((void*)gp2x_ram);
		benchmark ((void*)secbuf);
	*/
	return 0;
}


/*
#define SYS_CLK_FREQ 7372800
// system registers
static struct
{
  unsigned short SYSCLKENREG,SYSCSETREG,FPLLVSETREG,DUALINT920,DUALINT940,DUALCTRL940,DISPCSETREG;
}
system_reg;

//volatile unsigned short *MEM_REG;
unsigned MDIV,PDIV,SCALE;
volatile unsigned *arm940code;
//static int fclk;
static int cpufreq;
static Uint32 tvoutfix_sav;
static char name[256];

void set_FCLK(unsigned MHZ)
{
  unsigned v;
  unsigned mdiv,pdiv=3,scale=0;
  MHZ*=1000000;
  mdiv=(MHZ*pdiv)/SYS_CLK_FREQ;
  //printf ("Old value = %04X\r",MEM_REG[0x924>>1]," ");
  //printf ("APLL = %04X\r",MEM_REG[0x91A>>1]," ");
  mdiv=((mdiv-8)<<8) & 0xff00;
  pdiv=((pdiv-2)<<2) & 0xfc;
  scale&=3;
  v=mdiv | pdiv | scale;
  gp2x_memregs[0x910>>1]=v;
}

unsigned get_FCLK()
{
  return gp2x_memregs[0x910>>1];
}

unsigned get_freq_920_CLK()
{
  unsigned i;
  unsigned reg,mdiv,pdiv,scale;
  reg=gp2x_memregs[0x912>>1];
  mdiv = ((reg & 0xff00) >> 8) + 8;
  pdiv = ((reg & 0xfc) >> 2) + 2;
  scale = reg & 3;
  MDIV=mdiv;
  PDIV=pdiv;
  SCALE=scale;
  i = (gp2x_memregs[0x91c>>1] & 7)+1;
  return ((SYS_CLK_FREQ * mdiv)/(pdiv << scale))/i;
}
unsigned short get_920_Div()
{
  return (gp2x_memregs[0x91c>>1] & 0x7);
}

void gp2x_set_cpu_speed(void) {
  int overclock=CF_VAL(cf_get_item_by_name("cpu_speed"));
  char clockgen = 0;
  unsigned sysfreq=0;

  // save FCLOCK
  sysfreq=get_freq_920_CLK();
  sysfreq*=get_920_Div()+1;
  cpufreq=sysfreq/1000000;
  if (overclock!=0) {
    if (overclock<66) overclock=66;
    if (overclock>320) overclock=320;
    set_FCLK(overclock);
  }
}

void gp2x_ram_init(void) {
  if(!gp2x_dev_mem) gp2x_dev_mem = open("/dev/mem",   O_RDWR);
  gp2x_ram=(Uint8 *)mmap(0, 0x1000000, PROT_READ|PROT_WRITE, MAP_SHARED,
			 gp2x_dev_mem, 0x02000000);
  gp2x_ram2=(Uint8 *)mmap(0, 0x1000000, PROT_READ|PROT_WRITE, MAP_SHARED,
			  gp2x_dev_mem, 0x03000000);
  //printf("gp2x_ram %p\n",gp2x_ram);
  gp2x_memregl=(Uint32 *)mmap(0, 0x10000, PROT_READ|PROT_WRITE, MAP_SHARED,
			      gp2x_dev_mem, 0xc0000000);
  gp2x_memregs=(Uint16*)gp2x_memregl ;
}
*/
