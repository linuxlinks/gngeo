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

#include "940shared.h"
#include "mvs.h"
#include "DrZ80.h"
#include "2610intf.h"

Uint16 z80_bank[4];
static Uint8 *z80map1, *z80map2, *z80map3, *z80map4;
Uint8 drz80mem[0x10000];
Uint32 mydrz80_Z80PC,mydrz80_Z80SP;

struct DrZ80 mydrz80;
//extern Z80_Regs Z80;

unsigned int drz80_rebasePC(unsigned short address)
{
	//if (address==0x66) 
	//printf("Rebase PC %x\n",address);
        mydrz80.Z80PC_BASE = (unsigned int)drz80mem;
	mydrz80.Z80PC = mydrz80.Z80PC_BASE + address;
        return mydrz80.Z80PC_BASE + address;
}

unsigned int drz80_rebaseSP(unsigned short address)
{
	//printf("Rebase SP %x\n",address);
        mydrz80.Z80SP_BASE = (unsigned int)drz80mem;
	mydrz80.Z80SP = mydrz80.Z80SP_BASE + address;
	return mydrz80.Z80SP_BASE + address;
}

unsigned char drz80_read8(unsigned short address) {
        return (drz80mem[address&0xFFFF]);
}

unsigned short drz80_read16(unsigned short address) {
        return drz80_read8(address) | (drz80_read8(address + 1) << 8);
}

void drz80_write8(unsigned char data,unsigned short address) {
        if (address>=0xf800) drz80mem[address&0xFFFF]=data;
}

void drz80_write16(unsigned short data,unsigned short address) {
        drz80_write8(data & 0xFF,address);
        drz80_write8(data >> 8,address + 1);
}
/* cpu interface implementation */
void cpu_z80_switchbank(Uint8 bank, Uint16 PortNo)
{
	//printf("Switch bank %x %x\n",bank,PortNo);
    if (bank<=3)
	z80_bank[bank]=PortNo;

    switch (bank) {
    case 0:
	z80map1 = shared_data->sm1 + (0x4000 * ((PortNo >> 8) & 0x0f));
	memcpy(drz80mem + 0x8000, z80map1, 0x4000);
	break;
    case 1:
	z80map2 = shared_data->sm1 + (0x2000 * ((PortNo >> 8) & 0x1f));
	memcpy(drz80mem + 0xc000, z80map2, 0x2000);
	break;
    case 2:
	z80map3 = shared_data->sm1 + (0x1000 * ((PortNo >> 8) & 0x3f));
	memcpy(drz80mem + 0xe000, z80map3, 0x1000);
	break;
    case 3:
	z80map4 = shared_data->sm1 + (0x0800 * ((PortNo >> 8) & 0x7f));
	memcpy(drz80mem + 0xf000, z80map4, 0x0800);
	break;
    }
}


/* Z80 IO port handler */
Uint8 z80_port_read(Uint16 PortNo)
{
        //printf("z80_port_read PC=%04x p=%04x ",cpu_z80_get_pc(),PortNo);
	//printf("z80_port_read p=%04x \n",PortNo);
	switch (PortNo & 0xff) {
	case 0x0:
		shared_ctl->pending_command = 0;
		//printf("Reseting command. Return sndcode %x\n",sound_code);
		return shared_ctl->sound_code;
		break;

	case 0x4:
		//printf("v=%02x\n",YM2610_status_port_0_A_r(0));
		return YM2610_status_port_A_r(0);
		break;

	case 0x5:
		//printf("v=%02x\n",YM2610_read_port_0_r(0));
		return YM2610_read_port_r(0);
		break;

	case 0x6:
		//printf("v=%02x\n",YM2610_status_port_0_B_r(0));
		return YM2610_status_port_B_r(0);
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
		YM2610_control_port_A_w(0, data);
		break;

	case 0x5:
		YM2610_data_port_A_w(0, data);
		break;

	case 0x6:
		YM2610_control_port_B_w(0, data);
		break;

	case 0x7:
		YM2610_data_port_B_w(0, data);
		break;

	case 0xC:
		//printf("Setting result code to %0x\n",Value);
		shared_ctl->result_code = Value;
		break;
	}
}

void drz80_writeport16(Uint16 port, Uint8 value)
{
	//printf("Write port %d=%d\n",port,value);
    z80_port_write(port, value);
}

Uint8 drz80_readport16(Uint16 port)
{
	//printf("Read port %d\n",port);
    return z80_port_read(port);
}


void drz80_irq_callback(void)
{
        //if (mydrz80.Z80_IRQ ==0x2) 
	//mydrz80.Z80_IRQ = 0x00;
	//printf("Irq have been accepted %x %x\n",mydrz80.Z80_IRQ,mydrz80.Z80IF);
}


void cpu_z80_init(void)
{


        memset (&mydrz80, 0, sizeof(mydrz80));

        mydrz80.z80_rebasePC=drz80_rebasePC;
        mydrz80.z80_rebaseSP=drz80_rebaseSP;
        mydrz80.z80_read8   =drz80_read8;
        mydrz80.z80_read16  =drz80_read16;
        mydrz80.z80_write8  =drz80_write8;
        mydrz80.z80_write16 =drz80_write16;
        mydrz80.z80_in      =drz80_readport16; /*z80_in*/
        mydrz80.z80_out     =drz80_writeport16; /*z80_out*/
        //mydrz80.z80_irq_callback=drz80_irq_callback;
        mydrz80.Z80A = 0x00 <<24;
        mydrz80.Z80F = (1<<2); /* set ZFlag */
        mydrz80.Z80BC = 0x0000 <<16;
        mydrz80.Z80DE = 0x0000 <<16;
        mydrz80.Z80HL = 0x0000 <<16;
        mydrz80.Z80A2 = 0x00 <<24;
        mydrz80.Z80F2 = 1<<2;  /* set ZFlag */
        mydrz80.Z80BC2 = 0x0000 <<16;
        mydrz80.Z80DE2 = 0x0000 <<16;
        mydrz80.Z80HL2 = 0x0000 <<16;
        mydrz80.Z80IX = 0xFFFF;// <<16;
        mydrz80.Z80IY = 0xFFFF;// <<16;
        mydrz80.Z80I = 0x00;
        mydrz80.Z80IM = 0x01;
        mydrz80.Z80_IRQ = 0x00;
        mydrz80.Z80IF = 0x00;
        mydrz80.Z80PC=mydrz80.z80_rebasePC(0);
        mydrz80.Z80SP=mydrz80.z80_rebaseSP(0xffff);/*0xf000;*/



/* bank initalisation */
	z80map1 = shared_data->sm1 + 0x8000;
	z80map2 = shared_data->sm1 + 0xc000;
	z80map3 = shared_data->sm1 + 0xe000;
	z80map4 = shared_data->sm1 + 0xf000;


	
	z80_bank[0]=0x8000;
	z80_bank[1]=0xc000;
	z80_bank[2]=0xe000;
	z80_bank[3]=0xf000;

	memcpy(drz80mem, shared_data->sm1, 0xf800);

	//z80_init_save_state();
}
void cpu_z80_run(int nbcycle)
{
	DrZ80Run(&mydrz80, nbcycle);
}

void cpu_z80_nmi(void)
{
	mydrz80.Z80_IRQ |= 0x02;
}
void cpu_z80_raise_irq(int l)
{
	mydrz80.Z80_IRQ |= 0x1;
}
void cpu_z80_lower_irq(void)
{
	mydrz80.Z80_IRQ &= ~0x1;
}
