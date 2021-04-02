#include <file_compressor.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>



#define MODE_COMPRESS 0
#define MODE_DECOMPRESS 1
#define MODE_INFO 2



int main(int argc,const char** argv){
	const char* e_fp=*argv;
	argc--;
	argv++;
	if (!argc){
_help:
		printf("Usage:\n%s [-h] [-m <mode>] [-o <file>] <input files>\n\nArguments:\n  -h, --help: Prints the Help Message\n  -m <mode>, --mode <mode>: The Mode of the Operation: \n    c(ompress)\n    d(ecompress)\n    i(nfo)\n  -o <file>, --output <file>: The Output Filename (Not valid with 'info' Mode)\n\nNotes:\n * When Operating in the 'info or 'decompress' Mode, There can be a Maximum of One Input File\n * When Operating in the 'decompress' Mode, the Output File Denotes a Directory to Extract Files to\n",e_fp);
		return 1;
	}
	const char* ofp=NULL;
	uint8_t m=MODE_INFO;
	char** ifpl=NULL;
	FILE** ifpfl=NULL;
	uint32_t ifpll=0;
	while (argc){
		const char* e=*argv;
		argc--;
		argv++;
		if (*e=='-'){
			if (*(e+1)=='m'&&!*(e+2)){
				if (!argc){
					printf("Unterminated Switch!\n\n");
					goto _help;
				}
				e=*argv;
				argc--;
				argv++;
				if (*(e+1)){
					printf("Unknown Mode '%s'!\n\n",e);
					goto _help;
				}
				switch (*e){
					case 'c':
						m=MODE_COMPRESS;
						break;
					case 'd':
						m=MODE_DECOMPRESS;
						break;
					case 'i':
						m=MODE_INFO;
						break;
					default:
						printf("Unknown Mode '%s'!\n\n",e);
						goto _help;
				}
			}
			else if (*(e+1)=='h'&&!*(e+2)){
				goto _help;
			}
			else if (*(e+1)=='o'&&!*(e+2)){
				if (!argc){
					printf("Unterminated Switch!\n\n");
					goto _help;
				}
				ofp=*argv;
				argc--;
				argv++;
			}
			else{
				printf("Unknown Switch '%s'!\n\n",e);
				goto _help;
			}
		}
		else{
			ifpll++;
			ifpl=realloc(ifpl,ifpll*sizeof(char*));
			*(ifpl+ifpll-1)=(char*)e;
			ifpfl=realloc(ifpfl,ifpll*sizeof(FILE*));
			errno_t fe=fopen_s(ifpfl+ifpll-1,e,"rb");
			if (fe){
				printf("File '%s' Not Found!\n\n",e);
				return 1;
			}
		}
	}
	if (!ifpll){
		printf("No Input Files Supplied!\n\n");
		goto _help;
	}
	if (m!=MODE_COMPRESS&&ifpll>1){
		printf("The 'info' Mode Supports only One File!\n\n");
		goto _help;
	}
	if (m!=MODE_INFO&&!ofp){
		printf("No Output File Supplied!\n\n");
		goto _help;
	}
	switch (m){
		case MODE_COMPRESS:
			FILE* of;
			errno_t fe=fopen_s(&of,ofp,"wb");
			if (fe){
				printf("Unable to Open File '%s'!\n\n",ofp);
				return 1;
			}
			file_compressor_compress(of,ifpl,ifpfl,ifpll);
			fclose(of);
			break;
		case MODE_DECOMPRESS:
			file_compressor_decompress((char*)ofp,*ifpfl);
			break;
		case MODE_INFO:
			file_compressor_info(*ifpfl);
			break;
	}
	while (ifpll){
		ifpll--;
		fclose(*(ifpfl+ifpll));
	}
	return 0;
}
