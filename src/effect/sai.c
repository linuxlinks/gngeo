
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef I386_ASM
#include <stdlib.h>
#include "SDL.h"
#include "../screen.h"
#include "../video.h"

extern void Init_2xSaIMMX(Uint32 bitformat);
extern void _2xSaILine(Uint8 *srcPtr, Uint8 *deltaPtr, Uint32 srcPitch, Uint32 width,
                       Uint32 dstOffset, Uint32 dstPitch, Uint16 dstSegment);

extern void _2xSaISuper2xSaILine(Uint8 *srcPtr, Uint8 *deltaPtr, Uint32 srcPitch, Uint32 width,
                       Uint32 dstOffset, Uint32 dstPitch, Uint16 dstSegment);

extern void _2xSaISuperEagleLine(Uint8 *srcPtr, Uint8 *deltaPtr, Uint32 srcPitch, Uint32 width,
                       Uint32 dstOffset, Uint32 dstPitch, Uint16 dstSegment);

static Uint8 *deltaptr;
//static int screenw,screenh;

int
effect_sai_init()
{

	Init_2xSaIMMX(565);
	if (!deltaptr)
	    deltaptr = (Uint8*)malloc(352*256*2);
	printf("deltaptr=%p sai\n",deltaptr);
	/*
	screenw=screen->w>>1;
	screenh=screen->h>>1;
	*/
	
	
	return GN_TRUE;
}


void
effect_sai_update()
{	
	Uint8 *src = buffer->pixels + (352*visible_area.y+visible_area.x)*2;
	Uint8 *dst = screen->pixels + yscreenpadding;
	Uint8 *dptr = deltaptr;
		
	Uint8 height = visible_area.h;
	//printf("Update sai %p %p %p\n",src,dst,dptr);
	while(height--)
	{
	/* Params:
		1: Source pixmap
	   2: buffer(same size of source visible area)
		3: Src row bytes
		4: Src length(not in bytes)
		5: Destination source
		6: Destination row bytes
		7: ?� only use with djgpp.
	*/
		_2xSaILine(src, dptr, 352*2, visible_area.w, (Uint32)dst, visible_area.w<<2, 0);
		
		dst += (visible_area.w<<3);
		src += 352*2;
		dptr+= 352*4;
	}						
}

void
effect_supersai_update()
{	
	Uint8 *src = buffer->pixels + (352*visible_area.y+visible_area.x)*2;
	Uint8 *dst = screen->pixels;
	Uint8 *dptr = deltaptr;
		
	Uint8 height = visible_area.h;
	
	while(height--)
	{
	/* Params:
		1: Source pixmap
	   2: buffer(same size of source visible area)
		3: Src row bytes
		4: Src length(not in bytes)
		5: Destination source
		6: Destination row bytes
		7: ?� only use with djgpp.
	*/
		_2xSaISuper2xSaILine(src, dptr, 352*2, visible_area.w, (Uint32)dst, visible_area.w<<2, 0);
		
		dst += (visible_area.w<<3);
		src += 352*2;
		dptr+= 352*4;
	}						
}

void
effect_eagle_update()
{	
	Uint8 *src = buffer->pixels + (352*visible_area.y+visible_area.x)*2;
	Uint8 *dst = screen->pixels;
	Uint8 *dptr = deltaptr;
		
	Uint8 height = visible_area.h;			
	
	while(height--)
	{
	/* Params:
		1: Source pixmap
	   2: buffer(same size of source visible area)
		3: Src row bytes
		4: Src length(not in bytes)
		5: Destination source
		6: Destination row bytes
		7: ?� only use with djgpp.
	*/
		
		_2xSaISuperEagleLine(src, dptr, 352*2, visible_area.w, (Uint32)dst, visible_area.w<<2, 0);
		
		dst += (visible_area.w<<3);
		src += 352*2;
		dptr+= 352*4;
	}						
}
#endif
