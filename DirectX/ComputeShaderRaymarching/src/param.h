//-----------//-----------//-----------//-----------//-----------//-----------
//
//
//  param.h
//
//
//-----------//-----------//-----------//-----------//-----------//-----------
#ifndef _PARAM_H_
#define _PARAM_H_

//-----------//-----------//-----------//-----------//-----------//-----------
// macro
//-----------//-----------//-----------//-----------//-----------//-----------
#define ScreenX            800
#define ScreenY            600
#define WindowX            1024
#define WindowY            768
#define GroupX             32
#define GroupY             32

#define Aspect             ((float)ScreenY / (float)ScreenX)
//#define ScreenX            1920
//#define ScreenY            1080

#define SHADER_Cx_MAX      1024
#define SHADER_Mx_MAX      (SHADER_Cx_MAX / 4)
#define SHADER_FILENAME    "main.fx"

#endif //_PARAM_H_
