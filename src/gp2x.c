#ifdef GP2X

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include "SDL.h"
#include "SDL_gp2x.h"
#include "conf.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>


#include "gp2x.h"
#include "menu.h"

#define SYS_CLK_FREQ 7372800
#define GP2X_VIDEO_MEM_SIZE ((5*1024*1024) - 4096)

// system registers
static struct
{
        unsigned short SYSCLKENREG,SYSCSETREG,
		FPLLVSETREG,DUALINT920,DUALINT940,
		DUALCTRL940,DISPCSETREG;
}
system_reg;

//volatile unsigned short *MEM_REG;
unsigned MDIV,PDIV,SCALE;
volatile unsigned *arm940code;
//static int fclk;
static int cpufreq;
static Uint32 tvoutfix_sav;
static char name[256];
/*
  void cpuctrl_init()
  {
  MEM_REG=&gp2x_memregs[0];
  }*/

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

void set_RAM_Timings(int tRC, int tRAS, int tWR, int tMRD, int tRFC, int tRP, int tRCD)
{
        tRC -= 1; tRAS -= 1; tWR -= 1; tMRD -= 1; tRFC -= 1; tRP -= 1; tRCD -= 1; // ???
        gp2x_memregs[0x3802>>1] = ((tMRD & 0xF) << 12) | ((tRFC & 0xF) << 8) | ((tRP & 0xF) << 4) | (tRCD & 0xF);
        gp2x_memregs[0x3804>>1] = /*0x9000 |*/ ((tRC & 0xF) << 8) | ((tRAS & 0xF) << 4) | (tWR & 0xF);
}

void debug_gp2x_tvout(void)
{
	printf("Some TV out Regs\n");
	printf("TV-Out?       %04x\n",gp2x_memregs[0x2800>>1]&0x100);
	printf("horizontal    %04x\n",gp2x_memregs[0x2906>>1]);
	printf("vertical      %08x\n",gp2x_memregl[0x2908>>2]);
	printf("RGB Width     %04x\n",gp2x_memregs[0x290C>>1]);
	printf("RGB Windows   %04x\n",gp2x_memregs[0x28E2>>1]);
	printf("MLC_DPC_X_MAX %04x\n",gp2x_memregs[0x2816>>1]);
	printf("MLC_DPC_Y_MAX %04x\n",gp2x_memregs[0x2818>>1]);
	// C000 28E4h / C000 28ECh / C000 28F4h / C000 28FCh

	printf("STL1 ENDX     %04x\n",gp2x_memregs[0x28E4>>1]);
	printf("STL2 ENDX     %04x\n",gp2x_memregs[0x28EC>>1]);
	printf("STL3 ENDX     %04x\n",gp2x_memregs[0x28F4>>1]);
	printf("STL4 ENDX     %04x\n",gp2x_memregs[0x28FC>>1]);
	//gp2x_memregs[0x28E4>>1]=gp2x_memregs[0x290C>>1];
	//gp2x_memregs[0x290C>>1]*=2;
}

//volatile Uint32 *gp2x_ram;
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

Uint8 *gp2x_ram_malloc(size_t size,Uint32 page) {
	static volatile Uint8 *ram_ptr=0;
	static volatile Uint8 *ram_ptr2=0;
	volatile Uint8 *t;
	if (page==0) {
		if (!ram_ptr) {
			ram_ptr=gp2x_ram/*+0x8000+0x100000*/;
			//printf("Ram_ptr=%p\n",ram_ptr);
		}
		if ((Uint32)ram_ptr-(Uint32)gp2x_ram+size<=0x1000000) {
			t=ram_ptr;
			ram_ptr+=(((Uint32)size)|0xF)+0x1;
			//printf("allocating %d\n",size);
			return t;
		} else {
			printf("Out of page1 upper memory\n");
		}
	} else {
		if (!ram_ptr2) {
			ram_ptr2=gp2x_ram2+0x608100;
			//printf("Ram_ptr2=%p %p\n",ram_ptr2,gp2x_ram2);
		}
		if ((Uint32)ram_ptr2-(Uint32)gp2x_ram2+size<=0x1000000) {
			t=ram_ptr2;
			ram_ptr2+=(((Uint32)size)|0xF)+0x1;
			//printf("allocating %d\n",size);
			return t;
		} else {
			printf("Out of page2 upper memory\n");
		}
	}
	return NULL;
}

void gp2x_sound_volume_set(int l, int r) {
	static int L;
	L=(((l*0x50)/100)<<8)|((r*0x50)/100);          //0x5A, 0x60
	ioctl(gp2x_mixer, SOUND_MIXER_WRITE_PCM, &L); //SOUND_MIXER_WRITE_VOLUME
}
/* mono only */
int gp2x_sound_volume_get(void) {
	static int L;
	//L=(((l*0x50)/100)<<8)|((r*0x50)/100);          //0x5A, 0x60
	ioctl(gp2x_mixer, SOUND_MIXER_READ_PCM, &L); //SOUND_MIXER_READ_VOLUME
	printf("Vol=%x\n",L);
	return ((L&0xFF)*100.0/(float)0x50);
}

void gp2x_set_cpu_speed(void) {
	int overclock=CF_VAL(cf_get_item_by_name("cpu_speed"));
	char clockgen = 0;
        unsigned sysfreq=0;

	/* save FCLOCK */
	sysfreq=get_freq_920_CLK();
        sysfreq*=get_920_Div()+1;
        cpufreq=sysfreq/1000000;
	if (overclock!=0) {
		if (overclock<66) overclock=66;
		if (overclock>320) overclock=320;
		set_FCLK(overclock);
	}
}

void gp2x_quit(void) {
	//hackpgtable(1);
	
	if (gp2x_dev_mem!=-1) close(gp2x_dev_mem);
	if (gp2x_gfx_dump!=-1) close(gp2x_gfx_dump);
	if (gp2x_mixer!=-1) close(gp2x_mixer);

	if (CF_VAL(cf_get_item_by_name("cpu_speed"))!=0) {
		set_FCLK(cpufreq);
	}
	if (CF_BOOL(cf_get_item_by_name("tvout"))) 
		SDL_GP2X_TV(0);

	sync();
	SDL_Quit();

	if (strcmp("null",CF_STR(cf_get_item_by_name("frontend")))!=0) {
		/* TODO: chdir to the gpe dir before executing it */
		execl(CF_STR(cf_get_item_by_name("frontend")),
		      CF_STR(cf_get_item_by_name("frontend")),NULL);
	} else {
		/*
		  chdir("/usr/gp2x");
		  execl("gp2xmenu","gp2xmenu",NULL);
		*/
	}


}

void benchmark (void *memptr)
{
    int starttime = time (NULL);
    int a,b,c,d;
    volatile unsigned int *pp = (unsigned int *) memptr;

    while (starttime == time (NULL));

    printf ("\n\nmemory benchmark of volatile VA: %08X\n\nread test\n", memptr);
    for (d = 0; d < 3; d ++)
    {
        starttime = time (NULL);
        b = 0;
        c = 0;
        while (starttime == time (NULL))
        {
            for (a = 0; a < 20000; a ++)
            {
                b += pp[a];
            }
            c ++;
        }
        printf ("Count is %d. %dMB/sec\n",  c, (c * 20000)/1024/1024);
    }

    printf ("write test\n");
    for (d = 0; d < 3; d ++)
    {
        starttime = time (NULL);
        b = 0;
        c = 0;
        while (starttime == time (NULL))
        {
            for (a = 0; a < 20000; a ++)
            {
                pp[a] = 0x37014206;
            }
            c ++;
        }
        printf ("Count is %d. %dMB/sec\n",  c, (c * 20000)/1024/1024);
    }

    printf  ("combined test (read, write back)\n");
    for (d = 0; d < 3; d ++)
    {
        starttime = time (NULL);
        b = 0;
        c = 0;
        while (starttime == time (NULL))
        {
            for (a = 0; a < 20000; a ++)
            {
                pp[a] += 0x55017601;
            }
            c ++;
        }
        printf ("Count is %d. %dMB/sec\n",  c, (c * 20000)/1024/1024);
    }

    printf ("test complete\n");
}

int hack_the_mmu(void) {
	int mmufd = open("/dev/mmuhack", O_RDWR);
/*
  volatile unsigned int *secbuf = (unsigned int *)malloc (204800);
  benchmark ((void*)gp2x_ram);
  benchmark ((void*)secbuf);
*/

        if(mmufd < 0) {
                system("/sbin/insmod -f mmuhack.o");
                mmufd = open("/dev/mmuhack", O_RDWR);
        }
        if(mmufd < 0) return 1;
	
        close(mmufd);

/*
	benchmark ((void*)gp2x_ram);
	benchmark ((void*)secbuf);
*/
        return 0;
}

void gp2x_init_mixer(void) {
	printf("Open mixer\n");
	if (CF_BOOL(cf_get_item_by_name("sound"))) {
		gp2x_mixer=open("/dev/mixer", O_RDWR);
		if (gp2x_mixer==-1) {
			printf("Can't open mixer\n");
		}
	}	
}

Uint32 gp2x_is_tvout_on(void) {
	SDL_Rect r;
	SDL_GP2X_GetPhysicalScreenSize(&r);
	printf("screen= %d %d\n",r.w,r.h);
	if (r.w!=320) return 1;
	return 0;
}


void gp2x_init(void) {
	volatile unsigned int *secbuf = (unsigned int *)malloc (204800);

	gp2x_ram_init();

	/* Fix tvout */
	//tvoutfix_sav=gp2x_memregs[0x28E4>>1];
	if (gp2x_is_tvout_on())
		gp2x_memregs[0x28E4>>1]=800;//gp2x_memregs[0x290C>>1];

	if (CF_BOOL(cf_get_item_by_name("tvout"))) {
		SDL_GP2X_TV(1);
		SDL_GP2X_TVMode(3/*DISPLAY_TV_NTSC*/);
	}

	/* CraigX RAM timing */
	if (CF_BOOL(cf_get_item_by_name("ramhack"))) 
		set_RAM_Timings(6, 4, 1, 1, 1, 2, 2);
	//setpriority(PRIO_PROCESS,0,-20);


}

/* Load a 940 bin @ 0x3000000 */
void gp2x_940_loadbin(char *filename) {
	
}

#endif
