#ifndef _DRAW_H_
#define _DRAW_H_

#include "SDL.h"

#define COL32_TO_16(col) ((((col&0xff0000)>>19)<<11)|(((col&0xFF00)>>10)<<5)|((col&0xFF)>>3))

#define SHADOW_IN 0
#define SHADOW_OUT 1
#define SHADOW_ETCHED_IN 2
#define SHADOW_ETCHED_OUT 3
#define SHADOW_NONE 4

void draw_dotted_border(SDL_Surface *s,SDL_Rect *r,Uint32 color);
void draw_dotted_v_line(SDL_Surface *s,int x,int y,int l,Uint32 color);
void draw_dotted_h_line(SDL_Surface *s,int x,int y,int l,Uint32 color);
void draw_v_line(SDL_Surface *s,int x,int y,int l,Uint32 color);
void draw_h_line(SDL_Surface *s,int x,int y,int l,Uint32 color);
void draw_border(SDL_Surface *s,int type,SDL_Rect *r);
void draw_border_color(SDL_Surface *s,int type,SDL_Rect *r,Uint32 col);
void draw_border_label(SDL_Surface *s,SDL_Surface *font,int type,
		       SDL_Rect *r,char *label);
void draw_string(SDL_Surface *s,SDL_Surface *font,int x,int y,char *string);
SDL_Surface *load_ck_bmp(char *data_path,char *name);
SDL_Surface *load_img(char *path,char *name);
SDL_Surface *load_img_rle(char *path,char *name);

#endif
