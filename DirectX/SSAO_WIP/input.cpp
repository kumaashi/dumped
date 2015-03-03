#include "if.h"

BOOL CALLBACK Input::DIEnumGamePadProc(LPDIDEVICEINSTANCE pDIDInst, LPVOID pRef)
{
	printf("%s : ", pDIDInst->tszProductName);
	Input *input = (Input *)pRef;
	if (input->state.di->CreateDevice(pDIDInst->guidInstance, &input->state.dipad, NULL) != DI_OK) {
		printf("%s FAILED CreateDevice\n", __FUNCTION__);
		return DIENUM_CONTINUE;
	}
	printf("CONNECT \n");
	return DIENUM_CONTINUE;
}
