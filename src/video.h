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

#ifndef _VIDEO_H_
#define _VIDEO_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "SDL.h"


SDL_Surface *buffer, *screen, *sprbuf, *fps_buf, *scan, *fontbuf;
SDL_Surface *triplebuf[2];
/*
SDL_Overlay *overlay;
SDL_Rect ov_rect;
*/

#ifdef GP2X
#include "unzip.h"
typedef struct gfx_cache {
	Uint8 *data;  /* The cache */
	Uint32 size;  /* Tha allocated size of the cache */      
	Uint32 total_bank;  /* total number of rom bank */
	Uint8 **ptr/*[TOTAL_GFX_BANK]*/; /* ptr[i] Contain a pointer to cached data for bank i */
	int max_slot; /* Maximal numer of bank that can be cached (depend on cache size) */
	int slot_size;
	int *usage;   /* contain index to the banks in used order */
	unz_file_pos *z_pos/*[TOTAL_GFX_BANK]*/;
}GFX_CACHE;

extern GFX_CACHE gcache;
extern Uint32 *mem_bank_usage;
#endif

#define RASTER_LINES 261

unsigned int neogeo_frame_counter;
extern unsigned int neogeo_frame_counter_speed;

void init_video(void);
void debug_draw_tile(unsigned int tileno,int sx,int sy,int zx,int zy,
		     int color,int xflip,int yflip,unsigned char *bmp);
void draw_screen_scanline(int start_line, int end_line, int refresh);
void draw_screen(void);
// void show_cache(void);
void convert_all_char(unsigned char *Ptr, int Taille,
		      unsigned char *usage_ptr);
void convert_mgd2_tiles(unsigned char *buf, int len);
void convert_tile(int tileno);


#endif
