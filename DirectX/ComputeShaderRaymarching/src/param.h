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
#define ScreenX            1280
#define ScreenY            720
#define WindowX            1280
#define WindowY            720
#define GroupX             32
#define GroupY             32

#define Aspect             ((float)ScreenY / (float)ScreenX)
//#define ScreenX            1920
//#define ScreenY            1080

#define SHADER_Cx_MAX      1024
#define SHADER_Mx_MAX      (SHADER_Cx_MAX / 4)
#define SHADER_FILENAME    "main.fx"

#endif //_PARAM_H_
