#ifdef GP2X

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include "SDL.h"
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

void gp2x_video_RGB_setscaling(int W, int H)
{
#if 1
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
#else
 float escalaw,escalah;
 int bpp=(gp2x_memregs[0x28DA>>1]>>9)&0x3;

 if(gp2x_memregs[0x2800>>1]&0x100) //TV-Out
 {
   escalaw=489.0; //RGB Horiz TV (PAL, NTSC)
   if (gp2x_memregs[0x2818>>1]  == 287) //PAL
     escalah=274.0; //RGB Vert TV PAL
   else if (gp2x_memregs[0x2818>>1]  == 239) //NTSC
     escalah=331.0; //RGB Vert TV NTSC
 }
 else //LCD
 {
   escalaw=1024.0; //RGB Horiz LCD
   escalah=320.0; //RGB Vert LCD
 }

 // scale horizontal
 gp2x_memregs[0x2906>>1]=(unsigned short)((float)escalaw *(W/320.0));
 // scale vertical
 gp2x_memregl[0x2908>>2]=(unsigned long)((float)escalah *bpp *(H/240.0));

#endif
}

//volatile Uint32 *gp2x_ram;
void gp2x_ram_init(void) {
	if(!gp2x_dev_mem) gp2x_dev_mem = open("/dev/mem",   O_RDWR);
	gp2x_ram=(Uint8 *)mmap(0, 0x1000000, PROT_READ|PROT_WRITE, MAP_SHARED, 
				gp2x_dev_mem, 0x02000000);
	//printf("gp2x_ram %p\n",gp2x_ram);
	gp2x_memregl=(Uint32 *)mmap(0, 0x10000, PROT_READ|PROT_WRITE, MAP_SHARED, 
				    gp2x_dev_mem, 0xc0000000);
	gp2x_memregs=(Uint16*)gp2x_memregl ;
}

Uint8 *gp2x_ram_malloc(size_t size) {
	static volatile Uint8 *ram_ptr=0;
	volatile Uint8 *t;
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
		printf("Out of memory\n");
	}
	return NULL;
}

void gp2x_sound_volume(int l, int r) {
	static int L;
	L=(((l*0x50)/100)<<8)|((r*0x50)/100);          //0x5A, 0x60
	ioctl(gp2x_mixer, SOUND_MIXER_WRITE_PCM, &L); //SOUND_MIXER_WRITE_VOLUME
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

	sync();
	SDL_Quit();
	gp2x_video_RGB_setscaling(0,0);

	if (strcmp("null",CF_STR(cf_get_item_by_name("frontend")))!=0) {
		execl(CF_STR(cf_get_item_by_name("frontend")),
		      CF_STR(cf_get_item_by_name("frontend")),NULL);
	} else {
		/*
		  chdir("/usr/gp2x");
		  execl("gp2xmenu","gp2xmenu",NULL);
		*/
	}


}

/*** Highly expermimental code  ***/

#if 0
int myuname(char *buffer)
{
    asm volatile ("swi #0x90007a");
}
#endif

//static unsigned int save_cpt[4096][256];

void DecodeCoarse (unsigned int indx, unsigned int sa)
{
    unsigned int cpt[256];
    unsigned int dom = (sa >> 5) & 15;
    unsigned int temp;
    unsigned int i = 0;
    unsigned int j = indx;
    unsigned int wb = 0;
    unsigned int a;

    sa &= 0xfffffc00;
    indx *= 1048576;
    
    //printf ("  > %08X\n", sa);
    //printf ("%d\n",
    lseek (gp2x_dev_mem, sa, SEEK_SET);
    memset (cpt, 0, 256*4);
    temp = read (gp2x_dev_mem, cpt, 256*4);
    //printf ("%d\n", temp);
    if (temp != 256*4)
    {
        printf ("  # Bad read\n");
        return;
    }

    //printf ("%08X %08X %08X %08X\n", cpt[0], cpt[4], cpt[8], cpt[12]);
    
    for (i = 0; i < 256; i ++)
    {
        if (cpt[i])
        {
            switch (cpt[i] & 3)
            {
                case 0:
                    //printf ("  -- [%08X] Invalid --\n", cpt[i]);
                    break;
                case 1:
			/*
			  printf ("  VA: %08X PA: %08X - %08X A: %d %d %d %d D: %d C: %d B: %d\n", indx,
			  cpt[i] & 0xFFFF0000, (cpt[i] & 0xFFFF0000) | 0xFFFF,
			  (cpt[i] >> 10) & 3, (cpt[i] >> 8) & 3, (cpt[i] >> 6) & 3,
			  (cpt[i] >> 4) & 3, dom, (cpt[i] >> 3) & 1, (cpt[i] >> 2) & 1);
			*/
                    break;
                case 2:
			//a=((cpt[i] & 0xff000000)>>24);
			/*printf ("  VA: %08X PA: %08X - %08X A: %d %d %d %d D: %d C: %d B: %d\n", indx,
			  cpt[i] & 0xfffff000, (cpt[i] & 0xfffff000) | 0xFFF,
			  (cpt[i] >> 10) & 3, (cpt[i] >> 8) & 3, (cpt[i] >> 6) & 3,
			  (cpt[i] >> 4) & 3, dom, (cpt[i] >> 3) & 1, (cpt[i] >> 2) & 1);
			*/
			// This is where we look for any virtual addresses that map to physical address 0x03000000 and
			// alter the cachable and bufferable bits...
			if (((cpt[i] & 0xff000000) == 0x02000000) )
			{
				//printf("SOUND c and b bits not set, overwriting\n");
				if((cpt[i] & 12)==0) {
					cpt[i] |= 0xFFC;
					wb++;
				}
			}
			//if ((a>=0x31 && a<=0x36) && ((cpt[i] & 12)==0))
			if (((cpt[i] & 0xff000000) == 0x03000000) )
			{
				//printf("SDL   c and b bits not set, overwriting\n");
				if((cpt[i] & 12)==0) {
					cpt[i] |= 0xFFC;
					wb++;
				}
			}
			break;
                case 3:
                    //printf ("  -- [%08X/%d] Unsupported --\n", cpt[i],cpt[i] & 3);
                    break;
                default:
                    //printf ("  -- [%08X/%d] Unknown --\n", cpt[i], cpt[i] & 3);
                    break;
            }
        }
        indx += 4096;
    }
    //printf ("%08X %08X %08X %08X\n", cpt[0], cpt[4], cpt[8], cpt[12]);
    if (wb)
    {
        //printf("Hacking cpt\n");
        lseek (gp2x_dev_mem, sa, SEEK_SET);
        temp = write (gp2x_dev_mem, cpt, 256*4);
        printf("%d bytes written, %s\n", temp, temp == 1024 ? "yay!" : "oh fooble :(!");
    }
}

void dumppgtable (unsigned int ttb)
{
    unsigned int pgtable[4096];
    char *desctypes[] = {"Invalid", "Coarse", "Section", "Fine"};

    memset (pgtable, 0, 4096*4);
    lseek (gp2x_dev_mem, ttb, SEEK_SET);
    read (gp2x_dev_mem, pgtable, 4096*4);

    int i;
    for (i = 0; i < 4096; i ++)
    {
        int temp;
        
        if (pgtable[i])
        {
		//printf ("Indx: %d VA: %08X Type: %d [%s] \n", i, i * 1048576, pgtable[i] & 3, desctypes[pgtable[i]&3]);
            switch (pgtable[i]&3)
            {
                case 0:
                    //printf (" -- Invalid --\n");
                    break;
                case 1:
                    DecodeCoarse(i, pgtable[i]);
                    break;
                case 2:
                    temp = pgtable[i] & 0xFFF00000;
                    //printf ("  PA: %08X - %08X A: %d D: %d C: %d B: %d\n", temp, temp | 0xFFFFF,
                    //        (pgtable[i] >> 10) & 3, (pgtable[i] >> 5) & 15, (pgtable[i] >> 3) & 1,
                    //        (pgtable[i] >> 2) & 1);
                            
                    break;
                case 3:
                    printf ("  -- Unsupported! --\n");
                    break;
            }
        }
    }
}

int hackpgtable (void)
{
    unsigned int oldc1, oldc2, oldc3, oldc4;
    unsigned int newc1 = 0xee120f10, newc2 = 0xe12fff1e;
    unsigned int ttb, ttx;
    int a=0;int try=0;
    // We need to execute a "MRC p15, 0, r0, c2, c0, 0", to get the pointer to the translation table base, but we can't
    // do this in user mode, so we have to patch the kernel to get it to run it for us in supervisor mode. We do this
    // at the moment by overwriting the sys_newuname function and then calling it.

    lseek (gp2x_dev_mem, 0x6ec00, SEEK_SET); // fixme: We should ask the kernel for this address rather than assuming it...
    read (gp2x_dev_mem, &oldc1, 4);
    read (gp2x_dev_mem, &oldc2, 4);
    read (gp2x_dev_mem, &oldc3, 4);
    read (gp2x_dev_mem, &oldc4, 4);

    printf ("0:%08X %08X - %08X %08X\n", oldc1, oldc2, newc1, newc2);

   
    
    printf ("point1 %d\n",a);
    do {
	    lseek (gp2x_dev_mem, 0x6ec00, SEEK_SET);
	    a+=write (gp2x_dev_mem, &newc1, 4);
	    a+=write (gp2x_dev_mem, &newc2, 4);    
	    SDL_Delay(200);
	    try++;
	    ttb = myuname(name);
	    printf ("2:%08X try %d\n", ttb,try);
    } while (ttb==0 && try<4);


    
    lseek (gp2x_dev_mem, 0x6ec00, SEEK_SET);
    a+=write (gp2x_dev_mem, &oldc1, 4);
    a+=write (gp2x_dev_mem, &oldc2, 4);    

    printf ("2:%08X %d\n", ttb,a);
    if (ttb!=0) {
   

	    //printf ("Restored contents\n");
    
	    // We now have the translation table base ! Walk the table looking for our allocation and hack it :)
	    dumppgtable(ttb);    

	    // Now drain the write buffer and flush the tlb caches. Something else we can't do in user mode...
	    unsigned int tlbc1 = 0xe3a00000; // mov    r0, #0
	    unsigned int tlbc2 = 0xee070f9a; // mcr    15, 0, r0, cr7, cr10, 4
	    unsigned int tlbc3 = 0xee080f17; // mcr    15, 0, r0, cr8, cr7, 0
	    unsigned int tlbc4 = 0xe1a0f00e; // mov    pc, lr

	    lseek (gp2x_dev_mem, 0x6ec00, SEEK_SET);
	    write (gp2x_dev_mem, &tlbc1, 4);
	    write (gp2x_dev_mem, &tlbc2, 4);    
	    write (gp2x_dev_mem, &tlbc3, 4);    
	    write (gp2x_dev_mem, &tlbc4, 4);    

	    SDL_Delay(200);

	    ttx = myuname(name);
    
	    printf ("Return from uname: %08X\n", ttx);
    
	    lseek (gp2x_dev_mem, 0x6ec00, SEEK_SET);
	    write (gp2x_dev_mem, &oldc1, 4);
	    write (gp2x_dev_mem, &oldc2, 4);    
	    write (gp2x_dev_mem, &oldc3, 4);    
	    write (gp2x_dev_mem, &oldc4, 4);    
	    lseek (gp2x_dev_mem, 0x0, SEEK_SET);
	    return 0;
    }
    
    lseek (gp2x_dev_mem, 0x0, SEEK_SET);
    return 1;
    //printf ("Restored contents\n");

    //printf ("Pagetable after modification!\n");
    //printf ("-------------------------------\n");
    //dumppgtable(ttb);
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


void gp2x_init(void) {
	volatile unsigned int *secbuf = (unsigned int *)malloc (204800);

	gp2x_ram_init();

	/* Fix tvout */
	tvoutfix_sav=gp2x_memregs[0x28E4>>1];
	gp2x_memregs[0x28E4>>1]=gp2x_memregs[0x290C>>1];

	if (CF_BOOL(cf_get_item_by_name("sound"))) {
		gp2x_mixer=open("/dev/mixer", O_RDWR);
		if (gp2x_mixer==-1) {
			printf("Can't open mixer\n");
		}
	}
	//setpriority(PRIO_PROCESS,0,-20);

	/* here come the magic squidge :) */
	
	//hackpgtable();
	//benchmark ((void*)gp2x_ram);
	//benchmark ((void*)secbuf);
	

	gn_init_skin();
}


#endif
