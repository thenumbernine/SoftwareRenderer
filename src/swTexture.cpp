#include <assert.h>
#include <string.h>

#include "sw.h"
#include "swTexture.h"
#include "Main.h"

extern int swErrorCode;
extern int swThisPrim;

static const int MAX_TEXTURES = 1024;

static int textureIndex = 1;	//next new texture?

static SWTexture *textureArray[MAX_TEXTURES] = {NULL};

typedef struct {
	int selectedTexture;
	int enabled;
} textureState_t;

static textureState_t textureState[TEX_STATE_COUNT] = {
	{0,0},
	{0,0},
	{0,0},
	{0,0},
};

int SWImage::filterMethod = SW_NEAREST;

template <const int DIM>
bool SWImageND<DIM>::upload(int *sizes, const void *pixels) {	//assumed RGB

	data = NULL;

	int dataSize = 3;

	for (int i = 0; i < DIM; i++) {

		//calculate log base 2 value of each dimension's size
		logDim[i] = -1;
		for (int x = sizes[i]; x; x >>= 1) {
			logDim[i]++;
		}

		size[i] = 1 << logDim[i];

		//ensure that all the dimensions are a power of two
		if (size[i] != sizes[i]) {
			printf("texture had non-power-of-two size: %d\n", sizes[i]);
			return false;
		}

		//calculate the size, in bytes , of the texture
		dataSize *= size[i];
	}

	//allocate the texture
	data = new unsigned char[dataSize];
	if (!data) {
		printf("failed to allocate texture data of %d bytes\n", dataSize);
		return false;
	}

	memcpy(data, pixels, dataSize);

	return true;
}

void swGenTextures(int n, unsigned int *textures) {
	for (int i = 0; i < n; i++) {
		*textures = textureIndex;
		textureArray[textureIndex] = NULL;	//so long as we are always increasing the index
		textureIndex++;
		textures++;
	}
}

void swDeleteTextures(int n, unsigned int *textures) {
	for (int i = 0; i < n; i++) {

		//see if this texture is a valid texture
		if (*textures > 0 &&
			*textures < numberof(textureArray) &&
			textureArray[*textures])
		{
			//unalloc itself
			delete textureArray[*textures];
			//nullify pointer
			textureArray[*textures] = NULL;
		}

		//increment our parameter pointer
		textures++;
	}
}

void swSetTextureStateEnable(int state, int enable) {
	textureState[state].enabled = enable;
}

void SWTexture::setFilterMethod(int method) {
	filterMethod = method;
}

void SWTexture::setBorderMethod(int index, int method) {
	assert(index >= 0 && index < (int)numberof(borderMethod));
	borderMethod[index] = method;
}

void SWTexture::initFilter() {
	SWImage::setFilterMethod(filterMethod);
}

SWTexture *swGetStateTexture(int state) {
	assert(state >= 0 && state < TEX_STATE_COUNT);

	if (textureState[state].enabled) {
		int selectedTexture = textureState[state].selectedTexture;
		if (selectedTexture > 0 && selectedTexture < (int)numberof(textureArray)) {
			return textureArray[selectedTexture];
		}
	}

	return NULL;
}

SWTexture *swGetSelectedTexture() {
	SWTexture *tex;

	for (int state = TEX_STATE_COUNT-1; state >= 0; state--) {
		tex = swGetStateTexture(state);
		if (tex) return tex;
	}

	return NULL;
}

static int getTexTargetState(int target) {
	switch (target) {
	case SW_TEXTURE_1D:
		return TEX_STATE_1D;
	case SW_TEXTURE_2D:
		return TEX_STATE_2D;
	case SW_TEXTURE_CUBE_MAP:
		return TEX_STATE_CUBE_MAP;
	case SW_TEXTURE_3D:
		return TEX_STATE_3D;
	}

	return -1;
}

void swBindTexture(int target, int texture) {

	if (swThisPrim != -1) {
		printf("SW_INVALID_OPERATION\n");
		swErrorCode = SW_INVALID_OPERATION;
		return;
	}

	int state = getTexTargetState(target);
	if (state == -1) {
		printf("SW_INVALID_ENUM\n");
		swErrorCode = SW_INVALID_ENUM;
		return;
	}

	textureState[state].enabled = true;
	textureState[state].selectedTexture = texture;
}

void swTexImage1D(int target, int width, const void *pixels) {
	if (target != SW_TEXTURE_1D) return;

	//does TEXTURE_1D need to be enabled for texture uploads?
	int index = textureState[TEX_STATE_1D].selectedTexture;
	if (index <= 0 || index >= (int)numberof(textureArray)) {
		printf("swTexImage1D: index %d out of bounds\n", index);
		return;
	}

	//clear any old
	if (textureArray[index]) {
		delete textureArray[index];
	}

	//create any new
	SWTexture1D *tex1D = new SWTexture1D();
	textureArray[index] = tex1D;

	int sizes[] = {width};
	tex1D->image.upload(sizes, pixels);
}

//glTexImage2D
void swTexImage2D(int target, int width, int height, const void *pixels) {
	switch (target) {
	case SW_TEXTURE_CUBE_MAP_POSITIVE_X:
	case SW_TEXTURE_CUBE_MAP_NEGATIVE_X:
	case SW_TEXTURE_CUBE_MAP_POSITIVE_Y:
	case SW_TEXTURE_CUBE_MAP_NEGATIVE_Y:
	case SW_TEXTURE_CUBE_MAP_POSITIVE_Z:
	case SW_TEXTURE_CUBE_MAP_NEGATIVE_Z:
		if (!textureState[TEX_STATE_CUBE_MAP].enabled) {
			swErrorCode = SW_INVALID_ENUM;

			printf("TEX_STATE_CUBE_MAP not enabled!\n");

		} else {

			//first make sure we have a valid cubemap
			SWTexture *tex = swGetStateTexture(TEX_STATE_CUBE_MAP);
			if (!tex) {

				//ironically, this is the part in the code where the other states CREATE the
				//texture object.  however, even in opengl, the texture object can be modified
				//before uploading anything to it...
				//seems like a pickle to me.

//				printf("no cube map texture bound\n");
//				return;						//tex doesnt exist - throw an error

				int index = textureState[TEX_STATE_CUBE_MAP].selectedTexture;
				if (index <= 0 || index >= (int)numberof(textureArray)) {
					printf("swTexImageCubeMap: index %d out of bounds\n", index);
					return;
				}

				tex = new SWTextureCubeMap();
				textureArray[index] = tex;

			}
			if (tex->target() != SW_TEXTURE_CUBE_MAP) {
				printf("cube map texture has bad target\n");
				return;	//tex isnt cubemap - throw an error
			}
			int side = target - SW_TEXTURE_CUBE_MAP_POSITIVE_X;
			SWTextureCubeMap *texCubeMap = (SWTextureCubeMap *)tex;
			assert(side >= 0 && side < (int)numberof(texCubeMap->image));
			int sizes[] = {width, height};
			texCubeMap->image[side].upload(sizes, pixels);
		}
		break;

	case SW_TEXTURE_2D:
		{
			//does TEXTURE_2D need to be enabled for texture uploads?
			int index = textureState[TEX_STATE_2D].selectedTexture;
			if (index <= 0 || index >= (int)numberof(textureArray)) {
				printf("swTexImage2D: index %d out of bounds\n", index);
				return;
			}

			//clear any old
			if (textureArray[index]) {
				delete textureArray[index];
			}

			//create any new
			SWTexture2D *tex2D = new SWTexture2D();
			textureArray[index] = tex2D;

			int sizes[] = {width, height};
			tex2D->image.upload(sizes, pixels);
		}
		break;

	default:
		swErrorCode = SW_INVALID_ENUM;
		break;
	}
}

void swTexImage3D(int target, int width, int height, int depth, const void *pixels) {
	if (target != SW_TEXTURE_3D) return;

	//does TEXTURE_3D need to be enabled for texture uploads?
	int index = textureState[TEX_STATE_3D].selectedTexture;
	if (index <= 0 || index >= (int)numberof(textureArray)) {
		printf("swTexImage2D: index %d out of bounds\n", index);
		return;
	}

	//clear any old
	if (textureArray[index]) {
		delete textureArray[index];
	}

	//create any new
	SWTexture3D *tex3D = new SWTexture3D();
	textureArray[index] = tex3D;

	int sizes[] = {width, height, depth};
	tex3D->image.upload(sizes, pixels);
}

void swTexParameteri(int target, int pname, int param) {
	if (swThisPrim != -1) {
		swErrorCode = SW_INVALID_OPERATION;
		return;
	}

	int state = getTexTargetState(target);
	if (state == -1) {
		swErrorCode = SW_INVALID_ENUM;
		return;
	}

	int index = textureState[state].selectedTexture;
	if (!index) return;
	assert(index > 0 && index < (int)numberof(textureArray));

	SWTexture *tex = textureArray[index];
	assert(tex);

	//todo - accept
	//pname == SW_TEXTURE_WRAP_S, SW_TEXTURE_WRAP_T, SW_TEXTURE_WRAP_R
	//	param == SW_REPEAT, SW_CLAMP, SW_CLAMP_TO_EDGE
	//pname = SW_TEXTURE_MAG_FILTER
	//	param = SW_NEAREST, SW_LINEAR
	//pname = SW_TEXTURE_MIN_FILTER
	//	param = SW_NEAREST, SW_LINEAR, SW_NEAREST_MIPMAP_NEAREST, SW_LINEAR_MIPMAP_NEAREST, SW_NEAREST_MIPMAP_LINEAR, SW_LINEAR_MIPMAP_LINEAR

	switch (pname) {
	case SW_TEXTURE_MIN_FILTER:
		break;	//not yet coded

	case SW_TEXTURE_WRAP_S:
		tex->setBorderMethod(0, param);
		break;
	case SW_TEXTURE_WRAP_T:
		tex->setBorderMethod(1, param);
		break;
	case SW_TEXTURE_WRAP_R:
		tex->setBorderMethod(2, param);
		break;

	case SW_TEXTURE_MAG_FILTER:

		switch (param) {
		case SW_NEAREST:
		case SW_LINEAR:
			tex->setFilterMethod(param);
			break;
		default:
			swErrorCode = SW_INVALID_ENUM;
			return;
		}

		break;
	default:
		swErrorCode = SW_INVALID_ENUM;
		return;
	}

}

int swTexGenMode[4];

void swTexGeni(int coord, int pname, int param) {
	if (swThisPrim != -1) {
		swErrorCode = SW_INVALID_OPERATION;
		return;
	}

	if (pname != SW_TEXTURE_GEN_MODE) {
		swErrorCode = SW_INVALID_ENUM;
		return;
	}

	int index;

	switch (coord) {
	case SW_S:	index = 0;	break;
	case SW_T:	index = 1;	break;
	case SW_R:	index = 2;	break;
	case SW_Q:	index = 3;	break;
	default:
		swErrorCode = SW_INVALID_ENUM;
		return;
	}

	assert(index >= 0 && index < (int)numberof(swTexGenMode));

	switch (param) {
	case SW_OBJECT_LINEAR:
	case SW_NORMAL_MAP:
	case SW_REFLECTION_MAP:
	case SW_OBJECT_CYLINDER:
	case SW_OBJECT_SPHERE:
		break;
	default:
		swErrorCode = SW_INVALID_ENUM;
		return;
	}

	swTexGenMode[index] = param;

}
