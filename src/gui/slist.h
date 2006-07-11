#ifndef _SLIST_H_
#define _SLIST_H_

#include "SDL.h"
#include "../list.h"
#include "gui.h"
#include "container.h"
#include "slist.h"
#include "scrollbar.h"


typedef struct SLIST {
    CONTAINER cont;
    LIST *l;
    SCROLLBAR *vb,*hb;
    WIDGET *text;
    FRAME *f;
    Uint32 pos_x,pos_y;
    Uint32 nb_item;
    Uint32 selected;
    LIST *data_selected;
    char *(*get_label)(void *data); /* return a label for an item in the list */
    int (*sel_change)(struct SLIST *self);
    int (*sel_double_click)(struct SLIST* self);
}SLIST;

#define GUI_SLIST(a) ((SLIST*)(a))

SLIST *gui_slist_create(SLIST *sl,Sint16 x,Sint16 y,Uint16 w,Uint16 h);
void gui_slist_update(SLIST *sl);
void gui_slist_set_selection(SLIST *sl,Uint32 index);

#endif
