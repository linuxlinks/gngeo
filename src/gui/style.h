#ifndef _STYLE_H_
#define _STYLE_H_

#include "SDL.h"

enum {
    STYLE_BUTTON,
    STYLE_TEXT,
    STYLE_LABEL,
    STYLE_FRAME,
    STYLE_SELECTION,

    STYLE_VSB_UP,
    STYLE_VSB_DOWN,
    STYLE_VSB_MOBIL,
    STYLE_VSB_BACK,
    
    STYLE_HSB_UP,
    STYLE_HSB_DOWN,
    STYLE_HSB_MOBIL,
    STYLE_HSB_BACK,

    STYLE_MAX
};

//typedef void (*drawing_array)(SDL_Rect *rect) drawinf_func;

typedef struct STYLE {
    char *name;
    SDL_bool (*init)(char *datapath);
    void (*close)(void);
    //drawing_func drw_array[STYLE_MAX];
}STYLE;

SDL_bool style_init(char *name,char *datapath);
void style_close(void);
SDL_bool style_is_init(void);
Uint32 style_strlen(char *str);
Uint16 style_get_font_h(void);
Uint16 style_get_font_w(void);
Uint16 style_get_border_w(void);
Uint16 style_get_border_h(void);

/* widget state */
#define WIDGET_CLICKED  1
#define WIDGET_FOCUSED 2

#define FRAME_SHADOW_IN 1
#define FRAME_SHADOW_OUT 2
#define FRAME_SHADOW_ETCHED_IN 4
#define FRAME_SHADOW_ETCHED_OUT 8
//#define FRAME_SHADOW_NONE 
#define FRAME_NO_BACK  16

/* must be initialize by style->init function */
void (*style_drw_text)(SDL_Surface *desk,SDL_Rect *rect,char *text);
void (*style_drw_label)(SDL_Surface *desk,SDL_Rect *rect,char *text);
void (*style_drw_button)(SDL_Surface *desk,SDL_Rect *rect,Uint32 state);
void (*style_drw_check_button)(SDL_Surface *desk,SDL_Rect *rect,Uint32 state);
void (*style_drw_frame)(SDL_Surface *desk,SDL_Rect *rect,Uint8 type,char *label);
void (*style_drw_selection)(SDL_Surface *desk,SDL_Rect *rect);
void (*style_drw_cursor)(SDL_Surface *desk,SDL_Rect *rect);
void (*style_drw_txtbg)(SDL_Surface *desk,SDL_Rect *rect);
void (*style_drw_focus_hlight)(SDL_Surface *desk,SDL_Rect *rect);
void (*style_drw_key_focus_hlight)(SDL_Surface *desk,SDL_Rect *rect);
void (*style_drw_txt_cursor)(SDL_Surface *desk,Uint16 x,Uint16 y,Uint16 pos);

void (*style_drw_sbup)(SDL_Surface *desk,SDL_Rect *rect,Uint8 dir,Uint32 state);
void (*style_drw_sbdown)(SDL_Surface *desk,SDL_Rect *rect,Uint8 dir,Uint32 state);
void (*style_drw_sbmobil)(SDL_Surface *desk,SDL_Rect *rect,Uint8 dir,Uint32 state);
void (*style_drw_sbback)(SDL_Surface *desk,SDL_Rect *rect,Uint8 dir,Uint32 state);
void (*style_drw_progress)(SDL_Surface *desk,SDL_Rect *rect,Uint32 size,Uint32 val);

void (*style_get_font_size)(Uint16 *w,Uint16 *h);
void (*style_get_border_size)(Uint16 *w,Uint16 *h);


#endif
