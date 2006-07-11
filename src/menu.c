/*  gngeo, a neogeo emulator
 *  Copyright (C) 2001 Peponas Thomas & Peponas Mathieu
 * 
 *  This program is free software; you can redistribute it and/or modify  
 *  it under the terms of the GNU General Public License as published by   
 *  the Free Software Foundation; either version 2 of the License, or    
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. 
 */

#ifdef GP2X

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "menu.h"
#include "SDL.h"
#ifdef HAVE_LIBSDL_IMAGE
#include "SDL_image.h"
#endif
#include "messages.h"
#include "screen.h"
#include "video.h"
#include "gp2x.h"

static SDL_Surface *back;
static SDL_Surface *sfont;


#define MENU_TITLE_X (24 + 30)
#define MENU_TITLE_Y (16 + 20)

#define MENU_TEXT_X (24 + 30)
#define MENU_TEXT_Y (16 + 43)

static GN_MENU *main_menu;

#define COL32_TO_16(col) ((((col&0xff0000)>>19)<<11)|(((col&0xFF00)>>10)<<5)|((col&0xFF)>>3))
static void dr_putchar(SDL_Surface * dest,SDL_Surface *font,
		       int x, int y, unsigned char c)
{
    static SDL_Rect font_rect, dest_rect;
    int indice = c - 32;
    int font_w=font->w/(float)95;
    int font_h=font->h;

    if (c < 32 || c > 127)
	return;

    font_rect.x = indice * font_w;
    font_rect.y = 0;
    font_rect.w = font_w;
    font_rect.h = font_h;

    dest_rect.x = x;
    dest_rect.y = y;
    dest_rect.w = font_w;
    dest_rect.h = font_h;

    SDL_BlitSurface(font, &font_rect, dest, &dest_rect);

}
static void draw_string(SDL_Surface *s,SDL_Surface *font,int x,int y,char *string) {
    int i,xx=x;
    int font_w=font->w/(float)95;
    int font_h=font->h;

    if (string && s && font)
	    for (i = 0; i < strlen(string); i++) {
		    if (string[i]=='\n') {xx=x;y+=font_h;continue;}
		    dr_putchar(s,font, xx , y, string[i]);
		    xx+=font_w;
	    }

}

int gn_init_skin(void) {
	//back=IMG_Load("skin/back.png");
	back=SDL_LoadBMP("skin/back.bmp");
	sfont=SDL_LoadBMP("skin/font.bmp");
	SDL_SetColorKey(sfont,SDL_SRCCOLORKEY,SDL_MapRGB(sfont->format,0xFF,0,0xFF));

	if (!back) return 1;
	return 0;
}
void gn_init_pbar(char *name) {
	SDL_Rect dst_r={24,16,304,224};
	if (back) SDL_BlitSurface(back,NULL,buffer,&dst_r);
	else SDL_FillRect(buffer,NULL,COL32_TO_16(0xFAFEFB));

	//SDL_textout(buffer,46,36,name);
	draw_string(buffer,sfont,46,36,name);
}
void gn_popup_error(char *name,char *fmt,...) {
	char buf[512];
	va_list pvar;
	va_start(pvar,fmt);
	SDL_Event event;
	SDL_Rect dst_r={24,16,304,224};
	
	if (back) SDL_BlitSurface(back,NULL,buffer,&dst_r);
	else SDL_FillRect(buffer,NULL,COL32_TO_16(0xFE1111));

	//SDL_textout(buffer,46,36,name);
	draw_string(buffer,sfont,MENU_TITLE_X,MENU_TITLE_Y,name);

	vsnprintf(buf,511,fmt,pvar);
	//SDL_textout(buffer,62,62,buf);
	//SDL_textout(buffer,170,210,"Press any key ...");

	draw_string(buffer,sfont,MENU_TEXT_X,MENU_TEXT_Y,buf);
	draw_string(buffer,sfont,175+24,190+16,"Press any key ...");

	screen_update();



	while(1) {
		SDL_WaitEvent(&event);
		switch (event.type) {
			case SDL_JOYBUTTONDOWN:
				return;
				break;
		}
	}
}
int gn_popup_question(char *name,char *fmt,...) {
	char buf[512];
	va_list pvar;
	va_start(pvar,fmt);
	SDL_Event event;
	SDL_Rect dst_r={24,16,304,224};
	
	if (back) SDL_BlitSurface(back,NULL,buffer,&dst_r);
	else SDL_FillRect(buffer,NULL,COL32_TO_16(0x11A011));

	//SDL_textout(buffer,46,36,name);
	draw_string(buffer,sfont,MENU_TITLE_X,MENU_TITLE_Y,name);

	vsnprintf(buf,511,fmt,pvar);
	//SDL_textout(buffer,62,62,buf);
	//SDL_textout(buffer,170,210,"   B: Yes  X: No ");
	draw_string(buffer,sfont,MENU_TEXT_X,MENU_TEXT_Y,buf);
	draw_string(buffer,sfont,175+24,190+16,"   B: Yes  X: No");
	screen_update();



	while(1) {
		SDL_WaitEvent(&event);
		switch (event.type) {
		case SDL_JOYBUTTONDOWN:
			//joy_button[event.jbutton.which][event.jbutton.button] = 1;
			switch (event.jbutton.button) {
			case GP2X_B: return 1;
			case GP2X_X: return 0;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
}

#define NB_ITEM_2 4

static void draw_menu(GN_MENU *m) {
	SDL_Event event;
	SDL_Rect dst_r={24,16,304,224};
	int start,end,i;
	LIST *l=m->item;

	if (back) SDL_BlitSurface(back,NULL,buffer,&dst_r);
	else SDL_FillRect(buffer,NULL,COL32_TO_16(0x11A011));

	//if (m->title) SDL_textout(buffer,46,36,m->title);
	if (m->title) draw_string(buffer,sfont,MENU_TITLE_X,MENU_TITLE_Y,m->title);

	start=(m->current-NB_ITEM_2>=0?m->current-NB_ITEM_2:0);
	end=(m->current+NB_ITEM_2<m->nb_elem?m->current+NB_ITEM_2:m->nb_elem);
	

	for (i=0;i<start;i++,l=l->next);
	for (i=start;i<end;i++,l=l->next) {
		GN_MENU_ITEM *mi=(GN_MENU_ITEM *)l->data;
		//SDL_textout(buffer,62,62+(i*16),m->name);
		draw_string(buffer,sfont,MENU_TEXT_X+16,MENU_TEXT_Y+(i*sfont->h+2),mi->name);
		if (i==m->current) draw_string(buffer,sfont,MENU_TEXT_X,MENU_TEXT_Y+(i*sfont->h+2),">");
	}
	screen_update();
}

#undef NB_ITEM_2

GN_MENU_ITEM *gn_menu_create_item(char *name,Uint32 type,
				  int (*action)(GN_MENU_ITEM *self,void *param)) {
	GN_MENU_ITEM *t=malloc(sizeof(GN_MENU_ITEM));
	t->name=name;
	t->type=type;
	t->action=action;
	return t;
}

int test_action(GN_MENU_ITEM *self,void *param) {
	printf("Action!!\n");
	return 0;
}

void gn_init_menu(void) {
	main_menu=malloc(sizeof(GN_MENU));
	main_menu->title="Gngeo";
	main_menu->nb_elem=0;
	main_menu->current=0;
	main_menu->event_handling=NULL;
	main_menu->draw=draw_menu;
	main_menu->item=NULL;
	/* Create item */
	main_menu->item=list_append(main_menu->item,(void*)gn_menu_create_item("Load state",ACTION,test_action));
	main_menu->nb_elem++;
	main_menu->item=list_append(main_menu->item,(void*)gn_menu_create_item("Save state",ACTION,NULL));
	main_menu->nb_elem++;
}

Uint32 run_menu(void) {
	static Uint32 init=0;
	SDL_Event event;
	GN_MENU_ITEM *mi;
	LIST *l;

	if (init==0) {
		init=1;
		gn_init_menu();
	}
	main_menu->draw(main_menu);
	while(1) {
		SDL_WaitEvent(&event);
		switch (event.type) {
		case SDL_JOYBUTTONDOWN:
			switch (event.jbutton.button) {
			case GP2X_UP:
				if(main_menu->current>0) main_menu->current--;
				break;
			case GP2X_DOWN:
				if(main_menu->current<main_menu->nb_elem-1) main_menu->current++;
				break;
			case GP2X_X:
				return 0;
				break;
			case GP2X_B:
			case GP2X_A:
				l=list_get_item_by_index(main_menu->item, main_menu->current);
				if (l) {
					mi=(GN_MENU_ITEM *)l->data;
					if (mi->action) mi->action(mi,NULL);
				}
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
		main_menu->draw(main_menu);
	}
	return 0;
}

#endif
