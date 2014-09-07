//-----------------------------------------------------------------
//
// eforth.cpp
//
// lute. 2014
//
//-----------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <signal.h>

//-----------------------------------------------------------------
// stack
//-----------------------------------------------------------------
#define WORDS_MAX   512
int words[WORDS_MAX] = {0};
int word_index = 0;

#define DI_MAX      512
int di_stack[DI_MAX] = {0};

#define CS_MAX      512
int cs_stack[DI_MAX] = {0};
int cs_index = 0;

#define SP_MAX      512
typedef int sp_type;
sp_type sp_stack[SP_MAX] = {0};
int sp_index = 0;

#define PRIM_MAX    512
typedef void (* primfn)(void);
int    prim[PRIM_MAX]     = {0};
primfn primaddr[PRIM_MAX] = {0};

#define CHUNK_MAX   512
int chunkbuf[CHUNK_MAX]   = {0};
char *chunk = (char *)chunkbuf;


//-----------------------------------------------------------------
// debug
//-----------------------------------------------------------------
void savestack(const char *name) {
	FILE *fp = fopen(name, "wb");
	if(fp) {
		fwrite(words,    1, sizeof(words),    fp);
		fwrite(di_stack, 1, sizeof(di_stack), fp);
		fwrite(prim,     1, sizeof(prim),     fp);
		fwrite(primaddr, 1, sizeof(primaddr), fp);
		fwrite(chunkbuf, 1, sizeof(chunkbuf), fp);
		fwrite(cs_stack, 1, sizeof(cs_stack), fp);
		fwrite(sp_stack, 1, sizeof(sp_stack), fp);
		fclose(fp);
	}
}

void debugmarkstack() {
	unsigned long data = 0xFFFFFFF0;
	words[CS_MAX      - 1] = data++;
	di_stack[DI_MAX   - 1] = data++;
	cs_stack[CS_MAX   - 1] = data++;
	sp_stack[SP_MAX   - 1] = data++;
	prim[PRIM_MAX     - 1] = data++;
	primaddr[PRIM_MAX - 1] = (primfn)data++;
	chunkbuf[PRIM_MAX - 1] = data++;
}

void core_dump(int sig) {
	printf("! SEGV %d !\n", sig);
	savestack("core.bin");
	exit(1);
}

//-----------------------------------------------------------------
// fp
//-----------------------------------------------------------------
void  fst(int idx, float a) { *(float *)&sp_stack[idx] = a;    }
float fld(int idx)          { return *(float *)&sp_stack[idx]; }

//-----------------------------------------------------------------
// chunk
//-----------------------------------------------------------------
int getidx(char *str) {
	char *s = chunk + 1;
	while(*s) {
		if(!strcmp(s, str)) return s - chunk;
		s += strlen(s) + 1;
	}
	strcpy(s, str);
	return s - chunk;
}

//-----------------------------------------------------------------
// prim (int1(idx), primaddr)
//-----------------------------------------------------------------
int applyprim(int idx) {
	for(int i = 0 ; prim[i] ; i++) {
		if(prim[i] == idx) {
			primaddr[i]();
			return 1;
		}
	}
	return 0;
}

int entryprim(char *s, void (* fn)(void)) {
	int i = 0;
	int idx = getidx(s);
	for(; prim[i] ; i++) {
		if(prim[i] == idx) break;
	}
	prim[i] = idx;
	primaddr[i] = fn;
	return idx;
}


//-----------------------------------------------------------------
// primitive
//-----------------------------------------------------------------
int idx_branch   = 0;
int idx_jmp      = 0;

void prim_add()      { fst(sp_index - 1, fld(sp_index - 1) + fld(sp_index)); sp_index--; }
void prim_sub()      { fst(sp_index - 1, fld(sp_index - 1) - fld(sp_index)); sp_index--; }
void prim_mul()      { fst(sp_index - 1, fld(sp_index - 1) * fld(sp_index)); sp_index--; }
void prim_div()      { fst(sp_index - 1, fld(sp_index - 1) / fld(sp_index)); sp_index--; }
void prim_sin()      { float *w = (float *)&sp_stack[sp_index]; fst(sp_index, sin(*w));  }
void prim_cos()      { float *w = (float *)&sp_stack[sp_index]; fst(sp_index, cos(*w));  }
void prim_abs()      { float *w = (float *)&sp_stack[sp_index]; fst(sp_index, abs(*w));  }
void prim_less()     { float *w = (float *)&sp_stack[sp_index]; sp_stack[sp_index - 1] = *(w - 1) <  *(w); sp_index--; }
void prim_greater()  { float *w = (float *)&sp_stack[sp_index]; sp_stack[sp_index - 1] = *(w - 1) >  *(w); sp_index--; }
void prim_eless()    { float *w = (float *)&sp_stack[sp_index]; sp_stack[sp_index - 1] = *(w - 1) <= *(w); sp_index--; }
void prim_egreater() { float *w = (float *)&sp_stack[sp_index]; sp_stack[sp_index - 1] = *(w - 1) >= *(w); sp_index--; }
void prim_dup()      { sp_stack[sp_index + 1] = sp_stack[sp_index]; sp_index++; }
void prim_drop()     { sp_index--; };
void prim_swap()     { sp_type t = sp_stack[sp_index]; sp_stack[sp_index] = sp_stack[sp_index]; sp_stack[sp_index - 1] = t; };
void prim_branch()   { if(!sp_stack[sp_index]) word_index = words[word_index];  }
void prim_jmp()      { word_index = words[word_index]; }
void prim_halt()     { word_index = 0; cs_index = 0; }
void prim_ret()      { word_index = 0; }
void prim_cr()       { printf("\n"); }
void prim_sp()       { printf(" "); }
void prim_prn()      { printf("%f", fld(sp_index)); }
void prim_dump()     {
	float *w = (float *)sp_stack;
	printf("[");
	for(int i = 0 ; i < sp_index; i++) {
		printf("%3.4f, ", *w++);
	}
	printf("]\n");
}

void initprim() {
	entryprim("+",        prim_add);
	entryprim("-",        prim_sub);
	entryprim("*",        prim_mul);
	entryprim("/",        prim_div);
	entryprim("<",        prim_less);
	entryprim(">",        prim_greater);
	entryprim("<=",       prim_eless);
	entryprim(">=",       prim_egreater);

	entryprim("DUP",      prim_dup);
	entryprim("DROP",     prim_drop);
	entryprim("SWAP",     prim_swap);
	entryprim("HALT",     prim_halt);
	entryprim("RET",      prim_ret);
	entryprim("SIN",      prim_sin);
	entryprim("COS",      prim_cos);
	entryprim("ABS",      prim_abs);
	
	entryprim(".",        prim_prn);
	entryprim("CR",       prim_cr);
	entryprim("SP",       prim_sp);
	entryprim("DUMP",     prim_dump);

	//for mark
	idx_branch   = entryprim("BRANCH",   prim_branch);
	idx_jmp      = entryprim("JMP",      prim_jmp);
}


//-----------------------------------------------------------------
// IP
//-----------------------------------------------------------------
// (int1(idx), jmpidx)
void ipsubentry(int idx, int sub_index) {
	int i = 0;
	for(; di_stack[i]; i += 2) {
		if(di_stack[i] == idx) break;
	}
	di_stack[i + 0] = idx;
	di_stack[i + 1] = sub_index;
}

int ipsub(int idx) {
	for(int i = 0 ; di_stack[i]; i += 2) {
		if(di_stack[i] == idx)
			return di_stack[i + 1];
	}
	return 0;
}

void ipvmdo(int start) {
	word_index = start;
	while(1) {
		int idx = words[word_index++];
		if(idx == 0) {
			if(cs_index <= 0) break;
			word_index = cs_stack[cs_index--];
			continue;
		}
		
		int sub = ipsub(idx);
		if(sub) {
			cs_index++;
			cs_stack[cs_index] = word_index;
			word_index = sub;
			continue;
		}

		if(applyprim(idx)) continue;

		sp_index++;
		fst(sp_index, atof(&chunk[idx]));
	}
}


//-----------------------------------------------------------------
// readwords
//-----------------------------------------------------------------
void readwords(FILE *fp, int start_word_index) {
	char buf[256];
	char *s = buf;
	int c;
	int i = start_word_index;
	if(i <= 0) {
		printf("%s : Error. start word index must to be greater 0.\n");
		return ;
	}
	
	//0:no subroutine, 1:get subroutinename, 2:inner subroutine
	int substate = 0;
	
	//IF back porch
	int bp_stack[256]  = {0};
	int bp_index = 0;
	
	//ELSE back porch 
	int bpe_stack[256] = {0};
	int bpe_index = 0;

	while( (c = fgetc(fp)) > 0) {
		c = toupper(c);
		
		//comment
		if(c == '(') {
			while( (c = fgetc(fp)) != ')');
			if(c < 0) break;
			continue;
		}
		if(c == '\\') {
			while( (c = fgetc(fp)) != '\n');
			if(c < 0) break;
			continue;
		}
		
		//symbol
		if(!isspace(c)) {
			*s++ = c;
			continue;
		}
		
		//parse
		*s = 0;
		if(s == buf) continue;
		s = buf;

		//if else then
		if(!strcmp("IF", s))   {
			words[i++] = idx_branch;
			bp_index++;
			bp_stack[bp_index] = i++;
			continue;
		}

		if(!strcmp("ELSE", s)) {
			words[i++] = idx_jmp;
			bpe_index++;
			bpe_stack[bpe_index] = i++;
			if(bp_index  > 0) {
				words[ bp_stack[bp_index] ] = i;
				bp_index--;
			}
			continue;
		}

		if(!strcmp("THEN", s)) {
			if(bp_index  > 0) {
				words[ bp_stack[bp_index] ] = i;
				bp_index--;
			}
			if(bpe_index > 0) {
				words[ bpe_stack[bpe_index] ] = i;
				bpe_index--;
			}
			continue;
		}

		//subroutine
		int idx = getidx(s);
		if(*s == ';')      { substate = 0; words[i++] = 0;     continue; }
		if(*s == ':')      { substate = 1; continue; }
		if(substate == 1)  { substate = 2; ipsubentry(idx, i); continue; }
		
		//put symbol index.
		words[i++] = idx;

	}
}



//-----------------------------------------------------------------
// main
//-----------------------------------------------------------------
int main() {
	signal(SIGSEGV, core_dump);
	debugmarkstack();
	initprim();
	readwords(stdin, 1);
	ipvmdo(ipsub(getidx("MAIN")));
	savestack("data.bin");
}
