/*
 * gnutil.h
 *
 */

#ifndef GNUTIL_H_
#define GNUTIL_H_

char *get_gngeo_dir(void);
void chomp(char *str);
char *my_fgets(char *s, int size, FILE *stream);
char *file_basename(char *filename);
int check_dir(char *dir_name);

/* LOG generation */
#define GNGEO_LOG(...)
#define DEBUG_LOG printf
//#define GNGEO_LOG printf

#define GN_TRUE 1
#define GN_FALSE 0

/* TODO: redesign */
#define CHECK_ALLOC(a) {if (!a) {printf("Out of Memory\n");exit(1);}}

#endif /* GNUTIL_H_ */
