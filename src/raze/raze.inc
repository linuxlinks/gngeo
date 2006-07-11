; RAZE - complete Z80 opcode list
;
; Conventions:
; R   = A,B,C,D,E,F,H,L
; RR  = AF,BC,DE,HL,SP
; RR2 = AF2,BC2,DE2,HL2
; XY  = IX,IY
; XYr = A,B,C,D,E,F,IXl,IXh,IYl,IYh
; mRR = (RR)
; mXY = (XY+dd)
; mNN = (nnnn)
; N   = nn     (unsigned)
; NN  = nnnn   (unsigned)
; +dd = nn     (signed)
; b   = bit (0-7)
; cc  = c,nc,z,nc,pe,po,m,p
; Rf  = refresh
; I   = interrupt register
;
; FDCB instructions not included - use DDCB with IY instead of IX

; **************************************************************************
; LD r,r

DEF op, 40, 4, 0
	LD_R_R B, B

DEF op, 41, 4, 0
	LD_R_R B, C

DEF op, 42, 4, 0
	LD_R_R B, D

DEF op, 43, 4, 0
	LD_R_R B, E

DEF op, 44, 4, 0
	LD_R_R B, H

DEF op, 45, 4, 0
	LD_R_R B, L

DEF op, 47, 4, 0
	LD_R_R B, A

DEF op, 48, 4, 0
	LD_R_R C, B

DEF op, 49, 4, 0
	LD_R_R C, C

DEF op, 4a, 4, 0
	LD_R_R C, D

DEF op, 4b, 4, 0
	LD_R_R C, E

DEF op, 4c, 4, 0
	LD_R_R C, H

DEF op, 4d, 4, 0
	LD_R_R C, L

DEF op, 4f, 4, 0
	LD_R_R C, A

DEF op, 50, 4, 0
	LD_R_R D, B

DEF op, 51, 4, 0
	LD_R_R D, C

DEF op, 52, 4, 0
	LD_R_R D, D

DEF op, 53, 4, 0
	LD_R_R D, E

DEF op, 54, 4, 0
	LD_R_R D, H

DEF op, 55, 4, 0
	LD_R_R D, L

DEF op, 57, 4, 0
	LD_R_R D, A

DEF op, 58, 4, 0
	LD_R_R E, B

DEF op, 59, 4, 0
	LD_R_R E, C

DEF op, 5a, 4, 0
	LD_R_R E, D

DEF op, 5b, 4, 0
	LD_R_R E, E

DEF op, 5c, 4, 0
	LD_R_R E, H

DEF op, 5d, 4, 0
	LD_R_R E, L

DEF op, 5f, 4, 0
	LD_R_R E, A

DEF op, 60, 4, 0
	LD_R_R H, B

DEF op, 61, 4, 0
	LD_R_R H, C

DEF op, 62, 4, 0
	LD_R_R H, D

DEF op, 63, 4, 0
	LD_R_R H, E

DEF op, 64, 4, 0
	LD_R_R H, H

DEF op, 65, 4, 0
	LD_R_R H, L

DEF op, 67, 4, 0
	LD_R_R H, A

DEF op, 68, 4, 0
	LD_R_R L, B

DEF op, 69, 4, 0
	LD_R_R L, C

DEF op, 6a, 4, 0
	LD_R_R L, D

DEF op, 6b, 4, 0
	LD_R_R L, E

DEF op, 6c, 4, 0
	LD_R_R L, H

DEF op, 6d, 4, 0
	LD_R_R L, L

DEF op, 6f, 4, 0
	LD_R_R L, A

DEF op, 78, 4, 0
	LD_R_R A, B

DEF op, 79, 4, 0
	LD_R_R A, C

DEF op, 7a, 4, 0
	LD_R_R A, D

DEF op, 7b, 4, 0
	LD_R_R A, E

DEF op, 7c, 4, 0
	LD_R_R A, H

DEF op, 7d, 4, 0
	LD_R_R A, L

DEF op, 7f, 4, 0
	LD_R_R A, A


; **************************************************************************
; LD r,r (r=IXl,IXh,IYl,IYh)

DEF dd, 44, 8, 0
	LD_XYr_XYr B, IXh

DEF dd, 45, 8, 0
	LD_XYr_XYr B, IXl

DEF fd, 44, 8, 0
	LD_XYr_XYr B, IYh

DEF fd, 45, 8, 0
	LD_XYr_XYr B, IYl

DEF dd, 4c, 8, 0
	LD_XYr_XYr C, IXh

DEF dd, 4d, 8, 0
	LD_XYr_XYr C, IXl

DEF fd, 4c, 8, 0
	LD_XYr_XYr C, IYh

DEF fd, 4d, 8, 0
	LD_XYr_XYr C, IYl

DEF dd, 54, 8, 0
	LD_XYr_XYr D, IXh

DEF dd, 55, 8, 0
	LD_XYr_XYr D, IXl

DEF fd, 54, 8, 0
	LD_XYr_XYr D, IYh

DEF fd, 55, 8, 0
	LD_XYr_XYr D, IYl

DEF dd, 5c, 8, 0
	LD_XYr_XYr E, IXh

DEF dd, 5d, 8, 0
	LD_XYr_XYr E, IXl

DEF fd, 5c, 8, 0
	LD_XYr_XYr E, IYh

DEF fd, 5d, 8, 0
	LD_XYr_XYr E, IYl

DEF dd, 7c, 8, 0
	LD_XYr_XYr A, IXh

DEF dd, 7d, 8, 0
	LD_XYr_XYr A, IXl

DEF fd, 7c, 8, 0
	LD_XYr_XYr A, IYh

DEF fd, 7d, 8, 0
	LD_XYr_XYr A, IYl

DEF dd, 60, 8, 0
	LD_XYr_XYr IXh, B

DEF dd, 61, 8, 0
	LD_XYr_XYr IXh, C

DEF dd, 62, 8, 0
	LD_XYr_XYr IXh, D

DEF dd, 63, 8, 0
	LD_XYr_XYr IXh, E

;DEF dd, 64, 8, 0       ; it's a NOP anyway, don't include it
;   LD_XYr_XYr z80.rIXh, z80.rIXh

DEF dd, 65, 8, 0
	LD_XYr_XYr IXh, IXl

DEF dd, 67, 8, 0
	LD_XYr_XYr IXh, A

DEF dd, 68, 8, 0
	LD_XYr_XYr IXl, B

DEF dd, 69, 8, 0
	LD_XYr_XYr IXl, C

DEF dd, 6a, 8, 0
	LD_XYr_XYr IXl, D

DEF dd, 6b, 8, 0
	LD_XYr_XYr IXl, E

DEF dd, 6c, 8, 0
	LD_XYr_XYr IXl, IXh


;DEF dd, 6d, 8, 0       ; it's a NOP anyway, don't include it
;   LD_XYr_XYr z80.rIXl, z80.rIXl

DEF dd, 6f, 8, 0
	LD_XYr_XYr IXl, A

DEF fd, 60, 8, 0
	LD_XYr_XYr IYh, B

DEF fd, 61, 8, 0
	LD_XYr_XYr IYh, C

DEF fd, 62, 8, 0
	LD_XYr_XYr IYh, D

DEF fd, 63, 8, 0
	LD_XYr_XYr IYh, E


;DEF fd, 64, 8, 0       ; it's a NOP anyway, don't include it
;   LD_XYr_XYr z80.rIYh, z80.rIYh

DEF fd, 65, 8, 0
	LD_XYr_XYr IYh, IYl

DEF fd, 67, 8, 0
	LD_XYr_XYr IYh, A

DEF fd, 68, 8, 0
	LD_XYr_XYr IYl, B

DEF fd, 69, 8, 0
	LD_XYr_XYr IYl, C

DEF fd, 6a, 8, 0
	LD_XYr_XYr IYl, D

DEF fd, 6b, 8, 0
	LD_XYr_XYr IYl, E

DEF fd, 6c, 8, 0
	LD_XYr_XYr IYl, IYh


;DEF fd, 6d, 8, 0       ; it's a NOP anyway, don't include it
;   LD_XYr_XYr z80.rIYl, z80.rIYl

DEF fd, 6f, 8, 0
	LD_XYr_XYr IYl, A


; **************************************************************************
; LD r,n

DEF op, 06, 7, 0
	LD_R_N B

DEF op, 0e, 7, 0
	LD_R_N C

DEF op, 16, 7, 0
	LD_R_N D

DEF op, 1e, 7, 0
	LD_R_N E

DEF op, 26, 7, 0
	LD_R_N H

DEF op, 2e, 7, 0
	LD_R_N L

DEF op, 3e, 7, 0
	LD_R_N A


; **************************************************************************
; LD r,n (r=IXl,IXh,IYl,IYh)

DEF dd, 26, 11, 0
   LD_R_N IXh

DEF dd, 2e, 11, 0
   LD_R_N IXl

DEF fd, 26, 11, 0
   LD_R_N IYh

DEF fd, 2e, 11, 0
   LD_R_N IYl


; **************************************************************************
; LD r,(HL)

DEF op, 46, 7, 0
	LD_R_mRR B, HL

DEF op, 4e, 7, 0
	LD_R_mRR C, HL

DEF op, 56, 7, 0
	LD_R_mRR D, HL

DEF op, 5e, 7, 0
	LD_R_mRR E, HL

DEF op, 66, 7, 0
	LD_R_mRR H, HL

DEF op, 6e, 7, 0
	LD_R_mRR L, HL

DEF op, 7e, 7, 0
	LD_R_mRR A, HL


; **************************************************************************
; LD r,(XY+d)

DEF dd, 46, 19, 0
	LD_R_mXY B, IX

DEF fd, 46, 19, 0
	LD_R_mXY B, IY

DEF dd, 4e, 19, 0
	LD_R_mXY C, IX

DEF fd, 4e, 19, 0
	LD_R_mXY C, IY

DEF dd, 56, 19, 0
	LD_R_mXY D, IX

DEF fd, 56, 19, 0
	LD_R_mXY D, IY

DEF dd, 5e, 19, 0
	LD_R_mXY E, IX

DEF fd, 5e, 19, 0
	LD_R_mXY E, IY

DEF dd, 66, 19, 0
	LD_R_mXY H, IX

DEF fd, 66, 19, 0
	LD_R_mXY H, IY

DEF dd, 6e, 19, 0
	LD_R_mXY L, IX

DEF fd, 6e, 19, 0
	LD_R_mXY L, IY

DEF dd, 7e, 19, 0
	LD_R_mXY A, IX

DEF fd, 7e, 19, 0
	LD_R_mXY A, IY


; **************************************************************************
; LD (HL),r

DEF op, 70, 7, 0
	LD_mRR_R HL, B

DEF op, 71, 7, 0
	LD_mRR_R HL, C

DEF op, 72, 7, 0
	LD_mRR_R HL, D

DEF op, 73, 7, 0
	LD_mRR_R HL, E

DEF op, 74, 7, 0
	LD_mRR_R HL, H

DEF op, 75, 7, 0
	LD_mRR_R HL, L

DEF op, 77, 7, 0
	LD_mRR_R HL, A


; **************************************************************************
; LD (XY+d),r

DEF dd, 70, 19, 0
	LD_mXY_R IX, B

DEF fd, 70, 19, 0
	LD_mXY_R IY, B

DEF dd, 71, 19, 0
	LD_mXY_R IX, C

DEF fd, 71, 19, 0
	LD_mXY_R IY, C

DEF dd, 72, 19, 0
	LD_mXY_R IX, D

DEF fd, 72, 19, 0
	LD_mXY_R IY, D

DEF dd, 73, 19, 0
	LD_mXY_R IX, E

DEF fd, 73, 19, 0
	LD_mXY_R IY, E

DEF dd, 74, 19, 0
	LD_mXY_R IX, H

DEF fd, 74, 19, 0
	LD_mXY_R IY, H

DEF dd, 75, 19, 0
	LD_mXY_R IX, L

DEF fd, 75, 19, 0
	LD_mXY_R IY, L

DEF dd, 77, 19, 0
	LD_mXY_R IX, A

DEF fd, 77, 19, 0
	LD_mXY_R IY, A


; **************************************************************************
; LD (HL),n / LD (XY+d),n

DEF op, 36, 10, 0
	LD_mRR_N HL

DEF dd, 36, 19, 0
	LD_mXY_N IX

DEF fd, 36, 19, 0
	LD_mXY_N IY


; **************************************************************************
; LD A,(BC) / LD A,(DE) / LD A,(nn)

DEF op, 0a, 7, 0
	LD_R_mRR A, BC

DEF op, 1a, 7, 0
	LD_R_mRR A, DE

DEF op, 3a, 13, 0
	LD_R_mNN A


; **************************************************************************
; LD (BC),A / LD (DE),A / LD (nn),A

DEF op, 02, 7, 0
	LD_mRR_R BC, A

DEF op, 12, 7, 0
	LD_mRR_R DE, A

DEF op, 32, 13, 0
	LD_mNN_R A


; **************************************************************************
; LD [ARI],[ARI]

DEF ed, 57, 9, 0
	LD_R_I A

DEF ed, 5f, 9, 0
	LD_R_Rf A

DEF ed, 47, 9, 0
	LD_I_R A

DEF ed, 4f, 9, 0
	LD_Rf_R A


; **************************************************************************
; LD dd,nn

DEF op, 01, 10, 0
	LD_RR_NN BC

DEF op, 11, 10, 0
	LD_RR_NN DE

DEF op, 21, 10, 0
	LD_RR_NN HL

DEF op, 31, 10, 0
	LD_RR_NN SP

DEF dd, 21, 14, 0
   LD_RR_NN IX

DEF fd, 21, 14, 0
   LD_RR_NN IY


; **************************************************************************
; LD dd,(nn)

DEF op, 2a, 16, 0
	LD_RR_mNN HL

DEF ed, 4b, 16, 0
	LD_RR_mNN BC

DEF ed, 5b, 16, 0
	LD_RR_mNN DE

DEF ed, 6b, 16, 0
	LD_RR_mNN HL

DEF ed, 7b, 16, 0
	LD_RR_mNN SP

DEF dd, 2a, 20, 0
   LD_RR_mNN IX

DEF fd, 2a, 20, 0
   LD_RR_mNN IY


; **************************************************************************
; LD (nnnn),dd

DEF op, 22, 16, 0
	LD_mNN_RR HL

DEF ed, 43, 16, 0
	LD_mNN_RR BC

DEF ed, 53, 16, 0
	LD_mNN_RR DE

DEF ed, 63, 16, 0
	LD_mNN_RR HL

DEF ed, 73, 16, 0
	LD_mNN_RR SP

DEF dd, 22, 20, 0
   LD_mNN_RR IX

DEF fd, 22, 20, 0
   LD_mNN_RR IY


; **************************************************************************
; LD SP,dd

DEF op, f9, 6, 0
	LD_RR_RR SP, HL

DEF dd, f9, 10, 0
	LD_RR_XY SP, IX

DEF fd, f9, 10, 0
	LD_RR_XY SP, IY


; **************************************************************************
; PUSH rr

DEF op, c5, 11, 0
	PUSH_RR BC

DEF op, d5, 11, 0
	PUSH_RR DE

DEF op, e5, 11, 0
	PUSH_RR HL

DEF op, f5, 11, 0
	PUSH_RR AF

DEF dd, e5, 15, 0
   PUSH_RR IX

DEF fd, e5, 15, 0
   PUSH_RR IY


; **************************************************************************
; POP rr

DEF op, c1, 14, 0
	POP_RR BC

DEF op, d1, 14, 0
	POP_RR DE

DEF op, e1, 14, 0
	POP_RR HL

DEF op, f1, 14, 0
	POP_RR AF

DEF dd, e1, 14, 0
   POP_RR IX

DEF fd, e1, 14, 0
   POP_RR IY


; **************************************************************************
; EX group

DEF op, eb, 4, 0
	EX_RR_RR DE, HL

DEF op, 08, 4, 0
   EX_AF_AF2

DEF op, d9, 4, 0
	EXX

DEF op, e3, 19, 0
	EX_RR_mRR HL, SP

DEF dd, e3, 23, 0
   EX_RR_mRR IX, SP

DEF fd, e3, 23, 0
   EX_RR_mRR IY, SP


; **************************************************************************
; Block Transfer (LD[ID][R ])

DEF ed, a0, 16, 0
	LDI

DEF ed, a8, 16, 0
	LDD

DEF ed, b0, 16, 21
	LDIR

DEF ed, b8, 16, 21
	LDDR


; **************************************************************************
; search group CP[ID][R ]

DEF ed, a1, 16, 0
	CPI

DEF ed, a9, 16, 0
	CPD

DEF ed, b1, 16, 21
	CPIR

DEF ed, b9, 16, 21
	CPDR


; **************************************************************************
; 8 bit Arithmatic

DEF op, 80, 4, 0
	ART_R ADD, B

DEF op, 81, 4, 0
	ART_R ADD, C

DEF op, 82, 4, 0
	ART_R ADD, D

DEF op, 83, 4, 0
	ART_R ADD, E

DEF op, 84, 4, 0
	ART_R ADD, H

DEF op, 85, 4, 0
	ART_R ADD, L

DEF op, 87, 4, 0
	ART_R ADD, A

DEF dd, 84, 8, 0
   ART_R ADD, IXh

DEF dd, 85, 8, 0
   ART_R ADD, IXl

DEF fd, 84, 8, 0
   ART_R ADD, IYh

DEF fd, 85, 8, 0
   ART_R ADD, IYl

DEF op, 86, 7, 0
	ART_mRR ADD, HL

DEF op, c6, 8, 0
	ART_N ADD

DEF dd, 86, 19, 0
	ART_mXY ADD, IX

DEF fd, 86, 19, 0
	ART_mXY ADD, IY

DEF op, 88, 4, 0
	ART_R ADC, B

DEF op, 89, 4, 0
	ART_R ADC, C

DEF op, 8a, 4, 0
	ART_R ADC, D

DEF op, 8b, 4, 0
	ART_R ADC, E

DEF op, 8c, 4, 0
	ART_R ADC, H

DEF op, 8d, 4, 0
	ART_R ADC, L

DEF op, 8f, 4, 0
	ART_R ADC, A

DEF dd, 8c, 8, 0
   ART_R ADC, IXh

DEF dd, 8d, 8, 0
   ART_R ADC, IXl

DEF fd, 8c, 8, 0
   ART_R ADC, IYh

DEF fd, 8d, 8, 0
   ART_R ADC, IYl

DEF op, 8e, 7, 0
	ART_mRR ADC, HL

DEF op, ce, 8, 0
	ART_N ADC

DEF dd, 8e, 19, 0
	ART_mXY ADC, IX

DEF fd, 8e, 19, 0
	ART_mXY ADC, IY

DEF op, 90, 4, 0
	ART_R SUB, B

DEF op, 91, 4, 0
	ART_R SUB, C

DEF op, 92, 4, 0
	ART_R SUB, D

DEF op, 93, 4, 0
	ART_R SUB, E

DEF op, 94, 4, 0
	ART_R SUB, H

DEF op, 95, 4, 0
	ART_R SUB, L

DEF op, 97, 4, 0
	ART_R SUB, A

DEF dd, 94, 8, 0
   ART_R SUB, IXh

DEF dd, 95, 8, 0
   ART_R SUB, IXl

DEF fd, 94, 8, 0
   ART_R SUB, IYh

DEF fd, 95, 8, 0
   ART_R SUB, IYl

DEF op, 96, 7, 0
	ART_mRR SUB, HL

DEF op, d6, 8, 0
	ART_N SUB

DEF dd, 96, 19, 0
	ART_mXY SUB, IX

DEF fd, 96, 19, 0
	ART_mXY SUB, IY

DEF op, 98, 4, 0
	ART_R SBC, B

DEF op, 99, 4, 0
	ART_R SBC, C

DEF op, 9a, 4, 0
	ART_R SBC, D

DEF op, 9b, 4, 0
	ART_R SBC, E

DEF op, 9c, 4, 0
	ART_R SBC, H

DEF op, 9d, 4, 0
	ART_R SBC, L

DEF op, 9f, 4, 0
	ART_R SBC, A

DEF dd, 9c, 8, 0
   ART_R SBC, IXh

DEF dd, 9d, 8, 0
   ART_R SBC, IXl

DEF fd, 9c, 8, 0
   ART_R SBC, IYh

DEF fd, 9d, 8, 0
   ART_R SBC, IYl

DEF op, 9e, 7, 0
	ART_mRR SBC, HL

DEF op, de, 8, 0
	ART_N SBC

DEF dd, 9e, 19, 0
	ART_mXY SBC, IX

DEF fd, 9e, 19, 0
	ART_mXY SBC, IY


; **************************************************************************
; 8 bit logical group

DEF op, a0, 4, 0
	LOG_R AND, B

DEF op, a1, 4, 0
	LOG_R AND, C

DEF op, a2, 4, 0
	LOG_R AND, D

DEF op, a3, 4, 0
	LOG_R AND, E

DEF op, a4, 4, 0
	LOG_R AND, H

DEF op, a5, 4, 0
	LOG_R AND, L

DEF op, a7, 4, 0
	LOG_R AND, A

DEF dd, a4, 8, 0
   LOG_R AND, IXh

DEF dd, a5, 8, 0
   LOG_R AND, IXl

DEF fd, a4, 8, 0
   LOG_R AND, IYh

DEF fd, a5, 8, 0
   LOG_R AND, IYl

DEF op, e6, 8, 0
	LOG_N AND

DEF op, a6, 7, 0
	LOG_mRR AND, HL

DEF dd, a6, 19, 0
	LOG_mXY AND, IX

DEF fd, a6, 19, 0
	LOG_mXY AND, IY

DEF op, a8, 4, 0
	LOG_R XOR, B

DEF op, a9, 4, 0
	LOG_R XOR, C

DEF op, aa, 4, 0
	LOG_R XOR, D

DEF op, ab, 4, 0
	LOG_R XOR, E

DEF op, ac, 4, 0
	LOG_R XOR, H

DEF op, ad, 4, 0
	LOG_R XOR, L

DEF op, af, 4, 0
	LOG_R XOR, A

DEF dd, ac, 8, 0
   LOG_R XOR, IXh

DEF dd, ad, 8, 0
   LOG_R XOR, IXl

DEF fd, ac, 8, 0
   LOG_R XOR, IYh

DEF fd, ad, 8, 0
   LOG_R XOR, IYl

DEF op, ee, 8, 0
	LOG_N XOR

DEF op, ae, 7, 0
	LOG_mRR XOR, HL

DEF dd, ae, 19, 0
	LOG_mXY XOR, IX

DEF fd, ae, 19, 0
	LOG_mXY XOR, IY

DEF op, b0, 4, 0
	LOG_R OR, B

DEF op, b1, 4, 0
	LOG_R OR, C

DEF op, b2, 4, 0
	LOG_R OR, D

DEF op, b3, 4, 0
	LOG_R OR, E

DEF op, b4, 4, 0
	LOG_R OR, H

DEF op, b5, 4, 0
	LOG_R OR, L

DEF op, b7, 4, 0
	LOG_R OR, A

DEF dd, b4, 8, 0
   LOG_R OR, IXh

DEF dd, b5, 8, 0
   LOG_R OR, IXl

DEF fd, b4, 8, 0
   LOG_R OR, IYh

DEF fd, b5, 8, 0
   LOG_R OR, IYl

DEF op, f6, 8, 0
	LOG_N OR

DEF op, b6, 7, 0
	LOG_mRR OR, HL

DEF dd, b6, 19, 0
	LOG_mXY OR, IX

DEF fd, b6, 19, 0
	LOG_mXY OR, IY


; **************************************************************************
; CP group

DEF op, b8, 4, 0
	CP_R B

DEF op, b9, 4, 0
	CP_R C

DEF op, ba, 4, 0
	CP_R D

DEF op, bb, 4, 0
	CP_R E

DEF op, bc, 4, 0
	CP_R H

DEF op, bd, 4, 0
	CP_R L

DEF op, bf, 4, 0
	CP_R A

DEF dd, bc, 8, 0
   CP_R IXh

DEF dd, bd, 8, 0
   CP_R IXl

DEF fd, bc, 8, 0
   CP_R IYh

DEF fd, bd, 8, 0
   CP_R IYl

DEF op, be, 7, 0
	CP_mRR HL

DEF op, fe, 8, 0
	CP_N

DEF dd, be, 19, 0
	CP_mXY IX

DEF fd, be, 19, 0
	CP_mXY IY


; **************************************************************************
; DEC/INC 8 bit group

DEF op, 04, 4, 0
	DECINC_R INC, B

DEF op, 0c, 4, 0
	DECINC_R INC, C

DEF op, 14, 4, 0
	DECINC_R INC, D

DEF op, 1c, 4, 0
	DECINC_R INC, E

DEF op, 24, 4, 0
	DECINC_R INC, H

DEF op, 2c, 4, 0
	DECINC_R INC, L

DEF op, 3c, 4, 0
	DECINC_R INC, A

DEF dd, 24, 8, 0
   DECINC_R INC, IXh

DEF dd, 2c, 8, 0
   DECINC_R INC, IXl

DEF fd, 24, 8, 0
   DECINC_R INC, IYh

DEF fd, 2c, 8, 0
   DECINC_R INC, IYl

DEF op, 34, 11, 0
	DECINC_mRR INC, HL

DEF dd, 34, 23, 0
	DECINC_mXY INC, IX

DEF fd, 34, 23, 0
	DECINC_mXY INC, IY

DEF op, 05, 4, 0
	DECINC_R DEC, B

DEF op, 0d, 4, 0
	DECINC_R DEC, C

DEF op, 15, 4, 0
	DECINC_R DEC, D

DEF op, 1d, 4, 0
	DECINC_R DEC, E

DEF op, 25, 4, 0
	DECINC_R DEC, H

DEF op, 2d, 4, 0
	DECINC_R DEC, L

DEF op, 3d, 4, 0
	DECINC_R DEC, A

DEF dd, 25, 8, 0
   DECINC_R DEC, IXh

DEF dd, 2d, 8, 0
   DECINC_R DEC, IXl

DEF fd, 25, 8, 0
   DECINC_R DEC, IYh

DEF fd, 2d, 8, 0
   DECINC_R DEC, IYl

DEF op, 35, 11, 0
	DECINC_mRR DEC, HL

DEF dd, 35, 23, 0
	DECINC_mXY DEC, IX

DEF fd, 35, 23, 0
	DECINC_mXY DEC, IY


; **************************************************************************
; ADD HL,ss

DEF op, 09, 11, 0
	ADD_RR_RR HL, BC

DEF op, 19, 11, 0
	ADD_RR_RR HL, DE

DEF op, 29, 11, 0
	ADD_RR_RR HL, HL

DEF op, 39, 11, 0
	ADD_RR_RR HL, SP

DEF dd, 09, 15, 0
   ADD_RR_RR IX, BC

DEF dd, 19, 15, 0
   ADD_RR_RR IX, DE

DEF dd, 29, 15, 0
   ADD_RR_RR IX, IX

DEF dd, 39, 15, 0
   ADD_RR_RR IX, SP

DEF fd, 09, 15, 0
   ADD_RR_RR IY, BC

DEF fd, 19, 15, 0
   ADD_RR_RR IY, DE

DEF fd, 29, 15, 0
   ADD_RR_RR IY, IY

DEF fd, 39, 15, 0
   ADD_RR_RR IY, SP

DEF ed, 4a, 15, 0
	ADC_RR_RR HL, BC

DEF ed, 5a, 15, 0
	ADC_RR_RR HL, DE

DEF ed, 6a, 15, 0
	ADC_RR_RR HL, HL

DEF ed, 7a, 15, 0
	ADC_RR_RR HL, SP

DEF ed, 42, 15, 0
	SBC_RR_RR HL, BC

DEF ed, 52, 15, 0
	SBC_RR_RR HL, DE

DEF ed, 62, 15, 0
	SBC_RR_RR HL, HL

DEF ed, 72, 15, 0
	SBC_RR_RR HL, SP

DEF op, 03, 6, 0
	DECINC_RR INC, BC

DEF op, 13, 6, 0
	DECINC_RR INC, DE

DEF op, 23, 6, 0
	DECINC_RR INC, HL

DEF op, 33, 6, 0
	DECINC_RR INC, SP

DEF dd, 23, 10, 0
   DECINC_RR INC, IX

DEF fd, 23, 10, 0
   DECINC_RR INC, IY

DEF op, 0b, 6, 0
	DECINC_RR DEC, BC

DEF op, 1b, 6, 0
	DECINC_RR DEC, DE

DEF op, 2b, 6, 0
	DECINC_RR DEC, HL

DEF op, 3b, 6, 0
	DECINC_RR DEC, SP

DEF dd, 2b, 10, 0
   DECINC_RR DEC, IX

DEF fd, 2b, 10, 0
   DECINC_RR DEC, IY


; **************************************************************************
; rotate/shift

DEF op, 07, 4, 0
	ROT_R RLCA, A

DEF op, 0f, 4, 0
	ROT_R RRCA, A

DEF op, 17, 4, 0
	ROT_R RLA, A

DEF op, 1f, 4, 0
	ROT_R RRA, A

DEF cb, 00, 8, 0
	ROT_R RLC, B

DEF cb, 01, 8, 0
	ROT_R RLC, C

DEF cb, 02, 8, 0
	ROT_R RLC, D

DEF cb, 03, 8, 0
	ROT_R RLC, E

DEF cb, 04, 8, 0
	ROT_R RLC, H

DEF cb, 05, 8, 0
	ROT_R RLC, L

DEF cb, 07, 8, 0
	ROT_R RLC, A

DEF cb, 06, 15, 0
	ROT_mRR RLC, HL

DEF ddcb, 00, 23, 0
	ROT_mXY_R RLC, B

DEF ddcb, 01, 23, 0
	ROT_mXY_R RLC, C

DEF ddcb, 02, 23, 0
	ROT_mXY_R RLC, D

DEF ddcb, 03, 23, 0
	ROT_mXY_R RLC, E

DEF ddcb, 04, 23, 0
	ROT_mXY_R RLC, H

DEF ddcb, 05, 23, 0
	ROT_mXY_R RLC, L

DEF ddcb, 06, 23, 0
	ROT_mXY RLC

DEF ddcb, 07, 23, 0
	ROT_mXY_R RLC, A

DEF cb, 08, 8, 0
	ROT_R RRC, B

DEF cb, 09, 8, 0
	ROT_R RRC, C

DEF cb, 0a, 8, 0
	ROT_R RRC, D

DEF cb, 0b, 8, 0
	ROT_R RRC, E

DEF cb, 0c, 8, 0
	ROT_R RRC, H

DEF cb, 0d, 8, 0
	ROT_R RRC, L

DEF cb, 0f, 8, 0
	ROT_R RRC, A

DEF cb, 0e, 15, 0
	ROT_mRR RRC, HL

DEF ddcb, 08, 23, 0
	ROT_mXY_R RRC, B

DEF ddcb, 09, 23, 0
	ROT_mXY_R RRC, C

DEF ddcb, 0a, 23, 0
	ROT_mXY_R RRC, D

DEF ddcb, 0b, 23, 0
	ROT_mXY_R RRC, E

DEF ddcb, 0c, 23, 0
	ROT_mXY_R RRC, H

DEF ddcb, 0d, 23, 0
	ROT_mXY_R RRC, L

DEF ddcb, 0e, 23, 0
	ROT_mXY RRC

DEF ddcb, 0f, 23, 0
	ROT_mXY_R RRC, A

DEF cb, 10, 8, 0
	ROT_R RL, B

DEF cb, 11, 8, 0
	ROT_R RL, C

DEF cb, 12, 8, 0
	ROT_R RL, D

DEF cb, 13, 8, 0
	ROT_R RL, E

DEF cb, 14, 8, 0
	ROT_R RL, H

DEF cb, 15, 8, 0
	ROT_R RL, L

DEF cb, 17, 8, 0
	ROT_R RL, A

DEF cb, 16, 15, 0
	ROT_mRR RL, HL

DEF ddcb, 10, 23, 0
	ROT_mXY_R RL, B

DEF ddcb, 11, 23, 0
	ROT_mXY_R RL, C

DEF ddcb, 12, 23, 0
	ROT_mXY_R RL, D

DEF ddcb, 13, 23, 0
	ROT_mXY_R RL, E

DEF ddcb, 14, 23, 0
	ROT_mXY_R RL, H

DEF ddcb, 15, 23, 0
	ROT_mXY_R RL, L

DEF ddcb, 16, 23, 0
	ROT_mXY RL

DEF ddcb, 17, 23, 0
	ROT_mXY_R RL, A

DEF cb, 18, 8, 0
	ROT_R RR, B

DEF cb, 19, 8, 0
	ROT_R RR, C

DEF cb, 1a, 8, 0
	ROT_R RR, D

DEF cb, 1b, 8, 0
	ROT_R RR, E

DEF cb, 1c, 8, 0
	ROT_R RR, H

DEF cb, 1d, 8, 0
	ROT_R RR, L

DEF cb, 1f, 8, 0
	ROT_R RR, A

DEF cb, 1e, 15, 0
	ROT_mRR RR, HL

DEF ddcb, 18, 23, 0
	ROT_mXY_R RR, B

DEF ddcb, 19, 23, 0
	ROT_mXY_R RR, C

DEF ddcb, 1a, 23, 0
	ROT_mXY_R RR, D

DEF ddcb, 1b, 23, 0
	ROT_mXY_R RR, E

DEF ddcb, 1c, 23, 0
	ROT_mXY_R RR, H

DEF ddcb, 1d, 23, 0
	ROT_mXY_R RR, L

DEF ddcb, 1e, 23, 0
	ROT_mXY RR

DEF ddcb, 1f, 23, 0
	ROT_mXY_R RR, A

DEF cb, 20, 8, 0
	SHF_R SLA, B

DEF cb, 21, 8, 0
	SHF_R SLA, C

DEF cb, 22, 8, 0
	SHF_R SLA, D

DEF cb, 23, 8, 0
	SHF_R SLA, E

DEF cb, 24, 8, 0
	SHF_R SLA, H

DEF cb, 25, 8, 0
	SHF_R SLA, L

DEF cb, 27, 8, 0
	SHF_R SLA, A

DEF cb, 26, 15, 0
	SHF_mRR SLA, HL

DEF ddcb, 20, 23, 0
	SHF_mXY_R SLA, B

DEF ddcb, 21, 23, 0
	SHF_mXY_R SLA, C

DEF ddcb, 22, 23, 0
	SHF_mXY_R SLA, D

DEF ddcb, 23, 23, 0
	SHF_mXY_R SLA, E

DEF ddcb, 24, 23, 0
	SHF_mXY_R SLA, H

DEF ddcb, 25, 23, 0
	SHF_mXY_R SLA, L

DEF ddcb, 26, 23, 0
	SHF_mXY SLA

DEF ddcb, 27, 23, 0
	SHF_mXY_R SLA, A

DEF cb, 28, 8, 0
	SHF_R SRA, B

DEF cb, 29, 8, 0
	SHF_R SRA, C

DEF cb, 2a, 8, 0
	SHF_R SRA, D

DEF cb, 2b, 8, 0
	SHF_R SRA, E

DEF cb, 2c, 8, 0
	SHF_R SRA, H

DEF cb, 2d, 8, 0
	SHF_R SRA, L

DEF cb, 2f, 8, 0
	SHF_R SRA, A

DEF cb, 2e, 15, 0
	SHF_mRR SRA, HL

DEF ddcb, 28, 23, 0
	SHF_mXY_R SRA, B

DEF ddcb, 29, 23, 0
	SHF_mXY_R SRA, C

DEF ddcb, 2a, 23, 0
	SHF_mXY_R SRA, D

DEF ddcb, 2b, 23, 0
	SHF_mXY_R SRA, E

DEF ddcb, 2c, 23, 0
	SHF_mXY_R SRA, H

DEF ddcb, 2d, 23, 0
	SHF_mXY_R SRA, L

DEF ddcb, 2e, 23, 0
	SHF_mXY SRA

DEF ddcb, 2f, 23, 0
	SHF_mXY_R SRA, A

DEF cb, 30, 8, 0
	SHF_R SLL, B

DEF cb, 31, 8, 0
	SHF_R SLL, C

DEF cb, 32, 8, 0
	SHF_R SLL, D

DEF cb, 33, 8, 0
	SHF_R SLL, E

DEF cb, 34, 8, 0
	SHF_R SLL, H

DEF cb, 35, 8, 0
	SHF_R SLL, L

DEF cb, 37, 8, 0
	SHF_R SLL, A

DEF cb, 36, 15, 0
	SHF_mRR SLL, HL

DEF ddcb, 30, 23, 0
	SHF_mXY_R SLL, B

DEF ddcb, 31, 23, 0
	SHF_mXY_R SLL, C

DEF ddcb, 32, 23, 0
	SHF_mXY_R SLL, D

DEF ddcb, 33, 23, 0
	SHF_mXY_R SLL, E

DEF ddcb, 34, 23, 0
	SHF_mXY_R SLL, H

DEF ddcb, 35, 23, 0
	SHF_mXY_R SLL, L

DEF ddcb, 36, 23, 0
	SHF_mXY SLL

DEF ddcb, 37, 23, 0
	SHF_mXY_R SLL, A

DEF cb, 38, 8, 0
	SHF_R SRL, B

DEF cb, 39, 8, 0
	SHF_R SRL, C

DEF cb, 3a, 8, 0
	SHF_R SRL, D

DEF cb, 3b, 8, 0
	SHF_R SRL, E

DEF cb, 3c, 8, 0
	SHF_R SRL, H

DEF cb, 3d, 8, 0
	SHF_R SRL, L

DEF cb, 3f, 8, 0
	SHF_R SRL, A

DEF cb, 3e, 15, 0
	SHF_mRR SRL, HL

DEF ddcb, 38, 23, 0
	SHF_mXY_R SRL, B

DEF ddcb, 39, 23, 0
	SHF_mXY_R SRL, C

DEF ddcb, 3a, 23, 0
	SHF_mXY_R SRL, D

DEF ddcb, 3b, 23, 0
	SHF_mXY_R SRL, E

DEF ddcb, 3c, 23, 0
	SHF_mXY_R SRL, H

DEF ddcb, 3d, 23, 0
	SHF_mXY_R SRL, L

DEF ddcb, 3e, 23, 0
	SHF_mXY SRL

DEF ddcb, 3f, 23, 0
	SHF_mXY_R SRL, A

DEF ed, 67, 18, 0
	RRD

DEF ed, 6f, 18, 0
	RLD


; **************************************************************************
; BIT group

DEF cb, 40, 8, 0
	BIT_R_b B, 0

DEF cb, 41, 8, 0
	BIT_R_b C, 0

DEF cb, 42, 8, 0
	BIT_R_b D, 0

DEF cb, 43, 8, 0
	BIT_R_b E, 0

DEF cb, 44, 8, 0
	BIT_R_b H, 0

DEF cb, 45, 8, 0
	BIT_R_b L, 0

DEF cb, 47, 8, 0
	BIT_R_b A, 0

DEF cb, 46, 12, 0
	BIT_mRR_b HL, 0

DEF ddcb, 40, 20, 0
	BIT_mXY_b 0

DEF ddcb, 41, 20, 0
	BIT_mXY_b 0

DEF ddcb, 42, 20, 0
	BIT_mXY_b 0

DEF ddcb, 43, 20, 0
	BIT_mXY_b 0

DEF ddcb, 44, 20, 0
	BIT_mXY_b 0

DEF ddcb, 45, 20, 0
	BIT_mXY_b 0

DEF ddcb, 46, 20, 0
	BIT_mXY_b 0

DEF ddcb, 47, 20, 0
	BIT_mXY_b 0

DEF cb, 48, 8, 0
	BIT_R_b B, 1

DEF cb, 49, 8, 0
	BIT_R_b C, 1

DEF cb, 4a, 8, 0
	BIT_R_b D, 1

DEF cb, 4b, 8, 0
	BIT_R_b E, 1

DEF cb, 4c, 8, 0
	BIT_R_b H, 1

DEF cb, 4d, 8, 0
	BIT_R_b L, 1

DEF cb, 4f, 8, 0
	BIT_R_b A, 1

DEF cb, 4e, 12, 0
	BIT_mRR_b HL, 1

DEF ddcb, 48, 20, 0
	BIT_mXY_b 1

DEF ddcb, 49, 20, 0
	BIT_mXY_b 1

DEF ddcb, 4a, 20, 0
	BIT_mXY_b 1

DEF ddcb, 4b, 20, 0
	BIT_mXY_b 1

DEF ddcb, 4c, 20, 0
	BIT_mXY_b 1

DEF ddcb, 4d, 20, 0
	BIT_mXY_b 1

DEF ddcb, 4e, 20, 0
	BIT_mXY_b 1

DEF ddcb, 4f, 20, 0
	BIT_mXY_b 1

DEF cb, 50, 8, 0
	BIT_R_b B, 2

DEF cb, 51, 8, 0
	BIT_R_b C, 2

DEF cb, 52, 8, 0
	BIT_R_b D, 2

DEF cb, 53, 8, 0
	BIT_R_b E, 2

DEF cb, 54, 8, 0
	BIT_R_b H, 2

DEF cb, 55, 8, 0
	BIT_R_b L, 2

DEF cb, 57, 8, 0
	BIT_R_b A, 2

DEF cb, 56, 12, 0
	BIT_mRR_b HL, 2

DEF ddcb, 50, 20, 0
	BIT_mXY_b 2

DEF ddcb, 51, 20, 0
	BIT_mXY_b 2

DEF ddcb, 52, 20, 0
	BIT_mXY_b 2

DEF ddcb, 53, 20, 0
	BIT_mXY_b 2

DEF ddcb, 54, 20, 0
	BIT_mXY_b 2

DEF ddcb, 55, 20, 0
	BIT_mXY_b 2

DEF ddcb, 56, 20, 0
	BIT_mXY_b 2

DEF ddcb, 57, 20, 0
	BIT_mXY_b 2

DEF cb, 58, 8, 0
	BIT_R_b B, 3

DEF cb, 59, 8, 0
	BIT_R_b C, 3

DEF cb, 5a, 8, 0
	BIT_R_b D, 3

DEF cb, 5b, 8, 0
	BIT_R_b E, 3

DEF cb, 5c, 8, 0
	BIT_R_b H, 3

DEF cb, 5d, 8, 0
	BIT_R_b L, 3

DEF cb, 5f, 8, 0
	BIT_R_b A, 3

DEF cb, 5e, 12, 0
	BIT_mRR_b HL, 3

DEF ddcb, 58, 20, 0
	BIT_mXY_b 3

DEF ddcb, 59, 20, 0
	BIT_mXY_b 3

DEF ddcb, 5a, 20, 0
	BIT_mXY_b 3

DEF ddcb, 5b, 20, 0
	BIT_mXY_b 3

DEF ddcb, 5c, 20, 0
	BIT_mXY_b 3

DEF ddcb, 5d, 20, 0
	BIT_mXY_b 3

DEF ddcb, 5e, 20, 0
	BIT_mXY_b 3

DEF ddcb, 5f, 20, 0
	BIT_mXY_b 3

DEF cb, 60, 8, 0
	BIT_R_b B, 4

DEF cb, 61, 8, 0
	BIT_R_b C, 4

DEF cb, 62, 8, 0
	BIT_R_b D, 4

DEF cb, 63, 8, 0
	BIT_R_b E, 4

DEF cb, 64, 8, 0
	BIT_R_b H, 4

DEF cb, 65, 8, 0
	BIT_R_b L, 4

DEF cb, 67, 8, 0
	BIT_R_b A, 4

DEF cb, 66, 12, 0
	BIT_mRR_b HL, 4

DEF ddcb, 60, 20, 0
	BIT_mXY_b 4

DEF ddcb, 61, 20, 0
	BIT_mXY_b 4

DEF ddcb, 62, 20, 0
	BIT_mXY_b 4

DEF ddcb, 63, 20, 0
	BIT_mXY_b 4

DEF ddcb, 64, 20, 0
	BIT_mXY_b 4

DEF ddcb, 65, 20, 0
	BIT_mXY_b 4

DEF ddcb, 66, 20, 0
	BIT_mXY_b 4

DEF ddcb, 67, 20, 0
	BIT_mXY_b 4

DEF cb, 68, 8, 0
	BIT_R_b B, 5

DEF cb, 69, 8, 0
	BIT_R_b C, 5

DEF cb, 6a, 8, 0
	BIT_R_b D, 5

DEF cb, 6b, 8, 0
	BIT_R_b E, 5

DEF cb, 6c, 8, 0
	BIT_R_b H, 5

DEF cb, 6d, 8, 0
	BIT_R_b L, 5

DEF cb, 6f, 8, 0
	BIT_R_b A, 5

DEF cb, 6e, 12, 0
	BIT_mRR_b HL, 5

DEF ddcb, 68, 20, 0
	BIT_mXY_b 5

DEF ddcb, 69, 20, 0
	BIT_mXY_b 5

DEF ddcb, 6a, 20, 0
	BIT_mXY_b 5

DEF ddcb, 6b, 20, 0
	BIT_mXY_b 5

DEF ddcb, 6c, 20, 0
	BIT_mXY_b 5

DEF ddcb, 6d, 20, 0
	BIT_mXY_b 5

DEF ddcb, 6e, 20, 0
	BIT_mXY_b 5

DEF ddcb, 6f, 20, 0
	BIT_mXY_b 5

DEF cb, 70, 8, 0
	BIT_R_b B, 6

DEF cb, 71, 8, 0
	BIT_R_b C, 6

DEF cb, 72, 8, 0
	BIT_R_b D, 6

DEF cb, 73, 8, 0
	BIT_R_b E, 6

DEF cb, 74, 8, 0
	BIT_R_b H, 6

DEF cb, 75, 8, 0
	BIT_R_b L, 6

DEF cb, 77, 8, 0
	BIT_R_b A, 6

DEF cb, 76, 12, 0
	BIT_mRR_b HL, 6

DEF ddcb, 70, 20, 0
	BIT_mXY_b 6

DEF ddcb, 71, 20, 0
	BIT_mXY_b 6

DEF ddcb, 72, 20, 0
	BIT_mXY_b 6

DEF ddcb, 73, 20, 0
	BIT_mXY_b 6

DEF ddcb, 74, 20, 0
	BIT_mXY_b 6

DEF ddcb, 75, 20, 0
	BIT_mXY_b 6

DEF ddcb, 76, 20, 0
	BIT_mXY_b 6

DEF ddcb, 77, 20, 0
	BIT_mXY_b 6

DEF cb, 78, 8, 0
	BIT_R_b B, 7

DEF cb, 79, 8, 0
	BIT_R_b C, 7

DEF cb, 7a, 8, 0
	BIT_R_b D, 7

DEF cb, 7b, 8, 0
	BIT_R_b E, 7

DEF cb, 7c, 8, 0
	BIT_R_b H, 7

DEF cb, 7d, 8, 0
	BIT_R_b L, 7

DEF cb, 7f, 8, 0
	BIT_R_b A, 7

DEF cb, 7e, 12, 0
	BIT_mRR_b HL, 7

DEF ddcb, 78, 20, 0
	BIT_mXY_b 7

DEF ddcb, 79, 20, 0
	BIT_mXY_b 7

DEF ddcb, 7a, 20, 0
	BIT_mXY_b 7

DEF ddcb, 7b, 20, 0
	BIT_mXY_b 7

DEF ddcb, 7c, 20, 0
	BIT_mXY_b 7

DEF ddcb, 7d, 20, 0
	BIT_mXY_b 7

DEF ddcb, 7e, 20, 0
	BIT_mXY_b 7

DEF ddcb, 7f, 20, 0
	BIT_mXY_b 7

DEF cb, 80, 8, 0
	RES_R_b B, 0

DEF cb, 81, 8, 0
	RES_R_b C, 0

DEF cb, 82, 8, 0
	RES_R_b D, 0

DEF cb, 83, 8, 0
	RES_R_b E, 0

DEF cb, 84, 8, 0
	RES_R_b H, 0

DEF cb, 85, 8, 0
	RES_R_b L, 0

DEF cb, 87, 8, 0
	RES_R_b A, 0

DEF cb, 86, 15, 0
	RES_mRR_b HL, 0

DEF ddcb, 80, 23, 0
	RES_mXY_b_R 0, B

DEF ddcb, 81, 23, 0
	RES_mXY_b_R 0, C

DEF ddcb, 82, 23, 0
	RES_mXY_b_R 0, D

DEF ddcb, 83, 23, 0
	RES_mXY_b_R 0, E

DEF ddcb, 84, 23, 0
	RES_mXY_b_R 0, H

DEF ddcb, 85, 23, 0
	RES_mXY_b_R 0, L

DEF ddcb, 86, 23, 0
	RES_mXY_b 0

DEF ddcb, 87, 23, 0
	RES_mXY_b_R 0, A

DEF cb, 88, 8, 0
	RES_R_b B, 1

DEF cb, 89, 8, 0
	RES_R_b C, 1

DEF cb, 8a, 8, 0
	RES_R_b D, 1

DEF cb, 8b, 8, 0
	RES_R_b E, 1

DEF cb, 8c, 8, 0
	RES_R_b H, 1

DEF cb, 8d, 8, 0
	RES_R_b L, 1

DEF cb, 8f, 8, 0
	RES_R_b A, 1

DEF cb, 8e, 15, 0
	RES_mRR_b HL, 1

DEF ddcb, 88, 23, 0
	RES_mXY_b_R 1, B

DEF ddcb, 89, 23, 0
	RES_mXY_b_R 1, C

DEF ddcb, 8a, 23, 0
	RES_mXY_b_R 1, D

DEF ddcb, 8b, 23, 0
	RES_mXY_b_R 1, E

DEF ddcb, 8c, 23, 0
	RES_mXY_b_R 1, H

DEF ddcb, 8d, 23, 0
	RES_mXY_b_R 1, L

DEF ddcb, 8e, 23, 0
	RES_mXY_b 1

DEF ddcb, 8f, 23, 0
	RES_mXY_b_R 1, A

DEF cb, 90, 8, 0
	RES_R_b B, 2

DEF cb, 91, 8, 0
	RES_R_b C, 2

DEF cb, 92, 8, 0
	RES_R_b D, 2

DEF cb, 93, 8, 0
	RES_R_b E, 2

DEF cb, 94, 8, 0
	RES_R_b H, 2

DEF cb, 95, 8, 0
	RES_R_b L, 2

DEF cb, 97, 8, 0
	RES_R_b A, 2

DEF cb, 96, 15, 0
	RES_mRR_b HL, 2

DEF ddcb, 90, 23, 0
	RES_mXY_b_R 2, B

DEF ddcb, 91, 23, 0
	RES_mXY_b_R 2, C

DEF ddcb, 92, 23, 0
	RES_mXY_b_R 2, D

DEF ddcb, 93, 23, 0
	RES_mXY_b_R 2, E

DEF ddcb, 94, 23, 0
	RES_mXY_b_R 2, H

DEF ddcb, 95, 23, 0
	RES_mXY_b_R 2, L

DEF ddcb, 96, 23, 0
	RES_mXY_b 2

DEF ddcb, 97, 23, 0
	RES_mXY_b_R 2, A

DEF cb, 98, 8, 0
	RES_R_b B, 3

DEF cb, 99, 8, 0
	RES_R_b C, 3

DEF cb, 9a, 8, 0
	RES_R_b D, 3

DEF cb, 9b, 8, 0
	RES_R_b E, 3

DEF cb, 9c, 8, 0
	RES_R_b H, 3

DEF cb, 9d, 8, 0
	RES_R_b L, 3

DEF cb, 9f, 8, 0
	RES_R_b A, 3

DEF cb, 9e, 15, 0
	RES_mRR_b HL, 3

DEF ddcb, 98, 23, 0
	RES_mXY_b_R 3, B

DEF ddcb, 99, 23, 0
	RES_mXY_b_R 3, C

DEF ddcb, 9a, 23, 0
	RES_mXY_b_R 3, D

DEF ddcb, 9b, 23, 0
	RES_mXY_b_R 3, E

DEF ddcb, 9c, 23, 0
	RES_mXY_b_R 3, H

DEF ddcb, 9d, 23, 0
	RES_mXY_b_R 3, L

DEF ddcb, 9e, 23, 0
	RES_mXY_b 3

DEF ddcb, 9f, 23, 0
	RES_mXY_b_R 3, A

DEF cb, a0, 8, 0
	RES_R_b B, 4

DEF cb, a1, 8, 0
	RES_R_b C, 4

DEF cb, a2, 8, 0
	RES_R_b D, 4

DEF cb, a3, 8, 0
	RES_R_b E, 4

DEF cb, a4, 8, 0
	RES_R_b H, 4

DEF cb, a5, 8, 0
	RES_R_b L, 4

DEF cb, a7, 8, 0
	RES_R_b A, 4

DEF cb, a6, 15, 0
	RES_mRR_b HL, 4

DEF ddcb, a0, 23, 0
	RES_mXY_b_R 4, B

DEF ddcb, a1, 23, 0
	RES_mXY_b_R 4, C

DEF ddcb, a2, 23, 0
	RES_mXY_b_R 4, D

DEF ddcb, a3, 23, 0
	RES_mXY_b_R 4, E

DEF ddcb, a4, 23, 0
	RES_mXY_b_R 4, H

DEF ddcb, a5, 23, 0
	RES_mXY_b_R 4, L

DEF ddcb, a6, 23, 0
	RES_mXY_b 4

DEF ddcb, a7, 23, 0
	RES_mXY_b_R 4, A

DEF cb, a8, 8, 0
	RES_R_b B, 5

DEF cb, a9, 8, 0
	RES_R_b C, 5

DEF cb, aa, 8, 0
	RES_R_b D, 5

DEF cb, ab, 8, 0
	RES_R_b E, 5

DEF cb, ac, 8, 0
	RES_R_b H, 5

DEF cb, ad, 8, 0
	RES_R_b L, 5

DEF cb, af, 8, 0
	RES_R_b A, 5

DEF cb, ae, 15, 0
	RES_mRR_b HL, 5

DEF ddcb, a8, 23, 0
	RES_mXY_b_R 5, B

DEF ddcb, a9, 23, 0
	RES_mXY_b_R 5, C

DEF ddcb, aa, 23, 0
	RES_mXY_b_R 5, D

DEF ddcb, ab, 23, 0
	RES_mXY_b_R 5, E

DEF ddcb, ac, 23, 0
	RES_mXY_b_R 5, H

DEF ddcb, ad, 23, 0
	RES_mXY_b_R 5, L

DEF ddcb, ae, 23, 0
	RES_mXY_b 5

DEF ddcb, af, 23, 0
	RES_mXY_b_R 5, A

DEF cb, b0, 8, 0
	RES_R_b B, 6

DEF cb, b1, 8, 0
	RES_R_b C, 6

DEF cb, b2, 8, 0
	RES_R_b D, 6

DEF cb, b3, 8, 0
	RES_R_b E, 6

DEF cb, b4, 8, 0
	RES_R_b H, 6

DEF cb, b5, 8, 0
	RES_R_b L, 6

DEF cb, b7, 8, 0
	RES_R_b A, 6

DEF cb, b6, 15, 0
	RES_mRR_b HL, 6

DEF ddcb, b0, 23, 0
	RES_mXY_b_R 6, B

DEF ddcb, b1, 23, 0
	RES_mXY_b_R 6, C

DEF ddcb, b2, 23, 0
	RES_mXY_b_R 6, D

DEF ddcb, b3, 23, 0
	RES_mXY_b_R 6, E

DEF ddcb, b4, 23, 0
	RES_mXY_b_R 6, H

DEF ddcb, b5, 23, 0
	RES_mXY_b_R 6, L

DEF ddcb, b6, 23, 0
	RES_mXY_b 6

DEF ddcb, b7, 23, 0
	RES_mXY_b_R 6, A

DEF cb, b8, 8, 0
	RES_R_b B, 7

DEF cb, b9, 8, 0
	RES_R_b C, 7

DEF cb, ba, 8, 0
	RES_R_b D, 7

DEF cb, bb, 8, 0
	RES_R_b E, 7

DEF cb, bc, 8, 0
	RES_R_b H, 7

DEF cb, bd, 8, 0
	RES_R_b L, 7

DEF cb, bf, 8, 0
	RES_R_b A, 7

DEF cb, be, 15, 0
	RES_mRR_b HL, 7

DEF ddcb, b8, 23, 0
	RES_mXY_b_R 7, B

DEF ddcb, b9, 23, 0
	RES_mXY_b_R 7, C

DEF ddcb, ba, 23, 0
	RES_mXY_b_R 7, D

DEF ddcb, bb, 23, 0
	RES_mXY_b_R 7, E

DEF ddcb, bc, 23, 0
	RES_mXY_b_R 7, H

DEF ddcb, bd, 23, 0
	RES_mXY_b_R 7, L

DEF ddcb, be, 23, 0
	RES_mXY_b 7

DEF ddcb, bf, 23, 0
	RES_mXY_b_R 7, A

DEF cb, c0, 8, 0
	SET_R_b B, 0

DEF cb, c1, 8, 0
	SET_R_b C, 0

DEF cb, c2, 8, 0
	SET_R_b D, 0

DEF cb, c3, 8, 0
	SET_R_b E, 0

DEF cb, c4, 8, 0
	SET_R_b H, 0

DEF cb, c5, 8, 0
	SET_R_b L, 0

DEF cb, c7, 8, 0
	SET_R_b A, 0

DEF cb, c6, 15, 0
	SET_mRR_b HL, 0

DEF ddcb, c0, 23, 0
	SET_mXY_b_R 0, B

DEF ddcb, c1, 23, 0
	SET_mXY_b_R 0, C

DEF ddcb, c2, 23, 0
	SET_mXY_b_R 0, D

DEF ddcb, c3, 23, 0
	SET_mXY_b_R 0, E

DEF ddcb, c4, 23, 0
	SET_mXY_b_R 0, H

DEF ddcb, c5, 23, 0
	SET_mXY_b_R 0, L

DEF ddcb, c6, 23, 0
	SET_mXY_b 0

DEF ddcb, c7, 23, 0
	SET_mXY_b_R 0, A

DEF cb, c8, 8, 0
	SET_R_b B, 1

DEF cb, c9, 8, 0
	SET_R_b C, 1

DEF cb, ca, 8, 0
	SET_R_b D, 1

DEF cb, cb, 8, 0
	SET_R_b E, 1

DEF cb, cc, 8, 0
	SET_R_b H, 1

DEF cb, cd, 8, 0
	SET_R_b L, 1

DEF cb, cf, 8, 0
	SET_R_b A, 1

DEF cb, ce, 15, 0
	SET_mRR_b HL, 1

DEF ddcb, c8, 23, 0
	SET_mXY_b_R 1, B

DEF ddcb, c9, 23, 0
	SET_mXY_b_R 1, C

DEF ddcb, ca, 23, 0
	SET_mXY_b_R 1, D

DEF ddcb, cb, 23, 0
	SET_mXY_b_R 1, E

DEF ddcb, cc, 23, 0
	SET_mXY_b_R 1, H

DEF ddcb, cd, 23, 0
	SET_mXY_b_R 1, L

DEF ddcb, ce, 23, 0
	SET_mXY_b 1

DEF ddcb, cf, 23, 0
	SET_mXY_b_R 1, A

DEF cb, d0, 8, 0
	SET_R_b B, 2

DEF cb, d1, 8, 0
	SET_R_b C, 2

DEF cb, d2, 8, 0
	SET_R_b D, 2

DEF cb, d3, 8, 0
	SET_R_b E, 2

DEF cb, d4, 8, 0
	SET_R_b H, 2

DEF cb, d5, 8, 0
	SET_R_b L, 2

DEF cb, d7, 8, 0
	SET_R_b A, 2

DEF cb, d6, 15, 0
	SET_mRR_b HL, 2

DEF ddcb, d0, 23, 0
	SET_mXY_b_R 2, B

DEF ddcb, d1, 23, 0
	SET_mXY_b_R 2, C

DEF ddcb, d2, 23, 0
	SET_mXY_b_R 2, D

DEF ddcb, d3, 23, 0
	SET_mXY_b_R 2, E

DEF ddcb, d4, 23, 0
	SET_mXY_b_R 2, H

DEF ddcb, d5, 23, 0
	SET_mXY_b_R 2, L

DEF ddcb, d6, 23, 0
	SET_mXY_b 2

DEF ddcb, d7, 23, 0
	SET_mXY_b_R 2, A

DEF cb, d8, 8, 0
	SET_R_b B, 3

DEF cb, d9, 8, 0
	SET_R_b C, 3

DEF cb, da, 8, 0
	SET_R_b D, 3

DEF cb, db, 8, 0
	SET_R_b E, 3

DEF cb, dc, 8, 0
	SET_R_b H, 3

DEF cb, dd, 8, 0
	SET_R_b L, 3

DEF cb, df, 8, 0
	SET_R_b A, 3

DEF cb, de, 15, 0
	SET_mRR_b HL, 3

DEF ddcb, d8, 23, 0
	SET_mXY_b_R 3, B

DEF ddcb, d9, 23, 0
	SET_mXY_b_R 3, C

DEF ddcb, da, 23, 0
	SET_mXY_b_R 3, D

DEF ddcb, db, 23, 0
	SET_mXY_b_R 3, E

DEF ddcb, dc, 23, 0
	SET_mXY_b_R 3, H

DEF ddcb, dd, 23, 0
	SET_mXY_b_R 3, L

DEF ddcb, de, 23, 0
	SET_mXY_b 3

DEF ddcb, df, 23, 0
	SET_mXY_b_R 3, A

DEF cb, e0, 8, 0
	SET_R_b B, 4

DEF cb, e1, 8, 0
	SET_R_b C, 4

DEF cb, e2, 8, 0
	SET_R_b D, 4

DEF cb, e3, 8, 0
	SET_R_b E, 4

DEF cb, e4, 8, 0
	SET_R_b H, 4

DEF cb, e5, 8, 0
	SET_R_b L, 4

DEF cb, e7, 8, 0
	SET_R_b A, 4

DEF cb, e6, 15, 0
	SET_mRR_b HL, 4

DEF ddcb, e0, 23, 0
	SET_mXY_b_R 4, B

DEF ddcb, e1, 23, 0
	SET_mXY_b_R 4, C

DEF ddcb, e2, 23, 0
	SET_mXY_b_R 4, D

DEF ddcb, e3, 23, 0
	SET_mXY_b_R 4, E

DEF ddcb, e4, 23, 0
	SET_mXY_b_R 4, H

DEF ddcb, e5, 23, 0
	SET_mXY_b_R 4, L

DEF ddcb, e6, 23, 0
	SET_mXY_b 4

DEF ddcb, e7, 23, 0
	SET_mXY_b_R 4, A

DEF cb, e8, 8, 0
	SET_R_b B, 5

DEF cb, e9, 8, 0
	SET_R_b C, 5

DEF cb, ea, 8, 0
	SET_R_b D, 5

DEF cb, eb, 8, 0
	SET_R_b E, 5

DEF cb, ec, 8, 0
	SET_R_b H, 5

DEF cb, ed, 8, 0
	SET_R_b L, 5

DEF cb, ef, 8, 0
	SET_R_b A, 5

DEF cb, ee, 15, 0
	SET_mRR_b HL, 5

DEF ddcb, e8, 23, 0
	SET_mXY_b_R 5, B

DEF ddcb, e9, 23, 0
	SET_mXY_b_R 5, C

DEF ddcb, ea, 23, 0
	SET_mXY_b_R 5, D

DEF ddcb, eb, 23, 0
	SET_mXY_b_R 5, E

DEF ddcb, ec, 23, 0
	SET_mXY_b_R 5, H

DEF ddcb, ed, 23, 0
	SET_mXY_b_R 5, L

DEF ddcb, ee, 23, 0
	SET_mXY_b 5

DEF ddcb, ef, 23, 0
	SET_mXY_b_R 5, A

DEF cb, f0, 8, 0
	SET_R_b B, 6

DEF cb, f1, 8, 0
	SET_R_b C, 6

DEF cb, f2, 8, 0
	SET_R_b D, 6

DEF cb, f3, 8, 0
	SET_R_b E, 6

DEF cb, f4, 8, 0
	SET_R_b H, 6

DEF cb, f5, 8, 0
	SET_R_b L, 6

DEF cb, f7, 8, 0
	SET_R_b A, 6

DEF cb, f6, 15, 0
	SET_mRR_b HL, 6

DEF ddcb, f0, 23, 0
	SET_mXY_b_R 6, B

DEF ddcb, f1, 23, 0
	SET_mXY_b_R 6, C

DEF ddcb, f2, 23, 0
	SET_mXY_b_R 6, D

DEF ddcb, f3, 23, 0
	SET_mXY_b_R 6, E

DEF ddcb, f4, 23, 0
	SET_mXY_b_R 6, H

DEF ddcb, f5, 23, 0
	SET_mXY_b_R 6, L

DEF ddcb, f6, 23, 0
	SET_mXY_b 6

DEF ddcb, f7, 23, 0
	SET_mXY_b_R 6, A

DEF cb, f8, 8, 0
	SET_R_b B, 7

DEF cb, f9, 8, 0
	SET_R_b C, 7

DEF cb, fa, 8, 0
	SET_R_b D, 7

DEF cb, fb, 8, 0
	SET_R_b E, 7

DEF cb, fc, 8, 0
	SET_R_b H, 7

DEF cb, fd, 8, 0
	SET_R_b L, 7

DEF cb, ff, 8, 0
	SET_R_b A, 7

DEF cb, fe, 15, 0
	SET_mRR_b HL, 7

DEF ddcb, f8, 23, 0
	SET_mXY_b_R 7, B

DEF ddcb, f9, 23, 0
	SET_mXY_b_R 7, C

DEF ddcb, fa, 23, 0
	SET_mXY_b_R 7, D

DEF ddcb, fb, 23, 0
	SET_mXY_b_R 7, E

DEF ddcb, fc, 23, 0
	SET_mXY_b_R 7, H

DEF ddcb, fd, 23, 0
	SET_mXY_b_R 7, L

DEF ddcb, fe, 23, 0
	SET_mXY_b 7

DEF ddcb, ff, 23, 0
	SET_mXY_b_R 7, A


; **************************************************************************
; jump group

DEF op, c3, 10, 0
	JP_NN

DEF op, da, 10, 0
   JP_cc_NN z, C

DEF op, d2, 10, 0
   JP_cc_NN nz, C

DEF op, ca, 10, 0
   JP_cc_NN z, Z

DEF op, c2, 10, 0
   JP_cc_NN nz, Z

DEF op, ea, 10, 0
   JP_cc_NN z, P

DEF op, e2, 10, 0
   JP_cc_NN nz, P

DEF op, fa, 10, 0
   JP_cc_NN z, S

DEF op, f2, 10, 0
   JP_cc_NN nz, S

DEF op, 18, 12, 0
	JR_N

DEF op, 38, 7, 12
   JR_cc_N z, C

DEF op, 30, 7, 12
   JR_cc_N nz, C

DEF op, 28, 7, 12
   JR_cc_N z, Z

DEF op, 20, 7, 12
   JR_cc_N nz, Z

DEF op, e9, 4, 0
	JP_RR HL

DEF dd, e9, 8, 0
   JP_RR IX

DEF fd, e9, 8, 0
   JP_RR IY

DEF op, 10, 8, 13
	DJNZ_N


; **************************************************************************
; call and return group

DEF op, cd, 17, 0
	CALL_NN

DEF op, c4, 10, 17
   CALL_cc_NN nz, Z

DEF op, cc, 10, 17
   CALL_cc_NN z, Z

DEF op, d4, 10, 17
   CALL_cc_NN nz, C

DEF op, dc, 10, 17
   CALL_cc_NN z, C

DEF op, e4, 10, 17
   CALL_cc_NN nz, P

DEF op, ec, 10, 17
   CALL_cc_NN z, P

DEF op, f4, 10, 17
   CALL_cc_NN nz, S

DEF op, fc, 10, 17
   CALL_cc_NN z, S

DEF op, c9, 10, 0
	RET

DEF op, c0, 5, 11
   RET_cc nz, Z

DEF op, c8, 5, 11
   RET_cc z, Z

DEF op, d0, 5, 11
   RET_cc nz, C

DEF op, d8, 5, 11
   RET_cc z, C

DEF op, e0, 5, 11
   RET_cc nz, P

DEF op, e8, 5, 11
   RET_cc z, P

DEF op, f0, 5, 11
   RET_cc nz, S

DEF op, f8, 5, 11
   RET_cc z, S

DEF ed, 4d, 14, 0
	RETI

DEF ed, 45, 14, 0
	RETN

DEF ed, 5d, 14, 0
	RETN

DEF ed, 55, 14, 0
	RETN

DEF ed, 6d, 14, 0
	RETN

DEF ed, 65, 14, 0
	RETN

DEF ed, 7d, 14, 0
	RETN

DEF ed, 75, 14, 0
	RETN

DEF op, c7, 11, 0
	RST 0h

DEF op, cf, 11, 0
	RST 8h

DEF op, d7, 11, 0
	RST 10h

DEF op, df, 11, 0
	RST 18h

DEF op, e7, 11, 0
	RST 20h

DEF op, ef, 11, 0
	RST 28h

DEF op, f7, 11, 0
	RST 30h

DEF op, ff, 11, 0
	RST 38h


; **************************************************************************
; I/O

DEF op, d3, 11, 0
	OUT_N_R A

DEF op, db, 11, 0
	IN_R_N A

DEF ed, 40, 12, 0
	IN_R B

DEF ed, 48, 12, 0
	IN_R C

DEF ed, 50, 12, 0
	IN_R D

DEF ed, 58, 12, 0
	IN_R E

DEF ed, 60, 12, 0
	IN_R H

DEF ed, 68, 12, 0
	IN_R L

DEF ed, 70, 12, 0
	IN_F

DEF ed, 78, 12, 0
	IN_R A

DEF ed, 41, 12, 0
	OUT_R B

DEF ed, 49, 12, 0
	OUT_R C

DEF ed, 51, 12, 0
	OUT_R D

DEF ed, 59, 12, 0
	OUT_R E

DEF ed, 61, 12, 0
	OUT_R H

DEF ed, 69, 12, 0
	OUT_R L

DEF ed, 71, 12, 0
	OUT_0

DEF ed, 79, 12, 0
	OUT_R A

DEF ed, a2, 16, 0
	INI

DEF ed, aa, 16, 0
	IND

DEF ed, b2, 16, 21
	INIR

DEF ed, ba, 16, 21
	INDR

DEF ed, a3, 16, 0
	OUTI

DEF ed, ab, 16, 0
	OUTD

DEF ed, b3, 16, 21
	OTIR

DEF ed, bb, 16, 21
	OTDR


; **************************************************************************
; misc

DEF op, 27, 4, 0
	DAA

DEF op, 2f, 4, 0
	CPL

DEF ed, 44, 8, 0
	NEG

DEF ed, 4c, 8, 0
	NEG

DEF ed, 54, 8, 0
	NEG

DEF ed, 5c, 8, 0
	NEG

DEF ed, 64, 8, 0
	NEG

DEF ed, 6c, 8, 0
	NEG

DEF ed, 74, 8, 0
	NEG

DEF ed, 7c, 8, 0
	NEG

DEF op, 3f, 4, 0
	CCF

DEF op, 37, 4, 0
	SCF

DEF op, 00, 4, 0
	NOP

DEF op, 76, 4, 0
	HALT

DEF op, fb, 4, 0
	EI

DEF op, f3, 4, 0
	DI

DEF ed, 46, 8, 0
	IM 0

DEF ed, 66, 8, 0
	IM 0

DEF ed, 56, 8, 0
	IM 1

DEF ed, 76, 8, 0
	IM 1

DEF ed, 4e, 8, 0
	IM 1

DEF ed, 6e, 8, 0
	IM 1

DEF ed, 5e, 8, 0
	IM 2

DEF ed, 7e, 8, 0
	IM 2


; --- the end ---
