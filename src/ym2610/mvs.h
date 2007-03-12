#ifndef _MVS_H_
#define _MVS_H_
/* compatibility layer */
#define s8  signed char
#define s16 signed short
#define s32 signed long

#define u8  unsigned char
#define u16 unsigned short
#define u32 unsigned long

#define ALIGN_DATA
#define INLINE static __inline__
#define SOUND_SAMPLES 512
#define Limit(val, max, min)                    \
{                                               \
        if (val > max) val = max;               \
        else if (val < min) val = min;          \
}

#endif
