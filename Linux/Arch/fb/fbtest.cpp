#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

#define WIDTH (1280)
#define HEIGHT (800)

int main() {
	int fd = open("/dev/fb0", O_RDWR);
	if(fd < 0) {
		printf("ERROR failed open FB\n");
		exit(1);
	}

	static uint32_t buffer[WIDTH];
	int count = 0;
	while(1) {
		count++;
		uint32_t data = 0x12845667 + count;//rand();
		ioctl(fd, FBIO_WAITFORVSYNC, 0);
		lseek(fd, 0, SEEK_SET);
		for(int y = 0; y < HEIGHT; y+=4) {
			for(int x = 0; x < WIDTH; x++) {
				auto offset_x = x + count;
				auto offset_y = y - count;
				buffer[x] = data ^ (offset_x ^ offset_y);
			}
			write(fd, buffer, sizeof(buffer));
			lseek(fd, WIDTH * 4 * y, SEEK_SET);
		}
		//sleep(1);
	}
	close(fd);

	return 0;
}

