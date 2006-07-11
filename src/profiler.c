
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <SDL.h>
#include "profiler.h"

#ifdef ENABLE_PROFILER

Uint64 counter[MAX_BLOCK];

#ifdef I386_ASM
static __inline__ Uint64 prof_get_tick(void)
{
    Uint64 x;
    __asm__ volatile (".byte 0x0f, 0x31":"=A" (x));
  /*__asm__ volatile ("RTDSC" : "=A" (x));*/
    
    return x;
}
#else
static __inline__ Uint64 prof_get_tick(void)
{
        return SDL_GetTicks();
}
#endif
void profiler_start(int block)
{
    counter[block] = prof_get_tick();
}

void profiler_stop(int block)
{
    counter[block] = prof_get_tick() - counter[block];
}

void profiler_show_stat(void)
{
    static int count;
    char buffer[256];

    Uint64 all = prof_get_tick() - counter[PROF_ALL];

    static Uint32 video, sound, blit, z80, m68k;

    video = (video + (counter[PROF_VIDEO] * 100) / (float) all) / 2.0;
    sound = (sound + (counter[PROF_SOUND] * 100) / (float) all) / 2.0;
    blit = (blit + (counter[PROF_SDLBLIT] * 100) / (float) all) / 2.0;
    z80 = (z80 + (counter[PROF_Z80] * 100) / (float) all) / 2.0;
    m68k = (m68k + (counter[PROF_68K] * 100) / (float) all) / 2.0;

    sprintf(buffer, "V: %02d A: %02d M: %02d Z:%02d", video, sound, z80,
	    m68k);
    if (count > 1) {
	draw_message(buffer);
	count = 0;
    } else
	count++;

}

#endif
