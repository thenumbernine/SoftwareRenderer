#pragma once

#include "vec.h"
#include "Main.h"

/*
	texture class map:

Image
	unsigned char *data;
Image1D : Image
	int logWidth
Image2D : Image1D
	int logHeight
Image3D : Image2D
	int logDepth

	//or we can template it all
ImageND<n> : Image
	int logDim[n]

Texture
Texture1D : Texture
	Image1D image;
Texture2D : Texture
	Image2D image;
TextureCubeMap : Texture
	Image2D image[6]
Texture3D: Texture
	Image3D image;
*/

class SWImage {
private:

	void clear() {
		data = NULL;
	}

	void free() {
		if (data) delete[] data;
		clear();
	}

protected:

	unsigned char *data;

	static int filterMethod;

public:

	static void setFilterMethod(int method) {
		filterMethod = method;
	}

	bool active() {
		return !!data;
	}

	SWImage() { clear(); }
	~SWImage() { free(); }
};

//question: are template int's always const?
template <const	int DIM>
class SWImageND : public SWImage {
private:
	int logDim[DIM];
	int size[DIM];

public:
	void getTexCoordColor(const vec3fixed &texcoord, int *c) const;

	bool upload(int *sizes, const void *pixels);	//assumed RGB
};

//give us our three base image classes: (why did i use templates anyways?)
typedef SWImageND<1> SWImage1D;
typedef SWImageND<2> SWImage2D;
typedef SWImageND<3> SWImage3D;

template <>
void SWImageND<1>::getTexCoordColor(
	const vec3fixed &texcoord,	//texcoord in affine space
	int *c) const				//result color array of 3
{
	switch (filterMethod) {
	case SW_NEAREST:
		{

			int tx = FIXED_TO_INT(texcoord.x * size[0]) & (size[0] - 1);
			unsigned char *texel = data + 3 * tx;
			c[0] = texel[0];
			c[1] = texel[1];
			c[2] = texel[2];
		}
		return;

	case SW_LINEAR:
		{
			int ftx = texcoord.x * size[0];
			int frac1x = ftx & FIXED_FRACTION_MASK;
			int frac0x = FIXED_FRACTION_MASK - frac1x;
			int t0x = FIXED_TO_INT(ftx) & (size[0] - 1);
			int t1x = (t0x + 1) & (size[0] - 1);

			unsigned char *texel0 = data + 3 * t0x;
			unsigned char *texel1 = data + 3 * t1x;

			int frac0 = 0xFFFF & (frac0x >> (FIXED_FRACTION_BITS >> 1));
			int frac1 = 0xFFFF & (frac1x >> (FIXED_FRACTION_BITS >> 1));

#define calcTexel1D(i)									\
			c[i] = (											\
				( ((int)texel0[i] & 0xFF) * frac0				\
				+ ((int)texel1[i] & 0xFF) * frac1				\
				) >> FIXED_FRACTION_BITS);

			calcTexel1D(0);	//red
			calcTexel1D(1);	//green
			calcTexel1D(2);	//blue
		}
		return;
	}
}

template <>
void SWImageND<2>::getTexCoordColor(
	const vec3fixed &texcoord,	//texcoord in affine space
	int *c) const				//result color array of 3
{
	switch (filterMethod) {
	case SW_NEAREST:
		{

			int tx = FIXED_TO_INT(texcoord.x * size[0]) & (size[0] - 1);
			int ty = FIXED_TO_INT(texcoord.y * size[1]) & (size[1] - 1);
			unsigned char *texel = data + 3 * (tx + ty * size[0]);
			c[0] = texel[0];
			c[1] = texel[1];
			c[2] = texel[2];
		}
		return;

	case SW_LINEAR:
		{

			int ftx = texcoord.x * size[0];
			int frac1x = ftx & FIXED_FRACTION_MASK;
			int frac0x = FIXED_FRACTION_MASK - frac1x;
			int t0x = FIXED_TO_INT(ftx) & (size[0] - 1);
			int t1x = (t0x + 1) & (size[0] - 1);

			int fty = texcoord.y * size[1];
			int frac1y = fty & FIXED_FRACTION_MASK;
			int frac0y = FIXED_FRACTION_MASK - frac1y;
			int t0y = FIXED_TO_INT(fty) & (size[1] - 1);
			int t1y = (t0y + 1) & (size[1] - 1);

			unsigned char *texel00 = data + 3 * (t0x + t0y * size[0]);
			unsigned char *texel10 = data + 3 * (t1x + t0y * size[0]);
			unsigned char *texel01 = data + 3 * (t0x + t1y * size[0]);
			unsigned char *texel11 = data + 3 * (t1x + t1y * size[0]);

			int frac00 = 0xFFFF & ((frac0x * frac0y) >> FIXED_FRACTION_BITS);
			int frac01 = 0xFFFF & ((frac0x * frac1y) >> FIXED_FRACTION_BITS);
			int frac10 = 0xFFFF & ((frac1x * frac0y) >> FIXED_FRACTION_BITS);
			int frac11 = 0xFFFF & ((frac1x * frac1y) >> FIXED_FRACTION_BITS);

#define calcTexel2D(i)									\
			c[i] = (											\
				( ((int)texel00[i] & 0xFF) * frac00				\
				+ ((int)texel01[i] & 0xFF) * frac01				\
				+ ((int)texel10[i] & 0xFF) * frac10				\
				+ ((int)texel11[i] & 0xFF) * frac11				\
				) >> FIXED_FRACTION_BITS);

			calcTexel2D(0);
			calcTexel2D(1);
			calcTexel2D(2);
		}
		return;
	}
}

template <>
void SWImageND<3>::getTexCoordColor(
	const vec3fixed &texcoord,	//texcoord in affine space
	int *c) const				//result color array of 3
{

	switch (filterMethod) {
	case SW_NEAREST:
		{

			int tx = FIXED_TO_INT(texcoord.x * size[0]) & (size[0] - 1);
			int ty = FIXED_TO_INT(texcoord.y * size[1]) & (size[1] - 1);
			int tz = FIXED_TO_INT(texcoord.z * size[2]) & (size[2] - 1);
			unsigned char *texel = data + 3 * (tx + size[0] * (ty + size[1] * tz));
			c[0] = texel[0];
			c[1] = texel[1];
			c[2] = texel[2];
		}
		return;

	case SW_LINEAR:
		{
			int ftx = texcoord.x * size[0];
			int frac1x = ftx & FIXED_FRACTION_MASK;
			int frac0x = FIXED_FRACTION_MASK - frac1x;
			int t0x = FIXED_TO_INT(ftx) & (size[0] - 1);
			int t1x = (t0x + 1) & (size[0] - 1);

			int fty = texcoord.y * size[1];
			int frac1y = fty & FIXED_FRACTION_MASK;
			int frac0y = FIXED_FRACTION_MASK - frac1y;
			int t0y = FIXED_TO_INT(fty) & (size[1] - 1);
			int t1y = (t0y + 1) & (size[1] - 1);

			int ftz = texcoord.z * size[2];
			int frac1z = ftz & FIXED_FRACTION_MASK;
			int frac0z = FIXED_FRACTION_MASK - frac1z;
			int t0z = FIXED_TO_INT(ftz) & (size[2] - 1);
			int t1z = (t0z + 1) & (size[2] - 1);

			unsigned char *texel000 = data + 3 * (t0x + size[0] * (t0y + size[1] * t0z));
			unsigned char *texel100 = data + 3 * (t1x + size[0] * (t0y + size[1] * t0z));
			unsigned char *texel010 = data + 3 * (t0x + size[0] * (t1y + size[1] * t0z));
			unsigned char *texel110 = data + 3 * (t1x + size[0] * (t1y + size[1] * t0z));
			unsigned char *texel001 = data + 3 * (t0x + size[0] * (t0y + size[1] * t1z));
			unsigned char *texel101 = data + 3 * (t1x + size[0] * (t0y + size[1] * t1z));
			unsigned char *texel011 = data + 3 * (t0x + size[0] * (t1y + size[1] * t1z));
			unsigned char *texel111 = data + 3 * (t1x + size[0] * (t1y + size[1] * t1z));

		// a.b
		// c.d
		//----
		// 

			int frac000 = fixedMulLo(fixedMulLo(frac0x, frac0y), frac0z) & 0xFFFF;
			int frac100 = fixedMulLo(fixedMulLo(frac1x, frac0y), frac0z) & 0xFFFF;
			int frac010 = fixedMulLo(fixedMulLo(frac0x, frac1y), frac0z) & 0xFFFF;
			int frac110 = fixedMulLo(fixedMulLo(frac1x, frac1y), frac0z) & 0xFFFF;
			int frac001 = fixedMulLo(fixedMulLo(frac0x, frac0y), frac1z) & 0xFFFF;
			int frac101 = fixedMulLo(fixedMulLo(frac1x, frac0y), frac1z) & 0xFFFF;
			int frac011 = fixedMulLo(fixedMulLo(frac0x, frac1y), frac1z) & 0xFFFF;
			int frac111 = fixedMulLo(fixedMulLo(frac1x, frac1y), frac1z) & 0xFFFF;

#define calcTexel3D(i)												\
			c[i] = (												\
				( ((int)texel000[i] & 0xFF) * frac000				\
				+ ((int)texel010[i] & 0xFF) * frac010				\
				+ ((int)texel100[i] & 0xFF) * frac100				\
				+ ((int)texel110[i] & 0xFF) * frac110				\
				+ ((int)texel001[i] & 0xFF) * frac001				\
				+ ((int)texel011[i] & 0xFF) * frac011				\
				+ ((int)texel101[i] & 0xFF) * frac101				\
				+ ((int)texel111[i] & 0xFF) * frac111				\
				) >> FIXED_FRACTION_BITS);

			calcTexel3D(0);
			calcTexel3D(1);
			calcTexel3D(2);
		}
		return;
	}

}






class SWTexture {
protected:

	int filterMethod;
	int borderMethod[4];

public:

	virtual bool active() = 0;
	virtual int target() = 0;

	virtual void getTexCoordColor(const vec3fixed &tc, int *c) const = 0;

	SWTexture() {
		filterMethod = SW_NEAREST;
		for (size_t i = 0; i < numberof(borderMethod); i++) {
			borderMethod[i] = SW_REPEAT;
		}
	}

	virtual ~SWTexture() {}

	void setFilterMethod(int method);

	void initFilter();

	void setBorderMethod(int index, int method);

	void borderOp(fixed_t &tc, int i) const {
		switch (borderMethod[i]) {
		case SW_REPEAT:
			tc &= FIXED_FRACTION_MASK;
			break;
		case SW_CLAMP:
		case SW_CLAMP_TO_EDGE:			//what is this supposed to do different than GL_CLAMP?
			if (tc < 0) tc = 0;
			if (tc > FIXED_FRACTION_MASK) tc = FIXED_FRACTION_MASK;
			break;
		}
	}
};

class SWTexture1D : public SWTexture {
public:
	SWImage1D image;

	virtual bool active() {
		return image.active();
	}

	virtual int target() {
		return SW_TEXTURE_1D;
	}

	virtual void getTexCoordColor(const vec3fixed &texcoord, int *c) const {
		vec3fixed tc(texcoord);
		borderOp(tc.x, 0);
		image.getTexCoordColor(tc, c);
	}
};

class SWTexture2D : public SWTexture {
public:
	SWImage2D image;

	virtual bool active() {
		return image.active();
	}

	virtual int target() {
		return SW_TEXTURE_2D;
	}

	virtual void getTexCoordColor(const vec3fixed &texcoord, int *c) const {
		vec3fixed tc(texcoord);
		borderOp(tc.x, 0);
		borderOp(tc.y, 1);
		image.getTexCoordColor(tc, c);
	}
};

class SWTextureCubeMap : public SWTexture {
public:
	SWImage2D image[6];

	virtual bool active() {
		for (int i = 0; i < 6; i++) {
			if (!image[i].active()) return false;
		}
		return true;
	}

	virtual int target() {
		return SW_TEXTURE_CUBE_MAP;
	}

	virtual void getTexCoordColor(const vec3fixed &texcoord, int *c) const {

		//calc the maximum magnitude of all three texcoord's
		//use that to determine the image
		//then normalize the other texture coordinates
		//and pull the texel color

		const int fixed_1 = 1 << FIXED_FRACTION_BITS;

		vec3fixed tc;
		int side = -1;

		//initialize with any negative number, since all compared vals will be positive
		int major = -1;

		if (texcoord.x > 0) {
			if (major < texcoord.x) {
				major = texcoord.x;
				side = SW_TEXTURE_CUBE_MAP_POSITIVE_X;
			}
		} else {
			if (major < -texcoord.x) {
				major = -texcoord.x;
				side = SW_TEXTURE_CUBE_MAP_NEGATIVE_X;
			}
		}

		if (texcoord.y > 0) {
			if (major < texcoord.y) {
				major = texcoord.y;
				side = SW_TEXTURE_CUBE_MAP_POSITIVE_Y;
			}
		} else {
			if (major < -texcoord.y) {
				major = -texcoord.y;
				side = SW_TEXTURE_CUBE_MAP_NEGATIVE_Y;
			}
		}

		if (texcoord.z > 0) {
			if (major < texcoord.z) {
				major = texcoord.z;
				side = SW_TEXTURE_CUBE_MAP_POSITIVE_Z;
			}
		} else {
			if (major < -texcoord.z) {
				major = -texcoord.z;
				side = SW_TEXTURE_CUBE_MAP_NEGATIVE_Z;
			}
		}

		//bump us up a fraction if we got a zero texcoord
		if (!major) major = 1;

		major = fixedDiv( fixed_1, major);

		switch (side) {
		//negatives are backwards...

		case SW_TEXTURE_CUBE_MAP_POSITIVE_X:
			tc.x = (fixedMul(-texcoord.z, major) + fixed_1) >> 1;
			tc.y = (fixedMul(-texcoord.y, major) + fixed_1) >> 1;
			break;
		case SW_TEXTURE_CUBE_MAP_NEGATIVE_X:
			tc.x = -(fixedMul(-texcoord.z, major) + fixed_1) >> 1;
			tc.y = -(fixedMul( texcoord.y, major) + fixed_1) >> 1;
			break;
		case SW_TEXTURE_CUBE_MAP_POSITIVE_Y:
			tc.x = (fixedMul( texcoord.x, major) + fixed_1) >> 1;
			tc.y = (fixedMul( texcoord.z, major) + fixed_1) >> 1;
			break;
		case SW_TEXTURE_CUBE_MAP_NEGATIVE_Y:
			tc.x = -(fixedMul(-texcoord.x, major) + fixed_1) >> 1;
			tc.y = -(fixedMul( texcoord.z, major) + fixed_1) >> 1;
			break;
		case SW_TEXTURE_CUBE_MAP_POSITIVE_Z:
			tc.x = (fixedMul( texcoord.x, major) + fixed_1) >> 1;
			tc.y = (fixedMul(-texcoord.y, major) + fixed_1) >> 1;
			break;
		case SW_TEXTURE_CUBE_MAP_NEGATIVE_Z:
			tc.x = -(fixedMul( texcoord.x, major) + fixed_1) >> 1;
			tc.y = -(fixedMul( texcoord.y, major) + fixed_1) >> 1;
			break;
		}

		borderOp(tc.x, 0);
		borderOp(tc.y, 1);

		image[side - SW_TEXTURE_CUBE_MAP_POSITIVE_X].getTexCoordColor(tc, c);
	}
};

class SWTexture3D : public SWTexture {
public:
	SWImage3D image;

	virtual bool active() {
		return image.active();
	}

	virtual int target() {
		return SW_TEXTURE_3D;
	}

	virtual void getTexCoordColor(const vec3fixed &texcoord, int *c) const {
		vec3fixed tc(texcoord);
		borderOp(tc.x, 0);
		borderOp(tc.y, 1);
		borderOp(tc.z, 2);
		image.getTexCoordColor(tc, c);
	}
};

SWTexture *swGetSelectedTexture();

//perform different texel lookups for 2D and 3D
/*
fast integer LERP:
int ft_ = fixedDiv(invtc_, invz);			//fixed texture coordinate
int frac1_ = t & ((1 << frac_bits) - 1);		//fraction bits for that texcoord
int frac0_ = (1 << frac_bits) - 1 - frac1_;		//one minus the fraction

int t0_ = (t >> frac_bits) & (width - 1);	//integer bits for that texcoord
int t1_ = (t0_ + 1) & (width - 1);			//nextmost integer texcoord

result = (texel[t0_] * frac0_ + texel[t1_] * frac1_) >> frac_bits;

*/

//texture states:
enum {
	TEX_STATE_1D,
	TEX_STATE_2D,
	TEX_STATE_CUBE_MAP,
	TEX_STATE_3D,
	TEX_STATE_COUNT
};

void swSetTextureStateEnable(int state, int enable);
