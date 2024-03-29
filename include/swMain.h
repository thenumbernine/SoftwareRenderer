#pragma once

#include "sw.h"
#include <cstdint>

void swWindow_Destroy();
void swWindow_Resize(int w, int h);

extern uint32_t *swBMPBuffer;
extern float *swDepthBuffer;

extern int swBufferWidth;
extern int swBufferHeight;

void swResizeDepthBuffer(int w, int h);
