
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "SDL.h"
#include "../screen.h"
#include "../video.h"
#include "../gnutil.h"

#include "scale2x.h"
#include "scale3x.h"

static Uint16 *dst0, *dst1, *dst2, *src0, *src1, *src2;
static Uint16 height;
static SDL_Surface *scale4xtmp;
//static int screenw,screenh;

int effect_scale2x_init(void)
{
    /*
    screenw=screen->w>>1;
    screenh=screen->h>>1;
    */
    return GN_TRUE;
}
int effect_scale3x_init(void)
{
    /*
    screenw=screen->w;
    screenh=screen->h;
    */
    return GN_TRUE;
}
int effect_scale4x_init(void)
{
    /*
    screenw=screen->w>>2;
    screenh=screen->h>>2;
    */
    if (!scale4xtmp)
	scale4xtmp=SDL_CreateRGBSurface(SDL_SWSURFACE,visible_area.w<<2, (visible_area.h<<2)+16, 16,
					0xF800, 0x7E0, 0x1F, 0);
    return GN_TRUE;
}
static inline void internal_scale2x_16_def(Uint16* dst0, Uint16* dst1, const Uint16* src0, 
					   const Uint16* src1, const Uint16* src2, unsigned count) 
{
    /* we don't need to care about of this special case since our src bitmap is larger than
       the dest */
#if 0
	/* first pixel */
    dst0[0] = src1[0];
    dst1[0] = src1[0];
    if (src1[1] == src0[0] && src2[0] != src0[0])
        dst0[1] =src0[0];
    else
        dst0[1] =src1[0];
    if (src1[1] == src2[0] && src0[0] != src2[0])
        dst1[1] =src2[0];
    else
        dst1[1] =src1[0];
    ++src0;
    ++src1;
    ++src2;
    dst0 += 2;
    dst1 += 2;

	/* central pixels */
    count -= 2;
#endif
    while (count) 
	{
            if (src1[-1] == src0[0] && src2[0] != src0[0] && src1[1] != src0[0])
                dst0[0] = src0[0];
            else
                dst0[0] = src1[0];
            if (src1[1] == src0[0] && src2[0] != src0[0] && src1[-1] != src0[0])
                dst0[1] =src0[0];
            else
                dst0[1] =src1[0];

            if (src1[-1] == src2[0] && src0[0] != src2[0] && src1[1] != src2[0])
                dst1[0] =src2[0];
            else
                dst1[0] =src1[0];
            if (src1[1] == src2[0] && src0[0] != src2[0] && src1[-1] != src2[0])
                dst1[1] =src2[0];
            else
                dst1[1] =src1[0];

            ++src0;
            ++src1;
            ++src2;
            dst0 += 2;
            dst1 += 2;
            --count;
	}
#if 0
	/* last pixel */
    if (src1[-1] == src0[0] && src2[0] != src0[0])
        dst0[0] =src0[0];
    else
        dst0[0] =src1[0];
    if (src1[-1] == src2[0] && src0[0] != src2[0])
        dst1[0] =src2[0];
    else
        dst1[0] =src1[0];
    dst0[1] =src1[0];
    dst1[1] =src1[0];
#endif
}

/* scanline 50% variant of scale2x */
static inline void internal_scale2x_16_def_50(Uint16* dst0, Uint16* dst1, const Uint16* src0, 
					      const Uint16* src1, const Uint16* src2, unsigned count) 
{
#if 0
	/* first pixel */
	dst0[0] = src1[0];
	dst1[0] = src1[0];
	if (src1[1] == src0[0] && src2[0] != src0[0])
		dst0[1] =src0[0];
	else
		dst0[1] =src1[0];
	if (src1[1] == src2[0] && src0[0] != src2[0])
		dst1[1] =src2[0];
	else
		dst1[1] =src1[0];
	++src0;
	++src1;
	++src2;
	dst0 += 2;
	dst1 += 2;

	/* central pixels */
	count -= 2;
#endif
	while (count) 
	{
		if (src1[-1] == src0[0] && src2[0] != src0[0] && src1[1] != src0[0])
			dst0[0] = src0[0];
		else
			dst0[0] = src1[0];
		if (src1[1] == src0[0] && src2[0] != src0[0] && src1[-1] != src0[0])
			dst0[1] =src0[0];
		else
			dst0[1] =src1[0];

		if (src1[-1] == src2[0] && src0[0] != src2[0] && src1[1] != src2[0]) 
			dst1[0] =(src2[0]&0xf7de)>>1;
		else 
		  dst1[0] =(src1[0]&0xf7de)>>1;
		
		if (src1[1] == src2[0] && src0[0] != src2[0] && src1[-1] != src2[0])
			dst1[1] =(src2[0]&0xf7de)>>1;
		else
			dst1[1] =(src1[0]&0xf7de)>>1;
		

		++src0;
		++src1;
		++src2;
		dst0 += 2;
		dst1 += 2;
		--count;
	}
#if 0
	/* last pixel */
	if (src1[-1] == src0[0] && src2[0] != src0[0])
		dst0[0] =src0[0];
	else
		dst0[0] =src1[0];
	if (src1[-1] == src2[0] && src0[0] != src2[0])
		dst1[0] =src2[0];
	else
		dst1[0] =src1[0];
	dst0[1] =src1[0];
	dst1[1] =src1[0];
#endif
}

/* scanline 75% variant of scale2x */
static inline void internal_scale2x_16_def_75(Uint16* dst0, Uint16* dst1, const Uint16* src0, 
					      const Uint16* src1, const Uint16* src2, unsigned count) 
{
#if 0
	/* first pixel */
	dst0[0] = src1[0];
	dst1[0] = src1[0];
	if (src1[1] == src0[0] && src2[0] != src0[0])
		dst0[1] =src0[0];
	else
		dst0[1] =src1[0];
	if (src1[1] == src2[0] && src0[0] != src2[0])
		dst1[1] =src2[0];
	else
		dst1[1] =src1[0];
	++src0;
	++src1;
	++src2;
	dst0 += 2;
	dst1 += 2;

	/* central pixels */
	count -= 2;
#endif
	while (count) 
	{
		if (src1[-1] == src0[0] && src2[0] != src0[0] && src1[1] != src0[0])
			dst0[0] = src0[0];
		else
			dst0[0] = src1[0];
		if (src1[1] == src0[0] && src2[0] != src0[0] && src1[-1] != src0[0])
			dst0[1] =src0[0];
		else
			dst0[1] =src1[0];

		if (src1[-1] == src2[0] && src0[0] != src2[0] && src1[1] != src2[0]) {
			dst1[0] =(src2[0]&0xf7de)>>1;
			dst1[0] =((src2[0]&0xf7de)>>1)+((dst1[0]&0xf7de)>>1);
		} else {
		  dst1[0] =(src1[0]&0xf7de)>>1;
		  dst1[0] =((src1[0]&0xf7de)>>1)+((dst1[0]&0xf7de)>>1);
		}
		if (src1[1] == src2[0] && src0[0] != src2[0] && src1[-1] != src2[0]) {
			dst1[1] =(src2[0]&0xf7de)>>1;
			dst1[1] =((src2[0]&0xf7de)>>1)+((dst1[1]&0xf7de)>>1);
		} else {
			dst1[1] =(src1[0]&0xf7de)>>1;
			dst1[1] =((src1[0]&0xf7de)>>1)+((dst1[1]&0xf7de)>>1);
		}

		++src0;
		++src1;
		++src2;
		dst0 += 2;
		dst1 += 2;
		--count;
	}
#if 0
	/* last pixel */
	if (src1[-1] == src0[0] && src2[0] != src0[0])
		dst0[0] =src0[0];
	else
		dst0[0] =src1[0];
	if (src1[-1] == src2[0] && src0[0] != src2[0])
		dst1[0] =src2[0];
	else
		dst1[0] =src1[0];
	dst0[1] =src1[0];
	dst1[1] =src1[0];
#endif
}

void effect_scale2x_update()
{
    height = visible_area.h;
	
    dst0 = (Uint16 *)screen->pixels + yscreenpadding;
    dst1 = (Uint16 *)dst0 + (visible_area.w<<1);
    
    src1 = (Uint16 *)buffer->pixels + 352 * visible_area.y + visible_area.x;
    src0 = (Uint16 *)src1 - 352;   
    src2 = (Uint16 *)src1 + 352;
    while(height--)
    {
#ifdef I386_ASM_DESACTIVE
	scale2x_16_mmx(dst0, dst1, src0, src1, src2, visible_area.w);
#else
	scale2x_16_def(dst0, dst1, src0, src1, src2, visible_area.w);
#endif
	
	dst0 += (visible_area.w<<2);
	dst1 += (visible_area.w<<2);
	
	src0 += 352;
	src1 += 352;
	src2 += 352;	
    }
}

void effect_scale4x_update()
{
    height =visible_area.h;
	
    dst0 = (Uint16 *)scale4xtmp->pixels;
    dst1 = (Uint16 *)dst0 + (visible_area.w<<1);
    
    src1 = (Uint16 *)buffer->pixels + 352 * visible_area.y + visible_area.x;
    src0 = (Uint16 *)src1 - 352;
    src2 = (Uint16 *)src1 + 352;
    while(height--)
    {
/* mmx cause strange effect ... */
#ifdef I386_ASM_DESACTIVE
	scale2x_16_mmx(dst0, dst1, src0, src1, src2, visible_area.w);
#else
	scale2x_16_def(dst0, dst1, src0, src1, src2, visible_area.w);
#endif
	
	dst0 += (visible_area.w<<2);
	dst1 += (visible_area.w<<2);
	
	src0 += 352;
	src1 += 352;
	src2 += 352;	
    }

    height = (visible_area.h<<1);
    dst0 = (Uint16 *)screen->pixels + yscreenpadding;
    dst1 = (Uint16 *)dst0 + (visible_area.w<<2);
    
    src1 = (Uint16 *)scale4xtmp->pixels + (visible_area.w<<1);
    src0 = (Uint16 *)src1 - (visible_area.w<<1);   
    src2 = (Uint16 *)src1 + (visible_area.w<<1);
    while(height--)
    {
#ifdef I386_ASM_DESACTIVE
	scale2x_16_mmx(dst0, dst1, src0, src1, src2, (visible_area.w<<1));
#else
	scale2x_16_def(dst0, dst1, src0, src1, src2, (visible_area.w<<1));
#endif
	
	dst0 += (visible_area.w<<3);
	dst1 += (visible_area.w<<3);
	
	src0 += (visible_area.w<<1);
	src1 += (visible_area.w<<1);
	src2 += (visible_area.w<<1);	
    }
}



void effect_scale3x_update()
{
    height = visible_area.h;
	
    dst0 = (Uint16 *)screen->pixels + yscreenpadding;
    dst1 = (Uint16 *)dst0 + visible_area.w*3;
    dst2 = (Uint16 *)dst1 + visible_area.w*3;

    src1 = (Uint16 *)buffer->pixels + 352 * visible_area.y + visible_area.x;
    src0 = (Uint16 *)src1 - 352;
    src2 = (Uint16 *)src1 + 352;
    while(height--)
    {

	scale3x_16_def(dst0, dst1, dst2, src0, src1, src2, visible_area.w);
	
	dst0 += (visible_area.w*9);
	dst1 += (visible_area.w*9);
	dst2 += (visible_area.w*9);

	src0 += 352;
	src1 += 352;
	src2 += 352;	
    }
}

void
old_effect_scale2x_update()
{
	height = 224;
	
	dst0 = (Uint16 *)screen->pixels + yscreenpadding;
	dst1 = (Uint16 *)dst0 + 608;
	
	src1 = (Uint16 *)buffer->pixels + 336 * 16 + 16;
	src0 = (Uint16 *)src1 - 336;   
	src2 = (Uint16 *)src1 + 336;
	
	while(height--)
	{
	  internal_scale2x_16_def(dst0, dst1, src0, src1, src2, 304);
		
		dst0 += 608*2;
		dst1 += 608*2;
		
		src0 += 336;
		src1 += 336;
		src2 += 336;		
	}
}

void
effect_scale2x50_update()
{
	height = visible_area.h;
	
	dst0 = (Uint16 *)screen->pixels + yscreenpadding;
	dst1 = (Uint16 *)dst0 + (visible_area.w<<1);
	
	src1 = (Uint16 *)buffer->pixels + 352 * visible_area.y + visible_area.x;
	src0 = (Uint16 *)src1 - 352;
	src2 = (Uint16 *)src1 + 352;
	
	while(height--)
	{
	  internal_scale2x_16_def_50(dst0, dst1, src0, src1, src2, visible_area.w);
		
		dst0 += (visible_area.w<<2);
		dst1 += (visible_area.w<<2);
		
		src0 += 352;
		src1 += 352;
		src2 += 352;		
	}
}

void
effect_scale2x75_update()
{
	height = visible_area.h;
	
	dst0 = (Uint16 *)screen->pixels + yscreenpadding;
	dst1 = (Uint16 *)dst0 + (visible_area.w<<1);
	
	src1 = (Uint16 *)buffer->pixels + 352 * visible_area.y + visible_area.x;
	src0 = (Uint16 *)src1 - 352;
	src2 = (Uint16 *)src1 + 352;
	
	while(height--)
	{
	  internal_scale2x_16_def_75(dst0, dst1, src0, src1, src2, visible_area.w);
		
		dst0 += (visible_area.w<<2);
		dst1 += (visible_area.w<<2);
		
		src0 += 352;
		src1 += 352;
		src2 += 352;		
	}
}
