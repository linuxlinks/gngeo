#include "SDL.h"
#include <stdlib.h>
#include "container.h"
#include "style.h"
#include "../list.h"

void container_draw_son(WIDGET *self,SDL_Surface *desk) {
    CONTAINER *c=(CONTAINER *)self;
    LIST *l;
/*
    SDL_Rect clipsave;

    SDL_GetClipRect(desk,&clipsave);
    SDL_SetClipRect(desk,&self->rect);
*/  
    for(l=c->son_list;l;l=l->next) {
	WIDGET *w=l->data;
	if (WID_IS_MAPPED(w)) w->draw(w,desk);
    }

//    SDL_SetClipRect(desk,&clipsave);
}

void container_move_son(WIDGET *self,int x,int y) {
    LIST *l;
    for (l=GUI_CONTAINER(self)->son_list;l;l=l->next) {
	WIDGET *w=(WIDGET*)l->data;
	w->move(w,x,y);
    }
}

void container_destroy(WIDGET *self,SDL_bool destroy_son) {
    LIST *l;
    if (destroy_son==SDL_TRUE) {
	for(l=GUI_CONTAINER(self)->son_list;l;l=l->next) {
	    WIDGET *w=l->data;
	    w->destroy(w,destroy_son);
	}
    }
    free(self);
}

static void move_container(struct WIDGET *self,Sint16 x,Sint16 y) {
    //printf("move container %p %d %d\n",self,x,y);
    self->rect.x+=x;
    self->rect.y+=y;
    container_move_son(self,x,y);
}

static void map_container(WIDGET *self) {
    CONTAINER *c=(CONTAINER *)self;
    LIST *l;
    self->flags|=FLAG_MAPPED;

    for(l=c->son_list;l;l=l->next) {
	WIDGET *w=l->data;
	w->map(w);
    }

}

static void unmap_container(WIDGET *self) {
    CONTAINER *c=(CONTAINER *)self;
    LIST *l;
    self->flags&=(~FLAG_MAPPED);

    for(l=c->son_list;l;l=l->next) {
	WIDGET *w=l->data;
	w->unmap(w);
    }

}

static void container_draw(WIDGET *self,SDL_Surface *desk) {
    CONTAINER *c=(CONTAINER *)self;
//    SDL_Rect wrect={self->x,self->y,self->w,self->h};
    if (!WID_IS_MAPPED(self)) return;
    //SDL_FillRect(desk,&self->rect,rand()%0xFFFF);
    container_draw_son(self,desk);
}

static struct WIDGET* widget_at_pos(struct WIDGET* self,int mouse_x,int mouse_y) {
    CONTAINER *c=(CONTAINER *)self;
    LIST *l;

    if (!WID_IS_MAPPED(self)) return NULL;

    if (!point_is_in_rect(mouse_x,mouse_y,&self->rect))
	return NULL;
    
    for(l=c->son_list;l;l=l->next) {
	WIDGET *w=l->data;
	WIDGET *wid;
	if ((wid=w->widget_at_pos(w,mouse_x,mouse_y))!=NULL)
	    return wid;
    }
    return self;
}

#if 0
static struct WIDGET* get_key_widget(struct WIDGET* self) {
    CONTAINER *c=(CONTAINER *)self;
    LIST *l;
    WIDGET *w,*kw=NULL;
    printf("Get_key_widget_container %p %p %p\n",self,c->key_list,c->first_key);

     if (!WID_IS_MAPPED(self)) return NULL;
     if (c->son_list==NULL) return NULL;
     if (c->key_list==NULL) {
	 c->key_list=c->son_list;
	 return NULL;
     }     
     do {
	 w=c->key_list->data;
	 kw=w->get_key_widget(w);

	 c->key_list=c->key_list->next;
	 if (c->key_list==NULL || kw!=NULL) {
	     //c->key_list=c->son_list;
	     break;
	 }
     }while (1);

     return kw;

/*
    if (c->key_list==NULL) return NULL;
    c->key_list=c->key_list->next;
    if (c->key_list==NULL) {
	printf("WRAPP!!!\n");
	c->key_list=c->first_key;
    }
    w=c->key_list->data;
    printf("Get_key_widget_container %p -> %p\n",self,w);
    return w->get_key_widget(w);
*/
}
#endif

CONTAINER* gui_container_create(CONTAINER *c,Sint16 x,Sint16 y,Uint16 w,Uint16 h) 
{
    CONTAINER *t;
    if (!c)
	t=malloc(sizeof(CONTAINER));
    else
	t=c;

    //printf("ADD container %p %d %d %d %d\n",t,x,y,w,h);

    gui_widget_create(&t->widget,x,y,w,h);
    t->son_list=NULL;
    //t->key_list=NULL;t->first_key=NULL;
    t->blocked=SDL_FALSE;

    t->widget.draw=container_draw;
    t->widget.widget_at_pos=widget_at_pos;
    t->widget.map=map_container;
    t->widget.unmap=unmap_container;
    t->widget.move=move_container;
    t->widget.destroy=container_destroy;
//    t->widget.get_key_widget=get_key_widget;
    return t;
}

void gui_container_add(CONTAINER *self,WIDGET *son) {
    Uint16 bw,bh;
    WIDGET *kwidget;

    if (self->blocked) {
	fprintf(stderr,"WARNING: Trying to add a widget to a blocked container.\n");
	return ;
    }
    
    /* recode that for every widget */
    style_get_border_size(&bw,&bh);
    if (son->rect.x==0)
	son->rect.x=bw;
    if (son->rect.y==0)
	son->rect.y=bh;

    if (son->rect.w==0)
	son->rect.w=((WIDGET*)self)->rect.w - son->rect.x;
    if (son->rect.h==0)
	son->rect.h=((WIDGET*)self)->rect.h - son->rect.y;
    
    //printf("container_add %p,%p %d %d \n",self,son,((WIDGET*)self)->x,((WIDGET*)self)->y);
     
    son->move(son,((WIDGET*)self)->rect.x,((WIDGET*)self)->rect.y);

    
    son->father=((WIDGET*)self);
    self->son_list=list_prepend(self->son_list,son);
/*
    if (self->key_list==NULL)
	self->key_list=self->son_list;
*/
/*
    if ((kwidget=son->get_key_widget(son))!=NULL) {
	printf("Add key widget %p\n",kwidget);
	if (self->key_list==NULL) {
	    self->key_list=list_append(self->key_list,kwidget);
	    self->first_key=self->key_list;
	} else 
	    self->key_list=list_append(self->key_list,kwidget);
    }
*/

}

void gui_container_block(CONTAINER *self) {
    self->blocked=SDL_TRUE;
}
