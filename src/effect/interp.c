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

#include "SDL.h"
#include "../screen.h"

#define INTERP_Y_LIMIT (0x30 * 4)
#define INTERP_U_LIMIT (0x07 * 4)
#define INTERP_V_LIMIT (0x06 * 8)

int interp_16_diff(Uint16 p1, Uint16 p2)
{
        int r, g, b;
        int y, u, v;

        if (p1 == p2)
                return 0;

	b = (int)((p1 & 0x1F) - (p2 & 0x1F)) << 3;
	g = (int)((p1 & 0x7E0) - (p2 & 0x7E0)) >> 3;
	r = (int)((p1 & 0xF800) - (p2 & 0xF800)) >> 8;
        
	y = r + g + b;

        if (y < -INTERP_Y_LIMIT || y > INTERP_Y_LIMIT)
                return 1;

        u = r - b;

        if (u < -INTERP_U_LIMIT || u > INTERP_U_LIMIT)
                return 1;

        v = -r + 2*g - b;

        if (v < -INTERP_V_LIMIT || v > INTERP_V_LIMIT)
                return 1;

        return 0;
}

