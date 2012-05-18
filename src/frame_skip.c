/*  gngeo, a neogeo emulator
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

#include <unistd.h>
#include <sys/time.h>
#include <math.h>
#include "SDL.h"
#include "frame_skip.h"
#include "messages.h"
#include "emu.h"
#include "conf.h"
#include "gnutil.h"

#ifndef uclock_t
#define uclock_t Uint32
#endif

#define TICKS_PER_SEC 1000000UL
//#define CPU_FPS 60
//static int CPU_FPS=60;
static uclock_t F;

#define MAX_FRAMESKIP 10

static char init_frame_skip = 1;
char skip_next_frame = 0;
#if defined(HAVE_GETTIMEOFDAY) && !defined(WII)
static int CPU_FPS = 60;
static struct timeval init_tv = { 0, 0 };
#else
/* Looks like SDL_GetTicks is not as precise... */
static int CPU_FPS=61;
static Uint32 init_tv=0;
#endif
uclock_t bench;

#if defined(HAVE_GETTIMEOFDAY) && !defined(WII)
uclock_t get_ticks(void) {
	struct timeval tv;

	gettimeofday(&tv, 0);
	if (init_tv.tv_sec == 0)
		init_tv = tv;
	return (tv.tv_sec - init_tv.tv_sec) * TICKS_PER_SEC + tv.tv_usec
			- init_tv.tv_usec;

}
#else
Uint32 get_ticks(void)
{
	Uint32 tv;
	if (init_tv==0)
	init_tv=SDL_GetTicks();
	return (SDL_GetTicks()-init_tv)*1000;
}
#endif

void reset_frame_skip(void) {
#if defined(HAVE_GETTIMEOFDAY) && !defined(WII)
	init_tv.tv_usec = 0;
	init_tv.tv_sec = 0;
#else
	init_tv=0;
#endif
	skip_next_frame = 0;
	init_frame_skip = 1;
	if (conf.pal)
		CPU_FPS = 50;
	F = (uclock_t) ((double) TICKS_PER_SEC / CPU_FPS);
}

int frame_skip(int init) {
	static int f2skip;
	static uclock_t sec = 0;
	static uclock_t rfd;
	static uclock_t target;
	static int nbFrame = 0;
	static unsigned int nbFrame_moy = 0;
	static int nbFrame_min = 1000;
	static int nbFrame_max = 0;
	static int skpFrm = 0;
	static int count = 0;
	static int moy = 60;

	if (init_frame_skip) {
		init_frame_skip = 0;
		target = get_ticks();
		bench = (CF_BOOL(cf_get_item_by_name("bench")) ? 1/*get_ticks()*/: 0);

		nbFrame = 0;
		//f2skip=0;
		//skpFrm=0;
		sec = 0;
		return 0;
	}

	target += F;
	if (f2skip > 0) {
		f2skip--;
		skpFrm++;
		return GN_TRUE;
	} else
		skpFrm = 0;
//	printf("%d %d\n",conf.autoframeskip,conf.show_fps);

	rfd = get_ticks();

	if (conf.autoframeskip) {
		if (rfd < target && f2skip == 0)
			while (get_ticks() < target) {
#ifndef WIN32
				if (conf.sleep_idle) {
					usleep(5);
				}
#endif
			}
		else {
			f2skip = (rfd - target) / (double) F;
			if (f2skip > MAX_FRAMESKIP) {
				f2skip = MAX_FRAMESKIP;
				reset_frame_skip();
			}
			// printf("Skip %d frame(s) %lu %lu\n",f2skip,target,rfd);
		}
	}

	nbFrame++;
	nbFrame_moy++;
	/*
	 if(bench && totalFrame++>=5000) {
	 printf("average fps=%f \n",(TICKS_PER_SEC*5000.0/(get_ticks()-bench)));
	 exit(0);
	 }
	 */
	if (conf.show_fps) {
		if (get_ticks() - sec >= TICKS_PER_SEC) {
			//printf("%d\n",nbFrame);
			if (bench) {

				if (nbFrame_min > nbFrame)
					nbFrame_min = nbFrame;
				if (nbFrame_max < nbFrame)
					nbFrame_max = nbFrame;
				count++;
				moy = nbFrame_moy / (float) count;

				if (count == 30)
					count = 0;
				sprintf(fps_str, "%d %d %d %d\n", nbFrame - 1, nbFrame_min - 1,
						nbFrame_max - 1, moy - 1);
			} else {
				sprintf(fps_str, "%2d", nbFrame - 1);
			}

			nbFrame = 0;
			sec = get_ticks();
		}
	}
	return GN_FALSE;
}
