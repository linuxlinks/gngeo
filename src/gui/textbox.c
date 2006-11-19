#include <stdlib.h>
#include <string.h>
#include "gui.h"
#include "style.h"

static int key_event(struct WIDGET* self,SDL_KeyboardEvent *event) {
    TEXTBOX *tx=GUI_TEXTBOX(self);
    int i;

    if (event->type==SDL_KEYDOWN) {
	switch(event->keysym.sym) {
	case SDLK_LEFT:
	    if (tx->cur_pos>0) tx->cur_pos--;
	    break;
	case SDLK_RIGHT:
	    if (tx->cur_pos<tx->text_size) tx->cur_pos++;
	    break;
	case SDLK_DELETE:
	    if (tx->cur_pos<tx->text_size) {
		for(i=tx->cur_pos;i<tx->text_size;i++)
		    tx->text[i]=tx->text[i+1];
		tx->text_size--;
	    }
	    break;
	case SDLK_BACKSPACE:
	    if (tx->cur_pos>0) {
		for(i=tx->cur_pos-1;i<tx->text_size;i++)
		    tx->text[i]=tx->text[i+1];
		tx->text_size--;
		tx->cur_pos--;
	    }
	    break;
	case SDLK_HOME:
	    tx->cur_pos=0;
	    break;
	case SDLK_END:
	    tx->cur_pos=tx->text_size;
	    break;
	default:
	    if ( (event->keysym.unicode & 0xFF80) == 0 ) {
		int a=(event->keysym.unicode & 0x7F);
		if (a>=32 && a<128 && tx->text_size+1<tx->max) {
		    for(i=tx->text_size;i>tx->cur_pos;i--)
			tx->text[i]=tx->text[i-1];
		    tx->text[tx->cur_pos]=(char)a;
		    tx->text_size++;
		    tx->cur_pos++;
		}
	    }
	    break;
	}
	
	
    }


    return TAKE_EVENT;
}

static void textbox_draw(struct WIDGET* self,SDL_Surface *desk) {
    /*
    SDL_Rect wrect={self->x,self->y,self->w,self->h};
    SDL_Rect clip_rect={self->x+2,self->y+2,self->w-4,self->h-4};
    */
    
    SDL_Rect clip_save;
   

    TEXTBOX *tx=GUI_TEXTBOX(self);
    SDL_Rect textrect=self->rect;
    Uint16 gui_font_w,gui_font_h;
    int y;
    /*
      int y;
      int x;
    */
    if (!WID_IS_MAPPED(self)) return;

    style_get_font_size(&gui_font_w,&gui_font_h);
    y=(self->rect.h-gui_font_h)/2;

    if (tx->cur_pos*gui_font_w<tx->begin) {
	tx->begin=tx->cur_pos*gui_font_w;
	tx->end=tx->cur_pos*gui_font_w+self->rect.w;
    }
    if (tx->cur_pos*gui_font_w>tx->end) {
	tx->end=tx->cur_pos*gui_font_w;
	tx->begin=tx->cur_pos*gui_font_w-self->rect.w;
    }
    
    //draw_border_color(desk,SHADOW_IN,&wrect,0xFFFFFF);

    /* set clipping for text */
    
    SDL_GetClipRect(desk,&clip_save);
    SDL_SetClipRect(desk,&self->rect);
    

    //draw_string(desk,gui_font,self->x+3-tx->begin,self->y+y,tx->text);
    style_drw_txtbg(desk,&self->rect);
    textrect.x-=tx->begin-style_get_border_w()-1;
    textrect.y+=y;
    textrect.w=style_strlen(tx->text);
    style_drw_text(desk,&textrect,tx->text);
    style_drw_frame(desk,&self->rect,FRAME_SHADOW_IN | FRAME_NO_BACK,NULL);

    if (WID_HAS_FOCUS(self)) {
	//draw_v_line(desk,(self->x+3-tx->begin)+tx->cur_pos*gui_font_w-1,self->y+y-1,gui_font_h+2,0x000000);
	style_drw_txt_cursor(desk,textrect.x,textrect.y,tx->cur_pos);
    }
    
    /* restore clipping */
    SDL_SetClipRect(desk,&clip_save);
}


TEXTBOX *gui_textbox_create(TEXTBOX *tx,Sint16 x,Sint16 y,Uint16 w,Uint16 h,Uint32 text_max) {
    TEXTBOX *t;
    if (tx)
	t=tx;
    else
	t=malloc(sizeof(TEXTBOX));

    gui_widget_create(&t->widget,x,y,w,h);
    t->text=malloc(text_max*sizeof(char));
    memset(t->text,0,text_max*sizeof(char));

    t->max=text_max;
    t->text_size=0;
    t->cur_pos=0;

    t->begin=0;
    t->end=w;

    GUI_WIDGET(t)->key=key_event;
//    GUI_WIDGET(t)->get_key_widget=textbox_get_key;
    GUI_WIDGET(t)->draw=textbox_draw;
    return t;
}

char *gui_textbox_get_text(TEXTBOX *self) {
    return self->text;
}

void gui_textbox_set_text(TEXTBOX *self,char *text) {
    snprintf(self->text,self->max,"%s",text);
}
