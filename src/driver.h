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

#ifndef _DRIVER_H_
#define _DRIVER_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "SDL.h"
#include "list.h"

/* Section definition */
enum {
    SEC_CPU=0,
    SEC_SFIX,
    SEC_SM1,
    SEC_SOUND1,
    SEC_SOUND2,
    SEC_GFX,
    SEC_MAX
};

typedef enum BANKSW_TYPE {
  BANKSW_NORMAL=0,
  BANKSW_KOF2003,
  BANKSW_SCRAMBLE,
  BANKSW_MAX
} BANKSW_TYPE;

/* Rom type definition */
typedef enum ROM_TYPE {
    MGD2 = 0,
    MVS,
    MVS_CMC42,
    MVS_CMC50,
    ROMTYPE_MAX
}ROM_TYPE;

/* Loading type */
typedef enum LD_TYPE {
    LD_NORM=0,
    LD_ALTERNATE,
}LD_TYPE;

typedef struct SECTION_ITEM {
    char *filename;
    Uint32 begin;
    Uint32 size;
    LD_TYPE type;
}SECTION_ITEM;

typedef struct SECTION {
    Uint32 size;
    LIST *item;
}SECTION;

typedef struct DRIVER {
    char *name;
    Uint8 special_bios;
    SECTION bios;
    char *longname;
    ROM_TYPE rom_type;
    SECTION section[SEC_MAX];

    /* crypted rom bankswitch support */
    BANKSW_TYPE banksw_type;
    Uint8 xor;
	Uint8 sfix_bank;
    Uint32 banksw_addr;
    Uint8  banksw_unscramble[6];
    Uint32 banksw_off[64];
//    struct DRIVER *next;
}DRIVER;

void dr_list_all(void);
SDL_bool dr_load_driver(char *filename);
SDL_bool dr_load_driver_dir(char *dirname);
void dr_list_all(void);
void dr_list_available(void);
DRIVER *dr_get_by_name(char *name);

LIST *driver_get_all(void);
DRIVER *get_driver_for_zip(char *zip);
char *get_zip_name(char *name);
#endif




