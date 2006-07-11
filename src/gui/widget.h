#ifndef _WIDGET_H_
#define _WIDGET_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../list.h"
#include "SDL.h"

#define FLAG_NONE       0x00
#define FLAG_MAPPED     0x01 /* the widget is visible */
#define FLAG_MOUSEOVER  0x02 /* the widget has the mouse */
#define FLAG_FOCUS      0x04 /* The widget has the keyboard focus */
#define FLAG_MOUSECLICK 0x08 /* a button is pressed */
#define FLAG_CHECKED    0x10 /* for check/radio button */

#define WID_HAS_FOCUS(wid) (((wid)->flags&FLAG_FOCUS)==FLAG_FOCUS)
#define WID_HAS_MOUSE(wid) (((wid)->flags&FLAG_MOUSEOVER)==FLAG_MOUSEOVER)
#define WID_HAS_MOUSECLICK(wid) (((wid)->flags&FLAG_MOUSECLICK)==FLAG_MOUSECLICK)
#define WID_IS_MAPPED(wid) (((wid)->flags&FLAG_MAPPED)==FLAG_MAPPED)
#define WID_IS_CHECKED(wid) (((wid)->flags&FLAG_CHECKED)==FLAG_CHECKED)


typedef struct WIDGET {
//    Uint16 x,y,w,h;   /* size and pos of the widget */
    SDL_Rect rect;
    Sint16 rel_x,rel_y; /* relative position again his parent */
    //Uint8 have_focus; /* True is the widget has focus */
    //Uint8 m_state;    /* state of the mouse button */
    //Uint8 mapped;
    Uint32 flags; /* state of the widget */
    
    /* User data */
    int data1;
    int data2;
    void *pdata1;
    void *pdata2;  
    void *pdata3;

    struct WIDGET *father;

    /* keyboard focus list */
    LIST *key_focus;
    LIST *first_key;

    

    /* event */
    int (*click)(struct WIDGET* self,SDL_MouseButtonEvent *event);
    int (*double_click)(struct WIDGET* self,SDL_MouseButtonEvent *event);
    int (*mouse)(struct WIDGET* self,SDL_MouseMotionEvent *event);
    int (*key)(struct WIDGET* self,SDL_KeyboardEvent *event);
    void (*lose_focus)(struct WIDGET* self);
    void (*gain_focus)(struct WIDGET* self);

    void (*lose_mouse)(struct WIDGET* self);
    void (*gain_mouse)(struct WIDGET* self);

    void (*draw)(struct WIDGET* self,SDL_Surface *desk);
    struct WIDGET *(*widget_at_pos)(struct WIDGET* self,int mouse_x,int mouse_y);
    //struct WIDGET *(*get_key_widget)(struct WIDGET* self);
    void (*map)(struct WIDGET* self);
    void (*unmap)(struct WIDGET* self);
    void (*move)(struct WIDGET* self,Sint16 x,Sint16 y);
    void (*destroy)(struct WIDGET* self,SDL_bool destroy_son);
    
}WIDGET;

#define GUI_WIDGET(a) ((WIDGET *)a)

WIDGET *gui_widget_create(WIDGET *widget,Sint16 x,Sint16 y,Uint16 w,Uint16 h);
WIDGET *gui_get_root(WIDGET *self);
void gui_widget_destroy(WIDGET *w);
void gui_widget_destroy_all(WIDGET *w);
void gui_widget_add_keyfocus(WIDGET *base,WIDGET *key_wid);
WIDGET *gui_widget_get_next_keyfocus(WIDGET *base);

/* Utility functions */
SDL_bool point_is_in_rect(Sint16 x,Sint16 y,SDL_Rect *rect);

#endif

