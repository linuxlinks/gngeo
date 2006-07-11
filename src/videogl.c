#ifdef FULL_GL

#include "SDL.h"
#include "SDL_opengl.h"

#include "videogl.h"
#include "screen.h"
#include "memory.h"
#include "transpack.h"

#define TCACHE_SIZE 0x100
#define TCACHE_DECAL 8

typedef struct TILE_CACHE{
    Uint32 tileno;
    Uint16 pal; /* current pal */
    SDL_bool free;
    GLuint tex;
    SDL_Surface *tile;
}TILE_CACHE;

static TILE_CACHE tile_cache[TCACHE_SIZE];
static SDL_Surface *video_opengl;

static __inline__ void draw_tile(unsigned int tileno,int color,SDL_Surface *bmp)
{
    unsigned int *gfxdata,myword;
    int y;
    unsigned char col;
    unsigned short *br;
    unsigned short *paldata=(unsigned short *)&current_pc_pal[16*color];
    paldata[0]=0xF81F; /* colorkey=#FF00FF */

    tileno=tileno%memory.nb_of_tiles;
   
    gfxdata = (unsigned int *)&memory.gfx[ tileno<<7];
        
    br= (unsigned short *)bmp->pixels;
    for(y=0;y<16;y++) {
	myword = gfxdata[0];
	br[0]=paldata[(myword>>28)&0xf];
	br[1]=paldata[(myword>>24)&0xf];
	br[2]=paldata[(myword>>20)&0xf];
	br[3]=paldata[(myword>>16)&0xf];
	br[4]=paldata[(myword>>12)&0xf];
	br[5]=paldata[(myword>>8)&0xf];
	br[6]=paldata[(myword>>4)&0xf];
	br[7]=paldata[(myword>>0)&0xf];
	
	myword = gfxdata[1];
	br[8]=paldata[(myword>>28)&0xf];
	br[9]=paldata[(myword>>24)&0xf];
	br[10]=paldata[(myword>>20)&0xf];
	br[11]=paldata[(myword>>16)&0xf];
	br[12]=paldata[(myword>>12)&0xf];
	br[13]=paldata[(myword>>8)&0xf];
	br[14]=paldata[(myword>>4)&0xf];
	br[15]=paldata[(myword>>0)&0xf];
	br+=16;
	gfxdata+=2;
    }
}

static void init_cache(void) {
    int i;
    GLuint texture;

    for(i=0;i<TCACHE_SIZE;i++) {
	tile_cache[i].tileno=0;
	tile_cache[i].pal=0;
	tile_cache[i].free=SDL_TRUE;
	tile_cache[i].tile=SDL_CreateRGBSurface(SDL_SWSURFACE, 16, 16, 16, 0xF800, 0x7E0, 0x1F, 0);
	/*glGenTextures(1, &texture);
	printf("generate %d\n",texture);
	tile_cache[i].tex=texture;*/

	//glBindTexture(GL_TEXTURE_2D,texture);
    }
}

SDL_bool fgl_init_video(void) {
    Uint32 sdl_flags;
    Uint32 width = visible_area.w;
    Uint32 height = visible_area.h;		
    sdl_flags = (fullscreen?SDL_FULLSCREEN:0)| SDL_DOUBLEBUF | SDL_HWSURFACE
	| SDL_HWPALETTE | SDL_OPENGL | SDL_RESIZABLE;
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    if (scale>1) {
	width *= scale;
	height *= scale;
    }
    video_opengl = SDL_SetVideoMode(width, height, 16, sdl_flags);
	
    if ( video_opengl == NULL)
	return SDL_FALSE;
	
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
//    glColorMask(0xFF,0,0xFF,0);
    glEnable(GL_TEXTURE_2D);
    glViewport(0, 0, width, height);
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho(0.0, 320.0, 224.0, 0.0, -5.0, 5.0 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    /* Linear Filter */
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    /*
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    */
    /* Texture Mode */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

//    glPixelStorei(GL_UNPACK_ROW_LENGTH, 16);
    init_cache();
}

static __inline__ void fgl_drawtile(FGL_TILE *tile) {
//    printf("Draw %d %d %d %d\n",tile->x,(int)tile->y,tile->zx,(int)tile->zy);
//    printf("bind %d\n",tile_cache[tile->no>>TCACHE_DECAL].tex);
    glBindTexture(GL_TEXTURE_2D,tile->no>>TCACHE_DECAL);
    if (tile_cache[tile->no>>TCACHE_DECAL].tileno!=tile->no) {
	
	draw_tile(tile->no,tile->pal,tile_cache[tile->no>>TCACHE_DECAL].tile);
	tile_cache[tile->no>>TCACHE_DECAL].tileno=tile->no;
//	glTexImage2D(GL_TEXTURE_2D, 0, 3, 16, 16, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, tile_cache[tile->no>>16].tile->pixels);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 16, 16, 0, 
		     GL_RGB, GL_UNSIGNED_SHORT_5_6_5, tile_cache[tile->no>>TCACHE_DECAL].tile->pixels);

    }
    
    //glColor3f(1.0,1.0,1.0);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0,0.0);
    glVertex3f((GLfloat)tile->x,(GLfloat)tile->y,0.0);

    glTexCoord2f(1.0,0.0);
    glVertex3f((GLfloat)tile->x+tile->zx,(GLfloat)tile->y,0.0);

    glTexCoord2f(1.0,1.0);
    glVertex3f((GLfloat)tile->x+tile->zx,(GLfloat)tile->y+tile->zy,0.0);

    glTexCoord2f(0.0,1.0);
    glVertex3f((GLfloat)tile->x,(GLfloat)tile->y+tile->zy,0.0);
    glEnd();
}

static __inline__ void fgl_drawstrip(Uint32 offs,FGL_STRIP *strip) {
    int i;
    float y=strip->y;
    int tileno=0,tileatr;
    unsigned char *vidram=memory.video;

    //printf("Draw Strip %d %d %d %d %d\n",strip->x,strip->y,strip->zx,strip->zy);
    for(i=0;i<strip->nb_tile;i++) {
	tileno  = READ_WORD(&vidram[offs]);
	offs+=2;
	tileatr = READ_WORD(&vidram[offs]);
	offs+=2;
	
	if (memory.nb_of_tiles>0x10000 && tileatr&0x10) tileno+=0x10000;
	if (memory.nb_of_tiles>0x20000 && tileatr&0x20) tileno+=0x20000;
	if (memory.nb_of_tiles>0x40000 && tileatr&0x40) tileno+=0x40000;
	
	/* animation automatique */
	/*if (tileatr&0x80) printf("PLOP\n");*/
	if (tileatr&0x8) {
	    tileno=(tileno&~7)+((tileno+neogeo_frame_counter)&7);
	} else {
	    if (tileatr&0x4) {
		tileno=(tileno&~3)+((tileno+neogeo_frame_counter)&3);
	    }
	}
	
	if (tileno>memory.nb_of_tiles) {
	    continue;
	}
	
	if (memory.pen_usage[tileno]==TILE_UNCONVERTED) {
	    convert_tile(tileno);
	}
	if (memory.pen_usage[tileno]!=TILE_INVISIBLE) {
	    strip->tiles[i].no=tileno;
	    strip->tiles[i].pal=tileatr>>8;
	    strip->tiles[i].x=strip->x;
	    strip->tiles[i].y=y;
	    strip->tiles[i].zx=strip->zx;
	    strip->tiles[i].zy=strip->zy;/* /(float)strip->nb_tile*/;
	    fgl_drawtile(&strip->tiles[i]);
	}
	y+=strip->zy;
    }
}

void fgl_drawscreen(void) {
    int sx =0,sy =0,oy =0,my =0,zx = 1, rzy = 1;
    int offs,i,count,y;
    int tileno,tileatr,t1,t2,t3;
    char fullmode=0;
    int ddax=0,dday=0,rzx=15,yskip=0;
    unsigned char *vidram=memory.video;
    //    int drawtrans=0;

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    /* Draw sprites */
    for (count=0;count<0x300;count+=2) {
        t3 = READ_WORD( &vidram[0x10000 + count] );
        t1 = READ_WORD( &vidram[0x10400 + count] );
        t2 = READ_WORD( &vidram[0x10800 + count] );

	
	/* If this bit is set this new column is placed next to last one */
        if (t1 & 0x40) {
            sx += rzx;            /* new x */
	    /* Get new zoom for this column */
            zx = (t3 >> 8) & 0x0f;
            sy = oy;                /* y pos = old y pos */
        } else {	/* nope it is a new block */
	    /* Sprite scaling */
            zx = (t3 >> 8) & 0x0f;    /* zomm x */
            rzy = t3 & 0xff;          /* zoom y */
            if (rzy==0) continue;
            sx = (t2 >> 7);           /* x pos */

	    /* Number of tiles in this strip */
            my = t1 & 0x3f;

	    if (my == 0x20) fullmode = 1;
            else if (my >= 0x21) fullmode = 2;     /* most games use 0x21, but */
	    /* Alpha Mission II uses 0x3f */
            else fullmode = 0;

            sy = 0x200 - (t1 >> 7); /* sprite bank position */

            if (sy > 0x110) sy -= 0x200;
      
            if (fullmode == 2 || (fullmode == 1 && rzy == 0xff)) {
                while (sy < 0) sy += ((rzy + 1)<<1);
            }
            oy = sy; /* on se souvient de la position y */
	
            if (rzy < 0xff && my < 0x10 && my){
		//my = (my<<8)/(rzy+1);
                my = my*255/rzy;
                if (my > 0x10) my = 0x10;
            }
      
            if (my > 0x20) my=0x20;
      
            ddax=0;	/* =16; NS990110 neodrift fix */	/* setup x zoom */
        }

	rzx=zx+1;

	/* No point doing anything if tile strip is 0 */
        if (my==0) continue;
    
	if ( sx >= 0x1F0 ) sx -= 0x200;
        if(sx>=320) continue;
	//if (sx<-16) continue;

        offs = count<<6;
	strips[count>>1].nb_tile=my;
	strips[count>>1].x=sx;
	strips[count>>1].y=sy;
	strips[count>>1].zx=rzx;
	strips[count>>1].zy=rzy/16.0;
	fgl_drawstrip(offs,&strips[count>>1]);



    }  /* for count */

    SDL_GL_SwapBuffers();
}

#endif
