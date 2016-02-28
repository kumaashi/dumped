#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <windows.h>

#define MEM_CHUNK     1024
#define MEM_MAX       8192
#define MEM_IP        (MEM_CHUNK * 0)
#define MEM_RSP       (MEM_CHUNK * 1)
#define MEM_PSP       (MEM_CHUNK * 2)
int32_t ip     = MEM_IP   ;
int32_t rsp    = MEM_RSP  ;
int32_t psp    = MEM_PSP  ;
int32_t w      = 0;
int32_t ishalt = 0;
int32_t mem[MEM_MAX];

enum {
	OP_RET    = 0,
	OP_CALL   ,
	OP_JMP    ,
	OP_JZ     ,
	OP_EQ     ,  //[==]
	OP_GE     ,  //[<=]
	OP_LE     ,  //[>=]
	OP_GNE    , //[<=]
	OP_LNE    , //[>=]
	OP_PUSH   ,
	OP_POP    ,
	OP_DUP    ,
	OP_OVER   ,
	OP_DROP   ,
	OP_ADD    ,
	OP_SUB    ,
	OP_MUL    ,
	OP_DIV    ,
	OP_NEXT   ,
	OP_UNEXT  ,
	OP_NOP    = 0xF0,
	OP_DOT    ,
	OP_HALT   ,
	OP_USER   = 0x100,
	OP_MAX    = 0x1000,
};

void (*functable[OP_MAX])();
void entryprim(int idx, void (*fn)()) { functable[idx] = fn; }

void fn_op_ret () { if(MEM_RSP == rsp) ishalt = 1; ip = mem[rsp--];  }
void fn_op_call() { mem[++rsp] = ip + 1; ip = mem[ip];  }
void fn_op_jmp () { ip = mem[ip];  }
void fn_op_jz  () {
	if(mem[psp] == 0) {
		ip = mem[ip];
	} else {
		ip++;
	}
	psp--;
}

void fn_op_eq  () { mem[psp] = mem[psp - 1] == mem[psp];  }
void fn_op_ge  () { mem[psp] = mem[psp - 1] <= mem[psp];  }
void fn_op_le  () { mem[psp] = mem[psp - 1] >= mem[psp];  }
void fn_op_gne () { mem[psp] = mem[psp - 1] <  mem[psp];  }
void fn_op_lne () { mem[psp] = mem[psp - 1] >  mem[psp];  }
void fn_op_push() { mem[++psp] = mem[ip++];  }
void fn_op_drop() { psp--;  }
void fn_op_pop () { psp--;  }
void fn_op_dup () { mem[psp + 1] = mem[psp];     psp++;  }
void fn_op_over() { mem[psp + 1] = mem[psp - 1]; psp++;  }
void fn_op_add () { mem[psp - 1] += mem[psp];    psp--;  }
void fn_op_sub () { mem[psp - 1] -= mem[psp];    psp--;  }
void fn_op_mul () { mem[psp - 1] *= mem[psp];    psp--;  }
void fn_op_div () { mem[psp - 1] /= mem[psp];    psp--;  }
void fn_op_dot () { printf("%d\n", mem[psp]);  }
void fn_op_halt() { ishalt = 1; }
void fn_op_nop () {  }

void initprim() {
	entryprim(OP_RET , fn_op_ret );
	entryprim(OP_CALL, fn_op_call);
	entryprim(OP_JMP , fn_op_jmp );
	entryprim(OP_JZ  , fn_op_jz  );
	entryprim(OP_EQ  , fn_op_eq  );
	entryprim(OP_GE  , fn_op_ge  );
	entryprim(OP_LE  , fn_op_le  );
	entryprim(OP_GNE , fn_op_gne );
	entryprim(OP_LNE , fn_op_lne );
	entryprim(OP_PUSH, fn_op_push);
	entryprim(OP_DROP, fn_op_drop);
	entryprim(OP_POP , fn_op_pop );
	entryprim(OP_DUP , fn_op_dup );
	entryprim(OP_OVER, fn_op_over);
	entryprim(OP_ADD , fn_op_add );
	entryprim(OP_SUB , fn_op_sub );
	entryprim(OP_MUL , fn_op_mul );
	entryprim(OP_DIV , fn_op_div );
	entryprim(OP_DOT , fn_op_dot );
	entryprim(OP_HALT, fn_op_halt);
	entryprim(OP_NOP , fn_op_nop );
}

void dovm(int idx) {
	ip = idx;
	while(ishalt == 0) {
		(*functable[mem[ip++]])();
	}
}


void savevm(const char *name) {
	FILE *fp = fopen(name, "wb");
	if(fp) {
		fwrite(mem, 1, sizeof(mem), fp);
		fclose(fp);
	}
}

void dispop(int idx)
{
	switch(idx) {
		case OP_RET    : printf("OP_RET    \n"); break;
		case OP_CALL   : printf("OP_CALL   \n"); break;
		case OP_JMP    : printf("OP_JMP    \n"); break;
		case OP_JZ     : printf("OP_JZ     \n"); break;
		case OP_EQ     : printf("OP_EQ     \n"); break;
		case OP_GE     : printf("OP_GE     \n"); break;
		case OP_LE     : printf("OP_LE     \n"); break;
		case OP_GNE    : printf("OP_GNE    \n"); break;
		case OP_LNE    : printf("OP_LNE    \n"); break;
		case OP_PUSH   : printf("OP_PUSH   \n"); break;
		case OP_POP    : printf("OP_POP    \n"); break;
		case OP_DUP    : printf("OP_DUP    \n"); break;
		case OP_OVER   : printf("OP_OVER   \n"); break;
		case OP_DROP   : printf("OP_DROP   \n"); break;
		case OP_ADD    : printf("OP_ADD    \n"); break;
		case OP_SUB    : printf("OP_SUB    \n"); break;
		case OP_MUL    : printf("OP_MUL    \n"); break;
		case OP_DIV    : printf("OP_DIV    \n"); break;
		case OP_NEXT   : printf("OP_NEXT   \n"); break;
		case OP_UNEXT  : printf("OP_UNEX   \n"); break;
		case OP_NOP    : printf("OP_NOP    \n"); break;
		case OP_DOT    : printf("OP_DOT    \n"); break;
		case OP_HALT   : printf("OP_HALT   \n"); break;
		case OP_USER   : printf("OP_USER   \n"); break;
		case OP_MAX    : printf("OP_MAX    \n"); break;
		default: printf("UNKNOWN    \n"); break;
	}
}

void dispstate() {
	printf("ip=%08d(0x%08X), rsp=%08d, psp=%08d, fnum=%04d : ", ip, ip*4, rsp, psp, mem[ip]);
	dispop(mem[ip]);
	for(int i = MEM_PSP; i <= psp; i++) {
		printf("%d ", mem[i]);
	}
	printf("\n");
	Sleep(1000);
}


//--------------------------------------------------------
// FORTH COMPILER
//--------------------------------------------------------
#include <vector>
#include <map>
#include <string>

using namespace std;

int issep(char c, char *s) {
	while(*s) if(c == *s++) return 1;
	return 0;
}

int isnum(const char *s) {
	while(*s) {
		if(!isdigit(*s++)) {
			return 0;
		}
	}
	return 1;
}

map<string, int> vdic;
int getdic(const char *a) {
	return vdic[a];
}

int user_idx = OP_USER;
map<string, int> vuserprim;
void entryuserprim(const char *name, void (*fn)()) {
	char temp[256];
	char *s = temp;
	while(*name) {
		*s++ = toupper(*name++);
	}
	*s = 0;
	int idx = user_idx;
	vuserprim[temp] = idx;
	entryprim(idx, fn);
	user_idx++;
}

void readwords(const char *data) {
	int32_t c;
	int32_t idx = 0;
	char buf[256];

	//read
	vector<string> strtable;
	int iscomment = 0;
	while((c = *data++)) {
		c = toupper(c);
		if(issep(c, " \t\r\n()")) {
			if(c == '(') iscomment = 1;
			if(c == ')') iscomment = 0;
			if(!idx) continue;
			buf[idx] = 0;
			strtable.push_back(buf);
			idx = 0;
			continue;
		}
		if(!iscomment) buf[idx++] = c;
	}

	//parse
	int32_t rsp = 0;
	int32_t rspbuf[256];
	int i        = 0;
	int midx     = 0;
	int capture  = 0;
	string recur = "";
	while(i < strtable.size()) {
		string wordstr = strtable[i++];
		
		if(vuserprim.find(wordstr) != vuserprim.end()) {
			mem[midx++] = vuserprim[wordstr];
			continue;
		}

		if(capture) {
			vdic[wordstr] = midx;
			capture = 0;
			recur = wordstr;
			continue;
		}

		if(wordstr == ":")    {
			capture = 1;
			continue;
		}

		if(wordstr == ";")    {
			mem[midx++] = OP_RET;
			recur       = "";
			continue;
		}

		if(vdic.find(wordstr) != vdic.end()) {
			if(recur == wordstr) {
				mem[midx++] = OP_JMP;
				mem[midx++] = vdic[wordstr];
				continue;
			}
			mem[midx++] = OP_CALL;
			mem[midx++] = vdic[wordstr];
			continue;
		}

		if(isnum(wordstr.c_str())) {
			mem[midx++] = OP_PUSH;
			int32_t val = stoi(wordstr);
			mem[midx++] = val;
			continue;
		}

		if(wordstr == "IF") {
			mem[midx++] = OP_JZ;
			rspbuf[++rsp] = midx++;
			continue;
		}

		if(wordstr == "ELSE") {
			int32_t rsptemp = rspbuf[rsp--];
			mem[midx++] = OP_JMP;
			rspbuf[++rsp] = midx++;
			mem[rsptemp] = midx;
			continue;
		}

		if(wordstr == "THEN") {
			int32_t rsptemp = rspbuf[rsp--];
			mem[rsptemp] = midx;
			continue;
		}

		if(wordstr == "+")    { mem[midx++] = OP_ADD; continue; }
		if(wordstr == "-")    { mem[midx++] = OP_SUB; continue; }
		if(wordstr == "*")    { mem[midx++] = OP_MUL; continue; }
		if(wordstr == "/")    { mem[midx++] = OP_DIV; continue; }
		if(wordstr == ".")    { mem[midx++] = OP_DOT; continue; }
		if(wordstr == "<=")   { mem[midx++] = OP_GE;  continue; }
		if(wordstr == ">=")   { mem[midx++] = OP_LE;  continue; }
		if(wordstr == "<")    { mem[midx++] = OP_GNE; continue; }
		if(wordstr == ">")    { mem[midx++] = OP_LNE; continue; }
		if(wordstr == "HALT") { mem[midx++] = OP_HALT; continue; }
		if(wordstr == "POP")  { mem[midx++] = OP_POP;  continue; }
		if(wordstr == "DROP") { mem[midx++] = OP_DROP; continue; }
		if(wordstr == "DUP")  { mem[midx++] = OP_DUP;  continue; }

		//ERROR UNKNOWN ERROR
		printf("ERROR %s\n", wordstr.c_str());
		for(int i = 0; i < MEM_MAX; i++) {
			mem[i] = OP_HALT;
		}
		return;
	}
}

void foo() { printf("CALL foo\n"); }
void bar() { printf("CALL bar\n"); }

int main() {
	initprim();
	entryuserprim("foo", foo);
	entryuserprim("bar", bar);
	vector<char> vdata;
	int c = 0;
	while( (c = fgetc(stdin)) > 0) {
		vdata.push_back(c);
	}
	readwords(&vdata[0]);
	savevm("dump.bin");
	dovm(getdic("MAIN"));
	savevm("dump2.bin");
	return 1;
}

