
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "SDL.h"
#include "../emu.h"
#include "../screen.h"
#include "../video.h"
#include "../effect.h"
#include "../conf.h"
#ifdef GP2X
#include "../gp2x.h"

#define SC_STATUS     0x1802>>1
#define SC_DISP_FIELD (1<<8)

#endif
/*
static SDL_Rect buf_rect	 =	{16, 16, 304, 224};
*/
static SDL_Rect screen_rect =	{ 0,  0, 304, 224};

static SDL_Surface *offscreen;

SDL_bool
blitter_soft_init()
{
	Uint32 width = visible_area.w;
	Uint32 height = visible_area.h;
#ifdef GP2X
	int hw_surface=SDL_HWSURFACE/*|SDL_FULLSCREEN|SDL_DOUBLEBUF*/;
	// TVTEST
	//int hw_surface=SDL_SWSURFACE;
#else
	int hw_surface=(CF_BOOL(cf_get_item_by_name("hwsurface"))?SDL_HWSURFACE:SDL_SWSURFACE);
#endif
	//int screen_size=CF_BOOL(cf_get_item_by_name("screen320"));
	Uint32 sdl_flags = hw_surface|(fullscreen?SDL_FULLSCREEN:0);


	screen_rect.w=visible_area.w;
	screen_rect.h=visible_area.h;

		
	if (scale == 1) {
	    width *=effect[neffect].x_ratio;
	    height*=effect[neffect].y_ratio;
	} else {
	    if (scale > 3) scale=3;
	    width *=scale;
	    height *=scale;
	}
	
#ifdef GP2X
	//screen = SDL_SetVideoMode(width, height, 16, sdl_flags);

	screen = SDL_SetVideoMode(320, 240, 16, sdl_flags);
	
	
	//screen = SDL_SetVideoMode(304, 224, 16, sdl_flags);
	//gp2x_video_RGB_setscaling(320, 240);

	if (width!=320) {
		screen_rect.x=8;
	}
	screen_rect.y=8;

#else		
	screen = SDL_SetVideoMode(width, height, 16, sdl_flags);
#endif
	if (!screen) return SDL_FALSE;
	//offscreen = SDL_CreateRGBSurface(SDL_HWSURFACE, 304, 224, 16, 0xF800, 0x7E0, 0x1F, 0);

	return SDL_TRUE;
}

void 
update_double()
{
	Uint16 *src, *dst;
	Uint32 s, d;
	Uint8 w, h;
	
	src = (Uint16 *)buffer->pixels + visible_area.x + (buffer->w << 4);// LeftBorder + RowLength * UpperBorder
	dst = (Uint16 *)screen->pixels;
	
	for(h = visible_area.h; h > 0; h--)
	{
		for(w = visible_area.w>>1; w > 0; w--)
		{		
			s = *(Uint32 *)src;
#ifdef WORDS_BIGENDIAN
			d = (s & 0xFFFF0000) + ((s & 0xFFFF0000)>>16);
			*(Uint32 *)(dst) = d;
			*(Uint32 *)(dst+(visible_area.w<<1)) = d;
				
			d = (s & 0x0000FFFF) + ((s & 0x0000FFFF)<<16);
			*(Uint32 *)(dst+2) = d;
			*(Uint32 *)(dst+(visible_area.w<<1)+2) = d;
#else				
			d = (s & 0xFFFF0000) + ((s & 0xFFFF0000)>>16);
			*(Uint32 *)(dst+2) = d;
			*(Uint32 *)(dst+(visible_area.w<<1)+2) = d;
				
			d = (s & 0x0000FFFF) + ((s & 0x0000FFFF)<<16);
			*(Uint32 *)(dst) = d;
			*(Uint32 *)(dst+(visible_area.w<<1)) = d;
#endif			
			dst += 4;
			src += 2;
		}
		src += (visible_area.x<<1);		
		dst += (visible_area.w<<1);
	}
}

void 
update_triple()
{
	Uint16 *src, *dst;
	Uint32 s, d;
	Uint8 w, h;
	
	src = (Uint16 *)buffer->pixels + visible_area.x + (buffer->w << 4);// LeftBorder + RowLength * UpperBorder
	dst = (Uint16 *)screen->pixels;
	
	for(h = visible_area.h; h > 0; h--)
	{
		for(w = visible_area.w>>1; w > 0; w--)
		{		
			s = *(Uint32 *)src;
#ifdef WORDS_BIGENDIAN
			d = (s & 0xFFFF0000) + ((s & 0xFFFF0000)>>16);
			*(Uint32 *)(dst) = d;
			*(Uint32 *)(dst+(visible_area.w*3)) = d;
			*(Uint32 *)(dst+(visible_area.w*6)) = d;
				
			*(Uint32 *)(dst+2) = s;
			*(Uint32 *)(dst+(visible_area.w*3)+2) = s;
			*(Uint32 *)(dst+(visible_area.w*6)+2) = s;

			d = (s & 0x0000FFFF) + ((s & 0x0000FFFF)<<16);
			*(Uint32 *)(dst+4) = d;
			*(Uint32 *)(dst+(visible_area.w*3)+4) = d;
			*(Uint32 *)(dst+(visible_area.w*6)+4) = d;

#else				
			d = (s & 0xFFFF0000) + ((s & 0xFFFF0000)>>16);
			*(Uint32 *)(dst+4) = d;
			*(Uint32 *)(dst+(visible_area.w*3)+4) = d;
			*(Uint32 *)(dst+(visible_area.w*6)+4) = d;

			*(Uint32 *)(dst+2) = s;
			*(Uint32 *)(dst+(visible_area.w*3)+2) = s;
			*(Uint32 *)(dst+(visible_area.w*6)+2) = s;

			d = (s & 0x0000FFFF) + ((s & 0x0000FFFF)<<16);
			*(Uint32 *)(dst) = d;
			*(Uint32 *)(dst+(visible_area.w*3)) = d;
			*(Uint32 *)(dst+(visible_area.w*6)) = d;
#endif			
			dst += 6;
			src += 2;
		}
		src += (visible_area.x<<1);		
		dst += (visible_area.w*6);
	}
}

void
blitter_soft_update()
{
    int i;
  if (neffect == 0) {
      switch (scale) {
      case 2: update_double(); break;
      case 3: update_triple(); break;
      default:
	  SDL_BlitSurface(buffer, &visible_area, screen, &screen_rect);
	  break;
    }

  }

#ifdef GP2X
  SDL_Flip(screen);
#else
  SDL_UpdateRect(screen, 0, 0, 0, 0);
#endif
 
}

void
blitter_soft_close()
{
    
}
	
void
blitter_soft_fullscreen() {
  SDL_WM_ToggleFullScreen(screen);
}
	
