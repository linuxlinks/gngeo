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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>

#include "menu.h"
#include "SDL.h"
#include "messages.h"
#include "screen.h"
#include "video.h"
#include "event.h"
#include "state.h"
#include "emu.h"
#include "frame_skip.h"
#include "video.h"
#include "conf.h"
#include "resfile.h"
#include "fileio.h"
#include "sound.h"


typedef struct GNFONT {
	SDL_Surface *bmp;
	Uint8 ysize;
	Uint16 xpos[128 - 32];
	Uint8 xsize[128 - 32];
	Sint8 pad;
} GNFONT;

static SDL_Surface *menu_buf;
static SDL_Surface *back;
static GNFONT *sfont;
static GNFONT *mfont;
static SDL_Surface *gngeo_logo;

static SDL_Surface *arrow_l, *arrow_r;
static int interp;

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

static char *romlist[] = {
	"2020bb",
	"2020bba",
	"2020bbh",
	"3countb",
	"alpham2",
	"androdun",
	"aodk",
	"aof",
	"aof2",
	"aof2a",
	"aof3",
	"aof3k",
	"bakatono",
	"bangbead",
	"bjourney",
	"blazstar",
	"breakers",
	"breakrev",
	"bstars",
	"bstars2",
	"burningf",
	"burningfh",
	"crsword",
	"ct2k3sa",
	"ct2k3sp",
	"cthd2003",
	"ctomaday",
	"cyberlip",
	"diggerma",
	"doubledr",
	"eightman",
	"fatfursa",
	"fatfursp",
	"fatfury1",
	"fatfury2",
	"fatfury3",
	"fbfrenzy",
	"fightfev",
	"fightfeva",
	"flipshot",
	"fswords",
	"galaxyfg",
	"ganryu",
	"garou",
	"garoubl",
	"garouo",
	"garoup",
	"ghostlop",
	"goalx3",
	"gowcaizr",
	"gpilots",
	"gpilotsh",
	"gururin",
	"irrmaze",
	"janshin",
	"jockeygp",
	"joyjoy",
	"kabukikl",
	"karnovr",
	"kf10thep",
	"kf2k2mp",
	"kf2k2mp2",
	"kf2k2pla",
	"kf2k2pls",
	"kf2k3bl",
	"kf2k3bla",
	"kf2k3pcb",
	"kf2k3pl",
	"kf2k3upl",
	"kf2k5uni",
	"kizuna",
	"kof10th",
	"kof2000",
	"kof2000n",
	"kof2001",
	"kof2001h",
	"kof2002",
	"kof2002b",
	"kof2003",
	"kof2003h",
	"kof2k4se",
	"kof94",
	"kof95",
	"kof95h",
	"kof96",
	"kof96h",
	"kof97",
	"kof97a",
	"kof97pls",
	"kof98",
	"kof98k",
	"kof98n",
	"kof99",
	"kof99a",
	"kof99e",
	"kof99n",
	"kof99p",
	"kog",
	"kotm",
	"kotm2",
	"kotmh",
	"lans2004",
	"lastblad",
	"lastbladh",
	"lastbld2",
	"lastsold",
	"lbowling",
	"legendos",
	"lresort",
	"magdrop2",
	"magdrop3",
	"maglord",
	"maglordh",
	"mahretsu",
	"marukodq",
	"matrim",
	"matrimbl",
	"miexchng",
	"minasan",
	"mosyougi",
	"ms4plus",
	"ms5pcb",
	"ms5plus",
	"mslug",
	"mslug2",
	"mslug3",
	"mslug3b6",
	"mslug3h",
	"mslug3n", /* Driver don't have the good name */
	"mslug4",
	"mslug5",
	"mslug5h",
	"mslugx",
	"mutnat",
	"nam1975",
	"ncombat",
	"ncombath",
	"ncommand",
	"neobombe",
	"neocup98",
	"neodrift",
	"neomrdo",
	"ninjamas",
	"nitd",
	"nitdbl",
	"overtop",
	"panicbom",
	"pbobbl2n",
	"pbobblen",
	"pbobblena",
	"pgoal",
	"pnyaa",
	"popbounc",
	"preisle2",
	"pspikes2",
	"pulstar",
	"puzzldpr",
	"puzzledp",
	"quizdai2",
	"quizdais",
	"quizkof",
	"ragnagrd",
	"rbff1",
	"rbff1a",
	"rbff2",
	"rbff2h",
	"rbff2k",
	"rbffspec",
	"ridhero",
	"ridheroh",
	"roboarmy",
	"rotd",
	"s1945p",
	"samsh5sp",
	"samsh5sph",
	"samsh5spn", /* Driver don't have the good name */
	"samsho",
	"samsho2",
	"samsho3",
	"samsho3h",
	"samsho4",
	"samsho5",
	"samsho5b",
	"samsho5h",
	"samshoh",
	"savagere",
	"sdodgeb",
	"sengokh",
	"sengoku",
	"sengoku2",
	"sengoku3",
	"shocktr2",
	"shocktra",
	"shocktro",
	"socbrawl",
	"socbrawla",
	"sonicwi2",
	"sonicwi3",
	"spinmast",
	"ssideki",
	"ssideki2",
	"ssideki3",
	"ssideki4",
	"stakwin",
	"stakwin2",
	"strhoop",
	"superspy",
	"svc",
	"svcboot",
	"svcpcb",
	"svcpcba",
	"svcplus",
	"svcplusa",
	"svcsplus",
	"tophuntr",
	"tophuntra",
	"tpgolf",
	"trally",
	"turfmast",
	"twinspri",
	"tws96",
	"viewpoin",
	"vliner",
	"vlinero",
	"wakuwak7",
	"wh1",
	"wh1h",
	"wh1ha",
	"wh2",
	"wh2j",
	"wh2jh",
	"whp",
	"wjammers",
	"zedblade",
	"zintrckb",
	"zupapa",
	NULL
};

#define COL32_TO_16(col) ((((col&0xff0000)>>19)<<11)|(((col&0xFF00)>>10)<<5)|((col&0xFF)>>3))

GNFONT *load_font(char *file) {
	GNFONT *ft = malloc(sizeof (GNFONT));
	int i;
	int x = 0;
	Uint32 *b;
	if (!ft) return NULL;

	ft->bmp = res_load_stbi(file);
	if (!ft->bmp) {
		free(ft);
		return NULL;
	}
	//SDL_SetAlpha(ft->bmp,SDL_SRCALPHA,128);
	b = ft->bmp->pixels;
	//ft->bmp->format->Amask=0xFF000000;
	//ft->bmp->format->Ashift=24;
	//printf("shift=%d %d\n",ft->bmp->pitch,ft->bmp->w*4);
	if (ft->bmp->format->BitsPerPixel != 32) {
		printf("Unsupported font (bpp=%d)\n", ft->bmp->format->BitsPerPixel);
		SDL_FreeSurface(ft->bmp);
		free(ft);
		return NULL;
	}
	ft->xpos[0] = 0;
	for (i = 0; i < ft->bmp->w; i++) {
		//printf("%08x\n",b[i]);
		if (b[i] != b[0]) {
			ft->xpos[x + 1] = i + 1;
			if (x > 0)
				ft->xsize[x] = i - ft->xpos[x];
			else
				ft->xsize[x] = i;
			x++;
		}
	}
	//printf("NB char found:%d\n",x);
	if (x <= 0 || x > 95) return NULL;
	/*	b=ft->bmp->pixels+ft->bmp->pitch*3;
		for (i=0;i<ft->bmp->w;i++) {
			//printf("%08x\n",b[i]);
		}
	 */
	ft->xsize[94] = ft->bmp->w - ft->xpos[94];
	ft->ysize = ft->bmp->h;

	/* Default y padding=0 */
	ft->pad = 0;
	return ft;
}

static Uint32 string_len(GNFONT *f, char *str) {
	int i;
	int size = 0;
	if (str) {
		for (i = 0; i < strlen(str); i++) {
			switch (str[i]) {
				case ' ':
					size += f->xsize[0];
					break;
				case '\t':
					size += (f->xsize[0]*8);
					break;
				default:
					size += (f->xsize[(int) str[i] - 32] + f->pad);
					break;
			}
		}
		return size;
	} else
		return 0;
}

void draw_string(SDL_Surface *dst, GNFONT *f, int x, int y, char *str) {
	SDL_Rect srect, drect;
	int i;

	if (x == ALIGN_LEFT) x = MENU_TEXT_X;
	if (x == ALIGN_RIGHT) x = (MENU_TEXT_X_END - string_len(f, str));
	if (x == ALIGN_CENTER) x = (MENU_TEXT_X + (MENU_TEXT_X_END - MENU_TEXT_X - string_len(f, str)) / 2);
	if (y == ALIGN_UP) y = MENU_TEXT_Y;
	if (y == ALIGN_DOWN) y = (MENU_TEXT_Y_END - f->ysize);
	if (y == ALIGN_CENTER) y = (MENU_TEXT_Y + (MENU_TEXT_Y_END - MENU_TEXT_Y - f->ysize) / 2);

	drect.x = x;
	drect.y = y;
	drect.w = 32;
	drect.h = f->bmp->h;
	srect.h = f->bmp->h;
	srect.y = 0;
	for (i = 0; i < strlen(str); i++) {
		switch (str[i]) {
			case ' ': /* Optimize space */
				drect.x += f->xsize[0];
				break;
			case '\t':
				drect.x += (f->xsize[0]*8);
				break;
			case '\n':
				drect.x = x;
				drect.y += f->bmp->h;
				break;
			default:
				srect.x = f->xpos[(int) str[i] - 32];
				srect.w = f->xsize[(int) str[i] - 32];


				SDL_BlitSurface(f->bmp, &srect, dst, &drect);
				drect.x += (f->xsize[(int) str[i] - 32] + f->pad);
				break;
		}
	}
}

static void draw_back(void) {
	SDL_Rect dst_r = {24, 16, 304, 224};
	static SDL_Rect screen_rect = {0, 0, 304, 224};
	if (back) {
		//SDL_BlitSurface(buffer,NULL,menu_buf,NULL);
		SDL_BlitSurface(state_img, &screen_rect, menu_buf, &dst_r);
		SDL_BlitSurface(back, NULL, menu_buf, &dst_r);
	} else {
		SDL_Rect r1 = {24 + 16, 16 + 16, 271, 191};
		SDL_Rect r2 = {24 + 22, 16 + 35, 259, 166};
		SDL_Rect r3 = {24 + 24, 16 + 24, 271, 191};

		SDL_FillRect(menu_buf, &r3, COL32_TO_16(0x111111));
		SDL_FillRect(menu_buf, &r1, COL32_TO_16(0xE8E8E8));
		SDL_FillRect(menu_buf, &r2, COL32_TO_16(0x1C57A2));
	}

}

#define ALEFT  0
#define ARIGHT 1

static void draw_arrow(int type, int x, int y) {
	SDL_Rect dst_r = {x, y - 13, 32, 32};
	if (type)
		SDL_BlitSurface(arrow_r, NULL, menu_buf, &dst_r);
	else
		SDL_BlitSurface(arrow_l, NULL, menu_buf, &dst_r);
}

int gn_init_skin(void) {
	menu_buf = SDL_CreateRGBSurface(SDL_SWSURFACE, 352, 256, 32, 0xFF0000, 0xFF00, 0xFF, 0x0);

	back = res_load_stbi("skin/back.tga");
	sfont = load_font("skin/font_small.tga");
	mfont = load_font("skin/font_large.tga");
	arrow_r = res_load_stbi("skin/arrow_right.tga");
	arrow_l = res_load_stbi("skin/arrow_left.tga");
	gngeo_logo = res_load_stbi("skin/gngeo.tga");

	if (!back || !sfont || !mfont || !arrow_r || !arrow_l || !gngeo_logo || !menu_buf) return false;
	return true;
}

static int pbar_y;

void gn_reset_pbar(void) {
	pbar_y = 0;
}

void gn_init_pbar(char *name) {
	interp = interpolation;
	interpolation = 0;

	if (pbar_y == 0) {
		draw_back();
		draw_string(menu_buf, sfont, MENU_TITLE_X, MENU_TITLE_Y, "Loading...");
	}

	SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
	screen_update();
}

void gn_update_pbar(Uint32 pos, Uint32 size) {
	SDL_Rect src_r = {2, 0, (pos * 67) / size, gngeo_logo->h};
	SDL_Rect dst_r = {219 + 26, 146 + 16, gngeo_logo->w, gngeo_logo->h};

	SDL_BlitSurface(gngeo_logo, &src_r, menu_buf, &dst_r);

	SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
	screen_update();
}

void gn_terminate_pbar(void) {
	SDL_Rect src_r = {2, 0, gngeo_logo->w, gngeo_logo->h};
	SDL_Rect dst_r = {219 + 26, 146 + 16, gngeo_logo->w, gngeo_logo->h};

	SDL_BlitSurface(gngeo_logo, &src_r, menu_buf, &dst_r);

	SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
	screen_update();
	interpolation = interp;

}

void gn_popup_error(char *name, char *fmt, ...) {
	char buf[512];
	va_list pvar;
	va_start(pvar, fmt);

	draw_back();
	draw_string(menu_buf, sfont, MENU_TITLE_X, MENU_TITLE_Y, name);

	vsnprintf(buf, 511, fmt, pvar);

	draw_string(menu_buf, sfont, MENU_TEXT_X, MENU_TEXT_Y, buf);

	draw_string(menu_buf, sfont, ALIGN_RIGHT, ALIGN_DOWN, "Press any key ...");
	SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
	screen_update();

	while (wait_event() == 0);
}

/* TODO: use a mini yes/no menu instead of B/X */
int gn_popup_question(char *name, char *fmt, ...) {
	char buf[512];
	va_list pvar;
	va_start(pvar, fmt);

	draw_back();

	draw_string(menu_buf, sfont, MENU_TITLE_X, MENU_TITLE_Y, name);

	vsnprintf(buf, 511, fmt, pvar);
	draw_string(menu_buf, sfont, MENU_TEXT_X, MENU_TEXT_Y, buf);
	draw_string(menu_buf, sfont, ALIGN_RIGHT, ALIGN_DOWN, "B: Yes  X: No");
	SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
	screen_update();

	while (1) {
		switch (wait_event()) {
			case GN_A: return 0;
			case GN_B: return 1;
			default:
				break;
		}
	}

}

//#define NB_ITEM_2 4

static void draw_menu(GN_MENU *m) {
	int start, end, i;
	int nb_item;
	GNFONT *fnt;
	LIST *l = m->item;


	if (m->draw_type == MENU_BIG)
		fnt = mfont;
	else
		fnt = sfont;

	nb_item = (MENU_TEXT_Y_END - MENU_TEXT_Y) / fnt->ysize - 1;

	draw_back();

	if (m->title) draw_string(menu_buf, sfont, MENU_TITLE_X, MENU_TITLE_Y, m->title);

	start = (m->current - nb_item >= 0 ? m->current - nb_item : 0);
	end = (start + nb_item < m->nb_elem ? start + nb_item : m->nb_elem - 1);

	for (i = 0; i < start; i++, l = l->next);
	for (i = start; i <= end; i++, l = l->next) {
		GN_MENU_ITEM *mi = (GN_MENU_ITEM *) l->data;
		int j = i - start;

		if (m->draw_type == MENU_BIG) {
			draw_string(menu_buf, fnt, ALIGN_CENTER, MENU_TEXT_Y + (j * fnt->ysize + 2), mi->name);
			if (i == m->current) {
				int len = string_len(fnt, mi->name) / 2;
				draw_arrow(ARIGHT, 176 - len - 32, MENU_TEXT_Y + (j * fnt->ysize + 2) + fnt->ysize / 2);
				draw_arrow(ALEFT, 176 + len, MENU_TEXT_Y + (j * fnt->ysize + 2) + fnt->ysize / 2);
			}

		} else {
			draw_string(menu_buf, fnt, MENU_TEXT_X + 10, MENU_TEXT_Y + (j * fnt->ysize + 2), mi->name);
			if (i == m->current) draw_string(menu_buf, fnt, MENU_TEXT_X, MENU_TEXT_Y + (j * fnt->ysize + 2), ">");
		}
	}
	SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
	screen_update();
}

//#undef NB_ITEM_2

GN_MENU_ITEM *gn_menu_create_item(char *name, Uint32 type,
		int (*action)(GN_MENU_ITEM *self, void *param)) {
	GN_MENU_ITEM *t = malloc(sizeof (GN_MENU_ITEM));
	t->name = strdup(name);
	//t->name=name;
	t->type = type;
	t->action = action;
	return t;
}

int test_action(GN_MENU_ITEM *self, void *param) {
	printf("Action!!\n");
	return 0;
}

static int load_state_action(GN_MENU_ITEM *self, void *param) {
	static Uint32 slot = 0;
	SDL_Rect dstrect = {24 + 75, 16 + 66, 304 / 2, 224 / 2};
	SDL_Rect dstrect_binding = {24 + 73, 16 + 64, 304 / 2 + 4, 224 / 2 + 4};
	//SDL_Rect dst_r={24,16,304,224};
	//SDL_Event event;
	SDL_Surface *tmps, *slot_img;
	char slot_str[32];

	Uint32 nb_slot = how_many_slot(conf.game);

	if (nb_slot == 0) {
		gn_popup_info("Load State", "There is currently no save state available");
		return 0; // nothing to do
	}

	//slot_img=load_state_img(conf.game,slot);
	tmps = load_state_img(conf.game, slot);
	slot_img = SDL_ConvertSurface(tmps, menu_buf->format, SDL_SWSURFACE);

	while (1) {
		//if (back) SDL_BlitSurface(back,NULL,menu_buf,&dst_r);
		//else SDL_FillRect(menu_buf,NULL,COL32_TO_16(0x11A011));
		draw_back();
		SDL_FillRect(menu_buf, &dstrect_binding, COL32_TO_16(0xFEFEFE));
		SDL_SoftStretch(slot_img, NULL, menu_buf, &dstrect);

		draw_string(menu_buf, sfont, MENU_TITLE_X, MENU_TITLE_Y, "Load State");
		sprintf(slot_str, "Slot Number %03d", slot);
		//draw_string(menu_buf,sfont,24+102,16+40,slot_str);
		draw_string(menu_buf, sfont, ALIGN_CENTER, ALIGN_UP, slot_str);

		if (slot > 0) draw_arrow(ALEFT, 44 + 16, 224 / 2 + 16);
		if (slot < nb_slot - 1) draw_arrow(ARIGHT, 304 - 43, 224 / 2 + 16);

		SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
		screen_update();
		switch (wait_event()) {
			case GN_LEFT:
				if (slot > 0) slot--;
				slot_img = SDL_ConvertSurface(load_state_img(conf.game, slot), menu_buf->format, SDL_SWSURFACE);
				break;
			case GN_RIGHT:
				if (slot < nb_slot - 1) slot++;
				slot_img = SDL_ConvertSurface(load_state_img(conf.game, slot), menu_buf->format, SDL_SWSURFACE);
				break;
			case GN_A:
				return 0;
				break;
			case GN_B:
			case GN_C:
				load_state(conf.game, slot);
				printf("Load state!!\n");
				return 1;
				break;
			default:
				break;
		}
	}
	return 0;
}

static int save_state_action(GN_MENU_ITEM *self, void *param) {
	static Uint32 slot = 0;
	SDL_Rect dstrect = {24 + 75, 16 + 66, 304 / 2, 224 / 2};
	SDL_Rect dstrect_binding = {24 + 73, 16 + 64, 304 / 2 + 4, 224 / 2 + 4};
	SDL_Surface *tmps, *slot_img = NULL;
	char slot_str[32];
	Uint32 nb_slot = how_many_slot(conf.game);

	if (nb_slot != 0) {
		tmps = load_state_img(conf.game, slot);
		slot_img = SDL_ConvertSurface(tmps, menu_buf->format, SDL_SWSURFACE);
	}

	while (1) {
		draw_back();
		if (slot != nb_slot) {
			SDL_FillRect(menu_buf, &dstrect_binding, COL32_TO_16(0xFEFEFE));
			SDL_SoftStretch(slot_img, NULL, menu_buf, &dstrect);
		} else {
			SDL_FillRect(menu_buf, &dstrect_binding, COL32_TO_16(0xFEFEFE));
			SDL_FillRect(menu_buf, &dstrect, COL32_TO_16(0xA0B0B0));
			draw_string(menu_buf, sfont, ALIGN_CENTER, ALIGN_CENTER, "Create a new Slot");
		}

		draw_string(menu_buf, sfont, MENU_TITLE_X, MENU_TITLE_Y, "Save State");
		sprintf(slot_str, "Slot Number %03d", slot);
		draw_string(menu_buf, sfont, ALIGN_CENTER, ALIGN_UP, slot_str);

		if (slot > 0) draw_arrow(ALEFT, 44 + 16, 224 / 2 + 16);
		if (slot < nb_slot) draw_arrow(ARIGHT, 304 - 43, 224 / 2 + 16);

		SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
		screen_update();

		switch (wait_event()) {
			case GN_LEFT:
				if (slot > 0) slot--;
				if (slot != nb_slot) slot_img = SDL_ConvertSurface(load_state_img(conf.game, slot),
						menu_buf->format, SDL_SWSURFACE);
				break;
			case GN_RIGHT:
				if (slot < nb_slot) slot++;
				if (slot != nb_slot) slot_img = SDL_ConvertSurface(load_state_img(conf.game, slot),
						menu_buf->format, SDL_SWSURFACE);
				break;
			case GN_A:
				return 0;
				break;
			case GN_B:
			case GN_C:
				save_state(conf.game, slot);
				printf("Save state!!\n");
				return 1;
				break;
			default:
				break;
		}
	}
	return 0;
}

static int exit_action(GN_MENU_ITEM *self, void *param) {
	//exit(0);
	return 2;
}

int menu_event_handling(struct GN_MENU *self) {
	//static SDL_Event event;
	GN_MENU_ITEM *mi;
	int a;
	LIST *l;
	switch (wait_event()) {
		case GN_UP:
			if (self->current > 0)
				self->current--;
			else
				self->current = self->nb_elem - 1;
			break;
		case GN_DOWN:
			if (self->current < self->nb_elem - 1)
				self->current++;
			else
				self->current = 0;
			break;

		case GN_LEFT:
			self->current -= 10;
			if (self->current < 0) self->current = 0;
			break;
		case GN_RIGHT:
			self->current += 10;
			if (self->current >= self->nb_elem) self->current = self->nb_elem - 1;
			break;
		case GN_A:
			return 0;
			break;
		case GN_B:
		case GN_C:
			l = list_get_item_by_index(self->item, self->current);
			if (l) {
				mi = (GN_MENU_ITEM *) l->data;
				if (mi->action)
					if ((a = mi->action(mi, NULL))) return a;
			}
			break;
		default:
			break;

	}
	return -1;
}

int icasesort(const struct dirent **a, const struct dirent **b) {
	const char *ca = (*a)->d_name;
	const char *cb = (*b)->d_name;
	return strcasecmp(ca, cb);
}

static int loadrom_action(GN_MENU_ITEM *self, void *param) {
	printf("Loading %s\n", self->name);
	close_game();
	if (conf.sound) close_sdl_audio();

	if (init_game(self->name) != true) {
		printf("Can't init %s...\n", self->name);
		gn_popup_error("Error! :", "Gngeo Couldn't init %s... \n\n"
				" Maybe blabla", self->name);
		return 0;
	}

	return 1;
}

void init_rom_browser_menu(void) {
	int i;
	int nbf;
	char filename[strlen(CF_STR(cf_get_item_by_name("rompath"))) + 256];
	struct stat filestat;
	struct dirent **namelist;
	ROM_DEF *drv;
	//char name[32];
	int nb_roms = 0;

	rbrowser_menu = malloc(sizeof (GN_MENU));
	rbrowser_menu->title = "Load Game";
	rbrowser_menu->nb_elem = 0;
	rbrowser_menu->current = 0;
	rbrowser_menu->draw_type = MENU_SMALL;
	rbrowser_menu->event_handling = menu_event_handling;
	rbrowser_menu->draw = draw_menu;
	rbrowser_menu->item = NULL;
	/*
	if ((nbf = scandir(CF_STR(cf_get_item_by_name("rompath")), &namelist, NULL, icasesort )) != -1) {
		for (i = 0; i < nbf; i++) {
			sprintf(filename, "%s/%s", CF_STR(cf_get_item_by_name("rompath")), namelist[i]->d_name);
			lstat(filename, &filestat);
			if (!S_ISDIR(filestat.st_mode)) {
				printf("File %s\n",filename);
				if ((drv=dr_check_zip(filename))!=NULL) {
					rbrowser_menu->item=list_append(rbrowser_menu->item,
									(void*)gn_menu_create_item(drv->name,ACTION,loadrom_action));
					rbrowser_menu->nb_elem++;
				}
			}
		}
	}
	 */

	i = 0;
	while (romlist[i]) {
		sprintf(filename, "%s/%s.zip", CF_STR(cf_get_item_by_name("rompath")), romlist[i]);
		if (stat(filename, &filestat) == 0 && S_ISREG(filestat.st_mode)) {
			if ((drv = dr_check_zip(filename)) != NULL) {
				rbrowser_menu->item = list_append(rbrowser_menu->item,
						(void*) gn_menu_create_item(drv->name, ACTION, loadrom_action));
				rbrowser_menu->nb_elem++;
				free(drv);
				nb_roms++;
			}
		}
		sprintf(filename, "%s/%s.gno", CF_STR(cf_get_item_by_name("rompath")), romlist[i]);
		if (stat(filename, &filestat) == 0 && S_ISREG(filestat.st_mode)) {
			char *gnoname = dr_gno_romname(filename);
			if (gnoname != NULL) {
				rbrowser_menu->item = list_append(rbrowser_menu->item,
						(void*) gn_menu_create_item(filename, ACTION, loadrom_action));
				rbrowser_menu->nb_elem++;
				nb_roms++;
			}
		}


		i++;
	}

	if (nb_roms == 0) {
		rbrowser_menu->item = list_append(rbrowser_menu->item,
				(void*) gn_menu_create_item("No Games Found...", ACTION, NULL));
		rbrowser_menu->nb_elem++;
	}
}

int rom_browser_menu(void) {
	static Uint32 init = 0;
	int a;

	if (init == 0) {
		init = 1;

		draw_back();
		draw_string(menu_buf, sfont, MENU_TITLE_X, MENU_TITLE_Y, "Scanning ....");
		SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
		screen_update();

		init_rom_browser_menu();
	}
	while (1) {
		rbrowser_menu->draw(rbrowser_menu);
		if ((a = rbrowser_menu->event_handling(rbrowser_menu)) >= 0)
			return a;
	}
}

static int rbrowser_action(GN_MENU_ITEM *self, void *param) {
	//exit(0);
	return rom_browser_menu();
}

void gn_init_menu(void) {

	main_menu = malloc(sizeof (GN_MENU));
	main_menu->title = "Gngeo";
	main_menu->nb_elem = 0;
	main_menu->current = 0;
	main_menu->draw_type = MENU_BIG;
	main_menu->event_handling = menu_event_handling;
	main_menu->draw = draw_menu;
	main_menu->item = NULL;
	/* Create item */



	main_menu->item = list_append(main_menu->item,
			(void*) gn_menu_create_item("Load game", ACTION, rbrowser_action));
	main_menu->nb_elem++;


	main_menu->item = list_append(main_menu->item,
			(void*) gn_menu_create_item("Load state", ACTION, load_state_action));
	main_menu->nb_elem++;
	main_menu->item = list_append(main_menu->item,
			(void*) gn_menu_create_item("Save state", ACTION, save_state_action));
	main_menu->nb_elem++;
	main_menu->item = list_append(main_menu->item,
			(void*) gn_menu_create_item("Exit", ACTION, exit_action));
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
	static Uint32 init = 0;
	int a;

	if (init == 0) {
		init = 1;
		gn_init_menu();
	}

	while (1) {
		main_menu->draw(main_menu);
		if ((a = main_menu->event_handling(main_menu)) >= 0)
			return a;
	}

	if (conf.game == NULL) return 2; /* Exit */
	return 0;
}

