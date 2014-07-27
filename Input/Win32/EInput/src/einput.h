#ifndef _EINPUT_H_
#define _EINPUT_H_

#include <windows.h>
#include <map>
#include <string>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "user32.lib")

struct EInput
{
	std::map<std::string, bool> State;

	enum
	{
		PWM_P   =    0xBFFF,
		PWM_N   =    0x3FFF,
		KB0     =    'Z',
		KB1     =    'X',
	};

	void Update()
	{
		State.clear();
		State["Escape"] = GetKeyInput(VK_ESCAPE) ? true : false;
		State["Up"]     = GetKeyInput(VK_UP)     ? true : false;
		State["Down"]   = GetKeyInput(VK_DOWN)   ? true : false;
		State["Left"]   = GetKeyInput(VK_LEFT)   ? true : false;
		State["Right"]  = GetKeyInput(VK_RIGHT)  ? true : false;
		State["B0"]     = GetKeyInput(KB0)       ? true : false;
		State["B1"]     = GetKeyInput(KB1)       ? true : false;
		
		JOYINFO JoyInfo;
		if(joyGetPos(JOYSTICKID1, &JoyInfo) == JOYERR_NOERROR)
		{
			State["Up"]     |= (JoyInfo.wYpos    < PWM_N)       ? true : false;
			State["Down"]   |= (JoyInfo.wYpos    > PWM_P)       ? true : false;
			State["Left"]   |= (JoyInfo.wXpos    < PWM_N)       ? true : false;
			State["Right"]  |= (JoyInfo.wXpos    > PWM_P)       ? true : false;
			State["B0"]     |= (JoyInfo.wButtons & JOY_BUTTON1) ? true : false;
			State["B1"]     |= (JoyInfo.wButtons & JOY_BUTTON2) ? true : false;
		}
	}

	int GetKeyInput(int x)
	{
		return (GetAsyncKeyState(x) & 0x8000);
	}
	
	bool Get(const char *name, int isupdate = 1)
	{
		if(isupdate) Update();
		return State[name];
	}
};

#endif _EINPUT_H_

