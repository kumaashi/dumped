#include <stdio.h>
#include <math.h>

int main() {
	for(int i = 0 ; i < 0x40; i++) {
		if( (i % 4) == 0)
			printf("\n");
		float a = pow(2.0, (float(i + 0x30) + 1.0) / 12.0);
		printf("0x%08X, ", int(a));
	}
	return 0;
}

