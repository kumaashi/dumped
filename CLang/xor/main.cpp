#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

int main() {
	int index =0 ;
	while(true) {
		index++;
		system("cls");
		for(int j = 0 ; j < 32; j++) {
			for(int i = 0 ; i < 32; i++) {
				auto x = ((i + index) * 4);
				auto y = (j * 4);
				char c = (x ^ y) & 0x7F;
				c = isgraph(c) ? c : '.';
				printf("%c ", c);
			}
			printf("\n");
		}
	}
	return 0;
}

