
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "SDL.h"
#include "../screen.h"
#include "../video.h"
#include "../gnutil.h"

//static int screenw,screenh;

void do_inner_doublex_i386(void *dst,void *src,int screenw,int screenh,int sx);

int
effect_scanline_init() 
{
/*
    screenw=screen->w>>2;
    screenh=screen->h;
*/
    return GN_TRUE;
}

void 
effect_scanline_update()
{
	Uint16 *src, *dst;
	Uint32 s, d;
	Uint8 h, w;	
	static int i=0;

	src = (Uint16 *)buffer->pixels + visible_area.x + (352 << 4);// LeftBorder + RowLength * UpperBorder
	dst = (Uint16 *)screen->pixels + yscreenpadding;
	//if (i) dst+=screen->pitch;	i=1-i;
	for(h = visible_area.h; h > 0; h--)
	{
		for(w = visible_area.w>>1; w > 0; w--)
		{		
			s = *(Uint32 *)src;
#ifdef WORDS_BIGENDIAN
			d = (s & 0xFFFF0000) + ((s & 0xFFFF0000)>>16);
			*(Uint32 *)(dst) = d;			
				
			d = (s & 0x0000FFFF) + ((s & 0x0000FFFF)<<16);
			*(Uint32 *)(dst+2) = d;		
#else
			d = (s & 0xFFFF0000) + ((s & 0xFFFF0000)>>16);
			*(Uint32 *)(dst+2) = d;			
				
			d = (s & 0x0000FFFF) + ((s & 0x0000FFFF)<<16);
			*(Uint32 *)(dst) = d;			
#endif
			
			dst += 4;
			src += 2;
		}
		src += (visible_area.x<<1);		
		dst += (visible_area.w<<1);
	}
}

void 
effect_scanline50_update()
{
	Uint16 *src, *dst;
	Uint32 s, d;
	Uint8 h, w;
	
	src = (Uint16 *)buffer->pixels + visible_area.x + (352 << 4);// LeftBorder + RowLength * UpperBorder
	dst = (Uint16 *)screen->pixels + yscreenpadding;

	
	for(h = visible_area.h; h > 0; h--)
	{
		for(w = visible_area.w>>1; w > 0; w--)
		{		
			s = *(Uint32 *)src;
#ifdef WORDS_BIGENDIAN
			d = (s & 0xFFFF0000) + ((s & 0xFFFF0000)>>16);
			*(Uint32 *)(dst) = d;
			*(Uint32 *)(dst+(visible_area.w<<1)) = (d & 0xf7def7de) >> 1;
				
			d = (s & 0x0000FFFF) + ((s & 0x0000FFFF)<<16);
			*(Uint32 *)(dst+2) = d;
			*(Uint32 *)(dst+(visible_area.w<<1)+2) = (d & 0xf7def7de) >> 1;
#else				
			d = (s & 0xFFFF0000) + ((s & 0xFFFF0000)>>16);
			*(Uint32 *)(dst+2) = d;
			*(Uint32 *)(dst+(visible_area.w<<1)+2) = (d & 0xf7def7de) >> 1;
				
			d = (s & 0x0000FFFF) + ((s & 0x0000FFFF)<<16);
			*(Uint32 *)(dst) = d;
			*(Uint32 *)(dst+(visible_area.w<<1)) = (d & 0xf7def7de) >> 1;			
#endif			
			dst += 4;
			src += 2;
		}
		src += (visible_area.x<<1);		
		dst += (visible_area.w<<1);
	}
}

void effect_doublex_update()
{
	Uint16 *src, *dst;
	Uint32 s, d;
	Uint8 h, w;	
	
	src = (Uint16 *)buffer->pixels + visible_area.x + (352 << 4);// LeftBorder + RowLength * UpperBorder
	dst = (Uint16 *)screen->pixels + (yscreenpadding>>1);

#ifdef I386_ASM
	do_inner_doublex_i386(dst,src,visible_area.w>>1,visible_area.h,visible_area.x);
#else

	for(h = visible_area.h; h > 0; h--)
	{
		for(w = visible_area.w>>1; w > 0; w--)
		{		
			s = *(Uint32 *)src;
#ifdef WORDS_BIGENDIAN
			d = (s & 0xFFFF0000) + ((s & 0xFFFF0000)>>16);
			*(Uint32 *)(dst) = d;			
				
			d = (s & 0x0000FFFF) + ((s & 0x0000FFFF)<<16);
			*(Uint32 *)(dst+2) = d;		
#else
			d = (s & 0xFFFF0000) + ((s & 0xFFFF0000)>>16);
			*(Uint32 *)(dst+2) = d;			
				
			d = (s & 0x0000FFFF) + ((s & 0x0000FFFF)<<16);
			*(Uint32 *)(dst) = d;			
#endif
			
			dst += 4;
			src += 2;
		}
		src += (visible_area.x<<1);		
		//dst += 608;
	}
#endif
}
