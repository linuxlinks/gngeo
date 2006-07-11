#ifndef _CONTAINER_H_
#define _CONTAINER_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "SDL.h"
#include "widget.h"
#include "../list.h"

typedef struct CONTAINER {
    WIDGET widget;
    LIST *son_list;

    /* keyboard focus handling */
/*
    LIST *key_list;
    LIST *first_key;
*/
    /* If true, no more widgets are accepted */
    SDL_bool blocked;
}CONTAINER;

#define GUI_CONTAINER(a) ((CONTAINER *)a)

CONTAINER* gui_container_create(CONTAINER *c,Sint16 x,Sint16 y,Uint16 w,Uint16 h);
void gui_container_add(CONTAINER *self,WIDGET *son);
void container_draw_son(WIDGET *self,SDL_Surface *desk);
void container_move_son(WIDGET *self,int x,int y);
void gui_container_block(CONTAINER *self);

#endif
