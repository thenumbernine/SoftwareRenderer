#pragma once

#include "sw.h"

void swWindow_Init(/*HWND hWnd*/);
void swWindow_Destroy();
void swWindow_Paint();
void swWindow_Resize(int w, int h);

//extern BITMAPINFO swCurrentBMI;
//extern long *swBMPBuffer;

extern float *swDepthBuffer;
extern swImage SWCurrentImage;
void swResizeDepthBuffer(int w, int h);
