#ifndef _INTERP_H_
#define _INTERP_H_

int interp_16_diff(Uint16 p1, Uint16 p2);

#define INTERP_16_MASK_1(v) ((v) & 0xf81F)
#define INTERP_16_MASK_2(v) ((v) & 0x7E0)
#define INTERP_16_UNMASK_1(v) ((v) & 0xf81F)
#define INTERP_16_UNMASK_2(v) ((v) & 0x7E0)

static inline Uint16 interp_16_521(Uint16 p1, Uint16 p2, Uint16 p3)
{
        return INTERP_16_UNMASK_1((INTERP_16_MASK_1(p1)*5 + INTERP_16_MASK_1(p2)*2 + INTERP_16_MASK_1(p3)*1) / 8)
                | INTERP_16_UNMASK_2((INTERP_16_MASK_2(p1)*5 + INTERP_16_MASK_2(p2)*2 + INTERP_16_MASK_2(p3)*1) / 8);
}

static inline Uint16 interp_16_332(Uint16 p1, Uint16 p2, Uint16 p3)
{
        return INTERP_16_UNMASK_1((INTERP_16_MASK_1(p1)*3 + INTERP_16_MASK_1(p2)*3 + INTERP_16_MASK_1(p3)*2) / 8)
                | INTERP_16_UNMASK_2((INTERP_16_MASK_2(p1)*3 + INTERP_16_MASK_2(p2)*3 + INTERP_16_MASK_2(p3)*2) / 8);
}

static inline Uint16 interp_16_611(Uint16 p1, Uint16 p2, Uint16 p3)
{
        return INTERP_16_UNMASK_1((INTERP_16_MASK_1(p1)*6 + INTERP_16_MASK_1(p2) + INTERP_16_MASK_1(p3)) / 8)
                | INTERP_16_UNMASK_2((INTERP_16_MASK_2(p1)*6 + INTERP_16_MASK_2(p2) + INTERP_16_MASK_2(p3)) / 8);
}

static inline Uint16 interp_16_71(Uint16 p1, Uint16 p2)
{
        return INTERP_16_UNMASK_1((INTERP_16_MASK_1(p1)*7 + INTERP_16_MASK_1(p2)) / 8)
                | INTERP_16_UNMASK_2((INTERP_16_MASK_2(p1)*7 + INTERP_16_MASK_2(p2)) / 8);
}

static inline Uint16 interp_16_211(Uint16 p1, Uint16 p2, Uint16 p3)
{
        return INTERP_16_UNMASK_1((INTERP_16_MASK_1(p1)*2 + INTERP_16_MASK_1(p2) + INTERP_16_MASK_1(p3)) / 4)
                | INTERP_16_UNMASK_2((INTERP_16_MASK_2(p1)*2 + INTERP_16_MASK_2(p2) + INTERP_16_MASK_2(p3)) / 4);
}

static inline Uint16 interp_16_772(Uint16 p1, Uint16 p2, Uint16 p3)
{
        return INTERP_16_UNMASK_1(((INTERP_16_MASK_1(p1) + INTERP_16_MASK_1(p2))*7 + INTERP_16_MASK_1(p3)*2) / 16)
                | INTERP_16_UNMASK_2(((INTERP_16_MASK_2(p1) + INTERP_16_MASK_2(p2))*7 + INTERP_16_MASK_2(p3)*2) / 16);
}

static inline Uint16 interp_16_11(Uint16 p1, Uint16 p2)
{
        return INTERP_16_UNMASK_1((INTERP_16_MASK_1(p1) + INTERP_16_MASK_1(p2)) / 2)
                | INTERP_16_UNMASK_2((INTERP_16_MASK_2(p1) + INTERP_16_MASK_2(p2)) / 2);
}
static inline Uint16 interp_16_31(Uint16 p1, Uint16 p2)
{
        return INTERP_16_UNMASK_1((INTERP_16_MASK_1(p1)*3 + INTERP_16_MASK_1(p2)) / 4)
                | INTERP_16_UNMASK_2((INTERP_16_MASK_2(p1)*3 + INTERP_16_MASK_2(p2)) / 4);
}

static inline Uint16 interp_16_1411(Uint16 p1, Uint16 p2, Uint16 p3)
{
        return INTERP_16_UNMASK_1((INTERP_16_MASK_1(p1)*14 + INTERP_16_MASK_1(p2) + INTERP_16_MASK_1(p3)) / 16)
                | INTERP_16_UNMASK_2((INTERP_16_MASK_2(p1)*14 + INTERP_16_MASK_2(p2) + INTERP_16_MASK_2(p3)) / 16);
}

static inline Uint16 interp_16_431(Uint16 p1, Uint16 p2, Uint16 p3)
{
        return INTERP_16_UNMASK_1((INTERP_16_MASK_1(p1)*4 + INTERP_16_MASK_1(p2)*3 + INTERP_16_MASK_1(p3)) / 8)
                | INTERP_16_UNMASK_2((INTERP_16_MASK_2(p1)*4 + INTERP_16_MASK_2(p2)*3 + INTERP_16_MASK_2(p3)) / 8);
}

static inline Uint16 interp_16_53(Uint16 p1, Uint16 p2)
{
        return INTERP_16_UNMASK_1((INTERP_16_MASK_1(p1)*5 + INTERP_16_MASK_1(p2)*3) / 8)
                | INTERP_16_UNMASK_2((INTERP_16_MASK_2(p1)*5 + INTERP_16_MASK_2(p2)*3) / 8);
}

static inline Uint16 interp_16_151(Uint16 p1, Uint16 p2)
{
        return INTERP_16_UNMASK_1((INTERP_16_MASK_1(p1)*15 + INTERP_16_MASK_1(p2)) / 16)
                | INTERP_16_UNMASK_2((INTERP_16_MASK_2(p1)*15 + INTERP_16_MASK_2(p2)) / 16);
}

static inline Uint16 interp_16_97(Uint16 p1, Uint16 p2)
{
        return INTERP_16_UNMASK_1((INTERP_16_MASK_1(p1)*9 + INTERP_16_MASK_1(p2)*7) / 16)
                | INTERP_16_UNMASK_2((INTERP_16_MASK_2(p1)*9 + INTERP_16_MASK_2(p2)*7) / 16);
}


#endif
