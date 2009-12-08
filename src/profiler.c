
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <SDL.h>
#include "profiler.h"

//#ifdef ENABLE_PROFILER



#ifdef I386_ASM
Uint64 counter[MAX_BLOCK];
static __inline__ Uint64 prof_get_tick(void)
{
    Uint64 x;
    __asm__ volatile (".byte 0x0f, 0x31":"=A" (x));
  /*__asm__ volatile ("RTDSC" : "=A" (x));*/
    
    return x;
}
#else
Uint32 counter[MAX_BLOCK];
static __inline__ Uint32 prof_get_tick(void)
{
        return SDL_GetTicks();
}
#endif
void profiler_start(int block)
{
    counter[block] = prof_get_tick();
    //printf("start %d %d\n",block,counter[block]);
}

void profiler_stop(int block)
{
    counter[block] = prof_get_tick() - counter[block];
    //printf("end   %d %d\n",block,counter[block]);
}

void profiler_show_stat(void)
{
    static int count;
    char buffer[256];

    Uint32 all = prof_get_tick() - counter[PROF_ALL];

    static Uint32 video=0, sound, blit, z80, m68k;
/*
    video = (video + (counter[PROF_VIDEO] * 100) / (float) all) / 2.0;
    sound = (sound + (counter[PROF_SOUND] * 100) / (float) all) / 2.0;
    blit = (blit + (counter[PROF_SDLBLIT] * 100) / (float) all) / 2.0;
    z80 = (z80 + (counter[PROF_Z80] * 100) / (float) all) / 2.0;
    m68k = (m68k + (counter[PROF_68K] * 100) / (float) all) / 2.0;
*/
    if (counter[PROF_VIDEO]>video) video=counter[PROF_VIDEO];
    
    //sprintf(buffer, "V: %02d A: %02d M: %02d Z:%02d", video, sound, z80, m68k);
    sprintf(buffer, "V:%d (%d) A:%d M:%d Z:%d ALL:%d",
	    counter[PROF_VIDEO], video, counter[PROF_SOUND], 
	    counter[PROF_68K],counter[PROF_Z80],all);


    //if (count > 1) {
	draw_message(buffer);
/*
	count = 0;
    } else
	count++;
*/

}

//#endif
