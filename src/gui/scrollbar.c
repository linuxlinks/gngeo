#include <stdlib.h>
#include "SDL.h"
#include "gui.h"
#include "style.h"


#define SB_MIN SB_H

static void sb_draw_back(struct WIDGET* self,SDL_Surface *desk){
    //SDL_Rect wrect={self->x,self->y,self->w,self->h};
    SCROLLBAR *sb=(SCROLLBAR*)self->father;
    if (!WID_IS_MAPPED(self)) return;
    //SDL_FillRect(desk,&self->rect,COL32_TO_16(0x969696));
    if (WID_HAS_MOUSECLICK(self))
	style_drw_sbback(desk,&self->rect,sb->dir,self->flags);
    else
	style_drw_sbback(desk,&self->rect,sb->dir,self->flags);
}

static void sb_draw_up(struct WIDGET* self,SDL_Surface *desk){
    //SDL_Rect wrect={self->x,self->y,self->w,self->h};
    SCROLLBAR *sb=(SCROLLBAR*)self->father;
    if (!WID_IS_MAPPED(self)) return;
    //SDL_FillRect(desk,&self->rect,COL32_TO_16(0x969696));
    if (WID_HAS_MOUSECLICK(self))
	style_drw_sbup(desk,&self->rect,sb->dir,self->flags);
    else
	style_drw_sbup(desk,&self->rect,sb->dir,self->flags);
}

static void sb_draw_down(struct WIDGET* self,SDL_Surface *desk){
    //SDL_Rect wrect={self->x,self->y,self->w,self->h};
    SCROLLBAR *sb=(SCROLLBAR*)self->father;
    if (!WID_IS_MAPPED(self)) return;
    //SDL_FillRect(desk,&self->rect,COL32_TO_16(0x969696));
    if (WID_HAS_MOUSECLICK(self))
	style_drw_sbdown(desk,&self->rect,sb->dir,self->flags);
    else
	style_drw_sbdown(desk,&self->rect,sb->dir,self->flags);
}

static void sb_draw_mobil(struct WIDGET* self,SDL_Surface *desk){
    //SDL_Rect wrect={self->x,self->y,self->w,self->h};
    SCROLLBAR *sb=(SCROLLBAR*)self->father;
    if (!WID_IS_MAPPED(self)) return;
    //SDL_FillRect(desk,&self->rect,COL32_TO_16(0x969696));
    if (WID_HAS_MOUSECLICK(self))
	style_drw_sbmobil(desk,&self->rect,sb->dir,self->flags);
    else
	style_drw_sbmobil(desk,&self->rect,sb->dir,self->flags);
}


static void update_pos(SCROLLBAR *sb) {
    Uint16 mobil_pos;
    if (sb->dir==SB_VERTICAL)
	mobil_pos=GUI_WIDGET(sb->mobil)->rect.y-(GUI_WIDGET(sb)->rect.y+SB_H);
    else
	mobil_pos=GUI_WIDGET(sb->mobil)->rect.x-(GUI_WIDGET(sb)->rect.x+SB_H);
    sb->pos=(mobil_pos*sb->size)/(float)sb->free_size;
    if (sb->change) sb->change(GUI_WIDGET(sb),sb->pos);
}

static void update_mobil(SCROLLBAR *sb) {
    Uint16 mobil_pos;
    
    mobil_pos=(sb->pos*sb->free_size)/(float)sb->size;
    if (sb->dir==SB_VERTICAL)
	GUI_WIDGET(sb->mobil)->rect.y=GUI_WIDGET(sb)->rect.y+SB_H+mobil_pos;
    else
	GUI_WIDGET(sb->mobil)->rect.x=GUI_WIDGET(sb)->rect.x+SB_H+mobil_pos;
}

static Uint32 update_sb_up(Uint32 interval,void *self)
{
    SCROLLBAR *sb=(SCROLLBAR*)self;
    Uint16 dt=1;//sb->size/(float)sb->free_size;
    if (sb->pos > dt) 
	sb->pos-=dt;
    else
	sb->pos=0;

    if (sb->change) sb->change(GUI_WIDGET(sb),sb->pos);
    update_mobil(sb);
//    gui_redraw((WIDGET*)self);
    gui_redraw(gui_get_root((WIDGET*)self),SDL_TRUE);

    if (WID_HAS_MOUSECLICK(sb->up) && sb->pos!=0)
	return interval;
    else
	return 0;
}

static Uint32 update_sb_down(Uint32 interval,void *self)
{
    SCROLLBAR *sb=(SCROLLBAR*)self;
    Uint16 dt=1;//sb->size/(float)sb->free_size;
    if (sb->pos < sb->size-dt) 
	sb->pos+=dt;
    else
	sb->pos=sb->size;

    if (sb->change) sb->change(GUI_WIDGET(sb),sb->pos);
    update_mobil(sb);
    //gui_redraw((WIDGET*)self);
    gui_redraw(gui_get_root((WIDGET*)self),SDL_TRUE);

    if (WID_HAS_MOUSECLICK(sb->down) && sb->pos!=sb->size)
	return interval;
    else
	return 0;
}

static int mouse_mobil(struct WIDGET* self,SDL_MouseMotionEvent *event) {
    SCROLLBAR *sb=(SCROLLBAR*)self->father;

    if (sb->free_size==0) return TAKE_EVENT; /* nothing to do */

    if (WID_HAS_MOUSECLICK(self)) {
	if (sb->dir==SB_VERTICAL) {
	    GUI_WIDGET(sb->mobil)->rect.y+=event->yrel;
	    if (GUI_WIDGET(sb->mobil)->rect.y<GUI_WIDGET(sb)->rect.y+SB_H)
		GUI_WIDGET(sb->mobil)->rect.y=GUI_WIDGET(sb)->rect.y+SB_H;
	    if (GUI_WIDGET(sb->mobil)->rect.y>(GUI_WIDGET(sb)->rect.y-sb->mobil->rect.h)+GUI_WIDGET(sb)->rect.h-SB_H)
		GUI_WIDGET(sb->mobil)->rect.y=(GUI_WIDGET(sb)->rect.y-sb->mobil->rect.h)+GUI_WIDGET(sb)->rect.h-SB_H;
	} else {
	    GUI_WIDGET(sb->mobil)->rect.x+=event->xrel;
	    if (GUI_WIDGET(sb->mobil)->rect.x<GUI_WIDGET(sb)->rect.x+SB_H)
		GUI_WIDGET(sb->mobil)->rect.x=GUI_WIDGET(sb)->rect.x+SB_H;
	    if (GUI_WIDGET(sb->mobil)->rect.x>(GUI_WIDGET(sb)->rect.x-sb->mobil->rect.w)+GUI_WIDGET(sb)->rect.w-SB_H)
		GUI_WIDGET(sb->mobil)->rect.x=(GUI_WIDGET(sb)->rect.x-sb->mobil->rect.w)+GUI_WIDGET(sb)->rect.w-SB_H;
	}
	update_pos(sb);
    }
    return TAKE_EVENT;
}

static int click_mobil(struct WIDGET* self,SDL_MouseButtonEvent *event) {

    return TAKE_EVENT;
}

static int click_up(struct WIDGET* self,SDL_MouseButtonEvent *event) {
    SCROLLBAR *sb=(SCROLLBAR*)self->father;
    Uint16 dt=1;//sb->size/(float)sb->free_size;

    if (sb->free_size==0) return TAKE_EVENT; /* nothing to do */

    if (event->type == SDL_MOUSEBUTTONDOWN) {
	update_sb_up(0,sb);
	SDL_AddTimer(10,update_sb_up,sb);
    }
    return TAKE_EVENT;
}

static int click_down(struct WIDGET* self,SDL_MouseButtonEvent *event) {
    SCROLLBAR *sb=(SCROLLBAR*)self->father;
    Uint16 dt=1;//sb->size/(float)sb->free_size;

    if (sb->free_size==0) return TAKE_EVENT; /* nothing to do */

    if (event->type == SDL_MOUSEBUTTONDOWN) {
	update_sb_down(0,sb);
	SDL_AddTimer(10,update_sb_down,sb);
    }
    return TAKE_EVENT;
}

static struct WIDGET* widget_at_pos(struct WIDGET* self,int mouse_x,int mouse_y) {
    SCROLLBAR *sb=(SCROLLBAR*)self;

    if (!WID_IS_MAPPED(self)) return NULL;

    if (sb->mobil->widget_at_pos(sb->mobil,mouse_x,mouse_y))
	return sb->mobil;
    if (sb->back->widget_at_pos(sb->back,mouse_x,mouse_y))
	return sb->back;
    if (sb->up->widget_at_pos(sb->up,mouse_x,mouse_y))
	return sb->up;
    if (sb->down->widget_at_pos(sb->down,mouse_x,mouse_y))
	return sb->down;
    
    return NULL;
}

static void draw_sb(struct WIDGET* self,SDL_Surface *desk) {
    SCROLLBAR *sb=GUI_SCROLLBAR(self);

    if (!WID_IS_MAPPED(self)) return;

    sb->back->draw(sb->back,desk);
    sb->up->draw(sb->up,desk);
    sb->down->draw(sb->down,desk);
    sb->mobil->draw(sb->mobil,desk);
}

static void move_sb(struct WIDGET* self,Sint16 x,Sint16 y) {
    SCROLLBAR *sb=GUI_SCROLLBAR(self);
    
    self->rect.x+=x;
    self->rect.y+=y;
    sb->back->move(sb->back,x,y);
    sb->up->move(sb->up,x,y);
    sb->down->move(sb->down,x,y);
    sb->mobil->move(sb->mobil,x,y);
}

static void map_sb(struct WIDGET* self) {
    SCROLLBAR *t=GUI_SCROLLBAR(self);
    t->down->map(t->down);
    t->up->map(t->up);
    t->back->map(t->back);
    t->mobil->map(t->mobil);
    self->flags|=FLAG_MAPPED;
}

static void unmap_sb(struct WIDGET* self) {
    SCROLLBAR *t=GUI_SCROLLBAR(self);
    t->down->unmap(t->down);
    t->up->unmap(t->up);
    t->back->unmap(t->back);
    t->mobil->unmap(t->mobil);
    self->flags&=(~FLAG_MAPPED);
}

SCROLLBAR *gui_scrollbar_create(SCROLLBAR *sb,Sint16 x,Sint16 y,Uint16 s,
				Uint8 dir,Uint32 size,Uint32 vsize,void (*change)(WIDGET *self,Uint32 val)) {
    SCROLLBAR *t;
    Uint16 mobil_size=((vsize)*(s-2*SB_H))/(float)size;
    
    if (mobil_size<SB_MIN) mobil_size=SB_MIN;
    if (mobil_size>(s-2*SB_H)) mobil_size=(s-2*SB_H);
    
    if (sb)
	t=sb;
    else
	t=malloc(sizeof(SCROLLBAR));
    
    t->mobil_size=mobil_size;
    t->free_size=(s-2*SB_H)-mobil_size;
    t->vsize=vsize;
    t->s=s;

    // printf("scroll bar size %d %d\n",t->mobil_size,t->free_size);

    if (dir==SB_VERTICAL) {
	gui_widget_create(&t->widget,x,y,SB_H,s);
	t->back=gui_widget_create(NULL,x,y+SB_H,SB_H,s-SB_H*2);
	t->up=(WIDGET*)gui_widget_create(NULL,x,y,SB_H,SB_H);
	t->down=(WIDGET*)gui_widget_create(NULL,x,y+s-SB_H,SB_H,SB_H);
	t->mobil=(WIDGET*)gui_widget_create(NULL,x,y+SB_H,SB_H,mobil_size);
    } else {
	gui_widget_create(&t->widget,x,y,s,SB_H);
	t->back=gui_widget_create(NULL,x+SB_H,y,s-SB_H*2,SB_H);
	t->up=(WIDGET*)gui_button_create(NULL,x,y,SB_H,SB_H);
	t->down=(WIDGET*)gui_button_create(NULL,x+s-SB_H,y,SB_H,SB_H);
	t->mobil=(WIDGET*)gui_button_create(NULL,x+SB_H,y,mobil_size,SB_H);
    }

    t->dir=dir;
    t->change=change;
    t->pos=0;
    t->size=size;

    /* setup father */
    t->down->father=t->up->father=(WIDGET*)t;
    t->mobil->father=t->back->father=(WIDGET*)t;
    
    /* setup drawing func */
    t->back->draw=sb_draw_back;
    t->down->draw=sb_draw_down;
    t->up->draw=sb_draw_up;
    t->mobil->draw=sb_draw_mobil;

    /* setup event callback */
    GUI_WIDGET(t)->map=map_sb;
    GUI_WIDGET(t)->unmap=unmap_sb;

    t->down->click=click_down;
    t->up->click=click_up;
    t->mobil->click=click_mobil;
    t->mobil->mouse=mouse_mobil;

    

    GUI_WIDGET(t)->draw=draw_sb;
    GUI_WIDGET(t)->move=move_sb;
    GUI_WIDGET(t)->widget_at_pos=widget_at_pos;
    return t;
}

void gui_scrollbar_set_size(SCROLLBAR *sb,Uint32 size) {
    Uint16 mobil_size=((sb->vsize)*(sb->s-2*SB_H))/(float)size;

//    printf("SB set size to %d vsize=%d msize=%d (dir=%d )\n",size,sb->vsize,mobil_size,sb->dir);
    
    if (mobil_size<SB_MIN) mobil_size=SB_MIN;
    if (mobil_size>(sb->s-2*SB_H)) mobil_size=(sb->s-2*SB_H);
    if (size<(sb->s-2*SB_H)) mobil_size=(sb->s-2*SB_H);
    
    sb->mobil_size=mobil_size;
    sb->free_size=(sb->s-2*SB_H)-mobil_size;
    sb->size=size;
//    printf("sb update %d %d \n",sb->pos,size);
    //  if (sb->pos>size) sb->pos=size;
    sb->pos=0;

    if (sb->dir==SB_VERTICAL) {
	sb->mobil->rect.h=mobil_size;
    } else {
	sb->mobil->rect.w=mobil_size;
    }
    update_mobil(sb);
}
