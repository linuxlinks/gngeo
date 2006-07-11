/*
  File: fm.h -- header file for software emuration for FM sound genelator

*/
#ifndef _H_FM_FM_
#define _H_FM_FM_

#include "SDL.h" /* UintXX definition */ 
#include "ymdeltat.h"
/* --- select emulation chips --- */

#define BUILD_YM2610  1		//(HAS_YM2610)  /* build YM2610(OPNB)  emulator */

/* --- system optimize --- */
/* select stereo output buffer : 1=mixing / 0=separate */
#define FM_STEREO_MIX 0
/* select bit size of output : 8 or 16 */
#define FM_OUTPUT_BIT 16
/* select timer system internal or external */
#define FM_INTERNAL_TIMER 0

/* --- speedup optimize --- */
/* support LFO unit */
#ifdef GP2X
#define FM_LFO_SUPPORT 0
#else
#define FM_LFO_SUPPORT 1
#endif
/* support OPN SSG type envelope mode */
#define FM_SEG_SUPPORT 1

/* --- external SSG(YM2149/AY-3-8910)emulator interface port */
/* used by YM2203,YM2608,and YM2610 */

/* SSGClk   : Set SSG Clock      */
/* int n    = chip number        */
/* int clk  = MasterClock(Hz)    */
/* int rate = sample rate(Hz) */
#define SSGClk(/*chip,*/clock) AY8910_set_clock(/*chip,*/clock)

/* SSGWrite : Write SSG port     */
/* int n    = chip number        */
/* int a    = address            */
/* int v    = data               */
#define SSGWrite(/*n,*/a,v) AY8910Write(/*n,*/a,v)

/* SSGRead  : Read SSG port */
/* int n    = chip number   */
/* return   = Read data     */
#define SSGRead(/*n*/) AY8910Read(/*n*/)

/* SSGReset : Reset SSG chip */
/* int n    = chip number   */
#define SSGReset(/*chip*/) AY8910_reset(/*chip*/)

/* --- external callback funstions for realtime update --- */
#if BUILD_YM2610
  /* in 2610intf.c */
//#define YM2610UpdateReq(/*chip*/) YM2610UpdateRequest(/*chip*/);
#define YM2610UpdateReq()
#endif

#if FM_STEREO_MIX
#define YM2151_NUMBUF 1
#define YM2608_NUMBUF 1
#define YM2612_NUMBUF 1
#define YM2610_NUMBUF 1
#else
#define YM2151_NUMBUF 2		/* FM L+R */
#define YM2608_NUMBUF 2		/* FM L+R+ADPCM+RYTHM */
#define YM2610_NUMBUF 2		/* FM L+R+ADPCMA+ADPCMB */
#define YM2612_NUMBUF 2		/* FM L+R */
#endif

#if (FM_OUTPUT_BIT==16)
typedef short FMSAMPLE;
typedef unsigned long FMSAMPLE_MIX;
#endif
#if (FM_OUTPUT_BIT==8)
typedef unsigned char FMSAMPLE;
typedef unsigned short FMSAMPLE_MIX;
#endif

#define TIMER_SH 24

//typedef void (*FM_TIMERHANDLER) (int c, int cnt, double stepTime);
typedef void (*FM_TIMERHANDLER) (int c, int cnt, Uint32 stepTime);
typedef void (*FM_IRQHANDLER) (int n, int irq);
/* FM_TIMERHANDLER : Stop or Start timer         */
/* int n          = chip number                  */
/* int c          = Channel 0=TimerA,1=TimerB    */
/* int count      = timer count (0=stop)         */
/* doube stepTime = step time of one count (sec.)*/

/* FM_IRQHHANDLER : IRQ level changing sense     */
/* int n       = chip number                     */
/* int irq     = IRQ level 0=OFF,1=ON            */

/* ---------- OPN / OPM one channel  ---------- */
typedef struct fm_slot {
    Sint32 *DT;			/* detune          :DT_TABLE[DT]       */
    int DT2;			/* multiple,Detune2:(DT2<<4)|ML for OPM */
    int TL;			/* total level     :TL << 8            */
    Uint8 KSR;			/* key scale rate  :3-KSR              */
    const Sint32 *AR;		/* attack rate     :&AR_TABLE[AR<<1]   */
    const Sint32 *DR;		/* decay rate      :&DR_TABLE[DR<<1]   */
    const Sint32 *SR;		/* sustin rate     :&DR_TABLE[SR<<1]   */
    int SL;			/* sustin level    :SL_TABLE[SL]       */
    const Sint32 *RR;		/* release rate    :&DR_TABLE[RR<<2+2] */
    Uint8 SEG;			/* SSG EG type     :SSGEG              */
    Uint8 ksr;			/* key scale rate  :kcode>>(3-KSR)     */
    Uint32 mul;			/* multiple        :ML_TABLE[ML]       */
    /* Phase Generator */
    Uint32 Cnt;			/* frequency count :                   */
    Uint32 Incr;		/* frequency step  :                   */
    /* Envelope Generator */
    Uint8 state;
    void (*eg_next) (struct fm_slot * SLOT);	/* pointer of phase handler */
    Sint32 evc;			/* envelope counter                    */
    Sint32 eve;			/* envelope counter end point          */
    Sint32 evs;			/* envelope counter step               */
    Sint32 evsa;			/* envelope step for Attack            */
    Sint32 evsd;			/* envelope step for Decay             */
    Sint32 evss;			/* envelope step for Sustain           */
    Sint32 evsr;			/* envelope step for Release           */
    Sint32 TLL;			/* adjusted TotalLevel                 */
    /* LFO */
    Uint8 amon;			/* AMS enable flag              */
    Uint32 ams;			/* AMS depth level of this SLOT */
} FM_SLOT;

typedef struct fm_chan {
    FM_SLOT SLOT[4];
    Uint8 PAN;			/* PAN :NONE,LEFT,RIGHT or CENTER */
    Uint8 ALGO;			/* Algorythm                      */
    Uint8 FB;			/* shift count of self feed back  */
    Sint32 op1_out[2];		/* op1 output for beedback        */
    /* Algorythm (connection) */
    Sint32 *connect1;		/* pointer of SLOT1 output    */
    Sint32 *connect2;		/* pointer of SLOT2 output    */
    Sint32 *connect3;		/* pointer of SLOT3 output    */
    Sint32 *connect4;		/* pointer of SLOT4 output    */
    /* LFO */
    Sint32 pms;			/* PMS depth level of channel */
    Uint32 ams;			/* AMS depth level of channel */
    /* Phase Generator */
    Uint32 fc;			/* fnum,blk    :adjusted to sampling rate */
    Uint8 fn_h;			/* freq latch  :                   */
    Uint8 kcode;		/* key code    :                   */
} FM_CH;

/* OPN/OPM common state */
typedef struct fm_state {
    Uint8 index;		/* chip index (number of chip) */
    int clock;			/* master clock  (Hz)  */
    int rate;			/* sampling rate (Hz)  */
    double freqbase;		/* frequency base      */
    double TimerBase;		/* Timer base time     */
    Uint32 freqbase_f;		/* frequency base    FixedPoint  */
    Uint32 TimerBase_f;		/* Timer base time   FixedPoint  */
    Uint8 address;		/* address register    */
    Uint8 irq;			/* interrupt level     */
    Uint8 irqmask;		/* irq mask            */
    Uint8 status;		/* status flag         */
    Uint32 mode;		/* mode  CSM / 3SLOT   */
    Uint8 prescaler_sel;        /* prescaler selector	*/
    Uint8 fn_h;			/* freq latch			*/
    int TA;			/* timer a             */
    int TAC;			/* timer a counter     */
    Uint8 TB;			/* timer b             */
    int TBC;			/* timer b counter     */
    /* speedup customize */
    /* local time tables */
    Sint32 DT_TABLE[8][32];	/* DeTune tables       */
    Sint32 AR_TABLE[94];		/* Atttack rate tables */
    Sint32 DR_TABLE[94];		/* Decay rate tables   */
    /* Extention Timer and IRQ handler */
    FM_TIMERHANDLER Timer_Handler;
    FM_IRQHANDLER IRQ_Handler;
    /* timer model single / interval */
    Uint8 timermodel;
} FM_ST;

/***********************************************************/
/* OPN unit                                                */
/***********************************************************/

/* OPN 3slot struct */
typedef struct opn_3slot {
    Uint32 fc[3];		/* fnum3,blk3  :calcrated */
    Uint8 fn_h[3];		/* freq3 latch            */
    Uint8 kcode[3];		/* key code    :          */
} FM_3SLOT;

/* OPN/A/B common state */
typedef struct opn_f {
    Uint8 type;			/* chip type         */
    FM_ST ST;			/* general state     */
    FM_3SLOT SL3;		/* 3 slot mode state */
    FM_CH *P_CH;		/* pointer of CH     */
    Uint32 FN_TABLE[2048];	/* fnumber -> increment counter */
#if FM_LFO_SUPPORT
    /* LFO */
    Uint32 LFOCnt;
    Uint32 LFOIncr;
    Uint32 LFO_FREQ[8];		/* LFO FREQ table */
#endif
} FM_OPN;


/* adpcm type A struct */
typedef struct adpcm_state {
    Uint8 flag;			/* port state        */
    Uint8 flagMask;		/* arrived flag mask */
    Uint8 now_data;
    Uint32 now_addr;
    Uint32 now_step;
    Uint32 step;
    Uint32 start;
    Uint32 end;
    int IL;
    int volume;			/* calcrated mixing level */
    Sint32 *pan;			/* &out_ch[OPN_xxxx] */
    int /*adpcmm, */ adpcmx, adpcmd;
    int adpcml;			/* hiro-shi!! */
} ADPCM_CH;

/* here's the virtual YM2610 */
typedef struct ym2610_f {
    Uint8  REGS[512];           /* registers */
    FM_OPN OPN;			/* OPN state    */
    FM_CH CH[6];		/* channel state */
    int address1;		/* address register1 */
    /* ADPCM-A unit */
    Uint8 *pcmbuf;		/* pcm rom buffer */
    Uint32 pcm_size;		/* size of pcm rom */
    Sint32 *adpcmTL;		/* adpcmA total level */
    ADPCM_CH adpcm[6];		/* adpcm channels */
    Uint32 adpcmreg[0x30];	/* registers */
    Uint8 adpcm_arrivedEndAddress;
    /* Delta-T ADPCM unit */
    YM_DELTAT deltaT;
} YM2610;

extern YM2610 FM2610;

#if (BUILD_YM2610||BUILD_YM2610B)
/* -------------------- YM2610(OPNB) Interface -------------------- */
int YM2610Init(int baseclock, int rate,
	       void *pcmroma, int pcmasize, void *pcmromb, int pcmbsize,
	       FM_TIMERHANDLER TimerHandler, FM_IRQHANDLER IRQHandler);
void YM2610Shutdown(void);
void YM2610ResetChip(void);
void YM2610UpdateOne(int num, short **buffer, int length);

int YM2610Write(int a, unsigned char v);
unsigned char YM2610Read(int a);
int YM2610TimerOver(int c);

#endif				/* BUILD_YM2610 */



#endif				/* _H_FM_FM_ */
