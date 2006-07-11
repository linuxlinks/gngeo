#ifndef _TEXTBOX_H_
#define _TEXTBOX_H_

#include "SDL.h"
#include "gui.h"

typedef struct TEXTBOX {
    WIDGET widget;
    
    char *text;
    Uint32 max;

    Uint32 text_size;
    Uint32 cur_pos;

    Uint16 begin,end; /* drawing bound */
}TEXTBOX;

#define GUI_TEXTBOX(a) ((TEXTBOX *)(a))

TEXTBOX *gui_textbox_create(TEXTBOX *tx,Sint16 x,Sint16 y,Uint16 w,Uint16 h,Uint32 max);
char *gui_textbox_get_text(TEXTBOX *self);
void gui_textbox_set_text(TEXTBOX *self,char *text);

#endif
