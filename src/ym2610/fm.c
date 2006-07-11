/*
**
** File: fm.c -- software implementation of FM sound generator
**
** Copyright (C) 1998 Tatsuyuki Satoh , MultiArcadeMachineEmurator development
**
** Version 0.37
**
*/

/*
**** change log. (hiro-shi) ****
** 08-12-98:
** rename ADPCMA -> ADPCMB, ADPCMB -> ADPCMA
** move ROM limit check.(CALC_CH? -> 2610Write1/2)
** test program (ADPCMB_TEST)
** move ADPCM A/B end check.
** ADPCMB repeat flag(no check)
** change ADPCM volume rate (8->16) (32->48).
**
** 09-12-98:
** change ADPCM volume. (8->16, 48->64)
** replace ym2610 ch0/3 (YM-2610B)
** init cur_chip (restart bug fix)
** change ADPCM_SHIFT (10->8) missing bank change 0x4000-0xffff.
** add ADPCM_SHIFT_MASK
** change ADPCMA_DECODE_MIN/MAX.
*/

/*
    no check:
		YM2608 rhythm sound
		OPN SSG type envelope (SEG)
		YM2151 CSM speech mode

	no support:
		status BUSY flag (everytime not busy)
		YM2608 status mask (register :0x110)
		YM2608 RYTHM sound
		YM2608 PCM memory data access , DELTA-T-ADPCM with PCM port
		YM2151 CSM speech mode with internal timer

	preliminary :
		key scale level rate (?)
		attack rate time rate , curve
		decay  rate time rate , curve
		self-feedback algorythm
		YM2610 ADPCM-A mixing level , decode algorythm
		YM2151 noise mode (CH7.OP4)
		LFO contoller (YM2612/YM2610/YM2608/YM2151)

	note:
                        OPN                           OPM
		fnum          fM * 2^20 / (fM/(12*n))
		TimerOverA    ( 12*n)*(1024-NA)/fM        64*(1024-Na)/fM
		TimerOverB    (192*n)*(256-NB)/fM       1024*(256-Nb)/fM
		output bits   10bit<<3bit               16bit * 2ch (YM3012=10bit<<3bit)
		sampling rate fFM / (12*priscaler)      fM / 64
		lfo freq                                ( fM*2^(LFRQ/16) ) / (4295*10^6)
*/

/************************************************************************/
/*    comment of hiro-shi(Hiromitsu Shioya)                             */
/*    YM2610(B) = (OPN-B                                                */
/*    YM2610  : PSG:3ch FM:4ch ADPCM(18.5KHz):6ch DeltaT ADPCM:1ch      */
/*    YM2610B : PSG:3ch FM:6ch ADPCM(18.5KHz):6ch DeltaT ADPCM:1ch      */
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include "../sound.h"
#include "ay8910.h"
#include "fm.h"
#include "2610intf.h"
#include "../state.h"

#ifndef PI
#define PI 3.14159265358979323846
#endif



/***** shared function building option ****/
#define BUILD_OPN (BUILD_YM2610)
#define BUILD_OPNB (BUILD_YM2610)
#define BUILD_FM_ADPCMA (BUILD_OPNB)
#define BUILD_FM_ADPCMB (BUILD_OPNB)
#define BUILD_STEREO (BUILD_YM2610)
#define BUILD_LFO (BUILD_YM2610)

#if BUILD_FM_ADPCMB
/* include external DELTA-T ADPCM unit */
#include "ymdeltat.h"		/* DELTA-T ADPCM UNIT */
#define DELTAT_MIXING_LEVEL (4)	/* DELTA-T ADPCM MIXING LEVEL */
#endif

/* -------------------- sound quality define selection --------------------- */
/* sinwave entries */
/* used static memory = SIN_ENT * 4 (byte) */
#define SIN_ENT 2048

/* lower bits of envelope counter */
#define ENV_BITS 16

/* envelope output entries */
#define EG_ENT   4096
#define EG_STEP (96.0/EG_ENT)	/* OPL == 0.1875 dB */

#if FM_LFO_SUPPORT
/* LFO table entries */
#define LFO_ENT 512
#define LFO_SHIFT (32-9)
#define LFO_RATE 0x10000
#endif

/* -------------------- preliminary define section --------------------- */
/* attack/decay rate time rate */
#define OPM_ARRATE     399128
#define OPM_DRRATE    5514396
/* It is not checked , because I haven't YM2203 rate */
#define OPN_ARRATE  OPM_ARRATE
#define OPN_DRRATE  OPM_DRRATE

/* PG output cut off level : 78dB(14bit)? */
#define PG_CUT_OFF ((int)(78.0/EG_STEP))
/* EG output cut off level : 68dB? */
#define EG_CUT_OFF ((int)(68.0/EG_STEP))

#define FREQ_BITS 24		/* frequency turn          */

/* PG counter is 21bits @oct.7 */
#define FREQ_RATE   (1<<(FREQ_BITS-21))
#define TL_BITS    (FREQ_BITS+2)
/* OPbit = 14(13+sign) : TL_BITS+1(sign) / output = 16bit */
#define TL_SHIFT (TL_BITS+1-(14-16))

/* output final shift */
#define FM_OUTSB  (TL_SHIFT-FM_OUTPUT_BIT)
#define FM_MAXOUT ((1<<(TL_SHIFT-1))-1)
#define FM_MINOUT (-(1<<(TL_SHIFT-1)))

/* -------------------- local defines , macros --------------------- */

/* envelope counter position */
#define EG_AST   0		/* start of Attack phase */
#define EG_AED   (EG_ENT<<ENV_BITS)	/* end   of Attack phase */
#define EG_DST   EG_AED		/* start of Decay/Sustain/Release phase */
#define EG_DED   (EG_DST+(EG_ENT<<ENV_BITS)-1)	/* end   of Decay/Sustain/Release phase */
#define EG_OFF   EG_DED		/* off */
#if FM_SEG_SUPPORT
#define EG_UST   ((2*EG_ENT)<<ENV_BITS)	/* start of SEG UPSISE */
#define EG_UED   ((3*EG_ENT)<<ENV_BITS)	/* end of SEG UPSISE */
#endif

/* register number to channel number , slot offset */
#define OPN_CHAN(N) (N&3)
#define OPN_SLOT(N) ((N>>2)&3)
#define OPM_CHAN(N) (N&7)
#define OPM_SLOT(N) ((N>>3)&3)
/* slot number */
#define SLOT1 0
#define SLOT2 2
#define SLOT3 1
#define SLOT4 3

/* bit0 = Right enable , bit1 = Left enable */
#define OUTD_RIGHT  1
#define OUTD_LEFT   2
#define OUTD_CENTER 3

/* FM timer model */
#define FM_TIMER_SINGLE (0)
#define FM_TIMER_INTERVAL (1)


/* -------------------- tables --------------------- */

/* sustain lebel table (3db per step) */
/* 0 - 15: 0, 3, 6, 9,12,15,18,21,24,27,30,33,36,39,42,93 (dB)*/

#define SC(db) (int)((db*((3/EG_STEP)*(1<<ENV_BITS)))+EG_DST)
static const int SL_TABLE[16] = {
    SC(0), SC(1), SC(2), SC(3), SC(4), SC(5), SC(6), SC(7),
    SC(8), SC(9), SC(10), SC(11), SC(12), SC(13), SC(14), SC(31)
};

#undef SC

/* size of TL_TABLE = sinwave(max cut_off) + cut_off(tl + ksr + envelope + ams) */
#define TL_MAX (PG_CUT_OFF+EG_CUT_OFF+1)

/* TotalLevel : 48 24 12  6  3 1.5 0.75 (dB) */
/* TL_TABLE[ 0      to TL_MAX          ] : plus  section */
/* TL_TABLE[ TL_MAX to TL_MAX+TL_MAX-1 ] : minus section */
static Sint32 *TL_TABLE;

/* pointers to TL_TABLE with sinwave output offset */
static Sint32 *SIN_TABLE[SIN_ENT];

/* envelope output curve table */
#if FM_SEG_SUPPORT
/* attack + decay + SSG upside + OFF */
static Sint32 ENV_CURVE[3 * EG_ENT + 1];
#else
/* attack + decay + OFF */
static Sint32 ENV_CURVE[2 * EG_ENT + 1];
#endif
/* envelope counter conversion table when change Decay to Attack phase */
static int DRAR_TABLE[EG_ENT];

#define OPM_DTTABLE OPN_DTTABLE
static Uint8 OPN_DTTABLE[4 * 32] = {
/* this table is YM2151 and YM2612 data */
/* FD=0 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* FD=1 */
    0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2,
    2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 8, 8, 8, 8,
/* FD=2 */
    1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5,
    5, 6, 6, 7, 8, 8, 9, 10, 11, 12, 13, 14, 16, 16, 16, 16,
/* FD=3 */
    2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7,
    8, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 20, 22, 22, 22, 22
};

/* multiple table */
#define ML(n) (int)(n*2)
static const int MUL_TABLE[4 * 16] = {
/* 1/2, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15 */
    ML(0.50), ML(1.00), ML(2.00), ML(3.00), ML(4.00), ML(5.00), ML(6.00),
	ML(7.00),
    ML(8.00), ML(9.00), ML(10.00), ML(11.00), ML(12.00), ML(13.00),
	ML(14.00), ML(15.00),
/* DT2=1 *SQL(2)   */
    ML(0.71), ML(1.41), ML(2.82), ML(4.24), ML(5.65), ML(7.07), ML(8.46),
	ML(9.89),
    ML(11.30), ML(12.72), ML(14.10), ML(15.55), ML(16.96), ML(18.37),
	ML(19.78), ML(21.20),
/* DT2=2 *SQL(2.5) */
    ML(0.78), ML(1.57), ML(3.14), ML(4.71), ML(6.28), ML(7.85), ML(9.42),
	ML(10.99),
    ML(12.56), ML(14.13), ML(15.70), ML(17.27), ML(18.84), ML(20.41),
	ML(21.98), ML(23.55),
/* DT2=3 *SQL(3)   */
    ML(0.87), ML(1.73), ML(3.46), ML(5.19), ML(6.92), ML(8.65), ML(10.38),
	ML(12.11),
    ML(13.84), ML(15.57), ML(17.30), ML(19.03), ML(20.76), ML(22.49),
	ML(24.22), ML(25.95)
};
#undef ML

#if FM_LFO_SUPPORT

#define PMS_RATE 0x400
/* LFO runtime work */
static Uint32 lfo_amd;
static Sint32 lfo_pmd;
#endif

/* Dummy table of Attack / Decay rate ( use when rate == 0 ) */
static const Sint32 RATE_0[32] =
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* -------------------- state --------------------- */

/* some globals */
#define TYPE_SSG    0x01	/* SSG support          */
#define TYPE_OPN    0x02	/* OPN device           */
#define TYPE_LFOPAN 0x04	/* OPN type LFO and PAN */
#define TYPE_6CH    0x08	/* FM 6CH / 3CH         */
#define TYPE_DAC    0x10	/* YM2612's DAC device  */
#define TYPE_ADPCM  0x20	/* two ADPCM unit       */

#define TYPE_YM2203 (TYPE_SSG)
#define TYPE_YM2608 (TYPE_SSG |TYPE_LFOPAN |TYPE_6CH |TYPE_ADPCM)
#define TYPE_YM2610 (TYPE_SSG |TYPE_LFOPAN |TYPE_6CH |TYPE_ADPCM)
#define TYPE_YM2612 (TYPE_6CH |TYPE_LFOPAN |TYPE_DAC)

/* current chip state */
static void *cur_chip = 0;	/* pointer of current chip struct */
static FM_ST *State;		/* basic status */
static FM_CH *cch[8];		/* pointer of FM channels */
#if FM_LFO_SUPPORT
static Uint32 LFOCnt, LFOIncr;	/* LFO PhaseGenerator */
#endif

/* runtime work */
static Sint32 out_ch[4];		/* channel output NONE,LEFT,RIGHT or CENTER */
static Sint32 pg_in1, pg_in2, pg_in3, pg_in4;	/* PG input of SLOTs */

#if 0
static void print_struct(FILE *f,YM2610 *F2610,Uint8 *data) {
    int i,j;
    if (data==F2610) fprintf(f," - YM2610 - ");
    if (data==&F2610->REGS) fprintf(f," - REGS - ");
    if (data==&F2610->OPN) fprintf(f," - OPN - ");
    for(i=0;i<6;i++) {
	if (data==&F2610->CH[i]) fprintf(f," - CH[%d] - ",i);
	for(j=0;j<4;j++) {
	    if (data==&F2610->CH[i].SLOT[j]) fprintf(f," - SLOT[%d] - ",j);
	    if (data==&F2610->CH[i].SLOT[j].DT) fprintf(f," - SL.DT - ");
	    if (data==&F2610->CH[i].SLOT[j].DT2) fprintf(f," - SL.DT2 - ");
	    if (data==&F2610->CH[i].SLOT[j].TL) fprintf(f," - SL.TL - ");
	    if (data==&F2610->CH[i].SLOT[j].KSR) fprintf(f," - SL.KSR - ");
	    if (data==&F2610->CH[i].SLOT[j].AR) fprintf(f," - SL.AR - ");
	    if (data==&F2610->CH[i].SLOT[j].DR) fprintf(f," - SL.DR - ");
	    if (data==&F2610->CH[i].SLOT[j].SR) fprintf(f," - SL.SR - ");
	    if (data==&F2610->CH[i].SLOT[j].RR) fprintf(f," - SL.RR - ");
	    if (data==&F2610->CH[i].SLOT[j].SL) fprintf(f," - SL.SL - ");
	    if (data==&F2610->CH[i].SLOT[j].SEG) fprintf(f," - SL.SEG - ");
	    if (data==&F2610->CH[i].SLOT[j].ksr) fprintf(f," - SL.KSR2 - ");
	    if (data==&F2610->CH[i].SLOT[j].mul) fprintf(f," - SL.MUL - ");
	    if (data==&F2610->CH[i].SLOT[j].Cnt) fprintf(f," - SL.CNT - ");
	    if (data==&F2610->CH[i].SLOT[j].Incr) fprintf(f," - SL.INCR - ");
	    if (data==&F2610->CH[i].SLOT[j].state) fprintf(f," - SL.STATE - ");
	    if (data==&F2610->CH[i].SLOT[j].eg_next) fprintf(f," - SL.EG_NEXT - ");
	    if (data==&F2610->CH[i].SLOT[j].evc) fprintf(f," - SL.evc - ");
	    if (data==&F2610->CH[i].SLOT[j].eve) fprintf(f," - SL.eve - ");
	    if (data==&F2610->CH[i].SLOT[j].evs) fprintf(f," - SL.evs - ");
	    if (data==&F2610->CH[i].SLOT[j].evsa) fprintf(f," - SL.evsa - ");
	    if (data==&F2610->CH[i].SLOT[j].evsd) fprintf(f," - SL.evsd - ");
	    if (data==&F2610->CH[i].SLOT[j].evss) fprintf(f," - SL.evss - ");
	    if (data==&F2610->CH[i].SLOT[j].evsr) fprintf(f," - SL.evsr - ");
	    if (data==&F2610->CH[i].SLOT[j].TLL) fprintf(f," - SL.TLL - ");
	    if (data==&F2610->CH[i].SLOT[j].amon) fprintf(f," - SL.AMON - ");
	    if (data==&F2610->CH[i].SLOT[j].ams) fprintf(f," - SL.AMS - ");
	    
	    
 
	}
	if (data==&F2610->CH[i].PAN) fprintf(f," - PAN - ");
	if (data==&F2610->CH[i].ALGO) fprintf(f," - ALGO - ");
	if (data==&F2610->CH[i].FB) fprintf(f," - FB - ");
	if (data==&F2610->CH[i].op1_out) fprintf(f," - OP1OUT - ");
	if (data==&F2610->CH[i].connect1) fprintf(f," - CONNECT1 - ");
	if (data==&F2610->CH[i].connect2) fprintf(f," - CONNECT2 - ");
	if (data==&F2610->CH[i].connect3) fprintf(f," - CONNECT3 - ");
	if (data==&F2610->CH[i].connect4) fprintf(f," - CONNECT4 - ");
	if (data==&F2610->CH[i].pms) fprintf(f," - PMS - ");
	if (data==&F2610->CH[i].ams) fprintf(f," - AMS - ");
	if (data==&F2610->CH[i].fc) fprintf(f," - FC - ");
	if (data==&F2610->CH[i].fn_h) fprintf(f," - FN_H - ");
	if (data==&F2610->CH[i].kcode) fprintf(f," - KCODE - ");
    }
    if (data==&F2610->address1) fprintf(f," - ADD1 - ");
    if (data==&F2610->pcmbuf) fprintf(f," - PCMBUF - ");
    if (data==&F2610->adpcmTL) fprintf(f," - ADPCMTL - ");
    for(i=0;i<6;i++)
	if (data==&F2610->adpcm[i]) fprintf(f," - ADPCM[%d] - ",i);
    if (data==&F2610->adpcmreg) fprintf(f," - ADPCMREG - ");
    if (data==&F2610->adpcm_arrivedEndAddress) fprintf(f," - ADPCM_END - ");
    if (data==&F2610->deltaT) fprintf(f," - DELTAT - ");

}

static void dump_mem(FILE *f,YM2610 *F2610,Uint8 *data,int size) {
    int i;
    fprintf(f,"%04x : ",0);
    for(i=0;i<size;i++) {
	print_struct(f,F2610,&data[i]);
	fprintf(f,"%02x ",data[i]);
	if (!((i+1)%16)) fprintf(f,"\n%04x : ",i/16);
    }
}

void dump_struct(YM2610 *F2610,int a) {
    FILE *f;
    if (a) 
	f=fopen("log_save","w");
    else
	f=fopen("log_load","w");
    fprintf(f,"YM2610:\n");
    dump_mem(f,F2610,(Uint8*)F2610,sizeof(YM2610));
    fclose(f);
}

#endif

/* -------------------- log output  -------------------- */
/* log output level */
#define LOG_ERR  3		/* ERROR       */
#define LOG_WAR  2		/* WARNING     */
#define LOG_INF  1		/* INFORMATION */
#define LOG_LEVEL LOG_INF

//#ifndef __RAINE__
//#define LOG(n,x) if( (n)>=LOG_LEVEL ) logerror x
//#endif

/* ----- limitter ----- */
#define Limit(val, max,min) { \
	if ( val > max )      val = max; \
	else if ( val < min ) val = min; \
}

/* ----- buffering one of data(STEREO chip) ----- */
#if FM_STEREO_MIX
/* stereo mixing */
#define FM_BUFFERING_STEREO \
{								\
	/* get left & right output with clipping */	        \
	out_ch[OUTD_LEFT]  += out_ch[OUTD_CENTER];		\
	Limit( out_ch[OUTD_LEFT] , FM_MAXOUT, FM_MINOUT );	\
	out_ch[OUTD_RIGHT] += out_ch[OUTD_CENTER];		\
	Limit( out_ch[OUTD_RIGHT], FM_MAXOUT, FM_MINOUT );	\
	/* buffering */						\
	*bufL++ = out_ch[OUTD_LEFT] >>FM_OUTSB;			\
	*bufL++ = out_ch[OUTD_RIGHT]>>FM_OUTSB;			\
}
#else
/* stereo separate */
#define FM_BUFFERING_STEREO \
{								\
	/* get left & right output with clipping */		\
	out_ch[OUTD_LEFT]  += out_ch[OUTD_CENTER];		\
	Limit( out_ch[OUTD_LEFT] , FM_MAXOUT, FM_MINOUT );	\
	out_ch[OUTD_RIGHT] += out_ch[OUTD_CENTER];		\
	Limit( out_ch[OUTD_RIGHT], FM_MAXOUT, FM_MINOUT );	\
	/* buffering */						\
	bufL[i] = out_ch[OUTD_LEFT] >>FM_OUTSB;			\
	bufR[i] = out_ch[OUTD_RIGHT]>>FM_OUTSB;			\
}
#endif

#if FM_INTERNAL_TIMER
/* ----- internal timer mode , update timer */
/* ---------- calcrate timer A ---------- */
#define INTERNAL_TIMER_A(ST,CSM_CH)				\
{								\
	if( ST->TAC &&  (ST->Timer_Handler==0) )		\
		if( (ST->TAC -= (int)(ST->freqbase*4096)) <= 0 )\
		{						\
			TimerAOver( ST );			\
			/* CSM mode total level latch and auto key on */	\
			if( ST->mode & 0x80 )			\
				CSMKeyControll( CSM_CH );	\
		}						\
}
/* ---------- calcrate timer B ---------- */
#define INTERNAL_TIMER_B(ST,step)					\
{									\
	if( ST->TBC && (ST->Timer_Handler==0) )				\
		if( (ST->TBC -= (int)(ST->freqbase*4096*step)) <= 0 )	\
			TimerBOver( ST );				\
}
#else				/* FM_INTERNAL_TIMER */
/* external timer mode */
#define INTERNAL_TIMER_A(ST,CSM_CH)
#define INTERNAL_TIMER_B(ST,step)
#endif				/* FM_INTERNAL_TIMER */

/* --------------------- subroutines  --------------------- */
/* status set and IRQ handling */
inline void FM_STATUS_SET(FM_ST * ST, int flag)
{

    /* set status flag */
    ST->status |= flag;
    if (!(ST->irq) && (ST->status & ST->irqmask)) {
	// printf("FM_STATUS_SET\n");
	ST->irq = 1;
	/* callback user interrupt handler (IRQ is OFF to ON) */
	if (ST->IRQ_Handler)
	    (ST->IRQ_Handler) (ST->index, 1);
    }
}

/* status reset and IRQ handling */
inline void FM_STATUS_RESET(FM_ST * ST, int flag)
{

    /* reset status flag */
    ST->status &= ~flag;
    //printf("%d %d %d\n",ST->irq,ST->status,ST->irqmask);
    if ((ST->irq) && !(ST->status & ST->irqmask)) {
	//printf("FM_STATUS_RESET\n");
	ST->irq = 0;
	/* callback user interrupt handler (IRQ is ON to OFF) */
	if (ST->IRQ_Handler)
	    (ST->IRQ_Handler) (ST->index, 0);
    }
}

/* IRQ mask set */
inline void FM_IRQMASK_SET(FM_ST * ST, int flag)
{
    //printf("irqmask set\n");
    ST->irqmask = flag;
    /* IRQ handling check */
    FM_STATUS_SET(ST, 0);
    FM_STATUS_RESET(ST, 0);
}

/* ---------- event hander of Phase Generator ---------- */

/* Release end -> stop counter */
static void FM_EG_Release(FM_SLOT * SLOT)
{
    SLOT->evc = EG_OFF;
    SLOT->eve = EG_OFF + 1;
    SLOT->evs = 0;
}

/* SUSTAIN end -> stop counter */
static void FM_EG_SR(FM_SLOT * SLOT)
{
    SLOT->evs = 0;
    SLOT->evc = EG_OFF;
    SLOT->eve = EG_OFF + 1;
}

/* Decay end -> Sustain */
static void FM_EG_DR(FM_SLOT * SLOT)
{
    SLOT->eg_next = FM_EG_SR;
    SLOT->evc = SLOT->SL;
    SLOT->eve = EG_DED;
    SLOT->evs = SLOT->evss;
}

/* Attack end -> Decay */
static void FM_EG_AR(FM_SLOT * SLOT)
{
    /* next DR */
    SLOT->eg_next = FM_EG_DR;
    SLOT->evc = EG_DST;
    SLOT->eve = SLOT->SL;
    SLOT->evs = SLOT->evsd;
}

#if FM_SEG_SUPPORT
static void FM_EG_SSG_SR(FM_SLOT * SLOT);

/* SEG down side end  */
static void FM_EG_SSG_DR(FM_SLOT * SLOT)
{
    if (SLOT->SEG & 2) {
	/* reverce */
	SLOT->eg_next = FM_EG_SSG_SR;
	SLOT->evc = SLOT->SL + (EG_UST - EG_DST);
	SLOT->eve = EG_UED;
	SLOT->evs = SLOT->evss;
    } else {
	/* again */
	SLOT->evc = EG_DST;
    }
    /* hold */
    if (SLOT->SEG & 1)
	SLOT->evs = 0;
}

/* SEG upside side end */
static void FM_EG_SSG_SR(FM_SLOT * SLOT)
{
    if (SLOT->SEG & 2) {
	/* reverce  */
	SLOT->eg_next = FM_EG_SSG_DR;
	SLOT->evc = EG_DST;
	SLOT->eve = EG_DED;
	SLOT->evs = SLOT->evsd;
    } else {
	/* again */
	SLOT->evc = SLOT->SL + (EG_UST - EG_DST);
    }
    /* hold check */
    if (SLOT->SEG & 1)
	SLOT->evs = 0;
}

/* SEG Attack end */
static void FM_EG_SSG_AR(FM_SLOT * SLOT)
{
    if (SLOT->SEG & 4) {	/* start direction */
	/* next SSG-SR (upside start ) */
	SLOT->eg_next = FM_EG_SSG_SR;
	SLOT->evc = SLOT->SL + (EG_UST - EG_DST);
	SLOT->eve = EG_UED;
	SLOT->evs = SLOT->evss;
    } else {
	/* next SSG-DR (downside start ) */
	SLOT->eg_next = FM_EG_SSG_DR;
	SLOT->evc = EG_DST;
	SLOT->eve = EG_DED;
	SLOT->evs = SLOT->evsd;
    }
}
#endif				/* FM_SEG_SUPPORT */

/* ----- key on of SLOT ----- */
#define FM_KEY_IS(SLOT) ((SLOT)->eg_next!=FM_EG_Release)

inline void FM_KEYON(FM_CH * CH, int s)
{
    FM_SLOT *SLOT = &CH->SLOT[s];

    if (!FM_KEY_IS(SLOT)) {
	/* restart Phage Generator */
	SLOT->Cnt = 0;
	/* phase -> Attack */
#if FM_SEG_SUPPORT
	if (SLOT->SEG & 8)
	    SLOT->eg_next = FM_EG_SSG_AR;
	else
#endif
	    SLOT->eg_next = FM_EG_AR;
	SLOT->evs = SLOT->evsa;
#if 0
	/* convert decay count to attack count */
	/* --- This caused the problem by credit sound of paper boy. --- */
	SLOT->evc = EG_AST + DRAR_TABLE[ENV_CURVE[SLOT->evc >> ENV_BITS]];	/* + SLOT->evs; */
#else
	/* reset attack counter */
	SLOT->evc = EG_AST;
#endif
	SLOT->eve = EG_AED;
    }
}

/* ----- key off of SLOT ----- */
inline void FM_KEYOFF(FM_CH * CH, int s)
{

    FM_SLOT *SLOT = &CH->SLOT[s];

    if (FM_KEY_IS(SLOT)) {
	/* if Attack phase then adjust envelope counter */
	if (SLOT->evc < EG_DST)
	    SLOT->evc =
		(ENV_CURVE[SLOT->evc >> ENV_BITS] << ENV_BITS) + EG_DST;
	/* phase -> Release */
	SLOT->eg_next = FM_EG_Release;
	SLOT->eve = EG_DED;
	SLOT->evs = SLOT->evsr;
    }
}

/* setup Algorythm and PAN connection */
static void setup_connection(FM_CH * CH)
{
    Sint32 *carrier = &out_ch[CH->PAN];	/* NONE,LEFT,RIGHT or CENTER */

    switch (CH->ALGO) {
    case 0:
	/*  PG---S1---S2---S3---S4---OUT */
	CH->connect1 = &pg_in2;
	CH->connect2 = &pg_in3;
	CH->connect3 = &pg_in4;
	break;
    case 1:
	/*  PG---S1-+-S3---S4---OUT */
	/*  PG---S2-+               */
	CH->connect1 = &pg_in3;
	CH->connect2 = &pg_in3;
	CH->connect3 = &pg_in4;
	break;
    case 2:
	/* PG---S1------+-S4---OUT */
	/* PG---S2---S3-+          */
	CH->connect1 = &pg_in4;
	CH->connect2 = &pg_in3;
	CH->connect3 = &pg_in4;
	break;
    case 3:
	/* PG---S1---S2-+-S4---OUT */
	/* PG---S3------+          */
	CH->connect1 = &pg_in2;
	CH->connect2 = &pg_in4;
	CH->connect3 = &pg_in4;
	break;
    case 4:
	/* PG---S1---S2-+--OUT */
	/* PG---S3---S4-+      */
	CH->connect1 = &pg_in2;
	CH->connect2 = carrier;
	CH->connect3 = &pg_in4;
	break;
    case 5:
	/*         +-S2-+     */
	/* PG---S1-+-S3-+-OUT */
	/*         +-S4-+     */
	CH->connect1 = 0;	/* special case */
	CH->connect2 = carrier;
	CH->connect3 = carrier;
	break;
    case 6:
	/* PG---S1---S2-+     */
	/* PG--------S3-+-OUT */
	/* PG--------S4-+     */
	CH->connect1 = &pg_in2;
	CH->connect2 = carrier;
	CH->connect3 = carrier;
	break;
    case 7:
	/* PG---S1-+     */
	/* PG---S2-+-OUT */
	/* PG---S3-+     */
	/* PG---S4-+     */
	CH->connect1 = carrier;
	CH->connect2 = carrier;
	CH->connect3 = carrier;
    }
    CH->connect4 = carrier;
}

/* set detune & multiple */
inline void set_det_mul(FM_ST * ST, FM_CH * CH, FM_SLOT * SLOT, int v)
{
    SLOT->mul = MUL_TABLE[v & 0x0f];
    SLOT->DT = ST->DT_TABLE[(v >> 4) & 7];
    CH->SLOT[SLOT1].Incr = -1;
}

/* set total level */
inline void set_tl(FM_CH * CH, FM_SLOT * SLOT, int v, int csmflag)
{
    v &= 0x7f;
    v = (v << 7) | v;		/* 7bit -> 14bit */
    SLOT->TL = (v * EG_ENT) >> 14;
    /* if it is not a CSM channel , latch the total level */
    if (!csmflag)
	SLOT->TLL = SLOT->TL;
}

/* set attack rate & key scale  */
inline void set_ar_ksr(FM_CH * CH, FM_SLOT * SLOT, int v, Sint32 * ar_table)
{
    SLOT->KSR = 3 - (v >> 6);
    SLOT->AR = (v &= 0x1f) ? &ar_table[v << 1] : RATE_0;
    SLOT->evsa = SLOT->AR[SLOT->ksr];
    if (SLOT->eg_next == FM_EG_AR)
	SLOT->evs = SLOT->evsa;
    CH->SLOT[SLOT1].Incr = -1;
}

/* set decay rate */
inline void set_dr(FM_SLOT * SLOT, int v, Sint32 * dr_table)
{
    SLOT->DR = (v &= 0x1f) ? &dr_table[v << 1] : RATE_0;
    SLOT->evsd = SLOT->DR[SLOT->ksr];
    if (SLOT->eg_next == FM_EG_DR)
	SLOT->evs = SLOT->evsd;
}

/* set sustain rate */
inline void set_sr(FM_SLOT * SLOT, int v, Sint32 * dr_table)
{
    SLOT->SR = (v &= 0x1f) ? &dr_table[v << 1] : RATE_0;
    SLOT->evss = SLOT->SR[SLOT->ksr];
    if (SLOT->eg_next == FM_EG_SR)
	SLOT->evs = SLOT->evss;
}

/* set release rate */
inline void set_sl_rr(FM_SLOT * SLOT, int v, Sint32 * dr_table)
{
    SLOT->SL = SL_TABLE[(v >> 4)];
    SLOT->RR = &dr_table[((v & 0x0f) << 2) | 2];
    SLOT->evsr = SLOT->RR[SLOT->ksr];
    if (SLOT->eg_next == FM_EG_Release)
	SLOT->evs = SLOT->evsr;
}

#if 1
/* operator output calcrator */
#define OP_OUT(PG,EG)   SIN_TABLE[(PG/(0x1000000/SIN_ENT))&(SIN_ENT-1)][EG]
#define OP_OUTN(PG,EG)  NOISE_TABLE[(PG/(0x1000000/SIN_ENT))&(SIN_ENT-1)][EG]

#else
#define OP_OUT(PG,EG)   0 //SIN_TABLE[(PG/(0x1000000/SIN_ENT))&(SIN_ENT-1)][EG]
#endif

/* eg calcration */
#if FM_LFO_SUPPORT
#define FM_CALC_EG(OUT,SLOT)				\
{							\
	if( (SLOT.evc += SLOT.evs) >= SLOT.eve) 	\
		SLOT.eg_next(&(SLOT));			\
	OUT = SLOT.TLL+ENV_CURVE[SLOT.evc>>ENV_BITS];	\
	if(SLOT.ams)					\
		OUT += (SLOT.ams*lfo_amd/LFO_RATE);	\
}
#else
#define FM_CALC_EG(OUT,SLOT)				\
{							\
	if( (SLOT.evc += SLOT.evs) >= SLOT.eve) 	\
		SLOT.eg_next(&(SLOT));			\
	OUT = SLOT.TLL+ENV_CURVE[SLOT.evc>>ENV_BITS];	\
}
#endif

/* ---------- calcrate one of channel ---------- */
static inline void FM_CALC_CH(FM_CH * CH)
{
    Uint32 eg_out1, eg_out2, eg_out3, eg_out4;	/*envelope output */

    /* Phase Generator */
#if FM_LFO_SUPPORT
    Sint32 pms = lfo_pmd * CH->pms / LFO_RATE;
    if (pms) {
	pg_in1 = (CH->SLOT[SLOT1].Cnt +=
		  CH->SLOT[SLOT1].Incr +
		  (Sint32) (pms * CH->SLOT[SLOT1].Incr) / PMS_RATE);
	pg_in2 = (CH->SLOT[SLOT2].Cnt +=
		  CH->SLOT[SLOT2].Incr +
		  (Sint32) (pms * CH->SLOT[SLOT2].Incr) / PMS_RATE);
	pg_in3 = (CH->SLOT[SLOT3].Cnt +=
		  CH->SLOT[SLOT3].Incr +
		  (Sint32) (pms * CH->SLOT[SLOT3].Incr) / PMS_RATE);
	pg_in4 = (CH->SLOT[SLOT4].Cnt +=
		  CH->SLOT[SLOT4].Incr +
		  (Sint32) (pms * CH->SLOT[SLOT4].Incr) / PMS_RATE);
    } else
#endif
    {
	pg_in1 = (CH->SLOT[SLOT1].Cnt += CH->SLOT[SLOT1].Incr);
	pg_in2 = (CH->SLOT[SLOT2].Cnt += CH->SLOT[SLOT2].Incr);
	pg_in3 = (CH->SLOT[SLOT3].Cnt += CH->SLOT[SLOT3].Incr);
	pg_in4 = (CH->SLOT[SLOT4].Cnt += CH->SLOT[SLOT4].Incr);
    }

    /* Envelope Generator */
    FM_CALC_EG(eg_out1, CH->SLOT[SLOT1]);
    FM_CALC_EG(eg_out2, CH->SLOT[SLOT2]);
    FM_CALC_EG(eg_out3, CH->SLOT[SLOT3]);
    FM_CALC_EG(eg_out4, CH->SLOT[SLOT4]);

    /* Connection */
    if (eg_out1 < EG_CUT_OFF) {	/* SLOT 1 */
	if (CH->FB) {
	    /* with self feed back */
	    pg_in1 += (CH->op1_out[0] + CH->op1_out[1]) >> CH->FB;
	    CH->op1_out[1] = CH->op1_out[0];
	}
	CH->op1_out[0] = OP_OUT(pg_in1, eg_out1);
	/* output slot1 */
	if (!CH->connect1) {
	    /* algorythm 5  */
	    pg_in2 += CH->op1_out[0];
	    pg_in3 += CH->op1_out[0];
	    pg_in4 += CH->op1_out[0];
	} else {
	    /* other algorythm */
	    *CH->connect1 += CH->op1_out[0];
	}
    }
    if (eg_out2 < EG_CUT_OFF)	/* SLOT 2 */
	*CH->connect2 += OP_OUT(pg_in2, eg_out2);
    if (eg_out3 < EG_CUT_OFF)	/* SLOT 3 */
	*CH->connect3 += OP_OUT(pg_in3, eg_out3);
    if (eg_out4 < EG_CUT_OFF)	/* SLOT 4 */
	*CH->connect4 += OP_OUT(pg_in4, eg_out4);
}

/* ---------- frequency counter for operater update ---------- */
static inline void CALC_FCSLOT(FM_SLOT * SLOT, int fc, int kc)
{
    int ksr;

    /* frequency step counter */
    /* SLOT->Incr= (fc+SLOT->DT[kc])*SLOT->mul; */
    SLOT->Incr = fc * SLOT->mul + SLOT->DT[kc];
    ksr = kc >> SLOT->KSR;
    if (SLOT->ksr != ksr) {
	SLOT->ksr = ksr;
	/* attack , decay rate recalcration */
	SLOT->evsa = SLOT->AR[ksr];
	SLOT->evsd = SLOT->DR[ksr];
	SLOT->evss = SLOT->SR[ksr];
	SLOT->evsr = SLOT->RR[ksr];
    }
}

/* ---------- frequency counter  ---------- */
static inline void OPN_CALC_FCOUNT(FM_CH * CH)
{
    if (CH->SLOT[SLOT1].Incr == -1) {
	int fc = CH->fc;
	int kc = CH->kcode;
	CALC_FCSLOT(&CH->SLOT[SLOT1], fc, kc);
	CALC_FCSLOT(&CH->SLOT[SLOT2], fc, kc);
	CALC_FCSLOT(&CH->SLOT[SLOT3], fc, kc);
	CALC_FCSLOT(&CH->SLOT[SLOT4], fc, kc);
    }
}

/* ----------- initialize time tabls ----------- */
static void init_timetables(FM_ST * ST, Uint8 * DTTABLE, int ARRATE,
			    int DRRATE)
{
    int i, d;
    double rate;

    /* DeTune table */
    for (d = 0; d <= 3; d++) {
	for (i = 0; i <= 31; i++) {
	    rate = (double) DTTABLE[d * 32 + i] * ST->freqbase * FREQ_RATE;
	    ST->DT_TABLE[d][i] = (Sint32) rate;
	    ST->DT_TABLE[d + 4][i] = (Sint32) - rate;
	}
    }
    /* make Attack & Decay tables */
    for (i = 0; i < 4; i++)
	ST->AR_TABLE[i] = ST->DR_TABLE[i] = 0;
    for (i = 4; i < 64; i++) {
	rate = ST->freqbase;	/* frequency rate */
	if (i < 60)
	    rate *= 1.0 + (i & 3) * 0.25;	/* b0-1 : x1 , x1.25 , x1.5 , x1.75 */
	rate *= 1 << ((i >> 2) - 1);	/* b2-5 : shift bit */
	rate *= (double) (EG_ENT << ENV_BITS);
	ST->AR_TABLE[i] = (Sint32) (rate / ARRATE);
	ST->DR_TABLE[i] = (Sint32) (rate / DRRATE);
    }
    ST->AR_TABLE[62] = EG_AED;
    ST->AR_TABLE[63] = EG_AED;
    for (i = 64; i < 94; i++) {	/* make for overflow area */
	ST->AR_TABLE[i] = ST->AR_TABLE[63];
	ST->DR_TABLE[i] = ST->DR_TABLE[63];
    }

#if 0
    for (i = 0; i < 64; i++) {
	//LOG(LOG_WAR,("rate %2d , ar %f ms , dr %f ms \n",i,
	//    ((double)(EG_ENT<<ENV_BITS) / ST->AR_TABLE[i]) * (1000.0 / ST->rate),
	//    ((double)(EG_ENT<<ENV_BITS) / ST->DR_TABLE[i]) * (1000.0 / ST->rate) ));
    }
#endif
}

/* ---------- reset one of channel  ---------- */
static void reset_channel(FM_ST * ST, FM_CH * CH, int chan)
{
    int c, s;

    ST->mode = 0;		/* normal mode */
    FM_STATUS_RESET(ST, 0xff);
    ST->TA = 0;
    ST->TAC = 0;
    ST->TB = 0;
    ST->TBC = 0;

    for (c = 0; c < chan; c++) {
	CH[c].fc = 0;
	CH[c].PAN = OUTD_CENTER;
	for (s = 0; s < 4; s++) {
	    CH[c].SLOT[s].SEG = 0;
	    CH[c].SLOT[s].eg_next = FM_EG_Release;
	    CH[c].SLOT[s].evc = EG_OFF;
	    CH[c].SLOT[s].eve = EG_OFF + 1;
	    CH[c].SLOT[s].evs = 0;
	}
    }
}

/* ---------- generic table initialize ---------- */
static int FMInitTable(void)
{
    int s, t;
    double rate;
    int i, j;
    double pom;

    /* allocate total level table plus+minus section */
    TL_TABLE = (Sint32 *) malloc(2 * TL_MAX * sizeof(int));
    if (TL_TABLE == 0)
	return 0;
    /* make total level table */
    for (t = 0; t < TL_MAX; t++) {
	if (t >= PG_CUT_OFF)
	    rate = 0;		/* under cut off area */
	else
	    rate = ((1 << TL_BITS) - 1) / pow(10, EG_STEP * t / 20);	/* dB -> voltage */
	TL_TABLE[t] = (int) rate;
	TL_TABLE[TL_MAX + t] = -TL_TABLE[t];
/*		LOG(LOG_INF,("TotalLevel(%3d) = %x\n",t,TL_TABLE[t]));*/
    }

    /* make sinwave table (pointer of total level) */
    for (s = 1; s <= SIN_ENT / 4; s++) {
	pom = sin(2.0 * PI * s / SIN_ENT);	/* sin   */
	pom = 20 * log10(1 / pom);	/* -> decibel */
	j = (int) (pom / EG_STEP);	/* TL_TABLE steps */
	/* cut off check */
	if (j > PG_CUT_OFF)
	    j = PG_CUT_OFF;
	/* degree 0   -  90    , degree 180 -  90 : plus section */
	SIN_TABLE[s] = SIN_TABLE[SIN_ENT / 2 - s] = &TL_TABLE[j];
	/* degree 180 - 270    , degree 360 - 270 : minus section */
	SIN_TABLE[SIN_ENT / 2 + s] = SIN_TABLE[SIN_ENT - s] =
	    &TL_TABLE[TL_MAX + j];
	/* LOG(LOG_INF,("sin(%3d) = %f:%f db\n",s,pom,(double)j * EG_STEP)); */
    }
    /* degree 0 = degree 180                   = off */
    SIN_TABLE[0] = SIN_TABLE[SIN_ENT / 2] = &TL_TABLE[PG_CUT_OFF];

    /* envelope counter -> envelope output table */
    for (i = 0; i < EG_ENT; i++) {
	/* ATTACK curve */
	/* !!!!! preliminary !!!!! */
	pom = pow(((double) (EG_ENT - 1 - i) / EG_ENT), 8) * EG_ENT;
	/* if( pom >= EG_ENT ) pom = EG_ENT-1; */
	ENV_CURVE[i] = (int) pom;
	/* DECAY ,RELEASE curve */
	ENV_CURVE[(EG_DST >> ENV_BITS) + i] = i;
#if FM_SEG_SUPPORT
	/* DECAY UPSIDE (SSG ENV) */
	ENV_CURVE[(EG_UST >> ENV_BITS) + i] = EG_ENT - 1 - i;
#endif
    }
    /* off */
    ENV_CURVE[EG_OFF >> ENV_BITS] = EG_ENT - 1;

    /* decay to reattack envelope converttable */
    j = EG_ENT - 1;
    for (i = 0; i < EG_ENT; i++) {
	while (j && (ENV_CURVE[j] < i))
	    j--;
	DRAR_TABLE[i] = j << ENV_BITS;
    }
    return 1;
}


static void FMCloseTable(void)
{
    if (TL_TABLE)
	free(TL_TABLE);
    return;
}

/* OPN/OPM Mode  Register Write */
inline void FMSetMode(FM_ST * ST, int n, int v)
{
    /* b7 = CSM MODE */
    /* b6 = 3 slot mode */
    /* b5 = reset b */
    /* b4 = reset a */
    /* b3 = timer enable b */
    /* b2 = timer enable a */
    /* b1 = load b */
    /* b0 = load a */
    ST->mode = v;

    /* reset Timer b flag */
    if (v & 0x20)
	FM_STATUS_RESET(ST, 0x02);
    /* reset Timer a flag */
    if (v & 0x10)
	FM_STATUS_RESET(ST, 0x01);
    /* load b */
    if (v & 0x02) {
	if (ST->TBC == 0) {
	    ST->TBC = (256 - ST->TB) << 4;
	    /* External timer handler */
	    if (ST->Timer_Handler)
		(ST->Timer_Handler) (1, ST->TBC, ST->TimerBase_f);
	}
    } else {	/* stop interbval timer */
	if (ST->TBC != 0) {
	    ST->TBC = 0;
	    if (ST->Timer_Handler)
		(ST->Timer_Handler) (1, 0, ST->TimerBase_f);
	}
    }
    /* load a */
    if (v & 0x01) {
	if (ST->TAC == 0) {
	    ST->TAC = (1024 - ST->TA);
	    /* External timer handler */
	    if (ST->Timer_Handler)
		(ST->Timer_Handler) (0, ST->TAC, ST->TimerBase_f);
	}
    } else {	/* stop interbval timer */
	if (ST->TAC != 0) {
	    ST->TAC = 0;
	    if (ST->Timer_Handler)
		(ST->Timer_Handler) (0, 0, ST->TimerBase_f);
	}
    }
}

/* Timer A Overflow */
inline void TimerAOver(FM_ST * ST)
{
    //printf("TimerAOver\n");
    /* status set if enabled */
    if (ST->mode & 0x04)
	FM_STATUS_SET(ST, 0x01);
    /* clear or reload the counter */
    if (ST->timermodel == FM_TIMER_INTERVAL) {
	ST->TAC = (1024 - ST->TA);
	if (ST->Timer_Handler)
	    (ST->Timer_Handler) (0, ST->TAC, ST->TimerBase_f);
    } else
	ST->TAC = 0;
}

/* Timer B Overflow */
inline void TimerBOver(FM_ST * ST)
{
    /* status set if enabled */
    if (ST->mode & 0x08)
	FM_STATUS_SET(ST, 0x02);
    /* clear or reload the counter */
    if (ST->timermodel == FM_TIMER_INTERVAL) {
	ST->TBC = (256 - ST->TB) << 4;
	if (ST->Timer_Handler)
	    (ST->Timer_Handler) (1, ST->TBC, ST->TimerBase_f);
    } else
	ST->TBC = 0;
}

/* CSM Key Controll */
inline void CSMKeyControll(FM_CH * CH)
{
    /* all key off */
    /* FM_KEYOFF(CH,SLOT1); */
    /* FM_KEYOFF(CH,SLOT2); */
    /* FM_KEYOFF(CH,SLOT3); */
    /* FM_KEYOFF(CH,SLOT4); */
    /* total level latch */
    CH->SLOT[SLOT1].TLL = CH->SLOT[SLOT1].TL;
    CH->SLOT[SLOT2].TLL = CH->SLOT[SLOT2].TL;
    CH->SLOT[SLOT3].TLL = CH->SLOT[SLOT3].TL;
    CH->SLOT[SLOT4].TLL = CH->SLOT[SLOT4].TL;
    /* all key on */
    FM_KEYON(CH, SLOT1);
    FM_KEYON(CH, SLOT2);
    FM_KEYON(CH, SLOT3);
    FM_KEYON(CH, SLOT4);
}
#if 0
static struct {
    Uint32 DT,RR,SR,DR,AR;
    Uint8 eg_next;
}fm_state[0xFF];

/* FM Save state function */ 
static void FM_post_load(FM_CH *CH,int num_ch) {
    int slot , ch;

    for(ch=0;ch<num_ch;ch++,CH++) {
	setup_connection(CH);
	for(slot=0;slot<4;slot++) {
	    FM_SLOT *SLOT = &CH->SLOT[slot];
	    Uint8 a=(ch<<4)|slot;
	    //printf("PL_SLOT %02d %p %p %p %p %p\n",a,SLOT->AR,SLOT->DR,SLOT->SR,SLOT->RR,SLOT->DT);
	    switch (a) {
	    case 1:
		SLOT->eg_next=FM_EG_SSG_AR;
		break;
	    case 2:
		SLOT->eg_next=FM_EG_SSG_SR;
		break;
	    case 3:
		SLOT->eg_next=FM_EG_SSG_DR;
		break;
	    case 4:
		SLOT->eg_next=FM_EG_AR;
		break;
	    case 5:
		SLOT->eg_next=FM_EG_SR;
		break;
	    case 6:
		SLOT->eg_next=FM_EG_DR;
		break;
	    case 7:
		SLOT->eg_next=FM_EG_Release;
		break;
	    }
	}
    }
}

static void FM_pre_save(FM_CH *CH,int num_ch) {
    int slot , ch;
    for(ch=0;ch<num_ch;ch++,CH++) {
	for(slot=0;slot<4;slot++) {
	    FM_SLOT *SLOT = &CH->SLOT[slot];
	    Uint8 a=(ch<<4)|slot;
	    //printf("PS_SLOT %02d %p %p %p %p %p\n",a,SLOT->AR,SLOT->DR,SLOT->SR,SLOT->RR,SLOT->DT);
	    if (SLOT->eg_next==FM_EG_SSG_AR)
		fm_state[a].eg_next=1;
	    if (SLOT->eg_next==FM_EG_SSG_SR)
		fm_state[a].eg_next=2;
	    if (SLOT->eg_next==FM_EG_SSG_DR)
		fm_state[a].eg_next=3;
	    if (SLOT->eg_next==FM_EG_AR)
		fm_state[a].eg_next=4;
	    if (SLOT->eg_next==FM_EG_SR)
		fm_state[a].eg_next=5;
	    if (SLOT->eg_next==FM_EG_DR)
		fm_state[a].eg_next=6;
	    if (SLOT->eg_next==FM_EG_Release)
		fm_state[a].eg_next=7;
	}
    }
}
#endif
static void FMsave_state_channel(FM_CH *CH,int num_ch)
{
    int slot , ch;
    const char slot_array[4] = { 1 , 3 , 2 , 4 };

    for(ch=0;ch<num_ch;ch++,CH++) {
	/* channel */
	create_state_register(ST_YM2610_FM, "op1_out1" ,ch+1, &CH->op1_out[0] , 1*sizeof(Sint32),REG_INT32);
	create_state_register(ST_YM2610_FM, "op1_out2" ,ch+1, &CH->op1_out[1] , 1*sizeof(Sint32),REG_INT32);
	create_state_register(ST_YM2610_FM, "phasestep"   ,ch+1, &CH->fc , 1*sizeof(Uint32),REG_UINT32);
	create_state_register(ST_YM2610_FM, "fn_h"   ,ch+1, &CH->fn_h , 1*sizeof(Uint8),REG_UINT8);
	create_state_register(ST_YM2610_FM, "kcode"   ,ch+1, &CH->kcode , 1*sizeof(Uint8),REG_UINT8);
	create_state_register(ST_YM2610_FM, "PAN"   ,ch+1, &CH->PAN , 1*sizeof(Uint8),REG_UINT8);
	create_state_register(ST_YM2610_FM, "ALGO"   ,ch+1, &CH->ALGO , 1*sizeof(Uint8),REG_UINT8);
	create_state_register(ST_YM2610_FM, "FB"   ,ch+1, &CH->FB , 1*sizeof(Uint8),REG_UINT8);

	/* slots */
	for(slot=0;slot<4;slot++) {
	    FM_SLOT *SLOT = &CH->SLOT[slot];
	    create_state_register(ST_YM2610_FM, "phasecount" ,(ch<<4)|slot, &SLOT->Cnt, 1*sizeof(Uint32),REG_UINT32);
	    create_state_register(ST_YM2610_FM, "Incr"      ,(ch<<4)|slot, &SLOT->Incr, 1*sizeof(Uint32),REG_UINT32);
	    create_state_register(ST_YM2610_FM, "ksr"      ,(ch<<4)|slot, &SLOT->ksr, 1*sizeof(Uint8),REG_UINT8);
	    create_state_register(ST_YM2610_FM, "evc"     ,(ch<<4)|slot, &SLOT->evc, 1*sizeof(Sint32),REG_INT32);
	    create_state_register(ST_YM2610_FM, "eve"     ,(ch<<4)|slot, &SLOT->eve, 1*sizeof(Sint32),REG_INT32);
	    create_state_register(ST_YM2610_FM, "evs"     ,(ch<<4)|slot, &SLOT->evs, 1*sizeof(Sint32),REG_INT32);
	    create_state_register(ST_YM2610_FM, "evsa"     ,(ch<<4)|slot, &SLOT->evsa, 1*sizeof(Sint32),REG_INT32);
	    create_state_register(ST_YM2610_FM, "evsd"     ,(ch<<4)|slot, &SLOT->evsd, 1*sizeof(Sint32),REG_INT32);
	    create_state_register(ST_YM2610_FM, "evss"     ,(ch<<4)|slot, &SLOT->evss, 1*sizeof(Sint32),REG_INT32);
	    create_state_register(ST_YM2610_FM, "evsr"     ,(ch<<4)|slot, &SLOT->evsr, 1*sizeof(Sint32),REG_INT32);
	    create_state_register(ST_YM2610_FM, "TLL"     ,(ch<<4)|slot, &SLOT->TLL, 1*sizeof(Sint32),REG_INT32);
	    create_state_register(ST_YM2610_FM, "TL"     ,(ch<<4)|slot, &SLOT->TL, 1*sizeof(Uint32),REG_INT32);
//	    create_state_register(ST_YM2610_FM, "EG_NEXT"     ,(ch<<4)|slot, &fm_state[(ch<<4)|slot].eg_next, 1*sizeof(Uint8));
	}
    }
}
/*
static void FM_ST_post_load(FM_ST *ST)
{
    FMSetMode(ST,0,ST->mode);
}
*/
static void OPNWriteReg(FM_OPN * OPN, int r, int v);

void fm_post_load(void)
{
    YM2610 *F2610 = &(FM2610);
    int r;

    /* OPN registers */
    /* DT / MULTI , TL , KS / AR , AMON / DR , SR , SL / RR , SSG-EG */
#if 1
    for(r=0x30;r<0x9e;r++)
	if((r&3) != 3)
	{
	    OPNWriteReg(&F2610->OPN,r,F2610->REGS[r]);
	    OPNWriteReg(&F2610->OPN,r|0x100,F2610->REGS[r|0x100]);
	}
    /* FB / CONNECT , L / R / AMS / PMS */

    for(r=0xb0;r<0xb6;r++)
	if((r&3) != 3)
	{
	    OPNWriteReg(&F2610->OPN,r,F2610->REGS[r]);
	    OPNWriteReg(&F2610->OPN,r|0x100,F2610->REGS[r|0x100]);
	}
    /* FM channels */
    /*FM_channel_postload(F2610->CH,6);*/
#endif
    /*
    FMTimerInit();
    if (FM2610.OPN.ST.Timer_Handler) {
	(FM2610.OPN.ST.Timer_Handler) (1, FM2610.OPN.ST.TBC, FM2610.OPN.ST.TimerBase);
	(FM2610.OPN.ST.Timer_Handler) (0, FM2610.OPN.ST.TAC, FM2610.OPN.ST.TimerBase);
    }
    */

//    FM_ST_post_load(&FM2610.OPN.ST);
//    FM_post_load(FM2610.CH,6);
}

void fm_pre_save(void)
{
    YM2610 *F2610 = &(FM2610);
//    FM_pre_save(FM2610.CH,6);
}

static void FMsave_state_st(FM_ST *ST)
{
	create_state_register(ST_YM2610_FM, "address"   ,1, &ST->address , 1*sizeof(Uint8),REG_UINT8);
	create_state_register(ST_YM2610_FM, "IRQ"       ,1, &ST->irq     , 1*sizeof(Uint8),REG_UINT8);
	create_state_register(ST_YM2610_FM, "IRQ MASK"  ,1, &ST->irqmask , 1*sizeof(Uint8),REG_UINT8);
	create_state_register(ST_YM2610_FM, "status"    ,1, &ST->status  , 1*sizeof(Uint8),REG_UINT8);
	create_state_register(ST_YM2610_FM, "mode"      ,1, &ST->mode    , 1*sizeof(Uint32),REG_UINT32);
	create_state_register(ST_YM2610_FM, "prescaler" ,1, &ST->prescaler_sel , 1*sizeof(Uint8),REG_UINT8);
	create_state_register(ST_YM2610_FM, "freq latch",1, &ST->fn_h , 1*sizeof(Uint8),REG_UINT8);
	create_state_register(ST_YM2610_FM, "TIMER A"   ,1, &ST->TA   ,1*sizeof(Sint32),REG_INT32);
	create_state_register(ST_YM2610_FM, "TIMER Acnt",1, &ST->TAC  ,1*sizeof(Sint32),REG_INT32);
	create_state_register(ST_YM2610_FM, "TIMER B"   ,1, &ST->TB   ,1*sizeof(Uint8),REG_UINT8);
	create_state_register(ST_YM2610_FM, "TIMER Bcnt",1, &ST->TBC  ,1*sizeof(Sint32),REG_INT32);
	set_post_load_function(ST_YM2610_FM,fm_post_load);
//	set_pre_save_function(ST_YM2610_FM,fm_pres_save);
}


#if BUILD_OPN

/* OPN key frequency number -> key code follow table */
/* fnum higher 4bit -> keycode lower 2bit */
static const Uint8 OPN_FKTABLE[16] =
    { 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3 };

#if FM_LFO_SUPPORT
/* OPN LFO waveform table */
static Sint32 OPN_LFO_wave[LFO_ENT];
#endif

static int OPNInitTable(void)
{
#if FM_LFO_SUPPORT
    int i;

    /* LFO wave table */
    for (i = 0; i < LFO_ENT; i++) {
	OPN_LFO_wave[i] =
	    i <
	    LFO_ENT / 2 ? i * LFO_RATE / (LFO_ENT / 2) : (LFO_ENT -
							  i) * LFO_RATE /
	    (LFO_ENT / 2);
    }
#endif
    return FMInitTable();
}

/* ---------- priscaler set(and make time tables) ---------- */
static void OPNSetPris(FM_OPN * OPN, int pris, int TimerPris, int SSGpris)
{
    int i;

    /* frequency base */
    OPN->ST.freqbase =
	(OPN->ST.rate) ? ((double) OPN->ST.clock / OPN->ST.rate) /
	pris : 0;
    OPN->ST.freqbase_f=OPN->ST.freqbase*(1<<TIMER_SH);
    /* Timer base time */
    OPN->ST.TimerBase =
	1.0 / ((double) OPN->ST.clock / (double) TimerPris);
    OPN->ST.TimerBase_f=OPN->ST.TimerBase*(1<<TIMER_SH);

    /* SSG part  priscaler set */
    if (SSGpris)
	SSGClk(OPN->ST.clock * 2 / SSGpris);
    /* make time tables */
    init_timetables(&OPN->ST, OPN_DTTABLE, OPN_ARRATE, OPN_DRRATE);
    /* make fnumber -> increment counter table */
    for (i = 0; i < 2048; i++) {
	/* it is freq table for octave 7 */
	/* opn freq counter = 20bit */
	OPN->FN_TABLE[i] =
	    (Uint32) ((double) i * OPN->ST.freqbase * FREQ_RATE *
		      (1 << 7) / 2);
    }
#if FM_LFO_SUPPORT
    /* LFO freq. table */
    {
	/* 3.98Hz,5.56Hz,6.02Hz,6.37Hz,6.88Hz,9.63Hz,48.1Hz,72.2Hz @ 8MHz */
#define FM_LF(Hz) ((double)LFO_ENT*(1<<LFO_SHIFT)*(Hz)/(8000000.0/144))
	static const double freq_table[8] =
	    { FM_LF(3.98), FM_LF(5.56), FM_LF(6.02), FM_LF(6.37),
FM_LF(6.88), FM_LF(9.63), FM_LF(48.1), FM_LF(72.2) };
#undef FM_LF
	for (i = 0; i < 8; i++) {
	    OPN->LFO_FREQ[i] = (Uint32) (freq_table[i] * OPN->ST.freqbase);
	}
    }
#endif
/*	LOG(LOG_INF,("OPN %d set priscaler %d\n",OPN->ST.index,pris));*/
}

/* ---------- write a OPN mode register 0x20-0x2f ---------- */
static void OPNWriteMode(FM_OPN * OPN, int r, int v)
{
    Uint8 c;
    FM_CH *CH;

    switch (r) {
    case 0x21:			/* Test */
	break;
#if FM_LFO_SUPPORT
    case 0x22:			/* LFO FREQ (YM2608/YM2612) */
	if (OPN->type & TYPE_LFOPAN) {
	    OPN->LFOIncr = (v & 0x08) ? OPN->LFO_FREQ[v & 7] : 0;
	    cur_chip = NULL;
	}
	break;
#endif
    case 0x24:			/* timer A High 8 */
	OPN->ST.TA = (OPN->ST.TA & 0x03) | (((int) v) << 2);
	break;
    case 0x25:			/* timer A Low 2 */
	OPN->ST.TA = (OPN->ST.TA & 0x3fc) | (v & 3);
	break;
    case 0x26:			/* timer B */
	OPN->ST.TB = v;
	break;
    case 0x27:			/* mode , timer controll */
	FMSetMode(&(OPN->ST), OPN->ST.index, v);
	break;
    case 0x28:			/* key on / off */
	c = v & 0x03;
	if (c == 3)
	    break;
	if ((v & 0x04) && (OPN->type & TYPE_6CH))
	    c += 3;
	CH = OPN->P_CH;
	CH = &CH[c];
	/* csm mode */
	/* if( c == 2 && (OPN->ST.mode & 0x80) ) break; */
	if (v & 0x10)
	    FM_KEYON(CH, SLOT1);
	else
	    FM_KEYOFF(CH, SLOT1);
	if (v & 0x20)
	    FM_KEYON(CH, SLOT2);
	else
	    FM_KEYOFF(CH, SLOT2);
	if (v & 0x40)
	    FM_KEYON(CH, SLOT3);
	else
	    FM_KEYOFF(CH, SLOT3);
	if (v & 0x80)
	    FM_KEYON(CH, SLOT4);
	else
	    FM_KEYOFF(CH, SLOT4);
/*		LOG(LOG_INF,("OPN %d:%d : KEY %02X\n",n,c,v&0xf0));*/
	break;
    }
}

/* ---------- write a OPN register (0x30-0xff) ---------- */
static void OPNWriteReg(FM_OPN * OPN, int r, int v)
{
    Uint8 c;
    FM_CH *CH;
    FM_SLOT *SLOT;

    /* 0x30 - 0xff */
    if ((c = OPN_CHAN(r)) == 3)
	return;			/* 0xX3,0xX7,0xXB,0xXF */
    if ((r >= 0x100) /* && (OPN->type & TYPE_6CH) */ )
	c += 3;
    CH = OPN->P_CH;
    CH = &CH[c];

    SLOT = &(CH->SLOT[OPN_SLOT(r)]);
    switch (r & 0xf0) {
    case 0x30:			/* DET , MUL */
	set_det_mul(&OPN->ST, CH, SLOT, v);
	break;
    case 0x40:			/* TL */
	set_tl(CH, SLOT, v, (c == 2) && (OPN->ST.mode & 0x80));
	break;
    case 0x50:			/* KS, AR */
	set_ar_ksr(CH, SLOT, v, OPN->ST.AR_TABLE);
	break;
    case 0x60:			/*     DR */
	/* bit7 = AMS_ON ENABLE(YM2612) */
	set_dr(SLOT, v, OPN->ST.DR_TABLE);
#if FM_LFO_SUPPORT
	if (OPN->type & TYPE_LFOPAN) {
	    SLOT->amon = v >> 7;
	    SLOT->ams = CH->ams * SLOT->amon;
	}
#endif
	break;
    case 0x70:			/*     SR */
	set_sr(SLOT, v, OPN->ST.DR_TABLE);
	break;
    case 0x80:			/* SL, RR */
	set_sl_rr(SLOT, v, OPN->ST.DR_TABLE);
	break;
    case 0x90:			/* SSG-EG */
#if !FM_SEG_SUPPORT
	//if(v&0x08) 
	//LOG(LOG_ERR,("OPN %d,%d,%d :SSG-TYPE envelope selected (not supported )\n",OPN->ST.index,c,OPN_SLOT(r)));
#endif
	SLOT->SEG = v & 0x0f;
	break;
    case 0xa0:
	switch (OPN_SLOT(r)) {
	case 0:		/* 0xa0-0xa2 : FNUM1 */
	    {
		Uint32 fn = (((Uint32) ((CH->fn_h) & 7)) << 8) + v;
		Uint8 blk = CH->fn_h >> 3;
		/* make keyscale code */
		CH->kcode = (blk << 2) | OPN_FKTABLE[(fn >> 7)];
		/* make basic increment counter 32bit = 1 cycle */
		CH->fc = OPN->FN_TABLE[fn] >> (7 - blk);
		CH->SLOT[SLOT1].Incr = -1;
	    }
	    break;
	case 1:		/* 0xa4-0xa6 : FNUM2,BLK */
	    CH->fn_h = v & 0x3f;
	    break;
	case 2:		/* 0xa8-0xaa : 3CH FNUM1 */
	    if (r < 0x100) {
		Uint32 fn = (((Uint32) (OPN->SL3.fn_h[c] & 7)) << 8) + v;
		Uint8 blk = OPN->SL3.fn_h[c] >> 3;
		/* make keyscale code */
		OPN->SL3.kcode[c] = (blk << 2) | OPN_FKTABLE[(fn >> 7)];
		/* make basic increment counter 32bit = 1 cycle */
		OPN->SL3.fc[c] = OPN->FN_TABLE[fn] >> (7 - blk);
		(OPN->P_CH)[2].SLOT[SLOT1].Incr = -1;
	    }
	    break;
	case 3:		/* 0xac-0xae : 3CH FNUM2,BLK */
	    if (r < 0x100)
		OPN->SL3.fn_h[c] = v & 0x3f;
	    break;
	}
	break;
    case 0xb0:
	switch (OPN_SLOT(r)) {
	case 0:		/* 0xb0-0xb2 : FB,ALGO */
	    {
		int feedback = (v >> 3) & 7;
		CH->ALGO = v & 7;
		CH->FB = feedback ? 8 + 1 - feedback : 0;
		setup_connection(CH);
	    }
	    break;
	case 1:		/* 0xb4-0xb6 : L , R , AMS , PMS (YM2612/YM2608) */
	    if (OPN->type & TYPE_LFOPAN) {
#if FM_LFO_SUPPORT
		/* b0-2 PMS */
		/* 0,3.4,6.7,10,14,20,40,80(cent) */
		static const double pmd_table[8] =
		    { 0, 3.4, 6.7, 10, 14, 20, 40, 80 };
		static const int amd_table[4] =
		    { (int) (0 / EG_STEP), (int) (1.4 / EG_STEP),
(int) (5.9 / EG_STEP), (int) (11.8 / EG_STEP) };
		CH->pms =
		    (Sint32) ((1.5 / 1200.0) * pmd_table[v & 7] * PMS_RATE);
		/* b4-5 AMS */
		/* 0 , 1.4 , 5.9 , 11.8(dB) */
		CH->ams = amd_table[(v >> 4) & 0x03];
		CH->SLOT[SLOT1].ams = CH->ams * CH->SLOT[SLOT1].amon;
		CH->SLOT[SLOT2].ams = CH->ams * CH->SLOT[SLOT2].amon;
		CH->SLOT[SLOT3].ams = CH->ams * CH->SLOT[SLOT3].amon;
		CH->SLOT[SLOT4].ams = CH->ams * CH->SLOT[SLOT4].amon;
#endif
		/* PAN */
		CH->PAN = (v >> 6) & 0x03;	/* PAN : b6 = R , b7 = L */
		setup_connection(CH);
		/* LOG(LOG_INF,("OPN %d,%d : PAN %d\n",n,c,CH->PAN)); */
	    }
	    break;
	}
	break;
    }
}
#endif				/* BUILD_OPN */


#if (BUILD_YM2608||BUILD_OPNB)


/* here's the virtual YM2608 */
typedef YM2610 YM2608;
YM2610 FM2610;
#endif				/* (BUILD_YM2608||BUILD_OPNB) */

#if BUILD_FM_ADPCMA
/***************************************************************/
/*    ADPCMA units are made by Hiromitsu Shioya (MMSND)         */
/***************************************************************/

/**** YM2610 ADPCM defines ****/
#define ADPCMA_MIXING_LEVEL  (3)	/* ADPCMA mixing level   */
#define ADPCM_SHIFT    (16)	/* frequency step rate   */
#define ADPCMA_ADDRESS_SHIFT 8	/* adpcm A address shift */

/*#define ADPCMA_DECODE_RANGE 1024 */
#define ADPCMA_DECODE_RANGE 2048
#define ADPCMA_DECODE_MIN (-(ADPCMA_DECODE_RANGE*ADPCMA_MIXING_LEVEL))
#define ADPCMA_DECODE_MAX ((ADPCMA_DECODE_RANGE*ADPCMA_MIXING_LEVEL)-1)
#define ADPCMA_VOLUME_DIV 1

static Uint8 *pcmbufA;
static Uint32 pcmsizeA;

/************************************************************/
/************************************************************/
/* --------------------- subroutines  --------------------- */
/************************************************************/
/************************************************************/
/************************/
/*    ADPCM A tables    */
/************************/
static int jedi_table[(48 + 1) * 16];
static int decode_tableA1[16] = {
    -1 * 16, -1 * 16, -1 * 16, -1 * 16, 2 * 16, 5 * 16, 7 * 16, 9 * 16,
    -1 * 16, -1 * 16, -1 * 16, -1 * 16, 2 * 16, 5 * 16, 7 * 16, 9 * 16
};

/* 0.9 , 0.9 , 0.9 , 0.9 , 1.2 , 1.6 , 2.0 , 2.4 */
/* 8 = -1 , 2 5 8 11 */
/* 9 = -1 , 2 5 9 13 */
/* 10= -1 , 2 6 10 14 */
/* 12= -1 , 2 7 12 17 */
/* 20= -2 , 4 12 20 32 */

#if 1
static void InitOPNB_ADPCMATable(void)
{
    int step, nib;

    for (step = 0; step <= 48; step++) {
	double stepval =
	    floor(16.0 * pow(11.0 / 10.0, (double) step) *
		  ADPCMA_MIXING_LEVEL);
	/* loop over all nibbles and compute the difference */
	for (nib = 0; nib < 16; nib++) {
	    int value = (int) stepval * ((nib & 0x07) * 2 + 1) / 8;
	    jedi_table[step * 16 + nib] = (nib & 0x08) ? -value : value;
	}
    }
}
#else
static int decode_tableA2[49] = {
    0x0010, 0x0011, 0x0013, 0x0015, 0x0017, 0x0019, 0x001c, 0x001f,
    0x0022, 0x0025, 0x0029, 0x002d, 0x0032, 0x0037, 0x003c, 0x0042,
    0x0049, 0x0050, 0x0058, 0x0061, 0x006b, 0x0076, 0x0082, 0x008f,
    0x009d, 0x00ad, 0x00be, 0x00d1, 0x00e6, 0x00fd, 0x0117, 0x0133,
    0x0151, 0x0173, 0x0198, 0x01c1, 0x01ee, 0x0220, 0x0256, 0x0292,
    0x02d4, 0x031c, 0x036c, 0x03c3, 0x0424, 0x048e, 0x0502, 0x0583,
    0x0610
};
static void InitOPNB_ADPCMATable(void)
{
    int ta, tb, tc;
    for (ta = 0; ta < 49; ta++) {
	for (tb = 0; tb < 16; tb++) {
	    tc = 0;
	    if (tb & 0x04) {
		tc += ((decode_tableA2[ta] * ADPCMA_MIXING_LEVEL));
	    }
	    if (tb & 0x02) {
		tc += ((decode_tableA2[ta] * ADPCMA_MIXING_LEVEL) >> 1);
	    }
	    if (tb & 0x01) {
		tc += ((decode_tableA2[ta] * ADPCMA_MIXING_LEVEL) >> 2);
	    }
	    tc += ((decode_tableA2[ta] * ADPCMA_MIXING_LEVEL) >> 3);
	    if (tb & 0x08) {
		tc = (0 - tc);
	    }
	    jedi_table[ta * 16 + tb] = tc;
	}
    }
}
#endif

/**** ADPCM A (Non control type) ****/
inline void OPNB_ADPCM_CALC_CHA(YM2610 * F2610, ADPCM_CH * ch)
{
    Uint32 step;
    int data;

    ch->now_step += ch->step;
    if (ch->now_step >= (1 << ADPCM_SHIFT)) {
	step = ch->now_step >> ADPCM_SHIFT;
	ch->now_step &= (1 << ADPCM_SHIFT) - 1;
	/* end check */
	if ((ch->now_addr + step) > (ch->end << 1)) {
	    ch->flag = 0;
	    F2610->adpcm_arrivedEndAddress |= ch->flagMask;
	    return;
	}
	do {
#if 0
	    if (ch->now_addr > (pcmsizeA << 1)) {
		//LOG(LOG_WAR,("YM2610: Attempting to play past adpcm rom size!\n" ));
		return;
	    }
#endif
	    if (ch->now_addr & 1)
		data = ch->now_data & 0x0f;
	    else {
		ch->now_data = *(pcmbufA + (ch->now_addr >> 1));
		data = (ch->now_data >> 4) & 0x0f;
	    }
	    ch->now_addr++;

	    ch->adpcmx += jedi_table[ch->adpcmd + data];
	    Limit(ch->adpcmx, ADPCMA_DECODE_MAX, ADPCMA_DECODE_MIN);
	    ch->adpcmd += decode_tableA1[data];
	    Limit(ch->adpcmd, 48 * 16, 0 * 16);
			/**** calc pcm * volume data ****/
	    ch->adpcml = ch->adpcmx * ch->volume;
	} while (--step);
    }
    /* output for work of output channels (out_ch[OPNxxxx]) */
    *(ch->pan) += ch->adpcml;
}

/* ADPCM type A */
static void FM_ADPCMAWrite(YM2610 * F2610, int r, int v)
{
    ADPCM_CH *adpcm = F2610->adpcm;
    Uint8 c = r & 0x07;
    //printf("b: %p %p\n",F2610,adpcm);
    F2610->adpcmreg[r] = v & 0xff;	/* stock data */
    switch (r) {
    case 0x00:			/* DM,--,C5,C4,C3,C2,C1,C0 */
	/* F2610->port1state = v&0xff; */
	if (!(v & 0x80)) {
	    /* KEY ON */
	    for (c = 0; c < 6; c++) {
		if ((1 << c) & v) {
					/**** start adpcm ****/
			/*adpcm[c].step =
			(Uint32) ((float) (1 << ADPCM_SHIFT) *
			((float) F2610->OPN.ST.freqbase) / 3.0);*/
		    adpcm[c].step = (F2610->OPN.ST.freqbase_f>>(TIMER_SH-ADPCM_SHIFT)) / 3;
		    adpcm[c].now_addr = adpcm[c].start << 1;
		    adpcm[c].now_step = (1 << ADPCM_SHIFT) - adpcm[c].step;
		    /*adpcm[c].adpcmm   = 0; */
		    adpcm[c].adpcmx = 0;
		    adpcm[c].adpcmd = 0;
		    adpcm[c].adpcml = 0;
		    adpcm[c].flag = 1;
		    if (F2610->pcmbuf == NULL) {	/* Check ROM Mapped */
			adpcm[c].flag = 0;
		    } else {
			if (adpcm[c].end >= F2610->pcm_size) {	/* Check End in Range */
			    adpcm[c].end = F2610->pcm_size - 1;
			}
			if (adpcm[c].start >= F2610->pcm_size) {	/* Check Start in Range */

			    adpcm[c].flag = 0;
			}
		    }
		}			    /*** (1<<c)&v ***/
	    }			/**** for loop ****/
	} else {
	    /* KEY OFF */
	    for (c = 0; c < 6; c++) {
		if ((1 << c) & v)
		    adpcm[c].flag = 0;
	    }
	}
	break;
    case 0x01:			/* B0-5 = TL 0.75dB step */
	F2610->adpcmTL =
	    &(TL_TABLE[((v & 0x3f) ^ 0x3f) * (int) (0.75 / EG_STEP)]);
	for (c = 0; c < 6; c++) {
	    adpcm[c].volume =
		F2610->adpcmTL[adpcm[c].IL * (int) (0.75 / EG_STEP)] /
		ADPCMA_DECODE_RANGE / ADPCMA_VOLUME_DIV;
			/**** calc pcm * volume data ****/
	    adpcm[c].adpcml = adpcm[c].adpcmx * adpcm[c].volume;
	}
	break;
    default:
	c = r & 0x07;
	if (c >= 0x06)
	    return;
	switch (r & 0x38) {
	case 0x08:		/* B7=L,B6=R,B4-0=IL */
	    adpcm[c].IL = (v & 0x1f) ^ 0x1f;
	    adpcm[c].volume =
		F2610->adpcmTL[adpcm[c].IL * (int) (0.75 / EG_STEP)] /
		ADPCMA_DECODE_RANGE / ADPCMA_VOLUME_DIV;
	    adpcm[c].pan = &out_ch[(v >> 6) & 0x03];
			/**** calc pcm * volume data ****/
	    adpcm[c].adpcml = adpcm[c].adpcmx * adpcm[c].volume;
	    break;
	case 0x10:
	case 0x18:
	    adpcm[c].start =
		((F2610->adpcmreg[0x18 + c] *
		  0x0100 | F2610->adpcmreg[0x10 +
					   c]) << ADPCMA_ADDRESS_SHIFT);
	    //printf("DEBUG -- admpcma ch%d start %lx\n",c,adpcm[c].start);
	    break;
	case 0x20:
	case 0x28:
	    adpcm[c].end =
		((F2610->adpcmreg[0x28 + c] *
		  0x0100 | F2610->adpcmreg[0x20 +
					   c]) << ADPCMA_ADDRESS_SHIFT);
	    adpcm[c].end += (1 << ADPCMA_ADDRESS_SHIFT) - 1;
	    //printf("DEBUG -- admpcma ch%d end %lx\n",c,adpcm[c].end);
	    break;
	}
    }
}

/* FM channel save , internal state only */
static void adpcma_post_load(void)
{
    YM2610 *F2610 = &(FM2610);
    int r;
    /* rhythm(ADPCMA) *//*
    FM_ADPCMAWrite(F2610,1,F2610->REGS[0x111]);
    for( r=0x08 ; r<0x0c ; r++)
	FM_ADPCMAWrite(F2610,r,F2610->REGS[r+0x110]);
			*/

}

static void FMsave_state_adpcma(ADPCM_CH *adpcm)
{
    int ch;

    for(ch=0;ch<6;ch++,adpcm++) {
	create_state_register(ST_YM2610_ADPCMA, "flag"    ,ch+1, &adpcm->flag      , 1*sizeof(Uint8),REG_UINT8);
	create_state_register(ST_YM2610_ADPCMA, "flag_mask"    ,ch+1, &adpcm->flagMask      , 1*sizeof(Uint8),REG_UINT8);
	create_state_register(ST_YM2610_ADPCMA, "now_data"    ,ch+1, &adpcm->now_data  , 1*sizeof(Uint8),REG_UINT8);
	create_state_register(ST_YM2610_ADPCMA, "now_addr"    ,ch+1, &adpcm->now_addr  , 1*sizeof(Uint32),REG_UINT32);
	create_state_register(ST_YM2610_ADPCMA, "now_step"    ,ch+1, &adpcm->now_step  , 1*sizeof(Uint32),REG_UINT32);
	create_state_register(ST_YM2610_ADPCMA, "IL"   ,ch+1, &adpcm->IL , 1*sizeof(Sint32),REG_INT32);
	
	create_state_register(ST_YM2610_ADPCMA, "step"    ,ch+1, &adpcm->step  , 1*sizeof(Uint32),REG_UINT32);
	create_state_register(ST_YM2610_ADPCMA, "start"    ,ch+1, &adpcm->start  , 1*sizeof(Uint32),REG_UINT32);
	create_state_register(ST_YM2610_ADPCMA, "end"    ,ch+1, &adpcm->end  , 1*sizeof(Uint32),REG_UINT32);
	
	create_state_register(ST_YM2610_ADPCMA, "adpcmx"   ,ch+1, &adpcm->adpcmx , 1*sizeof(Sint32),REG_INT32);
	create_state_register(ST_YM2610_ADPCMA, "adpcmd"  ,ch+1, &adpcm->adpcmd, 1*sizeof(Sint32),REG_INT32);
	create_state_register(ST_YM2610_ADPCMA, "adpcml"   ,ch+1, &adpcm->adpcml , 1*sizeof(Sint32),REG_INT32);
	create_state_register(ST_YM2610_ADPCMA, "volume"   ,ch+1, &adpcm->volume , 1*sizeof(Sint32),REG_INT32);
    }
    set_post_load_function(ST_YM2610_ADPCMA,adpcma_post_load);
}

#endif				/* BUILD_FM_ADPCMA */


#if BUILD_OPNB
/* -------------------------- YM2610(OPNB) ---------------------------------- */

// static int YM2610NumChips;   /* total chip */

/* ---------- update one of chip (YM2610B FM6: ADPCM-A6: ADPCM-B:1) ----------- */
void YM2610UpdateOne(int num, Sint16 ** buffer, int length)
{
    YM2610 *F2610 = &(FM2610);
    FM_OPN *OPN = &(FM2610.OPN);
    YM_DELTAT *DELTAT = &(FM2610.deltaT);
    int i, j;
    int ch;
    FMSAMPLE *bufL, *bufR;
    Sint16 *aybuf[3];
/*
    for (i = 0; i < 0; i++) {
	aybuf[i] = alloca(sizeof(Sint16) * 16384);
    }
*/
    /* setup DELTA-T unit */
    YM_DELTAT_DECODE_PRESET(DELTAT);

    /* buffer setup */
    bufL = buffer[0];
    bufR = buffer[1];

    if ((void *) F2610 != cur_chip) {
	cur_chip = (void *) F2610;
	State = &OPN->ST;
	cch[0] = &F2610->CH[1];
	cch[1] = &F2610->CH[2];
	cch[2] = &F2610->CH[4];
	cch[3] = &F2610->CH[5];
	/* setup adpcm rom address */
	pcmbufA = F2610->pcmbuf;
	pcmsizeA = F2610->pcm_size;
#if FM_LFO_SUPPORT
	LFOCnt = OPN->LFOCnt;
	LFOIncr = OPN->LFOIncr;
	if (!LFOIncr)
	    lfo_amd = lfo_pmd = 0;
#endif
    }


    /* update frequency counter */
    OPN_CALC_FCOUNT(cch[0]);
    if ((State->mode & 0xc0)) {
	/* 3SLOT MODE */
	if (cch[1]->SLOT[SLOT1].Incr == -1) {
	    /* 3 slot mode */
	    CALC_FCSLOT(&cch[1]->SLOT[SLOT1], OPN->SL3.fc[1],
			OPN->SL3.kcode[1]);
	    CALC_FCSLOT(&cch[1]->SLOT[SLOT2], OPN->SL3.fc[2],
			OPN->SL3.kcode[2]);
	    CALC_FCSLOT(&cch[1]->SLOT[SLOT3], OPN->SL3.fc[0],
			OPN->SL3.kcode[0]);
	    CALC_FCSLOT(&cch[1]->SLOT[SLOT4], cch[1]->fc, cch[1]->kcode);
	}
    } else
	OPN_CALC_FCOUNT(cch[1]);

    OPN_CALC_FCOUNT(cch[2]);
    OPN_CALC_FCOUNT(cch[3]);


    /* buffering */
    for (i = 0; i < length; i++) {
#if FM_LFO_SUPPORT
	/* LFO */
	if (LFOIncr) {
	    lfo_amd = OPN_LFO_wave[(LFOCnt += LFOIncr) >> LFO_SHIFT];
	    lfo_pmd = lfo_amd - (LFO_RATE / 2);
	}
#endif
	/* clear output acc. */
	out_ch[OUTD_LEFT] = out_ch[OUTD_RIGHT] = out_ch[OUTD_CENTER] = 0;
		/**** deltaT ADPCM ****/

	if (DELTAT->flag)
	    YM_DELTAT_ADPCM_CALC(DELTAT);


	/* FM */
	for (ch = 0; ch < 4; ch++)
	    FM_CALC_CH(cch[ch]);

	for (j = 0; j < 6; j++) {
		/**** ADPCM ****/
		if (F2610->adpcm[j].flag)
			OPNB_ADPCM_CALC_CHA(F2610, &F2610->adpcm[j]);
	}


	/* buffering */
	FM_BUFFERING_STEREO;

	
	//my_timer();
	/* timer A controll */
	INTERNAL_TIMER_A(State, cch[1])
		
    }
    //my_timer();
    INTERNAL_TIMER_B(State, length)
#if FM_LFO_SUPPORT
	OPN->LFOCnt = LFOCnt;
#endif

    //SDL_UnlockAudio();

}

static void YM2610_postload(void)
{
    int num , r;
    YM2610 *F2610 = &(FM2610);

    //printf("POSTLOAD DUMP:\n");
    //dump_struct(F2610,0);

    /* SSG registers */
    for(r=0;r<16;r++)
    {
	SSGWrite(0,r);
	SSGWrite(1,F2610->REGS[r]);
    }


    /* Delta-T ADPCM unit */
    //YM_DELTAT_postload(&F2610->deltaT , &F2610->REGS[0x100] );
}

static void YM2610_presave(void) {
    YM2610 *F2610 = &(FM2610);
    //printf("PRESAVE DUMP:\n");
    //dump_struct(F2610,1);
}

static void YM2610_save_state(void)
{
    YM2610 *F2610 = &(FM2610);
//    create_state_register(ST_YM2610, "ym2610"   ,1, F2610   , sizeof(YM2610));
    create_state_register(ST_YM2610, "out_ch"   ,1, out_ch   , 4*sizeof(Sint32),REG_INT32);
    create_state_register(ST_YM2610, "pg_in1"   ,1, &pg_in1   , 1*sizeof(Sint32),REG_INT32);
    create_state_register(ST_YM2610, "pg_in2"   ,1, &pg_in2   , 1*sizeof(Sint32),REG_INT32);
    create_state_register(ST_YM2610, "pg_in3"   ,1, &pg_in3   , 1*sizeof(Sint32),REG_INT32);
    create_state_register(ST_YM2610, "pg_in4"   ,1, &pg_in4   , 1*sizeof(Sint32),REG_INT32);
    
  
#if 1
    create_state_register(ST_YM2610, "regs"   ,1, F2610->REGS   , 512*sizeof(Uint8),REG_UINT8);
    FMsave_state_st(&FM2610.OPN.ST);
    FMsave_state_channel(FM2610.CH,6);
    /* 3slots */
    create_state_register(ST_YM2610, "slot3fc" ,1, F2610->OPN.SL3.fc , 3*sizeof(Uint32),REG_UINT32);
    create_state_register(ST_YM2610, "slot3fh" ,1, &F2610->OPN.SL3.fn_h , 1*sizeof(Uint8),REG_UINT8);
    create_state_register(ST_YM2610, "slot3kc" ,1, F2610->OPN.SL3.kcode , 3*sizeof(Uint8),REG_UINT8);
    /* address register1 */
    create_state_register(ST_YM2610, "address1" ,1, &F2610->address1, 1*sizeof(Sint32),REG_INT32);
    create_state_register(ST_YM2610, "arrivedFlag",1, &F2610->adpcm_arrivedEndAddress , 1*sizeof(Uint8),REG_UINT8);
    /* rythm(ADPCMA) */
    FMsave_state_adpcma(F2610->adpcm);
    /* Delta-T ADPCM unit */
    YM_DELTAT_savestate(&FM2610.deltaT);


#endif
    set_post_load_function(ST_YM2610,YM2610_postload);
    set_pre_save_function(ST_YM2610,YM2610_presave);
  
}

#endif				/* BUILD_OPNB */


#if BUILD_OPNB
int YM2610Init(int clock, int rate,
	       void *pcmroma, int pcmsizea, void *pcmromb, int pcmsizeb,
	       FM_TIMERHANDLER TimerHandler, FM_IRQHANDLER IRQHandler)
{

    cur_chip = NULL;		//&FM2610;

    /* allocate total level table (128kb space) */
    if (!OPNInitTable()) {

	return (-1);
    }


    /* FM */
    FM2610.OPN.ST.index = 0;
    FM2610.OPN.type = TYPE_YM2610;
    FM2610.OPN.P_CH = FM2610.CH;
    FM2610.OPN.ST.clock = clock;
    FM2610.OPN.ST.rate = rate;
    FM2610.OPN.ST.timermodel = FM_TIMER_INTERVAL;
    /* Extend handler */
    FM2610.OPN.ST.Timer_Handler = TimerHandler;
    FM2610.OPN.ST.IRQ_Handler = IRQHandler;
    /* ADPCM */
    FM2610.pcmbuf = (Uint8 *) (pcmroma);
    FM2610.pcm_size = pcmsizea;
    /* DELTA-T */
    FM2610.deltaT.memory = (Uint8 *) (pcmromb);
    FM2610.deltaT.memory_size = pcmsizeb;
    /* */
    YM2610ResetChip();
    //      }
    InitOPNB_ADPCMATable();
    YM2610_save_state();
    return 0;
}

/* ---------- shut down emurator ----------- */
void YM2610Shutdown()
{

    FMCloseTable();
}

/* ---------- reset one of chip ---------- */
void YM2610ResetChip( /*int num */ void)
{
    int i;
    YM2610 *F2610 = &(FM2610);
    FM_OPN *OPN = &(FM2610.OPN);
    YM_DELTAT *DELTAT = &(FM2610.deltaT);

    /* Reset Priscaler */
    OPNSetPris(OPN, 6 * 24, 6 * 24, 4 * 2);	/* OPN 1/6 , SSG 1/4 */
    /* reset SSG section */
    SSGReset();
    /* status clear */
    FM_IRQMASK_SET(&OPN->ST, 0x03);
    OPNWriteMode(OPN, 0x27, 0x30);	/* mode 0 , timer reset */
    reset_channel(&OPN->ST, F2610->CH, 6);
    /* reset OPerator paramater */
    for (i = 0xb6; i >= 0xb4; i--) {
	OPNWriteReg(OPN, i, 0xc0);
	OPNWriteReg(OPN, i | 0x100, 0xc0);
    }
    for (i = 0xb2; i >= 0x30; i--) {
	OPNWriteReg(OPN, i, 0);
	OPNWriteReg(OPN, i | 0x100, 0);
    }
    for (i = 0x26; i >= 0x20; i--)
	OPNWriteReg(OPN, i, 0);
	/**** ADPCM work initial ****/
    for (i = 0; i < 6 + 1; i++) {
	F2610->adpcm[i].now_addr = 0;
	F2610->adpcm[i].now_step = 0;
	F2610->adpcm[i].step = 0;
	F2610->adpcm[i].start = 0;
	F2610->adpcm[i].end = 0;
	F2610->adpcm[i].volume = 0;
	F2610->adpcm[i].pan = &out_ch[OUTD_CENTER];	/* default center */
	F2610->adpcm[i].flagMask = (i == 6) ? 0x80 : (1 << i);
	F2610->adpcm[i].flag = 0;
	F2610->adpcm[i].adpcmx = 0;
	F2610->adpcm[i].adpcmd = 127;
	F2610->adpcm[i].adpcml = 0;
    }
    F2610->adpcmTL = &(TL_TABLE[0x3f * (int) (0.75 / EG_STEP)]);
    F2610->adpcm_arrivedEndAddress = 0;

    /* DELTA-T unit */
    DELTAT->freqbase = OPN->ST.freqbase;
    DELTAT->output_pointer = out_ch;
    DELTAT->portshift = 8;	/* allways 8bits shift */
    DELTAT->output_range = DELTAT_MIXING_LEVEL << TL_BITS;
    YM_DELTAT_ADPCM_Reset(DELTAT, OUTD_CENTER);
}

/* YM2610 write */
/* n = number  */
/* a = address */
/* v = value   */
int YM2610Write(int a, Uint8 v)
{
    YM2610 *F2610 = &(FM2610);
    FM_OPN *OPN = &(FM2610.OPN);
    int addr;
    int ch;

    switch (a & 3) {
    case 0:			/* address port 0 */
	OPN->ST.address = v & 0xff;
	/* Write register to SSG emurator */
	if (v < 16)
	    SSGWrite(0, v);
	break;
    case 1:			/* data port 0    */
	addr = OPN->ST.address;
	F2610->REGS[addr] = v; /* for save state */
	switch (addr & 0xf0) {
	case 0x00:		/* SSG section */
	    /* Write data to SSG emurator */
	    SSGWrite(a, v);
	    break;
	case 0x10:		/* DeltaT ADPCM */
	    YM2610UpdateReq();
	    switch (addr) {
	    case 0x1c:		/*  FLAG CONTROL : Extend Status Clear/Mask */
		{
		    Uint8 statusmask = ~v;
		    /* set arrived flag mask */
		    for (ch = 0; ch < 6; ch++)
			F2610->adpcm[ch].flagMask = statusmask & (1 << ch);
		    F2610->deltaT.flagMask = statusmask & 0x80;
		    /* clear arrived flag */
		    F2610->adpcm_arrivedEndAddress &= statusmask & 0x3f;
		    F2610->deltaT.arrivedFlag &= F2610->deltaT.flagMask;
		}
		break;
	    default:
		/* 0x10-0x1b */
		YM_DELTAT_ADPCM_Write(&F2610->deltaT, addr - 0x10, v);
	    }
	    break;
	case 0x20:		/* Mode Register */
	    YM2610UpdateReq();
	    OPNWriteMode(OPN, addr, v);
	    break;
	default:		/* OPN section */
	    YM2610UpdateReq( /*n */ );
	    /* write register */
	    OPNWriteReg(OPN, addr, v);
	}
	break;
    case 2:			/* address port 1 */
	F2610->address1 = v & 0xff;
	break;
    case 3:			/* data port 1    */
	YM2610UpdateReq();
	addr = F2610->address1;
	F2610->REGS[addr|0x100] = v;
	if (addr < 0x30) {
	    /* 100-12f : ADPCM A section */
	    //printf("a:%p\n",F2610);
	    FM_ADPCMAWrite(F2610, addr, v);
	} else
	    OPNWriteReg(OPN, addr | 0x100, v);
    }
    return OPN->ST.irq;
}

Uint8 YM2610Read(int a)
{
    YM2610 *F2610 = &(FM2610);
    int addr = F2610->OPN.ST.address;
    Uint8 ret = 0;

    switch (a & 3) {
    case 0:			/* status 0 : YM2203 compatible */
	ret = F2610->OPN.ST.status & 0x83;
	break;
    case 1:			/* data 0 */
	if (addr < 16)
	    ret = SSGRead();
	if (addr == 0xff)
	    ret = 0x01;
	break;
    case 2:			/* status 1 : + ADPCM status */
	/* ADPCM STATUS (arrived End Address) */
	/* B,--,A5,A4,A3,A2,A1,A0 */
	/* B     = ADPCM-B(DELTA-T) arrived end address */
	/* A0-A5 = ADPCM-A          arrived end address */
	ret = F2610->adpcm_arrivedEndAddress | F2610->deltaT.arrivedFlag;
	break;
    case 3:
	ret = 0;
	break;
    }
    return ret;
}

int YM2610TimerOver(int c)
{
    YM2610 *F2610 = &(FM2610);
    //printf("ym2610 timer over %d %d \n",n,c);
    if (c) {			/* Timer B */
	TimerBOver(&(F2610->OPN.ST));
    } else {			/* Timer A */
	YM2610UpdateReq();
	/* timer update */
	TimerAOver(&(F2610->OPN.ST));
	/* CSM mode key,TL controll */
	if (F2610->OPN.ST.mode & 0x80) {	/* CSM mode total level latch and auto key on */
	    CSMKeyControll(&(F2610->CH[2]));
	}
    }
    return F2610->OPN.ST.irq;
}

#endif				/* BUILD_OPNB */
