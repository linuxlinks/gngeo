## Process this file with automake to produce Makefile.in

noinst_PROGRAMS = def68k gen68k
BUILT_SOURCES = def68k-iibs.h def68k-funcs.h def68k-proto.h \
                cpu68k-0.c cpu68k-1.c cpu68k-2.c cpu68k-3.c \
                cpu68k-4.c cpu68k-5.c cpu68k-6.c cpu68k-7.c \
                cpu68k-8.c cpu68k-9.c cpu68k-a.c cpu68k-b.c \
                cpu68k-c.c cpu68k-d.c cpu68k-e.c cpu68k-f.c

## stage one - create iibs, funcs and proto header files from def68k.def

#def68k_SOURCES = def68k.c tab68k.c
INCLUDES = -I../ -I.

#head_SOURCES = def68k-iibs.h def68k-funcs.h def68k-proto.h

## see automake.info section on 'Support for executable extensions'

#head$(EXEEXT):	def68k-iibs.h def68k-funcs.h def68k-proto.h

#def68k-iibs.h:	def68k def68k.def
#	./def68k
#def68k-funcs.h:	def68k def68k.def
#	./def68k
#def68k-proto.h:	def68k def68k.def
#	./def68k

gcc_build_verbose = $(gcc_build_verbose_$(V))
gcc_build_verbose_ = $(gcc_build_verbose_$(AM_DEFAULT_VERBOSITY))
gcc_build_verbose_0 = @echo "  HCC    $@";

def68k-proto.h def68k-funcs.h def68k-iibs.h : def68k def68k.def
	$(AM_V_at)if [ ! -f def68k.def ] ; then cp $(srcdir)/def68k.def . ;fi
	$(AM_V_GEN)./def68k 2>&1 >/dev/null

cpu68k.c : def68k-proto.h

## stage two - create cpu C files and create library

gen68k_SOURCES = gen68k.c tab68k2.c
gen68k_DEPENDENCIES = def68k-iibs.h def68k

noinst_LIBRARIES = libgenerator68k.a
libgenerator68k_a_SOURCES = cpu68k.c reg68k.c registers.h diss68k.c \
			    mem68k.h generator.h cpu68k.h compile.h tab68k.c \
		 	    cpu68k-inline.h reg68k.h

libgenerator68k_a_LIBADD = cpu68k-0.o cpu68k-1.o cpu68k-2.o cpu68k-3.o \
                          cpu68k-4.o cpu68k-5.o cpu68k-6.o cpu68k-7.o \
                          cpu68k-8.o cpu68k-9.o cpu68k-a.o cpu68k-b.o \
                          cpu68k-c.o cpu68k-d.o cpu68k-e.o cpu68k-f.o #compile.o


libgenerator68k_a_DEPENDENCIES = cpu68k-0.o cpu68k-1.o cpu68k-2.o cpu68k-3.o \
                          cpu68k-4.o cpu68k-5.o cpu68k-6.o cpu68k-7.o \
                          cpu68k-8.o cpu68k-9.o cpu68k-a.o cpu68k-b.o \
                          cpu68k-c.o cpu68k-d.o cpu68k-e.o cpu68k-f.o \
			  def68k-iibs.h #compile.o

EXTRA_DIST = def68k.def compile.cg ccg/alpha ccg/asm-i386.h ccg/asm-sparc.h \
		ccg/asm-ppc.h ccg/asm-cache.h ccg/asm-common.h ccg/ccg.c \
		ccg/insns-i386.h ccg/insns-ppc.h ccg/insns-sparc.h

MOSTLYCLEANFILES = ${BUILT_SOURCES} ${head_SOURCES}

#ccg/ccg : ccg/ccg.c
#	$(COMPILE) -DCCGVERSION="\"1.1\""  ccg/ccg.c -o ccg/ccg

#compile.c : compile.cg ccg/ccg
#	./ccg/ccg -o compile.c compile.cg

#def68k : def68k.c
#	gcc def68k.c tab68k.c -o def68k -I .. -I ../.. -I . `sdl-config --cflags`
def68k.o gen68k.o tab68k2.o : %.o : %.c
	$(gcc_build_verbose)$(CC_FOR_BUILD) -c $< -o $@ -I .. -I ../.. -I . `sdl-config --cflags`

def68k : def68k.o tab68k2.o
	$(gcc_build_verbose)$(CC_FOR_BUILD) def68k.o tab68k2.o -o $@ 

gen68k : gen68k.o tab68k2.o 
	$(gcc_build_verbose)$(CC_FOR_BUILD) gen68k.o tab68k2.o -o $@

#compile.o : compile.c 
#	$(COMPILE) -c $< -o $@

CPU_SOURCE = cpu68k-0.c  cpu68k-1.c  cpu68k-2.c  cpu68k-3.c  \
	     cpu68k-4.c  cpu68k-5.c  cpu68k-6.c  cpu68k-7.c  \
	     cpu68k-8.c  cpu68k-9.c  cpu68k-a.c  cpu68k-b.c  \
	     cpu68k-c.c  cpu68k-d.c  cpu68k-e.c  cpu68k-f.c

CPU_OBJ    = cpu68k-0.o  cpu68k-1.o  cpu68k-2.o  cpu68k-3.o  \
	     cpu68k-4.o  cpu68k-5.o  cpu68k-6.o  cpu68k-7.o  \
	     cpu68k-8.o  cpu68k-9.o  cpu68k-a.o  cpu68k-b.o  \
	     cpu68k-c.o  cpu68k-d.o  cpu68k-e.o  cpu68k-f.o

$(CPU_SOURCE) : gen68k
	$(AM_V_GEN)./gen68k 2>&1 >/dev/null

$(CPU_OBJ) : %.o : %.c
	$(AM_V_CC)$(COMPILE) -c $< -o $@

#cpu68k-0.c:	gen68k
#	./gen68k
#cpu68k-1.c:	gen68k
#	./gen68k
#cpu68k-2.c:	gen68k
#	./gen68k
#cpu68k-3.c:	gen68k
#	./gen68k
#cpu68k-4.c:	gen68k
#	./gen68k
#cpu68k-5.c:	gen68k
#	./gen68k
#cpu68k-6.c:	gen68k
#	./gen68k
#cpu68k-7.c:	gen68k
#	./gen68k
#cpu68k-8.c:	gen68k
#	./gen68k
#cpu68k-9.c:	gen68k
#	./gen68k
#cpu68k-a.c:	gen68k
#	./gen68k
#cpu68k-b.c:	gen68k
#	./gen68k
#cpu68k-c.c:	gen68k
#	./gen68k
#cpu68k-d.c:	gen68k
#	./gen68k
#cpu68k-e.c:	gen68k
#	./gen68k
#cpu68k-f.c:	gen68k
#	./gen68k

#cpu68k-0.o: cpu68k-0.c
#	$(COMPILE) -c cpu68k-0.c
#cpu68k-1.o: cpu68k-1.c
#	$(COMPILE) -c cpu68k-1.c
#cpu68k-2.o: cpu68k-2.c
#	$(COMPILE) -c cpu68k-2.c
#cpu68k-3.o: cpu68k-3.c
#	$(COMPILE) -c cpu68k-3.c
#cpu68k-4.o: cpu68k-4.c
#	$(COMPILE) -c cpu68k-4.c
#cpu68k-5.o: cpu68k-5.c
#	$(COMPILE) -c cpu68k-5.c
#cpu68k-6.o: cpu68k-6.c
#	$(COMPILE) -c cpu68k-6.c
#cpu68k-7.o: cpu68k-7.c
#	$(COMPILE) -c cpu68k-7.c
#cpu68k-8.o: cpu68k-8.c
#	$(COMPILE) -c cpu68k-8.c
#cpu68k-9.o: cpu68k-9.c
#	$(COMPILE) -c cpu68k-9.c
#cpu68k-a.o: cpu68k-a.c
#	$(COMPILE) -c cpu68k-a.c
#cpu68k-b.o: cpu68k-b.c
#	$(COMPILE) -c cpu68k-b.c
#cpu68k-c.o: cpu68k-c.c
#	$(COMPILE) -c cpu68k-c.c
#cpu68k-d.o: cpu68k-d.c
#	$(COMPILE) -c cpu68k-d.c
#cpu68k-e.o: cpu68k-e.c
#	$(COMPILE) -c cpu68k-e.c
#cpu68k-f.o: cpu68k-f.c
#	$(COMPILE) -c cpu68k-f.c
