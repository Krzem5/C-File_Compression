#include <file_compressor.h>
#include <intrin.h>
#include <windows.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#pragma intrinsic(__stosq)
#pragma intrinsic(_BitScanForward64)



#define MAX_PATH_LENGTH 512



typedef struct __TREE_ELEM{
	uint8_t l;
	uint64_t m;
} tree_elem_t;



typedef struct __QUEUE_ELEM{
	uint64_t f;
	uint64_t bl[4];
} queue_elem_t;



typedef struct __TREE_DECODE_ELEM{
	char c;
	uint64_t m;
} tree_decode_elem_t;



typedef struct __TREE_ELEM_ARRAY{
	uint16_t l;
	tree_decode_elem_t* dt;
} tree_elem_array_t;



void file_compressor_compress(FILE* o,char** fpl,FILE** fpfl,uint32_t fpll){
	if (!fpll){
		return;
	}
	uint64_t fl[256];
	__stosq(fl,0,256);
	uint16_t ql=0;
	uint8_t si=0;
	uint64_t* fll=malloc(fpll*sizeof(uint64_t));
	for (uint32_t i=0;i<fpll;i++){
		FILE* f=*(fpfl+i);
		fseek(f,0,SEEK_END);
		*(fll+i)=ftell(f);
		rewind(f);
		while (1){
			int e=fgetc(f);
			if (e==EOF){
				break;
			}
			if (!fl[e]){
				si=(uint8_t)e;
				ql++;
			}
			fl[e]++;
		}
		rewind(f);
	}
	if (!ql){
		free(fll);
		return;
	}
	tree_elem_t t[256];
	__stosq((uint64_t*)t,0,256*sizeof(tree_elem_t)/sizeof(uint64_t));
	uint8_t mx=1;
	if (ql==1){
		(t+si)->l=1;
	}
	else{
		queue_elem_t* q=malloc(ql*sizeof(queue_elem_t));
		uint8_t i=0;
		for (uint16_t j=0;j<256;j++){
			if (fl[j]){
				queue_elem_t e={
					fl[j],
					{
						0,
						0,
						0,
						0
					}
				};
				e.bl[j>>6]=1ull<<(j&63);
				uint8_t k=i;
				while (k){
					uint8_t pi=(k-1)>>1;
					queue_elem_t pe=*(q+pi);
					if ((e.f!=pe.f?(e.f<pe.f):(e.bl[0]<pe.bl[0]||e.bl[1]<pe.bl[1]||e.bl[2]<pe.bl[2]||e.bl[3]<pe.bl[3]))){
						*(q+k)=pe;
						k=pi;
						continue;
					}
					break;
				}
				*(q+k)=e;
				if (i==ql-1){
					break;
				}
				i++;
			}
		}
		while (1){
			queue_elem_t ea=*q;
			ql--;
			queue_elem_t e=*(q+ql);
			uint16_t i=0;
			uint16_t ci=1;
			while (ci<ql){
				queue_elem_t ce=*(q+ci);
				if (ci+1<ql){
					queue_elem_t nce=*(q+ci+1);
					if ((ce.f!=nce.f?(ce.f>=nce.f):(ce.bl[0]>=nce.bl[0]||ce.bl[1]>=nce.bl[1]||ce.bl[2]>=nce.bl[2]||ce.bl[3]>=nce.bl[3]))){
						ci++;
						ce=*(q+ci);
					}
				}
				*(q+i)=ce;
				i=ci;
				ci=(i<<1)+1;
			}
			while (i){
				uint8_t pi=(i-1)>>1;
				queue_elem_t pe=*(q+pi);
				if ((e.f!=pe.f?(e.f<pe.f):(e.bl[0]<pe.bl[0]||e.bl[1]<pe.bl[1]||e.bl[2]<pe.bl[2]||e.bl[3]<pe.bl[3]))){
					*(q+i)=pe;
					i=pi;
					continue;
				}
				break;
			}
			*(q+i)=e;
			queue_elem_t eb=*q;
			for (uint8_t i=0;i<4;i++){
				uint64_t m=ea.bl[i];
				while (m){
					unsigned long j;
					_BitScanForward64(&j,m);
					m&=~(1ull<<j);
					j+=i<<6;
					t[j].l++;
					if (t[j].l>=sizeof(uint64_t)*8){
						printf("Encoding won't fit in 64-bit integer!\n");
						return;
					}
					if (t[j].l>mx){
						mx=t[j].l;
					}
				}
				m=eb.bl[i];
				while (m){
					unsigned long j;
					_BitScanForward64(&j,m);
					m&=~(1ull<<j);
					j+=i<<6;
					t[j].m|=(1ull<<t[j].l);
					t[j].l++;
					if (t[j].l>=sizeof(uint64_t)*8){
						printf("Encoding won't fit in 64-bit integer!\n");
						return;
					}
					if (t[j].l>mx){
						mx=t[j].l;
					}
				}
			}
			if (ql==1){
				break;
			}
			e.f=ea.f+eb.f;
			e.bl[0]=ea.bl[0]|eb.bl[0];
			e.bl[1]=ea.bl[1]|eb.bl[1];
			e.bl[2]=ea.bl[2]|eb.bl[2];
			e.bl[3]=ea.bl[3]|eb.bl[3];
			i=0;
			ci=1;
			while (ci<ql){
				queue_elem_t ce=*(q+ci);
				if (ci+1<ql){
					queue_elem_t nce=*(q+ci+1);
					if ((ce.f!=nce.f?(ce.f>=nce.f):(ce.bl[0]>=nce.bl[0]||ce.bl[1]>=nce.bl[1]||ce.bl[2]>=nce.bl[2]||ce.bl[3]>=nce.bl[3]))){
						ci++;
						ce=*(q+ci);
					}
				}
				*(q+i)=ce;
				i=ci;
				ci=(i<<1)+1;
			}
			while (i){
				uint8_t pi=(i-1)>>1;
				queue_elem_t pe=*(q+pi);
				if ((e.f!=pe.f?(e.f<pe.f):(e.bl[0]<pe.bl[0]||e.bl[1]<pe.bl[1]||e.bl[2]<pe.bl[2]||e.bl[3]<pe.bl[3]))){
					*(q+i)=pe;
					i=pi;
					continue;
				}
				break;
			}
			*(q+i)=e;
		}
		free(q);
	}
	fputc(mx-1,o);
	uint8_t i=0;
	while (1){
		tree_elem_t e=t[i];
		uint8_t j=(e.l+7)&0xf8;
		fputc(e.l,o);
		while (j){
			j-=8;
			fputc((uint8_t)(e.m>>j),o);
		}
		if (i==255){
			break;
		}
		i++;
	}
	while (fpll){
		fpll--;
		FILE* f=*(fpfl+fpll);
		uint64_t sz=*(fll+fpll);
		fputs(*(fpl+fpll),o);
		fputc(0,o);
		if (sz<=2147483647){
			fputc((uint8_t)(sz>>24),o);
			fputc((sz>>16)&0xff,o);
			fputc((sz>>8)&0xff,o);
			fputc(sz&0xff,o);
		}
		else{
			fputc((uint8_t)(sz>>56)|0x80,o);
			fputc((sz>>48)&0xff,o);
			fputc((sz>>40)&0xff,o);
			fputc((sz>>32)&0xff,o);
			fputc((sz>>24)&0xff,o);
			fputc((sz>>16)&0xff,o);
			fputc((sz>>8)&0xff,o);
			fputc(sz&0xff,o);
		}
		uint32_t bf=0;
		uint8_t bfl=0;
		for (uint64_t i=0;i<sz;i++){
			tree_elem_t e=t[fgetc(f)];
			uint8_t j=e.l;
			while (j){
				uint8_t k=(j>16?16:j);
				j-=k;
				bf=(bf<<k)|((e.m>>j)&0xffff);
				bfl+=k;
				while (bfl>=8){
					bfl-=8;
					fputc((uint8_t)(bf>>bfl),o);
				}
			}
		}
		if (bfl){
			fputc((uint8_t)(bf<<(8-bfl)),o);
		}
	}
	free(fll);
}



void file_compressor_info(FILE* f){
	fseek(f,0,SEEK_END);
	uint64_t f_sz=ftell(f);
	rewind(f);
	uint8_t tl=fgetc(f)+1;
	printf("Encoding:\n");
	tree_elem_array_t* t=malloc(tl*sizeof(tree_elem_array_t)+(tl+1)*sizeof(uint8_t));
	__stosq((uint64_t*)t,0,tl*sizeof(tree_elem_array_t)/sizeof(uint64_t));
	uint8_t i=0;
	while (1){
		uint8_t l=fgetc(f);
		if (l){
			uint8_t j=l-1;
			l=(l+7)/8;
			uint64_t m=0;
			while (l){
				l--;
				m=(m<<8)|fgetc(f);
			}
			printf("  [%.2x %c%c%c]: (%u) ",i,(i>31&&i<127?'(':' '),(i>31&&i<127?i:' '),(i>31&&i<127?')':' '),j+1);
			uint8_t k=j+1;
			while (k){
				k--;
				putchar((m&(1ull<<k)?'1':'0'));
			}
			putchar('\n');
			(t+j)->l++;
			(t+j)->dt=realloc((t+j)->dt,(t+j)->l*sizeof(tree_decode_elem_t));
			tree_decode_elem_t* e=(t+j)->dt+(t+j)->l-1;
			e->c=i;
			e->m=m;
		}
		if (i==255){
			break;
		}
		i++;
	}
	uint8_t* ti=(uint8_t*)(void*)((uint64_t)(void*)t+tl*sizeof(tree_elem_array_t));
	for (uint8_t i=0;i<tl+1;i++){
		uint8_t j=i+1;
		while (j<tl+1&&(!j||!(t+j-1)->l)){
			j++;
		}
		*(ti+i)=j-i;
	}
	char fp_bf[MAX_PATH_LENGTH];
	uint64_t t_sz=0;
	printf("Files:\n");
	while (1){
		uint64_t i=0;
		while (1){
			int c=fgetc(f);
			if (c==EOF){
				goto _end;
			}
			fp_bf[i]=(char)c;
			i++;
			if (!c){
				break;
			}
		}
		uint64_t sz=fgetc(f);
		if (sz&0x80){
			sz&=~0x80;
			for (uint8_t i=0;i<7;i++){
				sz=(sz<<8)|fgetc(f);
			}
		}
		else{
			for (uint8_t i=0;i<3;i++){
				sz=(sz<<8)|fgetc(f);
			}
		}
		t_sz+=sz;
		printf("  '%s': %"PRIu64"b\n",fp_bf,sz);
		uint32_t bf=0;
		uint8_t bfl=0;
		i=0;
		uint8_t j=0;
		uint16_t e=0;
		while (1){
			if (!j){
				e=fgetc(f);
				j=8;
			}
			uint8_t k=*(ti+bfl);
			if (k>j){
				k=j;
			}
			j-=k;
			bf=(bf<<k)|((e>>j)&((1<<k)-1));
			bfl+=k;
			tree_elem_array_t* a=t+bfl-1;
			for (uint16_t k=0;k<a->l;k++){
				tree_decode_elem_t de=*(a->dt+k);
				if (de.m==bf){
					i++;
					bf=0;
					bfl=0;
					break;
				}
			}
			if (i==sz){
				break;
			}
		}
	}
_end:
	printf("Compression Ratio: %"PRIu64" -> %"PRIu64" (%+.2f%%)\n",t_sz,f_sz,((float)f_sz-t_sz)/t_sz*100);
	while (tl){
		tl--;
		if ((t+tl)->l){
			free((t+tl)->dt);
		}
	}
	free(t);
}



void file_compressor_decompress(char* o,FILE* f){
	char fp_bf[MAX_PATH_LENGTH];
	uint64_t fp_bfi=0;
	while (*o){
		fp_bf[fp_bfi]=*o;
		fp_bfi++;
		o++;
	}
	if (fp_bf[fp_bfi-1]!='/'&&fp_bf[fp_bfi-1]!='\\'){
		fp_bf[fp_bfi]='/';
		fp_bfi++;
	}
	uint8_t tl=fgetc(f)+1;
	tree_elem_array_t* t=malloc(tl*sizeof(tree_elem_array_t)+(tl+1)*sizeof(uint8_t));
	__stosq((uint64_t*)t,0,tl*sizeof(tree_elem_array_t)/sizeof(uint64_t));
	uint8_t i=0;
	while (1){
		uint8_t l=fgetc(f);
		if (l){
			uint8_t j=l-1;
			l=(l+7)/8;
			uint64_t m=0;
			while (l){
				l--;
				m=(m<<8)|fgetc(f);
			}
			(t+j)->l++;
			(t+j)->dt=realloc((t+j)->dt,(t+j)->l*sizeof(tree_decode_elem_t));
			tree_decode_elem_t* e=(t+j)->dt+(t+j)->l-1;
			e->c=i;
			e->m=m;
		}
		if (i==255){
			break;
		}
		i++;
	}
	uint8_t* ti=(uint8_t*)(void*)((uint64_t)(void*)t+tl*sizeof(tree_elem_array_t));
	for (uint8_t i=0;i<tl+1;i++){
		uint8_t j=i+1;
		while (j<tl+1&&(!j||!(t+j-1)->l)){
			j++;
		}
		*(ti+i)=j-i;
	}
	while (1){
		uint64_t i=fp_bfi;
		while (1){
			int c=fgetc(f);
			if (c==EOF){
				goto _end;
			}
			if (c=='/'||c=='\\'){
				fp_bf[i]=0;
				CreateDirectoryA(fp_bf,NULL);
			}
			fp_bf[i]=(char)c;
			i++;
			if (!c){
				break;
			}
		}
		FILE* of=NULL;
#ifdef _MSC_VER
		if (fopen_s(&of,fp_bf,"wb")){
#else
		if (!(of=fopen(fp_bf,"wb"))){
#endif
			of=NULL;
		}
		uint64_t sz=fgetc(f);
		if (sz&0x80){
			sz&=~0x80;
			for (uint8_t i=0;i<7;i++){
				sz=(sz<<8)|fgetc(f);
			}
		}
		else{
			for (uint8_t i=0;i<3;i++){
				sz=(sz<<8)|fgetc(f);
			}
		}
		uint32_t bf=0;
		uint8_t bfl=0;
		i=0;
		uint8_t j=0;
		uint16_t e=0;
		while (1){
			if (!j){
				e=fgetc(f);
				j=8;
			}
			uint8_t k=*(ti+bfl);
			if (k>j){
				k=j;
			}
			j-=k;
			bf=(bf<<k)|((e>>j)&((1<<k)-1));
			bfl+=k;
			tree_elem_array_t* a=t+bfl-1;
			for (uint16_t k=0;k<a->l;k++){
				tree_decode_elem_t de=*(a->dt+k);
				if (de.m==bf){
					if (of){
						fputc(de.c,of);
					}
					i++;
					bf=0;
					bfl=0;
					break;
				}
			}
			if (i==sz){
				break;
			}
		}
		if (of){
			fclose(of);
		}
	}
_end:
	while (tl){
		tl--;
		if ((t+tl)->l){
			free((t+tl)->dt);
		}
	}
	free(t);
}
