#include <stdlib.h>
#include "SDL.h"
#include "gui.h"


static void frame_draw(WIDGET *self,SDL_Surface *desk) {
    FRAME *f=(FRAME *)self;
//    SDL_Rect wrect={self->x,self->y,self->w,self->h};

    if (!WID_IS_MAPPED(self)) return;
/*
    if (f->label)
	draw_border_label(desk,gui_font,f->type,&wrect,f->label);
    else
	draw_border(desk,f->type,&wrect);
*/
    style_drw_frame(desk,&self->rect,f->type,f->label);
	
    container_draw_son(self,desk);
}

FRAME *gui_frame_create(FRAME *f,Sint16 x,Sint16 y,Uint16 w,Uint16 h,int type,char *label) {
    FRAME *t;
    if (f)
	t=f;
    else
	t=malloc(sizeof(FRAME));

    gui_container_create(&t->cont,x,y,w,h);
    t->type=type;
    t->label=label;
    GUI_WIDGET(t)->draw=frame_draw;
    return t;
}
