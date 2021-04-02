#ifndef __FILE_COMPRESSOR_H__
#define __FILE_COMPRESSOR_H__ 1
#include <stdint.h>
#include <stdio.h>



void file_compressor_compress(FILE* o,char** fpl,FILE** fpfl,uint32_t fpll);



void file_compressor_info(FILE* f);



void file_compressor_decompress(char* o,FILE* f);



#endif
