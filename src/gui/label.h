#ifndef _LABEL_H_
#define _LABEL_H_

#include "gui.h"
#include "widget.h"

typedef struct LABEL {
    WIDGET widget;
    char *label;
} LABEL;

#define GUI_LABEL(a) ((LABEL *)a)

LABEL *gui_label_create(LABEL *l,Sint16 x,Sint16 y,Uint16 w,Uint16 h,char *name);
void gui_label_change(LABEL *self,char *name);

#endif
