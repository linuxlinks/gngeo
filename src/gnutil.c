/*
 * gnutil.c
 *
 * Contain various utility stuff used by gngeo
 *
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "SDL.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gnutil.h"

char gnerror[1024];

void chomp(char *str) {
    int i = 0;
    if (str) {
        while (str[i] != 0) {
            printf(" %d ", str[i]);
            i++;
        }
        printf("\n");
        if (str[i - 1] == 0x0A || str[i - 1] == 0x0D) str[i - 1] = 0;
        if (str[i - 2] == 0x0A || str[i - 2] == 0x0D) str[i - 2] = 0;

    }
}


char *file_basename(char *filename) {
    char *t;
    t = strrchr(filename, '/');
    if (t) return t + 1;
    return filename;
}

/* check if dir_name exist. Create it if not */
int check_dir(char *dir_name) {
    DIR *d;

    if (!(d = opendir(dir_name)) && (errno == ENOENT)) {
#ifdef WIN32
        mkdir(dir_name);
#else
        mkdir(dir_name, 0755);
#endif
        return GN_FALSE;
    }
    if(d)
    	closedir(d);
    return GN_TRUE;
}

/* return a char* to $HOME/.gngeo/
   DO NOT free it!
 */
#ifdef EMBEDDED_FS

char *get_gngeo_dir(void) {
    static char *filename = ROOTPATH"";
    return filename;
}
#else

char *get_gngeo_dir(void) {
    static char *filename = NULL;
#if defined (__AMIGA__)
    int len = strlen("/PROGDIR/data/") + 1;
#else
    int len = strlen(getenv("HOME")) + strlen("/.gngeo/") + 1;
#endif
    if (!filename) {
        filename = malloc(len * sizeof (char));
        CHECK_ALLOC(filename);
#if defined (__AMIGA__)
        sprintf(filename, "/PROGDIR/data/");
#else
        sprintf(filename, "%s/.gngeo/", getenv("HOME"));
#endif
    }
    check_dir(filename);
    //printf("get_gngeo_dir %s\n",filename);
    return filename;
}
#endif

void gn_set_error_msg(char *fmt,...) {
	va_list pvar;
	va_start(pvar, fmt);
	vsnprintf(gnerror,GNERROR_SIZE,fmt,pvar);
}

/*
 * replace any ending / with a 0
 */
void gn_rtrim_slash(char *dir) {
	if (dir[strlen(dir)-1]=='/' && strlen(dir)!=1)
		dir[strlen(dir)-1]=0;
}

void gn_strncat_dir(char *basedir,char *dir,size_t n) {
	gn_rtrim_slash(basedir);

	if (strcmp(dir,".")==0)
			return;
	if (strcmp(dir,"..")==0 ) {
		if (strlen(basedir)!=1) {
			char *slash=strrchr(basedir,'/');
			if (slash==basedir)
				slash[1]=0;
			else if (slash!=NULL)
				slash[0]=0;
		} else
			return;
	} else {
		if (strlen(basedir)!=1) strncat(basedir,"/",n);
		strncat(basedir,dir,n);
	}
}
