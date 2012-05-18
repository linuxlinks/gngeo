/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 2003 Andrea Mazzoleni
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * In addition, as a special exception, Andrea Mazzoleni
 * gives permission to link the code of this program with
 * the MAME library (or with modified versions of MAME that use the
 * same license as MAME), and distribute linked combinations including
 * the two.  You must obey the GNU General Public License in all
 * respects for all of the code used other than MAME.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "SDL.h"
#include "../screen.h"
#include "../video.h"
#include "../gnutil.h"

#include "lq2x.h"


/***************************************************************************/
/* LQ2x C implementation */

/*
 * This effect is derived from the hq2x effect made by Maxim Stepin
 */
static Uint16 *dst0, *dst1, *src0, *src1, *src2;
static Uint16 height;

int effect_lq2x_init(void)
{
    return GN_TRUE;
}

void lq2x_16_def(Uint16 *dst0, Uint16 *dst1, Uint16 *src0, Uint16 *src1, Uint16 *src2, unsigned count)
{
	unsigned i;

	for(i=0;i<count;++i) {
		unsigned char mask;

		Uint16 c[9];

		c[1] = src0[0];
		c[4] = src1[0];
		c[7] = src2[0];

		if (i>0) {
			c[0] = src0[-1];
			c[3] = src1[-1];
			c[6] = src2[-1];
		} else {
			c[0] = c[1];
			c[3] = c[4];
			c[6] = c[7];
		}

		if (i<count-1) {
			c[2] = src0[1];
			c[5] = src1[1];
			c[8] = src2[1];
		} else {
			c[2] = c[1];
			c[5] = c[4];
			c[8] = c[7];
		}

		mask = 0;

		if (c[0] != c[4])
			mask |= 1 << 0;
		if (c[1] != c[4])
			mask |= 1 << 1;
		if (c[2] != c[4])
			mask |= 1 << 2;
		if (c[3] != c[4])
			mask |= 1 << 3;
		if (c[5] != c[4])
			mask |= 1 << 4;
		if (c[6] != c[4])
			mask |= 1 << 5;
		if (c[7] != c[4])
			mask |= 1 << 6;
		if (c[8] != c[4])
			mask |= 1 << 7;

#define P0 dst0[0]
#define P1 dst0[1]
#define P2 dst1[0]
#define P3 dst1[1]
#define MUR (c[1] != c[5])
#define MDR (c[5] != c[7])
#define MDL (c[7] != c[3])
#define MUL (c[3] != c[1])
#define IC(p0) c[p0]
#define I11(p0,p1) interp_16_11(c[p0], c[p1])
#define I211(p0,p1,p2) interp_16_211(c[p0], c[p1], c[p2])
#define I31(p0,p1) interp_16_31(c[p0], c[p1])
#define I332(p0,p1,p2) interp_16_332(c[p0], c[p1], c[p2])
#define I431(p0,p1,p2) interp_16_431(c[p0], c[p1], c[p2])
#define I521(p0,p1,p2) interp_16_521(c[p0], c[p1], c[p2])
#define I53(p0,p1) interp_16_53(c[p0], c[p1])
#define I611(p0,p1,p2) interp_16_611(c[p0], c[p1], c[p2])
#define I71(p0,p1) interp_16_71(c[p0], c[p1])
#define I772(p0,p1,p2) interp_16_772(c[p0], c[p1], c[p2])
#define I97(p0,p1) interp_16_97(c[p0], c[p1])
#define I1411(p0,p1,p2) interp_16_1411(c[p0], c[p1], c[p2])
#define I151(p0,p1) interp_16_151(c[p0], c[p1])

		switch (mask) {
		#include "lq2x.dat"
		}

#undef P0
#undef P1
#undef P2
#undef P3
#undef MUR
#undef MDR
#undef MDL
#undef MUL
#undef IC
#undef I11
#undef I211
#undef I31
#undef I332
#undef I431
#undef I521
#undef I53
#undef I611
#undef I71
#undef I772
#undef I97
#undef I1411
#undef I151

		src0 += 1;
		src1 += 1;
		src2 += 1;
		dst0 += 2;
		dst1 += 2;
	}
}

void effect_lq2x_update(void)
{
    height = visible_area.h;
	
    dst0 = (Uint16 *)screen->pixels + yscreenpadding;
    dst1 = (Uint16 *)dst0 + (visible_area.w<<1);
    
    src1 = (Uint16 *)buffer->pixels + 352 * visible_area.y + visible_area.x;
    src0 = (Uint16 *)src1 - 352;   
    src2 = (Uint16 *)src1 + 352;
    while(height--)
    {
	lq2x_16_def(dst0, dst1, src0, src1, src2, visible_area.w);
	
	dst0 += (visible_area.w<<2);
	dst1 += (visible_area.w<<2);
	
	src0 += 352;
	src1 += 352;
	src2 += 352;	
    }
}
