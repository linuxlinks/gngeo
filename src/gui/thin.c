#include <string.h>
#include "SDL.h"
#include "gui.h"
#include "style.h"
#include "thin.h"
#include "draw.h"

/* Brown */
/*
#define BACK_COLOR   0xAC9D8B
#define BACK_FOCUS_COLOR   0xD5C3AC
#define LIGHT_COLOR  0xFFEACD
#define DARK_COLOR   0x5A4C4A 
#define TXTBG_COLOR  0xFFFFFF
#define SELECT_COLOR 0xFFEACD
#define SBBACK_COLOR 0x7A5C5A
*/

/* Gtk2 Simple color profile */
#define BACK_COLOR   0xE6E2DE
#define BACK_FOCUS_COLOR  0xAAD2AF
#define LIGHT_COLOR  0xFFFFFF
#define DARK_COLOR   0xA4A19C 
#define TXTBG_COLOR  0xFFFFFF
#define SELECT_COLOR 0xAAD2AF
#define SBBACK_COLOR 0xC5C2BD



static SDL_Surface *cursor;
static SDL_Surface *thin_font;
static SDL_Surface *sb_arrows;
static SDL_Rect arr_up_rect={0,0,7,7};
static SDL_Rect arr_down_rect={7,0,7,7};
static SDL_Rect arr_right_rect={14,0,7,7};
static SDL_Rect arr_left_rect={21,0,7,7};

static Uint16 fontw,fonth;

static void drw_border_out(SDL_Surface *desk,SDL_Rect *rect) {
    draw_h_line(desk,rect->x,rect->y,rect->w,COL32_TO_16(LIGHT_COLOR));
    draw_h_line(desk,rect->x,rect->y+rect->h-1,rect->w,COL32_TO_16(DARK_COLOR));
    draw_v_line(desk,rect->x,rect->y,rect->h,COL32_TO_16(LIGHT_COLOR));
    draw_v_line(desk,rect->x+rect->w-1,rect->y,rect->h,COL32_TO_16(DARK_COLOR));
}
static void drw_border_in(SDL_Surface *desk,SDL_Rect *rect) {
    draw_h_line(desk,rect->x,rect->y,rect->w,COL32_TO_16(DARK_COLOR));
    draw_h_line(desk,rect->x,rect->y+rect->h-1,rect->w,COL32_TO_16(LIGHT_COLOR));
    draw_v_line(desk,rect->x,rect->y,rect->h,COL32_TO_16(DARK_COLOR));
    draw_v_line(desk,rect->x+rect->w-1,rect->y,rect->h,COL32_TO_16(LIGHT_COLOR));
}

static void drw_button(SDL_Surface *desk,SDL_Rect *rect,Uint32 state) {
    SDL_Rect focus_rect={rect->x+1,rect->y+1,rect->w-3,rect->h-3};
    
    if (state&FLAG_MOUSEOVER) 
	SDL_FillRect(desk,rect,COL32_TO_16(BACK_FOCUS_COLOR));
    else
	SDL_FillRect(desk,rect,COL32_TO_16(BACK_COLOR));
    if (!((state&FLAG_MOUSECLICK)==FLAG_MOUSECLICK)) {
	drw_border_out(desk,rect);
    } else {
	drw_border_in(desk,rect);
    }
    if (state&FLAG_FOCUS)
	draw_dotted_border(desk,&focus_rect,COL32_TO_16(DARK_COLOR));
}

static void drw_ck_button(SDL_Surface *desk,SDL_Rect *rect,Uint32 state) {
    if (state&FLAG_MOUSEOVER) 
	SDL_FillRect(desk,rect,COL32_TO_16(BACK_FOCUS_COLOR));
    else
	SDL_FillRect(desk,rect,COL32_TO_16(BACK_COLOR));
    if (!((state&FLAG_CHECKED)==FLAG_CHECKED)) {
	drw_border_out(desk,rect);
    } else {
	drw_border_in(desk,rect);
    }
}

/*
static void drw_check_button(SDL_Surface *desk,SDL_Rect *rect,Uint8 state) {

}
*/
static void drw_text(SDL_Surface *desk,SDL_Rect *rect,char *text) {
    draw_string(desk,thin_font,rect->x,rect->y,text);
}

static void drw_label(SDL_Surface *desk,SDL_Rect *rect,char *text) {
     draw_string(desk,thin_font,rect->x,rect->y,text);
}

static void drw_frame(SDL_Surface *desk,SDL_Rect *rect,Uint8 type,char *label)
{
    SDL_Rect frect=*rect;
    if (!(type&FRAME_NO_BACK)) {
	SDL_FillRect(desk,rect,COL32_TO_16(BACK_COLOR));
    }
    if (label) {
	frect.y+=fonth/2-1;
	frect.h-=fonth/2-1;
    }
    if (type&FRAME_SHADOW_OUT) {
	drw_border_out(desk,rect);
    } else if (type&FRAME_SHADOW_IN) {
	drw_border_in(desk,rect);
    } else { /* etched */
	draw_h_line(desk,frect.x,frect.y,frect.w,COL32_TO_16(DARK_COLOR));
	draw_h_line(desk,frect.x,frect.y+frect.h-1,frect.w,COL32_TO_16(DARK_COLOR));
	draw_v_line(desk,frect.x,frect.y,frect.h,COL32_TO_16(DARK_COLOR));
	draw_v_line(desk,frect.x+frect.w-1,frect.y,frect.h,COL32_TO_16(DARK_COLOR));
    }
    if (label) {
	SDL_Rect lrect={rect->x+9,rect->y,style_strlen(label)+2,fonth};
	SDL_FillRect(desk,&lrect,COL32_TO_16(BACK_COLOR));
	draw_string(desk,thin_font,rect->x+10,rect->y,label);
    }
}

static void drw_selection(SDL_Surface *desk,SDL_Rect *rect) {
    SDL_FillRect(desk,rect,COL32_TO_16(SELECT_COLOR));
}

static void drw_cursor(SDL_Surface *desk,SDL_Rect *rect) {
    SDL_BlitSurface(cursor,NULL,desk,rect);
}

static void drw_txt_cursor(SDL_Surface *desk,Uint16 x,Uint16 y,Uint16 pos) {
    draw_v_line(desk,x+pos*fontw-1,y-1,fonth+2,0x000000);
}

static void drw_txtbg(SDL_Surface *desk,SDL_Rect *rect) {
    SDL_FillRect(desk,rect,COL32_TO_16(TXTBG_COLOR));
}

static void drw_focus_hlight(SDL_Surface *desk,SDL_Rect *rect) {
    SDL_FillRect(desk,rect,COL32_TO_16(BACK_FOCUS_COLOR));
}
static void drw_key_focus_hlight(SDL_Surface *desk,SDL_Rect *rect) {
    draw_dotted_border(desk,rect,COL32_TO_16(DARK_COLOR));
}

static void drw_sbup(SDL_Surface *desk,SDL_Rect *rect,Uint8 dir,Uint32 state) {
    SDL_Rect dest_rect={rect->x+1,rect->y+1,7,7};
    SDL_Rect focus_rect={rect->x+1,rect->y+1,rect->w-3,rect->h-3};

    if (state&FLAG_MOUSEOVER) 
	SDL_FillRect(desk,rect,COL32_TO_16(BACK_FOCUS_COLOR));
    else
	SDL_FillRect(desk,rect,COL32_TO_16(BACK_COLOR));

    if (!((state&FLAG_MOUSECLICK)==FLAG_MOUSECLICK)) {
	drw_border_out(desk,rect);
    } else {
	dest_rect.x++;
	dest_rect.y++;
	drw_border_in(desk,rect);
    }
    switch (dir) {
    case SB_VERTICAL:
	SDL_BlitSurface(sb_arrows,&arr_up_rect,desk,&dest_rect); break;
    case SB_HORIZONTAL:
	SDL_BlitSurface(sb_arrows,&arr_left_rect,desk,&dest_rect); break;
    }
    if (state&FLAG_FOCUS)
	draw_dotted_border(desk,&focus_rect,COL32_TO_16(DARK_COLOR));
}
static void drw_sbdown(SDL_Surface *desk,SDL_Rect *rect,Uint8 dir,Uint32 state) {
    SDL_Rect dest_rect={rect->x+1,rect->y+1,7,7};
    SDL_Rect focus_rect={rect->x+1,rect->y+1,rect->w-3,rect->h-3};

    if (state&FLAG_MOUSEOVER) 
	SDL_FillRect(desk,rect,COL32_TO_16(BACK_FOCUS_COLOR));
    else
	SDL_FillRect(desk,rect,COL32_TO_16(BACK_COLOR));
    
    if (!((state&FLAG_MOUSECLICK)==FLAG_MOUSECLICK)) {
	drw_border_out(desk,rect);
    } else {
	dest_rect.x++;
	dest_rect.y++;
	drw_border_in(desk,rect);
    }
    switch (dir) {
    case SB_VERTICAL:
	SDL_BlitSurface(sb_arrows,&arr_down_rect,desk,&dest_rect); break;
    case SB_HORIZONTAL:
	SDL_BlitSurface(sb_arrows,&arr_right_rect,desk,&dest_rect); break;
    }
    if (state&FLAG_FOCUS)
	draw_dotted_border(desk,&focus_rect,COL32_TO_16(DARK_COLOR));
}
static void drw_sbmobil(SDL_Surface *desk,SDL_Rect *rect,Uint8 dir,Uint32 state) 
{
    SDL_Rect mobil_rect={rect->x+(rect->w-4)/2.0,rect->y+(rect->h-4)/2.0,4,4};

    if (state&FLAG_MOUSEOVER) 
	SDL_FillRect(desk,rect,COL32_TO_16(BACK_FOCUS_COLOR));
    else
	SDL_FillRect(desk,rect,COL32_TO_16(BACK_COLOR));
    if (!((state&FLAG_MOUSECLICK)==FLAG_MOUSECLICK)) {
	drw_border_out(desk,rect);
    } else {
	drw_border_in(desk,rect);
    } 
    drw_border_in(desk,&mobil_rect);
}
static void drw_sbback(SDL_Surface *desk,SDL_Rect *rect,Uint8 dir,Uint32 state) 
{
    SDL_FillRect(desk,rect,COL32_TO_16(SBBACK_COLOR));
}

static void drw_progress(SDL_Surface *desk,SDL_Rect *rect,Uint32 size,Uint32 val) 
{
    static char percent[5];
    Uint16 tx,ty;

    SDL_Rect frect={rect->x,rect->y,rect->w,rect->h};
    frect.w=rect->w*val/(float)size;
    SDL_FillRect(desk,rect,COL32_TO_16(TXTBG_COLOR));
    SDL_FillRect(desk,&frect,COL32_TO_16(SELECT_COLOR));

    sprintf(percent,"%d%%",(int)(val*100.0/(float)size));
    ty=(rect->h-fonth)/2.0 +1;
    tx=(rect->w-(fonth*strlen(percent)/2.0))/2.0;
    draw_string(desk,thin_font,rect->x+tx,rect->y+ty,percent);

    drw_border_in(desk,rect);
}

static void get_font_size(Uint16 *w,Uint16 *h) {
    *w=fontw;
    *h=fonth;
}

static void get_border_size(Uint16 *w,Uint16 *h) {
    *w=1;
    *h=1;
}

SDL_bool thin_init(char *datapath) {
    style_drw_text=drw_text;
    style_drw_label=drw_label;
    style_drw_frame=drw_frame;
    style_drw_selection=drw_selection;
    style_drw_button=drw_button;
    style_drw_check_button=drw_ck_button;
    style_drw_cursor=drw_cursor;
    style_drw_txtbg=drw_txtbg;
    style_drw_focus_hlight=drw_focus_hlight;
    style_drw_txt_cursor=drw_txt_cursor;
    style_drw_sbup=drw_sbup;
    style_drw_sbdown=drw_sbdown;
    style_drw_sbmobil=drw_sbmobil;
    style_drw_sbback=drw_sbback;
    style_drw_key_focus_hlight=drw_key_focus_hlight;

    style_drw_progress=drw_progress;

    style_get_font_size=get_font_size;
    style_get_border_size=get_border_size;


    /* load data (should go in style )*/
    cursor=load_img_rle(datapath,"cursor");
    //thin_font=load_ck_bmp(datapath,"gui_font2.bmp");
    thin_font=load_img_rle(datapath,"little_font");
    sb_arrows=load_img_rle(datapath,"sb_arrows");
    fontw=(thin_font->w/(float)95);
    fonth=thin_font->h;

    return SDL_TRUE;
}

void thin_close(void) {

}
