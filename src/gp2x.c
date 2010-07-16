

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include "SDL.h"
#include "SDL_gp2x.h"
#include "conf.h"

#include <alloca.h>
#include <stdlib.h>
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
#include <libgen.h>
#include <stdio.h>

#include "screen.h"
#include "gp2x.h"
#include "menu.h"
#include "video.h"
#include "emu.h"
#include "ym2610-940/940shared.h"
#include "ym2610-940/940private.h"

#define SYS_CLK_FREQ 7372800
#define GP2X_VIDEO_MEM_SIZE ((5*1024*1024) - 4096)


static struct
{
        unsigned short SYSCLKENREG,SYSCSETREG,UPLLVSETREG,FPLLVSETREG,
                DUALINT920,DUALINT940,DUALCTRL940,MEMTIMEX0,MEMTIMEX1,DISPCSETREG,
                DPC_HS_WIDTH,DPC_HS_STR,DPC_HS_END,DPC_VS_END,DPC_DE;
}
system_reg;

//volatile unsigned short *MEM_REG;
unsigned MDIV,PDIV,SCALE;
volatile unsigned *arm940code;
//static int fclk;
static int cpufreq;
static Uint32 tvoutfix_sav;
static char name[256];
volatile _940_data_t *shared_data;
volatile _940_ctl_t *shared_ctl;
volatile _940_private_t *private_data;

typedef struct video_bucket {
  struct video_bucket *prev, *next;
  char *base;
  unsigned int size;
  short used;
  short dirty;
} video_bucket;



/*
  void cpuctrl_init()
  {
  MEM_REG=&gp2x_memregs[0];
  }*/

void cpuctrl_init(void)
{
        system_reg.DISPCSETREG=gp2x_memregs[0x924>>1];
        system_reg.UPLLVSETREG=gp2x_memregs[0x916>>1];
        system_reg.FPLLVSETREG=gp2x_memregs[0x912>>1];
        system_reg.SYSCSETREG=gp2x_memregs[0x91c>>1];
        system_reg.SYSCLKENREG=gp2x_memregs[0x904>>1];
        system_reg.DUALINT920=gp2x_memregs[0x3B40>>1];
        system_reg.DUALINT940=gp2x_memregs[0x3B42>>1];
        system_reg.DUALCTRL940=gp2x_memregs[0x3B48>>1];
        system_reg.MEMTIMEX0=gp2x_memregs[0x3802>>1];
        system_reg.MEMTIMEX1=gp2x_memregs[0x3804>>1];
        system_reg.DPC_HS_WIDTH=gp2x_memregs[0x281A>>1];
        system_reg.DPC_HS_STR=gp2x_memregs[0x281C>>1];
        system_reg.DPC_HS_END=gp2x_memregs[0x281E>>1];
        system_reg.DPC_VS_END=gp2x_memregs[0x2822>>1];
        system_reg.DPC_DE=gp2x_memregs[0x2826>>1];
}


void cpuctrl_deinit(void)
{
        gp2x_memregs[0x910>>1]=system_reg.FPLLVSETREG;
        gp2x_memregs[0x91c>>1]=system_reg.SYSCSETREG;
        gp2x_memregs[0x3B40>>1]=system_reg.DUALINT920;
        gp2x_memregs[0x3B42>>1]=system_reg.DUALINT940;
        gp2x_memregs[0x3B48>>1]=system_reg.DUALCTRL940;
        gp2x_memregs[0x904>>1]=system_reg.SYSCLKENREG;
        gp2x_memregs[0x3802>>1]=system_reg.MEMTIMEX0;
        gp2x_memregs[0x3804>>1]=system_reg.MEMTIMEX1 /*| 0x9000*/;
        unset_LCD_custom_rate();
}

#define SYS_CLK_FREQ 7372800

void SetClock(unsigned int MHZ)
{
#ifdef GP2X
  unsigned int v;
  unsigned int mdiv,pdiv=3,scale=0;
  MHZ*=1000000;
  mdiv=(MHZ*pdiv)/SYS_CLK_FREQ;

  mdiv=((mdiv-8)<<8) & 0xff00;
  pdiv=((pdiv-2)<<2) & 0xfc;
  scale&=3;
  v = mdiv | pdiv | scale;
  
  unsigned int l = gp2x_memregl[0x808>>2];// Get interupt flags
  gp2x_memregl[0x808>>2] = 0xFF8FFFE7;   //Turn off interrupts
  gp2x_memregs[0x910>>1]=v;              //Set frequentie
  while(gp2x_memregs[0x0902>>1] & 1);    //Wait for the frequentie to be ajused
  gp2x_memregl[0x808>>2] = l;            //Turn on interrupts
#endif
}

typedef struct
{
        unsigned short reg, valmask, val;
}
reg_setting;
	static Uint8 *ram_ptr=0;
	static Uint8 *ram_ptr2=0;

// ~59.998, couldn't figure closer values
static reg_setting rate_almost60[] =
{
        { 0x0914, 0xffff, (212<<8)|(2<<2)|1 },  /* UPLLSETVREG */
        { 0x0924, 0xff00, (2<<14)|(36<<8) },    /* DISPCSETREG */
        { 0x281A, 0x00ff, 1 },                  /* .HSWID(T2) */
        { 0x281C, 0x00ff, 0 },                  /* .HSSTR(T8) */
        { 0x281E, 0x00ff, 2 },                  /* .HSEND(T7) */
        { 0x2822, 0x01ff, 12 },                 /* .VSEND (T9) */
        { 0x2826, 0x0ff0, 34<<4 },              /* .DESTR(T3) */
        { 0, 0, 0 }
};

// perfect 50Hz?
static reg_setting rate_50[] =
{
        { 0x0914, 0xffff, (39<<8)|(0<<2)|2 },   /* UPLLSETVREG */
        { 0x0924, 0xff00, (2<<14)|(7<<8) },     /* DISPCSETREG */
        { 0x281A, 0x00ff, 31 },                 /* .HSWID(T2) */
        { 0x281C, 0x00ff, 16 },                 /* .HSSTR(T8) */
        { 0x281E, 0x00ff, 15 },                 /* .HSEND(T7) */
        { 0x2822, 0x01ff, 15 },                 /* .VSEND (T9) */
        { 0x2826, 0x0ff0, 37<<4 },              /* .DESTR(T3) */
        { 0, 0, 0 }
};
// 16639/2 ~120.20
static reg_setting rate_120_20[] =
{
        { 0x0914, 0xffff, (96<<8)|(0<<2)|2 },   /* UPLLSETVREG */
        { 0x0924, 0xff00, (2<<14)|(7<<8) },     /* DISPCSETREG */
        { 0x281A, 0x00ff, 19 },                 /* .HSWID(T2) */
        { 0x281C, 0x00ff, 7 },                  /* .HSSTR(T8) */
        { 0x281E, 0x00ff, 7 },                  /* .HSEND(T7) */
        { 0x2822, 0x01ff, 12 },                 /* .VSEND (T9) */
        { 0x2826, 0x0ff0, 37<<4 },              /* .DESTR(T3) */
        { 0, 0, 0 }
};

// 19997/2 ~100.02
static reg_setting rate_100_02[] =
{
        { 0x0914, 0xffff, (98<<8)|(0<<2)|2 },   /* UPLLSETVREG */
        { 0x0924, 0xff00, (2<<14)|(8<<8) },     /* DISPCSETREG */
        { 0x281A, 0x00ff, 26 },                 /* .HSWID(T2) */
        { 0x281C, 0x00ff, 6 },                  /* .HSSTR(T8) */
        { 0x281E, 0x00ff, 6 },                  /* .HSEND(T7) */
        { 0x2822, 0x01ff, 31 },                 /* .VSEND (T9) */
        { 0x2826, 0x0ff0, 37<<4 },              /* .DESTR(T3) */
        { 0, 0, 0 }
};



static reg_setting *possible_rates[] = { rate_almost60, rate_50, rate_120_20, rate_100_02 };
void set_LCD_custom_rate(lcd_rate_t rate)
{
        reg_setting *set;

        printf("setting custom LCD refresh, mode=%i... ", rate); fflush(stdout);
        for (set = possible_rates[rate]; set->reg; set++)
        {
                unsigned short val = gp2x_memregs[set->reg >> 1];
                val &= ~set->valmask;
                val |= set->val;
                gp2x_memregs[set->reg >> 1] = val;
        }
        printf("done.\n");
}

void unset_LCD_custom_rate(void)
{
        printf("reset to prev LCD refresh.\n");
        gp2x_memregs[0x914>>1]=system_reg.UPLLVSETREG;
        gp2x_memregs[0x924>>1]=system_reg.DISPCSETREG;
        gp2x_memregs[0x281A>>1]=system_reg.DPC_HS_WIDTH;
        gp2x_memregs[0x281C>>1]=system_reg.DPC_HS_STR;
        gp2x_memregs[0x281E>>1]=system_reg.DPC_HS_END;
        gp2x_memregs[0x2822>>1]=system_reg.DPC_VS_END;
        gp2x_memregs[0x2826>>1]=system_reg.DPC_DE;
}

void set_RAM_Timings(int tRC, int tRAS, int tWR, int tMRD, int tRFC, int tRP, int tRCD)
{
        tRC -= 1; tRAS -= 1; tWR -= 1; tMRD -= 1; tRFC -= 1; tRP -= 1; tRCD -= 1; // ???
        gp2x_memregs[0x3802>>1] = ((tMRD & 0xF) << 12) | ((tRFC & 0xF) << 8) | ((tRP & 0xF) << 4) | (tRCD & 0xF);
        gp2x_memregs[0x3804>>1] = /*0x9000 |*/ ((tRC & 0xF) << 8) | ((tRAS & 0xF) << 4) | (tWR & 0xF);
}

/* TODO */
/*int spend_cycles(int cycles) {
	int i,j=0;
	for(i=cycles/4;i>0;i--) {
		j=i+1;
	}
	return j;
	}*/


void print_shared_struct(void) {
	printf("Shared data:\n");
	printf("Cylce: %d sample_rate: %d\n",shared_data->z80_cycle,shared_data->sample_rate);
	printf("pcma: %08x pcmb: %08x\n",(Uint32)shared_data->pcmbufa,(Uint32)shared_data->pcmbufb);
	printf("asize: %d bsize: %d\n",shared_data->pcmbufa_size,shared_data->pcmbufb_size);
	printf("sm1: %08x",(Uint32)shared_data->sm1);
	printf("Shared ctl:\n");
	printf("result: %d sound_code: %d pending_cmd: %d nmi_pending: %d\n",
	       shared_ctl->result_code,shared_ctl->sound_code,shared_ctl->pending_command,
	       shared_ctl->nmi_pending);
	printf("z80_run: %d updateym: %d, play_buff:%08x\n",
	       shared_ctl->z80_run,shared_ctl->updateym,(Uint32)shared_ctl->play_buffer);

}

void wait_busy_940(int job)
{
        int i;

        job--;
        for (i = 0; (gp2x_memregs[0x3b46>>1] & (1<<job)) && i < 0x10000; i++)
		spend_cycles(8*1024); // tested to be best for mp3 dec
        if (i < 0x10000) return;
        /* 940 crashed */
        printf("940 crashed %d (cnt: %i, ve: ", job, shared_ctl->loopc);
        for (i = 0; i < 8; i++)
                printf("%i ", shared_ctl->vstarts[i]);
        printf(")\n");
	print_shared_struct();

        printf("irq pending flags: DUALCPU %04x, SRCPND %08x (see 26), INTPND %08x\n",
                gp2x_memregs[0x3b46>>1], gp2x_memregl[0x4500>>2], gp2x_memregl[0x4510>>2]);
        printf("last lr: %08x, lastjob: %i\n", shared_ctl->last_lr, shared_ctl->lastjob);
        printf("trying to interrupt..\n");
        gp2x_memregs[0x3B3E>>1] = 0xffff;
        for (i = 0; gp2x_memregs[0x3b46>>1] && i < 0x10000; i++)
                spend_cycles(8*1024);
        printf("i = 0x%x\n", i);
        printf("irq pending flags: DUALCPU %04x, SRCPND %08x (see 26), INTPND %08x\n",
                gp2x_memregs[0x3b46>>1], gp2x_memregl[0x4500>>2], gp2x_memregl[0x4510>>2]);
        printf("last lr: %08x, lastjob: %i\n", shared_ctl->last_lr, shared_ctl->lastjob);
}

/* 940 utilities fuction (largely based on Notaz's picodirve) */
void gp2x_add_job940(int job)
{
        if (job <= 0 || job > 16) {
                printf("add_job_940: bad job: %i\n", job);
                return;
        }
        // generate interrupt for this job
        job--;
        gp2x_memregs[(0x3B20+job*2)>>1] = 1;
	//printf("added %i, pending %04x\n", job+1, gp2x_memregs[0x3b46>>1]);
}

/* 940 */
void gp2x_pause940(int yes)
{
	printf ("Pause 940 %d\n",yes);
        if(yes)
                gp2x_memregs[0x0904>>1] &= 0xFFFE;
        else
                gp2x_memregs[0x0904>>1] |= 1;
}
void gp2x_reset940(int yes, int bank)
{
	printf ("Reset 940 %d\n",yes);
        gp2x_memregs[0x3B48>>1] = ((yes&1) << 7) | (bank & 0x03);
}
void gp2x_load_bin940(char *file) {
	FILE *f;
	unsigned char ucData[1024];
	int nRead, i, nLen = 0;
	char *buf;
	int size;
	f=fopen(file,"rb");
	if (!f) {
		printf("Error while loading %s\n",file);
		exit(1);
	}
/*
	fseek(f,0,SEEK_END);
	size=ftell(f);
	fseek(f,0,SEEK_SET);
	buf=alloca(size);

	fread(buf,size,1,f);
	fclose(f);
	memcpy(gp2x_ram2_uncached,buf,size);
*/
	while(1)
	{
		nRead = fread(ucData, 1, 1024, f);
		if(nRead <= 0)
			break;
		memcpy((void*)gp2x_ram2_uncached + nLen, ucData, nRead);
		nLen += nRead;
	}
	fclose(f);
	printf("CODE940 uploded [%02X %02X %02X %02X]\n",
	       gp2x_ram2_uncached[0],
	       gp2x_ram2_uncached[1],
	       gp2x_ram2_uncached[2],
	       gp2x_ram2_uncached[3]);
}
void gp2x_init_940(void) {
	struct video_bucket *bucket;
	gp2x_reset940(1, 2);
        gp2x_pause940(1);
	
        gp2x_memregs[0x3B40>>1] = 0;      // disable DUALCPU interrupts for 920
        gp2x_memregs[0x3B42>>1] = 1;      // enable  DUALCPU interrupts for 940
	
        gp2x_memregl[0x4504>>2] = 0;        // make sure no FIQs will be generated
	gp2x_memregl[0x4508>>2] = ~(1<<26); // unmask DUALCPU ints in the undocumented 940's interrupt controller



	gp2x_load_bin940("gngeo940.bin");

	printf("Shared data reset...\n");
	memset((void*)shared_data, 0, sizeof(*shared_data));
        memset((void*)shared_ctl,  0, sizeof(*shared_ctl));
	printf("Shared data reseted\n");


	gp2x_memregs[0x3B46>>1] = 0xffff; // clear pending DUALCPU interrupts for 940
        gp2x_memregl[0x4500>>2] = 0xffffffff; // clear pending IRQs in SRCPND
        gp2x_memregl[0x4510>>2] = 0xffffffff; // clear pending IRQs in INTPND
	printf("Clear interrupt\n");
	
        /* start the 940 */
        gp2x_reset940(0, 2);
        gp2x_pause940(0);
	//sleep(2);
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
	gp2x_ram=(Uint8 *)mmap(0, 0x2000000, PROT_READ|PROT_WRITE, MAP_SHARED, 
			       gp2x_dev_mem, 0x02000000);
	gp2x_ram2=(Uint8 *)mmap(0, 0x1000000, PROT_READ|PROT_WRITE, MAP_SHARED, 
				gp2x_dev_mem, 0x03000000);

	//printf("gp2x_ram %p %p %p\n",gp2x_ram,gp2x_ram2,gp2x_ram_all);
	gp2x_memregl=(Uint32 *)mmap(0, 0x10000, PROT_READ|PROT_WRITE, MAP_SHARED, 
				    gp2x_dev_mem, 0xc0000000);
	gp2x_memregs=(Uint16*)gp2x_memregl ;
}

void gp2x_ram_init_uncached(void) {
	/* Open non chached memory */
	gp2x_ram2_uncached=(Uint8 *)mmap(0, 0x2000000, PROT_READ|PROT_WRITE, MAP_SHARED, 
					 gp2x_dev_mem, 0x02000000);
	shared_data = (_940_data_t *) (gp2x_ram2_uncached+0x1C00000);
        /* this area must not get buffered on either side */
        shared_ctl =  (_940_ctl_t *)  (gp2x_ram2_uncached+0x1C00100);
        private_data =  (_940_private_t *)  (gp2x_ram2_uncached+0x1B00000);

	memset((void*)gp2x_ram2_uncached,0,0x100000);
	memset((void*)gp2x_ram2_uncached+0x1800000,0,0x800000);
}

Uint8 *gp2x_ram_malloc(size_t size,Uint32 page) {
	Uint8 *t;
	if (page==0) {
		if (!ram_ptr) {
			//ram_ptr=gp2x_ram/*+0x8000+0x100000*/;
			ram_ptr=(Uint8*)gp2x_ram+0x100000;
			//printf("Ram_ptr=%p\n",ram_ptr);
		}
		if ((Uint32)ram_ptr-(Uint32)gp2x_ram+size<=0x1100000) {
			t=ram_ptr;
			ram_ptr+=(((Uint32)size)|0xF)+0x1;
			//printf("allocating %d\n",size);
			return t;
		} else {
			printf("Out of page1 upper memory\n");
		}
	} else {
		if (!ram_ptr2) {
			//ram_ptr2=gp2x_ram2+0x608100;
			ram_ptr2=(Uint8*)gp2x_ram2+0x800000;
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

void gp2x_ram_ptr_reset(void) {
	ram_ptr=NULL;
	ram_ptr2=NULL;
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
	//sysfreq=get_freq_920_CLK();
        //sysfreq*=get_920_Div()+1;
        //cpufreq=sysfreq/1000000;

	if (overclock!=0) {
		if (overclock<66) overclock=66;
		if (overclock>320) overclock=320;
		SetClock(overclock);
	}
}

void gp2x_quit(void) {
	//hackpgtable(1);
	int i;
	char *frontend=strdup(CF_STR(cf_get_item_by_name("frontend")));
	char *fullpath=CF_STR(cf_get_item_by_name("frontend"));
/*
	for(i=0;i<gcache.total_bank;i++) {
		printf("BANK %06d %d\n",i,mem_bank_usage[i]);
	}
*/
#ifdef ENABLE_940T	
	sleep(1);
	print_shared_struct();
	gp2x_reset940(1, 2);
        gp2x_pause940(1);
#endif
	printf("cpuctrl_deinit \n");
	cpuctrl_deinit();

	printf("closing opend device \n");

	if (gp2x_dev_mem!=-1) close(gp2x_dev_mem);
	if (gp2x_gfx_dump!=-1) close(gp2x_gfx_dump);
	if (gp2x_mixer!=-1) close(gp2x_mixer);

	printf("stop tvout mode \n");	
	if (CF_BOOL(cf_get_item_by_name("tvout"))) 
		SDL_GP2X_TV(0);

	printf("sync\n");
	sync();
	printf("SDLQuit\n");
	SDL_Quit();

	printf("Restart menu\n");
	if (strcmp("null",frontend)!=0) {
		/* TODO: Special case for Rage2x */
		char *base=basename(frontend);
		char *dir=dirname(frontend);
		
		if (dir) {
			chdir(dir);
			printf("changinf dir to %s - %s - %s\n",dir,base,frontend);
		}
		if (strcmp("rage2x.gpe",base)==0) {
			char opt[128];
			snprintf(opt,120,"rom=%s",conf.game);
			printf("opt= %s\n",opt);
			execl(fullpath,fullpath,opt,NULL);
		} else
			execl(fullpath,fullpath,NULL);
	}
	
}

void benchmark (void *memptr)
{
    int starttime = time (NULL);
    int a,b,c,d;
    volatile unsigned int *pp = (unsigned int *) memptr;

    while (starttime == time (NULL));

    printf ("\n\nmemory benchmark of volatile VA: %08X\n\nread test\n",(Uint32)memptr);
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

        if(mmufd < 0) {
                system("/sbin/insmod -f mmuhack.o");
                mmufd = open("/dev/mmuhack", O_RDWR);
        }
        if(mmufd < 0) return 1;
	
        close(mmufd);

	gp2x_ram_init_uncached();

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
	if (r.w!=320) return r.h;
	return 0;
}


void gp2x_init(void) {
	volatile unsigned int *secbuf = (unsigned int *)malloc (204800);

	gp2x_ram_init();
	cpuctrl_init();

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


    sdl_set_title(NULL);
    //SDL_textout(screen, 1, 231, "Patching MMU ... ");SDL_Flip(screen);

    //   if (hackpgtable()==0) {
    if (hack_the_mmu()==0) {
	    SDL_textout(screen, 1, 231, "Patching MMU ... OK!");SDL_Flip(screen);
    } else {
	    SDL_textout(screen, 1, 231, "Patching MMU ... FAILED :(");SDL_Flip(screen);
	    SDL_Delay(300);
    }
#ifdef ENABLE_940T
    gp2x_init_940();
    gp2x_set_cpu_speed();    
#endif
    //SDL_Delay(200);
    //benchmark ((void*)screen->pixels);

    gp2x_init_mixer();
    SDL_FillRect(screen,NULL,0);
}

/* Load a 940 bin @ 0x3000000 */
void gp2x_940_loadbin(char *filename) {
	
}

