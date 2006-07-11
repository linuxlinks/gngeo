#include "style.h"
#include "thin.h" /* default style */

/* style definition */
STYLE gui_style[] = { 
    {"thin",thin_init,thin_close},
    {NULL,NULL,NULL},
};

static STYLE *cur_style;

static STYLE *get_style_by_name(char *name) {
    int i;
    if (name==NULL) return &gui_style[0];

    for(i=0;gui_style[i].name!=NULL;i++)
	if (strcmp(name,gui_style[i].name)!=0) 
	    return &gui_style[i];

    return &gui_style[0];
}

SDL_bool style_is_init(void) {
    if (cur_style)
	return SDL_TRUE;
    return SDL_FALSE;
}

SDL_bool style_init(char *name,char *datapath) {
    STYLE *s=get_style_by_name(name);
    
    if (cur_style)
	cur_style->close();

    if (s->init(datapath)==SDL_TRUE) {
	cur_style=s;
	return SDL_TRUE;
    }
    cur_style=NULL;
    return SDL_FALSE;
}

Uint16 style_get_font_h(void) {
    Uint16 w,h;
    style_get_font_size(&w,&h);
    return h;
}

Uint16 style_get_font_w(void) {
    Uint16 w,h;
    style_get_font_size(&w,&h);
    return w;
}

Uint16 style_get_border_h(void) {
    Uint16 w,h;
    style_get_border_size(&w,&h);
    return h;
}

Uint16 style_get_border_w(void) {
    Uint16 w,h;
    style_get_border_size(&w,&h);
    return w;
}

Uint32 style_strlen(char *str) {
    if (str && cur_style) {
	Uint16 fw,fh;
	style_get_font_size(&fw,&fh);
	return strlen(str)*fw;
    }
    return 0;
}

void style_close(void) {
    if (cur_style)
	cur_style->close();
}
