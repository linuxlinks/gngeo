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


#include "SDL.h"
#include "conf.h"
#include "fileio.h"
#include "driver.h"
#include "emu.h"
#include "fileio.h"

/*
static CONF_ITEM *cf_hash[128];
static cf_hash_size[128];
*/

static struct {
    CONF_ITEM **conf;
    int size,nb_item;
}cf_hash[128];

/*
static int default_key1[] = { 119, 120, 113, 115, 38, 34, 273, 274, 276, 275 };
static int default_key2[] = { 108, 109, 111, 112, 233, 39, 264, 261, 260, 262 };
*/
/* Now the default conf is for qwerty keyboard */
static int default_key1[] = { 122, 120, 97 , 115, 49, 51, 273, 274, 276, 275, -1, -1, -1, -1 };
static int default_key2[] = { 108, 59 , 111, 112, 50, 52, 264, 261, 260, 262, -1, -1, -1, -1 };
static int default_joy1[] = { 2, 3, 0, 1, 5, 4, 0, 1, 1, 1, -1, -1, -1, -1 };
static int default_joy2[] = { 1, 0, 3, 2, 7, 6, 0, 1, 1, 1, -1, -1, -1, -1 };
static int default_p1hotkey0[] = { 0,0,0,0 };
static int default_p1hotkey1[] = { 0,0,0,0 };
static int default_p1hotkey2[] = { 0,0,0,0 };
static int default_p1hotkey3[] = { 0,0,0,0 };
static int default_p2hotkey0[] = { 0,0,0,0 };
static int default_p2hotkey1[] = { 0,0,0,0 };
static int default_p2hotkey2[] = { 0,0,0,0 };
static int default_p2hotkey3[] = { 0,0,0,0 };

void cf_cache_conf(void) {
	char *country;
	char *system;
    /* cache some frequently used conf item */
    conf.sound=CF_BOOL(cf_get_item_by_name("sound"));
    conf.sample_rate=CF_BOOL(cf_get_item_by_name("samplerate"));
    conf.debug=CF_BOOL(cf_get_item_by_name("debug"));
    conf.raster=CF_BOOL(cf_get_item_by_name("raster"));
    conf.pal=CF_BOOL(cf_get_item_by_name("pal"));
    country=CF_STR(cf_get_item_by_name("country"));
    system=CF_STR(cf_get_item_by_name("system"));
    if (!strcmp(system,"unibios")) {
        conf.system=SYS_UNIBIOS;
    } else {
	    if (!strcmp(system,"home")) {
		    conf.system=SYS_HOME;
	    } else {
		    conf.system=SYS_ARCADE;
	    }
    }
    if (!strcmp(country,"japan")) {
        conf.country=CTY_JAPAN;
    } else if (!strcmp(country,"usa")) {
        conf.country=CTY_USA;
    } else if (!strcmp(country,"asia")) {
        conf.country=CTY_ASIA;
    } else {
        conf.country=CTY_EUROPE;
    }
}


static CONF_ITEM * create_conf_item(char *name,char *help,char short_opt,int (*action)(struct CONF_ITEM *self))
{
    int a;
    static int val=0x100;
    CONF_ITEM *t=(CONF_ITEM*)calloc(1,sizeof(CONF_ITEM));
    
    a=tolower ((int)name[0]);

    t->name=name;
    t->help=help;
    if (short_opt==0) {
	val++;
	t->short_opt=val;
    } else
	t->short_opt=short_opt;

    if (action) {
	t->action=action;
    }

    
    if (cf_hash[a].size<=cf_hash[a].nb_item) {
	cf_hash[a].size+=10;
	cf_hash[a].conf=(CONF_ITEM**)realloc(cf_hash[a].conf,cf_hash[a].size*sizeof(CONF_ITEM*));
    }

    cf_hash[a].conf[cf_hash[a].nb_item]=t;
    cf_hash[a].nb_item++;
    return t;
}

void cf_create_bool_item(char *name,char *help,char short_opt,SDL_bool def)
{
    CONF_ITEM *t=create_conf_item(name,help,short_opt,NULL);
    t->type=CFT_BOOLEAN;
    t->data.dt_bool.bool=def;
    t->data.dt_bool.default_bool=def;
}
void cf_create_action_item(char *name,char *help,char short_opt,int (*action)(struct CONF_ITEM *self))
{
    CONF_ITEM *t=create_conf_item(name,help,short_opt,action);
    t->type=CFT_ACTION;
}
void cf_create_action_arg_item(char *name,char *help,char short_opt,int (*action)(struct CONF_ITEM *self))
{
    CONF_ITEM *t=create_conf_item(name,help,short_opt,action);
    t->type=CFT_ACTION_ARG;
}

void cf_create_string_item(char *name,char *help,char short_opt,char *def)
{
    CONF_ITEM *t=create_conf_item(name,help,short_opt,NULL);
    t->type=CFT_STRING;
    strcpy(t->data.dt_str.str,def);
    t->data.dt_str.default_str=def;
}

void cf_create_int_item(char *name,char *help,char short_opt,int def)
{
    CONF_ITEM *t=create_conf_item(name,help,short_opt,NULL);
    t->type=CFT_INT;
    t->data.dt_int.val=def;
    t->data.dt_int.default_val=def;
}

void cf_create_array_item(char *name,char *help,char short_opt,int size,int *def)
{
    CONF_ITEM *t=create_conf_item(name,help,short_opt,NULL);
    t->type=CFT_ARRAY;
    t->data.dt_array.size=size;
    t->data.dt_array.array=(int*)calloc(1,size*sizeof(int));
    memcpy(t->data.dt_array.array,def,size*sizeof(int));
    t->data.dt_array.default_array=def;
}

CONF_ITEM* cf_get_item_by_name(char *name){
    int i;
    int a=tolower ((int)name[0]);

    for(i=0;i<cf_hash[a].nb_item;i++) {
	if (strcasecmp(cf_hash[a].conf[i]->name,name)==0)
	    return cf_hash[a].conf[i];
    }
    return NULL;
}

CONF_ITEM* cf_get_item_by_val(int val){
    int i,j;

    for(i=0;i<128;i++) {
	for(j=0;j<cf_hash[i].nb_item;j++) {
	    if (cf_hash[i].conf[j]->short_opt==val)
		return cf_hash[i].conf[j];
	}
    }
    return NULL;
}

void cf_print_help(void) {
    int i,j;
    CONF_ITEM *cf;
    printf("Usage: gngeo [OPTION]... ROMSET\n"
	   "Emulate the NeoGeo rom designed by ROMSET\n\n");
/*
    printf("  -h, --help                 Print this help and exit\n"
	   "  -l, --listgame             List all the available game\n");
*/
    for (i=0;i<128;i++) {
	for(j=0;j< cf_hash[i].nb_item;j++) {
	    cf=cf_hash[i].conf[j];
	    if (cf->short_opt<128 && cf->short_opt>=32)
		printf("  -%c, --",cf->short_opt);
	    else
		printf("      --");
	    switch (cf->type) {
	    case CFT_ARRAY:
	    {
		char buf[22];
		snprintf(buf,21,"%s=ARRAY",cf->name);
		printf("%-20s %s\n",buf,cf->help);
	    }
	    break;
	    case CFT_STRING:
	    case CFT_ACTION_ARG:
	    {
		char buf[22];
		snprintf(buf,21,"%s=STRING",cf->name);
		printf("%-20s %s\n",buf,cf->help);
	    }
	    break;
	    case CFT_INT:
	    {
		char buf[22];
		snprintf(buf,21,"%s=N",cf->name);
		printf("%-20s %s\n",buf,cf->help);
	    }
		break;
	    case CFT_BOOLEAN:
	    case CFT_ACTION:
		printf("%-20s %s\n",cf->name,cf->help);
		break;
	    }
	}
    }
    printf("\nAll boolean options can be disabled with --no-OPTION\n"
	   "(Ex: --no-sound turn sound off)\n\n");
}

static  int print_help(CONF_ITEM *self) {
    cf_print_help();
    return 0;
}

static int show_all_game(CONF_ITEM *self) {
    dr_load_driver_dir(CF_STR(cf_get_item_by_name("romrcdir")));
    //dr_load_driver(CF_STR(cf_get_item_by_name("romrc")));
    dr_list_all();//list_game();
    return 0;
}

static int show_version(CONF_ITEM *self) {
    printf("Gngeo %s\n",VERSION);
    printf("Copyright (C) 2001 Peponas Mathieu\n\n");

    return 0;
}

static int scan_dir(CONF_ITEM *self) {
#ifdef HAVE_SCANDIR
    int nbf,i;
    char filename[strlen(CF_STR(self))+256];
    struct stat filestat;
    struct dirent **namelist;

    //dr_load_driver(CF_STR(cf_get_item_by_name("romrc")));
    dr_load_driver_dir(CF_STR(cf_get_item_by_name("romrcdir")));


    printf("ShortName:LongName:Filename\n");
    if ((nbf = scandir(CF_STR(self), &namelist, NULL, alphasort )) != -1) {
	for (i = 0; i < nbf; i++) {
	    sprintf(filename, "%s/%s", CF_STR(self), namelist[i]->d_name);
	    lstat(filename, &filestat);
	    if (!S_ISDIR(filestat.st_mode)) {
		DRIVER *dr;
		if ((dr=get_driver_for_zip(filename))!=NULL) {
		    printf("%8s:%s:%s\n",dr->name,dr->longname,namelist[i]->d_name);
		}
	    }
	}
    }
#else
    printf("TODO: Scandir is unavailable on your platform, sorry :(\n");
#endif
    return 0;
}

void cf_init(void)
{
    CONF_ITEM *t;

    cf_create_action_item("help","Print this help and exit",'h',print_help);
    cf_create_action_item("listgame","Show all the game available in the romrc",'l',show_all_game);
    cf_create_action_item("version","Show version and exit",'v',show_version);
    cf_create_action_arg_item("scandir","Scan the given directory, and show available rom",0,scan_dir);
    cf_create_bool_item("forcepc","Force the PC to a correct value at startup",0,SDL_FALSE);
    cf_create_bool_item("fullscreen","Start gngeo in fullscreen",'f',SDL_FALSE);
    cf_create_bool_item("interpolation","Merge the last frame and the current",'I',SDL_FALSE);
    cf_create_bool_item("raster","Enable the raster interrupt",'r',SDL_FALSE);
    cf_create_bool_item("sound","Enable sound",0,SDL_TRUE);
    cf_create_bool_item("showfps","Show FPS at startup",0,SDL_FALSE);
    cf_create_bool_item("autoframeskip","Enable auto frameskip",0,SDL_TRUE);
    cf_create_bool_item("sleepidle","Sleep when idle",0,SDL_FALSE);
    cf_create_bool_item("joystick","Enable joystick support",0,SDL_TRUE);
    //cf_create_bool_item("invertjoy","Invert joystick order",0,SDL_FALSE);
    cf_create_bool_item("debug","Start with inline debuger",'D',SDL_FALSE);
    cf_create_bool_item("hwsurface","Use hardware surface for the screen",'H',SDL_FALSE);
    //cf_create_bool_item("convtile","Convert tile in internal format at loading",'c',SDL_TRUE);
    cf_create_bool_item("pal","Use PAL timing (buggy)",'P',SDL_FALSE);
    cf_create_bool_item("screen320","Use 320x224 output screen (instead 304x224)",0,SDL_TRUE);
    cf_create_bool_item("bench","Draw x frames, then quit and show average fps",0,SDL_FALSE);
/*
#ifdef GP2X
    cf_create_bool_item("selector","Go back to selector when exit",0,SDL_TRUE);
#endif
*/

    cf_create_string_item("country","Set the contry to japan, asia, usa or europe",0,"europe");
    cf_create_string_item("system","Set the system to home or arcade",0,"arcade");
    cf_create_string_item("rompath","Use STRING as rom path",'i',DATA_DIRECTORY);
    cf_create_string_item("biospath","Use STRING as bios path",'B',DATA_DIRECTORY);
    cf_create_string_item("romrc","Use STRING as romrc file",'d',DATA_DIRECTORY"/romrc");
    cf_create_string_item("romrcdir","Use STRING as romrc.d directory",0,DATA_DIRECTORY"/romrc.d");
    cf_create_string_item("libglpath","Use STRING as libGL",0,"/usr/lib/libGL.so");
    cf_create_string_item("effect","Use the specified effect (help for a list)",'e',"none");
    cf_create_string_item("blitter","Use the specified blitter (help for a list)",'b',"soft");
    cf_create_string_item("transpack","Use the specified transparency pack",'t',"none");
#ifdef GP2X
    cf_create_string_item("frontend","Execute STRING when exit. Usefull to return to Selector or Rage2x",0,"./gngeo2x.gpe");
#endif
   
    cf_create_array_item("p1key","Player1 Keyboard configuration",0,14,default_key1);
    cf_create_array_item("p2key","Player2 Keyboard configuration",0,14,default_key2);
    cf_create_array_item("p1joy","Player1 Joystick configuration",0,14,default_joy1);
    cf_create_array_item("p2joy","Player2 Joystick configuration",0,14,default_joy2);

    cf_create_array_item("p1hotkey0","Player1 Hotkey 0 configuration",0,4,default_p1hotkey0);
    cf_create_array_item("p1hotkey1","Player1 Hotkey 1 configuration",0,4,default_p1hotkey1);
    cf_create_array_item("p1hotkey2","Player1 Hotkey 2 configuration",0,4,default_p1hotkey2);
    cf_create_array_item("p1hotkey3","Player1 Hotkey 3 configuration",0,4,default_p1hotkey3);
    cf_create_array_item("p2hotkey0","Player2 Hotkey 0 configuration",0,4,default_p2hotkey0);
    cf_create_array_item("p2hotkey1","Player2 Hotkey 1 configuration",0,4,default_p2hotkey1);
    cf_create_array_item("p2hotkey2","Player2 Hotkey 2 configuration",0,4,default_p2hotkey2);
    cf_create_array_item("p2hotkey3","Player2 Hotkey 3 configuration",0,4,default_p2hotkey3);

    cf_create_int_item("scale","Scale the resolution by N",0,1);
    cf_create_int_item("samplerate","Set the sample rate to N",0,22050);
    cf_create_int_item("68kclock","Overclock the 68k by N% (-N% for underclk)",0,0);
    cf_create_int_item("z80clock","Overclock the Z80 by N% (-N% for underclk)",0,0);

    cf_create_int_item("p1joydev","Device index for p1joy (0 -> /dev/js0, etc.)",0,0);
    cf_create_int_item("p2joydev","Device index for p2joy",0,1);
#ifdef GP2X
    cf_create_int_item("cpu_speed","Overclock the GP2X cpu to N Mhz",0,0);
#endif

}

SDL_bool discard_line(char *buf)
{
    if (buf[0] == '#')
	return SDL_TRUE;
    if (buf[0] == '\n')
	return SDL_TRUE;
    if (buf[0] == 0)
	return SDL_TRUE;
    
    return SDL_FALSE;
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

SDL_bool cf_open_file(char *filename)
{
    /* if filename==NULL, we use the default one: $HOME/.gngeo/gngeorc */
    char *conf_file=filename;
    FILE *f;
    int i=0;
    char buf[512];
    char name[32];
    char val[255];
    CONF_ITEM *cf;

    if (!conf_file) {
	int len = strlen("gngeorc") + strlen(getenv("HOME")) + strlen("/.gngeo/") +	1;
	conf_file = (char *) alloca(len*sizeof(char));
	sprintf(conf_file, "%s/.gngeo/gngeorc", getenv("HOME"));
    }
    if ((f = fopen(conf_file, "rb")) == 0) {
	//printf("Unable to open %s\n",conf_file);
	return SDL_FALSE;
    }

    while (!feof(f)) {
	i = 0;
	my_fgets(buf, 510, f);
	if (discard_line(buf))
	    continue;
	sscanf(buf, "%s %s", name, val);
	cf=cf_get_item_by_name(name);
	if (cf) {
	    /*printf("Option %s\n",cf->name);*/
	    switch(cf->type) {
	    case CFT_INT:
		CF_VAL(cf)=atoi(val);
		break;
	    case CFT_BOOLEAN:
		CF_BOOL(cf)=(strcasecmp(val,"true")==0?SDL_TRUE:SDL_FALSE);
		break;
	    case CFT_STRING:
		strncpy(CF_STR(cf),val,254);
		break;
	    case CFT_ARRAY:
		read_array(CF_ARRAY(cf),val,CF_ARRAY_SIZE(cf));
		break;
	    case CFT_ACTION:
	    case CFT_ACTION_ARG:
		/* action are not available in the conf file */
		break;
	    }
	} else {
	    /*printf("Unknow option %s\n",name);*/
	    /* unknow option...*/
	}
    }

    cf_cache_conf();
    return SDL_TRUE;
}


static struct option *longopt;
static struct option *fake_longopt;

static void add_long_opt_item(char *name,int has_arg,int *flag,int val) {
    static int opt_size=0;
    static int opt=0;

    if (opt>=opt_size) {
	opt_size+=10;
	longopt=realloc(longopt,(opt_size+1)*sizeof(struct option));
	fake_longopt=realloc(fake_longopt,(opt_size+1)*sizeof(struct option));
    }
    
    longopt[opt].name=name;
    longopt[opt].has_arg=has_arg;
    longopt[opt].flag=flag;
    longopt[opt].val=val;

    fake_longopt[opt].name=name;
    fake_longopt[opt].has_arg=has_arg;
    fake_longopt[opt].flag=NULL;
    fake_longopt[opt].val=0;
    opt++;
}

int option_index = 0;
static char shortopt[255];
void cf_init_cmd_line(void) {
    int i,j;
    CONF_ITEM *cf;
    char *sbuf;
    int buflen;

    memset(shortopt,0,255);
    
    for (i=0;i<128;i++) {
	for(j=0;j< cf_hash[i].nb_item;j++) {
	    cf=cf_hash[i].conf[j];
	    
	    if (cf->short_opt<=128) {
		char b[2];
		sprintf(b,"%c",cf->short_opt);
		strcat(shortopt,b);
	    }

	    switch(cf->type) {
	    case CFT_ARRAY:
	    case CFT_STRING:
	    case CFT_INT:
	    case CFT_ACTION_ARG:
		if (cf->short_opt<=128) strcat(shortopt,":");
		add_long_opt_item(cf->name,1,NULL,cf->short_opt);
	    break;
	    case CFT_BOOLEAN:
		add_long_opt_item(cf->name,0,&CF_BOOL(cf),1);
		/* create the --no-option */
		buflen=strlen("no-")+strlen(cf->name);
		sbuf=malloc(buflen+2);
		snprintf(sbuf,buflen+1,"no-%s",cf->name);
		add_long_opt_item(sbuf,0,&CF_BOOL(cf),0);
		break;
	    case CFT_ACTION:
		add_long_opt_item(cf->name,0,NULL,cf->short_opt);
		break;
	    }
	}/* for j*/
    }/* for i*/
    
/*
    add_long_opt_item("help",0,NULL,'h');
    add_long_opt_item("listgame",0,NULL,'l');
    strcat(shortopt,"hl");
*/

    /* end the longopt array*/
    add_long_opt_item(0,0,0,0);
    //printf("shortopt:%s\n",shortopt);
}

/*
int cf_get_non_opt_index(int argc, char *argv[]) {
    int c,a;
    option_index=optind=0;
    while((c=getopt_long(argc,argv,shortopt,fake_longopt, &option_index))!=EOF) {
    }
    a=optind;

    option_index=optind=0;
    return a;
}
*/

char* cf_parse_cmd_line(int argc, char *argv[]) {

    
    //int opt_size=10,opt=-1;
    int i,j,c;
    CONF_ITEM *cf;
 

    option_index=optind=0;

    while((c=getopt_long(argc,argv,shortopt,longopt, &option_index))!=EOF) {
	if (c!=0) {
	    cf=cf_get_item_by_val(c);
	    if (cf) {
		switch(cf->type) {
		case CFT_INT:
		    CF_VAL(cf)=atoi(optarg);
		    break;
		case CFT_BOOLEAN:
		    CF_BOOL(cf)=1;
		    break;
		case CFT_STRING:
		    strcpy(CF_STR(cf),optarg);
		    //printf("conf %s %s\n",CF_STR(cf),optarg);
		    break;
		case CFT_ARRAY:
		    read_array(CF_ARRAY(cf),optarg,CF_ARRAY_SIZE(cf));
		    break;
		case CFT_ACTION_ARG:
		    strcpy(CF_STR(cf),optarg);
		    if (cf->action) {
			exit(cf->action(cf));
		    }
		    break;
		case CFT_ACTION:
		    if (cf->action) {
			exit(cf->action(cf));
		    }
		    break;
		}
	    }
	}
/*
	switch (c) {
	case 'h': 
	    cf_print_help();
	    exit(0);
	    break;
	case 'l':
	    dr_load_driver(CF_STR(cf_get_item_by_name("romrc")));
	    dr_list_all();//list_game();
	    exit(0);
	    break;
	}
*/
    }
    
    if (optind >= argc)
	return NULL;
    cf_cache_conf();
    return strdup(argv[optind]);
}

