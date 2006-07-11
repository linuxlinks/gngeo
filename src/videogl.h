#ifndef _VIDEOGL_H_
#define _VIDEOGL_H_

#ifdef FULL_GL
#include "SDL.h"
#include "SDL_opengl.h"

typedef struct FGL_TILE {
    Uint32 no;
    int x;
    float y;
    int zx;
    float zy;
    SDL_bool flipx,flipy;
    Uint16 pal;
}FGL_TILE;

typedef struct FGL_STRIP {
    int x,y;
    int zx,zy;
    int nb_tile;
    FGL_TILE tiles[0x3f];
}FGL_STRIP;

FGL_STRIP strips[0x180];

SDL_bool fgl_init_video(void);
void fgl_drawscreen(void);

#endif
#endif
