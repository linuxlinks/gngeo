#include "SDL.h"
#include <stdlib.h>
#include "gui.h"
#include "pixmap.h"
#include "widget.h"

static struct WIDGET* widget_at_pos(struct WIDGET* self,int mouse_x,int mouse_y) {
    return NULL; /* pixmap never get focus */
}

static void drw_pixmap(struct WIDGET* self,SDL_Surface *desk) {
    GPIX *p=GUI_GPIX(self);
    if ((!WID_IS_MAPPED(self)) || p->pix==NULL) return;
    //SDL_BlitSurface(p->pix,NULL,desk,&self->rect);
    /* TODO: use a better aproch */
    if (self->rect.w==p->pix->w && self->rect.h==p->pix->h)
	SDL_BlitSurface(p->pix,NULL,desk,&self->rect);
    else
	SDL_SoftStretch(p->pix,NULL,desk,&self->rect);
}

GPIX *gui_gpix_create(GPIX *p,Sint16 x,Sint16 y,Uint16 w,Uint16 h) {
    GPIX *t;
    if (p)
	t=p;
    else
	t=malloc(sizeof(GPIX));
    gui_widget_create(&t->wid,x,y,w,h);
    t->pix=NULL;
    GUI_WIDGET(t)->draw=drw_pixmap;
    GUI_WIDGET(t)->widget_at_pos=widget_at_pos;
    return t;
}

void gui_gpix_set_pixmap(GPIX *self,SDL_Surface *p) {
    self->pix=p;
}
