#include <stdio.h>
#include "einput.h"


int main() {
	EInput ei;
	while(!ei.Get("Escape")) {
		const char *name[] = {
			"Up"    ,
			"Down"  ,
			"Left"  ,
			"Right" ,
			"B0"    ,
			"B1"    ,
			0,
		};
		int index = 0;
		while(name[index])
		{
			const char *str =name[index];
			printf("[%02X:%s]", ei.Get(str), str);
			index++;
		}
		printf("\n");
		Sleep(15);
	}
	return 0;
}
