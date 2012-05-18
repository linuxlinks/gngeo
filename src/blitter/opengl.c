#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef HAVE_GL_GL_H
#ifndef WII
#ifndef GP2X

#include "SDL.h"
#include "../emu.h"
#include "../screen.h"
#include "../video.h"
#include "../effect.h"
#include "../conf.h"
#include "../gnutil.h"
#include "glproc.h"

//#define TEXMIN256

static float a;
static float b;
static float c;
static float d;

static SDL_Surface *video_opengl;
static SDL_Surface *tex_opengl;
static SDL_Rect glrectef;

static int load_glproc() {
    static int init=0;
    CONF_ITEM *cf_libgl=cf_get_item_by_name("libglpath");
    if (init) return GN_TRUE;
    init=1;
    if (SDL_GL_LoadLibrary(CF_STR(cf_libgl))==-1) {
        printf("Unable to load OpenGL library: %s\n", CF_STR(cf_libgl));
        return GN_FALSE;
    }
	
    pglClearColor	= SDL_GL_GetProcAddress("glClearColor");
    pglClear		= SDL_GL_GetProcAddress("glClear");
    pglEnable		= SDL_GL_GetProcAddress("glEnable");
    pglViewport		= SDL_GL_GetProcAddress("glViewport");
    pglTexParameteri	= SDL_GL_GetProcAddress("glTexParameteri");
    pglPixelStorei	= SDL_GL_GetProcAddress("glPixelStorei");
    pglTexImage2D	= SDL_GL_GetProcAddress("glTexImage2D");
    pglBegin		= SDL_GL_GetProcAddress("glBegin");
    pglEnd		= SDL_GL_GetProcAddress("glEnd");
    pglTexCoord2f	= SDL_GL_GetProcAddress("glTexCoord2f");
    pglVertex2f		= SDL_GL_GetProcAddress("glVertex2f");
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    return GN_TRUE;
}

int
blitter_opengl_init()
{
	Uint32 sdl_flags;
	Uint32 width = visible_area.w;
	Uint32 height = visible_area.h;		
	
        if (load_glproc() == GN_FALSE) return GN_FALSE;

	sdl_flags = (fullscreen?SDL_FULLSCREEN:0)| SDL_DOUBLEBUF | SDL_HWSURFACE
	    | SDL_HWPALETTE | SDL_OPENGL | SDL_RESIZABLE;
	//SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	if ((effect[neffect].x_ratio!=2 || effect[neffect].y_ratio!=2) &&  
	    (effect[neffect].x_ratio!=1 || effect[neffect].y_ratio!=1) ) {
	    printf("Opengl support only effect with a ratio of 2x2 or 1x1\n");
	    return GN_FALSE;
	}
	    
	/*
	  if (conf.res_x==304 && conf.res_y==224) {
	*/
	if (scale < 2) {
	    width *=effect[neffect].x_ratio;
	    height*=effect[neffect].y_ratio;
	}
	width *= scale;
	height *= scale;
	printf("%d %d %d %d %d\n",width,height,scale,visible_area.w,visible_area.h);
/*
	} else {
	    width = conf.res_x;
	    height=conf.res_y;
	}
	
*/
	conf.res_x=width;
	conf.res_y=height;
	
	video_opengl = SDL_SetVideoMode(width, height, 16, sdl_flags);
	
	if ( video_opengl == NULL)
		return GN_FALSE;
	
	pglClearColor(0, 0, 0, 0);
	pglClear(GL_COLOR_BUFFER_BIT);
	
	pglEnable(GL_TEXTURE_2D);
	pglViewport(0, 0, width, height);
	
	/* Linear Filter */
	
        pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	/*
	  pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	  pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	*/
	/* Texture Mode */
	pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
		
	if (neffect == 0)
	{
		/* Texture limits */
/*
		a = (240.0/304.0);
		b = (48.0/256.0);
		
		c = (240.0/256.0);
*/
#ifdef USE_GL2
	    tex_opengl= SDL_CreateRGBSurface(SDL_SWSURFACE, 512, 256, 16, 0xF800, 0x7E0, 0x1F, 0);
#else
	    pglPixelStorei(GL_UNPACK_ROW_LENGTH, 352);
#endif
	}
	else {		
	    /* Texture limits */
	    a = ((256.0/(float)visible_area.w) - 1.0f)*effect[neffect].x_ratio/2.0;
	    b = ((512.0/(float)visible_area.w) - 1.0f)*effect[neffect].x_ratio/2.0;
	    c = (((float)visible_area.h/256.0))*effect[neffect].y_ratio/2.0;
	    d = (((float)((visible_area.w<<1)-512)/256.0))*effect[neffect].y_ratio/2.0;
	    screen = SDL_CreateRGBSurface(SDL_SWSURFACE, visible_area.w<<1,  /*visible_area.h<<1*/512, 16, 0xF800, 0x7E0, 0x1F, 0);
	    //printf("[opengl] create_screen %p\n",screen);
#ifdef USE_GL2
	    tex_opengl= SDL_CreateRGBSurface(SDL_SWSURFACE, 1024, 512, 16, 0xF800, 0x7E0, 0x1F, 0);
	    if (visible_area.w==320) {
		glrectef.x=0;
		glrectef.y=0;
		glrectef.w=320*2;
		glrectef.h=224*2;
	    } else {
		glrectef.x=0;
		glrectef.y=0;
		glrectef.w=304*2;
		glrectef.h=224*2;
	    }
#else
	    pglPixelStorei(GL_UNPACK_ROW_LENGTH, visible_area.w<<1);
#endif
	}
	
	return GN_TRUE;
}

int
blitter_opengl_resize(int w,int h)
{
  Uint32 sdl_flags;

  sdl_flags = SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE | SDL_OPENGL | SDL_RESIZABLE;

  video_opengl = SDL_SetVideoMode(w, h, 16, sdl_flags);

  if ( video_opengl == NULL)
    return GN_FALSE;

  pglEnable(GL_TEXTURE_2D);
  pglViewport(0, 0, w, h);

  return GN_TRUE;
}

void 
blitter_opengl_update()
{
#ifndef USE_GL2
    float VALA=512/(float)visible_area.w-1.0; /* 240.0f*2.0f/304.0f - 1.0f */
#endif
//    int VALB=.0625; /* 16.0f/256.0f  */
//    int VALC=.9375; /* 240.0f/256.0f */
//int VALD .25   /* 64.0f/256.0f */
//    int VALD=1.0;

    if (neffect == 0) {
#ifndef USE_GL2

	pglTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 256, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, buffer->pixels + (visible_area.x<<1));
	pglBegin(GL_QUADS);
	//pglTexCoord2f(16.0f/256.0f, 16.0f/256.0f);
	//pglVertex2f	(-1.0f, 1.0f);
	pglTexCoord2f(0.0f, 16.0f/256.0f);
	pglVertex2f  (-1.0f, 1.0f);

	pglTexCoord2f(1.0f, 16.0f/256.0f);
	pglVertex2f  (VALA, 1.0f);
			
	pglTexCoord2f(1.0f, 1.0f-16.0f/256.0f);
	pglVertex2f  (VALA, -1.0f);
			
	pglTexCoord2f(0.0f, 1.0f-16.0f/256.0f);
	pglVertex2f	(-1.0f, -1.0f);
	pglEnd();
				
	pglTexImage2D(GL_TEXTURE_2D, 0, 3, 64, 256, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, buffer->pixels + 512+ (visible_area.x<<1));
	pglBegin(GL_QUADS);
	pglTexCoord2f(0.0f, 16.0f/256.0f);
	pglVertex2f	(VALA, 1.0f);
			
	pglTexCoord2f(1.0, 16.0f/256.0f);
	pglVertex2f	(1.0f, 1.0f);
			
	pglTexCoord2f(1.0, 1.0f-16.0f/256.0f);
	pglVertex2f	(1.0f, -1.0f);
			
	pglTexCoord2f(0.0f, 1.0f-16.0f/256.0f);
	pglVertex2f	(VALA, -1.0f);
	pglEnd();

#else
	SDL_BlitSurface(buffer, &visible_area, tex_opengl, NULL);
	pglTexImage2D(GL_TEXTURE_2D, 0, 3, 512, 256, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, tex_opengl->pixels);

	pglBegin(GL_QUADS);
	pglTexCoord2f(0.0f, 0.0f);
	pglVertex2f  (-1.0f, 1.0f);
	
	pglTexCoord2f((float)visible_area.w/512.0f, 0.0f);
	pglVertex2f  (1.0f, 1.0f);
			
	pglTexCoord2f((float)visible_area.w/512.0f, 1.0f-32.0f/256.0f);
	pglVertex2f  (1.0f, -1.0f);
			
	pglTexCoord2f(0.0f, 1.0f-32.0f/256.0f);
	pglVertex2f	(-1.0f, -1.0f);
	pglEnd();
		
#endif

    } 
    else
    {		
#ifndef USE_GL2
	pglTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 256, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, screen->pixels );
	pglBegin(GL_QUADS);
	pglTexCoord2f(0.0f, 0.0f);
	pglVertex2f	(-1.0f, 1.0f);
			
	pglTexCoord2f(1.0f, 0.0f);
	pglVertex2f	(a, 1.0f);
			
	pglTexCoord2f(1.0f, c);
	pglVertex2f	(a, 0.0f);
			
	pglTexCoord2f(0.0f, c);
	pglVertex2f	(-1.0f, 0.0f);
	pglEnd();
		
	pglTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 256, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, screen->pixels + 512 );
	pglBegin(GL_QUADS);
	pglTexCoord2f(0.0f, 0.0f);
	pglVertex2f	(a, 1.0f);
			
	pglTexCoord2f(1.0f, 0.0f);
	pglVertex2f	(b, 1.0f);
			
	pglTexCoord2f(1.0f, c);
	pglVertex2f	(b, 0.0f);
			
	pglTexCoord2f(0.0f, c);
	pglVertex2f	(a, 0.0f);
	pglEnd();
				
	pglTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 256, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, screen->pixels + 1024 );
	pglBegin(GL_QUADS);
	pglTexCoord2f(0.0f, 0.0f);
	pglVertex2f	(b, 1.0f);
			
	pglTexCoord2f(d, 0.0f);
	pglVertex2f	(1.0f, 1.0f);
			
	pglTexCoord2f(d, c);
	pglVertex2f	(1.0f, 0.0f);
			
	pglTexCoord2f(0.0f, c);
	pglVertex2f	(b, 0.0f);
	pglEnd();	
				
	pglTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 256, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, screen->pixels + (visible_area.w<<2) * 224 );
	pglBegin(GL_QUADS);
	pglTexCoord2f(0.0f, 0.0f);
	pglVertex2f	(-1.0f, 0.0f);
			
	pglTexCoord2f(1.0f, 0.0f);
	pglVertex2f	(a, 0.0f);
			
	pglTexCoord2f(1.0f, c);
	pglVertex2f	(a, -1.0f);
			
	pglTexCoord2f(0.0f, c);
	pglVertex2f	(-1.0f, -1.0f);
	pglEnd();
				
	pglTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 256, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, screen->pixels + (visible_area.w<<2) * 224 + 512 );
	pglBegin(GL_QUADS);		
	pglTexCoord2f(0.0f, 0.0f);
	pglVertex2f	(a, 0.0f);
			
	pglTexCoord2f(1.0f, 0.0f);
	pglVertex2f	(b, 0.0f);
			
	pglTexCoord2f(1.0f, c);
	pglVertex2f	(b, -1.0f);
			
	pglTexCoord2f(0.0f, c);
	pglVertex2f	(a, -1.0f);
	pglEnd();
		
	pglTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 256, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, screen->pixels + (visible_area.w<<2) * 224 + 1024 );
	pglBegin(GL_QUADS);
	pglTexCoord2f(0.0f, 0.0f);
	pglVertex2f	(b, 0.0f);
			
	pglTexCoord2f(d, 0.0f);
	pglVertex2f	(1.0f, 0.0f);
			
	pglTexCoord2f(d, c);
	pglVertex2f	(1.0f, -1.0f);
			
	pglTexCoord2f(0.0f, c);
	pglVertex2f	(b, -1.0f);
	pglEnd();
#else
	SDL_BlitSurface(screen, &glrectef, tex_opengl, NULL);
	pglTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 512, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, tex_opengl->pixels);

	pglBegin(GL_QUADS);
	pglTexCoord2f(0.0f, 0.0f);
	pglVertex2f  (-1.0f, 1.0f);
	
	pglTexCoord2f((float)visible_area.w*2/1024.0f, 0.0f);
	pglVertex2f  (1.0f, 1.0f);
			
	pglTexCoord2f((float)visible_area.w*2/1024.0f, 448.0f/512.0f);
	pglVertex2f  (1.0f, -1.0f);
			
	pglTexCoord2f(0.0f, 448.0f/512.0f);
	pglVertex2f	(-1.0f, -1.0f);
	pglEnd();
#endif
    }
	
    SDL_GL_SwapBuffers();	
}

void
blitter_opengl_close()
{
	//if (screen != NULL)
	//	SDL_FreeSurface(screen);
}

void blitter_opengl_fullscreen() {
/* TODO: Borken, at least on ubuntu with catalyst -> the screen goes black */
	SDL_WM_ToggleFullScreen(video_opengl);
	pglEnable(GL_TEXTURE_2D);
	pglViewport(0, 0, conf.res_x, conf.res_y);



}

#endif
#endif
#endif
