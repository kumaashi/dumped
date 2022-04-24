#include <stdint.h>
#include <vector>
#include <thread>
#include <vector>
#include <windows.h>
#include <dwmapi.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <stdio.h>
#include <windows.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dwmapi.lib")

int main()
{
	enum {
		Width = 80,
		Height = 25,
	};
	std::vector<uint8_t> screen(Width * Height);
	auto hcon = CreateConsoleScreenBuffer(
			GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	if (!hcon)
		return -1;
	SetConsoleActiveScreenBuffer(hcon);

	static int count = 0;
	while ( (GetAsyncKeyState(VK_ESCAPE) & 0x8000) == 0) {
		count++;
		auto tex = [](auto x, auto y) {
			x >>= 2;
			y >>= 2;
			return (x ^ y) % 5;
		};

		{
			auto *p = screen.data();
			for (int y = 0 ; y < Height; y++) {
				for (int x = 0 ; x < Width; x++) {
					int fov = 6;
					int ix = x - (Width / 2);
					int iy = y - (Height / 2);
					if (iy == 0)  *p = ' ';
					else *p = 0x2A + tex(count + (ix * fov) / abs(iy), abs(iy));
					p++;
				}
			}
		}

		DWORD dwBytesWritten = 0;
		auto data = screen.data();
		for (int i = 0 ; i < Height; i++) {
			WriteConsoleOutputCharacter(hcon, (LPCSTR)data, Width, {0, (SHORT)i }, &dwBytesWritten);
			data += Width;
		}
		DwmFlush();
	}
	CloseHandle(hcon);
}

