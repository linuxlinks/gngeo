if USE_STARSCREAM
m68k_subdir = star
m68k_lib = star/libstar.a
else
if USE_GENERATOR68K
m68k_subdir = generator68k
m68k_lib = generator68k/libgenerator68k.a
else
if USE_CYCLONE
m68k_subdir = cyclone
m68k_lib = cyclone/libcyclone.a
endif
endif
endif

if USE_RAZE
z80_subdir = raze
z80_lib = raze/libraze.a
else
if USE_MAMEZ80
z80_subdir = mamez80
z80_lib = mamez80/libmamez80.a
else
if USE_DRZ80
z80_subdir = drz80
z80_lib = drz80/libdrz80.a
endif
endif
endif

if USE_GUI
gui_lib = gui/libgui.a
gui_subdir = gui
endif

if HAVE_NASM
asm_lib = screen_i386.o video_i386.o
else
if USE_GP2X
asm_lib = video_arm.o memcpy.o
else
asm_lib = 
endif
endif




INCLUDES = -DDATA_DIRECTORY='"${pkgdatadir}"'

SUBDIRS = effect blitter ym2610 $(m68k_subdir) $(z80_subdir) $(gui_subdir)

bin_PROGRAMS = gngeo
gngeo_SOURCES = fileio.c         pd4990a.c        streams.c         gngeo_icon.h    \
		sound.h          video.c          fileio.h          profiler.c      \
		main.c           pd4990a.h        star_interf.c     video.h         \
		memory.c         profiler.h       streams.h         mamez80_interf.c\
		blitter.h        font.h           memory.h          raze_interf.c   \
		timer.c          effect.h         frame_skip.c      messages.c      \
		screen.c         timer.h          emu.c             frame_skip.h    \
		messages.h       screen.h         unzip.c           emu.h           \
		neocrypt.c       sound.c          unzip.h           generator68k_interf.c \
		debug.c          debug.h          neocrypt.h	    state.c         \
		state.h          video_template.h conf.c            conf.h          \
		transpack.h      transpack.c      pbar.h            pbar.c          \
		driver.h         driver.c         list.c            list.h	    \
		gui_interf.c     gui_interf.h     videogl.c	    videogl.h       \
		cyclone_interf.c gp2x.h		  gp2x.c	    drz80_interf.c  \
		menu.c		 menu.h

EXTRA_DIST = c32.mac screen_i386.asm video_i386.asm video_i386.inc getopt.h video_arm.s memcpy.S

if USE_GP2X
gngeo_LDADD = blitter/libblitter.a ym2610/libym2610.a $(m68k_lib) $(z80_lib) $(asm_lib) $(gui_lib) #$(LIBOBJS)
else
gngeo_LDADD = effect/libeffect.a blitter/libblitter.a ym2610/libym2610.a $(m68k_lib) $(z80_lib) $(asm_lib) $(gui_lib) #$(LIBOBJS)
endif

video_i386.o : video_i386.asm video_i386.inc
	nasm -f elf  video_i386.asm -o video_i386.o

screen_i386.o : screen_i386.asm
	nasm -f elf  screen_i386.asm -o screen_i386.o

