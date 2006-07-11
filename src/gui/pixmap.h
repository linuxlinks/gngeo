#ifndef _PIXMAP_H_
#define _PIXMAP_H_

#include "SDL.h"
#include "gui.h"
#include "widget.h"

typedef struct GPIX {
    WIDGET wid;
    SDL_Surface *pix;
}GPIX;

#define GUI_GPIX(a) ((GPIX*)(a))

GPIX *gui_gpix_create(GPIX *p,Sint16 x,Sint16 y,Uint16 w,Uint16 h);
void gui_gpix_set_pixmap(GPIX *self,SDL_Surface *p);

#endif
