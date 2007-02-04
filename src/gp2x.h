#ifndef _GP2X_H_
#define _GP2X_H_


/* GP2X upper Memory management */
//Uint32 gp2x_upmem_base;
int gp2x_dev_mem;
int gp2x_gfx_dump;
int gp2x_mixer;

volatile Uint8 *gp2x_ram;
volatile Uint8 *gp2x_ram2;
volatile Uint16 *gp2x_memregs;
volatile Uint32 *gp2x_memregl;

void gp2x_ram_init(void);
Uint8 *gp2x_ram_malloc(size_t size,Uint32 page);
void gp2x_quit(void);
void gp2x_sound_volume_set(int l, int r);
int gp2x_sound_volume_get(void);
void gp2x_init(void);
void gp2x_video_RGB_setscaling(int W, int H);
void gp2x_set_cpu_speed(void);
Uint32 gp2x_is_tvout_on(void);

enum  { GP2X_UP=0,
	GP2X_UP_LEFT,
	GP2X_LEFT,
	GP2X_DOWN_LEFT,
	GP2X_DOWN,
	GP2X_DOWN_RIGHT,
	GP2X_RIGHT,
	GP2X_UP_RIGHT,
	GP2X_START,
	GP2X_SELECT,
	GP2X_R,
	GP2X_L,
	GP2X_A,
	GP2X_B,
	GP2X_X,
	GP2X_Y,
	GP2X_VOL_UP,
	GP2X_VOL_DOWN,
	GP2X_PUSH
};


#endif
