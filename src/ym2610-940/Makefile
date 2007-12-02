
# you may or may not need to change this
#devkit_path = x:/stuff/dev/devkitgp2x/
#devkit_path = /usr/opt/eldk/usr/
devkit_path = /opt/open2x/gcc-4.1.1-glibc-2.3.6/
#lgcc_path = $(devkit_path)lib/gcc/arm-linux/4.0.0/
lgcc_path = $(devkit_path)lib/gcc/arm-open2x-linux/4.1.1/
#CROSS = arm-linux-
CROSS = $(devkit_path)bin/arm-open2x-linux-

# settings
#up = 1

DEFINC = -I../.. -I. -D__GP2X__ -DARM # -DBENCHMARK
#COPT_COMMON = -static -s -O3 -msoft-float -ftracer -fstrength-reduce -Wall -funroll-loops -fomit-frame-pointer -fstrict-aliasing -ffast-math
#COPT_COMMON = -static -s -O2 -msoft-float -ftracer -fstrength-reduce -Wall -fomit-frame-pointer -fstrict-aliasing -ffast-math
COPT_COMMON = -static -s -Os  -ftracer -fstrength-reduce -Wall -fomit-frame-pointer -fstrict-aliasing -ffast-math 
#COPT_COMMON = -static -s -Os -Wall -fomit-frame-pointer -fstrict-aliasing
COPT = $(COPT_COMMON) -mtune=arm940t


GCC = $(CROSS)gcc
STRIP = $(CROSS)strip
AS = $(CROSS)as
LD = $(CROSS)ld
OBJCOPY = $(CROSS)objcopy

all: gngeo940.bin


.c.o: 940shared.h
	@echo $<
	$(GCC) $(COPT) $(DEFINC) -c $< -o $@
.s.o:
	@echo $<
	$(GCC) $(COPT) $(DEFINC) -c $< -o $@


# stuff for 940 core

# init, emu_control, emu
OBJS940 += 940init.o 940.o memcpy.o  2610intf.o drz80_interf.o ym2610.o DrZ80.o timer.o
# the asm code seems to be faster when run on 920, but not on 940 for some reason
# OBJS940 += ../../Pico/sound/ym2612_asm.o

# uClibc library code
OBJS940 += uClibc/memset.o uClibc/s_floor.o uClibc/e_pow.o uClibc/e_sqrt.o uClibc/s_fabs.o
OBJS940 += uClibc/s_scalbn.o uClibc/s_copysign.o uClibc/k_sin.o uClibc/k_cos.o uClibc/s_sin.o
OBJS940 += uClibc/e_rem_pio2.o uClibc/k_rem_pio2.o uClibc/e_log.o uClibc/wrappers.o

gngeo940.bin : code940.gpe
	@echo $@
	@$(OBJCOPY) -O binary $< $@

code940.gpe : $(OBJS940)
	@echo $@
	@$(LD) -static -e code940 -Ttext 0x0 $^ -L$(lgcc_path) -lgcc -o $@ -Map code940.map


clean:
	rm *.o uClibc/*.o gngeo940.bin code940.gpe code940.map

# uClibc/e_pow.o : uClibc/e_pow.c
#	@echo $<
#	@$(GCC) $(COPT) $(DEFINC) -fno-profile-generate -c $< -o $@

# uClibc/e_sqrt.o : uClibc/e_sqrt.c
#	@echo $<
#	@$(GCC) $(COPT) $(DEFINC) -fno-profile-generate -c $< -o $@
