#ifndef _MVS_H_
#define _MVS_H_

#include "940shared.h"

/* compatibility layer */
#define s8  signed char
#define s16 signed short
#define s32 signed long

#define u8  unsigned char
#define u16 unsigned short
#define u32 unsigned long

#define Uint8 u8
#define Uint16 u16
#define Uint32 u32
#define Sint8 s8
#define Sint16 s16
#define Sint32 s32

#define ALIGN_DATA
#define INLINE static __inline__
#define SOUND_SAMPLES 512


#define Limit(val, max, min)                    \
{                                               \
        if (val > max) val = max;               \
        else if (val < min) val = min;          \
}

#endif
