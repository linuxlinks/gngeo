#ifndef _SCROLLBAR_H_
#define _SCROLLBAR_H_

#include "SDL.h"
#include "gui.h"

#define SB_H 10 /* Scrollbar size */

enum {
    SB_VERTICAL=0,
    SB_HORIZONTAL,
};

typedef struct SCROLLBAR {
    WIDGET widget;
    WIDGET *up,*down; /* the up and down (resp left/right) button */
    WIDGET *back; /* the back of the scrollbar */
    WIDGET *mobil; /* the mobil part of the scrollbar */
    Uint32 vsize;
    Uint16 s;
    Uint8 dir; /* the direction: SB_VERTICAL or SB_HORIZONTAL */
    Uint32 size,pos; /* 0<pos<size */
    Uint16 mobil_size,free_size;
    void (*change)(WIDGET* self,Uint32 val); /* this callback is call every time pos change */
/*
    Uint8 pressed_up,pressed_down;
    Uint8 pressed_mobil;
*/
} SCROLLBAR;

#define GUI_SCROLLBAR(a) ((SCROLLBAR *)(a))

SCROLLBAR *gui_scrollbar_create(SCROLLBAR *sb,Sint16 x,Sint16 y,Uint16 s,
				Uint8 dir,Uint32 size,Uint32 vsize,void (*change)(WIDGET *self,Uint32 val));

void gui_scrollbar_set_size(SCROLLBAR *sb,Uint32 size);

#endif
