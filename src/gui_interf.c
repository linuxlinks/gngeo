#ifdef USE_GUI

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "SDL.h"
#include "gui/gui.h"
#include "screen.h"
#include "effect.h"
#include "blitter.h"
#include "video.h"
#include "memory.h"
#include "frame_skip.h"
#include "emu.h"
//#include "driver.h"
#include "conf.h"
#include "fileio.h"
#include "sound.h"

#if defined (__AMIGA__)
# ifdef DATA_DIRECTORY
#  undef DATA_DIRECTORY
#  define DATA_DIRECTORY "/PROGDIR/data/"
# endif
#endif

static WIDGET *desktop;
static SDL_bool game_only=SDL_FALSE;



static SDL_Rect buf_rect  =	{24, 16, 304, 224};
static SDL_Rect desk_rect =	{ 0,  0, 304, 224};
static SDL_bool quit_emu=SDL_FALSE;

static void update_gngeo_gui(SDL_Surface *desk) {
    SDL_BlitSurface(desk,&desk_rect,buffer,&buf_rect);
    screen_update();
}

/*** loading progress bar ****/
static WIDGET *ld_pbar,*ld_label;

void loading_pbar_set_label(char *label) {
    if (label) {
	gui_label_change((LABEL*)ld_label,label);
	ld_label->map(ld_label);
	ld_pbar->map(ld_pbar);
    } else {
	ld_label->unmap(ld_label);
	ld_pbar->unmap(ld_pbar);
    }
}

void loading_pbar_update(Uint32 val,Uint32 size) {
    gui_progressbar_set_size((PROGRESSBAR*)ld_pbar,size);
    gui_progressbar_set_val((PROGRESSBAR*)ld_pbar,val);
}

static void gngeo_quit(void) {
    if (conf.sound)
	close_sdl_audio();
}

/*** Game selection ****/
typedef struct G_GAME_LIST{
    char *dir;
    char *file;
    DRIVER *dr;
}G_GAME_LIST;


static int my_alphasort (const void *a, const void *b) {
    return alphasort(b,a);
}

static LIST* create_list_from_dir(char *dir) {
    DIR *dir_p;
    G_GAME_LIST *t;
    struct dirent *dirent_p;
    char filename[strlen(dir)+256];
    LIST *l=NULL;
    struct stat filestat;
    struct dirent **namelist;
    int nbf, i;
/*
    if (dir[strlen(dir)-1]=='/')
	dir[strlen(dir)-1]=0;
*/
 
    if ((nbf = scandir(dir, &namelist, NULL/*game_selection*/, my_alphasort )) != -1) {
	for (i = 0; i < nbf; i++) {
	    sprintf(filename, "%s/%s", dir, namelist[i]->d_name);
	    lstat(filename, &filestat);
	    if (S_ISDIR(filestat.st_mode)) {
		t=malloc(sizeof(G_GAME_LIST));
		t->dir=malloc(strlen(namelist[i]->d_name)*sizeof(char)+2);
		sprintf(t->dir,"%s",namelist[i]->d_name);
		t->dr=NULL;
		t->file=NULL;
		l=list_prepend(l,t);
	    } else if (game_only==SDL_TRUE) {
		DRIVER *dr;
		if ((dr=get_driver_for_zip(filename))!=NULL) {
		    t=malloc(sizeof(G_GAME_LIST));
		    t->dir=NULL;
		    t->file=strdup(namelist[i]->d_name);
		    t->dr=dr;
		    l=list_prepend(l,t);
		    //printf("GAME %s\n",t->dr->longname);
		}
	    } else {
		t=malloc(sizeof(G_GAME_LIST));
		t->file=strdup(namelist[i]->d_name);
		t->dir=NULL;
		t->dr=NULL;
		l=list_prepend(l,t);
		//printf("FILE %s\n",t->file);
	    }
	}
    }
    if (nbf==-1) {
	/* the dir is unreadable, we create pseudo dir . and .. */
	t=malloc(sizeof(G_GAME_LIST));
	t->dir=strdup("..");
	t->dr=NULL;
	t->file=NULL;
	l=list_prepend(l,t);

	t=malloc(sizeof(G_GAME_LIST));
	t->dir=strdup(".");
	t->dr=NULL;
	t->file=NULL;
	l=list_prepend(l,t);
    }

    return l;
}

static char *get_glist_label(void *data) {
    G_GAME_LIST *d=data;
    static char dir[256];
    if(d->dir) {
	sprintf(dir,"%s/",d->dir);
	return dir;
    }
    else if (d->dr) {
	if (d->dr->longname && strcmp(d->dr->longname,"")!=0)
	    return d->dr->longname;
	else
	    return d->dr->name;
    }
    else
	return d->file;
}

#ifndef GP2X 
/* Old gui code, no more maintained*/
static SDL_Surface *screenshots,*unavail_shots;
static WIDGET *game_pix,*info_label;

static int game_list_show_rom_only_toggle(CHECK_BUTTON *self) {
    SLIST *sl=GUI_WIDGET(self)->pdata1;
    char *path=GUI_WIDGET(sl)->pdata1;
    if (WID_IS_CHECKED((WIDGET*)self))
	game_only=SDL_TRUE;
    else
	game_only=SDL_FALSE;
    sl->l=create_list_from_dir(path);
    gui_slist_update(sl);
    sl->data_selected=NULL;
    return TAKE_EVENT;
}

static void update_game_info(DRIVER *dr) {
    static char *shotsdir=DATA_DIRECTORY"/shots";
    static char lgname[32];
    
    if (dr) {
	SDL_FreeSurface(screenshots);
	screenshots=load_img(shotsdir,dr->name);
	if (!screenshots)
	    gui_gpix_set_pixmap((GPIX*)game_pix,unavail_shots);
	else
	    gui_gpix_set_pixmap((GPIX*)game_pix,screenshots);
	snprintf(lgname,30,"%s",dr->longname);
	gui_label_change((LABEL*)info_label,lgname);
	info_label->map(info_label);
    } else {
	SDL_FreeSurface(screenshots);screenshots=NULL;
	gui_gpix_set_pixmap((GPIX*)game_pix,NULL);
	info_label->unmap(info_label);
    }
}


static int game_list_sel_change(SLIST *self) {
    SLIST *sl=self;
    DRIVER *dr;
    static char *shotsdir=DATA_DIRECTORY"/shots";
    if (self->data_selected) {
	G_GAME_LIST *gl=sl->data_selected->data;
	if (gl->dir) {
	    update_game_info(NULL);
	    return TAKE_EVENT;
	}
	if (game_only==SDL_TRUE) {
	    update_game_info(gl->dr);
	} else {
	    char *filepath=alloca(strlen((char*)GUI_WIDGET(self)->pdata1)+strlen(gl->file)+1);
	    sprintf(filepath,"%s/%s",(char*)GUI_WIDGET(self)->pdata1,gl->file);
	    if ((dr=get_driver_for_zip(filepath))!=NULL) {
		update_game_info(dr);
	    } else {
		update_game_info(NULL);
	    }
	}
    }
    return TAKE_EVENT;
}

static int game_list_double_click(SLIST* self) {
    SLIST *sl=self;
    if (sl->data_selected) {
	G_GAME_LIST *gl=sl->data_selected->data;
	if (gl->dir) {
	    char *path=GUI_WIDGET(self)->pdata1;
	    if (strcmp(gl->dir,".")==0) return TAKE_EVENT;
	    if (strcmp(gl->dir,"..")==0) {
		char *t;
		if (strcmp(path,"/")==0) return TAKE_EVENT;
/*
		if (path[strlen(path)-1]=='/')
		    path[strlen(path)-1]=0;
		printf("PATH2=%s\n",path);
*/
/*
		t=strrchr(path,'/');
		if (t) t[0]=0;
*/
		t=strrchr(path,'/');
		if (t) t[0]=0;

		if (strcmp(path,"")==0)
		    sprintf(path,"/");

	    } else {
		if (path)
		    path=realloc(path,strlen(path)+strlen(gl->dir)+2);
		else
		    path=strdup("/");
		GUI_WIDGET(self)->pdata1=path;
		if (strcmp(path,"/")==0)
		    sprintf(path,"/%s",gl->dir);
		else
		    sprintf(path,"%s/%s",path,gl->dir);
		//printf("Change dir %s path=%s\n",gl->dir,path);
	    }
	    /* recreate list */
	    list_erase_all(GUI_SLIST(sl)->l,NULL);
	    GUI_SLIST(sl)->l=create_list_from_dir(path);
	    gui_slist_update(GUI_SLIST(sl));
	    sl->data_selected=NULL;
	} else if (gl->dr) {
	    char *filepath=alloca(strlen((char*)GUI_WIDGET(self)->pdata1)+strlen(gl->file)+1);
	    sprintf(filepath,"%s/%s",(char*)GUI_WIDGET(self)->pdata1,gl->file);
	    printf("Load game %s,%s\n",filepath,gl->dr->longname);
	    if (init_game(filepath))
		return QUIT_EVENT;
	    else
		return TAKE_EVENT;
	} else {
	    char *filepath=alloca(strlen((char*)GUI_WIDGET(self)->pdata1)+strlen(gl->file)+1);
	    sprintf(filepath,"%s/%s",(char*)GUI_WIDGET(self)->pdata1,gl->file);
	    printf("Should try to load file %s %s\n",filepath,gl->file);
	    if (init_game(filepath))
		return QUIT_EVENT;
	    else
		return TAKE_EVENT;
	}
    }
    return TAKE_EVENT;
}

/* effect/blitter */
static char *get_effect_label(void *data) {
    effect_func *e=data;
    return e->name;
}
static int switch_effect(COMBO_LIST *self) {
    char *effect_name=self->text->text;
    COMBO_LIST *bcl=GUI_WIDGET(self)->pdata1;
    printf("set effect to %s\n",effect_name);
    screen_change_blitter_and_effect(NULL,effect_name);
    gui_combol_set_selection(self,neffect);
    gui_combol_set_selection(bcl,nblitter);
    return TAKE_EVENT;
}
static char *get_blitter_label(void *data) {
    blitter_func *b=data;
    return b->name;
}
static int switch_blitter(COMBO_LIST *self) {
    char *blitter_name=self->text->text;
    COMBO_LIST *ecl=GUI_WIDGET(self)->pdata1;
    printf("set blitter to %s\n",blitter_name);
    screen_change_blitter_and_effect(blitter_name,NULL);
    gui_combol_set_selection(ecl,neffect);
    gui_combol_set_selection(self,nblitter);
    return TAKE_EVENT;
}
/* sound */
static int toggle_sound(CHECK_BUTTON *self) {
    if (conf.sound) {
	conf.sound=0;
	close_sdl_audio();
    } else {
	conf.sound=1;
	init_sdl_audio();
    }
    return TAKE_EVENT;
}

/* plop! */
static int test_click(struct BUTTON* self) {
    printf("Click!! %s\n",(char*)GUI_WIDGET(self)->pdata1);
    return TAKE_EVENT;
}

static void print_val(WIDGET*self,Uint32 val){
    printf("val=%d\n",val);
}

static char *get_game_label(void *data) {
    DRIVER *d=data;
    return d->longname;
}


static int test_game_list_double_click(struct SLIST* self) {
    SLIST *sl=self;
    if (sl->data_selected)
	printf("Humm I should load %s\n",((DRIVER*)sl->data_selected->data)->longname);
    return TAKE_EVENT;
}

static int quit_click(struct BUTTON* self) {
    //printf("Quit!! \n");
    quit_emu=SDL_TRUE;
    return QUIT_EVENT;
/*
    quit_emu=SDL_TRUE;
    return TAKE_EVENT;
*/
}


/*** Gui creation ***/
static WIDGET *wid_game,*wid_setting,*wid_control;
static WIDGET *wid_current;

static WIDGET* init_game_gui(void) {
    WIDGET *frame,*label,*glist,*button,*pix_frame;
    char *ld_label_str;
    frame=(WIDGET*)gui_frame_create(NULL,2,19,300,203,FRAME_SHADOW_ETCHED_IN,"Load game");

    /* Game list selection */
    glist=(WIDGET*)gui_slist_create(NULL,158,8,140,181);
    gui_container_add(GUI_CONTAINER(frame),glist);
    GUI_SLIST(glist)->sel_double_click=game_list_double_click;
    GUI_SLIST(glist)->sel_change=game_list_sel_change;
    GUI_SLIST(glist)->l=create_list_from_dir(CF_STR(cf_get_item_by_name("rompath")));
    glist->pdata1=strdup(CF_STR(cf_get_item_by_name("rompath")));
    GUI_SLIST(glist)->get_label=get_glist_label;
    gui_slist_update(GUI_SLIST(glist));
    

    /* Loading progress bar */
    ld_pbar=(WIDGET*)gui_progressbar_create(NULL,107,191,191,10);
    gui_container_add(GUI_CONTAINER(frame),ld_pbar);
    
    ld_label=(WIDGET*)gui_label_create(NULL,20,193,1,1,"");
    gui_container_add(GUI_CONTAINER(frame),ld_label);

    /* some Check button */
    button=(WIDGET*)gui_check_button_create(NULL,4,179,150,10,"Show only NeoGeo's rom");
    GUI_CHECK_BUTTON(button)->toggle=game_list_show_rom_only_toggle;
    button->pdata1=glist;
    gui_container_add(GUI_CONTAINER(frame),button);
    gui_widget_add_keyfocus(desktop,button);
    /* We want glist after check button in the key focus list  */
    gui_widget_add_keyfocus(desktop,GUI_SLIST(glist)->text);

    /* Game information */
    pix_frame=(WIDGET*)gui_frame_create(NULL,2,8,154,114,FRAME_SHADOW_IN,NULL);
    gui_container_add(GUI_CONTAINER(frame),pix_frame);
    game_pix=(WIDGET*)gui_gpix_create(NULL,1,1,152,112);

    screenshots=NULL;
    gui_gpix_set_pixmap(GUI_GPIX(game_pix),screenshots);
    gui_container_add(GUI_CONTAINER(pix_frame),game_pix);
    
    info_label=(WIDGET*)gui_label_create(NULL,5,125,1,1,"");
    gui_container_add(GUI_CONTAINER(frame),info_label);

    return frame;
}

static WIDGET* init_setting_gui(void) {
    WIDGET *frame,*label,*sl,*button;
    WIDGET *cl_effect,*cl_blitter;

    frame=(WIDGET*)gui_frame_create(NULL,2,19,300,203,FRAME_SHADOW_ETCHED_IN,"Settings");
    label=(WIDGET*)gui_label_create(NULL,10,10,100,8,"Settings Panel");
    gui_container_add(GUI_CONTAINER(frame),label);

    /* sound option */
    button=(WIDGET*)gui_check_button_create(NULL,4,20,150,10,"Enable sound");
    GUI_CHECK_BUTTON(button)->toggle=toggle_sound;
    if (conf.sound)
	button->flags|=FLAG_CHECKED;
    gui_container_add(GUI_CONTAINER(frame),button);
    gui_widget_add_keyfocus(desktop,button);


    /* effect combo list */
    sl=(WIDGET*)gui_combol_create(NULL,40,50,80,10,120);
    gui_container_add(GUI_CONTAINER(frame),sl);
    gui_widget_add_keyfocus(desktop,(WIDGET*)GUI_COMBO_LIST(sl)->dbut);
    GUI_COMBO_LIST(sl)->change=switch_effect;
    GUI_COMBO_LIST(sl)->slist->l=create_effect_list();
    GUI_COMBO_LIST(sl)->slist->get_label=get_effect_label;
    gui_combol_update(GUI_COMBO_LIST(sl));
    gui_combol_set_selection(GUI_COMBO_LIST(sl),neffect);
    cl_effect=sl;

    /* blitter combo list */
    sl=(WIDGET*)gui_combol_create(NULL,40,65,80,10,50);
    gui_container_add(GUI_CONTAINER(frame),sl);
    gui_widget_add_keyfocus(desktop,(WIDGET*)GUI_COMBO_LIST(sl)->dbut);
    GUI_COMBO_LIST(sl)->change=switch_blitter;
    GUI_COMBO_LIST(sl)->slist->l=create_blitter_list();
    GUI_COMBO_LIST(sl)->slist->get_label=get_blitter_label;
    gui_combol_update(GUI_COMBO_LIST(sl));
    gui_combol_set_selection(GUI_COMBO_LIST(sl),nblitter);
    cl_blitter=sl;

    cl_blitter->pdata1=cl_effect;
    cl_effect->pdata1=cl_blitter;

    return frame;
}

static WIDGET* init_control_gui(void) {
    WIDGET *frame,*label;
    frame=(WIDGET*)gui_frame_create(NULL,2,22,300,200,FRAME_SHADOW_ETCHED_IN,"Control");
    label=(WIDGET*)gui_label_create(NULL,10,10,100,8,"Control configuration Panel");
    gui_container_add(GUI_CONTAINER(frame),label);
    return frame;
}

static int change_panel(struct BUTTON* self) {
    WIDGET *w=(WIDGET*)GUI_WIDGET(self)->pdata1;
    if (w!=wid_current) {
	wid_current->unmap(wid_current);
	wid_current=w;
	wid_current->map(wid_current);
    }
    return TAKE_EVENT;
}

static SDL_Surface *pix_full,*pix_win;
static WIDGET *gpix_full;

static int toggle_fullscreen(struct BUTTON* self) {
    screen_fullscreen();
    if (!fullscreen)
	gui_gpix_set_pixmap((GPIX*)gpix_full,pix_full);
    else
	gui_gpix_set_pixmap((GPIX*)gpix_full,pix_win);
    return TAKE_EVENT;
}

void init_gngeo_gui(void) {
//    WIDGET *w,*button,*button2,*label;
    WIDGET *button,*pix;

    unavail_shots=load_img(DATA_DIRECTORY,"noshots");

    gui_init(304,224,DATA_DIRECTORY);
    gui_set_update(update_gngeo_gui);
    gui_set_resize(screen_resize);
    gui_set_destroy(gngeo_quit);
    
    /* first thing to create, because it contain the key focus list */
    desktop=(WIDGET*)gui_frame_create(NULL,0,0,304,224,FRAME_SHADOW_OUT,NULL);

    //printf("CREATE game\n");
    wid_game=init_game_gui();
    //printf("CREATE setting\n");
    wid_setting=init_setting_gui();
    //printf("CREATE control\n");
    wid_control=init_control_gui();
    
    /* Main button */
    button=(WIDGET*)gui_button_label_create(NULL,2,2,45,15,"Game");
    gui_widget_add_keyfocus(desktop,button);
    button->pdata1=wid_game;
    GUI_BUTTON(button)->activate=change_panel;
    gui_container_add(GUI_CONTAINER(desktop),button);

    button=(WIDGET*)gui_button_label_create(NULL,49,2,45,15,"Setting");
    gui_widget_add_keyfocus(desktop,button);
    button->pdata1=wid_setting;
    GUI_BUTTON(button)->activate=change_panel;
    gui_container_add(GUI_CONTAINER(desktop),button);

    button=(WIDGET*)gui_button_label_create(NULL,96,2,45,15,"Control");
    gui_widget_add_keyfocus(desktop,button);
    button->pdata1=wid_control;
    GUI_BUTTON(button)->activate=change_panel;
    gui_container_add(GUI_CONTAINER(desktop),button);

    /* toggle fullscreen button */
    button=(WIDGET*)gui_button_create(NULL,270,3,14,14);
    gui_widget_add_keyfocus(desktop,button);
    GUI_BUTTON(button)->activate=toggle_fullscreen;
    gui_container_add(GUI_CONTAINER(desktop),button);
    pix_full=load_img_rle(DATA_DIRECTORY,"fullscreen");
    pix_win=load_img_rle(DATA_DIRECTORY,"window");
    gpix_full=(WIDGET*)gui_gpix_create(NULL,1,1,12,12);
    if (!fullscreen)
	gui_gpix_set_pixmap((GPIX*)gpix_full,pix_full);
    else
	gui_gpix_set_pixmap((GPIX*)gpix_full,pix_win);
     gui_container_add(GUI_CONTAINER(button),gpix_full);


     /* quit button */
    button=(WIDGET*)gui_button_create(NULL,286,3,14,14);
    gui_widget_add_keyfocus(desktop,button);
    GUI_BUTTON(button)->activate=quit_click;
    gui_container_add(GUI_CONTAINER(desktop),button);
    pix=(WIDGET*)gui_gpix_create(NULL,1,1,12,12);
    gui_gpix_set_pixmap((GPIX*)pix,load_img_rle(DATA_DIRECTORY,"close"));
    gui_container_add(GUI_CONTAINER(button),pix);


    desktop->map(desktop);

    //printf("ADD game\n");
    gui_container_add(GUI_CONTAINER(desktop),wid_game);
    
    //printf("ADD setting\n");
    gui_container_add(GUI_CONTAINER(desktop),wid_setting);
    
    //printf("ADD control\n");
    gui_container_add(GUI_CONTAINER(desktop),wid_control);

    wid_game->map(wid_game);
    wid_setting->unmap(wid_setting);
    wid_control->unmap(wid_control);

    wid_current=wid_game;

    /* do not show progress bar for the moment */
    ld_pbar->unmap(ld_pbar);
    ld_label->unmap(ld_label);
	
    
    //desktop=(WIDGET*)gui_frame_create(NULL,0,0,304,224,SHADOW_ETCHED_IN,"Game:");

    
}
#else

#endif

SDL_bool main_gngeo_gui(void) {
    if (desktop==NULL)
	init_gngeo_gui();
    if (conf.sound) SDL_PauseAudio(1);

    /* disable interpolation */
    interpolation=SDL_FALSE;

    //SDL_ShowCursor(1);
    gui_main_loop(desktop/*,conf.res_x,conf.res_y*/);
    //SDL_ShowCursor(0);

    /* reinit interpolation */
    interpolation=CF_BOOL(cf_get_item_by_name("interpolation"));

    if (conf.sound) SDL_PauseAudio(0);
    reset_frame_skip();
//    printf("Quitemu=%d\n",quit_emu);
    return quit_emu;
}

#endif
