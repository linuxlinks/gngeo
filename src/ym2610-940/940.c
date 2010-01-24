// The top-level functions for the ARM940
// (c) Copyright 2006-2007, Grazvydas "notaz" Ignotas

#include "940shared.h"
#include "940private.h"
#include "mvs.h"
#include "2610intf.h"
//#include "drz80_interf.h"


volatile _940_data_t *shared_data = (_940_data_t *)   0x1C00000;
volatile _940_ctl_t  *shared_ctl  = (_940_ctl_t *)    0x1C00100;
volatile _940_private_t  *private_data  = (_940_private_t *)    0x1B00000; //0x1CA0000;
volatile unsigned long *gp2x_memregl = (unsigned long*) (0xc0000000-0x02000000);



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


void Main940(void)
{
	int job = 0;
	int l;

	//unsigned int z80_cycle_interlace=0;
	//unsigned int sample=0;
	//unsigned int tot_sample=0;
	//int nb_frame=0;


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

			//memset((Uint8*)shared_ctl->play_buffer,0,SAMPLE_BUFLEN*2);

			if (shared_data->z80_cycle==0) shared_data->z80_cycle=73333;

			private_data->z80_cycle_interlace=shared_data->z80_cycle/256.0;
			private_data->sample=(shared_data->sample_rate/60.0);
			private_data->tot_sample==0;
			private_data->nb_frame=0;

			cpu_z80_init();
			YM2610_sh_start();

			shared_ctl->nmi_pending=0;
			shared_ctl->test=0;
			init_timer(256);
			//shared_ctl->test=z80_cycle_interlace;
			shared_ctl->buf_pos=0;


		
			break;
			
		case JOB940_INVALIDATE_DCACHE:
			drain_wb();
			dcache_clean_flush();
			break;
		case JOB940_RUN_Z80:
			shared_ctl->z80_run=1;

			for (l=0;l<256;l++) {
				//while (shared_ctl->nmi_pending>0) {
				if (shared_ctl->nmi_pending) {
					shared_ctl->nmi_pending=0;
					cpu_z80_nmi();
					cpu_z80_run(300);
				}
				cpu_z80_run(private_data->z80_cycle_interlace);
				my_timer();
			}
			shared_ctl->z80_run=0;

			//shared_ctl->test=gp2x_memregl[0x0A00>>2];
			shared_ctl->updateym=1;
			//shared_ctl->test=nb_frame;
#if 1
			if (private_data->nb_frame>=60) {
				int a=shared_data->sample_rate-private_data->tot_sample;
				//shared_ctl->test=shared_data->sample_rate-tot_sample;
				if (a>0) 
					YM2610Update_stream(a);
				private_data->nb_frame=0;
				private_data->tot_sample=0;
			} else {
				YM2610Update_stream(private_data->sample);
				private_data->tot_sample+=private_data->sample;
			}
/*
			if (shared_ctl->nmi_pending) {
				shared_ctl->nmi_pending=0;
				cpu_z80_nmi(); 
				cpu_z80_run(300); 
			} 
*/
			private_data->nb_frame++;
#else
			YM2610Update_stream(shared_ctl->sample_todo);
#endif
			shared_ctl->updateym=0;


			//shared_ctl->test=shared_ctl->nb_frame;

			break;
		case JOB940_RUN_Z80_2: {
			int z80_cycle=(shared_ctl->sample_todo*4400000)/
				(float)shared_data->sample_rate;
			shared_ctl->test+=(shared_ctl->sample_todo*256*60)/
				(float)shared_data->sample_rate;


			if (shared_ctl->test>256) {
				shared_ctl->z80_run=1;
				for (l=0;l<shared_ctl->test;l++) {
					if (shared_ctl->nmi_pending) {
						shared_ctl->nmi_pending=0;
						cpu_z80_nmi();
						cpu_z80_run(300);
					}
					cpu_z80_run(z80_cycle/shared_ctl->test);
					my_timer();
				}
				shared_ctl->z80_run=0;
				shared_ctl->test=0;
			}
			shared_ctl->updateym=1;

			YM2610Update_stream(shared_ctl->sample_todo);
			shared_ctl->updateym=0;
		}
			break;
		case JOB940_RUN_Z80_NMI:
			if (shared_ctl->nmi_pending) {
				shared_ctl->nmi_pending=0;
				cpu_z80_nmi();
				cpu_z80_run(300);
			}
			break;
		case JOB940_RUN_Z80_300:
			cpu_z80_run(300);
			break;
		}

		shared_ctl->loopc++;
		dcache_clean();
	}
}

