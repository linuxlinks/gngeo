#include "SDL.h"
#include <stdlib.h>
#include "gui.h"
#include "widget.h"
#include "progress.h"
#include "style.h"

static void progressbar_draw(struct WIDGET* self,SDL_Surface *desk) {
    PROGRESSBAR *p=(PROGRESSBAR *)self;
    if (!WID_IS_MAPPED(self)) return;
    style_drw_progress(desk,&self->rect,p->size,p->val);
}

PROGRESSBAR *gui_progressbar_create(PROGRESSBAR *p,Sint16 x,Sint16 y,Uint16 w,Uint16 h) {
    PROGRESSBAR *t;
    if (p)
	t=p;
    else
	t=malloc(sizeof(PROGRESSBAR));

    gui_widget_create(&t->wid,x,y,w,h);
    t->size=100;
    t->val=0;
    GUI_WIDGET(t)->draw=progressbar_draw;
    return t;
}

void gui_progressbar_set_size(PROGRESSBAR *self,Uint32 size) {
    if (size==0)
	size=1;
    self->size=size;
}

void gui_progressbar_set_val(PROGRESSBAR *self,Uint32 val) {
    self->val=val;
    gui_redraw(gui_get_root((WIDGET*)self),SDL_TRUE);
}
