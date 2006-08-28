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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zlib.h>
#include "unzip.h"
#include "driver.h"
#include "conf.h"
#include "pbar.h"
#include "memory.h"
#include "neocrypt.h"

#ifdef GP2X
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "menu.h"

#ifndef MAP_NONBLOCK
# define MAP_NONBLOCK  0x10000
#endif

#endif

extern int neogeo_fix_bank_type;

static LIST *driver_list=NULL;

static SDL_bool goto_next_driver(FILE * f)
{
    char buf[512];
    char game[12], type[10];
    long pos;

    while (!feof(f)) {
	pos=ftell(f);
	my_fgets(buf, 510, f);
	if (sscanf(buf, "game %12s %10s", game, type) >= 2) {
	    fseek(f,pos,SEEK_SET);
	    return SDL_TRUE;
	}
    }
    return SDL_FALSE;
}

static int get_sec_by_name(char *section) {
    static char *sec[SEC_MAX]={"CPU","SFIX","SM1","SOUND1","SOUND2","GFX"};
    int i;
    for(i=0;i<SEC_MAX;i++) {
	if (strcmp(sec[i],section)==0)
	    return i;
    }
    /*printf("Unknow section %s\n",section);*/
    return -1;
}

static int get_romtype_by_name(char *type) {
    static char *rt[ROMTYPE_MAX]={"MGD2","MVS","MVS_CMC42","MVS_CMC50"};
    int i;
    for(i=0;i<ROMTYPE_MAX;i++) {
	if (strcmp(rt[i],type)==0)
	    return i;
    }
    printf("Unknown rom type %s\n",type);
    return -1;
}

static void add_driver_section(Uint32 s, SECTION *sec,FILE *f) {
    char buf[512], a[64], b[64];
    Uint32 size, offset;
    sec->size=s;
    sec->item=NULL;
    //  printf("SECTION size=%x\n",sec->size);
    while (1) {
        SECTION_ITEM *item;
        my_fgets(buf, 511, f);
        if (strcmp(buf, "END") == 0)
            break;
        sscanf(buf, "%s %x %x %s", a, &offset, &size, b);
        item=malloc(sizeof(SECTION_ITEM));
	CHECK_ALLOC(item);

        item->filename=strdup(a);
        item->begin=offset;
        item->size=size;
        item->type=(strcmp(b,"ALTERNATE")==0?LD_ALTERNATE:LD_NORM);
        sec->item=list_append(sec->item,item);
	//    printf("%s %x %x %d\n",item->filename,item->begin,item->size,item->type);
    }
}

/* add driver define in f */
static void add_driver(FILE *f,char *fullname)
{
	DRIVER *dr;
    char buf[512], a[64], b[64];
    Uint32 s, size, offset;
    int sec;
    char game[12], type[10];
    char *t;

    dr=malloc(sizeof(DRIVER));
    CHECK_ALLOC(dr);

    /* TODO: driver creation */
    my_fgets(buf, 510, f);
    sscanf(buf, "game %12s %10s", game, type);
    dr->name=strdup(game);
    dr->rom_type=get_romtype_by_name(type);
    dr->special_bios=0;
    dr->banksw_type=BANKSW_NORMAL;
    dr->sfix_bank=0;

#if 0
    t = strchr(buf, '"');
    if (t) {
	    char *e;
	    t+=1;
	    e=strchr(t, '"');
	    if (e) {
		    e[0]=0;
		    dr->longname=strdup(t);
	    } else
		    dr->longname=NULL;
    }
    else dr->longname=NULL;
#endif
    if (fullname)
	    dr->longname=strdup(fullname);
    else
	    dr->longname=NULL;

    //printf("Add %8s | %s \n",dr->name,dr->longname);

    while (1) {
	my_fgets(buf, 511, f);
	if (strcmp(buf, "END") == 0)
	    break;
	sscanf(buf, "%s %x", a, &s);
	sec=get_sec_by_name(a);
	if (sec==-1) {
	    int b=0;
	    if (strcmp(a,"BIOS")==0) { add_driver_section(s,&(dr->bios), f); dr->special_bios=1;}
	    else if (strcmp(a,"XOR")==0) { dr->xor=s; }
	    else if (strcmp(a,"SFIXBANK")==0) { dr->sfix_bank=s; }
	    else if (strcmp(a,"BANKSWITCH")==0) {
		dr->banksw_type=s;
		if(s==BANKSW_SCRAMBLE) {
		    /* not implemented yet */
		    my_fgets(buf,511,f);
		    sscanf(buf,"%x",&dr->banksw_addr);
		    my_fgets(buf, 511, f);
		    sscanf(buf, "%d %d %d %d %d %d",
			   (int *) &dr->banksw_unscramble[0],
			   (int *) &dr->banksw_unscramble[1],
			   (int *) &dr->banksw_unscramble[2],
			   (int *) &dr->banksw_unscramble[3],
			   (int *) &dr->banksw_unscramble[4],
			   (int *) &dr->banksw_unscramble[5]);
		    while (1) {
			my_fgets(buf, 511, f);
			if (strcmp(buf, "END") == 0)
			    break;
			sscanf(buf,"%x", &dr->banksw_off[b]);
			b++;
		    }
		}
	    } else {
		printf("Unknown section %s\n",a);
		return;
	    }
	} else add_driver_section(s,&(dr->section[sec]),f);
    }


    //driver_list=list_prepend(driver_list,dr);
    driver_list=list_append(driver_list,dr);
}

static void print_driver(void *data) {
    DRIVER *dr=data;
    int i;
    printf("game %8s | %s \n",dr->name,dr->longname);
    for (i=0;i<SEC_MAX;i++) {
	LIST *l;
	printf("Section %d\n",i);
	for(l=dr->section[i].item;l;l=l->next) {
	    SECTION_ITEM *item=l->data;
	    printf("%8s %x %x\n",item->filename,item->begin,item->size);
	}
    }
}

SDL_bool dr_load_driver_dir(char *dirname) {
    DIR *pdir;
    struct stat buf;
    struct dirent *pfile;
    if (!(pdir=opendir (dirname))) {
	    // printf("Couldn't find %s\n",dirname);
        return SDL_FALSE; 
    }
    while(pfile=readdir(pdir)) {
        char *filename=alloca(strlen(pfile->d_name)+strlen(dirname)+2);
        sprintf(filename,"%s/%s",dirname,pfile->d_name);
        stat(filename,&buf);
        if (S_ISREG(buf.st_mode)) {
            dr_load_driver(filename);
        }
    }
    closedir(pdir);
    return SDL_TRUE;
}

/* load the specified file, and create the driver struct */
SDL_bool dr_load_driver(char *filename) {
    FILE *f;
    LIST *i;
    char buf[512],*fullname;

    f=fopen(filename,"r");
    if (!f) {
	printf("Couldn't find %s\n",filename);
	return SDL_FALSE;
    }
    /* fill game information */
    my_fgets(buf, 510, f);
    
    if (strncmp(buf,"longname ",9)==0) {
	    fullname=buf+9; /* hummm, caca... */
	    //chomp(fullname);
    } else {
	    fullname=NULL;
    }

    while(goto_next_driver(f)==SDL_TRUE) {
	add_driver(f,fullname);
    }

    //list_foreach(driver_list,print_driver);

    fclose(f);
    return SDL_TRUE;
}

void dr_list_all(void);
void dr_list_available(void);

static SDL_bool file_is_zip(char *name) {
    unzFile *gz;
    gz=unzOpen(name);
    if (gz!=NULL) {
	unzClose(gz);
	return SDL_TRUE;
    }
    return SDL_FALSE;
}

static void free_ziplist_item(void *data) {
    free(data);
}

/* check if the driver dr correspond to the zip file pointed by gz 
   (zip_list contain the zip file content)
*/
static SDL_bool check_driver_for_zip(DRIVER *dr,unzFile *gz,LIST *zip_list) {
    int i;
    LIST *l,*zl;

    for (i=0;i<SEC_MAX;i++) {
	//printf("Check section %d\n",i);
	for(l=dr->section[i].item;l;l=l->next) {
	    SECTION_ITEM *item=l->data;
	    if (strcmp(item->filename,"-")!=0) {
		for(zl=zip_list;zl;zl=zl->next) {
		    //printf("Check filename %s %s\n",(char*)zl->data,item->filename);
		    if (strcasecmp(item->filename,(char*)zl->data)==0) {
			//printf("filename %s=%s\n",(char*)zl->data,item->filename);
			break;
		    }
		}
		//printf("Zl %s = %p\n",item->filename,zl);
		if (zl==NULL)
		    return SDL_FALSE;
	    }
	   
	}
    }
    
    return SDL_TRUE;
}

char *get_zip_name(char *name) {
    char *zip;
    char *path = CF_STR(cf_get_item_by_name("rompath"));
    if (file_is_zip(name)) {
	    zip=malloc(strlen(name)+1); CHECK_ALLOC(zip)
        strcpy(zip,name);
    } else {
        int len = strlen(path) + strlen(name) + 6;
        zip = malloc(len); CHECK_ALLOC(zip)
        sprintf(zip,"%s/%s.zip",path,name);
    }
    return zip;
}

/* return the correct driver for the zip file zip*/

DRIVER *get_driver_for_zip(char *zip) {
    unzFile *gz;
    int i;
    char zfilename[256];
    char *t;
    LIST *zip_list=NULL,*l,*zl;
    int res;

    /* first, we check if it a zip */
    gz = unzOpen(zip);
    if (gz==NULL) {
	return NULL;
    }
    //printf("Get driver for %s\n",zip);
    /* now, we create a list containing the content of the zip */
    i=0;
    unzGoToFirstFile(gz);
    do {
	unzGetCurrentFileInfo(gz,NULL,zfilename,256,NULL,0,NULL,0);
	//printf("List zip %s\n",zfilename);
	t=strrchr(zfilename,'.');
	if (! ( (strncasecmp(zfilename,"n",1)==0 && strlen(zfilename)<=12 )|| 
		(t && (strcasecmp(t,".rom")==0 || strcasecmp(t,".bin")==0) ) )
	    )
	    
	    i++;
	if (i>10) {
	    //printf("More than 10 file are not rom....\n");
	    /* more than 10 files are not rom.... must not be a valid romset 
	       10 files should be enough */
	    list_erase_all(zip_list,free_ziplist_item);
	    return NULL;
	}
	zip_list=list_prepend(zip_list,strdup(zfilename));
    } while (unzGoToNextFile(gz)!=UNZ_END_OF_LIST_OF_FILE);
    
    /* now we check every driver to see if it match the zip content */
    for (l=driver_list;l;l=l->next) {
	DRIVER *dr=l->data;
	if (check_driver_for_zip(dr,gz,zip_list)==SDL_TRUE) {
	    unzClose(gz);
	    list_erase_all(zip_list,free_ziplist_item);
	    return dr;
	}
    }
		
    list_erase_all(zip_list,free_ziplist_item);
    unzClose(gz);
    /* not match found */
    return NULL;
}

DRIVER *dr_get_by_name(char *name) {
    char *zip=get_zip_name(name);
    DRIVER *dr=get_driver_for_zip(zip);
    free(zip);
    return dr;
}

static int zfread(unzFile * f, void *buffer, int length)
{
    Uint8 *buf = (Uint8*)buffer;
    Uint8 *tempbuf;
    Uint32 totread, r, i;
    int totlength=length;
    totread = 0;
    tempbuf=alloca(4097);

    while (length) {

	r = length;
	if (r > 4096)
	    r = 4096;

	r = unzReadCurrentFile(f, tempbuf, r);
	if (r == 0) {
	    terminate_progress_bar();
	    return totread;
	}
	memcpy(buf, tempbuf, r);

	buf += r;
	totread += r;
	length -= r;
	update_progress_bar(totread,totlength);
    }

    terminate_progress_bar();
    return totread;
}

static int zfread_alternate(unzFile * f, void *buffer, int length, int inc)
{
    Uint8 *buf = buffer;
    Uint8 tempbuf[4096];
    Uint32 totread, r, i;
    int totlength=length;
    totread = 0;
    

    while (length) {

	r = length;
	if (r > 4096)
	    r = 4096;

	r = unzReadCurrentFile(f, tempbuf, r);
	if (r == 0) {
	    terminate_progress_bar();
	    return totread;
	}
	for (i = 0; i < r; i++) {
	    *buf = tempbuf[i];
	    buf += inc;
	}
	totread += r;
	length -= r;
	update_progress_bar(totread,totlength);
    }

    terminate_progress_bar();
    return totread;
}

SDL_bool dr_load_section(unzFile *gz, SECTION s, Uint8 *current_buf) {
    LIST *l;
    for(l=s.item;l;l=l->next) {
	SECTION_ITEM *item=l->data;
    
	if (strcmp(item->filename,"-")!=0) {
	    /* nouveau fichier, on l'ouvre */
	    if (unzLocateFile(gz, item->filename, 2) == UNZ_END_OF_LIST_OF_FILE) {
		unzClose(gz);
		return SDL_FALSE;
	    }
	    if (unzOpenCurrentFile(gz) != UNZ_OK) {
		unzClose(gz);
		return SDL_FALSE;
	    }
	}
	create_progress_bar(item->filename);
	if (item->type==LD_ALTERNATE)
	    zfread_alternate(gz, current_buf + item->begin, item->size, 2);
	else
	    zfread(gz, current_buf + item->begin, item->size);
    }
    return SDL_TRUE;
}
#ifdef GP2X
/* Big Warn!! This funtion assume that gfx are always load in ALTERNATE mode */
SDL_bool dr_dump_gfx(DRIVER *dr,unzFile *gz, SECTION s) {
    LIST *l,*i;
    int gfx_size=s.size;
    int size=0;
    int a=0,current_tile=0;
    int current_offset=0;
    Uint8 *data=NULL;
    Uint8 *usage=NULL;
    FILE *dump;
    char dumpname[256];
    
    sprintf(dumpname,"%s/%s.gfx",CF_STR(cf_get_item_by_name("rompath")),dr->name);
    dump=fopen(dumpname,"wb");
    if (!dump) {
	    printf("Can't open %s\n",dumpname);
	    return SDL_FALSE;
    }

    usage = malloc(memory.nb_of_tiles * sizeof(int));
    CHECK_ALLOC(usage);
    memset(usage, 0, memory.nb_of_tiles * sizeof(int));

    for(l=s.item;l;l=l->next) {
	    SECTION_ITEM *item_a,*item_b;
	    i=l->next;
	    item_a=l->data;
	    item_b=i->data;

	    if (strcmp(item_b->filename,"-")==0 && (item_b->begin-1!=item_a->begin)) {
		    /* Unhandled case */
		    printf("Unhandled case\n");
		    return SDL_FALSE;
	    }
	    if (item_a->size!=item_b->size) {
		    /* should not append */
		    printf("Strange things append while dumping gfx\n");
		    return SDL_FALSE;
	    }
	    if(data) free(data);
	    printf("Allocating %d bytes\n",item_a->size*2);
	    data=malloc(item_a->size*2);
	    if (data==NULL) {
		    printf("Out of memory :(\n");
		    return SDL_FALSE;
	    }

	    create_progress_bar(item_a->filename);
	    /* Open the first part */
	    if (unzLocateFile(gz, item_a->filename, 2) == UNZ_END_OF_LIST_OF_FILE) {
		unzClose(gz);
		return SDL_FALSE;
	    }
	    if (unzOpenCurrentFile(gz) != UNZ_OK) {
		unzClose(gz);
		return SDL_FALSE;
	    }
	    /* load data */
	    if (item_a->type==LD_ALTERNATE) {
		    zfread_alternate(gz, data, item_a->size, 2);
	    } else {
		    unzClose(gz);return SDL_FALSE;	
	    }

	    create_progress_bar(item_b->filename);
	    /* Open the second part */
	    if (strcmp(item_b->filename,"-")!=0) {
		    /* nouveau fichier, on l'ouvre */
		    if (unzLocateFile(gz, item_b->filename, 2) == UNZ_END_OF_LIST_OF_FILE) {
			    unzClose(gz);
			    return SDL_FALSE;
		    }
		    if (unzOpenCurrentFile(gz) != UNZ_OK) {
			    unzClose(gz);
			    return SDL_FALSE;
		    }
	    }
            /* load data */
	    if (item_b->type==LD_ALTERNATE) {
		    zfread_alternate(gz, data+1, item_b->size, 2);
	    } else {
		    unzClose(gz);return SDL_FALSE;	
	    }
	    
	    create_progress_bar("Convert tile");
	    //printf("Convert data\n");
	    /* Convert tiles */ 
	    memory.gfx=data;
	    memset(memory.pen_usage, 0, ((item_a->size*2)>>7) * sizeof(int));
	    for (a = 0; a < (item_a->size*2)>>7; a++) {
		    convert_tile(a);
		    if (a%100==0) update_progress_bar(a,(item_a->size*2)>>7);
	    }
	    terminate_progress_bar();
	    memcpy(usage+current_tile*sizeof(int),memory.pen_usage,
		   ((item_a->size*2)>>7)*sizeof(int));
	    current_tile+=((item_a->size*2)>>7);

	    //printf("Dump data\n");
	    /* dump the data */
	    create_progress_bar("Dump GFX");
	    a=(item_a->size*2);
	    for(a=0;a<(item_a->size*2);a+=0x10000) {
		    fwrite(data+a,0x10000,1,dump);
		    update_progress_bar(a,(item_a->size*2));
	    }
	    if (a!=(item_a->size*2)) {
		    fwrite(data+a,(item_a->size*2)-a,1,dump);
	    }
	    terminate_progress_bar();

	    l=l->next;
    }
    fwrite(usage,memory.nb_of_tiles*sizeof(Uint32),1,dump);
    fclose(dump);

    free(data);
    free(usage);

    return SDL_TRUE;
}
#endif

void set_bankswitchers(BANKSW_TYPE bt) {
    switch(bt) {
    case BANKSW_NORMAL:
	mem68k_fetch_bksw_byte=mem68k_fetch_bk_normal_byte;
	mem68k_fetch_bksw_word=mem68k_fetch_bk_normal_word;
	mem68k_fetch_bksw_long=mem68k_fetch_bk_normal_long;
	mem68k_store_bksw_byte=mem68k_store_bk_normal_byte;
	mem68k_store_bksw_word=mem68k_store_bk_normal_word;
	mem68k_store_bksw_long=mem68k_store_bk_normal_long;
	break;
    case BANKSW_KOF2003:
	mem68k_fetch_bksw_byte=mem68k_fetch_bk_kof2003_byte;
	mem68k_fetch_bksw_word=mem68k_fetch_bk_kof2003_word;
	mem68k_fetch_bksw_long=mem68k_fetch_bk_kof2003_long;
	mem68k_store_bksw_byte=mem68k_store_bk_kof2003_byte;
	mem68k_store_bksw_word=mem68k_store_bk_kof2003_word;
	mem68k_store_bksw_long=mem68k_store_bk_kof2003_long;
	break;
    }
}
SDL_bool dr_load_game(DRIVER *dr,char *name) {
    unzFile *gz;
    int i;
    LIST *l;
    char *zip=get_zip_name(name);
    gz = unzOpen(zip);
    free(zip);
    if (gz==NULL) {
	return SDL_FALSE;
    }
#ifdef GP2X
    memory.gp2x_gfx_mapped=SDL_FALSE;
#endif

    if (dr->special_bios) {
	    memory.bios=malloc(dr->bios.size); CHECK_ALLOC(memory.bios);
	memory.bios_size=dr->bios.size;
	if (!dr_load_section(gz,dr->bios,memory.bios)) return SDL_FALSE;
    }
    for (i=0;i<SEC_MAX;i++) {
	int s=dr->section[i].size;
	Uint8 *current_buf=NULL;
	//if (dr->section[i].item==NULL) continue;
	if (s==0) continue;
	//      printf("%p %d \n",dr->section[i].item,i);
	switch (i) {
	case SEC_CPU:
#         ifdef GP2X__
		memory.cpu = gp2x_ram_malloc(s);
#         else
		memory.cpu = malloc(s); CHECK_ALLOC(memory.cpu);
#         endif
		current_buf = memory.cpu;
		memory.cpu_size = s;
	    break;
	case SEC_SFIX:
#         ifdef GP2X__
		memory.sfix_game = gp2x_ram_malloc(s);
		memory.fix_game_usage = gp2x_ram_malloc(s >> 5);
#         else
		memory.sfix_game = malloc(s); CHECK_ALLOC(memory.sfix_game);
		memory.fix_game_usage = malloc(s >> 5);
		CHECK_ALLOC(memory.fix_game_usage);
#         endif
	    current_buf = memory.sfix_game;
	    memory.sfix_size = s;
	    break;
	case SEC_SM1:
#         ifdef GP2X__
		if (conf.sound) 
			memory.sm1 = gp2x_ram_malloc(s);
#         else
		memory.sm1 = malloc(s); CHECK_ALLOC(memory.sm1 );
#         endif
	    current_buf = memory.sm1;
	    memory.sm1_size = s;
	    break;
	case SEC_SOUND1:
#         ifdef GP2X
		if (conf.sound) 
			memory.sound1 = gp2x_ram_malloc(s);
#         else
		memory.sound1 = malloc(s); CHECK_ALLOC(memory.sound1);
#         endif
		memory.sound1_size = s;
		current_buf = memory.sound1;
		break;
	case SEC_SOUND2:
#         ifdef GP2X
		if (conf.sound) 
			memory.sound2 = gp2x_ram_malloc(s);
#         else
		memory.sound2 = malloc(s); CHECK_ALLOC(memory.sound2);
#         endif
		memory.sound2_size = s;
		current_buf = memory.sound2;
		break;
	case SEC_GFX:
		memory.gfx = malloc(s); 
#ifndef GP2X
		CHECK_ALLOC(memory.gfx);
#endif
		//printf("Alloc %x for GFX: %p\n",s,memory.gfx);
		memory.gfx_size = s;
		current_buf = memory.gfx;
		memory.pen_usage = malloc((s >> 7) * sizeof(int));
		CHECK_ALLOC(memory.pen_usage);
		memset(memory.pen_usage, 0, (s >> 7) * sizeof(int));
		memory.nb_of_tiles = s >> 7;
	    break;
	    /* TODO: Crypted rom */
	default:
	    break;
	}
	if (current_buf) {
		if (!dr_load_section(gz,dr->section[i],current_buf)) return SDL_FALSE;
	}
    }
    

#ifdef GP2X
    if (!memory.gfx) {
	    /* gfx couldn't be allocated. Check for a gfx dump, and mmap it */
	    //int gfxdump;
	    char dumpname[256];
	    Uint32 *pusage;
	    sprintf(dumpname,"%s/%s.gfx",CF_STR(cf_get_item_by_name("rompath")),dr->name);
	    gp2x_gfx_dump=open(dumpname,O_RDONLY);
	    if (gp2x_gfx_dump==-1) {
		    /* TODO: ask before!!! */
			if ((dr->rom_type == MGD2) || (dr->rom_type == MVS_CMC42) || (dr->rom_type == MVS_CMC50)) {
			    gn_popup_error("Failed!",
					   "%s is encrypted "
					   "It can't be dump on the GP2X. "
					   "If you want to run it, "
					   "dump it on your PC.",dr->name);
			    exit(1);
			}
		    if (gn_popup_question("GFX Dump?",
					  "In order to load %s a "
					  "dumpgfx file must be created. "
					  "This will require %d MB "
					  "of space on your SD card\n\n"
					  "Proceed?",dr->name,(memory.gfx_size/1024)/1024)) {
			    SDL_FillRect(screen,NULL,0);
			    free(memory.cpu);
			    if (memory.sm1) free(memory.sm1);
			    if (memory.sfix_game) free( memory.sfix_game);

			    gn_reset_pbar();

			    if (dr_dump_gfx(dr,gz,dr->section[SEC_GFX])!=SDL_TRUE) {
				    gn_popup_error("Dumping GFX:",
						   "Sorry, gngeo couldn't dump this roms :(");
				    unzClose(gz);
				    exit(1);  
			    }
			    //gp2x_gfx_dump=open(dumpname,O_RDONLY);
			    gn_popup_info("Succed!",
					  "The dumgfx file have been "
					  "created. Now, gngeo will "
					  "exit. Select %s again to "
					  "enjoy it :)",dr->name);
		    }
		    exit(1); 
	    }
	    if (gp2x_gfx_dump!=-1) {
		    memory.gfx=mmap(0,memory.gfx_size,PROT_READ, MAP_PRIVATE|MAP_NONBLOCK, gp2x_gfx_dump,0x0);
		    pusage=mmap(0,memory.nb_of_tiles*sizeof(int),PROT_READ, MAP_PRIVATE|MAP_NONBLOCK,
				gp2x_gfx_dump,memory.gfx_size);
		    memcpy(memory.pen_usage,pusage,memory.nb_of_tiles*sizeof(int));
		    
		    //printf("Mem GFX=%08X\n",memory.gfx);
		    memory.gp2x_gfx_mapped=SDL_TRUE;
	    } else {
		    gn_popup_error("Loading GFX:","Couldn't open gfx dump. "
				   "Check your roms dir and check for %s.gfx.",dr->name);
		    unzClose(gz);
		    exit(1);
	    }
    }
#endif
    unzClose(gz);

    /* TODO: Use directly the driver value insteed of recopying them */
    conf.game=dr->name;
    conf.rom_type=dr->rom_type;
    conf.special_bios=dr->special_bios;
    conf.extra_xor=dr->xor;
    neogeo_fix_bank_type = dr->sfix_bank;;
    set_bankswitchers(dr->banksw_type);
    for(i=0;i<6;i++)
	memory.bksw_unscramble[i]=dr->banksw_unscramble[i];
    for(i=0;i<64;i++)
	memory.bksw_offset[i]=dr->banksw_off[i];

#ifdef GP2X
    if (memory.gp2x_gfx_mapped==SDL_FALSE) {
#endif
	    if (conf.rom_type == MGD2) {
		    create_progress_bar("Convert MGD2");
		    convert_mgd2_tiles(memory.gfx, memory.gfx_size);
		    convert_mgd2_tiles(memory.gfx, memory.gfx_size);
		    terminate_progress_bar();
	    }
	    if (conf.rom_type == MVS_CMC42) {
		    create_progress_bar("Decrypt GFX ");
		    kof99_neogeo_gfx_decrypt(conf.extra_xor);
		    terminate_progress_bar();
	    }
	    if (conf.rom_type == MVS_CMC50) {
		    create_progress_bar("Decrypt GFX ");
		    kof2000_neogeo_gfx_decrypt(conf.extra_xor);
		    terminate_progress_bar();
	    }
#ifdef GP2X
    } else if ((conf.rom_type == MVS_CMC42) || (conf.rom_type == MVS_CMC50)) {
	    create_progress_bar("Decrypt SFIX");
	    neogeo_sfix_decrypt();
	    terminate_progress_bar();
	}
#endif
    convert_all_char(memory.sfix_game, memory.sfix_size,
		     memory.fix_game_usage);


#ifdef GP2X
    if (memory.gp2x_gfx_mapped==SDL_FALSE) {
#endif
	    //if (CF_BOOL(cf_get_item_by_name("convtile"))) {
	    create_progress_bar("Convert tile");
	    for (i = 0; i < memory.nb_of_tiles; i++) {
		    convert_tile(i);
		    if (i%100==0) update_progress_bar(i,memory.nb_of_tiles);
	    }
	    terminate_progress_bar();
	    //}
#ifdef GP2X
    }
#endif
    if (memory.sound2 == NULL) {
	memory.sound2 = memory.sound1;
	memory.sound2_size = memory.sound1_size;
    }

    //backup neogeo game vectors
    memcpy(memory.game_vector,memory.cpu,0x80);
    printf("                                                                             \r");

    init_video();

    return SDL_TRUE;
}

static int cmp_driver(void *a,void *b) {
    DRIVER *da=a;
    DRIVER *db=b;
    return strcmp(da->name,db->name);
}

void dr_list_all(void) {
    LIST *l;
    LIST *t=NULL;
    for(l=driver_list;l;l=l->next) {
	t=list_insert_sort_unique(t,l->data,cmp_driver);
    }
    for(l=t;l;l=l->next) {
	DRIVER *dr=l->data;
	printf("%-12s : %s\n",dr->name,dr->longname);
    }
}

LIST *driver_get_all(void) {
    return driver_list;
}
