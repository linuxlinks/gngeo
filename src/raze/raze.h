/*
 * RAZE-x86 Z80 emulator.
 *
 * Copyright (c) 1999 Richard Mitton
 *
 * This may only be distributed as part of the complete RAZE package.
 * See RAZE.TXT for license information.
 */

#ifndef __RAZE_H_INCLUDED__
#define __RAZE_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

   /* Fix this as you need it */
   #ifndef UBYTE
      #define UBYTE unsigned char
   #endif
   #ifndef UWORD
      #define UWORD unsigned short
   #endif

   /* Memory map constants */
   #define Z80_MAP_DIRECT  0  /* Reads/writes are done directly */
   #define Z80_MAP_HANDLED 1  /* Reads/writes use a function handler */

   /* Z80 registers */
   typedef enum {
      Z80_REG_AF=0,
      Z80_REG_BC,
      Z80_REG_DE,
      Z80_REG_HL,
      Z80_REG_IX,
      Z80_REG_IY,
      Z80_REG_PC,
      Z80_REG_SP,
      Z80_REG_AF2,
      Z80_REG_BC2,
      Z80_REG_DE2,
      Z80_REG_HL2,
      Z80_REG_IFF1,        /* boolean - 1 or 0 */
      Z80_REG_IFF2,        /* boolean - 1 or 0 */
      Z80_REG_IR,
      Z80_REG_IM,          /* 0, 1, or 2 */
      Z80_REG_IRQVector,   /* 0x00 to 0xff */
      Z80_REG_IRQLine      /* boolean - 1 or 0 */
   } z80_register;

   /* Z80 main functions */
   void  z80_reset(void);
   int   z80_emulate(int cycles);
   void  z80_raise_IRQ(UBYTE vector);
   void  z80_lower_IRQ(void);
   void  z80_cause_NMI(void);

   /* Z80 context functions */
   int   z80_get_context_size(void);
   void  z80_set_context(void *context);
   void  z80_get_context(void *context);
   UWORD z80_get_reg(z80_register reg);
   void  z80_set_reg(z80_register reg, UWORD value);

   /* Z80 cycle functions */
   int   z80_get_cycles_elapsed(void);
   void  z80_stop_emulating(void);
   void  z80_skip_idle(void);
   void  z80_do_wait_states(int n);

   /* Z80 I/O functions */
   void  z80_init_memmap(void);
   void  z80_map_fetch(UWORD start, UWORD end, UBYTE *memory);
   void  z80_map_read(UWORD start, UWORD end, UBYTE *memory);
   void  z80_map_write(UWORD start, UWORD end, UBYTE *memory);
   void  z80_add_read(UWORD start, UWORD end, int method, void *data);
   void  z80_add_write(UWORD start, UWORD end, int method, void *data);
   void  z80_set_in(UBYTE (*handler)(UWORD port));
   void  z80_set_out(void (*handler)(UWORD port, UBYTE value));
   void  z80_set_reti(void (*handler)(void));
   void  z80_set_fetch_callback(void (*handler)(UWORD pc));
   void  z80_end_memmap(void);

#ifdef __cplusplus
};
#endif

#endif /* __RAZE_H_INCLUDED__ */

