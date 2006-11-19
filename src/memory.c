/*  gngeo a neogeo emulator
 *  Copyright (C) 2001 Peponas Mathieu
 * 
 *  This program is free software; you can redistribute it and/or modify  
 *  it under the terms of the GNU General Public License as published by   
 *  the Free Software Foundation; either version 2 of the License, or    
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. 
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "emu.h"
#include "video.h"
#include "memory.h"
#include "pd4990a.h"
#include "transpack.h"


/* TODO: cleanup irq2 code */
int irq2enable, irq2start, irq2repeat = 1000, irq2control, irq2taken;
int lastirq2line = 1000;
int irq2repeat_limit;
Uint32 irq2pos_value;
Uint32 bankaddress = 0;

int neogeo_irq2type = 0;
//Uint8 neo_memcard[0x1000];
//Uint8 neo_memcard[0x800];
extern int current_line;



static void draw_border(SDL_Surface *buf,int x,int y,int w,int h,Uint32 color) {
    SDL_Rect rect;
    rect.x=x,rect.y=y;
    rect.w=w;rect.h=1;
    SDL_FillRect(buf,&rect,color);
    rect.h=h;rect.w=1;
    SDL_FillRect(buf,&rect,color);
    rect.x=x+w-1,rect.y=y;
    SDL_FillRect(buf,&rect,color);
    rect.x=x,rect.y=y+h-1;
    rect.w=w;rect.h=1;
    SDL_FillRect(buf,&rect,color);
}

void dump_hardware_reg(void) {
    int i,j;
    Uint8 *vidram=memory.video;
    Uint16 tctl1,tctl2,tctl3;
    Uint16 x=0,y=0,zx=0,zy=0,nb_tile=0,first_x=0,total_z=0;
    static SDL_Surface *video_dump=NULL;
    if (video_dump==NULL)
	video_dump=SDL_CreateRGBSurface(SDL_SWSURFACE,512+32, 512+0x3f*16+32, 16, 0xF800, 0x7E0, 0x1F, 0);
    SDL_FillRect(video_dump,NULL,0x0000);
    printf("Sprite Bank:\n");
    for(i=0;i<0x300;i+=2) {
	tctl3 = READ_WORD_ROM( &vidram[0x10000 + i] ); /* Zoom */
        tctl1 = READ_WORD_ROM( &vidram[0x10400 + i] ); /* Y pos + nb tile */
        tctl2 = READ_WORD_ROM( &vidram[0x10800 + i] ); /* X pos */
	if (tctl1&0x40) {
	    /* strip is part of the same block that the last one: y,zy don't change */
	    x += zx+1;
	    total_z+=zx+1;
	} else { /* new block */
	    if (i!=0) { /* print info of the last plane */
		if (nb_tile>0) {
		    printf("TilePlane: x,y=%d,%d zx,zy=%d,%d nb_tile=%d\n",first_x,y,total_z,zy,nb_tile);
		    draw_border(video_dump,first_x+16,y+16,total_z,((zy+1)*nb_tile/16.0),0xFE00);
		}
	    }
	    first_x=x=(tctl2>>7)&0x1FF;
	    y=0x200-((tctl1>>7)&0x1FF);
	    zy=tctl3&0xFF;
	    nb_tile=tctl1&0x3f;
	    total_z=(tctl3>>8)&0xF;
	}
	zx=(tctl3>>8)&0xF;
	
	if (nb_tile>0) {
	    int yy=y;
	    int offs = i<<6;
	    printf("SB %04x: x,y=%d,%d zx,zy=%d,%d nb_tile=%d\n",i>>1,x,y,zx,zy,nb_tile);
	    for(j=0;j<nb_tile;j++) {
		int tileno,tileatr;
		tileno = READ_WORD_ROM(&vidram[offs]);offs+=2;
		tileatr = READ_WORD_ROM(&vidram[offs]);offs+=2;

		if (memory.nb_of_tiles>0x10000 && tileatr&0x10) tileno+=0x10000;
		if (memory.nb_of_tiles>0x20000 && tileatr&0x20) tileno+=0x20000;
		if (memory.nb_of_tiles>0x40000 && tileatr&0x40) tileno+=0x40000;
		/*
		  if (tileatr&0x8) {
		  tileno=(tileno&~7)+((tileno+neogeo_frame_counter)&7);
		  } else {
		  if (tileatr&0x4) {
		  tileno=(tileno&~3)+((tileno+neogeo_frame_counter)&3);
		  }
		  }
		*/	
		if (tileno<=memory.nb_of_tiles) {
			/*
		    if (memory.pen_usage[tileno]==TILE_UNCONVERTED) {
			convert_tile(tileno);
			}*/
		    if (memory.pen_usage[tileno]!=TILE_INVISIBLE) {
			//printf("Tile %d %d %d\n",tileno,x,yy);
			debug_draw_tile(tileno,x+16,yy+16,16/*zx+1*/,16/*(zy+1)/16.0*/,tileatr>>8,tileatr & 0x01,tileatr & 0x02,
					(Uint8*)video_dump->pixels);
			
		    }
		    //draw_border(video_dump,x+16,yy+16,zx+1,(zy+1)/16.0,0xFFFF);
		}
		yy+=(zy+1)/16.0;
	    }
	}
    }
    draw_border(video_dump,16,16,496,496,0x001F);
    SDL_SaveBMP(video_dump,"/tmp/dump.bmp");
}

void neogeo_sound_irq(int irq)
{
    //printf("neogeo_sound_irq %d\n",irq);
    if (irq) {
	cpu_z80_raise_irq(0);
    } else
	cpu_z80_lower_irq();
    //printf("neogeo_sound_end %d\n",irq);
}

static __inline__ Uint16 read_neo_control(void)
{
    int irq_bit;
    int scan, bm_pos;
    //static int cycles;

    if (!conf.raster) {

#ifdef GP2X
#ifdef USE_CYCLONE
	    scan = current_line;
/*
	    printf("%d %d %d\n",current_line,
		   (cpu_68k_getcycle()/3)>>7,
		   (int)(cpu_68k_getcycle() / 766.28));
*/
#else
	    scan = cpu_68k_getcycle()/3;
	    scan = scan>>7;
#endif
#else
	scan = cpu_68k_getcycle() / 766.28;	/* current scanline */
#endif

	irq_bit = ((scan < 64 || scan > 256)?1:0);
	//irq_bit = ((scan > 248 && scan < 262)?1:0);
	//printf("%d\n",scan);
	return ((scan <<7) & 0xff80)	/* scanline */
		|(irq_bit << 15)	/* vblank or irq2 */
	    |(conf.pal<<3)
	    |(neogeo_frame_counter & 0x0007);	/* frame counter */
    } else {
	    scan = current_line /*+ 22*/;	/* current scanline */
	    irq_bit = irq2taken || (scan < 16 || scan > 248);	/* VBLANK or HBLANK */

	return ((scan <<7) & 0xff80)	/* scanline */
		|(irq_bit << 15)	/* vblank or irq2 */
	    |(conf.pal<<3)
	    |(neogeo_frame_counter & 0x0007);	/* frame counter */
    }
}

__inline__ void write_neo_control(Uint16 data)
{
    neogeo_frame_counter_speed = (((data >> 8) & 0xff)+1);
    irq2control = data & 0xff;
    return;
}

__inline__ void write_irq2pos(Uint32 data)
{
    irq2pos_value = data;
    if (irq2control & 0x20) {
	int line = (irq2pos_value + 0x3b) / 0x180;	/* turfmast goes as low as 0x145 */
	irq2start = line + current_line;
    }
}

/* Z80 IO port handler */
Uint8 z80_port_read(Uint16 PortNo)
{
        //printf("z80_port_read PC=%04x p=%04x ",cpu_z80_get_pc(),PortNo);
	//printf("z80_port_read p=%04x \n",PortNo);
    switch (PortNo & 0xff) {
    case 0x0:
	pending_command = 0;
	//printf("Reseting command. Return sndcode %x\n",sound_code);
	return sound_code;
	break;

    case 0x4:
	//printf("v=%02x\n",YM2610_status_port_0_A_r(0));
	return YM2610_status_port_0_A_r(0);
	break;

    case 0x5:
	//printf("v=%02x\n",YM2610_read_port_0_r(0));
	return YM2610_read_port_0_r(0);
	break;

    case 0x6:
	//printf("v=%02x\n",YM2610_status_port_0_B_r(0));
	return YM2610_status_port_0_B_r(0);
	break;

    case 0x08:
	//printf("v=00 (sb3)\n");
	cpu_z80_switchbank(3, PortNo);
	return 0;
	break;

    case 0x09:
	//printf("v=00 (sb2)\n");
	cpu_z80_switchbank(2, PortNo);
	return 0;
	break;

    case 0x0a:
	//printf("v=00 (sb1)\n");
	cpu_z80_switchbank(1, PortNo);
	return 0;
	break;

    case 0x0b:
	//printf("v=00 (sb0)\n");
	cpu_z80_switchbank(0, PortNo);
	return 0;
	break;
    };

    return 0;
}

void z80_port_write(Uint16 PortNb, Uint8 Value)
{
    Uint8 data = Value;
    //printf("z80_port_write PC=%04x OP=%02x p=%04x v=%02x\n",cpu_z80_get_pc(),memory.sm1[cpu_z80_get_pc()],PortNb,Value);
    //printf("Write port %04x %02x\n",PortNb,Value);
    switch (PortNb & 0xff) {
    case 0x4:
	YM2610_control_port_0_A_w(0, data);
	break;

    case 0x5:
	YM2610_data_port_0_A_w(0, data);
	break;

    case 0x6:
	YM2610_control_port_0_B_w(0, data);
	break;

    case 0x7:
	YM2610_data_port_0_B_w(0, data);
	break;

    case 0xC:
	    //printf("Setting result code to %0x\n",Value);
	result_code = Value;
	break;
    }
}

/* Protection hack */
Uint16 protection_9a37(Uint32 addr)
{
	return 0x9a37;
}

/* fetching function */
/**** INVALID FETCHING ****/
Uint8 mem68k_fetch_invalid_byte(Uint32 addr)
{
    return 0xF0;
}

Uint16 mem68k_fetch_invalid_word(Uint32 addr)
{
    return 0xF0F0;
}

Uint32 mem68k_fetch_invalid_long(Uint32 addr)
{
    return 0xF0F0F0F0;
}

/**** RAM FETCHING ****/
Uint8 mem68k_fetch_ram_byte(Uint32 addr)
{
    //  printf("mem68k_fetch_ram_byte %x\n",addr);
    addr &= 0xffff;
    return (READ_BYTE_ROM(memory.ram + addr));
}

Uint16 mem68k_fetch_ram_word(Uint32 addr)
{
	//printf("mem68k_fetch_ram_word %08x %04x\n",addr,READ_WORD_RAM(memory.ram + (addr&0xffff)));
    addr &= 0xffff;
    return (READ_WORD_ROM(memory.ram + addr));
}

LONG_FETCH(mem68k_fetch_ram);

/**** CPU ****/
Uint8 mem68k_fetch_cpu_byte(Uint32 addr)
{
    addr &= 0xFFFFF;
    return (READ_BYTE_ROM(memory.cpu + addr));
}

Uint16 mem68k_fetch_cpu_word(Uint32 addr)
{
    addr &= 0xFFFFF;
    return (READ_WORD_ROM(memory.cpu + addr));
}

LONG_FETCH(mem68k_fetch_cpu);

/**** BIOS ****/
Uint8 mem68k_fetch_bios_byte(Uint32 addr)
{
    addr &= 0xFFFFF;
    return (READ_BYTE_ROM(memory.bios + addr));
}

Uint16 mem68k_fetch_bios_word(Uint32 addr)
{
    addr &= 0xFFFFF;
    return (READ_WORD_ROM(memory.bios + addr));
}

LONG_FETCH(mem68k_fetch_bios);

/**** SRAM ****/
Uint8 mem68k_fetch_sram_byte(Uint32 addr)
{
    return memory.sram[addr - 0xd00000];
}

Uint16 mem68k_fetch_sram_word(Uint32 addr)
{
    addr -= 0xd00000;
    return (memory.sram[addr] << 8) | (memory.sram[addr + 1] & 0xff);
}

LONG_FETCH(mem68k_fetch_sram);

/**** PALETTE ****/
Uint8 mem68k_fetch_pal_byte(Uint32 addr)
{
    addr &= 0xffff;
    if (addr <= 0x1fff)
	return current_pal[addr];
    return 0;
}

Uint16 mem68k_fetch_pal_word(Uint32 addr)
{
    addr &= 0xffff;
    if (addr <= 0x1fff)
	return READ_WORD(&current_pal[addr]);
    return 0;
}

LONG_FETCH(mem68k_fetch_pal);

/**** VIDEO ****/
Uint8 mem68k_fetch_video_byte(Uint32 addr)
{
    //  printf("mem68k_fetch_video_byte %08x\n",addr);
    addr &= 0xFFFF;
    if (addr == 0xe)
	return 0xff;
    return 0;
}

Uint16 mem68k_fetch_video_word(Uint32 addr)
{
    addr &= 0xFFFF;
    /*
      if (addr==0x00) 
      return vptr;
    */
    if (addr == 0x00 || addr == 0x02 || addr == 0x0a)
	return READ_WORD(&memory.video[vptr << 1]);
    if (addr == 0x04)
	return modulo;
    if (addr == 0x06)
	return read_neo_control();
    return 0;
}
LONG_FETCH(mem68k_fetch_video);

/**** CONTROLLER ****/
Uint8 mem68k_fetch_ctl1_byte(Uint32 addr)
{
    Uint8 a;
    addr &= 0xFFFF;
    if (addr == 0x00)
	return memory.intern_p1;
    if (addr == 0x01)
	return (conf.test_switch ? 0xFE : 0xFF);

    if (addr == 0x81) {
	return (conf.test_switch ? 0x00 : 0x80);
    }


    return 0;
}

Uint16 mem68k_fetch_ctl1_word(Uint32 addr)
{
    //  printf("mem68k_fetch_ctl1_word\n");
    return 0;
}

Uint32 mem68k_fetch_ctl1_long(Uint32 addr)
{
    //  printf("mem68k_fetch_ctl1_long\n");
    return 0;
}

Uint8 mem68k_fetch_ctl2_byte(Uint32 addr)
{
    if ((addr & 0xFFFF) == 0x00)
	return memory.intern_p2;
    if ((addr & 0xFFFF) == 0x01)
	return 0xFF;
    return 0;
}

Uint16 mem68k_fetch_ctl2_word(Uint32 addr)
{
    return 0;
}

Uint32 mem68k_fetch_ctl2_long(Uint32 addr)
{
    return 0;
}

Uint8 mem68k_fetch_ctl3_byte(Uint32 addr)
{
	//printf("Fetch ctl3 byte %x\n",addr);
    if ((addr & 0xFFFF) == 0x0)
	    return memory.intern_start;
    return 0;
}

Uint16 mem68k_fetch_ctl3_word(Uint32 addr)
{
	/*
	printf("Fetch ctl3 word %x\n",addr);
	if ((addr & 0xFFFF) == 0x0)
		return memory.intern_start | 0xFF00;
	*/
    return 0;
}

Uint32 mem68k_fetch_ctl3_long(Uint32 addr)
{
    return 0;
}

Uint8 mem68k_fetch_coin_byte(Uint32 addr)
{
    int a;
    addr &= 0xFFFF;
    if (addr == 0x1) {
	int coinflip = read_4990_testbit();
	int databit = read_4990_databit();
	return memory.intern_coin ^ (coinflip << 6) ^ (databit << 7);
    }
    if (addr == 0x0) {
	int res = 0;
	if (conf.sound) {
		//printf("fetch coin byte, rescoe= %x\n",result_code);
	    res |= result_code;
	    if (pending_command)
		res &= 0x7f;
	} else {
	    res |= 0x01;
	}
	return res;
    }
    return 0;
}

Uint16 mem68k_fetch_coin_word(Uint32 addr)
{
    return 0;
}

Uint32 mem68k_fetch_coin_long(Uint32 addr)
{
    return 0;
}


/**** MEMCARD ****/
/* Even byte are FF 
   Odd  byte are data;
*/
Uint8 mem68k_fetch_memcrd_byte(Uint32 addr)
{
	addr&=0xFFF;
	if (addr&1)
		return 0xFF;
	else
		return memory.memcard[addr>>1];
}

Uint16 mem68k_fetch_memcrd_word(Uint32 addr)
{
	addr&=0xFFF;
	return memory.memcard[addr>>1] | 0xff00;
}

Uint32 mem68k_fetch_memcrd_long(Uint32 addr)
{
    return 0;
}

/* storring function */
/**** INVALID STORE ****/
void mem68k_store_invalid_byte(Uint32 addr, Uint8 data)
{
}
void mem68k_store_invalid_word(Uint32 addr, Uint16 data)
{
}
void mem68k_store_invalid_long(Uint32 addr, Uint32 data)
{
}

/**** RAM ****/
void mem68k_store_ram_byte(Uint32 addr, Uint8 data)
{
    addr &= 0xffff;
    WRITE_BYTE_ROM(memory.ram + addr,data);
    return;
}

void mem68k_store_ram_word(Uint32 addr, Uint16 data)
{
	//printf("Store rom word %08x %04x\n",addr,data);
    addr &= 0xffff;
    WRITE_WORD_ROM(memory.ram + addr,data);
    return;
}

LONG_STORE(mem68k_store_ram);

/**** SRAM ****/
void mem68k_store_sram_byte(Uint32 addr, Uint8 data)
{
    if (sram_lock)
	return;
    if (addr == 0xd00000 + sram_protection_hack && ((data & 0xff) == 0x01))
	return;
    memory.sram[addr - 0xd00000] = data;
}

void mem68k_store_sram_word(Uint32 addr, Uint16 data)
{
    if (sram_lock)
	return;
    if (addr == 0xd00000 + sram_protection_hack
	&& ((data & 0xffff) == 0x01))
	return;
    addr -= 0xd00000;
    memory.sram[addr] = data >> 8;
    memory.sram[addr + 1] = data & 0xff;
}

LONG_STORE(mem68k_store_sram);

/**** PALETTE ****/
static __inline__ Uint16 convert_pal(Uint16 npal)
{
    int r = 0, g = 0, b = 0;
    r = ((npal >> 7) & 0x1e) | ((npal >> 14) & 0x01);
    g = ((npal >> 3) & 0x1e) | ((npal >> 13) & 0x01);
    b = ((npal << 1) & 0x1e) | ((npal >> 12) & 0x01);
    return (r << 11) + (g << 6) + b;
}

void update_all_pal(void) {
    int i;
    Uint32 *pc_pal1=(Uint32*)memory.pal_pc1;
    Uint32 *pc_pal2=(Uint32*)memory.pal_pc2;
    for (i=0;i<0x1000;i++) {
	pc_pal1[i] = convert_pal(READ_WORD_ROM(&memory.pal1[i<<1]));
	pc_pal2[i] = convert_pal(READ_WORD_ROM(&memory.pal2[i<<1]));
    }
}

void mem68k_store_pal_byte(Uint32 addr, Uint8 data)
{
    /* TODO: verify this */
    addr &= 0xffff;
    if (addr <= 0x1fff) {
	Uint16 a = READ_WORD(&current_pal[addr & 0xfffe]);
	if (addr & 0x1)
	    a = data | (a & 0xff00);
	else
	    a = (a & 0xff) | (data << 8);
	WRITE_WORD(&current_pal[addr & 0xfffe], a);
	if ((addr >> 1)&0xF)
		current_pc_pal[(addr) >> 1] = convert_pal(a);
	else 
		current_pc_pal[(addr) >> 1] = 0xF81F;
    }
}

void mem68k_store_pal_word(Uint32 addr, Uint16 data)
{
	//printf("Store pal word @ %08x %08x %04x\n",cpu_68k_getpc(),addr,data);
    addr &= 0xffff;
    if (addr <= 0x1fff) {
	WRITE_WORD(&current_pal[addr], data);
	if ((addr >> 1)&0xF)
		current_pc_pal[(addr) >> 1] = convert_pal(data);
	else 
		current_pc_pal[(addr) >> 1] = 0xF81F;
    }
}

LONG_STORE(mem68k_store_pal);


/**** VIDEO ****/
void mem68k_store_video_byte(Uint32 addr, Uint8 data)
{
    /* garou write at 3c001f, 3c000f, 3c0015 */
    /* wjammers write, and fetch at 3c0000 .... */
    //  printf("mem68k_store_video_byte %x %x @pc=%08x\n",addr,data,cpu_68k_getpc());
}

void mem68k_store_video_word(Uint32 addr, Uint16 data)
{
	//SDL_Swap16(data);
	//printf("mem68k_store_video_word %08x %04x\n",addr,data);
#if 0
    addr &= 0xFFFF;
    if (addr == 0x0) {
	vptr = data & 0xffff;
	return;
    }
    if (addr == 0x2) {

	//if (((vptr<<1)==0x10800+0x8) ) printf("Store to video %08x @pc=%08x\n",vptr<<1,cpu_68k_getpc());

	WRITE_WORD(&memory.video[vptr << 1], data);
	vptr = (vptr + modulo) & 0xffff;
	return;
    }
    if (addr == 0x4) {
	modulo = (int) data;
	return;
    }
    if (addr == 0x6) {
	write_neo_control(data);
	return;
    }

    if (addr == 0x8) {
	write_irq2pos((irq2pos_value & 0xffff) | ((Uint32) data << 16));
	return;
    }
    if (addr == 0xa) {
	write_irq2pos((irq2pos_value & 0xffff0000) | (Uint32) data);
	return;
    }
    if (addr == 0xc) {
	/* games write 7 or 4 at 0x3c000c at every frame */
	/* IRQ acknowledge */
	return;
    }
    //  printf("mem68k_store_video_word %x %x (line=%d) @pc=%08x\n",addr,data,current_line,cpu_68k_getpc());
#else
    addr &= 0xFFFF;
    switch (addr) {
    case 0x0:
	vptr = data & 0xffff;
	break;
    case 0x2:	
	//if (((vptr<<1)==0x10800+0x8) ) printf("Store to video %08x @pc=%08x\n",vptr<<1,cpu_68k_getpc());
	/*
	  if (((vptr<<1)==0x10000+0x17e) ||
	  ((vptr<<1)==0x10400+0x17e) ||
	  ((vptr<<1)==0x10800+0x17e) ) printf("Store to video %08x @pc=%08x\n",vptr<<1,cpu_68k_getpc());
	*/
	WRITE_WORD(&memory.video[vptr << 1], data);
	vptr = (vptr + modulo) & 0xffff;
	break;
    case 0x4:
	modulo = (int) data;
	break;
    case 0x6:
	write_neo_control(data);
	break;
    case 0x8:
	write_irq2pos((irq2pos_value & 0xffff) | ((Uint32) data << 16));
	break;
    case 0xa:
	write_irq2pos((irq2pos_value & 0xffff0000) | (Uint32) data);
	break;
    case 0xc:
	/* games write 7 or 4 at 0x3c000c at every frame */
	/* IRQ acknowledge */
	break;
    }
       

#endif
}
LONG_STORE(mem68k_store_video);
/*
void mem68k_store_video_long(Uint32 addr, Uint32 data) {
	//printf("mem68k_store_video_long %08x %08X\n",addr,data);
	mem68k_store_video_word(addr,data>>16);
	mem68k_store_video_word(addr+2,data & 0xffff);
}
*/

/**** PD4990 ****/
void mem68k_store_pd4990_byte(Uint32 addr, Uint8 data)
{
    write_4990_control_w(addr, data);
}

void mem68k_store_pd4990_word(Uint32 addr, Uint16 data)
{
    write_4990_control_w(addr, data);
}

void mem68k_store_pd4990_long(Uint32 addr, Uint32 data)
{
    write_4990_control_w(addr, data);
}

/**** Z80 ****/
void mem68k_store_z80_byte(Uint32 addr, Uint8 data)
{
    if (addr == 0x320000) {
	sound_code = data & 0xff;
	pending_command = 1;
	//printf("Pending command. Sound_code=%02x\n",sound_code);
	if (conf.sound) {
	    cpu_z80_nmi();
	    cpu_z80_run(300);
	}
    }
}
void mem68k_store_z80_word(Uint32 addr, Uint16 data)
{
    /* tpgolf use word store for sound */
    if (addr == 0x320000) {
	sound_code = data >> 8;
	pending_command = 1;
	//printf("Pending command. Sound_code=%02x\n",sound_code);
	if (conf.sound) {
	    cpu_z80_nmi();
	    cpu_z80_run(300);
	}
    }
}
void mem68k_store_z80_long(Uint32 addr, Uint32 data)
{
    /* I don't think any game will use long store for sound.... */
    printf("Z80L %x %04x\n", addr, data);
}

/**** SETTINGS ****/
void mem68k_store_setting_byte(Uint32 addr, Uint8 data)
{
	//printf("mem68k_store_setting_byte %08x\n",addr);
    if (addr == 0x3a0003) {
	memcpy(memory.cpu,memory.bios,0x80);
    }
  
    if (addr == 0x3a0013) {
	memcpy(memory.cpu,memory.game_vector,0x80);
    }
  
    if (addr == 0x3a000b) {	/* select board fix */
	current_fix = memory.sfix_board;
	fix_usage = memory.fix_board_usage;
	return;
    }
    if (addr == 0x3a001b) {	/* select game fix */
	current_fix = memory.sfix_game;
	fix_usage = memory.fix_game_usage;
	return;
    }
    if (addr == 0x3a000d) {	/* sram lock */
	sram_lock = 1;
	return;
    }
    if (addr == 0x3a001d) {	/* sram unlock */
	sram_lock = 0;
	return;
    }
    if (addr == 0x3a000f) {	/* set palette 2 */
	current_pal = memory.pal2;
	current_pc_pal = (Uint32 *) memory.pal_pc2;
	return;
    }
    if (addr == 0x3a001f) {	/* set palette 1 */
	current_pal = memory.pal1;
	current_pc_pal = (Uint32 *) memory.pal_pc1;
	return;
    }
    /* garou write 0 to 3a0001 -> enable display, 3a0011 -> disable display */
    //printf("unknow mem68k_store_setting_byte %x %x\n",addr,data);

}

void mem68k_store_setting_word(Uint32 addr, Uint16 data)
{
    /* TODO: check if this is used */
      printf("mem68k_store_setting_word USED????\n");
    addr &= 0xFFFFFe;
    if (addr == 0x3a0002) {
	memcpy(memory.cpu,memory.bios,0x80);
    }
  
    if (addr == 0x3a0012) {
	memcpy(memory.cpu,memory.game_vector,0x80);
    }
    if (addr == 0x3a000a) {
	current_fix = memory.sfix_board;
	fix_usage = memory.fix_board_usage;
	return;
    }
    if (addr == 0x3a001a) {
	current_fix = memory.sfix_game;
	fix_usage = memory.fix_game_usage;
	return;
    }
    if (addr == 0x3a000c) {
	sram_lock = 1;
	return;
    }
    if (addr == 0x3a001c) {
	sram_lock = 0;
	return;
    }
    if (addr == 0x3a000e) {
	current_pal = memory.pal2;
	current_pc_pal = (Uint32 *) memory.pal_pc2;
	return;
    }
    if (addr == 0x3a001e) {
	current_pal = memory.pal1;
	current_pc_pal = (Uint32 *) memory.pal_pc1;
	return;
    }
}

void mem68k_store_setting_long(Uint32 addr, Uint32 data)
{
    //printf("setting long\n");
}

/**** MEMCARD ****/
/* TODO: mem card */
void mem68k_store_memcrd_byte(Uint32 addr, Uint8 data)
{
	addr&=0xFFF;
	memory.memcard[addr>>1]=data;
}
void mem68k_store_memcrd_word(Uint32 addr, Uint16 data)
{
	addr&=0xFFF;
	memory.memcard[addr>>1]=data&0xff;
}
void mem68k_store_memcrd_long(Uint32 addr, Uint32 data)
{
}

/**** bankswitchers ****/

/* Normal bankswitcher */
Uint8 mem68k_fetch_bk_normal_byte(Uint32 addr)
{
    addr &= 0xFFFFF;
    return (READ_BYTE_ROM(memory.cpu + bankaddress + addr));
}

Uint16 mem68k_fetch_bk_normal_word(Uint32 addr)
{
    addr &= 0xFFFFF;
    return (READ_WORD_ROM(memory.cpu + bankaddress + addr));
}

LONG_FETCH(mem68k_fetch_bk_normal);

static void bankswitch(Uint32 address, Uint8 data)
{

    if (memory.cpu_size <= 0x100000)
	return;

    if (address >= 0x2FFFF0) {
	data = data & 0x7;
	bankaddress = (data + 1) * 0x100000;
    } else
	return;

    if (bankaddress >= memory.cpu_size)
	bankaddress = 0x100000;
    cpu_68k_bankswitch(bankaddress);
}

void mem68k_store_bk_normal_byte(Uint32 addr, Uint8 data)
{
    //if (addr<0x2FFFF0) printf("bankswitch_b %x %x\n",addr,data);
    bankswitch(addr, data);
}

void mem68k_store_bk_normal_word(Uint32 addr, Uint16 data)
{
    //if (addr<0x2FFFF0)  printf("bankswitch_w %x %x\n",addr,data);
    bankswitch(addr, data);
}

LONG_STORE(mem68k_store_bk_normal);
    
/* Scramble bankswitcher */
void scramble_bankswitch(Uint32 address, Uint8 data) {
    if (address == memory.bksw_handler) {
	data =
	    (((data >> memory.bksw_unscramble[0]) & 1) << 0) +
	    (((data >> memory.bksw_unscramble[1]) & 1) << 1) +
	    (((data >> memory.bksw_unscramble[2]) & 1) << 2) +
	    (((data >> memory.bksw_unscramble[3]) & 1) << 3) +
	    (((data >> memory.bksw_unscramble[4]) & 1) << 4) +
	    (((data >> memory.bksw_unscramble[5]) & 1) << 5);
	bankaddress = 0x100000 + memory.bksw_offset[data];
    } else
	return;
    if (bankaddress >= memory.cpu_size)
	bankaddress = 0x100000;
    cpu_68k_bankswitch(bankaddress);
}

/* Kof2003 bankswitcher */
Uint8 mem68k_fetch_bk_kof2003_byte(Uint32 addr)
{
    if (addr<0x2fe000) {
	addr &= 0xFFFFF;
	return (READ_BYTE_ROM(memory.cpu + bankaddress + addr));
    } else {
	return (READ_BYTE_ROM(memory.kof2003_bksw + addr - 0x2fe000));
    }
}

Uint16 mem68k_fetch_bk_kof2003_word(Uint32 addr)
{
    if (addr<0x2fe000) {
	addr &= 0xFFFFF;
	return (READ_WORD_ROM(memory.cpu + bankaddress + addr));
    } else {
	return (READ_WORD_ROM(memory.kof2003_bksw + addr - 0x2fe000));
    }
}

LONG_FETCH(mem68k_fetch_bk_kof2003);

void bankswitch_kof2003(Uint32 offset, Uint16 mem_mask, Uint16 data) {
    Uint32 bankaddress;
    WRITE_WORD_ROM(memory.kof2003_bksw+offset, (READ_WORD_ROM(memory.kof2003_bksw+offset)&mem_mask)|((~mem_mask)&data));
    if(offset!=0x1ff2 && offset!=0x1ff0) return;
    bankaddress=((READ_BYTE_ROM(memory.kof2003_bksw+0x1ff0))|(READ_WORD_ROM(memory.kof2003_bksw+0x1ff2)<<8))+0x100000;
    WRITE_BYTE_ROM(memory.kof2003_bksw+0x1ff1,0xa0);
    WRITE_BYTE_ROM(memory.kof2003_bksw+0x1ff0,READ_BYTE_ROM(memory.kof2003_bksw+0x1ff0)&0xfe);
    WRITE_BYTE_ROM(memory.kof2003_bksw+0x1ff2,READ_BYTE_ROM(memory.kof2003_bksw+0x1ff2)&0x7f);
    cpu_68k_bankswitch(bankaddress);
    WRITE_BYTE_ROM(memory.cpu+0x58197,READ_BYTE_ROM(memory.kof2003_bksw + 0x1ff3));
}

void mem68k_store_bk_kof2003_byte(Uint32 addr, Uint8 data) {
    Uint32 offset=(addr-0x2fe000);
    if (addr<0x2fe000) return;
    if (addr & 1) {
	offset^=1;
	bankswitch_kof2003(offset,0xff00,((Uint16)data));
    } else {
	bankswitch_kof2003(offset,0x00ff,((Uint16)data)<<8);
    }
}
void mem68k_store_bk_kof2003_word(Uint32 addr, Uint16 data) {
    Uint32 offset=(addr-0x2fe000);
    if (addr<0x2fe000) return;
    bankswitch_kof2003(offset,0x0,data);
}

LONG_STORE(mem68k_store_bk_kof2003);
