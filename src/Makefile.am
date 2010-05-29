if USE_STARSCREAM
m68k_subdir = star
m68k_lib = star/libstar.a
m68ki_src = star_interf.c
else
if USE_GENERATOR68K
m68k_subdir = generator68k
m68k_lib = generator68k/libgenerator68k.a
m68ki_src = generator68k_interf.c
else
if USE_CYCLONE
m68k_subdir = cyclone
m68k_lib = cyclone/libcyclone.a
m68ki_src = cyclone_interf.c
endif
endif
endif

if USE_RAZE
z80_subdir = raze
z80_lib = raze/libraze.a
z80i_src = raze_interf.c
else
if USE_MAMEZ80
z80_subdir = mamez80
z80_lib = mamez80/libmamez80.a
z80i_src = mamez80_interf.c
else
if USE_DRZ80
z80_subdir = drz80
z80_lib = drz80/libdrz80.a
z80i_src = drz80_interf.c
endif
endif
endif

if USE_GUI
gui_lib = gui/libgui.a
gui_subdir = gui
gui_src = gui_interf.c gui_interf.h
endif

if HAVE_NASM
asm_lib = screen_i386.o video_i386.o
else
if PROCESSOR_ARM
#asm_lib = video_arm.o memcpy.o
if USE_GP2X
pdep_src = video_arm.s memcpy.S gp2x.h gp2x.c
else
pdep_src = video_arm.s memcpy.S
endif
else
asm_lib = 
endif
endif




INCLUDES = -DDATA_DIRECTORY='"${pkgdatadir}"'

SUBDIRS = effect blitter ym2610 $(m68k_subdir) $(z80_subdir) $(gui_subdir)

bin_PROGRAMS = gngeo
gngeo_SOURCES = fileio.c         pd4990a.c        gngeo_icon.h      crypt.h         \
		sound.h          video.c          fileio.h          profiler.c      \
		main.c           pd4990a.h        video.h           list.c          \
		memory.c         profiler.h                list.h          \
		blitter.h        font.h           memory.h          resfile.c       \
		timer.c          effect.h         frame_skip.c      messages.c      \
		screen.c         timer.h          emu.c             frame_skip.h    \
		messages.h       screen.h         unzip.h           emu.h           \
		neocrypt.c       sound.c          unzip.c           resfile.h       \
		debug.c          debug.h          neocrypt.h	    state.c         \
		state.h          video_template.h conf.c            conf.h          \
		transpack.h      transpack.c              \
		menu.c           menu.h           \
			         roms.h           roms.c            mame_layer.h    \
		mame_layer.c     neoboot.c        event.h           event.c         \
		stb_image.c      stb_image.h      stb_zlib.c		stb_zlib.h       \
		$(pdep_src) $(gui_src) $(m68ki_src) $(z80i_src)

EXTRA_DIST = c32.mac screen_i386.asm video_i386.asm video_i386.inc getopt.h

if USE_GP2X
#gngeo_LDADD = blitter/libblitter.a ym2610/libym2610.a $(m68k_lib) $(z80_lib) $(asm_lib) $(gui_lib) #$(LIBOBJS)
gngeo_LDADD = blitter/libblitter.a $(m68k_lib)  #$(LIBOBJS)

else
gngeo_LDADD = effect/libeffect.a blitter/libblitter.a ym2610/libym2610.a $(m68k_lib) $(z80_lib) $(asm_lib) $(gui_lib) #$(LIBOBJS)
endif

#video_i386.o : video_i386.asm video_i386.inc
#	nasm -f elf  video_i386.asm -o video_i386.o

#screen_i386.o : screen_i386.asm
#	nasm -f elf  screen_i386.asm -o screen_i386.o

video_i386.o : video_i386.inc
video_i386.o screen_i386.o : %.o: %.asm
	nasm -f elf -I $(srcdir)/ $<  -o $@ 
