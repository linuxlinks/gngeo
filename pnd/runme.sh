#!/bin/sh
# gngeo pnd launching script
# Thanks SteveM!

mkdir -p conf roms save
if [ ! -f ./conf/gngeorc ] ; then
    echo "effect none" >> ./conf/gngeorc
    #echo "hwsurface false" >> ./conf/gngeorc
    echo "fullscreen true" >> ./conf/gngeorc
    echo "autoframeskip false"
    echo "vsync true"
	zenity --question --text="Where gngeo will search roms?\nThe default location is\n/pandora/appdata/gngeo_pepone/roms" --ok-label="Use defaults dir" --cancel-label="Use a custom dir"
	if [ $? -eq 0 ] ; then # Use Default Dir
		echo "rompath ./roms" >> ./conf/gngeorc
	else
		rompath=`zenity --file-selection --directory`
		echo "rompath $rompath" >> ./conf/gngeorc
	fi
fi

export SDL_VIDEODRIVER=omapdss
#export SDL_OMAP_LAYER_SIZE=640x480
export SDL_OMAP_VSYNC=1
sudo -n /usr/pandora/scripts/op_lcdrate.sh 60
LD_PRELOAD=./libSDL-1.2.so.0 ./gngeo
