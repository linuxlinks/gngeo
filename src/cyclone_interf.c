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

/* cyclone interface */



#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef USE_CYCLONE

#include <stdlib.h>

#include "cyclone/Cyclone.h"
#include "memory.h"
#include "emu.h"
#include "state.h"
#include "debug.h"
#include "video.h"
#include "conf.h"

struct Cyclone MyCyclone;
static int total_cycles;
static int time_slice;
Uint32 cyclone_pc;
extern int current_line;

extern Uint32 irq2pos_value;
static __inline__ void cyclone68k_store_video_word(Uint32 addr, Uint16 data)
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

static void print_one_reg(Uint32 r) {
	printf("reg=%08x\n",r);
}

static void swap_memory(Uint8 * mem, Uint32 length)
{
    int i, j;

    /* swap bytes in each word */
    for (i = 0; i < length; i += 2) {
	j = mem[i];
	mem[i] = mem[i + 1];
	mem[i + 1] = j;
    }
}

static unsigned int   MyCheckPc(unsigned int pc) {
	int i;
	pc-=MyCyclone.membase; // Get the real program counter

	pc&=0xFFFFFF;
	MyCyclone.membase=-1;

/*	printf("## Check pc %08x\n",pc);
	for(i=0;i<8;i++) printf("  d%d=%08x a%d=%08x\n",i,MyCyclone.d[i],i,MyCyclone.a[i]); */

	//printf("PC %08x %08x\n",(pc&0xF00000),(pc&0xF00000)>>20);
	switch((pc&0xF00000)>>20) {
	case 0x0:
		MyCyclone.membase=(int)memory.rom.cpu_m68k.p;
		break;
	case 0x2:
		MyCyclone.membase=(int)(memory.rom.cpu_m68k.p+bankaddress)-0x200000;
		break;
	case 0x1:
		if (pc<=0x10FFff) 
			MyCyclone.membase=(int)memory.ram-0x100000;
		break;
	case 0xC:
		if (pc<=0xc1FFff)
			MyCyclone.membase=(int)memory.rom.bios_m68k.p-0xc00000;
		break;
	}
#if 0
	if (pc>=0x0 && pc<=0xFFFFF) MyCyclone.membase=(int)memory.rom.cpu_m68k.p;
	if (pc>=0x200000 && pc<=0x2FFFfF) {
		MyCyclone.membase=(int)(memory.rom.cpu_m68k.p+bankaddress)-0x200000;
	}
	if (pc>=0x100000 && pc<=0x10FFff) {
		MyCyclone.membase=(int)memory.ram-0x100000;
	}
	if (pc>=0xc00000 && pc<=0xc1FFff) {
		MyCyclone.membase=(int)memory.rom.bios_m68k.p-0xc00000;
		//printf("PC=%08x MemBase=%08x bios=%08x\n",pc,MyCyclone.membase,(int)memory.rom.bios_m68k.p);
	}

	//if (pc>=0xff0000) MyCyclone.membase=(int)RamMem-0xff0000; // Jump to Ram
#endif
 	if (MyCyclone.membase==-1) {
 		printf("ERROROROOR %08x\n",pc);
 		exit(1);
 	}

	return MyCyclone.membase+pc; // New program counter
}

#define MEMHANDLER_READ(start,end,func) {if (a>=start && a<=end) return func(a);} 
#define MEMHANDLER_WRITE(start,end,func) {if (a>=start && a<=end) {func(a,d);return;}}

static unsigned char  MyRead8  (unsigned int a) {
	unsigned int addr=a&0xFFFFF;
	unsigned int b=((a&0xF0000)>>16);
	a&=0xFFFFFF;
#if 0	
	switch((a&0xFF0000)>>16) {
	case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05:
	case 0x06: case 0x07: case 0x08: case 0x09: case 0x0A: case 0x0B:
	case 0x0C: case 0x0D: case 0x0E: case 0x0F:
		return (READ_BYTE_ROM(memory.rom.cpu_m68k.p + addr));
		break;
	case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25:
	case 0x26: case 0x27: case 0x28: case 0x29: case 0x2A: case 0x2B:
	case 0x2C: case 0x2D: case 0x2E: case 0x2F:
		return (READ_BYTE_ROM(memory.rom.cpu_m68k.p + bankaddress + addr));
		break;
	case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15:
	case 0x16: case 0x17: case 0x18: case 0x19: case 0x1A: case 0x1B:
	case 0x1C: case 0x1D: case 0x1E: case 0x1F:
		return (READ_BYTE_ROM(memory.ram + (addr&0xFFFF)));
		break;
	case 0xC0: case 0xC1:
		return (READ_BYTE_ROM(memory.rom.bios_m68k.p + addr));
		break;

	case 0xd0:
		return mem68k_fetch_sram_byte(a);
		break;
	case 0x40:
		return mem68k_fetch_pal_byte(a);
		break;
	case 0x3C:
		return mem68k_fetch_video_byte(a);
		break;
	case 0x30:
		return mem68k_fetch_ctl1_byte(a);
		break;
	case 0x34:
		return mem68k_fetch_ctl2_byte(a);
		break;
	case 0x38:
		return mem68k_fetch_ctl3_byte(a);
		break;
	case 0x32:
		return mem68k_fetch_coin_byte(a);
		break;
	case 0x80:
		return mem68k_fetch_memcrd_byte(a);
		break;
	}
#else
	switch((a&0xFF0000)>>20) {
	case 0x0:
		return (READ_BYTE_ROM(memory.rom.cpu_m68k.p + addr));
		break;
	case 0x2:
		return (READ_BYTE_ROM(memory.rom.cpu_m68k.p + bankaddress + addr));
		break;
	case 0x1:
		return (READ_BYTE_ROM(memory.ram + (addr&0xFFFF)));
		break;
	case 0xC:
		if (b<=1) return (READ_BYTE_ROM(memory.rom.bios_m68k.p + addr));
		break;
	case 0xd:
		if (b==0) return mem68k_fetch_sram_byte(a);
		break;
	case 0x4:
		if (b==0) return mem68k_fetch_pal_byte(a);
		break;
	case 0x3:
		if (b==0xC) return mem68k_fetch_video_byte(a);
		if (b==0) return mem68k_fetch_ctl1_byte(a);
		if (b==4) return mem68k_fetch_ctl2_byte(a);
		if (b==8) return mem68k_fetch_ctl3_byte(a);
		if (b==2) return mem68k_fetch_coin_byte(a);
		break;
	case 0x8:
		if (b==0) return mem68k_fetch_memcrd_byte(a);
		break;
	}
#endif
/*
	MEMHANDLER_READ(0,0xFFFFF,mem68k_fetch_cpu_byte);
	MEMHANDLER_READ(0x200000,0x2FFFFF,mem68k_fetch_bk_normal_byte);
	MEMHANDLER_READ(0x100000,0x10FFFF,mem68k_fetch_ram_byte);
	MEMHANDLER_READ(0xc00000,0xc1FFFF,mem68k_fetch_bios_byte);


	MEMHANDLER_READ(0xd00000, 0xd0ffff, mem68k_fetch_sram_byte);
	MEMHANDLER_READ(0x400000, 0x401fff, mem68k_fetch_pal_byte);
	MEMHANDLER_READ(0x3c0000, 0x3c0fff, mem68k_fetch_video_byte);
	MEMHANDLER_READ(0x300000, 0x300fff, mem68k_fetch_ctl1_byte);
	MEMHANDLER_READ(0x340000, 0x340fff, mem68k_fetch_ctl2_byte);
	MEMHANDLER_READ(0x380000, 0x380fff, mem68k_fetch_ctl3_byte);
	MEMHANDLER_READ(0x320000, 0x320fff, mem68k_fetch_coin_byte);
	MEMHANDLER_READ(0x800000, 0x800fff, mem68k_fetch_memcrd_byte);
	printf("Unhandled read8  @ %08x\n",a);
*/
	return 0xFF;
}
static unsigned short MyRead16 (unsigned int a) {
	unsigned int addr=a&0xFFFFF;
	unsigned int b=((a&0xF0000)>>16);
	//printf("read 32 %08x\n",a);
	a&=0xFFFFFF;
#if 0
	


	switch((a&0xFF0000)>>16) {
	case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05:
	case 0x06: case 0x07: case 0x08: case 0x09: case 0x0A: case 0x0B:
	case 0x0C: case 0x0D: case 0x0E: case 0x0F:
		return (READ_WORD_ROM(memory.rom.cpu_m68k.p + addr));
		break;
	case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25:
	case 0x26: case 0x27: case 0x28: case 0x29: case 0x2A: case 0x2B:
	case 0x2C: case 0x2D: case 0x2E: case 0x2F:
		return (READ_WORD_ROM(memory.rom.cpu_m68k.p + bankaddress + addr));
		break;
	case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15:
	case 0x16: case 0x17: case 0x18: case 0x19: case 0x1A: case 0x1B:
	case 0x1C: case 0x1D: case 0x1E: case 0x1F:
		return (READ_WORD_ROM(memory.ram + (addr&0xFFFF)));
		break;
	case 0xC0: case 0xC1:
		return (READ_WORD_ROM(memory.rom.bios_m68k.p + addr));
		break;

	case 0xd0:
		return mem68k_fetch_sram_word(a);
		break;
	case 0x40:
		return mem68k_fetch_pal_word(a);
		break;
	case 0x3C:
		return mem68k_fetch_video_word(a);
		break;
	case 0x30:
		return mem68k_fetch_ctl1_word(a);
		break;
	case 0x34:
		return mem68k_fetch_ctl2_word(a);
		break;
	case 0x38:
		return mem68k_fetch_ctl3_word(a);
		break;
	case 0x32:
		return mem68k_fetch_coin_word(a);
		break;
	case 0x80:
		return mem68k_fetch_memcrd_word(a);
		break;
	}
#else
	switch((a&0xFF0000)>>20) {
	case 0x0:
		return (READ_WORD_ROM(memory.rom.cpu_m68k.p + addr));
		break;
	case 0x2:
		return (READ_WORD_ROM(memory.rom.cpu_m68k.p + bankaddress + addr));
		break;
	case 0x1:
		return (READ_WORD_ROM(memory.ram + (addr&0xFFFF)));
		break;
	case 0xC:
		if (b<=1) return (READ_WORD_ROM(memory.rom.bios_m68k.p + addr));
		break;

	case 0xd:
		if (b==0) return mem68k_fetch_sram_word(a);
		break;
	case 0x4:
		if (b==0) return mem68k_fetch_pal_word(a);
		break;
	case 0x3:
		if (b==0xC) return mem68k_fetch_video_word(a);
		if (b==0) return mem68k_fetch_ctl1_word(a);
		if (b==4) return mem68k_fetch_ctl2_word(a);
		if (b==8) return mem68k_fetch_ctl3_word(a);
		if (b==2) return mem68k_fetch_coin_word(a);
		break;
	case 0x8:
		if (b==0) return mem68k_fetch_memcrd_word(a);
		break;
	}
#endif
/*
	MEMHANDLER_READ(0,0xFFFFF,mem68k_fetch_cpu_word);
	MEMHANDLER_READ(0x200000,0x2FFFFF,mem68k_fetch_bk_normal_word);
	MEMHANDLER_READ(0x100000,0x10FFFF,mem68k_fetch_ram_word);
	MEMHANDLER_READ(0xc00000,0xc1FFFF,mem68k_fetch_bios_word);

	MEMHANDLER_READ(0xd00000, 0xd0ffff, mem68k_fetch_sram_word);
	MEMHANDLER_READ(0x400000, 0x401fff, mem68k_fetch_pal_word);
	MEMHANDLER_READ(0x3c0000, 0x3c0fff, mem68k_fetch_video_word);
	MEMHANDLER_READ(0x300000, 0x300fff, mem68k_fetch_ctl1_word);
	MEMHANDLER_READ(0x340000, 0x340fff, mem68k_fetch_ctl2_word);
	MEMHANDLER_READ(0x380000, 0x380fff, mem68k_fetch_ctl3_word);
	MEMHANDLER_READ(0x320000, 0x320fff, mem68k_fetch_coin_word);
	MEMHANDLER_READ(0x800000, 0x800fff, mem68k_fetch_memcrd_word);
	printf("Unhandled read16 @ %08x\n",a);
*/
	return 0xF0F0;
}
static unsigned int   MyRead32 (unsigned int a) {
	//int i;
	unsigned int addr=a&0xFFFFF;
	unsigned int b=((a&0xF0000)>>16);
	a&=0xFFFFFF;
#if 0
	//printf("read 32 %08x\n",a);
	



	switch((a&0xFF0000)>>16) {
	case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05:
	case 0x06: case 0x07: case 0x08: case 0x09: case 0x0A: case 0x0B:
	case 0x0C: case 0x0D: case 0x0E: case 0x0F:
		//return mem68k_fetch_cpu_long(a);
		return ((READ_WORD_ROM(memory.rom.cpu_m68k.p + addr))<<16) | 
			(READ_WORD_ROM(memory.rom.cpu_m68k.p + (addr+2)));
		break;
	case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25:
	case 0x26: case 0x27: case 0x28: case 0x29: case 0x2A: case 0x2B:
	case 0x2C: case 0x2D: case 0x2E: case 0x2F:
		//return mem68k_fetch_bk_normal_long(a);
		return ((READ_WORD_ROM(memory.rom.cpu_m68k.p + bankaddress + addr))<<16) | 
			(READ_WORD_ROM(memory.rom.cpu_m68k.p + bankaddress + (addr+2)));
		break;
	case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15:
	case 0x16: case 0x17: case 0x18: case 0x19: case 0x1A: case 0x1B:
	case 0x1C: case 0x1D: case 0x1E: case 0x1F:
		//return mem68k_fetch_ram_long(a);
		addr&=0xFFFF;
		return ((READ_WORD_ROM(memory.ram + addr))<<16) | 
			(READ_WORD_ROM(memory.ram + (addr+2)));
		break;
	case 0xC0: case 0xC1:
		//return mem68k_fetch_bios_long(a);
		return ((READ_WORD_ROM(memory.rom.bios_m68k.p + addr))<<16) | 
			(READ_WORD_ROM(memory.rom.bios_m68k.p + (addr+2)));
		break;

	case 0xd0:
		return mem68k_fetch_sram_long(a);
		break;
	case 0x40:
		return mem68k_fetch_pal_long(a);
		break;
	case 0x3C:
		return mem68k_fetch_video_long(a);
		break;
	case 0x30:
		return mem68k_fetch_ctl1_long(a);
		break;
	case 0x34:
		return mem68k_fetch_ctl2_long(a);
		break;
	case 0x38:
		return mem68k_fetch_ctl3_long(a);
		break;
	case 0x32:
		return mem68k_fetch_coin_long(a);
		break;
	case 0x80:
		return mem68k_fetch_memcrd_long(a);
		break;
	}
#else
	switch((a&0xFF0000)>>20) {
	case 0x0:
		//return mem68k_fetch_cpu_long(a);
		return ((READ_WORD_ROM(memory.rom.cpu_m68k.p + addr))<<16) | 
			(READ_WORD_ROM(memory.rom.cpu_m68k.p + (addr+2)));
		break;
	case 0x2:
		//return mem68k_fetch_bk_normal_long(a);
		return ((READ_WORD_ROM(memory.rom.cpu_m68k.p + bankaddress + addr))<<16) | 
			(READ_WORD_ROM(memory.rom.cpu_m68k.p + bankaddress + (addr+2)));
		break;
	case 0x1:
		//return mem68k_fetch_ram_long(a);
		addr&=0xFFFF;
		return ((READ_WORD_ROM(memory.ram + addr))<<16) | 
			(READ_WORD_ROM(memory.ram + (addr+2)));
		break;
	case 0xC:
		//return mem68k_fetch_bios_long(a);
		if (b<=1) return ((READ_WORD_ROM(memory.rom.bios_m68k.p + addr))<<16) | 
				 (READ_WORD_ROM(memory.rom.bios_m68k.p + (addr+2)));
		break;

	case 0xd:
		if (b==0) return mem68k_fetch_sram_long(a);
		break;
	case 0x4:
		if (b==0) return mem68k_fetch_pal_long(a);
		break;
	case 0x3:
		if (b==0xC) return mem68k_fetch_video_long(a);
		if (b==0) return mem68k_fetch_ctl1_long(a);
		if (b==4) return mem68k_fetch_ctl2_long(a);
		if (b==8) return mem68k_fetch_ctl3_long(a);
		if (b==2) return mem68k_fetch_coin_long(a);
		break;
	case 0x8:
		if (b==0) return mem68k_fetch_memcrd_long(a);
		break;
	}
#endif
/*
	MEMHANDLER_READ(0,0xFFFFF,mem68k_fetch_cpu_long);
	MEMHANDLER_READ(0x200000,0x2FFFFF,mem68k_fetch_bk_normal_long);
	MEMHANDLER_READ(0x100000,0x10FFFF,mem68k_fetch_ram_long);
	MEMHANDLER_READ(0xc00000,0xc1FFFF,mem68k_fetch_bios_long);

	MEMHANDLER_READ(0xd00000, 0xd0ffff, mem68k_fetch_sram_long);
	MEMHANDLER_READ(0x400000, 0x401fff, mem68k_fetch_pal_long);
	MEMHANDLER_READ(0x3c0000, 0x3c0fff, mem68k_fetch_video_long);
	MEMHANDLER_READ(0x300000, 0x300fff, mem68k_fetch_ctl1_long);
	MEMHANDLER_READ(0x340000, 0x340fff, mem68k_fetch_ctl2_long);
	MEMHANDLER_READ(0x380000, 0x380fff, mem68k_fetch_ctl3_long);
	MEMHANDLER_READ(0x320000, 0x320fff, mem68k_fetch_coin_long);
	MEMHANDLER_READ(0x800000, 0x800fff, mem68k_fetch_memcrd_long);
	printf("Unhandled read32 @ %08x\n",a);
*/
/*
	cpu_68k_dumpreg();
	printf("%04x\n",mem68k_fetch_bios_word(0x00c110fc));
	printf("%04x\n",mem68k_fetch_bios_word(0x00c110fe));
	printf("%04x\n",mem68k_fetch_bios_word(0x00c11100));
*/
	return 0xFF00FF00;
}
static void MyWrite8 (unsigned int a,unsigned char  d) {
	unsigned int b=((a&0xF0000)>>16);
	a&=0xFFFFFF;
#if 0
	switch((a&0xFF0000)>>16) {
	case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15:
	case 0x16: case 0x17: case 0x18: case 0x19: case 0x1A: case 0x1B:
	case 0x1C: case 0x1D: case 0x1E: case 0x1F:
		WRITE_BYTE_ROM(memory.ram + (a&0xffff),d);
		return ;
		//mem68k_store_ram_byte(a,d);return;
		break;
	case 0x3c:
		mem68k_store_video_byte(a,d);return;
		break;
	case 0x40:
		mem68k_store_pal_byte(a,d);return;
		break;
	case 0xD0:
		mem68k_store_sram_byte(a,d);return;
		break;
	case 0x38:
		mem68k_store_pd4990_byte(a,d);return;
		break;
	case 0x32:
		mem68k_store_z80_byte(a,d);return;
		break;
	case 0x3A:
		mem68k_store_setting_byte(a,d);return;
		break;
	case 0x2F:
		mem68k_store_bk_normal_byte(a,d);return;
		break;
	case 0x80:
		mem68k_store_memcrd_byte(a,d);return;
		break;

	}
#else
	switch((a&0xFF0000)>>20) {
	case 0x1:
		WRITE_BYTE_ROM(memory.ram + (a&0xffff),d);
		return ;
		break;
	case 0x3:
		if (b==0xc) {mem68k_store_video_byte(a,d);return;}
		if (b==8) {mem68k_store_pd4990_byte(a,d);return;}
		if (b==2) {mem68k_store_z80_byte(a,d);return;}
		if (b==0xA) {mem68k_store_setting_byte(a,d);return;}
		break;
	case 0x4:
		if (b==0) mem68k_store_pal_byte(a,d);
		return;
		break;
	case 0xD:
		if (b==0) mem68k_store_sram_byte(a,d);return;
		break;
	case 0x2:
		if (b==0xF) mem68k_store_bk_normal_byte(a,d);return;
		break;
	case 0x8:
		if (b==0) mem68k_store_memcrd_byte(a,d);return;
		break;

	}
#endif
/*
	MEMHANDLER_WRITE(0x100000,0x10FFFF,mem68k_store_ram_byte);
	MEMHANDLER_WRITE(0x3c0000, 0x3c0fff, mem68k_store_video_byte);
	MEMHANDLER_WRITE(0x400000, 0x401fff, mem68k_store_pal_byte);
	MEMHANDLER_WRITE(0xd00000, 0xd0ffff, mem68k_store_sram_byte);
	MEMHANDLER_WRITE(0x380000, 0x380fff, mem68k_store_pd4990_byte);
	MEMHANDLER_WRITE(0x320000, 0x320fff, mem68k_store_z80_byte);
	MEMHANDLER_WRITE(0x3a0000, 0x3a0fff, mem68k_store_setting_byte);
	MEMHANDLER_WRITE(0x200000, 0x2fffff, mem68k_store_bk_normal_byte);
	MEMHANDLER_WRITE(0x800000, 0x800fff, mem68k_store_memcrd_byte);
*/
	if(a==0x300001) memory.watchdog=0; // Watchdog

	//printf("Unhandled write8  @ %08x = %02x\n",a,d);
}
static void MyWrite16(unsigned int a,unsigned short d) {
	unsigned int b=((a&0xF0000)>>16);
	a&=0xFFFFFF;
#if 0
	switch((a&0xFF0000)>>16) {
	case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15:
	case 0x16: case 0x17: case 0x18: case 0x19: case 0x1A: case 0x1B:
	case 0x1C: case 0x1D: case 0x1E: case 0x1F:
		WRITE_WORD_ROM(memory.ram + (a&0xffff),d);
		return;
		//mem68k_store_ram_word(a,d);return;
		break;
	case 0x3c:
		cyclone68k_store_video_word(a,d);return;
		//mem68k_store_video_word(a,d);return;
		break;
	case 0x40:
		mem68k_store_pal_word(a,d);return;
		break;
	case 0xD0:
		mem68k_store_sram_word(a,d);return;
		break;
	case 0x38:
		mem68k_store_pd4990_word(a,d);return;
		break;
	case 0x32:
		mem68k_store_z80_word(a,d);return;
		break;
	case 0x3A:
		mem68k_store_setting_word(a,d);return;
		break;
	case 0x2F:
		mem68k_store_bk_normal_word(a,d);return;
		break;
	case 0x80:
		mem68k_store_memcrd_word(a,d);return;
		break;
	}
#else
	switch((a&0xFF0000)>>20) {
	case 0x1:
		WRITE_WORD_ROM(memory.ram + (a&0xffff),d);
		return;
		//mem68k_store_ram_word(a,d);return;
		break;
	case 0x3:
		if (b==0xc) {cyclone68k_store_video_word(a,d);return;}
		if (b==8) {mem68k_store_pd4990_word(a,d);return;}
		if (b==2) {mem68k_store_z80_word(a,d);return;}
		if (b==0xA) {mem68k_store_setting_word(a,d);return;}
		break;	
	case 0x4:
		if (b==0) mem68k_store_pal_word(a,d);return;
		break;
	case 0xD:
		if (b==0) mem68k_store_sram_word(a,d);return;
		break;
	case 0x2:
		if (b==0xF) mem68k_store_bk_normal_word(a,d);return;
		break;
	case 0x8:
		if (b==0) mem68k_store_memcrd_word(a,d);return;
		break;
	}
#endif

/*
	MEMHANDLER_WRITE(0x100000,0x10FFFF,mem68k_store_ram_word);
	MEMHANDLER_WRITE(0x3c0000, 0x3c0fff, mem68k_store_video_word);
	MEMHANDLER_WRITE(0x400000, 0x401fff, mem68k_store_pal_word);
	MEMHANDLER_WRITE(0xd00000, 0xd0ffff, mem68k_store_sram_word);
	MEMHANDLER_WRITE(0x380000, 0x380fff, mem68k_store_pd4990_word);
	MEMHANDLER_WRITE(0x320000, 0x320fff, mem68k_store_z80_word);
	MEMHANDLER_WRITE(0x3a0000, 0x3a0fff, mem68k_store_setting_word);
	MEMHANDLER_WRITE(0x200000, 0x2fffff, mem68k_store_bk_normal_word);
	MEMHANDLER_WRITE(0x800000, 0x800fff, mem68k_store_memcrd_word);
*/
	//printf("Unhandled write16 @ %08x = %04x\n",a,d);
}
static void MyWrite32(unsigned int a,unsigned int   d) {
	unsigned int b=((a&0xF0000)>>16);
	a&=0xFFFFFF;
#if 0
	switch((a&0xFF0000)>>16) {
	case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15:
	case 0x16: case 0x17: case 0x18: case 0x19: case 0x1A: case 0x1B:
	case 0x1C: case 0x1D: case 0x1E: case 0x1F:
		WRITE_WORD_ROM(memory.ram + (a&0xffff),d>>16);
		WRITE_WORD_ROM(memory.ram + (a&0xffff)+2,d&0xFFFF);
		return;
		//mem68k_store_ram_long(a,d);return;
		break;
	case 0x3c:
		//mem68k_store_video_long(a,d);return;
		mem68k_store_video_word(a,d>>16);
		mem68k_store_video_word(a+2,d & 0xffff);
		return;
		break;
	case 0x40:
		mem68k_store_pal_long(a,d);return;
		break;
	case 0xD0:
		mem68k_store_sram_long(a,d);return;
		break;
	case 0x38:
		mem68k_store_pd4990_long(a,d);return;
		break;
	case 0x32:
		mem68k_store_z80_long(a,d);return;
		break;
	case 0x3A:
		mem68k_store_setting_long(a,d);return;
		break;
	case 0x2F:
		mem68k_store_bk_normal_long(a,d);return;
		break;
	case 0x80:
		mem68k_store_memcrd_long(a,d);return;
		break;
	}
#else
		switch((a&0xFF0000)>>20) {
	case 0x1:
		WRITE_WORD_ROM(memory.ram + (a&0xffff),d>>16);
		WRITE_WORD_ROM(memory.ram + (a&0xffff)+2,d&0xFFFF);
		return;
		break;
	case 0x3:
		if (b==0xc) {
			mem68k_store_video_word(a,d>>16);
			mem68k_store_video_word(a+2,d & 0xffff);
			return;
		}
		if (b==8) {mem68k_store_pd4990_long(a,d);return;}
		if (b==2) {mem68k_store_z80_long(a,d);return;}
		if (b==0xA) {mem68k_store_setting_long(a,d);return;}
		break;	
	case 0x4:
		if (b==0) mem68k_store_pal_long(a,d);return;
		break;
	case 0xD:
		if (b==0) mem68k_store_sram_long(a,d);return;
		break;
	case 0x2:
		if (b==0xF) mem68k_store_bk_normal_long(a,d);return;
		break;
	case 0x8:
		if (b==0) mem68k_store_memcrd_long(a,d);return;
		break;
	}
#endif
/*
	MEMHANDLER_WRITE(0x100000,0x10FFFF,mem68k_store_ram_long);
	MEMHANDLER_WRITE(0x3c0000, 0x3c0fff, mem68k_store_video_long);
	MEMHANDLER_WRITE(0x400000, 0x401fff, mem68k_store_pal_long);
	MEMHANDLER_WRITE(0xd00000, 0xd0ffff, mem68k_store_sram_long);
	MEMHANDLER_WRITE(0x380000, 0x380fff, mem68k_store_pd4990_long);
	MEMHANDLER_WRITE(0x320000, 0x320fff, mem68k_store_z80_long);
	MEMHANDLER_WRITE(0x3a0000, 0x3a0fff, mem68k_store_setting_long);
	MEMHANDLER_WRITE(0x200000, 0x2fffff, mem68k_store_bk_normal_long);
	MEMHANDLER_WRITE(0x800000, 0x800fff, mem68k_store_memcrd_long);
*/
	//printf("Unhandled write32 @ %08x = %08x\n",a,d);
}

static void cpu_68k_post_load_state(void) {
	
	MyCyclone.read8=MyRead8;
	MyCyclone.read16=MyRead16;
	MyCyclone.read32=MyRead32;

	MyCyclone.write8=MyWrite8;
	MyCyclone.write16=MyWrite16;
	MyCyclone.write32=MyWrite32;

	MyCyclone.checkpc=MyCheckPc;

	MyCyclone.fetch8  =MyRead8;
        MyCyclone.fetch16 =MyRead16;
        MyCyclone.fetch32 =MyRead32;

	cpu_68k_bankswitch(bankaddress);
	MyCyclone.membase=0;
	MyCyclone.pc=MyCheckPc(cyclone_pc);
	//printf("Loaded PC=%08x\n",cyclone_pc);
}

static void cpu_68k_pre_save_state(void) {
	cyclone_pc=MyCyclone.pc-MyCyclone.membase;
	//printf("Saved PC=%08x\n",cyclone_pc);
}

static void cpu_68k_init_save_state(void) {
	
	create_state_register(ST_68k,"cyclone68k",1,(void *)&MyCyclone,sizeof(MyCyclone),REG_UINT8);
	create_state_register(ST_68k,"pc",1,(void *)&cyclone_pc,sizeof(Uint32),REG_UINT32);
	create_state_register(ST_68k,"bank",1,(void *)&bankaddress,sizeof(Uint32),REG_UINT32);
	create_state_register(ST_68k,"ram",1,(void *)memory.ram,0x10000,REG_UINT8);
	create_state_register(ST_68k,"kof2003_bksw",1,(void *)memory.kof2003_bksw,0x1000,REG_UINT8);
	create_state_register(ST_68k,"current_vector",1,(void *)memory.rom.cpu_m68k.p,0x80,REG_UINT8);
	set_post_load_function(ST_68k,cpu_68k_post_load_state);
	set_pre_save_function(ST_68k,cpu_68k_pre_save_state);
	
}

int cpu_68k_getcycle(void) {
	return total_cycles-MyCyclone.cycles;
}
void bankswitcher_init() {
	bankaddress=0;
}
int cyclone_debug(unsigned short o) {
	printf("CYCLONE DEBUG %04x\n",o);
	return 0;
}

void cpu_68k_init(void) {
	int overclk=CF_VAL(cf_get_item_by_name("68kclock"));
	//printf("INIT \n");
	CycloneInit();
	memset(&MyCyclone, 0,sizeof(MyCyclone));
	/*
	swap_memory(memory.rom.cpu_m68k.p, memory.rom.cpu_m68k.size);
	swap_memory(memory.rom.bios_m68k.p, memory.rom.bios_m68k.size);
	swap_memory(memory.game_vector, 0x80);
	*/
	MyCyclone.read8=MyRead8;
	MyCyclone.read16=MyRead16;
	MyCyclone.read32=MyRead32;

	MyCyclone.write8=MyWrite8;
	MyCyclone.write16=MyWrite16;
	MyCyclone.write32=MyWrite32;

	MyCyclone.checkpc=MyCheckPc;

	MyCyclone.fetch8  =MyRead8;
        MyCyclone.fetch16 =MyRead16;
        MyCyclone.fetch32 =MyRead32;

	//MyCyclone.InvalidOpCallback=cyclone_debug;
	//MyCyclone.print_reg=print_one_reg;
	bankswitcher_init();

	

	if (memory.rom.cpu_m68k.size > 0x100000) {
		bankaddress = 0x100000;
	}
	cpu_68k_init_save_state();


	time_slice=(overclk==0?
		    200000:200000+(overclk*200000/100.0))/262.0;
}

void cpu_68k_reset(void) {

	//printf("Reset \n");
	MyCyclone.srh=0x27; // Set supervisor mode
	//CycloneSetSr(&MyCyclone,0x27);
	//MyCyclone.srh=0x20;
	//MyCyclone.irq=7;
	MyCyclone.irq=0;
	MyCyclone.a[7]=MyCyclone.read32(0);
	
	MyCyclone.membase=0;
	MyCyclone.pc=MyCyclone.checkpc(MyCyclone.read32(4)); // Get Program Counter

	//printf("PC=%08x\n SP=%08x\n",MyCyclone.pc-MyCyclone.membase,MyCyclone.a[7]);
}

int cpu_68k_run(Uint32 nb_cycle) {
	Uint32 i;
	if (conf.raster) {
		total_cycles=nb_cycle;MyCyclone.cycles=nb_cycle;	
		CycloneRun(&MyCyclone);
		return -MyCyclone.cycles;
	} else {
#if 0
		current_line=0;
		
		total_cycles=nb_cycle;
		//MyCyclone.cycles=nb_cycle;
		MyCyclone.cycles=0;//time_slice;
		for (i=0;i<261;i++) {
			MyCyclone.cycles+=time_slice;
			CycloneRun(&MyCyclone);
			current_line++;
		}
#else
		total_cycles=nb_cycle;MyCyclone.cycles=nb_cycle;	
		CycloneRun(&MyCyclone);
#endif
		return -MyCyclone.cycles;
	}
		//return 0;
}

void cpu_68k_interrupt(int a) {
	//printf("Interrupt %d\n",a);
	MyCyclone.irq=a;
}

void cpu_68k_bankswitch(Uint32 address) {
	//printf("Bankswitch %08x\n",address);
	bankaddress = address;
}

void cpu_68k_disassemble(int pc, int nb_instr) {
	/* TODO */
}

void cpu_68k_dumpreg(void) {
	int i;
	for(i=0;i<8;i++) printf("  d%d=%08x a%d=%08x\n",i,MyCyclone.d[i],i,MyCyclone.a[i]);
	/*printf("stack:\n");
	for (i=0;i<10;i++) {
		printf("%02d - %08x\n",i,mem68k_fetch_ram_long(MyCyclone.a[7]+i*4));
		}*/
	
}

int cpu_68k_run_step(void) {
	MyCyclone.cycles=0;
	CycloneRun(&MyCyclone);
	return -MyCyclone.cycles;
}

Uint32 cpu_68k_getpc(void) {
	return MyCyclone.pc-MyCyclone.membase;
}

void cpu_68k_fill_state(M68K_STATE *st) {
}


void cpu_68k_set_state(M68K_STATE *st) {
}

int cpu_68k_debuger(void (*execstep)(void),void (*dump)(void)) {
	/* TODO */
	return 0;
}



#endif
