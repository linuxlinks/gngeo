#ifndef _PROGRESS_H_
#define _PROGRESS_H_

#include "SDL.h"
#include "gui.h"
#include "widget.h"

typedef struct PROGRESSBAR {
    WIDGET wid;
    Uint32 size,val;
}PROGRESSBAR;

PROGRESSBAR *gui_progressbar_create(PROGRESSBAR *p,Sint16 x,Sint16 y,Uint16 w,Uint16 h);
void gui_progressbar_set_size(PROGRESSBAR *self,Uint32 size);
void gui_progressbar_set_val(PROGRESSBAR *self,Uint32 val);


#endif
