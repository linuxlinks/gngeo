;----------------------------------------------------------------------------;
; RAZE-x86 Z80 emulator.
; version 1.03
;
; Copyright (c) 1999 Richard Mitton
;
; This may only be distributed as part of the complete RAZE package.
; See RAZE.TXT for license information.
;----------------------------------------------------------------------------;


; You are not expected to understand this.


;----------------------------------------------------------------------------;
; Comment these in/out for faster speed (less accurate):
;%define EMULATE_UNDOC_FLAGS      ; a couple of undocumented flags
;%define EMULATE_BITS_3_5         ; bits 3/5 of the flags (undocumented)
;%define EMULATE_WEIRD_STUFF      ; misc *very obscure* undocumented behaviour
;%define EMULATE_R_REGISTER       ; R register (not usually needed)
;%define USE_FETCH_CALLBACK       ; call a callback for every fetch (slow!)
;----------------------------------------------------------------------------;

; Registers:
; AF = ax (flags in AH)
; HL = bx
; BC = cx
; PC = si
; R = ebp
;
; GCC wants EBX, ESI, EDI, ESP, and EBP to be preserved at all times
; DH should always be clear between macros
; The high 16-bits of all registers should be clear at all times (except EBP)
; This assumes LAHF will set bit 1 of AH. (supposed to be undefined...)

BITS 32
SECTION .bss

; Maximum number of memory handlers
%define MAX_MEM_HANDLERS 64

; The current context
align 4
context_start:
_z80_AF  resd 1
_z80_BC  resd 1
_z80_DE  resd 1
_z80_HL  resd 1
_z80_IX  resd 1
_z80_IY  resd 1
_z80_PC  resd 1
_z80_SP  resd 1
_z80_AF2 resd 1
_z80_BC2 resd 1
_z80_DE2 resd 1
_z80_HL2 resd 1
_z80_IFF1   resd 1
_z80_IFF2   resd 1
_z80_R      resd 1
_z80_R2     resd 1
_z80_I      resb 1
_z80_flag35 resb 1
_z80_IM     resb 1
_z80_IRQVector resb 1
_z80_IRQLine   resd 1
_z80_Extra_Cycles resd 1
registers_end:

_z80_Fetch  resd 256
_z80_Read   resd (256*2)
_z80_Write  resd (256*2)
_z80_In     resd 1
_z80_Out    resd 1
_z80_RetI   resd 1
_z80_Fetch_Callback  resd 1
_read_handlers       resd 4*MAX_MEM_HANDLERS
_write_handlers      resd 4*MAX_MEM_HANDLERS
context_end:
resd 1     ; safety gap, so z80_reset can use 32-bit transfers

; Variables
_z80_ICount          resd 1
_z80_Initial_ICount  resd 1
_z80_TempICount      resd 1
_z80_afterEI         resb 1

SECTION .text
;----------------------------------------------------------------------------;
; Prepare to call a GCC function
%macro CALLGCC_START 0
   mov [_z80_AF], ezAF        ; eax
   mov [_z80_BC], ezBC        ; ecx
   mov [_z80_PC], ezPC        ; esi
   mov [_z80_R],  zR          ; ebp
%endmacro

; Return from a GCC function
%macro CALLGCC_END 0
   mov ezAF, [_z80_AF]        ; eax
   mov ezBC, [_z80_BC]        ; ecx
   mov ezPC, [_z80_PC]        ; esi
   mov zR,   [_z80_R]         ; ebp
%endmacro

; Read a byte from the PC, into DL
; GETBYTE
%macro GETBYTE 0
%ifdef USE_FETCH_CALLBACK
   CALLGCC_START
   push ezPC
   call [_z80_Fetch_Callback]
   add esp, 4
   CALLGCC_END
%endif
   mov edx, ezPC
   shr edx, 8
   mov edi, [_z80_Fetch+edx*4]

   mov dl, [edi+ezPC]
   inc zPC
   xor edi, edi
%endmacro

; Read a byte from the PC, sign-extended into DI
; GETDISP
%macro GETDISP 0
%ifdef USE_FETCH_CALLBACK
   CALLGCC_START
   push ezPC
   call [_z80_Fetch_Callback]
   add esp, 4
   CALLGCC_END
%endif
   mov edx, ezPC
   shr edx, 8
   mov edx, [_z80_Fetch+edx*4]

   movsx di, [edx+ezPC]
   inc zPC
   xor edx, edx
%endmacro

; Read a word from the PC, into DI
; GETWORD
%macro GETWORD 0
%ifdef USE_FETCH_CALLBACK
   CALLGCC_START
   push ezPC
   call [_z80_Fetch_Callback]
   add esp, 4
   CALLGCC_END
%endif
   mov [_z80_BC], ezBC
   mov edx, ezPC           ; low byte
   shr edx, 8
   mov edi, [_z80_Fetch+edx*4]

   mov dl, [edi+ezPC]
   inc zPC
   
%ifdef USE_FETCH_CALLBACK
   push edx
   CALLGCC_START
   push ezPC
   call [_z80_Fetch_Callback]
   add esp, 4
   CALLGCC_END
   pop edx
%endif
   mov ecx, ezPC           ; high byte
   shr ecx, 8
   mov edi, [_z80_Fetch+ecx*4]

   mov dh, [edi+ezPC]
   inc zPC
   mov edi, edx
   mov ezBC, [_z80_BC]
   xor edx, edx
%endmacro

; Read a byte from memory (DI) into DL
; MEMREAD
%macro MEMREAD 0
   CALLGCC_START
   mov edx, edi
   shr edx, 8
   mov ecx, [_z80_Read+edx*8]

   test ecx, ecx     ; if handler exists, call it
   jnz %%handler
                     ; no handler, so go direct
   mov ecx, [_z80_Read+edx*8+4]
   mov dl, [ecx+edi]
   jmp %%finish

%%handler:
   push edi
   call ecx
   xor edx, edx
   add esp, 4
   mov dl, al
%%finish:
   CALLGCC_END
%endmacro

; Write a byte in DL to memory (DI)
%macro MEMWRITE 0
   CALLGCC_START
   xor eax, eax
   mov al, dl

   mov edx, edi
   shr edx, 8
   mov ecx, [_z80_Write+edx*8]

   test ecx, ecx        ; if handler exists, call it
   jnz %%handler
                        ; no handler, so go direct
   mov ecx, [_z80_Write+edx*8+4]
   mov [ecx+edi], al
   jmp %%finish

%%handler:
   push eax
   push edi
   call ecx
   add esp, 8
%%finish:
   CALLGCC_END
   xor edx, edx
%endmacro

; Read a byte from port (DI) into DL
; IOREAD
%macro IOREAD 0
   push edi
   CALLGCC_START
   call [_z80_In]
   xor edx, edx
   add esp, 4
   mov dl, al
   CALLGCC_END
%endmacro

; Write a byte in DL to port (DI)
%macro IOWRITE 0
   CALLGCC_START
   xor dh, dh
   push edx
   push edi
   call [_z80_Out]
   add esp, 8
   CALLGCC_END
   xor edx, edx
%endmacro

; Align code to the default boundary
%macro ALIGN 0
   align 4
%endmacro

; Define an instruction
; DEF prefix, opcode, std_cycles, extra_cycles
%macro DEF 4
   ALIGN
   %define std_cycles %3
   %define extra_cycles %4
%{1}_%2:
%endmacro

; Subtract the clock cycles for this instruction from the ICount
%macro DO_CYCLES 0
   sub dword [_z80_ICount], std_cycles
   jbe near z80_finish
%endmacro

; Subtract the extra clock cycles for this instruction from the ICount
%macro DO_EXTRA_CYCLES 0
   sub dword [_z80_ICount], extra_cycles
   jbe near z80_finish
%endmacro

; Finish straight away
%macro EXIT 0
   jmp z80_finish
%endmacro

; Jump to the next instruction
%macro NEXT 0
%ifdef EMULATE_R_REGISTER
   inc zR
%endif
   GETBYTE
   jmp [OpTable+edx*4]
%endmacro

; Check the IRQ line, cause an interrupt if needed
%macro CHECK_IRQ 0
   cmp [_z80_IRQLine], byte 0    ; check the IRQ line
   je near %%finish
   cmp [_z80_IFF1], byte 0       ; check ints are enabled
   je near %%finish

   ; disable interrupts
   mov [_z80_IFF1], byte 0
   mov [_z80_IFF2], byte 0
%ifdef EMULATE_R_REGISTER
   inc zR
%endif

   GETBYTE
   cmp dl, 76h          ; if we're HALTed, skip the HALT
   je %%skip_halt
   dec zPC
%%skip_halt:

   mov edx, ezPC        ; write the PC
   dec zSP

   xchg dl, dh
   mov edi, ezSP
   MEMWRITE
   dec zSP
   mov edx, ezPC
   mov edi, ezSP
   MEMWRITE

   cmp [_z80_IM], byte 0
   jne %%not_im0
   ; IM 0, 13 T-states
   add [_z80_Extra_Cycles], dword 13
   mov dl, [_z80_IRQVector]
   sub dl, 0c7h
   movzx ezPC, dl
   jmp %%finish

%%not_im0:
   cmp [_z80_IM], byte 1
   jne %%not_im1
   ; IM 1, 13 T-states
   add [_z80_Extra_Cycles], dword 13
   mov ezPC, 38h
   jmp %%finish

%%not_im1:
   ; IM 2, 19 T-states
   add [_z80_Extra_Cycles], dword 19
   mov dl, [_z80_IRQVector]
   mov dh, zI
   mov edi, edx
   MEMREAD
   inc di
   mov ezPC, edx
   MEMREAD
   xchg dl, dh
   or ezPC, edx
   xor dh, dh
%%finish:
%endmacro

;----------------------------------------------------------------------------;
; Helper functions

; Add A, %1
%macro DO_ADD 1
   add zA, %1
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zA
   %endif
   lahf
   jo %%set_overflow

   and zF, FLAG_S|FLAG_Z|FLAG_H|FLAG_C
   jmp %%finish

 %%set_overflow:
   and zF, FLAG_S|FLAG_Z|FLAG_H|FLAG_C
   or zF, FLAG_P
 %%finish:
%endmacro

; Sub A, %1
%macro DO_SUB 1
   sub zA, %1
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zA
   %endif
   lahf
   jo %%set_overflow

   and zF, FLAG_S|FLAG_Z|FLAG_H|FLAG_C|FLAG_N
   jmp %%finish

 %%set_overflow:
   or zF, FLAG_P
 %%finish:
%endmacro

; Adc A, %1
%macro DO_ADC 1
   ror zF, 1
   adc zA, %1
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zA
   %endif
   lahf
   jo %%set_overflow

   and zF, FLAG_S|FLAG_Z|FLAG_H|FLAG_C
   jmp %%finish

 %%set_overflow:
   and zF, FLAG_S|FLAG_Z|FLAG_H|FLAG_C
   or zF, FLAG_P
 %%finish:
%endmacro

; Sbc A, %1
%macro DO_SBC 1
   ror zF, 1
   sbb zA, %1
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zA
   %endif
   lahf
   jo %%set_overflow

   and zF, FLAG_S|FLAG_Z|FLAG_H|FLAG_C|FLAG_N
   jmp %%finish

 %%set_overflow:
   or zF, FLAG_P
 %%finish:
%endmacro

; And A, %1
%macro DO_AND 1
   and zA, %1
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zA
   %endif
   lahf
   and zF, ~(FLAG_N|FLAG_C)      ; clear N and carry
   or  zF, FLAG_H                ; set half-carry
%endmacro

; Or A, %1
%macro DO_OR 1
   or zA, %1
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zA
   %endif
   lahf
   and zF, ~(FLAG_N|FLAG_C|FLAG_H)  ; clear N, carry and half-carry
%endmacro

; Xor A, %1
%macro DO_XOR 1
   xor zA, %1
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zA
   %endif
   lahf
   and zF, ~(FLAG_N|FLAG_C|FLAG_H)  ; clear N, carry and half-carry
%endmacro

; Cp A, %1
%macro DO_CP 1
   %ifdef EMULATE_BITS_3_5
      mov ah, %1
      mov [_z80_flag35], ah
   %endif
   cmp zA, %1  
   lahf
   jo %%overflow

   and zF, ~FLAG_P              ; clear the overflow
   or  zF, FLAG_N               ; set the N flag
   jmp %%finish

%%overflow: 
   or  zF, FLAG_P|FLAG_N          ; set the overflow and N flag
%%finish:
%endmacro

; Inc %1
%macro DO_INC 1
   and zF, FLAG_C      ; leave carry alone
   %ifnidn %1, dl
      mov dl, %1
   %endif
   or  zF, [INC_Table+edx]
   inc %1
   %ifdef EMULATE_BITS_3_5
      %ifnidn %1, dl
         inc dl
      %endif
      mov [_z80_flag35], dl
   %endif
%endmacro

; Dec %1
%macro DO_DEC 1
   and zF, FLAG_C      ; leave carry alone
   %ifnidn %1, dl
      mov dl, %1
   %endif
   or  zF, [DEC_Table+edx]
   dec %1
   %ifdef EMULATE_BITS_3_5
      %ifnidn %1, dl
         dec dl
      %endif
      mov [_z80_flag35], dl
   %endif
%endmacro

; Add16 %1, %2
%macro DO_ADD16 2
   %ifidn %2, dx
      %error "DX used as parameter for DO_ADD16!"
   %endif
   mov edi, e%2
   %ifdef EMULATE_UNDOC_FLAGS
      mov edx, e%1
   %endif
   and zF, FLAG_S|FLAG_Z|FLAG_P

   add %1, di     ; do it
   adc zF, 0      ; merge the new carry in

   %ifdef EMULATE_UNDOC_FLAGS
      xor edx, e%1     ; work out our new half-carry
      xor edx, edi
      and dh, FLAG_H
      or zF, dh
   %endif

   %ifdef EMULATE_BITS_3_5
      mov dl, h%1
      mov [_z80_flag35], dl
   %endif
   %ifdef EMULATE_UNDOC_FLAGS
      xor dh, dh
   %endif
%endmacro

; Adc16 %1, %2
%macro DO_ADC16 2
   %ifidn %2, dx
      %error "DX used as parameter for DO_ADC16!"
   %endif                                     
   mov edi, e%2
   %ifdef EMULATE_UNDOC_FLAGS
      mov edx, e%1
   %endif
   ror zF, 1      ; load the carry
   adc %1, %2
   lahf
   jo %%overflow

   and zF, ~(FLAG_N|FLAG_H|FLAG_P)      ; clear N,H and overflow
   jmp %%finish

%%overflow:
   and zF, ~(FLAG_N|FLAG_H)   ; clear N and H
   or  zF,  FLAG_P            ; set overflow
%%finish:

   %ifdef EMULATE_UNDOC_FLAGS
      xor edx, e%1     ; work out our new half-carry
      xor edx, edi
      and dh, FLAG_H
      or zF, dh
   %endif

   %ifdef EMULATE_BITS_3_5
      mov dl, h%1
      mov [_z80_flag35], dl
   %endif
   %ifdef EMULATE_UNDOC_FLAGS
      xor dh, dh
   %endif
%endmacro

; Sbc16 %1, %2
%macro DO_SBC16 2
%ifidn %2, dx
   %error "DX used as parameter for DO_SBC16!"
%endif                                     
   %ifdef EMULATE_UNDOC_FLAGS
      mov edi, e%2
   %endif
   mov edx, e%1
   ror zF, 1      ; load the carry
   sbb %1, %2
   lahf
   jo %%overflow

   and zF, ~(FLAG_P|FLAG_H)   ; clear H and overflow
   or  zF,  FLAG_N            ; set N
   jmp %%finish

%%overflow:
   or zF, FLAG_N|FLAG_P ; set overflow and N
   and zF, ~FLAG_H      ; clear H
%%finish:

   %ifdef EMULATE_UNDOC_FLAGS
      xor edx, e%1     ; work out our new half-carry
      xor edx, edi
      and dh, FLAG_H
      or zF, dh
   %endif

   %ifdef EMULATE_BITS_3_5
      mov dl, h%1
      mov [_z80_flag35], dl
   %endif
   %ifdef EMULATE_UNDOC_FLAGS
      xor dh, dh
   %endif
%endmacro

; Rotate method, reg8
%macro DO_ROT 2
   %ifidn %1, RLA
      mov dl, zF
      and dl, FLAG_S|FLAG_Z|FLAG_P
      sahf
      rcl zA, 1
      adc dl, 0
      %ifdef EMULATE_BITS_3_5
         mov [_z80_flag35], zA
      %endif
      mov zF, dl
   %elifidn %1, RRA
      mov dl, zF
      and dl, FLAG_S|FLAG_Z|FLAG_P
      sahf
      rcr zA, 1
      adc dl, 0
      %ifdef EMULATE_BITS_3_5
         mov [_z80_flag35], zA
      %endif
      mov zF, dl
   %elifidn %1, RLCA
      and zF, FLAG_S|FLAG_Z|FLAG_P
      rol zA, 1
      adc zF, 0
      %ifdef EMULATE_BITS_3_5
         mov [_z80_flag35], zA
      %endif
   %elifidn %1, RRCA
      and zF, FLAG_S|FLAG_Z|FLAG_P
      ror zA, 1
      adc zF, 0
      %ifdef EMULATE_BITS_3_5
         mov [_z80_flag35], zA
      %endif
   %else
      %ifnidn %2, dl
         mov dl, %2
      %endif

      %ifidn %1, RLC
         rol dl, 1
      %elifidn %1, RRC
         ror dl, 1
      %elifidn %1, RL
         sahf
         rcl dl, 1
      %elifidn %1, RR
         sahf
         rcr dl, 1
      %endif

      adc dh, dh
      test dl, dl
      lahf
      and zF, FLAG_S|FLAG_Z|FLAG_P
      or zF, dh
      %ifdef EMULATE_BITS_3_5
         mov [_z80_flag35], dl
      %endif
      %ifnidn %2, dl
         mov %2, dl
      %endif
      xor dh, dh
   %endif
%endmacro

; Shift method, reg8
%macro DO_SHF 2
   %ifnidn %2, dl
      mov dl, %2
   %endif

   %ifidn %1, SLA
      add dl, dl
      adc dh, dh
   %elifidn %1, SLL
      add dl, dl
      adc dh, dh
      inc dl
   %elifidn %1, SRA
      sar dl, 1
      adc dh, dh
   %elifidn %1, SRL
      shr dl, 1
      adc dh, dh
   %endif

   test dl, dl
   lahf
   and zF, FLAG_S|FLAG_Z|FLAG_P
   or zF, dh
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], dl
   %endif
   %ifnidn %2, dl
      mov %2, dl
   %endif
   xor dh, dh
%endmacro

; BIT r,n      n = bit (0-7)
;              r = 8-bit register
%macro DO_BIT 2
   and zF, FLAG_C           
   test %1, (1 << %2)
   jz %%bit_is_clear
   %ifidn %2, 7
      or zF, FLAG_H|FLAG_S       ; set sign bit, if testing bit 7
   %elifidn %2, 5                   
      or zF, FLAG_H|FLAG_5       ; set bit 5, if testing bit 5
   %elifidn %2, 3
      or zF, FLAG_H|FLAG_3       ; set bit 3, if testing bit 3
   %else
      or zF, FLAG_H              ; set half-carry, and parity
   %endif
   jmp %%finish

%%bit_is_clear:
   or zF, FLAG_H|FLAG_Z|FLAG_P   ; set half-carry, parity, and zero
%%finish:
%endmacro

; SET r,n      n = bit (0-7)
;              r = 8-bit register
%macro DO_SET 2
   or %1, (1 << %2)
%endmacro

; RES r,n      n = bit (0-7)
;              r = 8-bit register
%macro DO_RES 2
   and %1, ~(1 << %2)
%endmacro

;----------------------------------------------------------------------------;
; Here are the interface functions:

; Move a register to another
; LD_R_R d8, s8      s8 = src reg, d8 = dest reg
%macro LD_R_R 2
%ifnidn %1, %2
   %ifidn %2, D
      mov dl, zD
      mov z%1, dl
   %elifidn %2, E
      mov dl, zE
      mov z%1, dl
   %else
      mov z%1, z%2
   %endif
%endif
   DO_CYCLES
   NEXT
%endmacro

; Move a register to another (IXh/IXl)
; LD_XYr_XYr d8, s8      s8 = src reg, d8 = dest reg
%macro LD_XYr_XYr 2
%ifnidn %1, %2
   mov dl, z%2
   mov z%1, dl
%endif
   DO_CYCLES
   NEXT
%endmacro

; Move an immediate byte into a register
; LD_R_N d8      d8 = dest reg
%macro LD_R_N 1
   GETBYTE
   mov z%1, dl
   DO_CYCLES
   NEXT
%endmacro

; Move from (RR) into a register
; LD_R_mRR d8, s16      d8 = dest reg, s16 = src reg
%macro LD_R_mRR 2
   mov edi, ez%2
   MEMREAD
   mov z%1, dl
   DO_CYCLES
   NEXT
%endmacro

; Move from (XY+dd) into a register
; LD_R_mXY d8, s16      d8 = dest reg, s16 = src reg
%macro LD_R_mXY 2
   GETDISP
   add di, z%2
   MEMREAD
   mov z%1, dl
   DO_CYCLES
   NEXT
%endmacro

; Move from a register into (RR)
; LD_mRR_R d16, s8      d16 = destreg, s8 = src reg
%macro LD_mRR_R 2
   mov edi, ez%1
   mov dl, z%2
   MEMWRITE
   DO_CYCLES
   NEXT
%endmacro

; Move from a register into (XY+dd)
; LD_mXY_R d16, s8      d16 = dest reg, s8 = src reg
%macro LD_mXY_R 2
   GETDISP
   add di, z%1
   mov dl, z%2
   MEMWRITE
   DO_CYCLES
   NEXT
%endmacro

; Move an immediate into (RR)
; LD_mRR_N d16          d16 = dest reg
%macro LD_mRR_N 1
   GETBYTE
   mov edi, ez%1
   MEMWRITE
   DO_CYCLES
   NEXT
%endmacro

; Move an immediate into (XY+dd)
; LD_mXY_N d16          d16 = dest reg
%macro LD_mXY_N 1
   GETDISP
   add di, z%1
   push edi
   GETBYTE
   pop edi
   MEMWRITE
   DO_CYCLES
   NEXT
%endmacro

; Move from (NN) into a register
; LD_R_mNN d8          d8 = dest reg
%macro LD_R_mNN 1
   GETWORD
   MEMREAD
   mov z%1, dl
   DO_CYCLES
   NEXT
%endmacro

; Move from a register into (NN)
; LD_mNN_R s8          s8 = src reg
%macro LD_mNN_R 1
   GETWORD
   mov dl, z%1
   MEMWRITE
   DO_CYCLES
   NEXT
%endmacro

; Move the Interrupt register into a register
; LD_R_I d8             d8 = dest reg
%macro LD_R_I 1
   mov z%1, zI
   and zF, FLAG_C       ; keep old carry
   mov dl, zF

   test z%1, z%1
	lahf
   and zF, FLAG_S|FLAG_Z   ; set S/Z from the I register
   or zF, dl
   or zF, [_z80_IFF2]
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], z%1
   %endif
   DO_CYCLES
   NEXT
%endmacro

; Move a register into the Interrupt register
; LD_I_R s8             s8 = src reg
%macro LD_I_R 1
   mov zI, z%1
   DO_CYCLES
   NEXT
%endmacro

; Move the Refresh register into a register
; LD_R_Rf d8             d8 = dest reg
%macro LD_R_Rf 1
   mov edx, [_z80_R2]
   and edx, 80h         ; take bits 0-6 from R, bit 7 from R2

   %ifndef EMULATE_R_REGISTER
      ; attempt to fake the R-register
      ; take the ICount, invert it, divide by 4
      ; this produces an increasing counter, roughly proportional to
      ; the number of instructions done
      mov zR, [_z80_ICount]
      not zR
      shr zR, 2
   %endif

   and zR, 7fh
   or edx, zR
   mov z%1, dl

   mov dl, zF
   and dl, FLAG_C       ; keep the old carry

   test z%1, z%1
	lahf
   and zF, FLAG_S|FLAG_Z
   or zF, dl
   or zF, [_z80_IFF2]
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], z%1
   %endif
   DO_CYCLES
   NEXT
%endmacro

; Move a register into the Refresh register
; LD_Rf_R s8             s8 = src reg
%macro LD_Rf_R 1
   mov dl, z%1
   mov zR, edx
   mov [_z80_R2], edx
   DO_CYCLES
   NEXT
%endmacro

; Move a 16-bit immediate into a register
; LD_RR_NN d16             d16 = dest reg
%macro LD_RR_NN 1
   GETWORD
   mov ez%1, edi
   DO_CYCLES
   NEXT
%endmacro

; Move from (NN) into a 16-bit register
; LD_RR_mNN d16             d16 = dest reg
%macro LD_RR_mNN 1
   GETWORD
   MEMREAD
   mov lz%1, dl
   inc di
   MEMREAD
   mov hz%1, dl
   DO_CYCLES
   NEXT
%endmacro

; Move from a 16-bit register into (NN)
; LD_mNN_RR s16             s16 = src reg
%macro LD_mNN_RR 1
   GETWORD
   mov dl, lz%1
   MEMWRITE
   inc di
   mov dl, hz%1
   MEMWRITE
   DO_CYCLES
   NEXT
%endmacro

; Move from a 16-bit register into another
; LD_RR_RR d16, s16             s16 = src reg, d16 = dest reg
%macro LD_RR_RR 2
   mov ez%1, ez%2
   DO_CYCLES
   NEXT
%endmacro

; Move from a 16-bit register into another
; LD_RR_XY d16, s16             s16 = src reg, d16 = dest reg
%macro LD_RR_XY 2
   mov edi, ez%2
   mov ez%1, edi
   DO_CYCLES
   NEXT
%endmacro

; Push a word on the stack
; PUSH_RR      s16               s16 = src reg
%macro PUSH_RR 1
   mov dl, hz%1
   dec zSP
   mov edi, ezSP
   MEMWRITE
   dec zSP
   mov edi, ezSP
   %ifidn %1, AF
      ; merge bits 3 and 5 in
      and zF, ~(FLAG_3|FLAG_5)
      mov dl, [_z80_flag35]
      and dl, FLAG_3|FLAG_5
      or zF, dl
   %endif
   mov dl, lz%1
   MEMWRITE
   DO_CYCLES
   NEXT
%endmacro

; Pop a word from the stack
; POP_RR      s16               s16 = src reg
%macro POP_RR 1
   mov edi, ezSP
   MEMREAD
   %ifidn %1, AF
      mov [_z80_flag35], dl      ; take out bits 3/5
   %endif
   mov lz%1, dl
   inc zSP
   mov edi, ezSP
   MEMREAD
   mov hz%1, dl
   inc zSP
   DO_CYCLES
   NEXT
%endmacro

; Exchange 2 16-bit registers
; EX_RR_RR      s16, d16      s16 = src reg, d16 = dest reg
%macro EX_RR_RR 2
   xchg ez%1, ez%2
   DO_CYCLES
   NEXT
%endmacro

; Exchange AF with AF'
; EX_AF_AF2
%macro EX_AF_AF2 0
   ; Merge bits 3+5 of the flags in first
   and zF, ~(FLAG_3|FLAG_5)
   mov dl, [_z80_flag35]
   and dl, FLAG_3|FLAG_5
   or zF, dl
   xchg ezAF, ezAF2

   ; Un-merge the flags again
   mov [_z80_flag35], zF
   DO_CYCLES
   NEXT
%endmacro

; Exchange the 16-bit registers with their primes
; EXX
%macro EXX 0
   mov edi, ezDE
   xchg ezBC, ezBC2
   xchg ezHL, ezHL2
   xchg edi, ezDE2
   mov ezDE, edi
   DO_CYCLES
   NEXT
%endmacro

; Exchange a 16-bit register with (RR)
; EX_RR_mRR     d16, m16      d16 = dest reg, m16 = memory reg
%macro EX_RR_mRR 2
   mov edi, ez%2     ; swap low byte
   MEMREAD
   xchg dl, lz%1
   MEMWRITE

   inc di            ; swap high byte
   MEMREAD
   xchg dl, hz%1
   MEMWRITE

   DO_CYCLES
   NEXT
%endmacro

; Move from (HL) to (DE), post-increment
; LDI
%macro LDI 0
   mov edi, ezHL
   MEMREAD
   %ifdef EMULATE_BITS_3_5
      push edx
   %endif
   mov edi, ezDE
   MEMWRITE

   %ifdef EMULATE_BITS_3_5
      pop edx
      add dl, zA
      mov dh, dl
      shl dl, 4
      and edx, FLAG_5 | (FLAG_3 << 8)
      or dl, dh
      mov [_z80_flag35], dl
      xor dh, dh
   %endif

   and zF, FLAG_S|FLAG_Z|FLAG_C
   inc zHL
   inc zDE
   dec zBC
   jz %%ldi_zero
   or zF, FLAG_P
%%ldi_zero:     
   DO_CYCLES
   NEXT
%endmacro

; Move from (HL) to (DE), post-decrement
; LDD
%macro LDD 0
   mov edi, ezHL
   MEMREAD
   %ifdef EMULATE_BITS_3_5
      push edx
   %endif
   mov edi, ezDE
   MEMWRITE

   %ifdef EMULATE_BITS_3_5
      pop edx
      add dl, zA
      mov dh, dl
      shl dl, 4
      and edx, FLAG_5 | (FLAG_3 << 8)
      or dl, dh
      mov [_z80_flag35], dl
      xor dh, dh
   %endif

   and zF, FLAG_S|FLAG_Z|FLAG_C
   dec zHL
   dec zDE
   dec zBC
   jz %%ldd_zero
   or zF, FLAG_P
%%ldd_zero:     
   DO_CYCLES
   NEXT
%endmacro

; Move from (HL) to (DE), post-increment, repeat
; LDIR
%macro LDIR 0
%%ldir_loop:
   mov edi, ezHL
   MEMREAD
   %ifdef EMULATE_BITS_3_5
      push edx
   %endif
   mov edi, ezDE
   MEMWRITE
   %ifdef EMULATE_BITS_3_5
      pop edx
   %endif

   inc zHL
   inc zDE
   dec zBC
   jz %%ldir_zero

%ifdef EMULATE_R_REGISTER
   inc zR
%endif
   sub [_z80_ICount], dword extra_cycles
   ja near %%ldir_loop

   and zF, FLAG_S|FLAG_Z|FLAG_C
   %ifdef EMULATE_BITS_3_5
      add dl, zA
      mov dh, dl
      shl dl, 4
      and edx, FLAG_5 | (FLAG_3 << 8)
      or dl, dh
      mov [_z80_flag35], dl
      xor dh, dh
   %endif
   or zF, FLAG_P
   sub zPC, 2           ; rewind to instruction start
   EXIT

%%ldir_zero:
   and zF, FLAG_S|FLAG_Z|FLAG_C
   %ifdef EMULATE_BITS_3_5
      add dl, zA
      mov dh, dl
      shl dl, 4
      and edx, FLAG_5 | (FLAG_3 << 8)
      or dl, dh
      mov [_z80_flag35], dl
      xor dh, dh
   %endif
   DO_CYCLES
   NEXT
%endmacro

; Move from (HL) to (DE), post-decrement, repeat
; LDDR
%macro LDDR 0
%%lddr_loop:
   mov edi, ezHL
   MEMREAD
   %ifdef EMULATE_BITS_3_5
      push edx
   %endif
   mov edi, ezDE
   MEMWRITE
   %ifdef EMULATE_BITS_3_5
      pop edx
   %endif

   dec zHL
   dec zDE
   dec zBC
   jz %%lddr_zero

%ifdef EMULATE_R_REGISTER
   inc zR
%endif
   sub [_z80_ICount], dword extra_cycles
   ja near %%lddr_loop

   and zF, FLAG_S|FLAG_Z|FLAG_C
   %ifdef EMULATE_BITS_3_5
      add dl, zA
      mov dh, dl
      shl dl, 4
      and edx, FLAG_5 | (FLAG_3 << 8)
      or dl, dh
      mov [_z80_flag35], dl
      xor dh, dh
   %endif
   or zF, FLAG_P
   sub zPC, 2           ; rewind to instruction start
   EXIT

%%lddr_zero:
   and zF, FLAG_S|FLAG_Z|FLAG_C
   %ifdef EMULATE_BITS_3_5
      add dl, zA
      mov dh, dl
      shl dl, 4
      and edx, FLAG_5 | (FLAG_3 << 8)
      or dl, dh
      mov [_z80_flag35], dl
      xor dh, dh
   %endif
   DO_CYCLES
   NEXT
%endmacro

; Compare A against (HL), post-increment
; CPI
%macro CPI 0
   mov edi, ezHL
   MEMREAD

   and zF, FLAG_C       ; keep the old carry
   mov dh, zF
   cmp zA, dl          
   lahf
   and zF, FLAG_S|FLAG_Z|FLAG_H|FLAG_N
   or zF, dh            ; SZHN from compare, C unchanged, rest are zero

   inc zHL
   dec zBC
   jz %%cpi_zero
   or zF, FLAG_P        ; set P if BC != 0
%%cpi_zero:

   %ifdef EMULATE_BITS_3_5
      mov dh, zA
      sub dh, dl
      test zF, FLAG_H
      jz %%cpi_hzero
      dec dh
   %%cpi_hzero:

      mov dl, dh
      shl dl, 4
      and edx, FLAG_5 | (FLAG_3 << 8)
      or dl, dh
      mov [_z80_flag35], dl
   %endif
   xor dh, dh

   DO_CYCLES
   NEXT
%endmacro

; Compare A against (HL), post-decrement
; CPD
%macro CPD 0
   mov edi, ezHL
   MEMREAD

   and zF, FLAG_C       ; keep the old carry
   mov dh, zF
   cmp zA, dl          
   lahf
   and zF, FLAG_S|FLAG_Z|FLAG_H|FLAG_N
   or zF, dh            ; SZHN from compare, C unchanged, rest are zero

   dec zHL
   dec zBC
   jz %%cpd_zero
   or zF, FLAG_P        ; set P if BC != 0
%%cpd_zero:

   %ifdef EMULATE_BITS_3_5
      mov dh, zA
      sub dh, dl
      test zF, FLAG_H
      jz %%cpd_hzero
      dec dh
   %%cpd_hzero:
      
      mov dl, dh
      shl dl, 4
      and edx, FLAG_5 | (FLAG_3 << 8)
      or dl, dh
      mov [_z80_flag35], dl
   %endif
   xor dh, dh

   DO_CYCLES
   NEXT
%endmacro

; Compare A against (HL), post-increment, repeat
; CPIR
%macro CPIR 0
   and zF, FLAG_C       ; keep the old carry
%%cpir_loop:
   mov edi, ezHL
   MEMREAD

   inc zHL
   dec zBC
   jz %%cpir_end_bc       ; end due to BC=0

%ifdef EMULATE_R_REGISTER
   inc zR
%endif
   cmp zA, dl
   je %%cpir_end_equal    ; end due to A=(HL)

   sub [_z80_ICount], dword extra_cycles
   ja %%cpir_loop

   ; out of cycles, but not finished
   mov dh, zF
   cmp zA, dl
   lahf
   and zF, FLAG_S|FLAG_Z|FLAG_H|FLAG_N
   or zF, dh            ; SZHN from compare, C unchanged, rest are zero

   %ifdef EMULATE_BITS_3_5
      mov dh, zA
      sub dh, dl
      test zF, FLAG_H
      jz %%cpir_hzero
      dec dh
   %%cpir_hzero:

      mov dl, dh
      shl dl, 4
      and edx, FLAG_5 | (FLAG_3 << 8)
      or dl, dh
      mov [_z80_flag35], dl
   %endif
   or zF, FLAG_P        ; set P to indicate not finished
   sub zPC, 2           ; rewind to instruction start
   xor dh, dh
   EXIT

%%cpir_end_equal:
   or zF, FLAG_P        ; set P to indicate not finished
%%cpir_end_bc:
   mov dh, zF
   cmp zA, dl
   lahf
   and zF, FLAG_S|FLAG_Z|FLAG_H|FLAG_N
   or zF, dh            ; SZHN from compare, C unchanged, rest are zero

   %ifdef EMULATE_BITS_3_5
      mov dh, zA
      sub dh, dl
      test zF, FLAG_H
      jz %%cpir_hzero2
      dec dh
   %%cpir_hzero2:

      mov dl, dh
      shl dl, 4
      and edx, FLAG_5 | (FLAG_3 << 8)
      or dl, dh
      mov [_z80_flag35], dl
   %endif
   xor dh, dh
   DO_CYCLES
   NEXT
%endmacro

; Compare A against (HL), post-decrement, repeat
; CPDR
%macro CPDR 0
   and zF, FLAG_C       ; keep the old carry
%%cpdr_loop:
   mov edi, ezHL
   MEMREAD

   dec zHL
   dec zBC
   jz %%cpdr_end_bc       ; end due to BC=0

%ifdef EMULATE_R_REGISTER
   inc zR
%endif
   cmp zA, dl
   je %%cpdr_end_equal    ; end due to A=(HL)

   sub [_z80_ICount], dword extra_cycles
   ja %%cpdr_loop

   ; out of cycles, but not finished
   mov dh, zF
   cmp zA, dl
   lahf
   and zF, FLAG_S|FLAG_Z|FLAG_H|FLAG_N
   or zF, dh            ; SZHN from compare, C unchanged, rest are zero

   %ifdef EMULATE_BITS_3_5
      mov dh, zA
      sub dh, dl
      test zF, FLAG_H
      jz %%cpdr_hzero
      dec dh
   %%cpdr_hzero:

      mov dl, dh
      shl dl, 4
      and edx, FLAG_5 | (FLAG_3 << 8)
      or dl, dh
      mov [_z80_flag35], dl
   %endif
   or zF, FLAG_P        ; set P to indicate not finished
   sub zPC, 2           ; rewind to instruction start
   xor dh, dh
   EXIT

%%cpdr_end_equal:
   or zF, FLAG_P        ; set P to indicate not finished
%%cpdr_end_bc:
   mov dh, zF
   cmp zA, dl
   lahf
   and zF, FLAG_S|FLAG_Z|FLAG_H|FLAG_N
   or zF, dh            ; SZHN from compare, C unchanged, rest are zero

   %ifdef EMULATE_BITS_3_5
      mov dh, zA
      sub dh, dl
      test zF, FLAG_H
      jz %%cpdr_hzero2
      dec dh
   %%cpdr_hzero2:

      mov dl, dh
      shl dl, 4
      and edx, FLAG_5 | (FLAG_3 << 8)
      or dl, dh
      mov [_z80_flag35], dl
   %endif
   xor dh, dh
   DO_CYCLES
   NEXT
%endmacro

; Perform arithmetic on the A register
; ART_R method, src8
%macro ART_R 2
   %ifidn %1, ADD
      DO_ADD z%2
   %elifidn %1, ADC
      DO_ADC z%2
   %elifidn %1, SUB
      DO_SUB z%2
   %elifidn %1, SBC
      DO_SBC z%2
   %endif

   DO_CYCLES
   NEXT
%endmacro

; Perform arithmetic on the A register (using an immediate)
; ART_N method
%macro ART_N 1
   GETBYTE
   %ifidn %1, ADD
      DO_ADD dl
   %elifidn %1, ADC
      DO_ADC dl
   %elifidn %1, SUB
      DO_SUB dl
   %elifidn %1, SBC
      DO_SBC dl
   %endif

   DO_CYCLES
   NEXT
%endmacro

; Perform arithmetic on the A register, using (XY)
; ART_mXY method, src16
%macro ART_mXY 2
   GETDISP
   add di, z%2
   MEMREAD
   %ifidn %1, ADD
      DO_ADD dl
   %elifidn %1, ADC
      DO_ADC dl
   %elifidn %1, SUB
      DO_SUB dl
   %elifidn %1, SBC
      DO_SBC dl
   %endif

   DO_CYCLES
   NEXT
%endmacro

; Perform arithmetic on the A register using (RR)
; ART_mRR method, src16
%macro ART_mRR 2
   mov edi, ez%2
   MEMREAD
   %ifidn %1, ADD
      DO_ADD dl
   %elifidn %1, ADC
      DO_ADC dl
   %elifidn %1, SUB
      DO_SUB dl
   %elifidn %1, SBC
      DO_SBC dl
   %endif

   DO_CYCLES
   NEXT
%endmacro

; Perform logic on the A register
; LOG_R method, src8
%macro LOG_R 2
   %ifidn %1, AND
      DO_AND z%2
   %elifidn %1, OR
      DO_OR z%2
   %elifidn %1, XOR
      DO_XOR z%2
   %endif

   DO_CYCLES
   NEXT
%endmacro

; Perform logic on the A register (using an immediate)
; LOG_N method
%macro LOG_N 1
   GETBYTE
   %ifidn %1, AND
      DO_AND dl
   %elifidn %1, OR
      DO_OR dl
   %elifidn %1, XOR
      DO_XOR dl
   %endif

   DO_CYCLES
   NEXT
%endmacro

; Perform logic on the A register, using (RR)
; LOG_mRR method, src16
%macro LOG_mRR 2
   mov edi, ez%2
   MEMREAD
   %ifidn %1, AND
      DO_AND dl
   %elifidn %1, OR
      DO_OR dl
   %elifidn %1, XOR
      DO_XOR dl
   %endif

   DO_CYCLES
   NEXT
%endmacro

; Perform logic on the A register, using (XY+dd)
; LOG_mXY method, src16
%macro LOG_mXY 2
   GETDISP
   add di, z%2
   MEMREAD
   %ifidn %1, AND
      DO_AND dl
   %elifidn %1, OR
      DO_OR dl
   %elifidn %1, XOR
      DO_XOR dl
   %endif

   DO_CYCLES
   NEXT
%endmacro

; Compare against the A register
; CP_R src8
%macro CP_R 1
   DO_CP z%1

   DO_CYCLES
   NEXT
%endmacro

; Compare against the A register, using an immediate
; CP_N
%macro CP_N 0
   GETBYTE
   DO_CP dl

   DO_CYCLES
   NEXT
%endmacro

; Compare against the A register, using (RR)
; CP_mRR src16
%macro CP_mRR 1
   mov edi, ez%1
   MEMREAD
   DO_CP dl

   DO_CYCLES
   NEXT
%endmacro

; Compare against the A register, using (XY+dd)
; CP_mXY src16
%macro CP_mXY 1
   GETDISP
   add di, z%1
   MEMREAD
   DO_CP dl

   DO_CYCLES
   NEXT
%endmacro

; Inc/dec a register
; DECINC_R method, reg8
%macro DECINC_R 2
   %ifidn %1, INC
      DO_INC z%2
   %elifidn %1, DEC
      DO_DEC z%2
   %endif

   DO_CYCLES
   NEXT
%endmacro

; Inc/dec (RR)
; DECINC_mRR method
%macro DECINC_mRR 2
   mov edi, ez%2
   MEMREAD
   %ifidn %1, INC
      DO_INC dl
   %elifidn %1, DEC
      DO_DEC dl
   %endif
   MEMWRITE

   DO_CYCLES
   NEXT
%endmacro

; Inc/dec (XY+dd)
; DECINC_mXY method
%macro DECINC_mXY 2
   GETDISP
   add di, z%2
   MEMREAD
   %ifidn %1, INC
      DO_INC dl
   %elifidn %1, DEC
      DO_DEC dl
   %endif
   MEMWRITE

   DO_CYCLES
   NEXT
%endmacro

; Add two 16-bit registers
; ADD_RR_RR dest16, src16
%macro ADD_RR_RR 2
   DO_ADD16 z%1, z%2

   DO_CYCLES
   NEXT
%endmacro

; Add two 16-bit registers, with carry
; ADC_RR_RR dest16, src16
%macro ADC_RR_RR 2
   DO_ADC16 z%1, z%2

   DO_CYCLES
   NEXT
%endmacro

; Subtract two 16-bit registers, with carry
; SBC_RR_RR dest16, src16
%macro SBC_RR_RR 2
   DO_SBC16 z%1, z%2

   DO_CYCLES
   NEXT
%endmacro

; Inc/dec a 16-bit register
; DECINC_RR method, reg16
%macro DECINC_RR 2
   %ifidn %1, INC
      inc z%2
   %elifidn %1, DEC
      dec z%2
   %endif

   DO_CYCLES
   NEXT
%endmacro

; Rotate a register
; ROT_R method, reg8
%macro ROT_R 2
   DO_ROT %1, z%2

   DO_CYCLES
   NEXT
%endmacro

; Rotate (RR)
; ROT_mRR method, src16
%macro ROT_mRR 2
   mov edi, ez%2
   MEMREAD
   DO_ROT %1, dl
   MEMWRITE

   DO_CYCLES
   NEXT
%endmacro

; Rotate (XY+dd)
; ROT_mXY method
%macro ROT_mXY 1
   MEMREAD
   DO_ROT %1, dl
   MEMWRITE

   DO_CYCLES
   NEXT
%endmacro

; Rotate (XY+dd), and store into a register
; ROT_mXY_R method, dest8
%macro ROT_mXY_R 2
   MEMREAD
   DO_ROT %1, dl
   mov z%2, dl
   MEMWRITE

   DO_CYCLES
   NEXT
%endmacro

; Shift a register
; SHF_R method, reg8
%macro SHF_R 2
   DO_SHF %1, z%2

   DO_CYCLES
   NEXT
%endmacro

; Shift (RR)
; SHF_mRR method, src16
%macro SHF_mRR 2
   mov edi, ez%2
   MEMREAD
   DO_SHF %1, dl
   MEMWRITE

   DO_CYCLES
   NEXT
%endmacro

; Shift (XY+dd)
; SHF_mXY method
%macro SHF_mXY 1
   MEMREAD
   DO_SHF %1, dl
   MEMWRITE

   DO_CYCLES
   NEXT
%endmacro

; Shift (XY+dd), and store into a register
; SHF_mXY_R method, dest8
%macro SHF_mXY_R 2
   MEMREAD
   DO_SHF %1, dl
   mov z%2, dl
   MEMWRITE

   DO_CYCLES
   NEXT
%endmacro

; Rotate A/(HL) left, via nibbles
; RLD
%macro RLD 0
   mov edi, ezHL
   MEMREAD
   mov dh, zA
   and zF, FLAG_C
   shl dh, 4
   rol dx, 4
   and zA, 0f0h
   or zA, dh
   mov dh, zF
   lahf
   and zF, FLAG_S|FLAG_Z|FLAG_P
   or zF, dh
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zA
   %endif
   MEMWRITE

   DO_CYCLES
   NEXT
%endmacro

; Rotate A/(HL) right, via nibbles
; RRD
%macro RRD 0
   mov edi, ezHL
   MEMREAD
   mov dh, zA
   and zF, FLAG_C
   ror dx, 4
   shr dh, 4
   and zA, 0f0h
   or zA, dh
   mov dh, zF
   lahf
   and zF, FLAG_S|FLAG_Z|FLAG_P
   or zF, dh
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zA
   %endif
   MEMWRITE

   DO_CYCLES
   NEXT
%endmacro

; Bit-test a register
; BIT_R src8, bit
%macro BIT_R_b 2
   DO_BIT z%1, %2

   DO_CYCLES
   NEXT
%endmacro

; Bit-test (RR)
; BIT_mRR_b src16, bit
%macro BIT_mRR_b 2
   mov edi, ez%1
   MEMREAD
   DO_BIT dl, %2

   DO_CYCLES
   NEXT
%endmacro

; Bit-test (XY+dd)
; BIT_mXY_b bit
%macro BIT_mXY_b 1
   MEMREAD
   DO_BIT dl, %1

   DO_CYCLES
   NEXT
%endmacro

; Bit-clear a register
; RES_R_b src8, bit
%macro RES_R_b 2
   DO_RES z%1, %2

   DO_CYCLES
   NEXT
%endmacro

; Bit-clear (RR)
; RES_mRR_b src16, bit
%macro RES_mRR_b 2
   mov edi, ez%1
   MEMREAD
   DO_RES dl, %2
   MEMWRITE

   DO_CYCLES
   NEXT
%endmacro

; Bit-clear (XY+dd)
; RES_mXY_b bit
%macro RES_mXY_b 1
   MEMREAD
   DO_RES dl, %1
   MEMWRITE

   DO_CYCLES
   NEXT
%endmacro

; Bit-clear (XY+dd), with extra store
; RES_mXY_b_R bit, dest8
%macro RES_mXY_b_R 2
   MEMREAD
   DO_RES dl, %1
   mov z%2, dl
   MEMWRITE

   DO_CYCLES
   NEXT
%endmacro

; Bit-set a register
; SET_R_b src8, bit
%macro SET_R_b 2
   DO_SET z%1, %2

   DO_CYCLES
   NEXT
%endmacro

; Bit-set (RR)
; SET_mRR_b src16, bit
%macro SET_mRR_b 2
   mov edi, ez%1
   MEMREAD
   DO_SET dl, %2
   MEMWRITE

   DO_CYCLES
   NEXT
%endmacro

; Bit-set (XY+dd)
; SET_mXY_b bit
%macro SET_mXY_b 1
   MEMREAD
   DO_SET dl, %1
   MEMWRITE

   DO_CYCLES
   NEXT
%endmacro

; Bit-set (XY+dd), with extra store
; SET_mXY_b_R bit, dest8
%macro SET_mXY_b_R 2
   MEMREAD
   DO_SET dl, %1
   mov z%2, dl
   MEMWRITE

   DO_CYCLES
   NEXT
%endmacro

; Jump to an absolute address
; JP_NN
%macro JP_NN 0
   GETWORD
   mov ezPC, edi
   DO_CYCLES
   NEXT
%endmacro

; Jump to an absolute address, conditionally
; JP_cc_NN nz,cc
%macro JP_cc_NN 2
   test zF, FLAG_%2
   j%1 near %%dont_take_jump
   GETWORD
   mov ezPC, edi
   DO_CYCLES
   NEXT

   ALIGN
%%dont_take_jump:
   add zPC, 2     ; skip address
   DO_CYCLES
   NEXT
%endmacro

; Jump to an relative address
; JR_N
%macro JR_N 0
   GETDISP
   add zPC, di
   DO_CYCLES
   NEXT
%endmacro

; Jump to an relative address, conditionally
; JR_cc_N nz,cc
%macro JR_cc_N 2
   test zF, FLAG_%2
   j%1 near %%dont_take_jump
   GETDISP
   add zPC, di
   DO_EXTRA_CYCLES
   NEXT

   ALIGN
%%dont_take_jump:
   inc zPC           ; skip over the displacement
   DO_CYCLES
   NEXT
%endmacro

; Jump to an absolute address (in a register)
; JP_RR addr16
%macro JP_RR 1
   mov ezPC, ez%1
   DO_CYCLES
   NEXT
%endmacro

; Decrease B, if non-zero, jump to a relative address
; DJNZ_N
%macro DJNZ_N 0
   dec zB
   jz near %%dont_take_jump
   GETDISP
   add zPC, di
   DO_EXTRA_CYCLES
   NEXT

   ALIGN
%%dont_take_jump:
   inc zPC           ; skip over the displacement
   DO_CYCLES
   NEXT
%endmacro

; Call an absolute address
; CALL_NN
%macro CALL_NN 0
   GETWORD
   mov edx, ezPC        ; write the PC
   dec zSP
   push edi

   xchg dl, dh
   mov edi, ezSP
   MEMWRITE
   dec zSP
   mov edx, ezPC
   mov edi, ezSP
   MEMWRITE

   pop ezPC             ; get the new PC
   DO_CYCLES
   NEXT
%endmacro

; Call an absolute address, conditionally
; CALL_cc_NN
%macro CALL_cc_NN 2
   test zF, FLAG_%2
   j%1 near %%dont_take_jump
   GETWORD
   mov edx, ezPC        ; write the PC
   push edi

   dec zSP
   xchg dl, dh
   mov edi, ezSP
   MEMWRITE
   dec zSP
   mov edx, ezPC
   mov edi, ezSP
   MEMWRITE

   pop ezPC             ; get the new PC
   DO_CYCLES
   NEXT

   ALIGN
%%dont_take_jump:
   add zPC, 2     ; skip address
   DO_CYCLES
   NEXT
%endmacro

; Pop the PC
; RET
%macro RET 0
   mov edi, ezSP
   MEMREAD
   inc zSP
   mov ezPC, edx
   mov edi, ezSP
   MEMREAD
   xchg dl, dh
   or ezPC, edx
   inc zSP

   xor dh, dh
   DO_CYCLES
   NEXT
%endmacro

; Pop the PC, conditionally
; RET_cc
%macro RET_cc 2
   test zF, FLAG_%2
   j%1 near %%dont_take_jump
   mov edi, ezSP
   MEMREAD
   inc zSP
   mov ezPC, edx
   mov edi, ezSP
   MEMREAD
   xchg dl, dh
   or ezPC, edx
   inc zSP

   xor dh, dh
   DO_CYCLES
   NEXT

   ALIGN
%%dont_take_jump:
   DO_CYCLES
   NEXT
%endmacro

; Pop the PC, restore IFF1, signal the BUS
; RETI        
%macro RETI 0
   mov edi, ezSP
   MEMREAD
   mov ezPC, edx
   inc zSP
   mov edi, ezSP
   MEMREAD
   xchg dl, dh
   or ezPC, edx
   inc zSP

   mov edx, [_z80_IFF2]
   mov [_z80_IFF1], edx 
   call [_z80_RetI]           ; signal the user handler

   DO_CYCLES
   NEXT
%endmacro

; Pop the PC, restore IFF1
; RETN
%macro RETN 0
   mov edi, ezSP
   MEMREAD
   mov ezPC, edx
   inc zSP
   mov edi, ezSP
   MEMREAD
   xchg dl, dh
   or ezPC, edx
   inc zSP

   mov edx, [_z80_IFF2]
   mov [_z80_IFF1], edx
   DO_CYCLES
   NEXT
%endmacro

; Call an absolute address
; RST addr8
%macro RST 1
   dec zSP
   mov edx, ezPC        ; write the PC
   xchg dl, dh
   mov edi, ezSP
   MEMWRITE
   dec zSP
   mov edx, ezPC
   mov edi, ezSP
   MEMWRITE

   mov ezPC, %1         ; get the new PC
   DO_CYCLES
   NEXT
%endmacro

; Out R to R*256+N
; OUT_N_R src8
%macro OUT_N_R 1
   GETBYTE
   mov dh, z%1
   mov edi, edx
   mov dl, z%1
   IOWRITE
   DO_CYCLES
   NEXT
%endmacro

; In from R*256+N, into R
; IN_R_N dest8
%macro IN_R_N 1
   GETBYTE
   mov dh, z%1
   mov edi, edx
   IOREAD
   mov z%1, dl
   DO_CYCLES
   NEXT
%endmacro

; In from BC
; IN_R dest8
%macro IN_R 1
   mov edi, ezBC
   IOREAD
   mov dh, zF
   and dh, FLAG_C       ; keep the old carry
   mov z%1, dl
   test dl, dl          ; other flags come from the result
   lahf
   and zF, FLAG_S|FLAG_Z|FLAG_P
   or zF, dh
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], dl
   %endif
   xor dh, dh
   
   DO_CYCLES
   NEXT
%endmacro

; In from BC, ignore result
; IN_F
%macro IN_F 0
   mov edi, ezBC
   IOREAD
   mov dh, zF
   and dh, FLAG_C       ; keep the old carry
   test dl, dl          ; other flags come from the result
   lahf
   and zF, FLAG_S|FLAG_Z|FLAG_P
   or zF, dh
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], dl
   %endif
   xor dh, dh
   
   DO_CYCLES
   NEXT
%endmacro

; Out to BC
; OUT_R src8
%macro OUT_R 1
   mov edi, ezBC
   mov dl, z%1
   IOWRITE
   DO_CYCLES
   NEXT
%endmacro

; Out zero to BC
; OUT_0
%macro OUT_0 0
   mov edi, ezBC
   xor dl, dl
   IOWRITE
   DO_CYCLES
   NEXT
%endmacro

; In from BC into (HL), increase HL, decrease B
%macro INI 0
; FIXME - Parity flag is not emulated
   movzx edi, zC           ; high byte clear? (documentation is fuzzy)

   IOREAD                  ; read from the port
   mov edi, ezHL
   %ifdef EMULATE_WEIRD_STUFF
      push edx             ; preserve the value
   %endif
   MEMWRITE                ; write to (HL)
   %ifdef EMULATE_WEIRD_STUFF
      pop edx
   %endif

   inc zHL                 ; increase HL
   dec zB                  ; decrease B
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zB
   %endif

   lahf
   and zF, FLAG_S|FLAG_Z|FLAG_P

   ; This is a prime example of the weirdness of the Z80...
   %ifdef EMULATE_WEIRD_STUFF
      mov dh, zC
      inc dh
      add dh, dl
      jnc %%no_carry
      or zF, FLAG_H|FLAG_C
   %%no_carry:
      shr dl, 6
      and edx, FLAG_N
      or zF, dl
   %endif
   DO_CYCLES
   NEXT
%endmacro

; In from BC into (HL), decrease HL, decrease B
%macro IND 0
; FIXME - Parity flag is not emulated
   movzx edi, zC           ; high byte clear? (documentation is fuzzy)

   IOREAD                  ; read from the port
   mov edi, ezHL
   %ifdef EMULATE_WEIRD_STUFF
      push edx             ; preserve the value
   %endif
   MEMWRITE                ; write to (HL)
   %ifdef EMULATE_WEIRD_STUFF
      pop edx
   %endif

   dec zHL                 ; increase HL
   dec zB                  ; decrease B
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zB
   %endif

   lahf
   and zF, FLAG_S|FLAG_Z|FLAG_P

   ; This is a prime example of the weirdness of the Z80...
   %ifdef EMULATE_WEIRD_STUFF
      mov dh, zC
      dec dh
      add dh, dl
      jnc %%no_carry
      or zF, FLAG_H|FLAG_C
   %%no_carry:
      shr dl, 6
      and edx, FLAG_N
      or zF, dl
   %endif
   DO_CYCLES
   NEXT
%endmacro

; In from BC into (HL), increase HL, decrease B, repeat
%macro INIR 0
; FIXME - Parity flag is not emulated
%%inir_loop:
   movzx edi, zC           ; high byte clear? (documentation is fuzzy)

   IOREAD                  ; read from the port
   mov edi, ezHL
   %ifdef EMULATE_WEIRD_STUFF
      push edx             ; preserve the value
   %endif
   MEMWRITE                ; write to (HL)
   %ifdef EMULATE_WEIRD_STUFF
      pop edx
   %endif

   inc zHL                 ; increase HL
   dec zB                  ; decrease B
   jz %%inir_zero

   sub [_z80_ICount], dword extra_cycles
   ja near %%inir_loop            ; repeat while cycles left and B>0

   ; out of cycles, but operation not finished
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zB
   %endif

   mov zF, zB
   and zF, FLAG_S

   ; This is a prime example of the weirdness of the Z80...
   %ifdef EMULATE_WEIRD_STUFF
      mov dh, zC
      inc dh
      add dh, dl
      jnc %%no_carry
      or zF, FLAG_H|FLAG_C
   %%no_carry:
      shr dl, 6
      and edx, FLAG_N
      or zF, dl
   %endif
   sub zPC, 2           ; rewind to instruction start
   EXIT

; B=0
%%inir_zero:
   mov zF, FLAG_Z|FLAG_P
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zF
   %endif

   ; This is a prime example of the weirdness of the Z80...
   %ifdef EMULATE_WEIRD_STUFF
      mov dh, zC
      inc dh
      add dh, dl
      jnc %%no_carry2
      or zF, FLAG_H|FLAG_C
   %%no_carry2:
      shr dl, 6
      and edx, FLAG_N
      or zF, dl
   %endif
   DO_CYCLES
   NEXT
%endmacro

; In from BC into (HL), decrease HL, decrease B, repeat
%macro INDR 0
; FIXME - Parity flag is not emulated
%%indr_loop:
   movzx edi, zC           ; high byte clear? (documentation is fuzzy)

   IOREAD                  ; read from the port
   mov edi, ezHL
   %ifdef EMULATE_WEIRD_STUFF
      push edx             ; preserve the value
   %endif
   MEMWRITE                ; write to (HL)
   %ifdef EMULATE_WEIRD_STUFF
      pop edx
   %endif

   dec zHL                 ; decrease HL
   dec zB                  ; decrease B
   jz %%indr_zero

   sub [_z80_ICount], dword extra_cycles
   ja near %%indr_loop            ; repeat while cycles left and B>0

   ; out of cycles, but operation not finished
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zB
   %endif

   mov zF, zB
   and zF, FLAG_S

   ; This is a prime example of the weirdness of the Z80...
   %ifdef EMULATE_WEIRD_STUFF
      mov dh, zC
      inc dh
      add dh, dl
      jnc %%no_carry
      or zF, FLAG_H|FLAG_C
   %%no_carry:
      shr dl, 6
      and edx, FLAG_N
      or zF, dl
   %endif
   sub zPC, 2           ; rewind to instruction start
   EXIT

; B=0
%%indr_zero:
   mov zF, FLAG_Z|FLAG_P
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zF
   %endif

   ; This is a prime example of the weirdness of the Z80...
   %ifdef EMULATE_WEIRD_STUFF
      mov dh, zC
      inc dh
      add dh, dl
      jnc %%no_carry2
      or zF, FLAG_H|FLAG_C
   %%no_carry2:
      shr dl, 6
      and edx, FLAG_N
      or zF, dl
   %endif
   DO_CYCLES
   NEXT
%endmacro

; Out from (HL) into BC, increase HL, decrease B
%macro OUTI 0
; FIXME - Parity flag is not emulated
   mov edi, ezHL

   MEMREAD                 ; read from (HL)
   movzx edi, zC           ; high byte clear? (documentation is fuzzy)
   %ifdef EMULATE_WEIRD_STUFF
      push edx             ; preserve the value
   %endif
   IOWRITE                 ; write to the port
   %ifdef EMULATE_WEIRD_STUFF
      pop edx
   %endif

   inc zHL                 ; increase HL
   dec zB                  ; decrease B
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zB
   %endif

   lahf
   and zF, FLAG_S|FLAG_Z|FLAG_P

   ; This is a prime example of the weirdness of the Z80...
   %ifdef EMULATE_WEIRD_STUFF
      mov dh, zC
      inc dh
      add dh, dl
      jnc %%no_carry
      or zF, FLAG_H|FLAG_C
   %%no_carry:
      shr dl, 6
      and edx, FLAG_N
      or zF, dl
   %endif
   DO_CYCLES
   NEXT
%endmacro

; Out from (HL) into BC, decrease HL, decrease B
%macro OUTD 0
; FIXME - Parity flag is not emulated
   mov edi, ezHL

   MEMREAD                 ; read from (HL)
   movzx edi, zC           ; high byte clear? (documentation is fuzzy)
   %ifdef EMULATE_WEIRD_STUFF
      push edx             ; preserve the value
   %endif
   IOWRITE                 ; write to the port
   %ifdef EMULATE_WEIRD_STUFF
      pop edx
   %endif

   dec zHL                 ; decrease HL
   dec zB                  ; decrease B
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zB
   %endif

   lahf
   and zF, FLAG_S|FLAG_Z|FLAG_P

   ; This is a prime example of the weirdness of the Z80...
   %ifdef EMULATE_WEIRD_STUFF
      mov dh, zC
      inc dh
      add dh, dl
      jnc %%no_carry
      or zF, FLAG_H|FLAG_C
   %%no_carry:
      shr dl, 6
      and edx, FLAG_N
      or zF, dl
   %endif
   DO_CYCLES
   NEXT
%endmacro

; Out from (HL) into BC, increase HL, decrease B, repeat
%macro OTIR 0
%%otir_loop:
   mov edi, ezHL
   MEMREAD
   movzx edi, zC           ; high byte clear? (documentation is fuzzy)
   %ifdef EMULATE_WEIRD_STUFF
      push edx             ; preserve the value
   %endif
   IOWRITE
   %ifdef EMULATE_WEIRD_STUFF
      pop edx
   %endif

   inc zHL
   dec zB
   jz %%otir_zero

%ifdef EMULATE_R_REGISTER
   inc zR
%endif
   sub [_z80_ICount], dword extra_cycles
   ja near %%otir_loop

; out of cycles, but not finished
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zB
   %endif

   mov zF, zB
   and zF, FLAG_S

   %ifdef EMULATE_WEIRD_STUFF
      mov dh, zC
      inc dh
      add dh, dl
      jnc %%otir_nocarry
      or zF, FLAG_H|FLAG_C
   %%otir_nocarry:
      shr dl, 6
      and edx, FLAG_N
      or zF, dl
   %endif
   sub zPC, 2           ; rewind to instruction start
   EXIT

%%otir_zero:
   mov zF, FLAG_Z|FLAG_P
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zF
   %endif
   %ifdef EMULATE_WEIRD_STUFF
      mov dh, zC
      inc dh
      add dh, dl
      jnc %%otir_nocarry2
      or zF, FLAG_H|FLAG_C
   %%otir_nocarry2:
      shr dl, 6
      and edx, FLAG_N
      or zF, dl
   %endif
   DO_CYCLES
   NEXT
%endmacro

; Out from (HL) into BC, decrease HL, decrease B, repeat
%macro OTDR 0
%%otdr_loop:
   mov edi, ezHL
   MEMREAD
   movzx edi, zC           ; high byte clear? (documentation is fuzzy)
   %ifdef EMULATE_WEIRD_STUFF
      push edx             ; preserve the value
   %endif
   IOWRITE
   %ifdef EMULATE_WEIRD_STUFF
      pop edx
   %endif

   dec zHL
   dec zB
   jz %%otdr_zero

%ifdef EMULATE_R_REGISTER
   inc zR
%endif
   sub [_z80_ICount], dword extra_cycles
   ja near %%otdr_loop

; out of cycles, but not finished
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zB
   %endif

   mov zF, zB
   and zF, FLAG_S

   %ifdef EMULATE_WEIRD_STUFF
      mov dh, zC
      inc dh
      add dh, dl
      jnc %%otdr_nocarry
      or zF, FLAG_H|FLAG_C
   %%otdr_nocarry:
      shr dl, 6
      and edx, FLAG_N
      or zF, dl
   %endif
   sub zPC, 2           ; rewind to instruction start
   EXIT

%%otdr_zero:
   mov zF, FLAG_Z|FLAG_P
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zF
   %endif
   %ifdef EMULATE_WEIRD_STUFF
      mov dh, zC
      inc dh
      add dh, dl
      jnc %%otdr_nocarry2
      or zF, FLAG_H|FLAG_C
   %%otdr_nocarry2:
      shr dl, 6
      and edx, FLAG_N
      or zF, dl
   %endif
   DO_CYCLES
   NEXT
%endmacro
	
; BCD adjust
%macro DAA 0
   test zF, FLAG_H
   jnz %%daa_h_set
   and eax, 03ffh
   mov ax, [DAA_Table + eax*2]
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zF
   %endif
   DO_CYCLES
   NEXT

%%daa_h_set:
   and eax, 03ffh
   mov ax, [DAA_Table + eax*2 + 4*512]
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zF
   %endif
   DO_CYCLES
   NEXT
%endmacro

; Complement A
%macro CPL 0
   not zA
   or zF, FLAG_H|FLAG_N
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zA
   %endif
   DO_CYCLES
   NEXT
%endmacro

; Negate A
%macro NEG 0
   neg zA
   lahf
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zA
   %endif
   jo %%neg_overflow
   and zF, ~FLAG_P
   DO_CYCLES
   NEXT

%%neg_overflow:
   or zF, FLAG_P
   DO_CYCLES
   NEXT
%endmacro

; Set carry flag
%macro SCF 0
   and zF, FLAG_S|FLAG_Z|FLAG_P
   or zF, FLAG_C
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zA
   %endif
   DO_CYCLES
   NEXT
%endmacro

; Complement carry flag
%macro CCF 0
   test zF, FLAG_C
   jz %%set_carry
   and zF, FLAG_S|FLAG_Z|FLAG_P
   or zF, FLAG_H
   %ifdef EMULATE_BITS_3_5
      mov [_z80_flag35], zA
   %endif
   DO_CYCLES
   NEXT

%%set_carry:
   SCF
%endmacro

; No operation
%macro NOP 0
   DO_CYCLES
   NEXT
%endmacro

; Halt the processor
%macro HALT 0
   and [_z80_ICount], dword 3
   sub [_z80_ICount], dword 4
   dec zPC
   EXIT
%endmacro

; Enable interrupts
%macro EI 0
   mov edx, FLAG_P      ; set the P/V bit, for use in "LD I,A" etc
   mov [_z80_IFF1], edx
   mov [_z80_IFF2], edx

   ; force the next instruction to be executed, regardless of ICount
   ; then force an IRQ check after it.
   mov [_z80_afterEI], byte 1
   push eax
   mov eax, [_z80_ICount]        ; set ICount to zero, so it will terminate
   mov [_z80_TempICount], eax    ; after the next instruction.
   mov [_z80_ICount], dword 1    ; we can then pick it up again and carry on.
   pop eax
   NEXT
%endmacro

; Disable interrupts
%macro DI 0
   xor edx, edx
   mov [_z80_IFF1], edx
   mov [_z80_IFF2], edx
   DO_CYCLES
   NEXT
%endmacro

; Set interrupt mode
%macro IM 1
   mov [_z80_IM], byte %1
   DO_CYCLES
   NEXT
%endmacro

;----------------------------------------------------------------------------;
; Bring in the opcodes...
%include 'raze.inc'
;----------------------------------------------------------------------------;

;----------------------------------------------------------------------------;
; Support functions:

DEF op, cb, 0, 0
%ifdef EMULATE_R_REGISTER
   inc zR
%endif
   GETBYTE
   jmp [CBTable+edx*4]

DEF op, ed, 0, 0
%ifdef EMULATE_R_REGISTER
   inc zR
%endif
   GETBYTE
   jmp [EDTable+edx*4]

DEF op, dd, 0, 0
%ifdef EMULATE_R_REGISTER
   inc zR
%endif
   GETBYTE
   jmp [DDTable+edx*4]

DEF op, fd, 0, 0
%ifdef EMULATE_R_REGISTER
   inc zR
%endif
   GETBYTE
   jmp [FDTable+edx*4]

DEF dd, cb, 0, 0
%ifdef EMULATE_R_REGISTER
   inc zR
%endif
   GETDISP
   add di, zIX
   push edi
   GETBYTE
   pop edi
   jmp [DDCBTable+edx*4]

DEF fd, cb, 0, 0
%ifdef EMULATE_R_REGISTER
   inc zR
%endif
   GETDISP
   add di, zIY
   push edi
   GETBYTE
   pop edi
   jmp [DDCBTable+edx*4]

DEF ed, xx, 8, 0
%ifdef EMULATE_R_REGISTER
   inc zR
%endif
   DO_CYCLES
   NEXT

DEF dd, xx, 4, 0
   dec zPC
   DO_CYCLES
   NEXT

DEF fd, xx, 0, 0
   dec zPC
   DO_CYCLES
   NEXT

; called when out-of-cycles
z80_finish:
   cmp [_z80_afterEI], byte 1    ; test for the post-EI behaviour
   jne near really_finish
   push eax
   mov eax, [_z80_TempICount]
   mov [_z80_ICount], eax
   mov [_z80_afterEI], byte 0
   pop eax
   CHECK_IRQ
   sub dword [_z80_ICount], 4    ; subtract EI's cycles
   jbe near z80_finish
   NEXT

really_finish:   
   mov [_z80_AF], ezAF
   mov [_z80_HL], ezHL
   mov [_z80_BC], ezBC
   mov [_z80_PC], ezPC
   mov [_z80_R], zR
   pop ebp
   pop edi
   pop esi
   pop ebx
   mov eax, [_z80_Initial_ICount]
   sub eax, [_z80_ICount]
   ret

z80_default_in:
z80_default_read:
   mov eax, 0ffh
   ret

z80_default_out:
z80_default_write:
z80_default_reti:
z80_default_fetch_callback:
   ret

; called to look up a read handler
; address is on the stack
read_dispatcher:
   mov eax, [esp+4]
   mov edx, _read_handlers
rd_find:
   add edx, 16
   cmp [edx-16], eax
   ja rd_find
   cmp [edx-12], eax
   jb rd_find

   cmp [edx-8], dword 1
   je rd_handler

   add eax, [edx-4]
   movzx eax, byte [eax]
   ret

rd_handler:
   jmp [edx-4]    ; call the GCC function

; called to look up a write handler
; address+value are on the stack
write_dispatcher:
   mov eax, [esp+4]
   mov edx, _write_handlers
wd_find:
   add edx, 16
   cmp [edx-16], eax
   ja wd_find
   cmp [edx-12], eax
   jb wd_find

   cmp [edx-8], dword 1
   je wd_handler

   add eax, [edx-4]
   mov ecx, [esp+8]
   mov [eax], cl
   ret

wd_handler:
   jmp [edx-4]    ; call the GCC function

; int z80_emulate(int cycles)
; Emulate for 'cycles' T-states,
; Returns the number of cycles actually emulated.
global _z80_emulate
_z80_emulate:
   mov eax, [esp+4]
   push ebx
   push esi
   push edi
   push ebp

   add eax, [_z80_Extra_Cycles]
   mov [_z80_ICount], eax
   mov [_z80_Initial_ICount], eax
   xor edx, edx
   xor edi, edi
   mov [_z80_Extra_Cycles], edx
   mov [_z80_afterEI], dl
   mov ezAF, [_z80_AF]
   mov ezHL, [_z80_HL]
   mov ezBC, [_z80_BC]
   mov ezPC, [_z80_PC]
   mov zR,   [_z80_R]
   NEXT

; void z80_reset(void)
; Resets the Z80.
global _z80_reset
_z80_reset:
   push edi
   xor eax, eax      ; just zero it all
   mov edi, context_start
   mov ecx, (registers_end - context_start)
   rep stosb
   pop edi
   mov [_z80_IX], word 0ffffh
   mov [_z80_IY], word 0ffffh
   mov [_z80_AF], word 04000h
   ret

; void z80_raise_IRQ(UBYTE vector)
; Causes an interrupt with the given vector.
; In IM 0, the vector has to be an RST opcode.
; If interrupts are disabled, the interrupt will still happen as long as
; the IRQ line stays raised.
global _z80_raise_IRQ
_z80_raise_IRQ:
   mov [_z80_IRQLine], dword 1
   mov eax, [esp+4]
   mov [_z80_IRQVector], al

   xor edx, edx
   push ebx
   push esi
   push edi
   push ebp
   CALLGCC_END          ; grab our Z80 regs

   CHECK_IRQ

   CALLGCC_START
   pop ebp
   pop edi
   pop esi
   pop ebx
   ret

; void z80_lower_IRQ(void)
; Lowers the IRQ line.
global _z80_lower_IRQ
_z80_lower_IRQ:
   mov [_z80_IRQLine], dword 0
   ret

; void z80_cause_NMI(void)
; Causes a non-maskable interrupt.
; This will always be accepted.
; There is no raise/lower functions, as the NMI is edge-triggered.
global _z80_cause_NMI
_z80_cause_NMI:
   xor edx, edx
   push ebx
   push esi
   push edi
   push ebp
   CALLGCC_END          ; grab our Z80 regs

   ; disable interrupts
   mov [_z80_IFF1], byte 0

   ; take up 11 T-states
   add [_z80_Extra_Cycles], dword 11
%ifdef EMULATE_R_REGISTER
   inc zR
%endif

   GETBYTE
   cmp dl, 76h          ; if we're HALTed, skip the HALT
   je nmi_skip_halt
   dec zPC
nmi_skip_halt:

   mov edx, ezPC        ; write the PC
   dec zSP

   xchg dl, dh
   mov edi, ezSP
   MEMWRITE
   dec zSP
   mov edx, ezPC
   mov edi, ezSP
   MEMWRITE

   mov ezPC, 66h        ; set the new PC

   CALLGCC_START
   pop ebp
   pop edi
   pop esi
   pop ebx
   ret

; int z80_get_context_size(void)
; Returns the size of the context, in bytes
global _z80_get_context_size
_z80_get_context_size:
   mov eax, (context_end - context_start) + 4   ; +4 for a safety buffer
   ret


; void z80_set_context(void *context)
; Copy the given context to the current Z80.
global _z80_set_context
_z80_set_context:
   push esi
   push edi
   mov ecx, (context_end - context_start) / 4
   mov esi, [esp+12]
   mov edi, context_start
   rep movsd
   pop edi
   pop esi
   ret

; void z80_get_context(void *context)
; Copy the current Z80 to the given context.
global _z80_get_context
_z80_get_context:
   push esi
   push edi
   mov ecx, (context_end - context_start) / 4
   mov edi, [esp+12]
   mov esi, context_start
   rep movsd
   pop edi
   pop esi
   ret

; int z80_get_cycles_elapsed(void)
; Returns the number of cycles emulated since z80_emulate was called.
global _z80_get_cycles_elapsed
_z80_get_cycles_elapsed:
   mov eax, [_z80_Initial_ICount]
   sub eax, [_z80_ICount]
   cmp [_z80_afterEI], byte 0
   je z80_gcd_fin
   sub eax, [_z80_TempICount]
z80_gcd_fin:
   ret

; void z80_stop_emulating(void)
; Stops the emulation dead. (waits for the current instruction to finish
; first though).
; The return value from z80_execute will obviously be lower than expected.
global _z80_stop_emulating
_z80_stop_emulating:
   mov eax, [_z80_ICount]
   sub [_z80_Initial_ICount], eax
   cmp [_z80_afterEI], byte 0
   je z80_se_fin
   mov ecx, [_z80_TempICount]
   sub [_z80_Initial_ICount], ecx
z80_se_fin:
   mov [_z80_ICount], dword 0
   mov [_z80_TempICount], dword 0
   ret

; void z80_skip_idle(void)
; Stops the emulation dead. (waits for the current instruction to finish
; first though).
; The return value from z80_execute will appear as though it had executed
; all the instructions like normal.
global _z80_skip_idle
_z80_skip_idle:
   mov [_z80_ICount], dword 0
   mov [_z80_TempICount], dword 0
   ret

; void z80_do_wait_states(int n)
; Halts the CPU temporarily, to execute 'n' memory wait states.
global _z80_do_wait_states
_z80_do_wait_states:
   add [_z80_Initial_ICount], eax
   ret

; UWORD z80_get_reg(z80_register reg)
; Return the value of the specified register.
global _z80_get_reg
_z80_get_reg:
   mov ecx, [esp+4]
   cmp ecx, Z80_REG_HL2
   mov eax, [context_start + ecx*4]
   jbe getreg_simple
   
   cmp ecx, Z80_REG_IFF2
   mov eax, [context_start + ecx*4]
   jbe getreg_boolean

   cmp ecx, Z80_REG_IRQLine
   mov eax, [_z80_IRQLine]
   je getreg_boolean

   cmp ecx, Z80_REG_IM
   movzx eax, byte [_z80_IM]
   je getreg_finish

   cmp ecx, Z80_REG_IRQVector
   movzx eax, byte [_z80_IRQVector]
   je getreg_finish

   ; construct IR
   mov eax, [_z80_R]
   and eax, 7fh
   mov edx, [_z80_R2]
   and edx, 80h
   or eax, edx
   mov edx, [_z80_I]
   mov ah, dl
   ret

getreg_simple:
   cmp ecx, Z80_REG_AF
   je getreg_swap
   cmp ecx, Z80_REG_AF2
   je getreg_swap
getreg_finish:
   ret
getreg_swap:
   xchg al, ah          ; swap A and F
   ret

getreg_boolean:
   test eax, eax
   jz getreg_finish
   mov eax, 1
   ret

; void z80_set_reg(z80_register reg, UWORD value)
; Set the specified register to a new value.
global _z80_set_reg
_z80_set_reg:
   mov ecx, [esp+4]
   mov eax, [esp+8]
   cmp ecx, Z80_REG_HL2
   jbe setreg_simple
   
   cmp ecx, Z80_REG_IFF2
   lea edx, [context_start + ecx*4]
   jbe setreg_boolean

   cmp ecx, Z80_REG_IRQLine
   lea edx, [_z80_IRQLine]
   je setreg_boolean

   cmp ecx, Z80_REG_IM
   lea edx, [_z80_IM]
   je setreg_value

   cmp ecx, Z80_REG_IRQVector
   lea edx, [_z80_IRQVector]
   je setreg_value

   ; deconstruct IR
   mov edx, eax
   mov [_z80_R2], eax
   and eax, 7fh
   mov [_z80_R], eax
   mov [_z80_I], dh
   ret

setreg_simple:
   cmp ecx, Z80_REG_AF
   je setreg_swap
   cmp ecx, Z80_REG_AF2
   je setreg_swap
setreg_finish:
   mov [context_start + ecx*4], eax
   ret
setreg_swap:
   xchg al, ah          ; swap A and F
   mov [context_start + ecx*4], eax
   ret

setreg_boolean:
   test eax, eax
   jz setreg_value
   mov eax, 1|FLAG_P
setreg_value:
   mov [edx], al
   ret

; void z80_init_memmap(void)
; Reset the current memory map
global _z80_init_memmap
_z80_init_memmap:
   mov [_z80_In], dword z80_default_in
   mov [_z80_Out], dword z80_default_out
   mov [_z80_RetI], dword z80_default_reti
   mov [_z80_Fetch_Callback], dword z80_default_fetch_callback

   push edi    ; clear the memory handlers
   mov eax, -1
   mov ecx, MAX_MEM_HANDLERS*4
   mov edi, _read_handlers
   rep stosd
   mov ecx, MAX_MEM_HANDLERS*4
   mov edi, _write_handlers
   rep stosd

   ; reset the read table
   mov edi, _z80_Read
   mov ecx, 100h
clear_read:
   mov [edi], dword z80_default_read
   add edi, 8
   loop clear_read

   ; reset the write table
   mov edi, _z80_Write
   mov ecx, 100h
clear_write:
   mov [edi], dword z80_default_write
   add edi, 8
   loop clear_write

   pop edi   
   ret

; void z80_end_memmap(void)
; Finishes the current memory map
global _z80_end_memmap
_z80_end_memmap:
   ; put the entry at the end of the read table
   mov edx, _read_handlers
er_find:
   add edx, 16
   cmp [edx-16], dword (-1)
   jne er_find
   sub edx, 16

   ; put it in there
   mov [edx], dword 0
   mov [edx+4], dword 0ffffh
   mov [edx+8], dword 0
   mov [edx+12], dword z80_default_read

   ; put the entry at the end of the write table
   mov edx, _write_handlers
ew_find:
   add edx, 16
   cmp [edx-16], dword (-1)
   jne ew_find
   sub edx, 16

   ; put it in there
   mov [edx], dword 0
   mov [edx+4], dword 0ffffh
   mov [edx+8], dword 0
   mov [edx+12], dword z80_default_write
   ret

; void z80_map_fetch(UWORD start, UWORD end, UBYTE *memory)
; Set the given area for op-code fetching
; start/end = the area it covers
; memory = the ROM/RAM to read from
global _z80_map_fetch
_z80_map_fetch:
   push ebx
   mov eax, [esp+8]
   mov ebx, [esp+12]
   mov ecx, [esp+16]
   sub ecx, eax

mf_loop:
; if there's no area left inbetween, stop now.
   mov edx, eax
   shr edx, 8

   cmp eax, ebx
   ja mf_finish

   mov [_z80_Fetch+edx*4], ecx
   add eax, 0100h
   jmp mf_loop

mf_finish:
   pop ebx
   ret

; void z80_map_read(UWORD start, UWORD end, UBYTE *memory)
; Moves a Z80_MAP_MAPPED area to use a different region of memory
; start/end = the area it covers (must be page-aligned)
; memory = the ROM/RAM to read from
global _z80_map_read
_z80_map_read:
   push ebx
   mov eax, [esp+8]
   mov ebx, [esp+12]
   mov ecx, [esp+16]
   sub ecx, eax

mr_loop:
; if there's no area left inbetween, stop now.
   mov edx, eax
   shr edx, 8

   cmp eax, ebx
   ja mr_finish

   mov [_z80_Read+edx*8], dword 0
   mov [_z80_Read+edx*8+4], ecx
   add eax, 0100h
   jmp mr_loop

mr_finish:
   pop ebx
   ret

; void z80_map_write(UWORD start, UWORD end, UBYTE *memory)
; Moves a Z80_MAP_MAPPED area to use a different region of memory
; start/end = the area it covers (must be page-aligned)
; memory = the ROM/RAM to read from
global _z80_map_write
_z80_map_write:
   push ebx
   mov eax, [esp+8]
   mov ebx, [esp+12]
   mov ecx, [esp+16]
   sub ecx, eax

mw_loop:
; if there's no area left inbetween, stop now.
   mov edx, eax
   shr edx, 8

   cmp eax, ebx
   ja mw_finish

   mov [_z80_Write+edx*8], dword 0
   mov [_z80_Write+edx*8+4], ecx
   add eax, 0100h
   jmp mw_loop

mw_finish:
   pop ebx
   ret

; void z80_add_read(UWORD start, UWORD end, int method, void *data)
; Add a READ memory handler to the memory map
; start/end = the area it covers
; method = 0 for direct, 1 for handled
; data = RAM for direct, handler for handled
global _z80_add_read
_z80_add_read:
   push ebx
   mov eax, [esp+8]
   mov ebx, [esp+12]
   mov ecx, [esp+20]

   ; put the entry in the read table first
   mov edx, _read_handlers
ar_find:
   add edx, 16
   cmp [edx-16], dword (-1)
   jne ar_find
   sub edx, 16

   ; put it in there
   mov [edx], eax
   mov [edx+4], ebx
   mov eax, [esp+16]
   mov [edx+8], eax

   mov eax, ecx         ; if direct, subtract the start from the memory area
   cmp [esp+16], dword 1
   je ar_dont_adjust
   sub eax, [esp+8]
ar_dont_adjust:
   mov [edx+12], eax

   mov eax, [esp+8]
   
; process the start, so it's page aligned
   test al, al
   jz ar_start_done

   ; put the start handler to go to the dispatcher
   mov edx, eax
   shr edx, 8
   mov [_z80_Read+edx*8], dword read_dispatcher
   xor al, al
   add eax, 0100h

ar_start_done:
; process the end, so it's page aligned
   cmp bl, 0ffh
   je ar_end_done

   ; put the end handler to go to the dispatcher
   mov edx, ebx
   shr edx, 8
   mov [_z80_Read+edx*8], dword read_dispatcher
   mov bl, 0ffh
   sub ebx, 0100h

ar_end_done:

   sub ecx, eax
ar_loop:
; if there's no area left inbetween, stop now.
   mov edx, eax
   shr edx, 8

   cmp eax, ebx
   jg ar_finish

   cmp [esp+16], dword 0
   je ar_put_direct

   push ecx
   mov ecx, [esp+24]
   mov [_z80_Read+edx*8], ecx
   pop ecx
   add eax, 0100h
   jmp ar_loop

ar_put_direct:
   mov [_z80_Read+edx*8], dword 0
   mov [_z80_Read+edx*8+4], ecx
   add eax, 0100h
   jmp ar_loop

ar_finish:
   pop ebx
   ret

; void z80_add_write(UWORD start, UWORD end, int method, void *data)
; Add a WRITE memory handler to the memory map
; start/end = the area it covers
; method = 0 for direct, 1 for handled
; data = RAM for direct, handler for handled
global _z80_add_write
_z80_add_write:
   push ebx
   mov eax, [esp+8]
   mov ebx, [esp+12]
   mov ecx, [esp+20]

   ; put the entry in the read table first
   mov edx, _write_handlers
aw_find:
   add edx, 16
   cmp [edx-16], dword (-1)
   jne aw_find
   sub edx, 16

   ; put it in there
   mov [edx], eax
   mov [edx+4], ebx
   mov eax, [esp+16]
   mov [edx+8], eax

   mov eax, ecx         ; if direct, subtract the start from the memory area
   cmp [esp+16], dword 1
   je aw_dont_adjust
   sub eax, [esp+8]
aw_dont_adjust:
   mov [edx+12], eax
   
   mov eax, [esp+8]

; process the start, so it's page aligned
   test al, al
   jz aw_start_done

   ; put the start handler to go to the dispatcher
   mov edx, eax
   shr edx, 8
   mov [_z80_Write+edx*8], dword write_dispatcher
   xor al, al
   add eax, 0100h

aw_start_done:
; process the end, so it's page aligned
   cmp bl, 0ffh
   je aw_end_done

   ; put the end handler to go to the dispatcher
   mov edx, ebx
   shr edx, 8
   mov [_z80_Write+edx*8], dword write_dispatcher
   mov bl, 0ffh
   sub ebx, 0100h

aw_end_done:

   sub ecx, eax
aw_loop:
; if there's no area left inbetween, stop now.
   mov edx, eax
   shr edx, 8

   cmp eax, ebx
   jg aw_finish

   cmp [esp+16], dword 0
   je aw_put_direct

   push ecx
   mov ecx, [esp+24]
   mov [_z80_Write+edx*8], ecx
   pop ecx
   add eax, 0100h
   jmp aw_loop

aw_put_direct:
   mov [_z80_Write+edx*8], dword 0
   mov [_z80_Write+edx*8+4], ecx
   add eax, 0100h
   jmp aw_loop

aw_finish:
   pop ebx
   ret

; void z80_set_in(UBYTE (*handler)(UWORD port))
; Set the IN port handler to the given function
global _z80_set_in
_z80_set_in:
   mov eax, [esp+4]
   mov [_z80_In], eax
   ret

; void z80_set_out(void (*handler)(UWORD port, UBYTE value))
; Set the OUT port handler to the given function
global _z80_set_out
_z80_set_out:
   mov eax, [esp+4]
   mov [_z80_Out], eax
   ret

; void z80_set_reti(void (*handler)(void))
; Set the RETI handler to the given function
global _z80_set_reti
_z80_set_reti:
   mov eax, [esp+4]
   mov [_z80_RetI], eax
   ret

; void z80_set_fetch_callback(void (*handler)(UWORD pc))
; Set the fetch callback to the given function
global _z80_set_fetch_callback
_z80_set_fetch_callback:
   mov eax, [esp+4]
   mov [_z80_Fetch_Callback], eax
   ret

;----------------------------------------------------------------------------;
; the jump tables
OpTable:
   dd op_00, op_01, op_02, op_03, op_04, op_05, op_06, op_07
   dd op_08, op_09, op_0a, op_0b, op_0c, op_0d, op_0e, op_0f
   dd op_10, op_11, op_12, op_13, op_14, op_15, op_16, op_17
   dd op_18, op_19, op_1a, op_1b, op_1c, op_1d, op_1e, op_1f
   dd op_20, op_21, op_22, op_23, op_24, op_25, op_26, op_27
   dd op_28, op_29, op_2a, op_2b, op_2c, op_2d, op_2e, op_2f
   dd op_30, op_31, op_32, op_33, op_34, op_35, op_36, op_37
   dd op_38, op_39, op_3a, op_3b, op_3c, op_3d, op_3e, op_3f
   dd op_40, op_41, op_42, op_43, op_44, op_45, op_46, op_47
   dd op_48, op_49, op_4a, op_4b, op_4c, op_4d, op_4e, op_4f
   dd op_50, op_51, op_52, op_53, op_54, op_55, op_56, op_57
   dd op_58, op_59, op_5a, op_5b, op_5c, op_5d, op_5e, op_5f
   dd op_60, op_61, op_62, op_63, op_64, op_65, op_66, op_67
   dd op_68, op_69, op_6a, op_6b, op_6c, op_6d, op_6e, op_6f
   dd op_70, op_71, op_72, op_73, op_74, op_75, op_76, op_77
   dd op_78, op_79, op_7a, op_7b, op_7c, op_7d, op_7e, op_7f
   dd op_80, op_81, op_82, op_83, op_84, op_85, op_86, op_87
   dd op_88, op_89, op_8a, op_8b, op_8c, op_8d, op_8e, op_8f
   dd op_90, op_91, op_92, op_93, op_94, op_95, op_96, op_97
   dd op_98, op_99, op_9a, op_9b, op_9c, op_9d, op_9e, op_9f
   dd op_a0, op_a1, op_a2, op_a3, op_a4, op_a5, op_a6, op_a7
   dd op_a8, op_a9, op_aa, op_ab, op_ac, op_ad, op_ae, op_af
   dd op_b0, op_b1, op_b2, op_b3, op_b4, op_b5, op_b6, op_b7
   dd op_b8, op_b9, op_ba, op_bb, op_bc, op_bd, op_be, op_bf
   dd op_c0, op_c1, op_c2, op_c3, op_c4, op_c5, op_c6, op_c7
   dd op_c8, op_c9, op_ca, op_cb, op_cc, op_cd, op_ce, op_cf
   dd op_d0, op_d1, op_d2, op_d3, op_d4, op_d5, op_d6, op_d7
   dd op_d8, op_d9, op_da, op_db, op_dc, op_dd, op_de, op_df
   dd op_e0, op_e1, op_e2, op_e3, op_e4, op_e5, op_e6, op_e7
   dd op_e8, op_e9, op_ea, op_eb, op_ec, op_ed, op_ee, op_ef
   dd op_f0, op_f1, op_f2, op_f3, op_f4, op_f5, op_f6, op_f7
   dd op_f8, op_f9, op_fa, op_fb, op_fc, op_fd, op_fe, op_ff

CBTable:
   dd cb_00, cb_01, cb_02, cb_03, cb_04, cb_05, cb_06, cb_07
   dd cb_08, cb_09, cb_0a, cb_0b, cb_0c, cb_0d, cb_0e, cb_0f
   dd cb_10, cb_11, cb_12, cb_13, cb_14, cb_15, cb_16, cb_17
   dd cb_18, cb_19, cb_1a, cb_1b, cb_1c, cb_1d, cb_1e, cb_1f
   dd cb_20, cb_21, cb_22, cb_23, cb_24, cb_25, cb_26, cb_27
   dd cb_28, cb_29, cb_2a, cb_2b, cb_2c, cb_2d, cb_2e, cb_2f
   dd cb_30, cb_31, cb_32, cb_33, cb_34, cb_35, cb_36, cb_37
   dd cb_38, cb_39, cb_3a, cb_3b, cb_3c, cb_3d, cb_3e, cb_3f
   dd cb_40, cb_41, cb_42, cb_43, cb_44, cb_45, cb_46, cb_47
   dd cb_48, cb_49, cb_4a, cb_4b, cb_4c, cb_4d, cb_4e, cb_4f
   dd cb_50, cb_51, cb_52, cb_53, cb_54, cb_55, cb_56, cb_57
   dd cb_58, cb_59, cb_5a, cb_5b, cb_5c, cb_5d, cb_5e, cb_5f
   dd cb_60, cb_61, cb_62, cb_63, cb_64, cb_65, cb_66, cb_67
   dd cb_68, cb_69, cb_6a, cb_6b, cb_6c, cb_6d, cb_6e, cb_6f
   dd cb_70, cb_71, cb_72, cb_73, cb_74, cb_75, cb_76, cb_77
   dd cb_78, cb_79, cb_7a, cb_7b, cb_7c, cb_7d, cb_7e, cb_7f
   dd cb_80, cb_81, cb_82, cb_83, cb_84, cb_85, cb_86, cb_87
   dd cb_88, cb_89, cb_8a, cb_8b, cb_8c, cb_8d, cb_8e, cb_8f
   dd cb_90, cb_91, cb_92, cb_93, cb_94, cb_95, cb_96, cb_97
   dd cb_98, cb_99, cb_9a, cb_9b, cb_9c, cb_9d, cb_9e, cb_9f
   dd cb_a0, cb_a1, cb_a2, cb_a3, cb_a4, cb_a5, cb_a6, cb_a7
   dd cb_a8, cb_a9, cb_aa, cb_ab, cb_ac, cb_ad, cb_ae, cb_af
   dd cb_b0, cb_b1, cb_b2, cb_b3, cb_b4, cb_b5, cb_b6, cb_b7
   dd cb_b8, cb_b9, cb_ba, cb_bb, cb_bc, cb_bd, cb_be, cb_bf
   dd cb_c0, cb_c1, cb_c2, cb_c3, cb_c4, cb_c5, cb_c6, cb_c7
   dd cb_c8, cb_c9, cb_ca, cb_cb, cb_cc, cb_cd, cb_ce, cb_cf
   dd cb_d0, cb_d1, cb_d2, cb_d3, cb_d4, cb_d5, cb_d6, cb_d7
   dd cb_d8, cb_d9, cb_da, cb_db, cb_dc, cb_dd, cb_de, cb_df
   dd cb_e0, cb_e1, cb_e2, cb_e3, cb_e4, cb_e5, cb_e6, cb_e7
   dd cb_e8, cb_e9, cb_ea, cb_eb, cb_ec, cb_ed, cb_ee, cb_ef
   dd cb_f0, cb_f1, cb_f2, cb_f3, cb_f4, cb_f5, cb_f6, cb_f7
   dd cb_f8, cb_f9, cb_fa, cb_fb, cb_fc, cb_fd, cb_fe, cb_ff

EDTable:
   dd ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx
   dd ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx
   dd ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx
   dd ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx
   dd ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx
   dd ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx
   dd ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx
   dd ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx
   dd ed_40, ed_41, ed_42, ed_43, ed_44, ed_45, ed_46, ed_47
   dd ed_48, ed_49, ed_4a, ed_4b, ed_4c, ed_4d, ed_4e, ed_4f
   dd ed_50, ed_51, ed_52, ed_53, ed_54, ed_55, ed_56, ed_57
   dd ed_58, ed_59, ed_5a, ed_5b, ed_5c, ed_5d, ed_5e, ed_5f
   dd ed_60, ed_61, ed_62, ed_63, ed_64, ed_65, ed_66, ed_67
   dd ed_68, ed_69, ed_6a, ed_6b, ed_6c, ed_6d, ed_6e, ed_6f
   dd ed_70, ed_71, ed_72, ed_73, ed_74, ed_75, ed_76, ed_xx
   dd ed_78, ed_79, ed_7a, ed_7b, ed_7c, ed_7d, ed_7e, ed_xx
   dd ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx
   dd ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx
   dd ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx
   dd ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx
   dd ed_a0, ed_a1, ed_a2, ed_a3, ed_xx, ed_xx, ed_xx, ed_xx
   dd ed_a8, ed_a9, ed_aa, ed_ab, ed_xx, ed_xx, ed_xx, ed_xx
   dd ed_b0, ed_b1, ed_b2, ed_b3, ed_xx, ed_xx, ed_xx, ed_xx
   dd ed_b8, ed_b9, ed_ba, ed_bb, ed_xx, ed_xx, ed_xx, ed_xx
   dd ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx
   dd ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx
   dd ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx
   dd ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx
   dd ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx
   dd ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx
   dd ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx
   dd ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx, ed_xx

DDTable:
   dd dd_xx, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx
   dd dd_xx, dd_09, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx
   dd dd_xx, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx
   dd dd_xx, dd_19, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx
   dd dd_xx, dd_21, dd_22, dd_23, dd_24, dd_25, dd_26, dd_xx
   dd dd_xx, dd_29, dd_2a, dd_2b, dd_2c, dd_2d, dd_2e, dd_xx
   dd dd_xx, dd_xx, dd_xx, dd_xx, dd_34, dd_35, dd_36, dd_xx
   dd dd_xx, dd_39, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx
   dd dd_xx, dd_xx, dd_xx, dd_xx, dd_44, dd_45, dd_46, dd_xx
   dd dd_xx, dd_xx, dd_xx, dd_xx, dd_4c, dd_4d, dd_4e, dd_xx
   dd dd_xx, dd_xx, dd_xx, dd_xx, dd_54, dd_55, dd_56, dd_xx
   dd dd_xx, dd_xx, dd_xx, dd_xx, dd_5c, dd_5d, dd_5e, dd_xx
   dd dd_60, dd_61, dd_62, dd_63, dd_xx, dd_65, dd_66, dd_67
   dd dd_68, dd_69, dd_6a, dd_6b, dd_6c, dd_xx, dd_6e, dd_6f
   dd dd_70, dd_71, dd_72, dd_73, dd_74, dd_75, dd_xx, dd_77
   dd dd_xx, dd_xx, dd_xx, dd_xx, dd_7c, dd_7d, dd_7e, dd_xx
   dd dd_xx, dd_xx, dd_xx, dd_xx, dd_84, dd_85, dd_86, dd_xx
   dd dd_xx, dd_xx, dd_xx, dd_xx, dd_8c, dd_8d, dd_8e, dd_xx
   dd dd_xx, dd_xx, dd_xx, dd_xx, dd_94, dd_95, dd_96, dd_xx
   dd dd_xx, dd_xx, dd_xx, dd_xx, dd_9c, dd_9d, dd_9e, dd_xx
   dd dd_xx, dd_xx, dd_xx, dd_xx, dd_a4, dd_a5, dd_a6, dd_xx
   dd dd_xx, dd_xx, dd_xx, dd_xx, dd_ac, dd_ad, dd_ae, dd_xx
   dd dd_xx, dd_xx, dd_xx, dd_xx, dd_b4, dd_b5, dd_b6, dd_xx
   dd dd_xx, dd_xx, dd_xx, dd_xx, dd_bc, dd_bd, dd_be, dd_xx
   dd dd_xx, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx
   dd dd_xx, dd_xx, dd_xx, dd_cb, dd_xx, dd_xx, dd_xx, dd_xx
   dd dd_xx, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx
   dd dd_xx, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx
   dd dd_xx, dd_e1, dd_xx, dd_e3, dd_xx, dd_e5, dd_xx, dd_xx
   dd dd_xx, dd_e9, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx
   dd dd_xx, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx
   dd dd_xx, dd_f9, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx, dd_xx

FDTable:
   dd fd_xx, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx
   dd fd_xx, fd_09, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx
   dd fd_xx, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx
   dd fd_xx, fd_19, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx
   dd fd_xx, fd_21, fd_22, fd_23, fd_24, fd_25, fd_26, fd_xx
   dd fd_xx, fd_29, fd_2a, fd_2b, fd_2c, fd_2d, fd_2e, fd_xx
   dd fd_xx, fd_xx, fd_xx, fd_xx, fd_34, fd_35, fd_36, fd_xx
   dd fd_xx, fd_39, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx
   dd fd_xx, fd_xx, fd_xx, fd_xx, fd_44, fd_45, fd_46, fd_xx
   dd fd_xx, fd_xx, fd_xx, fd_xx, fd_4c, fd_4d, fd_4e, fd_xx
   dd fd_xx, fd_xx, fd_xx, fd_xx, fd_54, fd_55, fd_56, fd_xx
   dd fd_xx, fd_xx, fd_xx, fd_xx, fd_5c, fd_5d, fd_5e, fd_xx
   dd fd_60, fd_61, fd_62, fd_63, fd_xx, fd_65, fd_66, fd_67
   dd fd_68, fd_69, fd_6a, fd_6b, fd_6c, fd_xx, fd_6e, fd_6f
   dd fd_70, fd_71, fd_72, fd_73, fd_74, fd_75, fd_xx, fd_77
   dd fd_xx, fd_xx, fd_xx, fd_xx, fd_7c, fd_7d, fd_7e, fd_xx
   dd fd_xx, fd_xx, fd_xx, fd_xx, fd_84, fd_85, fd_86, fd_xx
   dd fd_xx, fd_xx, fd_xx, fd_xx, fd_8c, fd_8d, fd_8e, fd_xx
   dd fd_xx, fd_xx, fd_xx, fd_xx, fd_94, fd_95, fd_96, fd_xx
   dd fd_xx, fd_xx, fd_xx, fd_xx, fd_9c, fd_9d, fd_9e, fd_xx
   dd fd_xx, fd_xx, fd_xx, fd_xx, fd_a4, fd_a5, fd_a6, fd_xx
   dd fd_xx, fd_xx, fd_xx, fd_xx, fd_ac, fd_ad, fd_ae, fd_xx
   dd fd_xx, fd_xx, fd_xx, fd_xx, fd_b4, fd_b5, fd_b6, fd_xx
   dd fd_xx, fd_xx, fd_xx, fd_xx, fd_bc, fd_bd, fd_be, fd_xx
   dd fd_xx, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx
   dd fd_xx, fd_xx, fd_xx, fd_cb, fd_xx, fd_xx, fd_xx, fd_xx
   dd fd_xx, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx
   dd fd_xx, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx
   dd fd_xx, fd_e1, fd_xx, fd_e3, fd_xx, fd_e5, fd_xx, fd_xx
   dd fd_xx, fd_e9, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx
   dd fd_xx, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx
   dd fd_xx, fd_f9, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx, fd_xx

DDCBTable:
   dd ddcb_00, ddcb_01, ddcb_02, ddcb_03, ddcb_04, ddcb_05, ddcb_06, ddcb_07
   dd ddcb_08, ddcb_09, ddcb_0a, ddcb_0b, ddcb_0c, ddcb_0d, ddcb_0e, ddcb_0f
   dd ddcb_10, ddcb_11, ddcb_12, ddcb_13, ddcb_14, ddcb_15, ddcb_16, ddcb_17
   dd ddcb_18, ddcb_19, ddcb_1a, ddcb_1b, ddcb_1c, ddcb_1d, ddcb_1e, ddcb_1f
   dd ddcb_20, ddcb_21, ddcb_22, ddcb_23, ddcb_24, ddcb_25, ddcb_26, ddcb_27
   dd ddcb_28, ddcb_29, ddcb_2a, ddcb_2b, ddcb_2c, ddcb_2d, ddcb_2e, ddcb_2f
   dd ddcb_30, ddcb_31, ddcb_32, ddcb_33, ddcb_34, ddcb_35, ddcb_36, ddcb_37
   dd ddcb_38, ddcb_39, ddcb_3a, ddcb_3b, ddcb_3c, ddcb_3d, ddcb_3e, ddcb_3f
   dd ddcb_40, ddcb_41, ddcb_42, ddcb_43, ddcb_44, ddcb_45, ddcb_46, ddcb_47
   dd ddcb_48, ddcb_49, ddcb_4a, ddcb_4b, ddcb_4c, ddcb_4d, ddcb_4e, ddcb_4f
   dd ddcb_50, ddcb_51, ddcb_52, ddcb_53, ddcb_54, ddcb_55, ddcb_56, ddcb_57
   dd ddcb_58, ddcb_59, ddcb_5a, ddcb_5b, ddcb_5c, ddcb_5d, ddcb_5e, ddcb_5f
   dd ddcb_60, ddcb_61, ddcb_62, ddcb_63, ddcb_64, ddcb_65, ddcb_66, ddcb_67
   dd ddcb_68, ddcb_69, ddcb_6a, ddcb_6b, ddcb_6c, ddcb_6d, ddcb_6e, ddcb_6f
   dd ddcb_70, ddcb_71, ddcb_72, ddcb_73, ddcb_74, ddcb_75, ddcb_76, ddcb_77
   dd ddcb_78, ddcb_79, ddcb_7a, ddcb_7b, ddcb_7c, ddcb_7d, ddcb_7e, ddcb_7f
   dd ddcb_80, ddcb_81, ddcb_82, ddcb_83, ddcb_84, ddcb_85, ddcb_86, ddcb_87
   dd ddcb_88, ddcb_89, ddcb_8a, ddcb_8b, ddcb_8c, ddcb_8d, ddcb_8e, ddcb_8f
   dd ddcb_90, ddcb_91, ddcb_92, ddcb_93, ddcb_94, ddcb_95, ddcb_96, ddcb_97
   dd ddcb_98, ddcb_99, ddcb_9a, ddcb_9b, ddcb_9c, ddcb_9d, ddcb_9e, ddcb_9f
   dd ddcb_a0, ddcb_a1, ddcb_a2, ddcb_a3, ddcb_a4, ddcb_a5, ddcb_a6, ddcb_a7
   dd ddcb_a8, ddcb_a9, ddcb_aa, ddcb_ab, ddcb_ac, ddcb_ad, ddcb_ae, ddcb_af
   dd ddcb_b0, ddcb_b1, ddcb_b2, ddcb_b3, ddcb_b4, ddcb_b5, ddcb_b6, ddcb_b7
   dd ddcb_b8, ddcb_b9, ddcb_ba, ddcb_bb, ddcb_bc, ddcb_bd, ddcb_be, ddcb_bf
   dd ddcb_c0, ddcb_c1, ddcb_c2, ddcb_c3, ddcb_c4, ddcb_c5, ddcb_c6, ddcb_c7
   dd ddcb_c8, ddcb_c9, ddcb_ca, ddcb_cb, ddcb_cc, ddcb_cd, ddcb_ce, ddcb_cf
   dd ddcb_d0, ddcb_d1, ddcb_d2, ddcb_d3, ddcb_d4, ddcb_d5, ddcb_d6, ddcb_d7
   dd ddcb_d8, ddcb_d9, ddcb_da, ddcb_db, ddcb_dc, ddcb_dd, ddcb_de, ddcb_df
   dd ddcb_e0, ddcb_e1, ddcb_e2, ddcb_e3, ddcb_e4, ddcb_e5, ddcb_e6, ddcb_e7
   dd ddcb_e8, ddcb_e9, ddcb_ea, ddcb_eb, ddcb_ec, ddcb_ed, ddcb_ee, ddcb_ef
   dd ddcb_f0, ddcb_f1, ddcb_f2, ddcb_f3, ddcb_f4, ddcb_f5, ddcb_f6, ddcb_f7
   dd ddcb_f8, ddcb_f9, ddcb_fa, ddcb_fb, ddcb_fc, ddcb_fd, ddcb_fe, ddcb_ff


;----------------------------------------------------------------------------;
; flag tables ('borrowed' from MAZE, by Ishmair)
INC_Table:
DB  0,0,0,0,0,0,0,8,8,8,8,8,8,8,8,16,0,0,0,0
DB  0,0,0,8,8,8,8,8,8,8,8,48,32,32,32,32,32,32,32,40
DB  40,40,40,40,40,40,40,48,32,32,32,32,32,32,32,40,40,40,40,40
DB  40,40,40,16,0,0,0,0,0,0,0,8,8,8,8,8,8,8,8,16
DB  0,0,0,0,0,0,0,8,8,8,8,8,8,8,8,48,32,32,32,32
DB  32,32,32,40,40,40,40,40,40,40,40,48,32,32,32,32,32,32,32,40
DB  40,40,40,40,40,40,40,148,128,128,128,128,128,128,128,136,136,136,136,136
DB  136,136,136,144,128,128,128,128,128,128,128,136,136,136,136,136,136,136,136,176
DB  160,160,160,160,160,160,160,168,168,168,168,168,168,168,168,176,160,160,160,160
DB  160,160,160,168,168,168,168,168,168,168,168,144,128,128,128,128,128,128,128,136
DB  136,136,136,136,136,136,136,144,128,128,128,128,128,128,128,136,136,136,136,136
DB  136,136,136,176,160,160,160,160,160,160,160,168,168,168,168,168,168,168,168,176
DB  160,160,160,160,160,160,160,168,168,168,168,168,168,168,168,80

DEC_Table:
DB  186,66,2,2,2,2,2,2,2,10,10,10,10,10,10,10,26,2,2,2
DB  2,2,2,2,2,10,10,10,10,10,10,10,26,34,34,34,34,34,34,34
DB  34,42,42,42,42,42,42,42,58,34,34,34,34,34,34,34,34,42,42,42
DB  42,42,42,42,58,2,2,2,2,2,2,2,2,10,10,10,10,10,10,10
DB  26,2,2,2,2,2,2,2,2,10,10,10,10,10,10,10,26,34,34,34
DB  34,34,34,34,34,42,42,42,42,42,42,42,58,34,34,34,34,34,34,34
DB  34,42,42,42,42,42,42,42,62,130,130,130,130,130,130,130,130,138,138,138
DB  138,138,138,138,154,130,130,130,130,130,130,130,130,138,138,138,138,138,138,138
DB  154,162,162,162,162,162,162,162,162,170,170,170,170,170,170,170,186,162,162,162
DB  162,162,162,162,162,170,170,170,170,170,170,170,186,130,130,130,130,130,130,130
DB  130,138,138,138,138,138,138,138,154,130,130,130,130,130,130,130,130,138,138,138
DB  138,138,138,138,154,162,162,162,162,162,162,162,162,170,170,170,170,170,170,170
DB  186,162,162,162,162,162,162,162,162,170,170,170,170,170,170,170

; DAA table ('borrowed' from Z80Emul, by unknown)
DAA_Table:
DW  04400h, 00001h, 00002h, 00403h, 00004h, 00405h, 00406h, 00007h
DW  00808h, 00C09h, 01010h, 01411h, 01412h, 01013h, 01414h, 01015h
DW  00010h, 00411h, 00412h, 00013h, 00414h, 00015h, 00016h, 00417h
DW  00C18h, 00819h, 03020h, 03421h, 03422h, 03023h, 03424h, 03025h
DW  02020h, 02421h, 02422h, 02023h, 02424h, 02025h, 02026h, 02427h
DW  02C28h, 02829h, 03430h, 03031h, 03032h, 03433h, 03034h, 03435h
DW  02430h, 02031h, 02032h, 02433h, 02034h, 02435h, 02436h, 02037h
DW  02838h, 02C39h, 01040h, 01441h, 01442h, 01043h, 01444h, 01045h
DW  00040h, 00441h, 00442h, 00043h, 00444h, 00045h, 00046h, 00447h
DW  00C48h, 00849h, 01450h, 01051h, 01052h, 01453h, 01054h, 01455h
DW  00450h, 00051h, 00052h, 00453h, 00054h, 00455h, 00456h, 00057h
DW  00858h, 00C59h, 03460h, 03061h, 03062h, 03463h, 03064h, 03465h
DW  02460h, 02061h, 02062h, 02463h, 02064h, 02465h, 02466h, 02067h
DW  02868h, 02C69h, 03070h, 03471h, 03472h, 03073h, 03474h, 03075h
DW  02070h, 02471h, 02472h, 02073h, 02474h, 02075h, 02076h, 02477h
DW  02C78h, 02879h, 09080h, 09481h, 09482h, 09083h, 09484h, 09085h
DW  08080h, 08481h, 08482h, 08083h, 08484h, 08085h, 08086h, 08487h
DW  08C88h, 08889h, 09490h, 09091h, 09092h, 09493h, 09094h, 09495h
DW  08490h, 08091h, 08092h, 08493h, 08094h, 08495h, 08496h, 08097h
DW  08898h, 08C99h, 05500h, 01101h, 01102h, 01503h, 01104h, 01505h
DW  04500h, 00101h, 00102h, 00503h, 00104h, 00505h, 00506h, 00107h
DW  00908h, 00D09h, 01110h, 01511h, 01512h, 01113h, 01514h, 01115h
DW  00110h, 00511h, 00512h, 00113h, 00514h, 00115h, 00116h, 00517h
DW  00D18h, 00919h, 03120h, 03521h, 03522h, 03123h, 03524h, 03125h
DW  02120h, 02521h, 02522h, 02123h, 02524h, 02125h, 02126h, 02527h
DW  02D28h, 02929h, 03530h, 03131h, 03132h, 03533h, 03134h, 03535h
DW  02530h, 02131h, 02132h, 02533h, 02134h, 02535h, 02536h, 02137h
DW  02938h, 02D39h, 01140h, 01541h, 01542h, 01143h, 01544h, 01145h
DW  00140h, 00541h, 00542h, 00143h, 00544h, 00145h, 00146h, 00547h
DW  00D48h, 00949h, 01550h, 01151h, 01152h, 01553h, 01154h, 01555h
DW  00550h, 00151h, 00152h, 00553h, 00154h, 00555h, 00556h, 00157h
DW  00958h, 00D59h, 03560h, 03161h, 03162h, 03563h, 03164h, 03565h
DW  02560h, 02161h, 02162h, 02563h, 02164h, 02565h, 02566h, 02167h
DW  02968h, 02D69h, 03170h, 03571h, 03572h, 03173h, 03574h, 03175h
DW  02170h, 02571h, 02572h, 02173h, 02574h, 02175h, 02176h, 02577h
DW  02D78h, 02979h, 09180h, 09581h, 09582h, 09183h, 09584h, 09185h
DW  08180h, 08581h, 08582h, 08183h, 08584h, 08185h, 08186h, 08587h
DW  08D88h, 08989h, 09590h, 09191h, 09192h, 09593h, 09194h, 09595h
DW  08590h, 08191h, 08192h, 08593h, 08194h, 08595h, 08596h, 08197h
DW  08998h, 08D99h, 0B5A0h, 0B1A1h, 0B1A2h, 0B5A3h, 0B1A4h, 0B5A5h
DW  0A5A0h, 0A1A1h, 0A1A2h, 0A5A3h, 0A1A4h, 0A5A5h, 0A5A6h, 0A1A7h
DW  0A9A8h, 0ADA9h, 0B1B0h, 0B5B1h, 0B5B2h, 0B1B3h, 0B5B4h, 0B1B5h
DW  0A1B0h, 0A5B1h, 0A5B2h, 0A1B3h, 0A5B4h, 0A1B5h, 0A1B6h, 0A5B7h
DW  0ADB8h, 0A9B9h, 095C0h, 091C1h, 091C2h, 095C3h, 091C4h, 095C5h
DW  085C0h, 081C1h, 081C2h, 085C3h, 081C4h, 085C5h, 085C6h, 081C7h
DW  089C8h, 08DC9h, 091D0h, 095D1h, 095D2h, 091D3h, 095D4h, 091D5h
DW  081D0h, 085D1h, 085D2h, 081D3h, 085D4h, 081D5h, 081D6h, 085D7h
DW  08DD8h, 089D9h, 0B1E0h, 0B5E1h, 0B5E2h, 0B1E3h, 0B5E4h, 0B1E5h
DW  0A1E0h, 0A5E1h, 0A5E2h, 0A1E3h, 0A5E4h, 0A1E5h, 0A1E6h, 0A5E7h
DW  0ADE8h, 0A9E9h, 0B5F0h, 0B1F1h, 0B1F2h, 0B5F3h, 0B1F4h, 0B5F5h
DW  0A5F0h, 0A1F1h, 0A1F2h, 0A5F3h, 0A1F4h, 0A5F5h, 0A5F6h, 0A1F7h
DW  0A9F8h, 0ADF9h, 05500h, 01101h, 01102h, 01503h, 01104h, 01505h
DW  04500h, 00101h, 00102h, 00503h, 00104h, 00505h, 00506h, 00107h
DW  00908h, 00D09h, 01110h, 01511h, 01512h, 01113h, 01514h, 01115h
DW  00110h, 00511h, 00512h, 00113h, 00514h, 00115h, 00116h, 00517h
DW  00D18h, 00919h, 03120h, 03521h, 03522h, 03123h, 03524h, 03125h
DW  02120h, 02521h, 02522h, 02123h, 02524h, 02125h, 02126h, 02527h
DW  02D28h, 02929h, 03530h, 03131h, 03132h, 03533h, 03134h, 03535h
DW  02530h, 02131h, 02132h, 02533h, 02134h, 02535h, 02536h, 02137h
DW  02938h, 02D39h, 01140h, 01541h, 01542h, 01143h, 01544h, 01145h
DW  00140h, 00541h, 00542h, 00143h, 00544h, 00145h, 00146h, 00547h
DW  00D48h, 00949h, 01550h, 01151h, 01152h, 01553h, 01154h, 01555h
DW  00550h, 00151h, 00152h, 00553h, 00154h, 00555h, 00556h, 00157h
DW  00958h, 00D59h, 03560h, 03161h, 03162h, 03563h, 03164h, 03565h
DW  04600h, 00201h, 00202h, 00603h, 00204h, 00605h, 00606h, 00207h
DW  00A08h, 00E09h, 00204h, 00605h, 00606h, 00207h, 00A08h, 00E09h
DW  00210h, 00611h, 00612h, 00213h, 00614h, 00215h, 00216h, 00617h
DW  00E18h, 00A19h, 00614h, 00215h, 00216h, 00617h, 00E18h, 00A19h
DW  02220h, 02621h, 02622h, 02223h, 02624h, 02225h, 02226h, 02627h
DW  02E28h, 02A29h, 02624h, 02225h, 02226h, 02627h, 02E28h, 02A29h
DW  02630h, 02231h, 02232h, 02633h, 02234h, 02635h, 02636h, 02237h
DW  02A38h, 02E39h, 02234h, 02635h, 02636h, 02237h, 02A38h, 02E39h
DW  00240h, 00641h, 00642h, 00243h, 00644h, 00245h, 00246h, 00647h
DW  00E48h, 00A49h, 00644h, 00245h, 00246h, 00647h, 00E48h, 00A49h
DW  00650h, 00251h, 00252h, 00653h, 00254h, 00655h, 00656h, 00257h
DW  00A58h, 00E59h, 00254h, 00655h, 00656h, 00257h, 00A58h, 00E59h
DW  02660h, 02261h, 02262h, 02663h, 02264h, 02665h, 02666h, 02267h
DW  02A68h, 02E69h, 02264h, 02665h, 02666h, 02267h, 02A68h, 02E69h
DW  02270h, 02671h, 02672h, 02273h, 02674h, 02275h, 02276h, 02677h
DW  02E78h, 02A79h, 02674h, 02275h, 02276h, 02677h, 02E78h, 02A79h
DW  08280h, 08681h, 08682h, 08283h, 08684h, 08285h, 08286h, 08687h
DW  08E88h, 08A89h, 08684h, 08285h, 08286h, 08687h, 08E88h, 08A89h
DW  08690h, 08291h, 08292h, 08693h, 08294h, 08695h, 08696h, 08297h
DW  08A98h, 08E99h, 02334h, 02735h, 02736h, 02337h, 02B38h, 02F39h
DW  00340h, 00741h, 00742h, 00343h, 00744h, 00345h, 00346h, 00747h
DW  00F48h, 00B49h, 00744h, 00345h, 00346h, 00747h, 00F48h, 00B49h
DW  00750h, 00351h, 00352h, 00753h, 00354h, 00755h, 00756h, 00357h
DW  00B58h, 00F59h, 00354h, 00755h, 00756h, 00357h, 00B58h, 00F59h
DW  02760h, 02361h, 02362h, 02763h, 02364h, 02765h, 02766h, 02367h
DW  02B68h, 02F69h, 02364h, 02765h, 02766h, 02367h, 02B68h, 02F69h
DW  02370h, 02771h, 02772h, 02373h, 02774h, 02375h, 02376h, 02777h
DW  02F78h, 02B79h, 02774h, 02375h, 02376h, 02777h, 02F78h, 02B79h
DW  08380h, 08781h, 08782h, 08383h, 08784h, 08385h, 08386h, 08787h
DW  08F88h, 08B89h, 08784h, 08385h, 08386h, 08787h, 08F88h, 08B89h
DW  08790h, 08391h, 08392h, 08793h, 08394h, 08795h, 08796h, 08397h
DW  08B98h, 08F99h, 08394h, 08795h, 08796h, 08397h, 08B98h, 08F99h
DW  0A7A0h, 0A3A1h, 0A3A2h, 0A7A3h, 0A3A4h, 0A7A5h, 0A7A6h, 0A3A7h
DW  0ABA8h, 0AFA9h, 0A3A4h, 0A7A5h, 0A7A6h, 0A3A7h, 0ABA8h, 0AFA9h
DW  0A3B0h, 0A7B1h, 0A7B2h, 0A3B3h, 0A7B4h, 0A3B5h, 0A3B6h, 0A7B7h
DW  0AFB8h, 0ABB9h, 0A7B4h, 0A3B5h, 0A3B6h, 0A7B7h, 0AFB8h, 0ABB9h
DW  087C0h, 083C1h, 083C2h, 087C3h, 083C4h, 087C5h, 087C6h, 083C7h
DW  08BC8h, 08FC9h, 083C4h, 087C5h, 087C6h, 083C7h, 08BC8h, 08FC9h
DW  083D0h, 087D1h, 087D2h, 083D3h, 087D4h, 083D5h, 083D6h, 087D7h
DW  08FD8h, 08BD9h, 087D4h, 083D5h, 083D6h, 087D7h, 08FD8h, 08BD9h
DW  0A3E0h, 0A7E1h, 0A7E2h, 0A3E3h, 0A7E4h, 0A3E5h, 0A3E6h, 0A7E7h
DW  0AFE8h, 0ABE9h, 0A7E4h, 0A3E5h, 0A3E6h, 0A7E7h, 0AFE8h, 0ABE9h
DW  0A7F0h, 0A3F1h, 0A3F2h, 0A7F3h, 0A3F4h, 0A7F5h, 0A7F6h, 0A3F7h
DW  0ABF8h, 0AFF9h, 0A3F4h, 0A7F5h, 0A7F6h, 0A3F7h, 0ABF8h, 0AFF9h
DW  04700h, 00301h, 00302h, 00703h, 00304h, 00705h, 00706h, 00307h
DW  00B08h, 00F09h, 00304h, 00705h, 00706h, 00307h, 00B08h, 00F09h
DW  00310h, 00711h, 00712h, 00313h, 00714h, 00315h, 00316h, 00717h
DW  00F18h, 00B19h, 00714h, 00315h, 00316h, 00717h, 00F18h, 00B19h
DW  02320h, 02721h, 02722h, 02323h, 02724h, 02325h, 02326h, 02727h
DW  02F28h, 02B29h, 02724h, 02325h, 02326h, 02727h, 02F28h, 02B29h
DW  02730h, 02331h, 02332h, 02733h, 02334h, 02735h, 02736h, 02337h
DW  02B38h, 02F39h, 02334h, 02735h, 02736h, 02337h, 02B38h, 02F39h
DW  00340h, 00741h, 00742h, 00343h, 00744h, 00345h, 00346h, 00747h
DW  00F48h, 00B49h, 00744h, 00345h, 00346h, 00747h, 00F48h, 00B49h
DW  00750h, 00351h, 00352h, 00753h, 00354h, 00755h, 00756h, 00357h
DW  00B58h, 00F59h, 00354h, 00755h, 00756h, 00357h, 00B58h, 00F59h
DW  02760h, 02361h, 02362h, 02763h, 02364h, 02765h, 02766h, 02367h
DW  02B68h, 02F69h, 02364h, 02765h, 02766h, 02367h, 02B68h, 02F69h
DW  02370h, 02771h, 02772h, 02373h, 02774h, 02375h, 02376h, 02777h
DW  02F78h, 02B79h, 02774h, 02375h, 02376h, 02777h, 02F78h, 02B79h
DW  08380h, 08781h, 08782h, 08383h, 08784h, 08385h, 08386h, 08787h
DW  08F88h, 08B89h, 08784h, 08385h, 08386h, 08787h, 08F88h, 08B89h
DW  08790h, 08391h, 08392h, 08793h, 08394h, 08795h, 08796h, 08397h
DW  08B98h, 08F99h, 08394h, 08795h, 08796h, 08397h, 08B98h, 08F99h
DW  00406h, 00007h, 00808h, 00C09h, 00C0Ah, 0080Bh, 00C0Ch, 0080Dh
DW  0080Eh, 00C0Fh, 01010h, 01411h, 01412h, 01013h, 01414h, 01015h
DW  00016h, 00417h, 00C18h, 00819h, 0081Ah, 00C1Bh, 0081Ch, 00C1Dh
DW  00C1Eh, 0081Fh, 03020h, 03421h, 03422h, 03023h, 03424h, 03025h
DW  02026h, 02427h, 02C28h, 02829h, 0282Ah, 02C2Bh, 0282Ch, 02C2Dh
DW  02C2Eh, 0282Fh, 03430h, 03031h, 03032h, 03433h, 03034h, 03435h
DW  02436h, 02037h, 02838h, 02C39h, 02C3Ah, 0283Bh, 02C3Ch, 0283Dh
DW  0283Eh, 02C3Fh, 01040h, 01441h, 01442h, 01043h, 01444h, 01045h
DW  00046h, 00447h, 00C48h, 00849h, 0084Ah, 00C4Bh, 0084Ch, 00C4Dh
DW  00C4Eh, 0084Fh, 01450h, 01051h, 01052h, 01453h, 01054h, 01455h
DW  00456h, 00057h, 00858h, 00C59h, 00C5Ah, 0085Bh, 00C5Ch, 0085Dh
DW  0085Eh, 00C5Fh, 03460h, 03061h, 03062h, 03463h, 03064h, 03465h
DW  02466h, 02067h, 02868h, 02C69h, 02C6Ah, 0286Bh, 02C6Ch, 0286Dh
DW  0286Eh, 02C6Fh, 03070h, 03471h, 03472h, 03073h, 03474h, 03075h
DW  02076h, 02477h, 02C78h, 02879h, 0287Ah, 02C7Bh, 0287Ch, 02C7Dh
DW  02C7Eh, 0287Fh, 09080h, 09481h, 09482h, 09083h, 09484h, 09085h
DW  08086h, 08487h, 08C88h, 08889h, 0888Ah, 08C8Bh, 0888Ch, 08C8Dh
DW  08C8Eh, 0888Fh, 09490h, 09091h, 09092h, 09493h, 09094h, 09495h
DW  08496h, 08097h, 08898h, 08C99h, 08C9Ah, 0889Bh, 08C9Ch, 0889Dh
DW  0889Eh, 08C9Fh, 05500h, 01101h, 01102h, 01503h, 01104h, 01505h
DW  00506h, 00107h, 00908h, 00D09h, 00D0Ah, 0090Bh, 00D0Ch, 0090Dh
DW  0090Eh, 00D0Fh, 01110h, 01511h, 01512h, 01113h, 01514h, 01115h
DW  00116h, 00517h, 00D18h, 00919h, 0091Ah, 00D1Bh, 0091Ch, 00D1Dh
DW  00D1Eh, 0091Fh, 03120h, 03521h, 03522h, 03123h, 03524h, 03125h
DW  02126h, 02527h, 02D28h, 02929h, 0292Ah, 02D2Bh, 0292Ch, 02D2Dh
DW  02D2Eh, 0292Fh, 03530h, 03131h, 03132h, 03533h, 03134h, 03535h
DW  02536h, 02137h, 02938h, 02D39h, 02D3Ah, 0293Bh, 02D3Ch, 0293Dh
DW  0293Eh, 02D3Fh, 01140h, 01541h, 01542h, 01143h, 01544h, 01145h
DW  00146h, 00547h, 00D48h, 00949h, 0094Ah, 00D4Bh, 0094Ch, 00D4Dh
DW  00D4Eh, 0094Fh, 01550h, 01151h, 01152h, 01553h, 01154h, 01555h
DW  00556h, 00157h, 00958h, 00D59h, 00D5Ah, 0095Bh, 00D5Ch, 0095Dh
DW  0095Eh, 00D5Fh, 03560h, 03161h, 03162h, 03563h, 03164h, 03565h
DW  02566h, 02167h, 02968h, 02D69h, 02D6Ah, 0296Bh, 02D6Ch, 0296Dh
DW  0296Eh, 02D6Fh, 03170h, 03571h, 03572h, 03173h, 03574h, 03175h
DW  02176h, 02577h, 02D78h, 02979h, 0297Ah, 02D7Bh, 0297Ch, 02D7Dh
DW  02D7Eh, 0297Fh, 09180h, 09581h, 09582h, 09183h, 09584h, 09185h
DW  08186h, 08587h, 08D88h, 08989h, 0898Ah, 08D8Bh, 0898Ch, 08D8Dh
DW  08D8Eh, 0898Fh, 09590h, 09191h, 09192h, 09593h, 09194h, 09595h
DW  08596h, 08197h, 08998h, 08D99h, 08D9Ah, 0899Bh, 08D9Ch, 0899Dh
DW  0899Eh, 08D9Fh, 0B5A0h, 0B1A1h, 0B1A2h, 0B5A3h, 0B1A4h, 0B5A5h
DW  0A5A6h, 0A1A7h, 0A9A8h, 0ADA9h, 0ADAAh, 0A9ABh, 0ADACh, 0A9ADh
DW  0A9AEh, 0ADAFh, 0B1B0h, 0B5B1h, 0B5B2h, 0B1B3h, 0B5B4h, 0B1B5h
DW  0A1B6h, 0A5B7h, 0ADB8h, 0A9B9h, 0A9BAh, 0ADBBh, 0A9BCh, 0ADBDh
DW  0ADBEh, 0A9BFh, 095C0h, 091C1h, 091C2h, 095C3h, 091C4h, 095C5h
DW  085C6h, 081C7h, 089C8h, 08DC9h, 08DCAh, 089CBh, 08DCCh, 089CDh
DW  089CEh, 08DCFh, 091D0h, 095D1h, 095D2h, 091D3h, 095D4h, 091D5h
DW  081D6h, 085D7h, 08DD8h, 089D9h, 089DAh, 08DDBh, 089DCh, 08DDDh
DW  08DDEh, 089DFh, 0B1E0h, 0B5E1h, 0B5E2h, 0B1E3h, 0B5E4h, 0B1E5h
DW  0A1E6h, 0A5E7h, 0ADE8h, 0A9E9h, 0A9EAh, 0ADEBh, 0A9ECh, 0ADEDh
DW  0ADEEh, 0A9EFh, 0B5F0h, 0B1F1h, 0B1F2h, 0B5F3h, 0B1F4h, 0B5F5h
DW  0A5F6h, 0A1F7h, 0A9F8h, 0ADF9h, 0ADFAh, 0A9FBh, 0ADFCh, 0A9FDh
DW  0A9FEh, 0ADFFh, 05500h, 01101h, 01102h, 01503h, 01104h, 01505h
DW  00506h, 00107h, 00908h, 00D09h, 00D0Ah, 0090Bh, 00D0Ch, 0090Dh
DW  0090Eh, 00D0Fh, 01110h, 01511h, 01512h, 01113h, 01514h, 01115h
DW  00116h, 00517h, 00D18h, 00919h, 0091Ah, 00D1Bh, 0091Ch, 00D1Dh
DW  00D1Eh, 0091Fh, 03120h, 03521h, 03522h, 03123h, 03524h, 03125h
DW  02126h, 02527h, 02D28h, 02929h, 0292Ah, 02D2Bh, 0292Ch, 02D2Dh
DW  02D2Eh, 0292Fh, 03530h, 03131h, 03132h, 03533h, 03134h, 03535h
DW  02536h, 02137h, 02938h, 02D39h, 02D3Ah, 0293Bh, 02D3Ch, 0293Dh
DW  0293Eh, 02D3Fh, 01140h, 01541h, 01542h, 01143h, 01544h, 01145h
DW  00146h, 00547h, 00D48h, 00949h, 0094Ah, 00D4Bh, 0094Ch, 00D4Dh
DW  00D4Eh, 0094Fh, 01550h, 01151h, 01152h, 01553h, 01154h, 01555h
DW  00556h, 00157h, 00958h, 00D59h, 00D5Ah, 0095Bh, 00D5Ch, 0095Dh
DW  0095Eh, 00D5Fh, 03560h, 03161h, 03162h, 03563h, 03164h, 03565h
DW  0BEFAh, 0BAFBh, 0BEFCh, 0BAFDh, 0BAFEh, 0BEFFh, 04600h, 00201h
DW  00202h, 00603h, 00204h, 00605h, 00606h, 00207h, 00A08h, 00E09h
DW  01E0Ah, 01A0Bh, 01E0Ch, 01A0Dh, 01A0Eh, 01E0Fh, 00210h, 00611h
DW  00612h, 00213h, 00614h, 00215h, 00216h, 00617h, 00E18h, 00A19h
DW  01A1Ah, 01E1Bh, 01A1Ch, 01E1Dh, 01E1Eh, 01A1Fh, 02220h, 02621h
DW  02622h, 02223h, 02624h, 02225h, 02226h, 02627h, 02E28h, 02A29h
DW  03A2Ah, 03E2Bh, 03A2Ch, 03E2Dh, 03E2Eh, 03A2Fh, 02630h, 02231h
DW  02232h, 02633h, 02234h, 02635h, 02636h, 02237h, 02A38h, 02E39h
DW  03E3Ah, 03A3Bh, 03E3Ch, 03A3Dh, 03A3Eh, 03E3Fh, 00240h, 00641h
DW  00642h, 00243h, 00644h, 00245h, 00246h, 00647h, 00E48h, 00A49h
DW  01A4Ah, 01E4Bh, 01A4Ch, 01E4Dh, 01E4Eh, 01A4Fh, 00650h, 00251h
DW  00252h, 00653h, 00254h, 00655h, 00656h, 00257h, 00A58h, 00E59h
DW  01E5Ah, 01A5Bh, 01E5Ch, 01A5Dh, 01A5Eh, 01E5Fh, 02660h, 02261h
DW  02262h, 02663h, 02264h, 02665h, 02666h, 02267h, 02A68h, 02E69h
DW  03E6Ah, 03A6Bh, 03E6Ch, 03A6Dh, 03A6Eh, 03E6Fh, 02270h, 02671h
DW  02672h, 02273h, 02674h, 02275h, 02276h, 02677h, 02E78h, 02A79h
DW  03A7Ah, 03E7Bh, 03A7Ch, 03E7Dh, 03E7Eh, 03A7Fh, 08280h, 08681h
DW  08682h, 08283h, 08684h, 08285h, 08286h, 08687h, 08E88h, 08A89h
DW  09A8Ah, 09E8Bh, 09A8Ch, 09E8Dh, 09E8Eh, 09A8Fh, 08690h, 08291h
DW  08292h, 08693h, 02334h, 02735h, 02736h, 02337h, 02B38h, 02F39h
DW  03F3Ah, 03B3Bh, 03F3Ch, 03B3Dh, 03B3Eh, 03F3Fh, 00340h, 00741h
DW  00742h, 00343h, 00744h, 00345h, 00346h, 00747h, 00F48h, 00B49h
DW  01B4Ah, 01F4Bh, 01B4Ch, 01F4Dh, 01F4Eh, 01B4Fh, 00750h, 00351h
DW  00352h, 00753h, 00354h, 00755h, 00756h, 00357h, 00B58h, 00F59h
DW  01F5Ah, 01B5Bh, 01F5Ch, 01B5Dh, 01B5Eh, 01F5Fh, 02760h, 02361h
DW  02362h, 02763h, 02364h, 02765h, 02766h, 02367h, 02B68h, 02F69h
DW  03F6Ah, 03B6Bh, 03F6Ch, 03B6Dh, 03B6Eh, 03F6Fh, 02370h, 02771h
DW  02772h, 02373h, 02774h, 02375h, 02376h, 02777h, 02F78h, 02B79h
DW  03B7Ah, 03F7Bh, 03B7Ch, 03F7Dh, 03F7Eh, 03B7Fh, 08380h, 08781h
DW  08782h, 08383h, 08784h, 08385h, 08386h, 08787h, 08F88h, 08B89h
DW  09B8Ah, 09F8Bh, 09B8Ch, 09F8Dh, 09F8Eh, 09B8Fh, 08790h, 08391h
DW  08392h, 08793h, 08394h, 08795h, 08796h, 08397h, 08B98h, 08F99h
DW  09F9Ah, 09B9Bh, 09F9Ch, 09B9Dh, 09B9Eh, 09F9Fh, 0A7A0h, 0A3A1h
DW  0A3A2h, 0A7A3h, 0A3A4h, 0A7A5h, 0A7A6h, 0A3A7h, 0ABA8h, 0AFA9h
DW  0BFAAh, 0BBABh, 0BFACh, 0BBADh, 0BBAEh, 0BFAFh, 0A3B0h, 0A7B1h
DW  0A7B2h, 0A3B3h, 0A7B4h, 0A3B5h, 0A3B6h, 0A7B7h, 0AFB8h, 0ABB9h
DW  0BBBAh, 0BFBBh, 0BBBCh, 0BFBDh, 0BFBEh, 0BBBFh, 087C0h, 083C1h
DW  083C2h, 087C3h, 083C4h, 087C5h, 087C6h, 083C7h, 08BC8h, 08FC9h
DW  09FCAh, 09BCBh, 09FCCh, 09BCDh, 09BCEh, 09FCFh, 083D0h, 087D1h
DW  087D2h, 083D3h, 087D4h, 083D5h, 083D6h, 087D7h, 08FD8h, 08BD9h
DW  09BDAh, 09FDBh, 09BDCh, 09FDDh, 09FDEh, 09BDFh, 0A3E0h, 0A7E1h
DW  0A7E2h, 0A3E3h, 0A7E4h, 0A3E5h, 0A3E6h, 0A7E7h, 0AFE8h, 0ABE9h
DW  0BBEAh, 0BFEBh, 0BBECh, 0BFEDh, 0BFEEh, 0BBEFh, 0A7F0h, 0A3F1h
DW  0A3F2h, 0A7F3h, 0A3F4h, 0A7F5h, 0A7F6h, 0A3F7h, 0ABF8h, 0AFF9h
DW  0BFFAh, 0BBFBh, 0BFFCh, 0BBFDh, 0BBFEh, 0BFFFh, 04700h, 00301h
DW  00302h, 00703h, 00304h, 00705h, 00706h, 00307h, 00B08h, 00F09h
DW  01F0Ah, 01B0Bh, 01F0Ch, 01B0Dh, 01B0Eh, 01F0Fh, 00310h, 00711h
DW  00712h, 00313h, 00714h, 00315h, 00316h, 00717h, 00F18h, 00B19h
DW  01B1Ah, 01F1Bh, 01B1Ch, 01F1Dh, 01F1Eh, 01B1Fh, 02320h, 02721h
DW  02722h, 02323h, 02724h, 02325h, 02326h, 02727h, 02F28h, 02B29h
DW  03B2Ah, 03F2Bh, 03B2Ch, 03F2Dh, 03F2Eh, 03B2Fh, 02730h, 02331h
DW  02332h, 02733h, 02334h, 02735h, 02736h, 02337h, 02B38h, 02F39h
DW  03F3Ah, 03B3Bh, 03F3Ch, 03B3Dh, 03B3Eh, 03F3Fh, 00340h, 00741h
DW  00742h, 00343h, 00744h, 00345h, 00346h, 00747h, 00F48h, 00B49h
DW  01B4Ah, 01F4Bh, 01B4Ch, 01F4Dh, 01F4Eh, 01B4Fh, 00750h, 00351h
DW  00352h, 00753h, 00354h, 00755h, 00756h, 00357h, 00B58h, 00F59h
DW  01F5Ah, 01B5Bh, 01F5Ch, 01B5Dh, 01B5Eh, 01F5Fh, 02760h, 02361h
DW  02362h, 02763h, 02364h, 02765h, 02766h, 02367h, 02B68h, 02F69h
DW  03F6Ah, 03B6Bh, 03F6Ch, 03B6Dh, 03B6Eh, 03F6Fh, 02370h, 02771h
DW  02772h, 02373h, 02774h, 02375h, 02376h, 02777h, 02F78h, 02B79h
DW  03B7Ah, 03F7Bh, 03B7Ch, 03F7Dh, 03F7Eh, 03B7Fh, 08380h, 08781h
DW  08782h, 08383h, 08784h, 08385h, 08386h, 08787h, 08F88h, 08B89h
DW  09B8Ah, 09F8Bh, 09B8Ch, 09F8Dh, 09F8Eh, 09B8Fh, 08790h, 08391h
DW  08392h, 08793h, 08394h, 08795h, 08796h, 08397h, 08B98h, 08F99h

NEG_Table:
DB  0,66,255,187,254,187,253,187,252,187,251,187,250,187,249,187,248,187,247,179
DB  246,179,245,179,244,179,243,179,242,179,241,179,240,163,239,187,238,187,237,187
DB  236,187,235,187,234,187,233,187,232,187,231,179,230,179,229,179,228,179,227,179
DB  226,179,225,179,224,163,223,155,222,155,221,155,220,155,219,155,218,155,217,155
DB  216,155,215,147,214,147,213,147,212,147,211,147,210,147,209,147,208,131,207,155
DB  206,155,205,155,204,155,203,155,202,155,201,155,200,155,199,147,198,147,197,147
DB  196,147,195,147,194,147,193,147,192,131,191,187,190,187,189,187,188,187,187,187
DB  186,187,185,187,184,187,183,179,182,179,181,179,180,179,179,179,178,179,177,179
DB  176,163,175,187,174,187,173,187,172,187,171,187,170,187,169,187,168,187,167,179
DB  166,179,165,179,164,179,163,179,162,179,161,179,160,163,159,155,158,155,157,155
DB  156,155,155,155,154,155,153,155,152,155,151,147,150,147,149,147,148,147,147,147
DB  146,147,145,147,144,131,143,155,142,155,141,155,140,155,139,155,138,155,137,155
DB  136,155,135,147,134,147,133,147,132,147,131,147,130,147,129,147,128,135,127,59
DB  126,59,125,59,124,59,123,59,122,59,121,59,120,59,119,51,118,51,117,51
DB  116,51,115,51,114,51,113,51,112,35,111,59,110,59,109,59,108,59,107,59
DB  106,59,105,59,104,59,103,51,102,51,101,51,100,51,99,51,98,51,97,51
DB  96,35,95,27,94,27,93,27,92,27,91,27,90,27,89,27,88,27,87,19
DB  86,19,85,19,84,19,83,19,82,19,81,19,80,3,79,27,78,27,77,27
DB  76,27,75,27,74,27,73,27,72,27,71,19,70,19,69,19,68,19,67,19
DB  66,19,65,19,64,3,63,59,62,59,61,59,60,59,59,59,58,59,57,59
DB  56,59,55,51,54,51,53,51,52,51,51,51,50,51,49,51,48,35,47,59
DB  46,59,45,59,44,59,43,59,42,59,41,59,40,59,39,51,38,51,37,51
DB  36,51,35,51,34,51,33,51,32,35,31,27,30,27,29,27,28,27,27,27
DB  26,27,25,27,24,27,23,19,22,19,21,19,20,19,19,19,18,19,17,19
DB  16,3,15,27,14,27,13,27,12,27,11,27,10,27,9,27,8,27,7,19
DB  6,19,5,19,4,19,3,19,2,19,1,19

;- the end ------------------------------------------------------------------;

