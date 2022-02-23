#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

uint32_t mrand() {
	static uint32_t a = 1;
	static uint32_t b = 2345678;
	static uint32_t c = 9012345;
	a += b;
	b += c;
	c += a;
	return (a >> 16);
}

float fmrand() {
	return (float)mrand() / float(0xFFFF);
}

int main() {
	for(int i =0  ; i < 10; i++)
		printf("%f\n", fmrand());
	return 0;
}
