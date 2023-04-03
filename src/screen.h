
#ifndef _SCREEN_H_
#define _SCREEN_H_

#include "SDL.h"
#include "list.h"

typedef struct RGB2YUV
{
  Uint16 y;
  Uint8  u;
  Uint8  v;
  Uint32 yuy2;
}RGB2YUV;

extern RGB2YUV rgb2yuv[65536];

extern void init_rgb2yuv_table(void);
extern SDL_Surface *screen;
extern SDL_Surface *buffer, *sprbuf, *fps_buf, *scan, *fontbuf;
//SDL_Surface *triplebuf[2];
extern SDL_Rect visible_area;
extern int yscreenpadding;
extern Uint8 interpolation;
extern Uint8 nblitter;
extern Uint8 neffect;
extern Uint8 scale;
extern Uint8 fullscreen;
extern Uint8 get_effect_by_name(char *name);
extern Uint8 get_blitter_by_name(char *name);
extern void print_blitter_list(void);
extern void print_effect_list(void);
//void screen_change_blitter_and_effect(char *bname,char *ename);
extern LIST* create_effect_list(void);
extern LIST* create_blitter_list(void);
extern int screen_init();
extern int screen_reinit(void);
extern int screen_resize(int w, int h);
extern void screen_update();
extern void screen_close();
extern void screen_fullscreen();
extern void sdl_set_title(char *name);
extern void init_sdl(void);

#endif
