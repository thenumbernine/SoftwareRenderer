#include "sw.h"
#include "swMain.h"
#include "swState.h"
#include "swTexture.h"

//this is me working out the header abstraction level
extern int swThisPrim;
extern int swErrorCode;

//todo - make an integer array out of this if we ever succeed 32 bits
int swEnableBits = 0;

void swEnableDisable(int cap, int enable) {

	if (swThisPrim != -1) {		//func between glBegin and glEnd
		swErrorCode = SW_INVALID_OPERATION;
		return;
	}

	switch (cap) {
		case SW_DEPTH_TEST: setSWStateBit(SW_BIT_DEPTH_TEST, enable); break;
		case SW_NORMALIZE: setSWStateBit(SW_BIT_NORMALIZE, enable); break;
		case SW_TEXTURE_GEN_S: setSWStateBit(SW_BIT_TEXGEN_S, enable); break;
		case SW_TEXTURE_GEN_T: setSWStateBit(SW_BIT_TEXGEN_T, enable); break;
		case SW_TEXTURE_GEN_R: setSWStateBit(SW_BIT_TEXGEN_R, enable); break;
		case SW_TEXTURE_GEN_Q: setSWStateBit(SW_BIT_TEXGEN_Q, enable); break;
		case SW_BLEND: setSWStateBit(SW_BIT_BLEND, enable); break;

		case SW_TEXTURE_1D: swSetTextureStateEnable(TEX_STATE_1D, enable); break;
		case SW_TEXTURE_2D: swSetTextureStateEnable(TEX_STATE_2D, enable); break;
		case SW_TEXTURE_CUBE_MAP:swSetTextureStateEnable(TEX_STATE_CUBE_MAP, enable); break;
		case SW_TEXTURE_3D: swSetTextureStateEnable(TEX_STATE_3D, enable); break;
	default:
		//throw an error
		swErrorCode = SW_INVALID_ENUM;
		break;
	}
}

void swEnable(int cap) {
	swEnableDisable(cap, true);
}

void swDisable(int cap) {
	swEnableDisable(cap, false);
}
