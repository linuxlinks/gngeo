#ifndef _COMBOL_H_
#define _COMBOL_H_

#include "SDL.h"
#include "gui.h"
#include "container.h"
#include "slist.h"
#include "textbox.h"
#include "../list.h"


typedef struct COMBO_LIST {
    CONTAINER cont;
    TEXTBOX *text;
    BUTTON *dbut;
    SLIST *slist;
    int (*change)(struct COMBO_LIST *self);
}COMBO_LIST;

#define GUI_COMBO_LIST(a) ((COMBO_LIST*)(a))

COMBO_LIST *gui_combol_create(COMBO_LIST *c,Sint16 x,Sint16 y,Uint16 w,Uint16 h,Uint16 sl_size);
void gui_combol_update(COMBO_LIST *c);
void gui_combol_set_selection(COMBO_LIST *c,Uint32 index);

#endif
