/* drawing routine */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "draw.h"
#include "zlib.h"
#ifdef HAVE_LIBSDL_IMAGE
#include "SDL_image.h"
#endif
void draw_v_line(SDL_Surface *s,int x,int y,int l,Uint32 color)
{
  SDL_Rect r;
  r.x=x;
  r.y=y;
  r.w=1;
  r.h=l;
  SDL_FillRect(s,&r,color);
}

void draw_h_line(SDL_Surface *s,int x,int y,int l,Uint32 color)
{
  SDL_Rect r;
  r.x=x;
  r.y=y;
  r.w=l;
  r.h=1;
  SDL_FillRect(s,&r,color);
}

void draw_dotted_h_line(SDL_Surface *s,int x,int y,int l,Uint32 color) {
    int i;
    int start=(x<s->clip_rect.x?s->clip_rect.x:x);
    int end=(x+l>s->clip_rect.x+s->clip_rect.w?s->clip_rect.x+s->clip_rect.w:x+l);
    Uint16 *buf;
    if (y<s->clip_rect.y || y>s->clip_rect.y+s->clip_rect.h) return;
    /* TODO: other pixel format */
    buf=(Uint16*)s->pixels+(y*s->w);
    for (i=start;i<end;i+=2) {
	buf[i]=color;
    }
}

void draw_dotted_v_line(SDL_Surface *s,int x,int y,int l,Uint32 color) {
    int i;
    int start=(y<s->clip_rect.y?s->clip_rect.y:y);
    int end=(y+l>s->clip_rect.y+s->clip_rect.h?s->clip_rect.y+s->clip_rect.h:y+l);
    Uint16 *buf;
    if (x<s->clip_rect.x || x>s->clip_rect.x+s->clip_rect.w) return;
    /* TODO: other pixel format */
    buf=(Uint16*)s->pixels+(start*s->w)+x;
    for (i=start;i<end;i+=2) {
	buf[0]=color;
	buf+=2*s->w;
    }
}

void draw_dotted_border(SDL_Surface *s,SDL_Rect *r,Uint32 color) {
    draw_dotted_h_line(s,r->x,r->y,r->w,color);
    draw_dotted_h_line(s,r->x,r->y+r->h,r->w,color);
    draw_dotted_v_line(s,r->x,r->y,r->h,color);
    draw_dotted_v_line(s,r->x+r->w,r->y,r->h,color);
}

static inline void inter_draw_border(SDL_Surface *s,int type,SDL_Rect *r)
{
    switch(type) {
    case SHADOW_OUT:
	draw_h_line(s,r->x,r->y,r->w,COL32_TO_16(0xFFFFFF));
	draw_h_line(s,r->x+1,r->y+1,r->w-1,COL32_TO_16(0xd6d6d6));
	draw_h_line(s,r->x,r->y+r->h-1,r->w,COL32_TO_16(0x000000));
	draw_h_line(s,r->x+2,r->y+r->h-2,r->w-3,COL32_TO_16(0x969696));

	draw_v_line(s,r->x,r->y,r->h,COL32_TO_16(0xFFFFFF));
	draw_v_line(s,r->x+1,r->y+1,r->h-2,COL32_TO_16(0xd6d6d6));
	draw_v_line(s,r->x+r->w-1,r->y,r->h,COL32_TO_16(0x000000));
	draw_v_line(s,r->x+r->w-2,r->y+1,r->h-2,COL32_TO_16(0x969696));
	break;
    case SHADOW_IN:
	draw_h_line(s,r->x,r->y,r->w,COL32_TO_16(0x000000));
	draw_h_line(s,r->x+1,r->y+1,r->w-1,COL32_TO_16(0x969696));
	draw_h_line(s,r->x,r->y+r->h-1,r->w,COL32_TO_16(0xFFFFFF));
	draw_h_line(s,r->x+2,r->y+r->h-2,r->w-3,COL32_TO_16(0xd6d6d6));
    
	draw_v_line(s,r->x,r->y,r->h,COL32_TO_16(0x000000));
	draw_v_line(s,r->x+1,r->y+1,r->h-2,COL32_TO_16(0x969696));
	draw_v_line(s,r->x+r->w-1,r->y,r->h,COL32_TO_16(0xFFFFFF));
	draw_v_line(s,r->x+r->w-2,r->y+1,r->h-2,COL32_TO_16(0xd6d6d6));
	break;
    case SHADOW_ETCHED_IN:
	draw_h_line(s,r->x,r->y,r->w,COL32_TO_16(0x969696));
	draw_h_line(s,r->x+1,r->y+1,r->w-1,COL32_TO_16(0xFFFFFF));
	draw_h_line(s,r->x,r->y+r->h-1,r->w-1,COL32_TO_16(0x969696));
	draw_h_line(s,r->x,r->y+r->h,r->w,COL32_TO_16(0xFFFFFF));

	draw_v_line(s,r->x,r->y,r->h-1,COL32_TO_16(0x969696));
	draw_v_line(s,r->x+1,r->y+1,r->h-2,COL32_TO_16(0xFFFFFF));
	draw_v_line(s,r->x+r->w-1,r->y+2,r->h-2,COL32_TO_16(0x969696));
	draw_v_line(s,r->x+r->w,r->y+1,r->h,COL32_TO_16(0xFFFFFF));
	break;
    case SHADOW_NONE:
	break;
    default:
	break;
    }
}

void draw_border(SDL_Surface *s,int type,SDL_Rect *r) {
    SDL_FillRect(s,r,COL32_TO_16(0xd6d6d6));
    inter_draw_border(s,type,r);
}

void draw_border_color(SDL_Surface *s,int type,SDL_Rect *r,Uint32 col) {
    SDL_FillRect(s,r,COL32_TO_16(col));
    inter_draw_border(s,type,r);
}

void draw_border_label(SDL_Surface *s,SDL_Surface *font,int type,SDL_Rect *r,char *label) {
    int lw=(font->w/(float)95)*strlen(label);
    int lh=font->h;
    int dt=(lh-2)/(float)2;
    SDL_Rect wrect={r->x+dt,r->y+dt,r->w-dt*2,r->h-dt*2};
    SDL_Rect lrect={r->x+14,r->y,lw+2,lh};

    SDL_FillRect(s,r,COL32_TO_16(0xd6d6d6));
    inter_draw_border(s,type,&wrect);
    SDL_FillRect(s,&lrect,COL32_TO_16(0xd6d6d6));
    draw_string(s,font,r->x+15,r->y,label);
}

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

void draw_string(SDL_Surface *s,SDL_Surface *font,int x,int y,char *string) {
    int i;
    int font_w=font->w/(float)95;
    

    if (string && s && font)
	for (i = 0; i < strlen(string); i++)
	    dr_putchar(s,font, x + i * font_w, y, string[i]);

}
/*
SDL_Surface *load_ck_bmp(char *data_path,char *name) {
    char bmp[strlen(data_path)+strlen(name)+2];
    SDL_Surface *surf1,*surf2;

    sprintf(bmp,"%s/%s",data_path,name);

    surf1=SDL_LoadBMP(bmp);
    if (surf1==NULL) {
	printf("Cant load %s\n",bmp);
	exit(1);
    }
    SDL_SetColorKey(surf1,SDL_SRCCOLORKEY,SDL_MapRGB(surf1->format,0xFF,0,0xFF));
    surf2=SDL_DisplayFormat(surf1);
    SDL_FreeSurface(surf1);
    return surf2;
}
*/
static SDL_Surface *intern_load_img(char *path,char *name,SDL_bool ckey) {
#ifdef HAVE_LIBSDL_IMAGE
    static char *ext[]={"","png","pnm","bmp","xpm","xcf","pcx","gif","jpg","tif","lbm",NULL};
#else
    static char *ext[]={"","bmp",NULL};
#endif
    char filename[strlen(path)+1+strlen(name)+5];
    int i=0;
    SDL_Surface *surf1=NULL,*surf2;
    while(ext[i]!=NULL) {
	sprintf(filename,"%s/%s.%s",path,name,ext[i]);
#ifdef HAVE_LIBSDL_IMAGE
	surf1=IMG_Load(filename);
#else
	surf1=SDL_LoadBMP(filename);
#endif
	if (surf1!=NULL)
	    break;
	i++;
    }
    if (surf1) {
	if (ckey==SDL_TRUE) SDL_SetColorKey(surf1,SDL_SRCCOLORKEY,SDL_MapRGB(surf1->format,0xFF,0,0xFF));
	surf2=SDL_DisplayFormat(surf1);
	SDL_FreeSurface(surf1);
	return surf2;
    }
    return NULL;
}


/* try to load the file 'name' 
   if it fail, try to add different extension.
   if it fail, return NULL;
   The surface is convert to the format of the screen
 */
SDL_Surface *load_img(char *path,char *name) {
    return intern_load_img(path,name,SDL_FALSE);
}

/* Same as load_img, but with colorkey */
SDL_Surface *load_img_rle(char *path,char *name) {
    return intern_load_img(path,name,SDL_TRUE);
}
