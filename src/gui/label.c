#include "stdlib.h"
#include "SDL.h"
#include "label.h"
#include "style.h"

static struct WIDGET* widget_at_pos(struct WIDGET* self,int mouse_x,int mouse_y) {
    return NULL; /* label never get focus */
}


void label_draw(struct WIDGET* self,SDL_Surface *desk) {
    //draw_string(desk,gui_font,self->x,self->y,GUI_LABEL(self)->label);
    if (!WID_IS_MAPPED(self)) return;
    style_drw_label(desk,&self->rect,GUI_LABEL(self)->label);
}

LABEL *gui_label_create(LABEL *l,Sint16 x,Sint16 y,Uint16 w,Uint16 h,char *name) {
    LABEL *t;
    if (l)
	t=l;
    else
	t=malloc(sizeof(LABEL));
    gui_widget_create(&t->widget,x,y,w,h);
    //printf("label %p %p\n",t,t->widget.move);
    t->label=name;
    GUI_WIDGET(t)->widget_at_pos=widget_at_pos;
    GUI_WIDGET(t)->draw=label_draw;
    return t;
}
void gui_label_change(LABEL *self,char *name) {
    self->label=name;
}
