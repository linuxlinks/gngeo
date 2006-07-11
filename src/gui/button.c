#include <stdlib.h>
#include "SDL.h"
#include "button.h"
#include "gui.h"
#include "draw.h"
#include "style.h"

static int def_activate(struct BUTTON *self) {
    return TAKE_EVENT;
}
static int button_click(struct WIDGET* self,SDL_MouseButtonEvent *event) {
    BUTTON *b=(BUTTON*)self;
    //printf("Clicke event \n");
    if (event->type==SDL_MOUSEBUTTONDOWN) return TAKE_EVENT;
    if (b->activate) return b->activate(b);
    return TAKE_EVENT;
}
static int key_event(struct WIDGET* self,SDL_KeyboardEvent *event) {
    BUTTON *b=(BUTTON*)self;

    if (event->type==SDL_KEYDOWN) {
	switch(event->keysym.sym) {
	case SDLK_RETURN: {
	    if (b->activate) return b->activate(b);
	    break;
	}
	default:
	    break;
	}
    }
    return TAKE_EVENT;
}


static void button_draw(struct WIDGET* self,SDL_Surface *desk) {
    //SDL_Rect but_rec={self->x,self->y,self->w,self->h};
    if ((self->flags&FLAG_MAPPED)!=FLAG_MAPPED) return;
    if ((self->flags&FLAG_MOUSECLICK)) {
	//draw_border(desk,SHADOW_IN,&but_rec);
	style_drw_button(desk,&self->rect,self->flags);
	container_move_son(self,1,1);
	container_draw_son(self,desk);
	container_move_son(self,-1,-1);
    } else {
	//draw_border(desk,SHADOW_OUT,&but_rec);
	style_drw_button(desk,&self->rect,self->flags);
	container_draw_son(self,desk);
    }
}

static void check_button_draw(struct WIDGET* self,SDL_Surface *desk) {
    CHECK_BUTTON *b=GUI_CHECK_BUTTON(self);
    char *label=b->label;
    SDL_Rect focus_rect={self->rect.x+1,self->rect.y+1,self->rect.w-3,self->rect.h-3};

    if ((self->flags&FLAG_MAPPED)!=FLAG_MAPPED) return;
    if (label) {
	SDL_Rect crect={self->rect.x+2,self->rect.y+2,self->rect.h-4,self->rect.h-4};
	SDL_Rect trect={self->rect.x+self->rect.h+2,self->rect.y+2,1,1};
	if (self->flags&FLAG_MOUSEOVER) style_drw_focus_hlight(desk,&self->rect);
	if ((self->flags&FLAG_CHECKED))
	    style_drw_check_button(desk,&crect,self->flags);
	else
	    style_drw_check_button(desk,&crect,self->flags);
	
	style_drw_text(desk,&trect,label);
	if (self->flags&FLAG_FOCUS)
	    style_drw_key_focus_hlight(desk,&focus_rect);
	
    } else {
	if (self->flags&FLAG_CHECKED)
	    style_drw_check_button(desk,&self->rect,self->flags);
	else
	    style_drw_check_button(desk,&self->rect,self->flags);
	container_draw_son(self,desk);
    }
}

static int button_default_click(struct WIDGET* self,SDL_MouseButtonEvent *event) {
    /* nothing but don't send the event to father */
    return TAKE_EVENT;
}

static int check_button_click(struct WIDGET* self,SDL_MouseButtonEvent *event) {

    CHECK_BUTTON *b=GUI_CHECK_BUTTON(self);

    if (event->type==SDL_MOUSEBUTTONDOWN) return TAKE_EVENT;


    if (self->flags&FLAG_CHECKED)
	self->flags&=(~FLAG_CHECKED);
    else
	self->flags|=FLAG_CHECKED;
    //b->checked=(b->checked==SDL_TRUE?SDL_FALSE:SDL_TRUE);
    if (b->toggle) return b->toggle(b);
    return TAKE_EVENT;
}

static int check_key_event(struct WIDGET* self,SDL_KeyboardEvent *event) {
    CHECK_BUTTON *b=GUI_CHECK_BUTTON(self);

    if (event->type==SDL_KEYDOWN) {
	switch(event->keysym.sym) {
	case SDLK_RETURN: {
	    if (self->flags&FLAG_CHECKED)
		self->flags&=(~FLAG_CHECKED);
	    else
		self->flags|=FLAG_CHECKED;
	    if (b->toggle) return b->toggle(b);
	    break;
	}
	default:
	    break;
	}
    }
    return TAKE_EVENT;
}


BUTTON *gui_button_create(BUTTON *b,Sint16 x,Sint16 y,Uint16 w,Uint16 h) {
    BUTTON *t;
    if (b)
	t=b;
    else
	t=malloc(sizeof(BUTTON));
    gui_container_create(&t->cont,x,y,w,h);
//    ((WIDGET*)t)->get_key_widget=button_get_key;
    ((WIDGET*)t)->click=button_click;
    ((WIDGET*)t)->draw=button_draw;
    GUI_WIDGET(t)->key=key_event;
    return t;
}

CHECK_BUTTON *gui_check_button_create(CHECK_BUTTON *b,Sint16 x,Sint16 y,Uint16 w,Uint16 h,char *label) {
    CHECK_BUTTON *t;
    if (b)
	t=b;
    else
	t=malloc(sizeof(CHECK_BUTTON));
    gui_container_create(&t->cont,x,y,w,h);
    
    ((WIDGET*)t)->draw=check_button_draw;
    ((WIDGET*)t)->click=check_button_click;
//    ((WIDGET*)t)->get_key_widget=button_get_key;
    GUI_WIDGET(t)->key=check_key_event;

    t->label=label;
    t->toggle=NULL;
    return t;
}


BUTTON *gui_button_label_create(BUTTON *b,Sint16 x,Sint16 y,Uint16 w,Uint16 h,char *label)
{
    BUTTON *t;
    LABEL *l;
    int l_w=style_strlen(label);
    int l_h=style_get_font_h();
    int l_x=(w-l_w)/2;
    int l_y=(h-l_h)/2;

    //printf("button %p\n",b);
    
    if (l_x<0) l_x=0;
    if (l_y<0) l_y=0;
    
    if (b)
	t=b;
    else
	t=malloc(sizeof(BUTTON));
    
    //printf("-- button %p\n",b);

    gui_container_create(&t->cont,x,y,w,h);
    ((WIDGET*)t)->draw=button_draw;
    GUI_WIDGET(t)->key=key_event;
    ((WIDGET*)t)->click=button_click;
    t->activate=def_activate;

//    ((WIDGET*)t)->get_key_widget=button_get_key;
    
    l=gui_label_create(NULL,l_x,l_y,0,0,label);
    gui_container_add(GUI_CONTAINER(t),GUI_WIDGET(l));

    return t;
}
