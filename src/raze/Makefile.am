noinst_LIBRARIES = libraze.a
libraze_a_SOURCES = 
EXTRA_DIST = raze.asm  raze.h  raze.inc  raze.reg  raze.txt  strip_
libraze_a_LIBADD = raze.o

raze.asmu : raze.asm
	./strip_ raze.asm raze.asmu

raze.incu : raze.inc
	./strip_ raze.inc raze.incu

raze.regu : raze.reg
	./strip_ raze.reg raze.regu

raze2.asm: raze.asmu raze.incu raze.regu
	nasm -w+orphan-labels -e $< -o $@

# This assembles the file, once it's been pre-processed
raze.o: raze2.asm
	nasm -w+orphan-labels -f elf $< -o $@ -praze.regu

