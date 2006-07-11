#ifndef _GUI_H_
#define _GUI_H_

#include "SDL.h"
#include "widget.h"
#include "container.h"
#include "button.h"
#include "label.h"
#include "frame.h"
#include "scrollbar.h"
#include "textbox.h"
#include "slist.h"
#include "progress.h"
#include "pixmap.h"
#include "combol.h"
#include "draw.h"
#include "style.h"

extern SDL_Surface *desk_buf;
extern SDL_mutex *disp_mut;
/*
extern SDL_Surface *gui_font;
extern int gui_font_w;
extern int gui_font_h;
*/

enum {
    PASS_EVENT=0,
    TAKE_EVENT,
    QUIT_EVENT,
};

void gui_init(int x,int y,char *data_path);
void gui_main_loop(WIDGET *base/*,int resx,int resy*/);
void gui_set_update(void (*update)(SDL_Surface *desk));
void gui_set_destroy(void (*destroy)(void));
void gui_redraw(WIDGET *base,SDL_bool show_mouse);
void gui_error_box(int x,int y,int w,int h,char *title,char *format, ...);
void gui_set_resize(SDL_bool (*resize)(int w,int h));

#endif
