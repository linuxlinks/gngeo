#include <stdlib.h>
#include "SDL.h"
#include "gui.h"
#include "style.h"

static void slist_vb_change(WIDGET *self,Uint32 val) {
    SLIST *sl=(SLIST *)self->father;
    int textsize=sl->text->rect.h-style_get_border_h()*2;
    sl->pos_y=val*(sl->vb->size-textsize)/(float)sl->vb->size;;
}

static void slist_hb_change(WIDGET *self,Uint32 val) {
    SLIST *sl=(SLIST *)self->father;
    int textsize=sl->text->rect.w-style_get_border_w()*2;
    sl->pos_x=val*(sl->hb->size-textsize)/(float)sl->hb->size;
}

static void slist_draw(WIDGET *self,SDL_Surface *desk) {
    SLIST *sl=(SLIST *)(self->father);
    LIST *l;
    Uint32 cur_pos=0;
    Uint32 i=0;
    SDL_Rect focus_rect={self->rect.x-1,self->rect.y-1,
			 self->rect.w+1,self->rect.h+1};
/*
    SDL_Rect wrect={self->rect.x,self->rect.y,self->rect.w-SB_H-1,self->rect.h-SB_H-1};
    SDL_Rect clip_rect={self->rect.x+2,self->rect.y+2,self->rect.w-SB_H-5,self->rect.h-SB_H-5};
*/
    SDL_Rect clip_save;
    Uint16 gui_font_h,gui_font_w;
    Uint16 borderw,borderh;

    if (!WID_IS_MAPPED(self)) return;

    style_get_font_size(&gui_font_w,&gui_font_h);
    style_get_border_size(&borderw,&borderh);

    //  draw_border_color(desk,SHADOW_IN,&wrect,0xFFFFFF);
    //container_draw_son(GUI_WIDGET(sl),desk);
    style_drw_txtbg(desk,&self->rect);

    /* set clipping for text */

    SDL_GetClipRect(desk,&clip_save);
    SDL_SetClipRect(desk,&self->rect);


    // printf("Draw %d %d\n",cur_pos,sl->pos_y);
    for (l=sl->l;l;l=l->next) {

	if (cur_pos+gui_font_h>=sl->pos_y) {
	    int y=self->rect.y+borderh+1+(cur_pos-sl->pos_y);
	    int x=self->rect.x+borderw+1-sl->pos_x;

	    if (i==sl->selected) {
		SDL_Rect sel={self->rect.x,y,self->rect.w,gui_font_h};
		//SDL_FillRect(desk,&sel,0x4876FF);
		style_drw_selection(desk,&sel);
	    }

	    if (sl->get_label) {
		SDL_Rect txt={x,y,0,0};
		//draw_string(desk,gui_font,x,y,sl->get_label(l->data));
		style_drw_text(desk,&txt,sl->get_label(l->data));
	    }
	}
	
	/* If nothing more is visible -> go out */
	if (cur_pos>sl->pos_y+gui_font_h+self->rect.h)
	    break;
	cur_pos+=gui_font_h+1;
	i++;
    }

    style_drw_frame(desk,&self->rect,FRAME_SHADOW_IN | FRAME_NO_BACK,NULL);

    /* restore clipping */
    SDL_SetClipRect(desk,&clip_save);
    if (self->flags&FLAG_FOCUS)
	style_drw_key_focus_hlight(desk,&focus_rect);
}

static SDL_bool selection_have_changed;

static int slist_click(struct WIDGET* self,SDL_MouseButtonEvent *event) {
    SLIST *sl=(SLIST*)(self->father);
    int i;
    
    if (event->type == SDL_MOUSEBUTTONDOWN) {
	i=((event->y-self->rect.y)+sl->pos_y)/((float)(style_get_font_h()+1));
	if (sl->selected!=i) {
	    selection_have_changed=SDL_TRUE;
	    sl->data_selected=list_get_item_by_index(sl->l,i);
	    sl->selected=i;
	} else {
	    selection_have_changed=SDL_FALSE;
	}
	if (sl->sel_change) return sl->sel_change(sl);
    }
    return TAKE_EVENT;
}

static int slist_double_click(struct WIDGET* self,SDL_MouseButtonEvent *event) {
    SLIST *sl=(SLIST*)(self->father);
    if (selection_have_changed==SDL_TRUE) return TAKE_EVENT; 
    if (sl->sel_double_click) return sl->sel_double_click(sl);
    return TAKE_EVENT;
}

static int key_event(struct WIDGET* self,SDL_KeyboardEvent *event) {
    SLIST *sl=(SLIST*)(self->father);
    int i;

    if (event->type==SDL_KEYDOWN) {
	switch(event->keysym.sym) {
	case SDLK_UP:
	    if (sl->selected>0) {
		sl->selected--;
		sl->data_selected=list_get_item_by_index(sl->l,sl->selected);
		if (sl->sel_change) sl->sel_change(sl);
	    }
	    break;
	case SDLK_DOWN:
	    if (sl->selected<list_nb_item(sl->l)-1) {
		sl->selected++;
		sl->data_selected=list_get_item_by_index(sl->l,sl->selected);
		if (sl->sel_change) sl->sel_change(sl);
	    }
	    break;
	case SDLK_RETURN:
	    if (sl->sel_double_click) return sl->sel_double_click(sl);
	    break;
	default:
	    break;
	}
    }
    return TAKE_EVENT;
}


SLIST *gui_slist_create(SLIST *sl,Sint16 x,Sint16 y,Uint16 w,Uint16 h) {
    SLIST *t;
    if (sl)
	t=sl;
    else
	t=malloc(sizeof(SLIST));

    gui_container_create(&t->cont,x,y,w,h);

    t->l=NULL;
    t->get_label=NULL;
    t->pos_x=t->pos_y=0;
    t->selected=0;
    t->data_selected=NULL;
    t->sel_change=NULL;

    t->vb=gui_scrollbar_create(NULL,w-SB_H,0,h-SB_H,SB_VERTICAL,0,h-SB_H-3,slist_vb_change);
    t->hb=gui_scrollbar_create(NULL,0,h-SB_H,w-SB_H,SB_HORIZONTAL,0,w-SB_H-3,slist_hb_change);
    t->text=gui_widget_create(NULL,0,0,w-SB_H-3,h-SB_H-3);
    
    GUI_WIDGET(t->text)->draw=slist_draw;
    GUI_WIDGET(t->text)->click=slist_click;
    GUI_WIDGET(t->text)->key=key_event;
    GUI_WIDGET(t->text)->double_click=slist_double_click;
//    GUI_WIDGET(t)->get_key_widget=slist_get_key;

    gui_container_add(GUI_CONTAINER(t),GUI_WIDGET(t->hb));
    gui_container_add(GUI_CONTAINER(t),GUI_WIDGET(t->vb));
    gui_container_add(GUI_CONTAINER(t),GUI_WIDGET(t->text));

    return t;
}

void gui_slist_update(SLIST *sl) {
    int size,max;
    LIST *l;
    char *label;
    Uint16 gui_font_h,gui_font_w;
    style_get_font_size(&gui_font_w,&gui_font_h);

    sl->pos_x=sl->pos_y=0;
    sl->selected=0;
    sl->data_selected=NULL;

    sl->nb_item=list_nb_item(sl->l);

    size=sl->nb_item*(gui_font_h+1)/*-(GUI_WIDGET(sl->text)->rect.h)*/;
    if (size<0) size=0;
//    printf("Size1 = %d %d %d %d\n",size,sl->nb_item,gui_font_h+1,GUI_WIDGET(sl->text)->rect.h);
    gui_scrollbar_set_size(sl->vb,size);
    max=0;
    for(l=sl->l;l;l=l->next) {
	if (sl->get_label) {
	    label=sl->get_label(l->data);
	    //printf("Have label %s\n",label);
	    size=(label?strlen(label):0);
	} else
	    size=0;
	
	if (size>max)
	    max=size;
    }
    //printf("max=%d %d\n",max,GUI_WIDGET(sl->text)->rect.w);
    size=max*gui_font_w/*-(GUI_WIDGET(sl->text)->rect.w)*/;
    if (size<0) size=0;
//    printf("Size2 = %d %d %d\n",size,max,gui_font_w);
    gui_scrollbar_set_size(sl->hb,size);
}

void gui_slist_set_selection(SLIST *sl,Uint32 index) {
    if (index<list_nb_item(sl->l)) {
	sl->selected=index;
	sl->data_selected=list_get_item_by_index(sl->l,sl->selected);
    }
}


