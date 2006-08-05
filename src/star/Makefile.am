noinst_LIBRARIES = libstar.a
libstar_a_SOURCES = cpudebug.c  cpudebug.h 
EXTRA_DIST =  star.c  starcpu.h  stardoc.txt  strip_
libstar_a_LIBADD = star.o

mkstar : star.c starcpu.h
	$(CC)  $(srcdir)/star.c -o mkstar

star.asm : mkstar
	./mkstar _star.asm -hog
	$(srcdir)/strip_ _star.asm star.asm

star.o : %.o: %.asm
	nasm -f elf -I $(srcdir)/ $<