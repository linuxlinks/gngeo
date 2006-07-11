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
/*
SDL_Overlay *overlay;
SDL_Rect ov_rect;
*/


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
