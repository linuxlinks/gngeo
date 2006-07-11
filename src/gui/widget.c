#include "SDL.h"
#include <stdlib.h>
#include "gui.h"
#include "widget.h"

static int default_click(struct WIDGET* self,SDL_MouseButtonEvent *event) {
    char *type;
    if (event->type == SDL_MOUSEBUTTONDOWN)
	type="DOWN";
    else
	type="UP";
    
    return PASS_EVENT;
}

static int default_double_click(struct WIDGET* self,SDL_MouseButtonEvent *event) {
    return PASS_EVENT;
}

static int default_mouse(struct WIDGET* self,SDL_MouseMotionEvent *event) {
    return PASS_EVENT;
}

static int default_key(struct WIDGET* self,SDL_KeyboardEvent *event) {
    return PASS_EVENT;
}

static void default_lose_focus(struct WIDGET* self) {
}

static void default_gain_focus(struct WIDGET* self) {
}

static void default_lose_mouse(struct WIDGET* self) {
}

static void default_gain_mouse(struct WIDGET* self) {
}

static void map_widget(struct WIDGET* self) {
    //self->mapped=SDL_TRUE;
    self->flags|=FLAG_MAPPED;
}

static void unmap_widget(struct WIDGET* self) {
    //self->mapped=SDL_FALSE;
    self->flags&=(~FLAG_MAPPED);
}

static void move_widget(struct WIDGET* self,Sint16 x,Sint16 y) {
//    printf("move widget %p %d %d\n",self,x,y);
    self->rect.x+=x;
    self->rect.y+=y;
}

static void draw_widget(struct WIDGET* self,SDL_Surface *desk) {
    if (!WID_IS_MAPPED(self)) return;
    SDL_FillRect(desk,&self->rect,rand()%0xFFFF);
}

static void destroy_widget(struct WIDGET* self,SDL_bool destroy_son) {
    free(self);
}

static struct WIDGET* widget_at_pos(struct WIDGET* self,int mouse_x,int mouse_y) {
    if (!WID_IS_MAPPED(self)) return NULL;
    if (point_is_in_rect(mouse_x,mouse_y,&self->rect))
	return self;
    return NULL;
}
static struct WIDGET* default_key_widget(struct WIDGET* self) {
    printf("Get_key_widget default %p\n",self);
    return NULL;
}
/* create a widget and return it.
   if w is not NULL, use it instead of allocate a new one
*/
WIDGET *gui_widget_create(WIDGET *widget,Sint16 x,Sint16 y,Uint16 w,Uint16 h) {
    WIDGET *t=NULL;

    if (!widget)
	t=malloc(sizeof(WIDGET));
    else
	t=widget;

    //printf("Create wid %p\n",t);

    t->rel_x=t->rect.x=x;
    t->rel_y=t->rect.y=y;
    t->rect.w=w;
    t->rect.h=h;
    t->data1=t->data2=0;
    t->pdata1=t->pdata2=t->pdata3=NULL;
    t->father=NULL;
    //t->have_focus=SDL_FALSE;
    //t->m_state=0;
    t->flags=0;
    t->flags|=FLAG_MAPPED;
    t->click=default_click;
    t->double_click=default_double_click;
    t->mouse=default_mouse;
    t->key=default_key;
    t->lose_focus=default_lose_focus;
    t->gain_focus=default_gain_focus;
    
    t->lose_mouse=default_lose_mouse;
    t->gain_mouse=default_gain_mouse;

    t->key_focus=NULL;
    t->first_key=NULL;

    t->draw=draw_widget;
    t->widget_at_pos=widget_at_pos;
    //t->next_key_focus=NULL;
    t->map=map_widget;
    t->unmap=unmap_widget;
    t->move=move_widget;
    t->destroy=destroy_widget;
    return t;
}

WIDGET *gui_get_root(WIDGET *self) {
    WIDGET *w;
    for (w=self;w->father;w=w->father);
    return w;
}

/* Destroy only the widget w */
void gui_widget_destroy(WIDGET *w) {
    w->destroy(w,SDL_FALSE);
}

/* destroy all the widget, including its son */
void gui_widget_destroy_all(WIDGET *w) {
    w->destroy(w,SDL_TRUE);
}

void gui_widget_add_keyfocus(WIDGET *base,WIDGET *key_wid) {
    base->key_focus=list_append(base->key_focus,key_wid);
    if (base->first_key==NULL)
	base->first_key=base->key_focus;
}

WIDGET *gui_widget_get_next_keyfocus(WIDGET *base) {
    WIDGET *w;
    LIST *t=base->key_focus;

    if (base->first_key==NULL) return NULL;
    do {
	w=base->key_focus->data;
	base->key_focus=base->key_focus->next;
	if (base->key_focus==NULL)
	    base->key_focus=base->first_key;
    } while((!WID_IS_MAPPED(w)) && base->key_focus!=t);
    if (WID_IS_MAPPED(w))
	return w;
    return NULL;
}

/* return SDL_True if point is inside rect */
SDL_bool point_is_in_rect(Sint16 x,Sint16 y,SDL_Rect *rect) {
    if (x<rect->x || y<rect->y)
	return SDL_FALSE;
    if (x>=rect->x+rect->w || y>rect->y+rect->h)
	return SDL_FALSE;
    return SDL_TRUE; 
}
