GNGEO : A NeoGeo emulator for linux (and maybe some other unix)

Licence:
	Gngeo is build arround many different block with various licence.
	The original code is released under the GPLV2 with this special exeption:
	
		As a special exception, You have
		the permission to link the code of this program with
		independent modules,regardless of the license terms of these
		independent modules, and to copy and distribute the resulting
		executable under terms of your choice, provided that you also
		meet, for each linked independent module, the terms and conditions
		of the license of that module. An independent module is a module
		which is not derived from or based on Gngeo. If you modify
		this library, you may extend this exception to your version of the
		library, but you are not obligated to do so.  If you do not wish
		to do so, delete this exception statement from your version.

	Gngeo could not exist without the Mame project, and some code come
	directly from it (the ym2610 for example). As you may know, the Mame
	licence forbid commercial use, and as a conscequence, commercial use
	of gngeo (as a whole) is also forbided. This is the same with Cyclone,
	Drz80, Raze and Starscream.	  

REQUIREMENT : 
	SDL-1.2.x and the libz
	optional: nasm 0.98 for i386 asm optimisation
	You will also need a neogeo BIOS

INSTALLATION :
	./configure && make
	then 'make install' as root 

	The configure script will detect the presence of nasm, and
	choose the different CPU core in consequence. You can force
	the use of one cpu core with the following configure switch:
	 --with-m68kcore=[cyclone|starscream|gen68k]
	 --with-z80core=[drz80|raze|mamez80]
    
    cyclone is written in arm assembler (default on arm target)
    starscream is written in x86 assembler (default on x86 target if nasm is found)
    gen68k is written in portable C (default in every other case)
    
    drz80 is written in arm assembler (default on arm target)
    raze is written in x86 assembler (default on x86 target if nasm is found)
    mamez80 is written in portable C  (default in every other case)

	To disable completly i386 assembler on x86
	 --disable-i386

CROSSCOMPILATION :
	To build gngeo in a crosscompilation environement, use the folowing
	./configure --build=i686-pc-linux-gnu --host=[HOST] --with-sdl-prefix=[Path to your corsscompiled SDL] CFLAGS=-I[PATH to the default include dir of your crosscompil env]

USAGE :
	You can start a game with the folowing command:
	# gngeo game
	where game is the mame name of the rom, for example mslug for Metal Slug

Usage: gngeo [OPTION]... ROMSET
Emulate the NeoGeo rom designed by ROMSET

      --68kclock=x           Overclock the 68k by x% (-x% for underclk)
      --autoframeskip        Enable auto frameskip
      --bench                Draw x frames, then quit and show average fps
  -B, --biospath=PATH        Tell gngeo where your neogeo bios is
  -b, --blitter=Blitter      Use the specified blitter (help for a list)
      --country=...          Set the contry to japan, asia, usa or europe
  -D, --debug                Start with inline debuger
  -e, --effect=Effetc        Use the specified effect (help for a list)
      --forcepc              Force the PC to a correct value at startup
  -f, --fullscreen           Start gngeo in fullscreen
  -d, --gngeo.dat=PATH       Tell gngeo where his ressource file is
  -h, --help                 Print this help and exit
  -H, --hwsurface            Use hardware surface for the screen
  -I, --interpolation        Merge the last frame and the current
      --joystick             Enable joystick support
  -l, --listgame             Show all the game available in the romrc
      --libglpath=PATH       Path to your libGL.so
  -P, --pal                  Use PAL timing (buggy)
      --p1control=...        Player1 control configutation
      --p2control=...        Player2 control configutation
      --p1hotkey0=...        Player1 Hotkey 0 configuration
      --p1hotkey1=...        Player1 Hotkey 1 configuration
      --p1hotkey2=...        Player1 Hotkey 2 configuration
      --p1hotkey3=...        Player1 Hotkey 3 configuration
      --p2hotkey0=...        Player2 Hotkey 0 configuration
      --p2hotkey1=...        Player2 Hotkey 1 configuration
      --p2hotkey2=...        Player2 Hotkey 2 configuration
      --p2hotkey3=...        Player2 Hotkey 3 configuration
  -r, --raster               Enable the raster interrupt
  -i, --rompath=PATH         Tell gngeo where your roms are
      --sound                Enable sound
      --showfps              Show FPS at startup
      --sleepidle            Sleep when idle
      --screen320            Use 320x224 output screen (instead 304x224)
      --system=...           Set the system to home, arcade or unibios
      --scale=X              Scale the resolution by X
      --samplerate=RATE      Set the sample rate to RATE
  -t, --transpack=Transpack  Use the specified transparency pack
  -v, --version              Show version and exit
      --z80clock=x           Overclock the Z80 by x% (-x% for underclk)

All boolean options can be disabled with --no-OPTION
(Ex: --no-sound turn sound off)


CONFIGURATION :
	All configuration can be done in $HOME/.gngeo/gngeorc
	a sample file is provide : sample_gngeorc
	Every option are also accessible on the command line.
	Per game configuration is also possible in $HOME/.gngeo/game.rc
	
	By default, gngeo search bios and games in
	$prefix/share/gngeo/ (the game must be in a zip file, the bios not)

FRONTEND :
	GGF:      The first frontend available for gngeo. It's written in Java, support
	          game preview, very detailled game description and support many gngeo options.
	          - License: GPL
	          - Homepage: 
	
	XGngeo:   Written in python with GTK2. Support internationalization, game preview,
	          and many configuration possibility, including a key configurator.
	          - License: GPL
		  - Homepage: http://choplair.tuxfamily.org/
	
	gngeogui: A new frontend written in Perl with libgtk-perl. Support game preview, and
	          nearly all the gngeo options. 
	          - License: GPL
	          - Homepage: 







