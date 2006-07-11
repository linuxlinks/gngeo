#ifndef _FRAME_H_
#define _FRAME_H_

#include "SDL.h"
#include "gui.h"

typedef struct FRAME {
    CONTAINER cont;
    int type;
    char *label;
} FRAME;

#define GUI_FRAME(a) ((FRAME*)(a))

FRAME *gui_frame_create(FRAME *f,Sint16 x,Sint16 y,Uint16 w,Uint16 h,int type,char *label);

#endif
