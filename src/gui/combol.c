
#include "SDL.h"
#include <stdlib.h>
#include "combol.h"
#include "container.h"

static int show_list(BUTTON *self) {
    COMBO_LIST *c=GUI_COMBO_LIST(GUI_WIDGET(self)->father);
    /* first redraw all the gui, but without the cursor*/
    gui_redraw(gui_get_root((WIDGET*)self),SDL_FALSE);

    GUI_WIDGET(c->slist)->map(GUI_WIDGET(c->slist));
    gui_main_loop((WIDGET*)c->slist);
    GUI_WIDGET(c->slist)->unmap(GUI_WIDGET(c->slist));
    return TAKE_EVENT;
}

static int list_change(SLIST *sl) {
    COMBO_LIST *c=GUI_COMBO_LIST(GUI_WIDGET(sl)->father);
    if (sl->get_label && sl->data_selected)
	gui_textbox_set_text(c->text,sl->get_label(sl->data_selected->data));
    if (c->change) c->change(c);
    return QUIT_EVENT;
}

static void cl_map(WIDGET *self) {
    COMBO_LIST *c=(COMBO_LIST *)self;

    self->flags|=FLAG_MAPPED;

    GUI_WIDGET(c->dbut)->map(GUI_WIDGET(c->dbut));
    GUI_WIDGET(c->text)->map(GUI_WIDGET(c->text));

}

static void cl_unmap(WIDGET *self) {
    COMBO_LIST *c=(COMBO_LIST *)self;

    self->flags&=(~FLAG_MAPPED);

    GUI_WIDGET(c->dbut)->unmap(GUI_WIDGET(c->dbut));
    GUI_WIDGET(c->text)->unmap(GUI_WIDGET(c->text));
}

static void dbut_draw(WIDGET *self,SDL_Surface *desk) {
    style_drw_sbdown(desk,&self->rect,SB_VERTICAL,self->flags);
}

COMBO_LIST *gui_combol_create(COMBO_LIST *c,Sint16 x,Sint16 y,Uint16 w,Uint16 h,Uint16 sl_size) {
    COMBO_LIST *t;
    if (c)
	t=c;
    else
	t=malloc(sizeof(COMBO_LIST));

    gui_container_create(&t->cont,x,y,w,h);
    t->change=NULL;
    t->text=gui_textbox_create(NULL,0,0,w-SB_H-2,h,255);
    t->dbut=gui_button_create(NULL,0+w-SB_H,0,SB_H,SB_H);
    t->dbut->activate=show_list;
    GUI_WIDGET(t->dbut)->draw=dbut_draw;
    t->slist=gui_slist_create(NULL,0,2+h,w,sl_size);

    gui_container_add(GUI_CONTAINER(t),(WIDGET*)(t->text));
    gui_container_add(GUI_CONTAINER(t),(WIDGET*)(t->dbut));
    gui_container_add(GUI_CONTAINER(t),(WIDGET*)(t->slist));

    GUI_WIDGET(t)->unmap=cl_unmap;
    GUI_WIDGET(t)->map=cl_map;
    GUI_WIDGET(t->slist)->unmap(GUI_WIDGET(t->slist));
    t->slist->sel_change=list_change;
    gui_widget_add_keyfocus(GUI_WIDGET(t->slist),GUI_WIDGET(t->slist->text));
    return t;
}

void gui_combol_update(COMBO_LIST *c) {
    gui_slist_update(c->slist);
}
void gui_combol_set_selection(COMBO_LIST *c,Uint32 index) {
    gui_slist_set_selection(c->slist,index);
    gui_textbox_set_text(c->text,c->slist->get_label(c->slist->data_selected->data));
}
