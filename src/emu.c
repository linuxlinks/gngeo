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

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "emu.h"
#include "memory.h"
#include "frame_skip.h"
#include "pd4990a.h"
#include "messages.h"
#include "profiler.h"
#include "debug.h"

#include "timer.h"
//#include "streams.h"
#include "ym2610/2610intf.h"
#include "sound.h"
#include "screen.h"
#include "neocrypt.h"
#include "conf.h"
//#include "driver.h"
//#include "gui_interf.h"
#ifdef FULL_GL
#include "videogl.h"
#endif
#ifdef GP2X
#include "gp2x.h"

#include "ym2610-940/940shared.h"
#include "ym2610-940/940private.h"
#endif
#include "menu.h"
#include "event.h"

int frame;
int nb_interlace = 256;
int current_line;
extern SDL_Surface *state_img;
//static int arcade;
//
//extern int irq2enable, irq2start, irq2repeat, irq2control, irq2taken;
//extern int lastirq2line;
//extern int irq2repeat_limit;
//extern Uint32 irq2pos_value;

void setup_misc_patch(char *name) {


	if (!strcmp(name, "ssideki")) {
		WRITE_WORD_ROM(&memory.rom.cpu_m68k.p[0x2240], 0x4e71);
	}

	//if (!strcmp(name, "fatfury3")) {
	//	WRITE_WORD_ROM(memory.rom.cpu_m68k.p, 0x0010);
	//}

	if (!strcmp(name, "mslugx")) {
		/* patch out protection checks */
		int i;
		Uint8 *RAM = memory.rom.cpu_m68k.p;
		for (i = 0; i < memory.rom.cpu_m68k.size; i += 2) {
			if ((READ_WORD_ROM(&RAM[i + 0]) == 0x0243)
					&& (READ_WORD_ROM(&RAM[i + 2]) == 0x0001) && /* andi.w  #$1, D3 */
			(READ_WORD_ROM(&RAM[i + 4]) == 0x6600)) { /* bne xxxx */

				WRITE_WORD_ROM(&RAM[i + 4], 0x4e71);
				WRITE_WORD_ROM(&RAM[i + 6], 0x4e71);
			}
		}

		WRITE_WORD_ROM(&RAM[0x3bdc], 0x4e71);
		WRITE_WORD_ROM(&RAM[0x3bde], 0x4e71);
		WRITE_WORD_ROM(&RAM[0x3be0], 0x4e71);
		WRITE_WORD_ROM(&RAM[0x3c0c], 0x4e71);
		WRITE_WORD_ROM(&RAM[0x3c0e], 0x4e71);
		WRITE_WORD_ROM(&RAM[0x3c10], 0x4e71);

		WRITE_WORD_ROM(&RAM[0x3c36], 0x4e71);
		WRITE_WORD_ROM(&RAM[0x3c38], 0x4e71);
	}

}

void neogeo_reset(void) {
    //	memory.vid.modulo = 1; /* TODO: Move to init_video */
	sram_lock = 0;
	sound_code = 0;
	pending_command = 0;
	result_code = 0;
#ifdef ENABLE_940T
	shared_ctl->sound_code = sound_code;
	shared_ctl->pending_command = pending_command;
	shared_ctl->result_code = result_code;
#endif
	if (memory.rom.cpu_m68k.size > 0x100000)
		cpu_68k_bankswitch(0x100000);
	else
		cpu_68k_bankswitch(0);
	cpu_68k_reset();

}

void init_sound(void) {

	if (conf.sound) init_sdl_audio();

#ifdef ENABLE_940T
		printf("Init all neo");
		shared_data->sample_rate = conf.sample_rate;
		shared_data->z80_cycle = (z80_overclk == 0 ? 73333 : 73333
				+ (z80_overclk * 73333 / 100.0));
		//gp2x_add_job940(JOB940_INITALL);
		gp2x_add_job940(JOB940_INITALL);
		wait_busy_940(JOB940_INITALL);
		printf("The YM2610 have been initialized\n");
#else
		cpu_z80_init();
		//streams_sh_start();
		YM2610_sh_start();
#endif
	if (conf.sound)	pause_audio(0);

}

void init_neo(void) {
#ifdef ENABLE_940T
	int z80_overclk = CF_VAL(cf_get_item_by_name("z80clock"));
#endif

	neogeo_init_save_state();

#ifdef GP2X
	gp2x_ram_ptr_reset();
#endif

	cpu_68k_init();
//	neogeo_reset();
	pd4990a_init();
//	setup_misc_patch(rom_name);

	init_sound();

	neogeo_reset();
}

static void take_screenshot(void) {
	time_t ltime;
	struct tm *today;
	char buf[256];
	char date_str[101];
	//static SDL_Rect buf_rect    =	{16, 16, 304, 224};
	static SDL_Rect screen_rect = { 0, 0, 304, 224 };
	static SDL_Surface *shoot;

	screen_rect.w = visible_area.w;
	screen_rect.h = visible_area.h;

	if (shoot == NULL)
		shoot = SDL_CreateRGBSurface(SDL_SWSURFACE, visible_area.w,
				visible_area.h, 16, 0xF800, 0x7E0, 0x1F, 0);

	time(&ltime);
	today = localtime(&ltime);
#if defined (__AMIGA__)
	strftime (date_str,100,"%Y%m%d_%H%M",today);
	snprintf(buf,255,"%s/%s_%s.bmp","/PROGDIR/shots",conf.game,date_str);
#else
	strftime(date_str, 100, "%a_%b_%d_%T_%Y", today);
	snprintf(buf, 255, "%s/%s_%s.bmp", getenv("HOME"), conf.game, date_str);
#endif
	printf("save to %s\n", buf);

	SDL_BlitSurface(buffer, &visible_area, shoot, &screen_rect);
	SDL_SaveBMP(shoot, buf);
}

static int fc;
static int last_line;
static int skip_this_frame = 0;

static inline int neo_interrupt(void) {
    static int frames;

	pd4990a_addretrace();
	// printf("neogeo_frame_counter_speed %d\n",neogeo_frame_counter_speed);
	if (!(memory.vid.irq2control & 0x8)) {
		if (fc >= neogeo_frame_counter_speed) {
			fc = 0;
			neogeo_frame_counter++;
		}
		fc++;
	}

	skip_this_frame = skip_next_frame;
	skip_next_frame = frame_skip(0);

	if (!skip_this_frame) {
		PROFILER_START(PROF_VIDEO);

		draw_screen();

		PROFILER_STOP(PROF_VIDEO);
	}
    /*
    frames++;
    printf("FRAME %d\n",frames);
    if (frames==262) {
        FILE *f;
        sleep(1);
        f=fopen("/tmp/video.dmp","wb");
        fwrite(&memory.vid.ram,0x20000,1,f);
        fclose(f);
    }
    */
	return 1;
}

static inline void update_screen(void) {

	if (memory.vid.irq2control & 0x40)
		memory.vid.irq2start = (memory.vid.irq2pos + 3) / 0x180; /* ridhero gives 0x17d */
	else
		memory.vid.irq2start = 1000;

	if (!skip_this_frame) {
		if (last_line < 21) { /* there was no IRQ2 while the beam was in the
							 * visible area -> no need for scanline rendering */
			draw_screen();
		} else {
			draw_screen_scanline(last_line - 21, 262, 1);
		}
	}

	last_line = 0;

	pd4990a_addretrace();
	if (fc >= neogeo_frame_counter_speed) {
		fc = 0;
		neogeo_frame_counter++;
	}
	fc++;

	skip_this_frame = skip_next_frame;
	skip_next_frame = frame_skip(0);
}

static inline int update_scanline(void) {
	memory.vid.irq2taken = 0;

	if (memory.vid.irq2control & 0x10) {

		if (current_line == memory.vid.irq2start) {
			if (memory.vid.irq2control & 0x80)
				memory.vid.irq2start += (memory.vid.irq2pos + 3) / 0x180;
			memory.vid.irq2taken = 1;
		}
	}

	if (memory.vid.irq2taken) {
		if (!skip_this_frame) {
			if (last_line < 21)
				last_line = 21;
			if (current_line < 20)
				current_line = 20;
			draw_screen_scanline(last_line - 21, current_line - 20, 0);
		}
		last_line = current_line;
	}
	current_line++;
	return memory.vid.irq2taken;
}

static int slow_motion = 0;

void main_loop(void) {
	int neo_emu_done = 0;
	int m68k_overclk = CF_VAL(cf_get_item_by_name("68kclock"));
	int z80_overclk = CF_VAL(cf_get_item_by_name("z80clock"));
	//int nb_frames = 0;
	int a,i;
#ifdef GP2X
	//int snd_volume=gp2x_sound_volume_get();
	int snd_volume = 60;
	char volbuf[21];

	FILE *sndbuf;
	unsigned int sample_len = conf.sample_rate / 60.0;
	static unsigned int gp2x_timer;
	static unsigned int gp2x_timer_prev;
#endif
	static SDL_Rect buf_rect = { 24, 16, 304, 224 };
	static SDL_Rect screen_rect = { 0, 0, 304, 224 };
	Uint32 cpu_68k_timeslice = (m68k_overclk == 0 ? 200000 : 200000
			+ (m68k_overclk * 200000 / 100.0));
	Uint32 cpu_68k_timeslice_scanline = cpu_68k_timeslice / 264.0;
	Uint32 cpu_z80_timeslice = (z80_overclk == 0 ? 73333 : 73333 + (z80_overclk
			* 73333 / 100.0));
	Uint32 tm_cycle = 0;

	Uint32 cpu_z80_timeslice_interlace = cpu_z80_timeslice
			/ (float) nb_interlace;

#ifdef GP2X
	gp2x_sound_volume_set(snd_volume, snd_volume);
#endif

	reset_frame_skip();
	my_timer();

	//pause_audio(0);
	while (!neo_emu_done) {
		if (conf.test_switch == 1)
			conf.test_switch = 0;

		//neo_emu_done=
		if (handle_event()) {
			int interp = interpolation;
			SDL_BlitSurface(buffer, &buf_rect, state_img, &screen_rect);
			interpolation = 0;
			if (conf.sound) pause_audio(1);
			if (run_menu() == 2) {
				neo_emu_done = 1;/*printf("Unlock audio\n");SDL_UnlockAudio()*/
				return;
			} // A bit ugly...
			if (conf.sound) pause_audio(0);
			//neo_emu_done = 1;
			interpolation = interp;
			reset_frame_skip();
			reset_event();
		}

#if 0
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_JOYAXISMOTION:
				if (event.jaxis.value >5000 || event.jaxis.value<-5000)
				printf("AXE %d %d dir %d\n",event.jaxis.which,event.jaxis.axis,event.jaxis.value);
				joy_axe[event.jaxis.which][event.jaxis.axis] = event.jaxis.value;
				if (show_keysym) {
					sprintf(ksym_code, "%d", event.jaxis.axis);
					draw_message(ksym_code);
				}
				break;

				case SDL_JOYHATMOTION:
				printf("HAT %d dir %d\n",event.jhat.which,event.jhat.value);
				switch (event.jhat.value) {
					case SDL_HAT_CENTERED:
					joy_axe[event.jhat.which][(event.jhat.hat * 2) + joy_numaxes[event.jhat.which]] = 0;
					joy_axe[event.jhat.which][(event.jhat.hat * 2) + joy_numaxes[event.jhat.which] + 1] = 0;
					break;

					case SDL_HAT_UP:
					joy_axe[event.jhat.which][(event.jhat.hat * 2) + joy_numaxes[event.jhat.which] + 1] = -32767;
					joy_axe[event.jhat.which][(event.jhat.hat * 2) + joy_numaxes[event.jhat.which]] = 0;
					break;

					case SDL_HAT_DOWN:
					joy_axe[event.jhat.which][(event.jhat.hat * 2) + joy_numaxes[event.jhat.which] + 1] = 32767;
					joy_axe[event.jhat.which][(event.jhat.hat * 2) + joy_numaxes[event.jhat.which]] = 0;
					break;

					case SDL_HAT_LEFT:
					joy_axe[event.jhat.which][(event.jhat.hat * 2) + joy_numaxes[event.jhat.which]] = -32767;
					joy_axe[event.jhat.which][(event.jhat.hat * 2) + joy_numaxes[event.jhat.which] + 1] = 0;
					break;

					case SDL_HAT_RIGHT:
					joy_axe[event.jhat.which][(event.jhat.hat * 2) + joy_numaxes[event.jhat.which]] = 32767;
					joy_axe[event.jhat.which][(event.jhat.hat * 2) + joy_numaxes[event.jhat.which] + 1] = 0;
					break;

					case SDL_HAT_RIGHTUP:
					joy_axe[event.jhat.which][(event.jhat.hat * 2) + joy_numaxes[event.jhat.which]] = 32767;
					joy_axe[event.jhat.which][(event.jhat.hat * 2) + joy_numaxes[event.jhat.which] + 1] = -32767;
					break;

					case SDL_HAT_RIGHTDOWN:
					joy_axe[event.jhat.which][(event.jhat.hat * 2) + joy_numaxes[event.jhat.which]] = 32767;
					joy_axe[event.jhat.which][(event.jhat.hat * 2) + joy_numaxes[event.jhat.which] + 1] = 32767;
					break;

					case SDL_HAT_LEFTUP:
					joy_axe[event.jhat.which][(event.jhat.hat * 2) + joy_numaxes[event.jhat.which]] = -32767;
					joy_axe[event.jhat.which][(event.jhat.hat * 2) + joy_numaxes[event.jhat.which] + 1] = -32767;
					break;

					case SDL_HAT_LEFTDOWN:
					joy_axe[event.jhat.which][(event.jhat.hat * 2) + joy_numaxes[event.jhat.which]] = -32767;
					joy_axe[event.jhat.which][(event.jhat.hat * 2) + joy_numaxes[event.jhat.which] + 1] = 32767;
					break;

				}

				if (show_keysym) {
					sprintf(ksym_code, "%d", event.jhat.hat);
					draw_message(ksym_code);
				}
				break;

				case SDL_JOYBUTTONDOWN:
				joy_button[event.jbutton.which][event.jbutton.button] = 1;
				//printf("Joy event %d %d\n",event.jbutton.which,event.jbutton.button);

				if (show_keysym) {
					sprintf(ksym_code, "%d", event.jbutton.button);
					draw_message(ksym_code);
				}
#ifdef GP2X
				if ((joy_button[0][GP2X_VOL_UP]) && conf.sound) {
					if (snd_volume<100) snd_volume+=5; else snd_volume=100;
					gp2x_sound_volume_set(snd_volume,snd_volume);
					for (i=0;i<snd_volume/5;i++) volbuf[i]='|';
					for (i=snd_volume/5;i<20;i++) volbuf[i]='-';
					volbuf[20]=0;
					draw_message(volbuf);
				}
				if ((joy_button[0][GP2X_VOL_DOWN] && conf.sound)) {
					if (snd_volume>0) snd_volume-=5; else snd_volume=0;
					gp2x_sound_volume_set(snd_volume,snd_volume);
					for (i=0;i<snd_volume/5;i++) volbuf[i]='|';
					for (i=snd_volume/5;i<20;i++) volbuf[i]='-';
					volbuf[20]=0;
					draw_message(volbuf);
				}
				if ((joy_button[0][GP2X_PUSH]) &&
						(joy_button[0][GP2X_VOL_DOWN]) &&
						(joy_button[0][GP2X_VOL_UP]) ) {
					//if ((joy_button[0][GP2X_L])) {
					draw_message("Test Switch ON");
					conf.test_switch = 1;
					break;
				}
				//if ((joy_button[0][GP2X_PUSH]) && (joy_button[0][GP2X_R])) {
				if (joy_button[0][GP2X_R] && joy_button[0][GP2X_L] &&
						(joy_button[0][GP2X_START] || joy_button[0][GP2X_SELECT])) {
					joy_button[0][GP2X_R] = joy_button[0][GP2X_L] = 0;
					joy_button[0][GP2X_START] = joy_button[0][GP2X_SELECT] = 0;

					SDL_BlitSurface(buffer, &buf_rect, state_img, &screen_rect);
					if (conf.sound) {pause_audio(1); SDL_LockAudio();}
					if (run_menu()==2) {neo_emu_done = 1;SDL_UnlockAudio();} // A bit ugly...
					if (conf.sound) {pause_audio(0); SDL_UnlockAudio();}
					//neo_emu_done = 1;
					reset_frame_skip();
					break;
				}
				/*
				 switch (event.jbutton.button) {
				 case GP2X_R:
				 if (joy_button[0][GP2X_PUSH]) neo_emu_done = 1;
				 break;	// ESC
				 default:
				 break;
				 }*/
#endif
				break;
				case SDL_JOYBUTTONUP:
				joy_button[event.jbutton.which][event.jbutton.button] = 0;
				break;
#ifndef GP2X
				case SDL_KEYUP:
				key[event.key.keysym.sym] = 0;
				break;
				case SDL_KEYDOWN:
				scancode = event.key.keysym.sym;
				if (show_keysym) {
					sprintf(ksym_code, "%d", scancode);
					draw_message(ksym_code);
				}
				key[scancode] = 1;

				switch (scancode) {
					case SDLK_ESCAPE:
					neo_emu_done = 1;
					break; // ESC
					/*
					 case SDLK_TAB:
					 main_gngeo_gui();
					 break;
					 */
					case SDLK_F1:
					draw_message("Reset");
					//neogeo_init();
					cpu_68k_reset();
					break;
					case SDLK_F2:
					take_screenshot();
					draw_message("Screenshot saved");
					break;
					case SDLK_F3:
					draw_message("Test Switch ON");
					conf.test_switch = 1;
					break;
					case SDLK_F5:
					show_fps ^= GN_TRUE;
					break;
					case SDLK_F4:
					show_keysym = 1 - show_keysym;
					if (show_keysym)
					draw_message("Show keysym code : ON");
					else
					draw_message("Show keysym code : OFF");
					break;
					case SDLK_F6:
					slow_motion = 1 - slow_motion;
					if (slow_motion)
					draw_message("SlowMotion : ON");
					else {
						draw_message("SlowMotion : OFF");
						reset_frame_skip();
					}
					break;
					case SDLK_F7:
					//screen_set_effect("scanline");
					if (conf.debug) {
						dbg_step = 1;
					}
					break;
					case SDLK_F8:
					{
						int val;
						char *endptr;
						text_input("Save to slot [0-999]? ",16,227,input_buf,3);
						val=strtol(input_buf,&endptr,10);
						if (input_buf != endptr) {
							pending_save_state=val+1;
						}
					}
					break;
					case SDLK_F9:
					{
						int val;
						char *endptr;
						text_input("Load from slot [0-999]? ",16,227,input_buf,3);
						val=strtol(input_buf,&endptr,10);
						if (input_buf != endptr) {
							pending_load_state=val+1;
						}
					}
					break;
					case SDLK_F10:
					autoframeskip ^= GN_TRUE;
					if (autoframeskip) {
						reset_frame_skip();
						draw_message("AutoFrameSkip : ON");
					} else
					draw_message("AutoFrameSkip : OFF");
					break;
					case SDLK_F11:
					sleep_idle ^= GN_TRUE;
					if (sleep_idle)
					draw_message("Sleep idle : ON");
					else
					draw_message("Sleep idle : OFF");
					break;
					case SDLK_F12:
					screen_fullscreen();
					break;
				}
				break;
#endif
				case SDL_VIDEORESIZE:
				conf.res_x=event.resize.w;
				conf.res_y=event.resize.h;
				screen_resize(event.resize.w, event.resize.h);
				break;
				case SDL_QUIT:
				neo_emu_done = 1;
				break;
				default:
				break;
			}
		}

		/* update the internal representation of keyslot */
		update_p1_key();
		update_p2_key();
		update_start();
		update_coin();

#endif

		if (slow_motion)
			SDL_Delay(100);

#ifndef ENABLE_940T
		if (conf.sound) {
			PROFILER_START(PROF_Z80);

			for (i = 0; i < nb_interlace; i++) {
				cpu_z80_run(cpu_z80_timeslice_interlace);
				my_timer();
			}

			//cpu_z80_run(cpu_z80_timeslice);
			PROFILER_STOP(PROF_Z80);
		} /*
		 else
		 my_timer();*/
#endif
#ifdef ENABLE_940T
		if (conf.sound) {
			PROFILER_START(PROF_Z80);
			wait_busy_940(JOB940_RUN_Z80);
#if 0
			if (gp2x_timer) {
				gp2x_timer_prev=gp2x_timer;
				gp2x_timer=gp2x_memregl[0x0A00>>2];
				shared_ctl->sample_todo=(unsigned int)(((gp2x_timer-gp2x_timer_prev)*conf.sample_rate)/7372800.0);
			} else {
				gp2x_timer=gp2x_memregl[0x0A00>>2];
				shared_ctl->sample_todo=sample_len;
			}
#endif
			gp2x_add_job940(JOB940_RUN_Z80);
			PROFILER_STOP(PROF_Z80);
		}

#endif

		if (!conf.debug) {
			if (conf.raster) {
				current_line = 0;
				for (i = 0; i < 264; i++) {
					tm_cycle = cpu_68k_run(cpu_68k_timeslice_scanline
							- tm_cycle);
					if (update_scanline())
						cpu_68k_interrupt(2);
				}
				tm_cycle = cpu_68k_run(cpu_68k_timeslice_scanline - tm_cycle);
				//state_handling(pending_save_state, pending_load_state);

				update_screen();
				memory.watchdog++;
				if (memory.watchdog > 7) {
                    printf("WATCHDOG RESET\n");
					cpu_68k_reset();
                }
				cpu_68k_interrupt(1);
			} else {
				PROFILER_START(PROF_68K);
				tm_cycle = cpu_68k_run(cpu_68k_timeslice - tm_cycle);
				PROFILER_STOP(PROF_68K);
				a = neo_interrupt();

				/* state handling (we save/load before interrupt) */
				//state_handling(pending_save_state, pending_load_state);

				memory.watchdog++;

				if (memory.watchdog > 7) { /* Watchdog reset after ~0.13 == ~7.8 frames */
                    printf("WATCHDOG RESET %d\n",memory.watchdog);
					cpu_68k_reset();
                }

				if (a) {
					cpu_68k_interrupt(a);
				}
			}
		} else {
			/* we are in debug mode -> we are just here for event handling */
			neo_emu_done = 1;
		}

#ifdef ENABLE_PROFILER
		profiler_show_stat();
#endif
		PROFILER_START(PROF_ALL);
	}
	pause_audio(1);
#ifdef ENABLE_940T
	wait_busy_940(JOB940_RUN_Z80);
	wait_busy_940(JOB940_RUN_Z80_NMI);
	shared_ctl->z80_run = 0;
#endif

}

void cpu_68k_dpg_step(void) {
	static Uint32 nb_cycle;
	static Uint32 line_cycle;
	Uint32 cpu_68k_timeslice = 200000;
	Uint32 cpu_68k_timeslice_scanline = 200000 / (float) 262;
	Uint32 cycle;
	if (nb_cycle == 0) {
		main_loop(); /* update event etc. */
	}
	cycle = cpu_68k_run_step();
	add_bt(cpu_68k_getpc());
	line_cycle += cycle;
	nb_cycle += cycle;
	if (nb_cycle >= cpu_68k_timeslice) {
		nb_cycle = line_cycle = 0;
		if (conf.raster) {
			update_screen();
		} else {
			neo_interrupt();
		}
		//state_handling(pending_save_state, pending_load_state);
		cpu_68k_interrupt(1);
	} else {
		if (line_cycle >= cpu_68k_timeslice_scanline) {
			line_cycle = 0;
			if (conf.raster) {
				if (update_scanline())
					cpu_68k_interrupt(2);
			}
		}
	}
}

void debug_loop(void) {
	int a;
	do {
	  a = cpu_68k_debuger(cpu_68k_dpg_step, /*dump_hardware_reg*/NULL);
	} while (a != -1);
}
