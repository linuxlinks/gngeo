all-before : config.h star.o raze.o

config.h : config.win32
	copy config.win32 config.h

star.o : star.asm
	nasm -f win32 star.asm -o star.o

star.asm : mkstar.exe
	mkstar.exe star.asm -hog

mkstar.exe : src/star/star.c
	$(CC) src/star/star.c -o mkstar.exe

raze2.asm: src/raze/raze.asm src/raze/raze.inc src/raze/raze.reg
	nasm -w+orphan-labels -I src/raze/ -e src/raze/raze.asm -o raze2.asm

raze.o: raze2.asm
	nasm -w+orphan-labels -f win32 -I src/raze/ raze2.asm -o raze.o -psrc/raze/raze.reg


