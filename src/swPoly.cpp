#include <assert.h>

#include "sw.h"
#include "swMain.h"
#include "swPoly.h"
#include "swTexture.h"
#include "swState.h"
#include "swMatrix.h"
#include "swRender.h"

#include "vec.h"
#include "Main.h"


/**
 * depth range values
 */
float swZNear = 0, swZFar = 1;

/**
 * error code of our graphics library state
 * equivalent to the GL_*** error codes
 */
int swErrorCode = SW_NO_ERROR;

/**
 * the current vertex color
 */
static vec4f curColor(1,1,1,1);

/** 
 * the current surface normal
 */
static vec3f curNormal(0,0,0);

/**
 * the current texture coordinate
 */
static vec3f curTexCoord(0,0,0);

/**
 * the current primitive type being drawn
 */
int swThisPrim = -1;

/**
 * the vector of vertices per polygon
 */
Vertex swVertexStack[SW_VERTEX_STACK_SIZE];
int swVertexStackSize = 0;


static int swViewportX = 0, swViewportY = 0, swViewportWidth = 1, swViewportHeight = 1;

void swViewport(int x, int y, int width, int height) {
	if (swThisPrim != -1) {
		//then set the error code (which we're not using!)
		swErrorCode = SW_INVALID_OPERATION;
		return;
	}

	swViewportX = x;
	swViewportY = y;
	swViewportWidth = width;
	swViewportHeight = height;
}

static int swCurrentPolygonMode = SW_FILL;

void swPolygonMode(/* int face, */ int mode) {
	if (swThisPrim != -1) {
		//then set the error code (which we're not using!)
		swErrorCode = SW_INVALID_OPERATION;
		return;
	}

	switch (mode) {
	case SW_FILL:
	case SW_POINT:
	case SW_LINE:
		swCurrentPolygonMode = mode;
		break;
	default:
		swErrorCode = SW_INVALID_ENUM;
		break;
	}
}


/**
 * converts from normalized device coordinates to window coordinates
 * depends upon:
 *  centerX, centerY, viewportRect, swZNear, swZFar
 */
void Vertex::normalizedDeviceToWindow() {

//	//hard-coded until i see some code that makes viewport() calls...
	int viewportX = swViewportX;
	int viewportY = swViewportY;
	int viewportWidth = swViewportWidth;
	int viewportHeight = swViewportHeight;
	
	//sw reference:
	int halfWidth = viewportWidth >> 1;
	int halfHeight = viewportHeight >> 1;
	int centerX = viewportX + halfWidth;
	int centerY = viewportY + halfHeight;

	coord.x = centerX + (float)halfWidth * coord.x;
	coord.y = centerY - (float)halfHeight * coord.y;	//negative this to adjust for window Y coordinate flips
	coord.z = ((swZFar - swZNear) * coord.z + (swZNear + swZFar)) * 0.5f;
}

void swNormal3f(float x, float y, float z) {
	curNormal = vec3f(x,y,z);
}

void swColor3f(float r, float g, float b) {
	curColor = vec4f(r,g,b,1);
}

void swColor4f(float r, float g, float b, float a) {
	curColor = vec4f(r,g,b,a);
}

void swTexCoord2f(float x, float y) {
	curTexCoord = vec3f(x,y,0);
}

void swTexCoord3f(float x, float y, float z) {
	curTexCoord = vec3f(x,y,z);
}


void swVertex4f(float x, float y, float z, float w) {

	const mat44f &projMat = getProjectionMatrix();
	const mat44f &mdlvMat = getModelviewMatrix();
	const mat44f &texMat = getTextureMatrix();

	assert(&projMat);
	assert(&mdlvMat);
	assert(&texMat);

//coordinate
	vec4f curCoord(x,y,z,w);
	vec4f mdlvCoord = mdlvMat * curCoord;
	vec4f projCoord = projMat * mdlvCoord;

//color
	//just grab the color.  no color matrices. yet.
	vec3fixed color = vec3fixed(
		FLOAT_TO_FIXED(curColor.x * 255.f),
		FLOAT_TO_FIXED(curColor.y * 255.f),
		FLOAT_TO_FIXED(curColor.z * 255.f));

//normal
	vec3f mdlvNormal = mdlvMat.transformVector(curNormal);
	vec3f projNormal = projMat.transformVector(mdlvNormal);
	if (getSWStateBit(SW_BIT_NORMALIZE)) {
		curNormal = vecUnit(curNormal);
		mdlvNormal = vecUnit(mdlvNormal);
		projNormal = vecUnit(projNormal);
	}

//texcoord
	//transform texcoords by current texture matrix
	vec4f texCoord4(VEC3ELEM(curTexCoord),1);

	//texgen
	{
		bool calcRefl = false;
		vec3f reflVec;

		extern int swTexGenMode[4];
		for (int i = 0; i < 4; i++) {

			if (!getSWStateBit(SW_BIT_TEXGEN_S + i)) continue;

			switch (swTexGenMode[i]) {

			case SW_OBJECT_LINEAR:
				texCoord4.fp()[i] = curCoord.fp()[i];
				break;

			case SW_NORMAL_MAP:
				if (i < 3) {
					texCoord4.fp()[i] = curNormal.fp()[i];
				} else {
					texCoord4.w = 1.f;
				}
				break;

			case SW_REFLECTION_MAP:

				if (!calcRefl) {
					calcRefl = true;
					reflVec = vecUnit(curNormal * ((curNormal % curCoord.vp()[0]) * 2.f) - curCoord.vp()[0]);
				}
					
				//assuming
				//	view pos = 0,0,0
				//  view dir = 0,0,1
				if (i < 3) {
//					texCoord4.fp()[i] = 2.f * curNormal.fp()[i] *
//						(curNormal % curCoord.vp()[0]) - curCoord.fp()[i];
					texCoord4.fp()[i] = reflVec.fp()[i];
				} else {
					texCoord4.w = 1.f;
				}
				break;

			case SW_OBJECT_CYLINDER:
				switch (i) {
				case 0: texCoord4.x = (float)(atan2( y, x) / (2.0 * M_PI));	break;
				case 1:	texCoord4.y = z; break;
				case 2: texCoord4.z = (float)sqrt(x * x + y * y); break;
				case 3: texCoord4.w = 1.f; break;
				}
				break;
			case SW_OBJECT_SPHERE:
				switch (i) {
				case 0: texCoord4.x = (float)(atan2( y, x) / (2.0 * M_PI));	break;
				case 1: texCoord4.y = (float)(atan2( z, sqrt(x * x + y * y)) / M_PI) + 0.5f;	break;
				case 2: texCoord4.z = vecLength(mdlvCoord.vp()[0]); break;
				case 3: texCoord4.w = 1.f; break;
				}
				break;
			}
		}
	}

	//grab our texcoords
	texCoord4 = texMat * texCoord4;
	vec3fixed texCoordFixed(
		FLOAT_TO_FIXED(texCoord4.x),
		FLOAT_TO_FIXED(texCoord4.y),
		FLOAT_TO_FIXED(texCoord4.z));

#ifdef USE_FRAGMENT_NORMALS
	//dangerous stuff here
	vec3fixed normalFixed(
		FLOAT_TO_FIXED(curNormal.x),
		FLOAT_TO_FIXED(curNormal.y),
		FLOAT_TO_FIXED(curNormal.z));
#endif

//vertex
	//create our mesh vertex object
	Vertex vertex(projCoord, color, texCoordFixed
#ifdef USE_FRAGMENT_NORMALS
		, normalFixed
#endif
		);

	//clip to normalized device
	vertex.clipToNormalizedDevice();

	//push it onto the vertex stack
	assert(swVertexStackSize < (int)numberof(swVertexStack));
	swVertexStack[swVertexStackSize] = vertex;
	swVertexStackSize++;
}

void swVertex3f(float x, float y, float z) {
	swVertex4f(x,y,z,1);
}

void swVertex3fv(float *v) {
	swVertex4f(v[0],v[1],v[2],1);
}

void swVertex2f(float x, float y) {
	swVertex4f(x,y,0,1);
}

void swBegin(int prim) {

	if (swThisPrim != -1) {
		//then set the error code (which we're not using!)
		swErrorCode = SW_INVALID_OPERATION;
		return;
	}

	//does this unalloc the vector's buffer, or simply zero the size and keep the capacity?
	swVertexStackSize = 0;

	switch (prim) {
	case SW_POLYGON:
	case SW_LINES:
		swThisPrim = prim;
		break;

	default:
		swErrorCode = SW_INVALID_ENUM;
		return;
	}
}

void swEnd() {
	//finalize any rendering
	switch (swThisPrim) {
	case SW_LINES:
		swDrawLines();
		break;

	case SW_LINE_LOOP:
		swDrawLineLoop();
		break;

	case SW_POLYGON:
		switch (swCurrentPolygonMode) {
		case SW_POINT:
			swDrawPoints();
			break;
		case SW_LINE:
			swDrawLineLoop();
			break;
		default:	//SW_FILL:
			swDrawPolygons();
			break;
		}
		break;

	default:
		swErrorCode = SW_INVALID_OPERATION;
		break;
	}

	//and clear the primitive state
	swThisPrim = -1;
}
