#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "SDL.h"
#include "gui.h"
#include "widget.h"
#include "style.h"
#include "../list.h"

static int mouse_x,mouse_y;
static int first_clicked;

static void (*update_display)(SDL_Surface *desk);
static void (*gui_exit_app)(void);
static SDL_bool (*resize_display)(int w,int h);

SDL_Surface *desk_buf,*desk_save;
SDL_mutex *disp_mut;

/*
SDL_Surface *gui_font;

int gui_font_w;
int gui_font_h;
*/

static void internal_exit(void) {
    exit(0);
}

void gui_init(int x,int y,char *data_path) {
    if (!desk_buf) {
	desk_buf=SDL_CreateRGBSurface(SDL_SWSURFACE,x, y, 16, 0xF800, 0x7E0, 0x1F, 0);
	desk_save=SDL_CreateRGBSurface(SDL_SWSURFACE,x, y, 16, 0xF800, 0x7E0, 0x1F, 0);
    }
    /* init the default style if necessary */
    if (style_is_init()==SDL_FALSE) 
	style_init(NULL,data_path);
    
    gui_exit_app=internal_exit;

    SDL_InitSubSystem(SDL_INIT_TIMER);
    disp_mut=SDL_CreateMutex();
}

Uint32 double_click_timer(Uint32 interval, void *param) {
    first_clicked=0;
    return 0;
}

int quit_dialog(struct WIDGET* self,SDL_MouseButtonEvent *event) {
    if (event->type==SDL_MOUSEBUTTONUP) return QUIT_EVENT;
    return TAKE_EVENT;
}

/* show a dialog boy (the message MUST end with a \n) */
void gui_error_box(int x,int y,int w,int h,char *title,char *format, ...) {
    char *buf=alloca(512*sizeof(char));
    WIDGET *dialog,*label,*title_label,*button,*frame;
    char *t,*ot;
    int ypos=3;
    va_list pvar;
    va_start(pvar,format);

    vsnprintf(buf,511,format,pvar);
    dialog=(WIDGET*)gui_frame_create(NULL,x,y,w,h,FRAME_SHADOW_OUT,NULL);
    title_label=(WIDGET*)gui_label_create(NULL,3,3,1,1,title);
    gui_container_add(GUI_CONTAINER(dialog),title_label);

    frame=(WIDGET*)gui_frame_create(NULL,2,4+style_get_font_h(),
				    w-4,h-31,FRAME_SHADOW_ETCHED_IN,NULL);
    gui_container_add(GUI_CONTAINER(dialog),frame);

    ot=buf;
    while((t=strchr(ot,'\n'))!=NULL) {
	t[0]=0;t++;
	if (style_strlen(ot)>w-5) {
	    int pos=(w-5)/(float)style_get_font_w()-4;
	    ot[pos]='.';ot[pos+1]='.';ot[pos+2]='.';ot[pos+3]=0;
	}
	label=(WIDGET*)gui_label_create(NULL,3,ypos,1,1,ot);
	gui_container_add(GUI_CONTAINER(frame),label);
	ot=t;
	ypos+=style_get_font_h()+1;
    }
    button=(WIDGET*)gui_button_label_create(NULL,w-42,h-15,40,12,"Ok");
    button->click=quit_dialog;
    gui_container_add(GUI_CONTAINER(dialog),button);
    dialog->map(dialog);
    gui_main_loop(dialog);

    /* clean up */
    gui_widget_destroy_all(dialog);
}

void gui_redraw(WIDGET *base,SDL_bool show_mouse) {
    SDL_Rect curs_rect={mouse_x,mouse_y,16,16};

    srand(0);

    /* gui_redraw can be call from different thread (mainly in scrollbar code) */
    SDL_mutexP(disp_mut);
//    SDL_SetClipRect(desk_buf,NULL);

    SDL_BlitSurface(desk_save,NULL,desk_buf,NULL);

    base->draw(base,desk_buf);

    if (show_mouse) style_drw_cursor(desk_buf,&curs_rect);
    update_display(desk_buf);
    SDL_mutexV(disp_mut);
}

/* 
   return the widget that have the mouse over itself
*/
WIDGET *update_mouse_focus(WIDGET *base,WIDGET *focused,int mx,int my) {
    WIDGET *t=focused;
    WIDGET *wid;

    wid=base->widget_at_pos(base,mouse_x,mouse_y);
    
    if (wid!=focused) {
	if (focused) {
	    //printf("%p lose mouse\n",focused);
	    focused->flags&=(~FLAG_MOUSEOVER);
	    //focused->m_state=0;
	    focused->lose_mouse(focused);
	}
	if (wid) {
	    //printf("%p gain mouse\n",wid);
	    wid->gain_mouse(wid);
	    //wid->have_focus=SDL_TRUE;
	    wid->flags|=FLAG_MOUSEOVER;
	}
	focused=wid;
    }
    return focused;
}

WIDGET *update_key_focus(WIDGET *base,WIDGET *focused) {
    WIDGET *t=focused;
    WIDGET *wid;

    wid=gui_widget_get_next_keyfocus(base);
    
    if (wid!=focused) {
	if (focused) {
	    //focused->have_focus=SDL_FALSE;
	    focused->flags&=(~FLAG_FOCUS);
	    //focused->m_state=0;
	    focused->lose_focus(focused);
	}
	if (wid) {
	    wid->gain_focus(wid);
	    //wid->have_focus=SDL_TRUE;
	    wid->flags|=FLAG_FOCUS;
	}
	focused=wid;
    }
    return focused;
}


/*
WIDGET *gui_first_key_widget(WIDGET *wid) {
    WIDGET *w=wid->get_key_widget(wid);
    printf("First widget = %p\n",w);
    return w;
}

WIDGET *gui_next_key_widget(WIDGET *wid) {
    WIDGET *w=wid->get_key_widget(wid);
    printf("Next widget = %p\n",w);
    return w;
}
*/

void gui_main_loop(WIDGET *base/*,int resx,int resy*/) {
    SDL_Event event;
    Uint8 gui_done=0;
    Uint32 nb_event;
    WIDGET *mouse_focused=NULL; /* mouse focus */
    static WIDGET *key_focused=NULL; /* key focus */
    if (base==NULL)
	return;

    SDL_BlitSurface(desk_buf,NULL,desk_save,NULL);
    gui_redraw(base,SDL_TRUE);

    SDL_EnableUNICODE(1);
    mouse_focused=base->widget_at_pos(base,mouse_x,mouse_y);
    key_focused=base->key_focus->data;
    if (!key_focused)
	key_focused=update_key_focus(base,NULL);
/*
    key_focused=gui_widget_get_next_keyfocus(base);
    if (key_focused) {
	key_focused->flags|=FLAG_FOCUS;
    }
*/

    while (gui_done!=1) {
	if (SDL_WaitEvent(&event) /*&& gui_done!=1*/) 
	{
	    if (event.type == SDL_QUIT) {
		//printf("QUIIT\n");
		gui_exit_app();
		exit(0);
	    }
	    nb_event=0;
	    while(SDL_PollEvent(&event)) {
		if (event.type!=SDL_MOUSEMOTION)
		    break;
	    }
	    switch (event.type) {
	    case SDL_VIDEORESIZE:
		if (resize_display) resize_display(event.resize.w, event.resize.h);
		break;
	    case SDL_KEYDOWN:
		if (event.key.keysym.sym==SDLK_ESCAPE) {
		    gui_done=1;
		    break;
		}
		if ( event.key.keysym.sym==SDLK_TAB) {
		    key_focused=update_key_focus(base,key_focused);
		    break;
		}
		if (key_focused) {
		    WIDGET *t=key_focused;
		    int e=PASS_EVENT;
		    while(t && ((e=t->key(t,&event.key))==PASS_EVENT))
			t=t->father;
		    //printf("Res: %d\n",e);
		    if (e==QUIT_EVENT) gui_done=1;
		} else { /* if no key focus list was setup, we use mouse focus */
		    WIDGET *t=mouse_focused;
		    int e=PASS_EVENT;
		    while(t && ((e=t->key(t,&event.key))==PASS_EVENT))
			t=t->father;
		    if (e==QUIT_EVENT) gui_done=1;
		}
		break;
	    case SDL_MOUSEMOTION: {
		WIDGET *wid;
		WIDGET *t=mouse_focused;
		int e=PASS_EVENT;
/*
		if (event.motion.x>=desk_buf->w) SDL_WarpMouse(desk_buf->w,event.motion.y);
		if (event.motion.y>=desk_buf->h) SDL_WarpMouse(event.motion.x,desk_buf->h);
*/
		/* recalc relative mouse movement since we don't handle
		   all the event.
		*/

		event.motion.xrel=event.motion.x-mouse_x;
		event.motion.yrel=event.motion.y-mouse_y;

		/*curs_rect.x=*/mouse_x=(event.motion.x);//*desk_buf->w)/(float)resx;
		/*curs_rect.y=*/mouse_y=(event.motion.y);//*desk_buf->h)/(float)resy;
		
		//printf("mouse: %d %d\n",mouse_x,mouse_y);
		
		event.motion.x=mouse_x;
		event.motion.y=mouse_y;
		//event.motion.xrel=(event.motion.xrel*desk_buf->w)/(float)resx;
		//event.motion.yrel=(event.motion.yrel*desk_buf->h)/(float)resy;
		while(t && ((e=t->mouse(t,&event.motion))==PASS_EVENT))
		    t=t->father;
		if (e==QUIT_EVENT) gui_done=1;
		
		/* don't change the focus if a button is pressed */
		if (mouse_focused && WID_HAS_MOUSECLICK(mouse_focused)) break; 
		mouse_focused=update_mouse_focus(base,mouse_focused,mouse_x,mouse_y);
	    }
	    break;
	    case SDL_MOUSEBUTTONDOWN:
	    case SDL_MOUSEBUTTONUP: {
		WIDGET *t=mouse_focused;
		int e=PASS_EVENT;

		/* recalc focus */
		mouse_focused=update_mouse_focus(base,mouse_focused,mouse_x,mouse_y);

		event.button.x=mouse_x;
		event.button.y=mouse_y;

		if (event.type==SDL_MOUSEBUTTONUP && first_clicked==event.button.button) {
		    while(t && ((e=t->double_click(t,&event.button))==PASS_EVENT))
			t=t->father;
		    first_clicked=0;
		} else {
		    while(t && ((e=t->click(t,&event.button))==PASS_EVENT))
			t=t->father;
		    if (event.type==SDL_MOUSEBUTTONUP) first_clicked=event.button.button;
		    SDL_AddTimer(450,double_click_timer,NULL);
		}
		
		if (t) {
		    if (event.button.state == SDL_PRESSED)
			t->flags|=FLAG_MOUSECLICK;
		    else
			t->flags&=(~FLAG_MOUSECLICK);
		    /*
		    Uint8 state=t->m_state;
		    switch (event.button.button) {
		    case SDL_BUTTON_LEFT:
			if (event.button.state == SDL_PRESSED) 
			    state|=0x1;
			else
			    state&=(~0x1);
			break;
		    case SDL_BUTTON_MIDDLE:
			if (event.button.state == SDL_PRESSED) 
			    state|=0x2;
			else
			    state&=(~0x2);
			break;
		    case SDL_BUTTON_RIGHT:
			if (event.button.state == SDL_PRESSED) 
			    state|=0x4;
			else
			    state&=(~0x4);
			break;
		    }
		    //printf("state=%d\n",state);
		    t->m_state=state;
		    */
		}
		if (e==QUIT_EVENT) gui_done=1;
	    }
	    break;
	    }
	
	    /* Redraw.. */
	    gui_redraw(base,SDL_TRUE);
	    
	    
	}
    }
    SDL_EnableUNICODE(0);
}

void gui_set_update(void (*update)(SDL_Surface *desk)) {
    update_display=update;
}

void gui_set_destroy(void (*destroy)(void)) {
    gui_exit_app=destroy;
}

void gui_set_resize(SDL_bool (*resize)(int w,int h)) {
    resize_display=resize;
}
