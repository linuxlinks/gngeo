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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "menu.h"
#include "SDL.h"
#ifdef HAVE_LIBSDL_IMAGE
#include "SDL_image.h"
#endif
#include "messages.h"
#include "screen.h"
#include "video.h"
#include "gp2x.h"
#include "state.h"
#include "emu.h"
#include "frame_skip.h"
#include "video.h"
#include "conf.h"
#include "driver.h"

static SDL_Surface *back;
static SDL_Surface *sfont;
static SDL_Surface *mfont;
static SDL_Surface *pbar;
static SDL_Surface *pbar_back;

#define MENU_BIG   0
#define MENU_SMALL 1

#define MENU_TITLE_X (24 + 30)
#define MENU_TITLE_Y (16 + 20)

#define MENU_TEXT_X (24 + 26)
#define MENU_TEXT_Y (16 + 43)

#define MENU_TEXT_X_END (24 + 277)
#define MENU_TEXT_Y_END (16 + 198)

#define ALIGN_LEFT   -1
#define ALIGN_RIGHT  -2
#define ALIGN_CENTER -3
#define ALIGN_UP   -1
#define ALIGN_DOWN  -2

static GN_MENU *main_menu;
static GN_MENU *rbrowser_menu;

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
    int i,xx;
    int font_w=font->w/(float)95;
    int font_h=font->h;

    if (x==ALIGN_LEFT) x=MENU_TEXT_X;
    if (x==ALIGN_RIGHT) x=(MENU_TEXT_X_END-font_w*strlen(string));
    if (x==ALIGN_CENTER) x=(MENU_TEXT_X+(MENU_TEXT_X_END-MENU_TEXT_X-font_w*strlen(string))/2);
    if (y==ALIGN_UP) y=MENU_TEXT_Y;
    if (y==ALIGN_DOWN) y=(MENU_TEXT_Y_END-font_h);
    if (y==ALIGN_CENTER) y=(MENU_TEXT_Y+(MENU_TEXT_Y_END-MENU_TEXT_Y-font_h)/2);
    xx=x;

    if (string && s && font)
	    for (i = 0; i < strlen(string); i++) {
		    //printf("%c\n",string[i]);
		    if (string[i]=='\n') {xx=x;y+=font_h;continue;}
		    if (string[i]==' ' && string[i+1]!=0x0) {
			    int len;
			    char *ind=index(&string[i+1],' ');
			    if (xx+font_w>MENU_TEXT_X_END) {xx=x;y+=font_h;continue;}
			    if (ind) {
				    len=((Uint32)ind-(Uint32)(&string[i+1]))*font_w;
				    
			    } else {
				    len=strlen(&string[i+1])*font_w;
				    
			    }
			    if (xx+len>MENU_TEXT_X_END-font_w) {xx=x;y+=font_h;continue;}
		    }
		    
		    // Check how many space we need for the next word
		    dr_putchar(s,font, xx , y, string[i]);
		    xx+=font_w;
	    }

}

static Uint32 string_len(SDL_Surface *font,char *string) {
	int font_w=font->w/(float)95;
	if (string) 
		return strlen(string)*font_w;
	else
		return 0;
}

static void draw_back(void) {
	SDL_Rect dst_r={24,16,304,224};
	
	if (back) SDL_BlitSurface(back,NULL,buffer,&dst_r);
	else {
		SDL_Rect r1={24+16,16+16,271,191};
		SDL_Rect r2={24+22,16+35,259,166};
		SDL_Rect r3={24+24,16+24,271,191};

		SDL_FillRect(buffer,&r3,COL32_TO_16(0x111111));
		SDL_FillRect(buffer,&r1,COL32_TO_16(0xE8E8E8));
		SDL_FillRect(buffer,&r2,COL32_TO_16(0x1C57A2));
	}

}

int gn_init_skin(void) {
	//back=IMG_Load("skin/back.png");
	back=SDL_LoadBMP("skin/back.bmp");
	sfont=SDL_LoadBMP("skin/font.bmp");
	mfont=SDL_LoadBMP("skin/font_x2.bmp");
	pbar=SDL_LoadBMP("skin/pbar.bmp");
	pbar_back=SDL_LoadBMP("skin/pbar_back.bmp");
	if (!sfont) sfont=fontbuf;
	if (!mfont) mfont=sfont;
	if (back) SDL_SetColorKey(back,SDL_SRCCOLORKEY,SDL_MapRGB(back->format,0xFF,0,0xFF));
	SDL_SetColorKey(sfont,SDL_SRCCOLORKEY,SDL_MapRGB(sfont->format,0xFF,0,0xFF));
	SDL_SetColorKey(mfont,SDL_SRCCOLORKEY,SDL_MapRGB(mfont->format,0xFF,0,0xFF));
	if (pbar) 
		SDL_SetColorKey(pbar,SDL_SRCCOLORKEY,SDL_MapRGB(pbar->format,0xFF,0,0xFF));
	if (pbar_back) 
		SDL_SetColorKey(pbar_back,SDL_SRCCOLORKEY,SDL_MapRGB(pbar_back->format,0xFF,0,0xFF));

	if (!back) return 1;
	return 0;
}

static int pbar_y;

void gn_reset_pbar(void) {
	pbar_y=0;
}
void gn_init_pbar(char *name) {
	int len=(sfont->w/95.0)*13;
	int w=MENU_TEXT_X_END-(MENU_TEXT_X+len);
	SDL_Rect r1={
		MENU_TEXT_X+len,MENU_TEXT_Y+sfont->h*pbar_y+1,
		w,sfont->h-2
	}; 
	if (pbar && pbar_back) {r1.w=pbar_back->w;r1.h=pbar_back->h;}
	if (pbar_y==0) {
		draw_back();
		draw_string(buffer,sfont,MENU_TITLE_X,MENU_TITLE_Y,"Loading ...");
	}
	draw_string(buffer,sfont,ALIGN_LEFT,MENU_TEXT_Y+sfont->h*pbar_y,name);
	if (pbar && pbar_back) 
		SDL_BlitSurface(pbar_back,NULL,buffer,&r1);
	else
		SDL_FillRect(buffer,&r1,COL32_TO_16(0xA11111));

	screen_update();
}
void gn_update_pbar(Uint32 pos,Uint32 size) {
	int len=(sfont->w/95.0)*13;
	int w=MENU_TEXT_X_END-(MENU_TEXT_X+len);
	SDL_Rect r1={
		MENU_TEXT_X+len,MENU_TEXT_Y+sfont->h*pbar_y+1,
		w,sfont->h-2
	}; 
	SDL_Rect r2={r1.x+1,r1.y+1,(pos*(r1.w-2))/size,r1.h-2};
	if (pbar && pbar_back) {
		SDL_Rect r3={0,0,(pos*pbar->w)/size,pbar->h};
		r1.w=pbar_back->w;r1.h=pbar_back->h;
		r2.w=(pos*pbar->w)/size;r2.h=pbar->h;
		//SDL_FillRect(buffer,&r1,COL32_TO_16(0x111111));
		SDL_BlitSurface(pbar_back,NULL,buffer,&r1);
		SDL_BlitSurface(pbar,&r3,buffer,&r2);
	} else {
		SDL_FillRect(buffer,&r1,COL32_TO_16(0x111111));
		SDL_FillRect(buffer,&r2,COL32_TO_16(0x1123AE));
	}


	screen_update();
}

void gn_terminate_pbar(void) {
	int len=(sfont->w/95.0)*13;
	int w=MENU_TEXT_X_END-(MENU_TEXT_X+len);
	SDL_Rect r1={
		MENU_TEXT_X+len,MENU_TEXT_Y+sfont->h*pbar_y+1,
		w,sfont->h-2
	};
	SDL_Rect r2={r1.x+1,r1.y+1,r1.w-2,r1.h-2};
	if (pbar && pbar_back) {
		r1.w=pbar_back->w;r1.h=pbar_back->h;
		r2.w=pbar->w;r2.h=pbar->h;
		//SDL_FillRect(buffer,&r1,COL32_TO_16(0x111111));
		SDL_BlitSurface(pbar_back,NULL,buffer,&r1);
		SDL_BlitSurface(pbar,NULL,buffer,&r2);
	} else {
		SDL_FillRect(buffer,&r1,COL32_TO_16(0x111111));
		SDL_FillRect(buffer,&r2,COL32_TO_16(0x1123AE));
	}

	pbar_y++;
	if (MENU_TEXT_Y+(pbar_y+1)*sfont->h>MENU_TEXT_Y_END) {
		pbar_y=0;
	}
	//printf(" %d %d\n",r2.w,r2.h);

	screen_update();

	
}
void gn_popup_error(char *name,char *fmt,...) {
	char buf[512];
	va_list pvar;
	va_start(pvar,fmt);
	SDL_Event event;

	draw_back();
	//SDL_textout(buffer,46,36,name);
	draw_string(buffer,sfont,MENU_TITLE_X,MENU_TITLE_Y,name);

	vsnprintf(buf,511,fmt,pvar);
	//SDL_textout(buffer,62,62,buf);
	//SDL_textout(buffer,170,210,"Press any key ...");

	draw_string(buffer,sfont,MENU_TEXT_X,MENU_TEXT_Y,buf);
	//draw_string(buffer,sfont,175+24,190+16,"Press any key ...");
	draw_string(buffer,sfont,ALIGN_RIGHT,ALIGN_DOWN,"Press any key ...");

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

	draw_back();

	//SDL_textout(buffer,46,36,name);
	draw_string(buffer,sfont,MENU_TITLE_X,MENU_TITLE_Y,name);

	vsnprintf(buf,511,fmt,pvar);
	//SDL_textout(buffer,62,62,buf);
	//SDL_textout(buffer,170,210,"   B: Yes  X: No ");
	draw_string(buffer,sfont,MENU_TEXT_X,MENU_TEXT_Y,buf);
	draw_string(buffer,sfont,ALIGN_RIGHT,ALIGN_DOWN,"B: Yes  X: No");
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

//#define NB_ITEM_2 4

static void draw_menu(GN_MENU *m) {
	SDL_Event event;
	int start,end,i;
	int nb_item;
	SDL_Surface *fnt;
	LIST *l=m->item;


	if (m->draw_type==MENU_BIG)
		fnt=mfont;
	else
		fnt=sfont;

	nb_item=(MENU_TEXT_Y_END-MENU_TEXT_Y)/fnt->h-1;

	draw_back();

	//if (m->title) SDL_textout(buffer,46,36,m->title);
	if (m->title) draw_string(buffer,sfont,MENU_TITLE_X,MENU_TITLE_Y,m->title);

	start=(m->current-nb_item>=0?m->current-nb_item:0);
	//end=(m->current+nb_item<m->nb_elem?m->current+nb_item:m->nb_elem);
	end=(start+nb_item<m->nb_elem?start+nb_item:m->nb_elem-1);
	//printf("%d %d %d %d\n",nb_item,start,end,m->current);

	for (i=0;i<start;i++,l=l->next);
	for (i=start;i<=end;i++,l=l->next) {
		GN_MENU_ITEM *mi=(GN_MENU_ITEM *)l->data;
		int j=i-start;
		//SDL_textout(buffer,62,62+(i*16),m->name);
		if (m->draw_type==MENU_BIG) {
			draw_string(buffer,fnt,ALIGN_CENTER,MENU_TEXT_Y+(j*fnt->h+2),mi->name);
			if (i==m->current) draw_string(buffer,fnt,ALIGN_CENTER,MENU_TEXT_Y+(j*fnt->h+2),">           <");
		} else {
			draw_string(buffer,fnt,MENU_TEXT_X+10,MENU_TEXT_Y+(j*fnt->h+2),mi->name);
			if (i==m->current) draw_string(buffer,fnt,MENU_TEXT_X,MENU_TEXT_Y+(j*fnt->h+2),">");
		}
	}
	screen_update();
}

//#undef NB_ITEM_2

GN_MENU_ITEM *gn_menu_create_item(char *name,Uint32 type,
				  int (*action)(GN_MENU_ITEM *self,void *param)) {
	GN_MENU_ITEM *t=malloc(sizeof(GN_MENU_ITEM));
	t->name=strdup(name);
	//t->name=name;
	t->type=type;
	t->action=action;
	return t;
}

int test_action(GN_MENU_ITEM *self,void *param) {
	printf("Action!!\n");
	return 0;
}


static int load_state_action(GN_MENU_ITEM *self,void *param) {
	static Uint32 slot=0;
	SDL_Rect dstrect={24+75,16+66,304/2,224/2};
	SDL_Rect dstrect_binding={24+73,16+64,304/2+4,224/2+4};
	//SDL_Rect dst_r={24,16,304,224};
	SDL_Event event;
	SDL_Surface *slot_img;
	char slot_str[32];

	Uint32 nb_slot=how_many_slot(conf.game);

	if (nb_slot==0) {
		gn_popup_info("Load State","There is currently no save state available");
		return 0; // nothing to do
	}

	slot_img=load_state_img(conf.game,slot);

	while(1) {
		//if (back) SDL_BlitSurface(back,NULL,buffer,&dst_r);
		//else SDL_FillRect(buffer,NULL,COL32_TO_16(0x11A011));
		draw_back();
		SDL_FillRect(buffer,&dstrect_binding,COL32_TO_16(0xFEFEFE));
		SDL_SoftStretch(slot_img, NULL,buffer, &dstrect);

		draw_string(buffer,sfont,MENU_TITLE_X,MENU_TITLE_Y,"Load State");
		sprintf(slot_str,"Slot Number %03d",slot);
		//draw_string(buffer,sfont,24+102,16+40,slot_str);
		draw_string(buffer,sfont,ALIGN_CENTER,ALIGN_UP,slot_str);

		if(slot>0) draw_string(buffer,mfont,ALIGN_LEFT,ALIGN_CENTER," <<");
		if (slot<nb_slot-1) draw_string(buffer,mfont,ALIGN_RIGHT,ALIGN_CENTER,">> ");
		
		screen_update();

		SDL_WaitEvent(&event);
		//while(SDL_PollEvent(&event));
		switch (event.type) {
		case SDL_JOYBUTTONDOWN:
			switch (event.jbutton.button) {
			case GP2X_LEFT:
				if (slot>0) slot--;
				slot_img=load_state_img(conf.game,slot);
				break;
			case GP2X_RIGHT:
				if(slot<nb_slot-1) slot++;
				slot_img=load_state_img(conf.game,slot);
				break;
			case GP2X_X:
				return 0;
				break;
			case GP2X_B:
			case GP2X_A:
				load_state(conf.game,slot);
				printf("Load state!!\n");
				return 1;
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
	return 0;
}
static int save_state_action(GN_MENU_ITEM *self,void *param) {
	static Uint32 slot=0;
	SDL_Rect dstrect={24+75,16+66,304/2,224/2};
	SDL_Rect dstrect_binding={24+73,16+64,304/2+4,224/2+4};
	//SDL_Rect dst_r={24,16,304,224};
	SDL_Event event;
	SDL_Surface *slot_img;
	char slot_str[32];
	Uint32 nb_slot=how_many_slot(conf.game);

	if (nb_slot!=0) slot_img=load_state_img(conf.game,slot);

	while(1) {
		//if (back) SDL_BlitSurface(back,NULL,buffer,&dst_r);
		//else SDL_FillRect(buffer,NULL,COL32_TO_16(0x11A011));
		draw_back();
		if (slot!=nb_slot) {
			SDL_FillRect(buffer,&dstrect_binding,COL32_TO_16(0xFEFEFE));
			SDL_SoftStretch(slot_img, NULL,buffer, &dstrect);
		} else {
			SDL_FillRect(buffer,&dstrect_binding,COL32_TO_16(0xFEFEFE));
			SDL_FillRect(buffer,&dstrect,COL32_TO_16(0xA0B0B0));
			//draw_string(buffer,sfont,24+102,16+107,"Create a new Slot");
			draw_string(buffer,sfont,ALIGN_CENTER,ALIGN_CENTER,"Create a new Slot");
		}

		draw_string(buffer,sfont,MENU_TITLE_X,MENU_TITLE_Y,"Save State");
		sprintf(slot_str,"Slot Number %03d",slot);
		draw_string(buffer,sfont,ALIGN_CENTER,ALIGN_UP,slot_str);
		if(slot>0) draw_string(buffer,mfont,ALIGN_LEFT,ALIGN_CENTER," <<");
		if (slot<nb_slot) draw_string(buffer,mfont,ALIGN_RIGHT,ALIGN_CENTER,">> ");

		screen_update();

		SDL_WaitEvent(&event);
		//while(SDL_PollEvent(&event));
		switch (event.type) {
		case SDL_JOYBUTTONDOWN:
			switch (event.jbutton.button) {
			case GP2X_LEFT:
				if (slot>0) slot--;
				if (slot!=nb_slot) slot_img=load_state_img(conf.game,slot);
				break;
			case GP2X_RIGHT:
				if(slot<nb_slot) slot++;
				if (slot!=nb_slot) slot_img=load_state_img(conf.game,slot);
				break;
			case GP2X_X:
				return 0;
				break;
			case GP2X_B:
			case GP2X_A:
				save_state(conf.game,slot);
				printf("Save state!!\n");
				return 1;
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
	return 0;
}


static int old_save_state_action(GN_MENU_ITEM *self,void *param) {
	save_state(conf.game,0);
	printf("Save state!!\n");
	return 0;
}
static int exit_action(GN_MENU_ITEM *self,void *param) {
	//exit(0);
	return 2;
}

int menu_event_handling(struct GN_MENU *self) {
	static SDL_Event event;
	GN_MENU_ITEM *mi;
	int a;
	LIST *l;

	SDL_WaitEvent(&event);
	//SDL_PollEvent(&event);
	//while(SDL_PollEvent(&event));

	switch (event.type) {
	case SDL_JOYBUTTONDOWN:
		switch (event.jbutton.button) {
		case GP2X_UP:
		case GP2X_UP_LEFT:
		case GP2X_UP_RIGHT:
			if(self->current>0) 
				self->current--;
			else
				self->current=self->nb_elem-1;
			break;
		case GP2X_DOWN:
		case GP2X_DOWN_LEFT:
		case GP2X_DOWN_RIGHT:
			if(self->current<self->nb_elem-1) 
				self->current++;
			else
				self->current=0;
			break;
		case GP2X_L:
			self->current-=10;
			if (self->current<0) self->current=0;
			break;
		case GP2X_R:
			self->current+=10;
			if (self->current>=self->nb_elem) self->current=self->nb_elem-1;
			break;
		case GP2X_X:
			return 0;
			break;
		case GP2X_B:
		case GP2X_A:
			l=list_get_item_by_index(self->item, self->current);
			if (l) {
				mi=(GN_MENU_ITEM *)l->data;
				if (mi->action) 
					if ((a=mi->action(mi,NULL))) return a;
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return -1;
}
int icasesort(const struct dirent **a, const struct dirent **b) {
	char *ca=(*a)->d_name;
	char *cb=(*b)->d_name;
	return strcasecmp(ca,cb);
}


void init_rom_browser_menu(void) {
	int i; char buf[35];
	int nbf;
	char filename[strlen(CF_STR(cf_get_item_by_name("rompath")))+256];
	struct stat filestat;
	struct dirent **namelist;
	//char name[32];
	
	rbrowser_menu=malloc(sizeof(GN_MENU));
	rbrowser_menu->title="Load Game";
	rbrowser_menu->nb_elem=0;
	rbrowser_menu->current=0;
	rbrowser_menu->draw_type=MENU_SMALL;
	rbrowser_menu->event_handling=menu_event_handling;
	rbrowser_menu->draw=draw_menu;
	rbrowser_menu->item=NULL;

	if ((nbf = scandir(CF_STR(cf_get_item_by_name("rompath")), &namelist, NULL, icasesort )) != -1) {
		for (i = 0; i < nbf; i++) {
			sprintf(filename, "%s/%s", CF_STR(cf_get_item_by_name("rompath")), namelist[i]->d_name);
			lstat(filename, &filestat);
			if (!S_ISDIR(filestat.st_mode)) {
				DRIVER *dr;
				if ((dr=get_driver_for_zip(filename))!=NULL) {
					printf("%8s:%s:%s\n",dr->name,dr->longname,namelist[i]->d_name);
					if (dr->longname) {
						/* 
						if (strlen(dr->longname)>24)
							sprintf(filename,"%-.27s...",dr->longname);
						else
							sprintf(filename,"%-.27s",dr->longname);
						*/
						sprintf(filename,"%-.27s",dr->longname);
						strtok(filename,"/(");
					} else
						sprintf(filename,"%s",dr->name);
					rbrowser_menu->item=list_append(rbrowser_menu->item,
									(void*)gn_menu_create_item(filename,ACTION,NULL));
					rbrowser_menu->nb_elem++;

				}
			}
		}
	}

	rbrowser_menu->item=list_append(rbrowser_menu->item,
					(void*)gn_menu_create_item("Nothing yet",ACTION,NULL));
	rbrowser_menu->nb_elem++;

}

int rom_browser_menu(void) {
	static Uint32 init=0;
	int a;
	
	if (init==0) {
		init=1;
		
		draw_back();
		draw_string(buffer,sfont,MENU_TITLE_X,MENU_TITLE_Y,"Scanning ....");
		screen_update();
		
		init_rom_browser_menu();
	}
	while(1) {
		rbrowser_menu->draw(rbrowser_menu);
		if ((a=rbrowser_menu->event_handling(rbrowser_menu))>=0)
			return a;
	}
}

static int rbrowser_action(GN_MENU_ITEM *self,void *param) {
	//exit(0);
	return rom_browser_menu();
}

void gn_init_menu(void) {
	int i; char buf[24];

	main_menu=malloc(sizeof(GN_MENU));
	main_menu->title="Gngeo";
	main_menu->nb_elem=0;
	main_menu->current=0;
	main_menu->draw_type=MENU_BIG;
	main_menu->event_handling=menu_event_handling;
	main_menu->draw=draw_menu;
	main_menu->item=NULL;
	/* Create item */
/*
	main_menu->item=list_append(main_menu->item,(void*)gn_menu_create_item("Load game",ACTION,rbrowser_action));
	main_menu->nb_elem++;
*/
	main_menu->item=list_append(main_menu->item,(void*)gn_menu_create_item("Load state",ACTION,load_state_action));
	main_menu->nb_elem++;
	main_menu->item=list_append(main_menu->item,(void*)gn_menu_create_item("Save state",ACTION,save_state_action));
	main_menu->nb_elem++;
	main_menu->item=list_append(main_menu->item,(void*)gn_menu_create_item("Exit",ACTION,exit_action));
	main_menu->nb_elem++;
/*
	for (i=0;i<10;i++) {
		sprintf(buf,"Plop %d",i);
		main_menu->item=list_append(main_menu->item,(void*)gn_menu_create_item(buf,ACTION,NULL));
		main_menu->nb_elem++;
	}
*/
}

Uint32 run_menu(void) {
	static Uint32 init=0;
	//SDL_Event event;
	int a;

	//while (SDL_PollEvent(&event));

	if (init==0) {
		init=1;
		gn_init_menu();
	}
	//main_menu->draw(main_menu);
	

	while(1) {
		main_menu->draw(main_menu);
		if ((a=main_menu->event_handling(main_menu))>=0)
			return a;
	}
	//while (SDL_PollEvent(&event));
	return 0;
}

#endif
