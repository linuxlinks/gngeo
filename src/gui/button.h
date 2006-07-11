#ifndef _BUTTON_H_
#define _BUTTON_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "SDL.h"
#include "container.h"

typedef struct BUTTON {
    CONTAINER cont;
    int (*activate)(struct BUTTON *self);
}BUTTON;

typedef struct CHECK_BUTTON {
    CONTAINER cont;
//    SDL_bool checked;
    char *label;
    int (*toggle)(struct CHECK_BUTTON *self);
}CHECK_BUTTON;

#define GUI_BUTTON(a) ((BUTTON *)a)
#define GUI_CHECK_BUTTON(a) ((CHECK_BUTTON *)a)

BUTTON *gui_button_create(BUTTON *b,Sint16 x,Sint16 y,Uint16 w,Uint16 h);
BUTTON *gui_button_label_create(BUTTON *b,Sint16 x,Sint16 y,Uint16 w,Uint16 h,char *label);
CHECK_BUTTON *gui_check_button_create(CHECK_BUTTON *b,Sint16 x,Sint16 y,Uint16 w,Uint16 h,char *label);

#endif
