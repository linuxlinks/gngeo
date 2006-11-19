/*  gngeo a neogeo emulator
 *  Copyright (C) 2001 Peponas Mathieu
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
#include <config.h>
#endif

#include "SDL.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "streams.h"
#include "ym2610/2610intf.h"
#include "font.h"
#include "fileio.h"
#include "video.h"
#include "screen.h"
#include "emu.h"
#include "sound.h"
#include "messages.h"
#include "memory.h"
#include "debug.h"
#include "blitter.h"
#include "effect.h"
#include "conf.h"
#include "transpack.h"
#include "gngeo_icon.h"
#include "driver.h"

#ifdef USE_GUI
#include "gui_interf.h"
#endif
#ifdef GP2X
#include "gp2x.h"
#include "menu.h"
#endif


void calculate_hotkey_bitmasks()
{
    int *p;
    int i, j, mask;
    char *p1_key_list[] = { "p1hotkey0", "p1hotkey1", "p1hotkey2", "p1hotkey3" };
    char *p2_key_list[] = { "p2hotkey0", "p2hotkey1", "p2hotkey2", "p2hotkey3" };


    for ( i = 0; i < 4; i++ ) {
	p=CF_ARRAY(cf_get_item_by_name(p1_key_list[i]));
	for ( mask = 0, j = 0; j < 4; j++ ) mask |= p[j];
	conf.p1_hotkey[i] = mask;
    }

    for ( i = 0; i < 4; i++ ) {
	p=CF_ARRAY(cf_get_item_by_name(p2_key_list[i]));
	for ( mask = 0, j = 0; j < 4; j++ ) mask |= p[j];
	conf.p2_hotkey[i] = mask;
    }

}

void init_joystick(void) 
{
    //    int invert_joy=CF_BOOL(cf_get_item_by_name("invertjoy"));
    int i;
    int joyindex[3];
    int lastinit=-1;
#ifdef GP2X_
    int nb_joy=3;
    joyindex[0]=0;
    joyindex[1]=CF_VAL(cf_get_item_by_name("p1joydev"))+1;
    joyindex[2]=CF_VAL(cf_get_item_by_name("p2joydev"))+1;
#else
    int nb_joy=2;
    joyindex[0]=CF_VAL(cf_get_item_by_name("p1joydev"));
    joyindex[1]=CF_VAL(cf_get_item_by_name("p2joydev"));
#endif

    if (!CF_BOOL(cf_get_item_by_name("joystick")))
	return;

    SDL_JoystickEventState(SDL_ENABLE);

    conf.nb_joy = SDL_NumJoysticks();
#ifdef GP2X
    conf.nb_joy--;
#endif
    //printf("Nb_joy=%d %d %d \n",conf.nb_joy,joyindex[1],joyindex[2]);
    /* on ne gere que les deux premiers joysticks */
    /*
      if (conf.nb_joy > 2)
      conf.nb_joy = 2;

      if (invert_joy && conf.nb_joy < 2) {
      invert_joy = 0;
      CF_BOOL(cf_get_item_by_name("invertjoy"))=SDL_FALSE;
      }
    */
 
    for (i=0;i<conf.nb_joy;i++) {
	if (lastinit!=joyindex[i]) {
	    lastinit=joyindex[i];
	    //printf("Try to init joy %d\n",i);
	    conf.joy[i] = SDL_JoystickOpen(joyindex[i]);
	    if (conf.joy[i] && joyindex[i]<conf.nb_joy) {
		    joy_numaxes[i] = SDL_JoystickNumAxes(conf.joy[i]);
		    printf("joy %s, axe:%d, button:%d\n",
			   SDL_JoystickName(i),
			   // HAT SUPPORT SDL_JoystickNumAxes(conf.joy[i]),
			   joy_numaxes[i] + (SDL_JoystickNumHats(conf.joy[i]) * 2),
			   SDL_JoystickNumButtons(conf.joy[i]));
		    joy_button[i] =	(Uint8 *) malloc(SDL_JoystickNumButtons(conf.joy[i]));
		    // joy_axe[i] = (Uint32 *) malloc(SDL_JoystickNumAxes(conf.joy[i]) * sizeof(Sint32));
		    joy_axe[i] = (Uint32 *) malloc((joy_numaxes[i] + (SDL_JoystickNumHats(conf.joy[i]) * 2)) * sizeof(Uint32));
		    memset(joy_button[i], 0, SDL_JoystickNumButtons(conf.joy[i]));
		    // memset(joy_axe[i], 0, SDL_JoystickNumAxes(conf.joy[i]) * sizeof(Sint32));
		    memset(joy_axe[i], 0, (joy_numaxes[i] + (SDL_JoystickNumHats(conf.joy[i]) * 2)) * sizeof(Uint32));
	    }
	} else {
	    printf("Joystick number %d used for Player1, skip..\n",joyindex[i]);
	}
    }
    conf.p2_joy = CF_ARRAY(cf_get_item_by_name("p2joy"));
    conf.p1_joy = CF_ARRAY(cf_get_item_by_name("p1joy"));
 
}

void sdl_set_title(char *name) {
    char *title;
    if (name) {
	title = malloc(strlen("Gngeo : ")+strlen(name)+1);
	sprintf(title,"Gngeo : %s",name);
	SDL_WM_SetCaption(title, NULL);
    } else {
	SDL_WM_SetCaption("Gngeo", NULL);
    }
}

void init_sdl(void /*char *rom_name*/)
{
    Uint32 sdl_flag = 0;
    //char title[32] = "Gngeo : ";
    int i;
#ifdef GP2X
    int surface_type = SDL_HWSURFACE;//(conf.hw_surface ? SDL_HWSURFACE : SDL_SWSURFACE);
#else
    int surface_type = SDL_SWSURFACE;//(conf.hw_surface ? SDL_HWSURFACE : SDL_SWSURFACE);
#endif


    char *nomouse = getenv("SDL_NOMOUSE");
    SDL_Surface *icon;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

#ifdef GP2X
    atexit(gp2x_quit);
#else
    atexit(SDL_Quit);
#endif

#ifdef DEBUG_VIDEO
    screen = SDL_SetVideoMode(512 + 32, 512 + 32, 16, sdl_flag);
    buffer = SDL_CreateRGBSurface(surface_type, 512 + 32, 512 + 32, 16, 0xF800,
				  0x7E0, 0x1F, 0);
#else

    if (screen_init() == SDL_FALSE) {
	printf("Screen initialization failed.\n");
	exit(-1);
    }
    buffer = SDL_CreateRGBSurface(surface_type, 352, 256, 16, 0xF800, 0x7E0,
				  0x1F, 0);
    SDL_FillRect(buffer,NULL,SDL_MapRGB(buffer->format,0xE5,0xE5,0xE5));
#endif

    fontbuf = SDL_CreateRGBSurfaceFrom(font_image.pixel_data, font_image.width, font_image.height
				       , 24, font_image.width * 3, 0xFF0000, 0xFF00, 0xFF, 0);
    SDL_SetColorKey(fontbuf,SDL_SRCCOLORKEY,SDL_MapRGB(fontbuf->format,0xFF,0,0xFF));
    fontbuf=SDL_DisplayFormat(fontbuf);
    icon = SDL_CreateRGBSurfaceFrom(gngeo_icon.pixel_data, gngeo_icon.width,
				    gngeo_icon.height, gngeo_icon.bytes_per_pixel*8,
				    gngeo_icon.width * gngeo_icon.bytes_per_pixel,
				    0xFF, 0xFF00, 0xFF0000, 0);
    
#ifdef GP2X
    sprbuf=SDL_CreateRGBSurface(surface_type, 16, 16, 16, 0xF800, 0x7E0,
				0x1F, 0);
    SDL_SetColorKey(sprbuf,SDL_SRCCOLORKEY,0xF81F);
#endif

    /*
      strncat(title, rom_name, 8);
      SDL_WM_SetCaption(title, NULL);
    */
    SDL_WM_SetIcon(icon,NULL);

    calculate_hotkey_bitmasks();    
    init_joystick();
    /* init key mapping */
    conf.p1_key=CF_ARRAY(cf_get_item_by_name("p1key"));
    conf.p2_key=CF_ARRAY(cf_get_item_by_name("p2key"));

    if (nomouse == NULL)
	SDL_ShowCursor(0);
}

int main(int argc, char *argv[])
{
    char *rom_name;
    int nopt;
    char *drconf,*gpath;
    DRIVER *dr;
    Uint8 gui_res,gngeo_quit=0;
    char *country;
    char *system;
    /* faut bien le mettre quelque part */

    cf_init(); /* must be the first thing to do */
//#ifdef GP2X
//    cf_open_file("conf/gngeorc");
//#else
    cf_open_file(NULL);
//#endif
    cf_init_cmd_line();


    //nopt=cf_get_non_opt_index(argc,argv);
    rom_name=cf_parse_cmd_line(argc,argv); /* First pass */

    /* First try to suppress the romrc in favor of romrc.d */
    //dr_load_driver(CF_STR(cf_get_item_by_name("romrc")));

    dr_load_driver_dir(CF_STR(cf_get_item_by_name("romrcdir")));
#if !defined (GP2X) && !defined(WIN32)
    {
	    int len = strlen("romrc.d") + strlen(getenv("HOME")) + strlen("/.gngeo/") +	1;
	    char *rc_dir = (char *) alloca(len*sizeof(char));
	    sprintf(rc_dir, "%s/.gngeo/romrc.d", getenv("HOME"));
	    dr_load_driver_dir(rc_dir);
    }
#endif

    /* print effect/blitter list if asked by user */
    if (!strcmp(CF_STR(cf_get_item_by_name("effect")),"help")) {
	print_effect_list();
	exit(0);
    }
    if (!strcmp(CF_STR(cf_get_item_by_name("blitter")),"help")) {
	print_blitter_list();
	exit(0);
    }

    if (!rom_name) {
	    cf_print_help();
	    exit(0);
    }
   

#ifdef GP2X
    gp2x_init();
    init_sdl();

    sdl_set_title(NULL);
    SDL_textout(screen, 1, 231, "Patching MMU ... ");SDL_Flip(screen);

    //   if (hackpgtable()==0) {
    if (hack_the_mmu()==0) {
	    SDL_textout(screen, 1, 231, "Patching MMU ... OK!");SDL_Flip(screen);
    } else {
	    SDL_textout(screen, 1, 231, "Patching MMU ... FAILED :(");SDL_Flip(screen);
	    SDL_Delay(300);
    }
    //SDL_Delay(200);
    //benchmark ((void*)screen->pixels);
    gn_init_skin();
#endif

 
/* per game config */
#if defined (GP2X) || defined (WIN32)
    gpath="conf/";
#else
    gpath=get_gngeo_dir();
#endif
    dr=dr_get_by_name(rom_name);
    if(!dr) {
#ifdef GP2X
	    gn_popup_error("Bad roms!",
			   "Unknow or unsupported romset. "
 			   "Check it and your romrc");
#else
	    printf("Unknow or unsupported romset.\n");
#endif
	    exit(1);
    }
    drconf=alloca(strlen(gpath)+strlen(dr->name)+strlen(".cf")+1);
    sprintf(drconf,"%s%s.cf",gpath,dr->name);
    cf_open_file(drconf);
 
    rom_name=cf_parse_cmd_line(argc,argv); /* Second pass */

    if (conf.debug) conf.sound=0;


#ifdef GP2X
    gp2x_init_mixer();
    gp2x_set_cpu_speed();
    SDL_FillRect(screen,NULL,0);
#else
    if (!rom_name) {
	cf_print_help();
	exit(0);
    }
#endif

       
    if (init_game(rom_name)!=SDL_TRUE) {
	    printf("Can't init %s...\n",rom_name);
            exit(1);
    }    
    if (conf.debug)
	    debug_loop();
    else
	    main_loop();


    save_nvram(conf.game);
    save_memcard(conf.game);

    return 0;
}
