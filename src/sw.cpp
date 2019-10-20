#include "sw.h"
#include "swMain.h"

#include "Main.h"
#include "vec.h"

#include "swToolkit.h"

#include "Common/File.h"

#include <memory.h>
#include <assert.h>

#include <sstream>
#include <math.h>

typedef unsigned char Byte;

static int /* SWWriteMode */ swCurrentMode = SW_REPLACE;

uint32_t* swBMPBuffer = nullptr;
float *swDepthBuffer = nullptr;

int swBufferWidth = 1;
int swBufferHeight = 1;

void swWindow_Resize(int w, int h) {
	swBufferWidth = w;
	swBufferHeight = h;
	
	delete[] swBMPBuffer;
	swBMPBuffer = new uint32_t[w * h];
	
	delete[] swDepthBuffer;
	swDepthBuffer = new float[w * h];
}

void swWindow_Destroy() {
	delete[] swBMPBuffer;
	swBMPBuffer = nullptr;
	
	delete[] swDepthBuffer;
	swDepthBuffer = nullptr;
}

static quatf swClearColorValue(0,0,0,0);

static float swClearDepthValue = 0.f;

void swClear(int bits) {
	const int count = swBufferWidth * swBufferHeight;

	if (bits & SW_COLOR_BUFFER_BIT && swBMPBuffer) {
		int write = (unsigned char)(swClearColorValue.z * 255.f) |
					(unsigned char)(swClearColorValue.y * 255.f) << 8 |
					(unsigned char)(swClearColorValue.x * 255.f) << 16 |
					(unsigned char)(swClearColorValue.w * 255.f) << 24;
		std::fill(swBMPBuffer, swBMPBuffer + count, write);
	}

	//for some odd reason, these memsets only look at the first 8 bits of the 32-bit fill value passed.  WHY?!?!?!

	if (bits & SW_DEPTH_BUFFER_BIT && swDepthBuffer) {
		std::fill(swDepthBuffer, swDepthBuffer + count, swClearDepthValue);
	}
}

void swClearColor(float r, float g, float b, float a) {
	swClearColorValue = quatf(r,g,b,a);
}

void swClearDepth(double depth) {
	swClearDepthValue = 1.f - (float)depth;
}

void swFlush() {
	swutPostRedisplay();
}

void swSetWriteMode(int /* SWWriteMode */ mode) {
	swCurrentMode = mode;
}
