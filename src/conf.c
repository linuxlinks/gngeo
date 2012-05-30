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

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#ifdef HAVE_GETOPT_LONG
#include <getopt.h>
#else
#include "getopt.h"
#endif

#include <zlib.h>
#include "unzip.h"
#include "SDL.h"
#include "conf.h"
#include "emu.h"
#include "memory.h"
#include "gnutil.h"


/* 

   Xbox360 conf: A=J0B0,B=J0B1,C=J0B2,D=J0B3,START=J0B6,COIN=J0B10,UP=J0a1,DOWN=J0a1,LEFT=J0A0,RIGHT=J0A0,MENU=J0B7
   sixaxis conf: Need special calibration :(
   GP2X conf:    
   Pandora conf:
   wii conf:
   etc.


 */

static struct {
	CONF_ITEM **conf;
	int size, nb_item;
} cf_hash[128];


#ifdef GP2X
static int default_tvoffset[] = {0, 0};
#endif

#if defined(GP2X) || defined(WIZ)
static char * default_p1control = "UP=J0B0,DOWN=J0B4,LEFT=J0B2,RIGHT=J0B6,A=J0B14,"
		"B=J0B13,C=J0B12,D=J0B15,COIN=J0B9,START=J0B8,HOTKEY1=J0B10,HOTKEY2=J0B11";
static char * default_p2control = "";
#elif defined(PANDORA)
static char * default_p1control = "A=K281,B=K279,C=K278,D=K280,START=K308,COIN=K306,"
		"UP=K273,DOWN=K274,LEFT=K276,RIGHT=K275,MENU=K32";
static char * default_p2control = "";
#elif defined (DINGUX)
static char * default_p1control = "A=K308,B=K306,C=K304,D=K32,START=K13,COIN=K9,"
		"UP=K273,DOWN=K274,LEFT=K276,RIGHT=K275,MENU=K113";
static char * default_p2control = "";
#elif defined (WII)
static char *default_p1control = "A=J0B9,B=J0B10,C=J0B11,D=J0B12,START=J0B18,COIN=J0B17"
	"UPDOWN=J0A1,LEFTRIGHT=J0A0,JOY=J0H0";
static char *default_p2control = "....";
#else
	/* TODO: Make Querty default instead of azerty */
static char * default_p1control = "A=K119,B=K120,C=K113,D=K115,START=K38,COIN=K34,"
		"UP=K273,DOWN=K274,LEFT=K276,RIGHT=K275,MENU=K27";
static char * default_p2control = "";
#endif

static int default_p1hotkey0[] = {0, 0, 0, 0};
static int default_p1hotkey1[] = {0, 0, 0, 0};
static int default_p1hotkey2[] = {0, 0, 0, 0};
static int default_p1hotkey3[] = {0, 0, 0, 0};
static int default_p2hotkey0[] = {0, 0, 0, 0};
static int default_p2hotkey1[] = {0, 0, 0, 0};
static int default_p2hotkey2[] = {0, 0, 0, 0};
static int default_p2hotkey3[] = {0, 0, 0, 0};

void cf_cache_conf(void) {
	char *country;
	char *system;
	/* cache some frequently used conf item */
	//	printf("Update Conf Cache, sample rate=%d -> %d\n",conf.sample_rate,CF_VAL(cf_get_item_by_name("samplerate")));
	conf.sound = CF_BOOL(cf_get_item_by_name("sound"));
	conf.vsync = CF_BOOL(cf_get_item_by_name("vsync"));
	conf.sample_rate = CF_VAL(cf_get_item_by_name("samplerate"));
	conf.debug = CF_BOOL(cf_get_item_by_name("debug"));
	conf.raster = CF_BOOL(cf_get_item_by_name("raster"));
	conf.pal = CF_BOOL(cf_get_item_by_name("pal"));

	conf.autoframeskip = CF_BOOL(cf_get_item_by_name("autoframeskip"));
	conf.show_fps = CF_BOOL(cf_get_item_by_name("showfps"));
	conf.sleep_idle = CF_BOOL(cf_get_item_by_name("sleepidle"));
	conf.screen320 = CF_BOOL(cf_get_item_by_name("screen320"));
#ifdef GP2X
	conf.accurate940 = CF_BOOL(cf_get_item_by_name("940sync"));
#endif
	country = CF_STR(cf_get_item_by_name("country"));
	system = CF_STR(cf_get_item_by_name("system"));
	if (!strcmp(system, "unibios")) {
		conf.system = SYS_UNIBIOS;
	} else {
		if (!strcmp(system, "home")) {
			conf.system = SYS_HOME;
		} else {
			conf.system = SYS_ARCADE;
		}
	}
	if (!strcmp(country, "japan")) {
		conf.country = CTY_JAPAN;
	} else if (!strcmp(country, "usa")) {
		conf.country = CTY_USA;
	} else if (!strcmp(country, "asia")) {
		conf.country = CTY_ASIA;
	} else {
		conf.country = CTY_EUROPE;
	}
}

static void read_array(int *tab, char *val, int size) {
	int i = 0;
	char *v;

	v = strtok(val, ",");

	while (v != NULL && i < size) {
		tab[i] = atoi(v);
		v = strtok(NULL, ",");
		i++;
	}
}

static char **read_str_array(char *val, int *size) {
	char *v;
	int nb_elem = 1;
	int i = 0;
	char **tab;
	while (val[i] != 0) {
		if (val[i] == ',') nb_elem++;
		i++;
	}
	printf("%s :NB elem %d\n", val, nb_elem);
	tab = malloc(nb_elem * sizeof (char*));
	if (!tab) return NULL;

	v = strtok(val, ",");
	printf("V1=%s\n", v);
	for (i = 0; i < nb_elem; i++) {
		tab[i] = strdup(v);
		v = strtok(NULL, ",");
		printf("V%d=%s\n", i, v);
	}
	*size = nb_elem;
	return tab;
}

static CONF_ITEM * create_conf_item(const char *name, const char *help, char short_opt, int (*action)(struct CONF_ITEM *self)) {
	int a;
	static int val = 0x100;
	CONF_ITEM *t = (CONF_ITEM*) calloc(1, sizeof (CONF_ITEM));

	a = tolower((int) name[0]);

	t->name = strdup(name);
	t->help = strdup(help);
	t->modified = 0;
	if (short_opt == 0) {
		val++;
		t->short_opt = val;
	} else
		t->short_opt = short_opt;

	if (action) {
		t->action = action;
	}


	if (cf_hash[a].size <= cf_hash[a].nb_item) {
		cf_hash[a].size += 10;
		cf_hash[a].conf = (CONF_ITEM**) realloc(cf_hash[a].conf, cf_hash[a].size * sizeof (CONF_ITEM*));
	}

	cf_hash[a].conf[cf_hash[a].nb_item] = t;
	cf_hash[a].nb_item++;
	return t;
}

/*
 * Create a boolean confirutation item
 * name: Name of conf item
 * help: Brief description of the item
 * short_opt: charatech use for short option
 * def: default value
 */
void cf_create_bool_item(const char *name, const char *help, char short_opt, int def) {
	CONF_ITEM *t = create_conf_item(name, help, short_opt, NULL);
	t->type = CFT_BOOLEAN;
	t->data.dt_bool.boolean = def;
	t->data.dt_bool.default_bool = def;
}

void cf_create_action_item(const char *name, const char *help, char short_opt, int (*action)(struct CONF_ITEM *self)) {
	CONF_ITEM *t = create_conf_item(name, help, short_opt, action);
	t->type = CFT_ACTION;
}

void cf_create_action_arg_item(const char *name, const char *help, const char *hlp_arg, char short_opt, int (*action)(struct CONF_ITEM *self)) {
	CONF_ITEM *t = create_conf_item(name, help, short_opt, action);
	t->type = CFT_ACTION_ARG;
	t->help_arg = (char *) hlp_arg;
}

void cf_create_string_item(const char *name, const char *help, const char *hlp_arg, char short_opt, const char *def) {
	CONF_ITEM *t = create_conf_item(name, help, short_opt, NULL);
	t->type = CFT_STRING;
	strcpy(t->data.dt_str.str, def);
	t->data.dt_str.default_str = strdup(def);
	t->help_arg = (char *) hlp_arg;
}

void cf_create_int_item(const char *name, const char *help, const char *hlp_arg, char short_opt, int def) {
	CONF_ITEM *t = create_conf_item(name, help, short_opt, NULL);
	t->type = CFT_INT;
	t->data.dt_int.val = def;
	t->data.dt_int.default_val = def;
	t->help_arg = (char *) hlp_arg;
}

void cf_create_array_item(const char *name, const char *help, const char *hlp_arg, char short_opt, int size, int *def) {
	CONF_ITEM *t = create_conf_item(name, help, short_opt, NULL);
	t->type = CFT_ARRAY;
	t->data.dt_array.size = size;
	t->data.dt_array.array = (int*) calloc(1, size * sizeof (int));
	memcpy(t->data.dt_array.array, def, size * sizeof (int));
	t->data.dt_array.default_array = def;
	t->help_arg = (char *) hlp_arg;
}

void cf_create_str_array_item(const char *name, const char *help, const char *hlp_arg, char short_opt, char *def) {
	CONF_ITEM *t = create_conf_item(name, help, short_opt, NULL);
	t->type = CFT_STR_ARRAY;
	t->data.dt_str_array.size = 0; /* Calculated on the fly */
	if (def != NULL)
		t->data.dt_str_array.array = read_str_array(def, &t->data.dt_str_array.size);
	else
		t->data.dt_str_array.array = NULL;
	t->data.dt_str_array.default_array = strdup(def);
	t->help_arg = (char *) hlp_arg;
}

CONF_ITEM* cf_get_item_by_name(const char *name) {
	int i;
	int a = tolower((int) name[0]);

	for (i = 0; i < cf_hash[a].nb_item; i++) {
		if (strcasecmp(cf_hash[a].conf[i]->name, name) == 0)
			return cf_hash[a].conf[i];
	}
	return NULL;
}

CONF_ITEM* cf_get_item_by_val(int val) {
	int i, j;

	for (i = 0; i < 128; i++) {
		for (j = 0; j < cf_hash[i].nb_item; j++) {
			if (cf_hash[i].conf[j]->short_opt == val)
				return cf_hash[i].conf[j];
		}
	}
	return NULL;
}

void cf_reset_all_changed_flag() {
	int i, j;

	for (i = 0; i < 128; i++)
		for (j = 0; j < cf_hash[i].nb_item; j++)
			cf_hash[i].conf[j]->modified=0;
}

void cf_item_has_been_changed(CONF_ITEM * item) {
	if (item)
		item->modified = 1;
}

void cf_print_help(void) {
	int i, j;
	CONF_ITEM *cf;
	printf("Usage: gngeo [OPTION]... ROMSET\n"
			"Emulate the NeoGeo rom designed by ROMSET\n\n");

	for (i = 0; i < 128; i++) {
		for (j = 0; j < cf_hash[i].nb_item; j++) {
			cf = cf_hash[i].conf[j];
			if (cf->short_opt < 128 && cf->short_opt >= 32)
				printf("  -%c, --", cf->short_opt);
			else
				printf("      --");
			switch (cf->type) {
				case CFT_ARRAY:
				case CFT_STR_ARRAY:
				case CFT_STRING:
				case CFT_ACTION_ARG:
				case CFT_INT:
				{
					char buf[22];
					snprintf(buf, 21, "%s=%s", cf->name, cf->help_arg);
					printf("%-20s %s\n", buf, cf->help);
				}
					break;
				case CFT_BOOLEAN:
				case CFT_ACTION:
					printf("%-20s %s\n", cf->name, cf->help);
					break;
			}
		}
	}
	printf("\nAll boolean options can be disabled with --no-OPTION\n"
			"(Ex: --no-sound turn sound off)\n\n");
}

static int print_help(CONF_ITEM *self) {
	cf_print_help();
	return 0;
}

static int show_all_game(CONF_ITEM *self) {
	printf("Not implemented yet\n");
	/*
		dr_load_driver_dir(CF_STR(cf_get_item_by_name("romrcdir")));
	#if ! defined (GP2X) && ! defined (WIN32)
		{
	#if defined (__AMIGA__)
			int len = strlen("romrc.d") + strlen("/PROGDIR/data/") + 1;
			char *rc_dir = (char *) alloca(len*sizeof(char));
			sprintf(rc_dir, "/PROGDIR/data/romrc.d");
	#else
			int len = strlen("romrc.d") + strlen(getenv("HOME")) + strlen("/.gngeo/") +	1;
			char *rc_dir = (char *) alloca(len*sizeof(char));
			sprintf(rc_dir, "%s/.gngeo/romrc.d", getenv("HOME"));
	#endif
			dr_load_driver_dir(rc_dir);
		}
	#endif

		//dr_load_driver(CF_STR(cf_get_item_by_name("romrc")));
		dr_list_all();//list_game();
	 */
	return 0;
}

static int show_version(CONF_ITEM *self) {
	printf("Gngeo %s\n", VERSION);
	printf("Copyright (C) 2001 Peponas Mathieu\n\n");

	return 0;
}

void cf_init(void) {

	cf_create_action_item("help", "Print this help and exit", 'h', print_help);
	cf_create_action_item("listgame", "Show all the game available in the romrc", 'l', show_all_game);
	cf_create_action_item("version", "Show version and exit", 'v', show_version);

	cf_create_bool_item("forcepc", "Force the PC to a correct value at startup", 0, GN_FALSE);
	cf_create_bool_item("dump", "Create a gno dump in the current dir and exit", 0, GN_FALSE);

	cf_create_bool_item("interpolation", "Merge the last frame and the current", 'I', GN_FALSE);
	cf_create_bool_item("raster", "Enable the raster interrupt", 'r', GN_FALSE);
	cf_create_bool_item("sound", "Enable sound", 0, GN_TRUE);
	cf_create_bool_item("showfps", "Show FPS at startup", 0, GN_FALSE);

	cf_create_bool_item("sleepidle", "Sleep when idle", 0, GN_FALSE);
	cf_create_bool_item("joystick", "Enable joystick support", 0, GN_TRUE);
	cf_create_bool_item("debug", "Start with inline debuger", 'D', GN_FALSE);
	cf_create_bool_item("hwsurface", "Use hardware surface for the screen", 'H', GN_TRUE);
#ifdef PANDORA
	cf_create_bool_item("vsync", "Synchronise the display with VBLANK", 0, GN_TRUE);
	cf_create_bool_item("autoframeskip", "Enable auto frameskip", 0, GN_FALSE);
	cf_create_bool_item("fullscreen", "Start gngeo in fullscreen", 'f', GN_TRUE);
	cf_create_bool_item("wide", "Use all the Pandora Screen", 0, GN_FALSE);
#else
	cf_create_bool_item("vsync", "Synchronise the display with VBLANK", 0, GN_FALSE);
	cf_create_bool_item("autoframeskip", "Enable auto frameskip", 0, GN_TRUE);
	cf_create_bool_item("fullscreen", "Start gngeo in fullscreen", 'f', GN_FALSE);
	
#endif
	cf_create_bool_item("pal", "Use PAL timing (buggy)", 'P', GN_FALSE);
	cf_create_bool_item("screen320", "Use 320x224 output screen (instead 304x224)", 0, GN_FALSE);
	cf_create_bool_item("bench", "Draw x frames, then quit and show average fps", 0, GN_FALSE);


	cf_create_string_item("country", "Set the contry to japan, asia, usa or europe", "...", 0, "europe");
	cf_create_string_item("system", "Set the system to home, arcade or unibios", "...", 0, "arcade");
#ifdef EMBEDDED_FS
	cf_create_string_item("rompath", "Tell gngeo where your roms are", "PATH", 'i', ROOTPATH"./roms");
	cf_create_string_item("biospath", "Tell gngeo where your neogeo bios is", "PATH", 'B', ROOTPATH"./roms");
	cf_create_string_item("datafile", "Tell gngeo where his ressource file is", "PATH", 'd', ROOTPATH"./gngeo_data.zip");
#else
	cf_create_string_item("rompath", "Tell gngeo where your roms are", "PATH", 'i', DATA_DIRECTORY);
	cf_create_string_item("biospath", "Tell gngeo where your neogeo bios is", "PATH", 'B', DATA_DIRECTORY);
	cf_create_string_item("datafile", "Tell gngeo where his ressource file is", "PATH", 'd', DATA_DIRECTORY"/gngeo_data.zip");
#endif
	//cf_create_string_item("romrcdir","Use STRING as romrc.d directory",0,DATA_DIRECTORY"/romrc.d");
	cf_create_string_item("libglpath", "Path to your libGL.so", "PATH", 0, "/usr/lib/libGL.so");
	cf_create_string_item("effect", "Use the specified effect (help for a list)", "Effect", 'e', "none");
	cf_create_string_item("blitter", "Use the specified blitter (help for a list)", "Blitter", 'b', "soft");
	cf_create_string_item("transpack", "Use the specified transparency pack", "Transpack", 't', "none");
	cf_create_string_item("p1control", "Player1 control configutation", "...", 0, default_p1control);
	cf_create_string_item("p2control", "Player2 control configutation", "...", 0, default_p2control);
/*
#if defined(GP2X) || defined(WIZ)
	cf_create_string_item("p1control", "Player1 control configutation", "...", 0,
			"UP=J0B0,DOWN=J0B4,LEFT=J0B2,RIGHT=J0B6,A=J0B14,B=J0B13,C=J0B12,D=J0B15,COIN=J0B9,START=J0B8,HOTKEY1=J0B10,HOTKEY2=J0B11");
	cf_create_string_item("p2control", "Player2 control configutation", "...", 0, "");
#elif defined(PANDORA)
	cf_create_string_item("p1control", "Player1 control configutation", "...", 0,
			"A=K281,B=K279,C=K278,D=K280,START=K308,COIN=K306,UP=K273,DOWN=K274,LEFT=K276,RIGHT=K275,MENU=K113");
	cf_create_string_item("p2control", "Player2 control configutation", "...", 0, "");
#elif defined (DINGUX)
	cf_create_string_item("p1control", "Player1 control configutation", "...", 0,
			"A=K308,B=K306,C=K304,D=K32,START=K13,COIN=K9,UP=K273,DOWN=K274,LEFT=K276,RIGHT=K275,MENU=K113");
	cf_create_string_item("p2control", "Player2 control configutation", "...", 0, "");
#else
	
	cf_create_string_item("p1control", "Player1 control configutation", "...", 0,
			"A=K119,B=K120,C=K113,D=K115,START=K38,COIN=K34,UP=K273,DOWN=K274,LEFT=K276,RIGHT=K275,MENU=K27");
	cf_create_string_item("p2control", "Player2 control configutation", "...", 0, "");
#endif
*/

	cf_create_array_item("p1hotkey0", "Player1 Hotkey 0 configuration", "...", 0, 4, default_p1hotkey0);
	cf_create_array_item("p1hotkey1", "Player1 Hotkey 1 configuration", "...", 0, 4, default_p1hotkey1);
	cf_create_array_item("p1hotkey2", "Player1 Hotkey 2 configuration", "...", 0, 4, default_p1hotkey2);
	cf_create_array_item("p1hotkey3", "Player1 Hotkey 3 configuration", "...", 0, 4, default_p1hotkey3);
	cf_create_array_item("p2hotkey0", "Player2 Hotkey 0 configuration", "...", 0, 4, default_p2hotkey0);
	cf_create_array_item("p2hotkey1", "Player2 Hotkey 1 configuration", "...", 0, 4, default_p2hotkey1);
	cf_create_array_item("p2hotkey2", "Player2 Hotkey 2 configuration", "...", 0, 4, default_p2hotkey2);
	cf_create_array_item("p2hotkey3", "Player2 Hotkey 3 configuration", "...", 0, 4, default_p2hotkey3);



	cf_create_int_item("scale", "Scale the resolution by X", "X", 0, 1);
	cf_create_int_item("samplerate", "Set the sample rate to RATE", "RATE", 0, 22050);
	cf_create_int_item("68kclock", "Overclock the 68k by x% (-x% for underclk)", "x", 0, 0);
	cf_create_int_item("z80clock", "Overclock the Z80 by x% (-x% for underclk)", "x", 0, 0);


#ifdef GP2X
	cf_create_bool_item("ramhack", "Enable CraigX's RAM timing hack", 0, GN_FALSE);
	cf_create_bool_item("tvout", "Enable Tvout (NTSC)", 0, GN_FALSE);
	cf_create_array_item("tv_offset", "Shift TV screen by x,y pixel", "x,y", 0, 2, default_tvoffset);
	cf_create_bool_item("940sync", "Accurate synchronisation between the both core", 0, GN_TRUE);
	cf_create_int_item("cpu_speed", "Overclock the GP2X cpu to x Mhz", "x", 0, 0);
	cf_create_string_item("frontend", "Execute CMD when exit. Usefull to return to Selector or Rage2x", "CMD", 0, "/usr/gp2x/gp2xmenu");
#endif
	//CF_SYSTEMOPT
	cf_get_item_by_name("rompath")->flags|=CF_SYSTEMOPT;
	cf_get_item_by_name("libglpath")->flags|=CF_SYSTEMOPT;
	cf_get_item_by_name("datafile")->flags|=CF_SYSTEMOPT;
}

/* TODO: lame, do it better */
int discard_line(char *buf) {
	if (buf[0] == '#')
		return GN_TRUE;
	if (buf[0] == '\n')
		return GN_TRUE;
	if (buf[0] == 0)
		return GN_TRUE;

	return GN_FALSE;
}

/* like standard fgets, but work with unix/dos line ending */
char *my_fgets(char *s, int size, FILE *stream) {
	int i = 0;
	int ch;
	while (i < size && !feof(stream)) {
		ch = fgetc(stream); //printf("ch=%d\n",ch);
		if (ch == 0x0D) continue;
		if (ch == 0x0A) {
			s[i] = 0;
			return s;
		}
		s[i] = ch;
		i++;
	}
	return s;
}



int cf_save_option(char *filename, char *optname,int flags) {
	char *conf_file = filename;
	char *conf_file_dst;
	FILE *f;
	FILE *f_dst;
	int i = 0, j, a;
	char buf[512];
	char name[32];
	char val[255];
	CONF_ITEM *cf;
	CONF_ITEM *tosave; //cf_get_item_by_name(optname);

	if (!conf_file) {
#ifdef EMBEDDED_FS
		int len = strlen("gngeorc") + strlen(ROOTPATH"conf/") + 1;
		conf_file = (char *) alloca(len * sizeof (char));
		sprintf(conf_file, ROOTPATH"conf/gngeorc");
#elif __AMIGA__
		int len = strlen("gngeorc") + strlen("/PROGDIR/data/") + 1;
		conf_file = (char *) alloca(len * sizeof (char));
		sprintf(conf_file, "/PROGDIR/data/gngeorc");
#else /* POSIX */
		int len = strlen("gngeorc") + strlen(getenv("HOME")) + strlen("/.gngeo/") + 1;
		conf_file = (char *) alloca(len * sizeof (char));
		sprintf(conf_file, "%s/.gngeo/gngeorc", getenv("HOME"));
#endif
	}
	conf_file_dst = alloca(strlen(conf_file) + 4);
	sprintf(conf_file_dst, "%s.t", conf_file);

	if ((f_dst = fopen(conf_file_dst, "w")) == 0) {
		//printf("Unable to open %s\n",conf_file);
		return GN_FALSE;
	}
	if (optname!=NULL) {
		tosave=cf_get_item_by_name(optname);
		if (tosave) cf_item_has_been_changed(tosave);
	} else tosave=NULL;

	if ((f = fopen(conf_file, "rb"))) {

		//printf("Loading current .cf\n");

		while (!feof(f)) {
			i = 0;
			my_fgets(buf, 510, f);
			if (discard_line(buf)) {
				fprintf(f_dst, "%s\n", buf);
				continue;
			}

			//sscanf(buf, "%s %s", name, val);
			sscanf(buf, "%s ", name);
			strncpy(val, buf + strlen(name) + 1, 254);

			cf = cf_get_item_by_name(name);
			if (cf && (cf==tosave || tosave==NULL)) {
				if (cf->modified) {
					cf->modified = 0;
					switch (cf->type) {
						case CFT_INT:
							fprintf(f_dst, "%s %d\n", cf->name, CF_VAL(cf));
							break;
						case CFT_BOOLEAN:
							if (CF_BOOL(cf))
								fprintf(f_dst, "%s true\n", cf->name);
							else
								fprintf(f_dst, "%s false\n", cf->name);
							break;
						case CFT_STRING:
							fprintf(f_dst, "%s %s\n", cf->name, CF_STR(cf));
							break;
						case CFT_ARRAY:
							fprintf(f_dst, "%s ", cf->name);
							for (a = 0; a < CF_ARRAY_SIZE(cf) - 1; a++)
								fprintf(f_dst, "%d,", CF_ARRAY(cf)[a]);
							fprintf(f_dst, "%d\n", CF_ARRAY(cf)[a]);
							break;
						case CFT_ACTION:
						case CFT_ACTION_ARG:
							break;
						case CFT_STR_ARRAY:
							printf("TODO: Save CFT_STR_ARRAY\n");
							break;
					}
				} else
					fprintf(f_dst, "%s\n", buf);
			}
		}
		fclose(f);

	}
	/* Now save options that were not in the previous file */
	for (i = 0; i < 128; i++) {
		for (j = 0; j < cf_hash[i].nb_item; j++) {
			cf = cf_hash[i].conf[j];
			//printf("Option %s %d\n",cf->name,cf->modified);
			if (cf->modified!=0  && (cf==tosave || tosave==NULL)) {
				cf->modified=0;
				switch (cf->type) {
					case CFT_INT:
						fprintf(f_dst, "%s %d\n", cf->name, CF_VAL(cf));
						break;
					case CFT_BOOLEAN:
						if (CF_BOOL(cf))
							fprintf(f_dst, "%s true\n", cf->name);
						else
							fprintf(f_dst, "%s false\n", cf->name);
						break;
					case CFT_STRING:
						fprintf(f_dst, "%s %s\n", cf->name, CF_STR(cf));
						break;
					case CFT_ARRAY:
						fprintf(f_dst, "%s ", cf->name);
						for (a = 0; a < CF_ARRAY_SIZE(cf) - 1; a++)
							fprintf(f_dst, "%d,", CF_ARRAY(cf)[a]);
						fprintf(f_dst, "%d\n", CF_ARRAY(cf)[a]);
						break;
					case CFT_ACTION:
					case CFT_ACTION_ARG:
						/* action are not available in the conf file */
						break;
					case CFT_STR_ARRAY:
						printf("TODO: Save CFT_STR_ARRAY\n");
						break;
				}
			}
		}
	}
	fclose(f_dst);

	remove(conf_file);
	rename(conf_file_dst, conf_file);

	return GN_TRUE;
}

int cf_save_file(char *filename, int flags) {
	return cf_save_option(filename,NULL,flags);
}

void cf_reset_to_default(void) {
	int i,j;
	CONF_ITEM *cf;
	for (i = 0; i < 128; i++) {
		for (j = 0; j < cf_hash[i].nb_item; j++) {
			cf = cf_hash[i].conf[j];
			if (!cf->modified && !(cf->flags & CF_SETBYCMD) &&!(cf->flags & CF_SYSTEMOPT)) {
				switch (cf->type) {
					case CFT_INT:
						CF_VAL(cf) = cf->data.dt_int.default_val;
						break;
					case CFT_BOOLEAN:
						CF_BOOL(cf) = cf->data.dt_bool.default_bool;
						break;
					case CFT_STRING:
						strncpy(CF_STR(cf), cf->data.dt_str.default_str, 254);
						break;
					case CFT_ARRAY:
						memcpy(cf->data.dt_array.array, cf->data.dt_array.default_array,
								CF_ARRAY_SIZE(cf) * sizeof (int));
						//read_array(CF_ARRAY(cf), val, CF_ARRAY_SIZE(cf));
						break;
					default:
						break;
				}
			}
		}
	}
}

int cf_open_file(char *filename) {
	/* if filename==NULL, we use the default one: $HOME/.gngeo/gngeorc */
	char *conf_file = filename;
	FILE *f;
	int i = 0;
	char buf[512];
	char name[32];
	char val[255];
	CONF_ITEM *cf;

	if (!conf_file) {
#ifdef EMBEDDED_FS
		int len = strlen("gngeorc") + strlen(ROOTPATH"conf/") + 1;
		conf_file = (char *) alloca(len * sizeof (char));
		sprintf(conf_file, ROOTPATH"conf/gngeorc");
#elif __AMIGA__
		int len = strlen("gngeorc") + strlen("/PROGDIR/data/") + 1;
		conf_file = (char *) alloca(len * sizeof (char));
		sprintf(conf_file, "/PROGDIR/data/gngeorc");
#else
		int len = strlen("gngeorc") + strlen(getenv("HOME")) + strlen("/.gngeo/") + 1;
		conf_file = (char *) alloca(len * sizeof (char));
		sprintf(conf_file, "%s/.gngeo/gngeorc", getenv("HOME"));
#endif
	}
	if ((f = fopen(conf_file, "rb")) == 0) {
		//printf("Unable to open %s\n",conf_file);
		return GN_FALSE;
	}

	while (!feof(f)) {
		i = 0;
		my_fgets(buf, 510, f);
		if (discard_line(buf))
			continue;
	
		/* TODO: Verify this on Win32 */
		sscanf(buf, "%s %s", name, val);

		//sscanf(buf, "%s ", name);
		//strncpy(val,buf+strlen(name)+1,254);

		//printf("%s|%s|\n",name,val);
		cf = cf_get_item_by_name(name);
		if (cf && !(cf->flags & CF_SETBYCMD) && (!cf->modified)) {
			printf("Option %s\n",cf->name);
			switch (cf->type) {
				case CFT_INT:
					CF_VAL(cf) = atoi(val);
					break;
				case CFT_BOOLEAN:
					CF_BOOL(cf) = (strcasecmp(val, "true") == 0 ? GN_TRUE : GN_FALSE);
					break;
				case CFT_STRING:
					strncpy(CF_STR(cf), val, 254);
					break;
				case CFT_ARRAY:
					read_array(CF_ARRAY(cf), val, CF_ARRAY_SIZE(cf));
					break;
				case CFT_ACTION:
				case CFT_ACTION_ARG:
					/* action are not available in the conf file */
					break;
				case CFT_STR_ARRAY:
					CF_STR_ARRAY(cf) = read_str_array(val, &CF_STR_ARRAY_SIZE(cf));
					break;
			}
		} else {
			/*printf("Unknow option %s\n",name);*/
			/* unknow option...*/
		}
	}

	cf_cache_conf();
	return GN_TRUE;
}


static struct option *longopt;
//static struct option *fake_longopt;

static void add_long_opt_item(char *name, int has_arg, int *flag, int val) {
	static int opt_size = 0;
	static int opt = 0;

	if (opt >= opt_size) {
		opt_size += 10;
		longopt = realloc(longopt, (opt_size + 1) * sizeof (struct option));
		//fake_longopt=realloc(fake_longopt,(opt_size+1)*sizeof(struct option));
	}

	longopt[opt].name = name;
	longopt[opt].has_arg = has_arg;
	longopt[opt].flag = flag;
	longopt[opt].val = val;

	/*
		fake_longopt[opt].name=name;
		fake_longopt[opt].has_arg=has_arg;
		fake_longopt[opt].flag=NULL;
		fake_longopt[opt].val=0;
	 */
	opt++;
}

int option_index = 0;
static char shortopt[255];

void cf_init_cmd_line(void) {
	int i, j;
	CONF_ITEM *cf;
	char *sbuf;
	int buflen;

	memset(shortopt, 0, 255);

	for (i = 0; i < 128; i++) {
		for (j = 0; j < cf_hash[i].nb_item; j++) {
			cf = cf_hash[i].conf[j];

			if (cf->short_opt <= 128) {
				char b[2];
				sprintf(b, "%c", cf->short_opt);
				strcat(shortopt, b);
			}

			switch (cf->type) {
				case CFT_ARRAY:
				case CFT_STR_ARRAY:
				case CFT_STRING:
				case CFT_INT:
				case CFT_ACTION_ARG:
					if (cf->short_opt <= 128) strcat(shortopt, ":");
					add_long_opt_item(cf->name, 1, NULL, cf->short_opt);
					break;
				case CFT_BOOLEAN:
					//add_long_opt_item(cf->name, 0, &CF_BOOL(cf), 1);
					add_long_opt_item(cf->name, 0, NULL, cf->short_opt);
					/* create the --no-option */
					buflen = strlen("no-") + strlen(cf->name);
					sbuf = malloc(buflen + 2);
					snprintf(sbuf, buflen + 1, "no-%s", cf->name);
					//add_long_opt_item(sbuf, 0, &CF_BOOL(cf), 0);
					add_long_opt_item(sbuf, 0, NULL, cf->short_opt+0x1000);
					break;
				case CFT_ACTION:
					add_long_opt_item(cf->name, 0, NULL, cf->short_opt);
					break;
			}
		}/* for j*/
	}/* for i*/

	/* end the longopt array*/
	add_long_opt_item(0, 0, 0, 0);

}

char* cf_parse_cmd_line(int argc, char *argv[]) {
	int c;
	CONF_ITEM *cf;


	option_index = optind = 0;
#ifdef WII
	return NULL;
#endif
	while ((c = getopt_long(argc, argv, shortopt, longopt, &option_index)) != EOF) {
		//if (c != 0) {
			printf("c=%d\n",c);
			cf = cf_get_item_by_val(c&0xFFF);
			if (cf) {
				cf->flags |= CF_SETBYCMD;
				printf("flags %s set on cmd line\n", cf->name);
				switch (cf->type) {

					case CFT_INT:
						CF_VAL(cf) = atoi(optarg);
						break;
					case CFT_BOOLEAN:
					if (c & 0x1000)
						CF_BOOL(cf) = 0;
					else
						CF_BOOL(cf) = 1;
						break;
					case CFT_STRING:
						strcpy(CF_STR(cf), optarg);
						//printf("conf %s %s\n",CF_STR(cf),optarg);
						break;
					case CFT_ARRAY:
						read_array(CF_ARRAY(cf), optarg, CF_ARRAY_SIZE(cf));
						break;
					case CFT_ACTION_ARG:
						strcpy(CF_STR(cf), optarg);
						if (cf->action) {
							exit(cf->action(cf));
						}
						break;
					case CFT_ACTION:
						if (cf->action) {
							exit(cf->action(cf));
						}
						break;
					case CFT_STR_ARRAY:
						/* TODO */
						break;
				}
			//}
		}
	}
	cf_cache_conf();
	if (optind >= argc)
		return NULL;

	return strdup(argv[optind]);
}

