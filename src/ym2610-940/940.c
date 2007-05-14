// The top-level functions for the ARM940
// (c) Copyright 2006-2007, Grazvydas "notaz" Ignotas

#include "940shared.h"
#include "mvs.h"
#include "2610intf.h"


volatile _940_data_t *shared_data = (_940_data_t *)   0x1C00000;
volatile _940_ctl_t  *shared_ctl  = (_940_ctl_t *)    0x1C80000;
volatile unsigned long *gp2x_memregl = (unsigned long*) (0xc0000000-0x02000000);
int nb_interlace=256;


// from init.s
int  wait_get_job(int oldjob);
void spend_cycles(int c);
void dcache_clean(void);
void dcache_clean_flush(void);
void drain_wb(void);
// this should help to resolve race confition where shared var
// is changed by other core just before we update it
void set_if_not_changed(int *val, int oldval, int newval);

//	asm volatile ("mov r0, #0" ::: "r0");
//	asm volatile ("mcr p15, 0, r0, c7, c6,  0" ::: "r0"); /* flush dcache */
//	asm volatile ("mcr p15, 0, r0, c7, c10, 4" ::: "r0"); /* drain write buffer */

unsigned int z80_cycle_interlace;
unsigned int sample;
unsigned int tot_sample;
int nb_frame;

void Main940(void)
{
	int job = 0;
	int l;

	for (;;)
	{
		job = wait_get_job(job);
		shared_ctl->lastjob = job;

		switch (job)
		{
		case JOB940_INITALL:
			//shared_ctl->test=(unsigned long) shared_data->sm1;
			//shared_ctl->test=((unsigned long*)shared_data->sm1)[0];
			//shared_data->sm1[0]=0;
			//z80_cycle_interlace=shared_data->z80_cycle/(float)nb_interlace;

			memset((Uint8*)shared_ctl->play_buffer,0,SAMPLE_BUFLEN*2);
			z80_cycle_interlace=shared_data->z80_cycle/(float)nb_interlace;
			sample=(shared_data->sample_rate/60.0);

			cpu_z80_init();
			YM2610_sh_start();

			shared_ctl->test=0;
			init_timer();
			//shared_ctl->test=z80_cycle_interlace;
			shared_ctl->buf_pos=0;
			nb_frame=0;
			break;
			
		case JOB940_INVALIDATE_DCACHE:
			drain_wb();
			dcache_clean_flush();
			break;
		case JOB940_RUN_Z80:
			//while (shared_ctl->z80_run!=0) {
			
			shared_ctl->z80_run=1;
				//shared_ctl->test++;
				for (l=0;l<nb_interlace;l++) {
					cpu_z80_run(z80_cycle_interlace);
					my_timer();
					if (shared_ctl->nmi_pending) {
						shared_ctl->nmi_pending=0;
						cpu_z80_nmi();
					}
				
				
				}


				//shared_ctl->test=gp2x_memregl[0x0A00>>2];
				shared_ctl->updateym=1;
				if (nb_frame>=60) {
					int a=shared_data->sample_rate-tot_sample;
					//shared_ctl->test=shared_data->sample_rate-tot_sample;
					if (a>0) 
						YM2610Update_stream(a);
					nb_frame=0;
					tot_sample=0;

				} else {
					YM2610Update_stream(sample);
					tot_sample+=sample;
				}
				shared_ctl->updateym=0;
				nb_frame++;

			//shared_ctl->test=gp2x_memregl[0x0A00>>2]-shared_ctl->test;			
				shared_ctl->z80_run=0;
			//}
			break;
		case JOB940_RUN_Z80_BIS:
			while (shared_ctl->z80_run) {
				while (shared_ctl->z80_render_a_frame==0) {
					if (shared_ctl->updateym) {
						YM2610Update_stream(shared_ctl->updateym);
						shared_ctl->updateym=0;
					}
				}
				for (l=0;l<nb_interlace;l++) {
					cpu_z80_run(z80_cycle_interlace);
					my_timer();
					if (shared_ctl->nmi_pending) {
						shared_ctl->nmi_pending=0;
						cpu_z80_nmi();
					}
					if (shared_ctl->updateym) {
						YM2610Update_stream(shared_ctl->updateym);
						shared_ctl->updateym=0;
					}
				}
				shared_ctl->z80_render_a_frame=0;
			}
			break;
		}

		shared_ctl->loopc++;
		dcache_clean();
	}
}

