/*
 *	Header file for the PD4990A Serial I/O calendar & clock.
 */
#include "memory.h"
struct pd4990a_s
{
  int seconds;
  int minutes;
  int hours;
  int days;
  int month;
  int year;
  int weekday;
};

void pd4990a_init(void);
//void pd4990a_init_save_state(void);
void pd4990a_addretrace(void);
int read_4990_testbit(void);
int read_4990_databit(void);
void write_4990_control_w(Uint32 address, Uint32 data);
void pd4990a_increment_day(void);
void pd4990a_increment_month(void);
