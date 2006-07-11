#ifndef AY8910_H
#define AY8910_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../emu.h"


#define MAX_8910 5
#define ALL_8910_CHANNELS -1

typedef Uint32(*mem_read_handler) (Uint32 offset);
typedef void (*mem_write_handler) (Uint32 offset, Uint32 data);


struct AY8910interface {
    int num;			/* total number of 8910 in the machine */
    int baseclock;
    int mixing_level[MAX_8910];
    mem_read_handler portAread[MAX_8910];
    mem_read_handler portBread[MAX_8910];
    mem_write_handler portAwrite[MAX_8910];
    mem_write_handler portBwrite[MAX_8910];
    void (*handler[MAX_8910]) (int irq);	/* IRQ handler for the YM2203 */
};

void AY8910_reset( /*int chip */ void);

void AY8910_set_clock( /*int chip, */ int _clock);
void AY8910_set_volume( /*int chip, */ int channel, int volume);


void AY8910Write( /*int chip, */ int a, int data);
int AY8910Read( /*int chip */ void);


Uint32 AY8910_read_port_0_r(Uint32 offset);
Uint32 AY8910_read_port_1_r(Uint32 offset);
Uint32 AY8910_read_port_2_r(Uint32 offset);
Uint32 AY8910_read_port_3_r(Uint32 offset);
Uint32 AY8910_read_port_4_r(Uint32 offset);

void AY8910_control_port_0_w(Uint32 offset, Uint32 data);
void AY8910_control_port_1_w(Uint32 offset, Uint32 data);
void AY8910_control_port_2_w(Uint32 offset, Uint32 data);
void AY8910_control_port_3_w(Uint32 offset, Uint32 data);
void AY8910_control_port_4_w(Uint32 offset, Uint32 data);

void AY8910_write_port_0_w(Uint32 offset, Uint32 data);
void AY8910_write_port_1_w(Uint32 offset, Uint32 data);
void AY8910_write_port_2_w(Uint32 offset, Uint32 data);
void AY8910_write_port_3_w(Uint32 offset, Uint32 data);
void AY8910_write_port_4_w(Uint32 offset, Uint32 data);

int AY8910_sh_start( /*const struct MachineSound *msound */ void);
void AY8910Update(int param, Sint16 ** buffer, int length);

#endif
