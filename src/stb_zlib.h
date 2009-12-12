#ifndef STB_ZLIB_H
#define STB_ZLIB_H
#include "stdio.h"

// ZLIB client - used by PNG, available for other purposes

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef   signed short  int16;
typedef unsigned int   uint32;
typedef   signed int    int32;
typedef unsigned int   uint;


// fast-way is faster to check than jpeg huffman, but slow way is slower
#define ZFAST_BITS  9 // accelerate all cases in default tables
#define ZFAST_MASK  ((1 << ZFAST_BITS) - 1)

// zlib-style huffman encoding
// (jpegs packs from left, zlib from right, so can't share code)
typedef struct
{
   uint16 fast[1 << ZFAST_BITS];
   uint16 firstcode[16];
   int maxcode[17];
   uint16 firstsymbol[16];
   uint8  size[288];
   uint16 value[288]; 
} zhuffman;

typedef struct
{
	uint8 *zbuffer, *zbuffer_end;
	FILE *zf;
	int totread,totsize;
	int num_bits;
	uint32 code_buffer;
	
	char *zout;
	char *zout_start;
	char *zout_end;
	int   z_expandable;
	
	uint8 *cbuf;
	uint32 cb_pos;
	int final,left,type,dist;

	zhuffman z_length, z_distance;
} zbuf;

typedef struct ZFILE {
	//char *name;
	int pos;
	zbuf *zb;
	FILE *f;
	int csize,uncsize;
	int cmeth; /* compression method */
	int readed;
}ZFILE;

typedef struct PKZIP {
	FILE *file;
	unsigned int cd_offset; /* Central dir offset */
	unsigned int cd_size;
	unsigned int cde_offset;
	uint16 nb_item;
}PKZIP;


char *stbi_zlib_decode_malloc(const char *buffer, int len, int *outlen);

zbuf *stbi_zlib_create_zbuf(const char *ibuffer,FILE *f, int ilen);
int   stbi_zlib_decode_noheader_stream(zbuf *a,char *obuffer, int olen);

void gn_unzip_fclose(ZFILE *z);
int gn_unzip_fread(ZFILE *z,uint8 *data,int size);
ZFILE *gn_unzip_fopen(PKZIP *zf,char *filename,uint32 file_crc);
PKZIP *gn_open_zip(char *file);
uint8 *gn_unzip_file_malloc(PKZIP *zf,char *filename,uint32 file_crc,int *outlen);
void gn_close_zip(PKZIP *zf);

#endif
