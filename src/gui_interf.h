#ifndef _GUI_INTERF_H_
#define _GUI_INTERF_H_

#include "SDL.h"

/* Progress bar use for loading (and converting/decrypting) feedback */
void loading_pbar_set_label(char *label);
void loading_pbar_update(Uint32 val,Uint32 size);


void init_gngeo_gui(void);
SDL_bool main_gngeo_gui(void);

#endif
