#pragma once

enum {
	SW_BIT_NORMALIZE,
	SW_BIT_DEPTH_TEST,
	SW_BIT_TEXGEN_S,
	SW_BIT_TEXGEN_T,
	SW_BIT_TEXGEN_R,
	SW_BIT_TEXGEN_Q,
	SW_BIT_BLEND,
	SW_BIT_COUNT
};

extern int swEnableBits;

inline int getSWStateBit(int index) {
	return (swEnableBits >> index) & 1;
}

inline void setSWStateBit(int index, int value) {
	if (value) {
		swEnableBits |= (1 << index);
	} else {
		swEnableBits &= ~(1 << index);
	}
}
