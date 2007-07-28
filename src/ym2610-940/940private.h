#ifndef _940PRIVATE_H_
#define _940PRIVATE_H_

typedef struct {
	double inc;
	int nb_frame;
	int sample;
	int tot_sample;
	int z80_cycle_interlace;
} _940_private_t;

extern volatile _940_private_t *private_data;

#endif
