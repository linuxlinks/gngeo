noinst_LIBRARIES = libraze.a
libraze_a_SOURCES = 
EXTRA_DIST = raze.asm  raze.h  raze.inc  raze.reg  raze.txt  strip_
libraze_a_LIBADD = raze.o

#raze.asmu : raze.asm
#	$(srcdir)/strip_ raze.asm raze.asmu

#raze.incu : raze.inc
#	$(srcdir)/strip_ raze.inc raze.incu

#raze.regu : raze.reg
#	$(srcdir)/strip_ raze.reg raze.regu

raze.asmu : raze.asm
	$(srcdir)/strip_ $< $@
raze.incu : raze.inc
	$(srcdir)/strip_ $< $@
raze.regu : raze.reg
	$(srcdir)/strip_ $< $@

raze2.asm: raze.asmu raze.incu raze.regu
	nasm -w+orphan-labels -I $(srcdir)/ -e $< -o $@

# This assembles the file, once it's been pre-processed
raze.o: raze2.asm
	nasm -w+orphan-labels -f elf -I $(srcdir)/ $< -o $@ -praze.regu

