#include "sw.h"
#include "swMatrix.h"

#include <assert.h>

//this is me working out the header abstraction level
extern int swThisPrim;
extern int swErrorCode;

/**
 * structure for matrix stacks
 */
template <int size>		//i may get in trouble in the future for using something as rigid as templates...
class MatrixStack {
public:

	// the array of matrices
	mat44f matrix[size];

	// the currently selected matrix in the stack
	int index;

	// creates a new matrix stack
	MatrixStack() {
		reset();
	}

	// resets the matrix index and contents
	void reset() {
		//zero our index initially
		index = 0;

		//just in case this doesn't happen, fill each matrix with the identity
		mat44f ident = mat44f();
		for (int i = 0; i < size; i++) {
			matrix[i] = ident;
		}
	}

	/**
	 * pushes the current matrix
	 * returns 'true' upon success, 'false' upon failure
	 */
	bool push() {
		//test for stack overflow
		if (index == size - 1) return false;

		//copy up the new matrix
		matrix[index+1] = matrix[index];

		//increment our current pointer
		index++;

		//and return 'true'
		return true;
	}

	/**
	 * pops the current matrix
	 * returns 'true' upon success, 'false' upon failure
	 */
	bool pop() {
		//same idea as above
		if (index == 0) return false;
		index--;
		return true;
	}


	mat44f *getCurrent() {
		return matrix + index;
	}
};

/**
 * modelview matrix stack
 */
static MatrixStack<32> modelviewMS;

/**
 * projection matrix stack
 */
static MatrixStack<2> projectionMS;

/**
 * texture coordinate matrix stack
 */
static MatrixStack<2> textureMS;

/**
 * the current matrix stack selected (via swMatrixMode)
 */
static int matrixMode = SW_MODELVIEW;

void swInitialize() {
	modelviewMS.reset();
	projectionMS.reset();
	textureMS.reset();
	matrixMode = SW_MODELVIEW;
}

/**
 * selects the current matrix
 */
void swMatrixMode(int mode) {
	if (swThisPrim != -1) {		//func between glBegin and glEnd
		swErrorCode = SW_INVALID_OPERATION;
		return;
	}

	switch (mode) {
	case SW_MODELVIEW:
	case SW_PROJECTION:
	case SW_TEXTURE:
		matrixMode = mode;
		break;
	default:
		swErrorCode = SW_INVALID_ENUM;
		return;
	}
}

/**
 * returns a pointer to the current matrix of the current stack
 */
static mat44f *getSelectedMatrix() {
	switch (matrixMode) {
	case SW_MODELVIEW:
		return modelviewMS.getCurrent();
	case SW_PROJECTION:
		return projectionMS.getCurrent();
	case SW_TEXTURE:
		return textureMS.getCurrent();
	}
	return NULL;
}

/**
 * post-multiplies the parameter matrix with the current stack's current matrix
 */
static void postMult(const mat44f &m) {
	mat44f *s = getSelectedMatrix();
	assert(s);
	*s *= m;
}

/**
 * pushes the current matrix onto the selected stack
 */
void swPushMatrix() {	//glPushMatrix
	if (swThisPrim != -1) {		//func between glBegin and glEnd
		swErrorCode = SW_INVALID_OPERATION;
		return;
	}

	bool res = true;

	switch (matrixMode) {
	case SW_MODELVIEW:
		res = modelviewMS.push();
		break;
	case SW_PROJECTION:
		res = projectionMS.push();
		break;
	case SW_TEXTURE:
		res = textureMS.push();
		break;
	}

	if (!res) {
		swErrorCode = SW_STACK_OVERFLOW;
		return;
	}
}

/**
 * pops the current matrix from the selected stack
 */
void swPopMatrix() {

	bool res = true;

	switch (matrixMode) {
	case SW_MODELVIEW:
		res = modelviewMS.pop();
		break;
	case SW_PROJECTION:
		res = projectionMS.pop();
		break;
	case SW_TEXTURE:
		res = textureMS.pop();
		break;
	}

	if (!res) {
		swErrorCode = SW_STACK_UNDERFLOW;
		return;
	}
}

/**
 * sets the current matrix to identity
 */
void swLoadIdentity() {
	mat44f *s = getSelectedMatrix();
	assert(s);
	*s = mat44f();
}

/**
 * rotates the current matrix
 */
void swRotatef(float degrees, float x, float y, float z) {
	if (swThisPrim != -1) {		//func between glBegin and glEnd
		swErrorCode = SW_INVALID_OPERATION;
		return;
	}
	float len = (float)sqrt(x*x + y*y + z*z);
	if (len > 0) {
		len = 1.f / len;
		x *= len;
		y *= len;
		z *= len;
	}
	float theta = degrees * (float)M_PI / 180.f;
	float c = (float)cos(theta);
	float s = (float)sin(theta);
	float t = 1.f - c;

	//todo - only multiply the upper 3x3 components
	postMult(
		mat44f(
			t*x*x + c,		t*x*y + s*z,	t*x*z - s*y,	0,
			t*y*x - s*z,	t*y*y + c,		t*y*z + s*x,	0,
			t*z*x + s*y,	t*z*y - s*x,	t*z*z + c,		0,
			0,				0,				0,				1));
}

/**
 * scales the current matrix
 */
void swScalef(float x, float y, float z) {
	if (swThisPrim != -1) {		//func between glBegin and glEnd
		swErrorCode = SW_INVALID_OPERATION;
		return;
	}

//	postMult(
//		mat44f(
//			x,0,0,0,
//			0,y,0,0,
//			0,0,z,0,
//			0,0,0,1));

	//faster scale routine:

	mat44f &s = *getSelectedMatrix();
	assert(&s);

	s.x *= x;
	s.y *= y;
	s.z *= z;

}

/**
 * translates the current matrix
 */
void swTranslatef(float x, float y, float z) {
	if (swThisPrim != -1) {		//func between glBegin and glEnd
		swErrorCode = SW_INVALID_OPERATION;
		return;
	}

//	postMult(
//		mat44f(
//			1,0,0,0,
//			0,1,0,0,
//			0,0,1,0,
//			x,y,z,1));

	//faster translate routine:

	mat44f &s = *getSelectedMatrix();
	assert(&s);

	s.w += s.x * x + s.y * y + s.z * z;
}

/**
 * sets up the ortho matrix
 */
void swOrtho(float l, float r, float b, float t, float n, float f) {

	if (swThisPrim != -1) {
		swErrorCode = SW_INVALID_OPERATION;
		return;
	}

	float dz = f - n;
	float dx = r - l;
	float dy = t - b;

	postMult(
		mat44f(
#if 0	//gl specs:
			2.f/dx,		0,			0,			0,
			0,			2.f/dy,		0,			0,
			0,			0,			-2.f/dz,	0,
			(-r-l)/dx,	(-t-b)/dy,	(-f-n)/dz,	1.f));
#else	//the math:
			2.f/dx,		0,			0,			0,
			0,			2.f/dy,		0,			0,
			0,			0,			2.f/dz,		0,
			(-r-l)/dx,	(-t-b)/dy,	(-f-n)/dz,	1.f));
#endif
}

/**
 * sets up a frustum matrix
 */
void swFrustum(float l, float r, float b, float t, float n, float f) {
	if (n <= 0 && f <= 0) {
		swErrorCode = SW_INVALID_VALUE;
		return;
	}
	if (swThisPrim != -1) {		//func between glBegin and glEnd
		swErrorCode = SW_INVALID_OPERATION;
		return;
	}
	float dx = r - l;
	float dy = t - b;
	float dz = f - n;

	postMult(
		mat44f(
#if 0	//gl specs:
			2.f*n/dx,	0,			0,			0,
			0,			2.f*n/dy,	0,			0,
			(r+l)/dx,	(t+b)/dy,	(-f-n)/dz,	-1,
			0,			0,			-2.f*f*n/dz,0));
#else	//the math:
			-2.f*n/dx,	0,			0,			0,
			0,			-2.f*n/dy,	0,			0,
			(r+l)/dx,	(t+b)/dy,	(-f-n)/dz,	-1,
			0,			0,			2.f*f*n/dz,	0));
#endif
}

/**
 * applies a perspective view on the scene
 */
void swPerspective(float fovy, float aspectRatio, float f, float n) {
	if (swThisPrim != -1) {		//func between glBegin and glEnd
		swErrorCode = SW_INVALID_OPERATION;
		return;
	}

	float dz = f - n;
	float sz = f + n;
	float pz = f * n;

	float l = 1.f / (float)tan(fovy * M_PI / 360.0 /*y-axis fov*/);

	postMult(
		mat44f(
			-l*n/aspectRatio, 0, 0,		0,
			0,		-l*n,	0,			0,
			0,		0,		-sz/dz,		-1,
			0,		0,		2.f*pz/dz,	0));
}

/**
 * applies the matrix stored in 'm' to the scene
 * m is a column-major array of 16 floats
 */
void swMultMatrixf(const float *m) {
	if (swThisPrim != -1) {		//func between glBegin and glEnd
		swErrorCode = SW_INVALID_OPERATION;
		return;
	}

	postMult( *(mat44f *)m );
}

/**
 * replcae the current matrix by that stored in 'm'
 * column-major 16 floats
 */
void swLoadMatrixf(const float *m) {
	mat44f *s = getSelectedMatrix();
	assert(s);
	*s = *(mat44f *)m;
}

/**
 * shortcuts our perspective view (?)
 */
void swLookAt(float from[3], float at[3], float up[3]) {
	mat44f m(
		0, 0, 0, 0,						//x column - compute me
		up[0], up[1], up[2], 0,			//y column - cross & normalize me
		-at[0], -at[1], -at[2], 0,		//z column - normalize
		from[0], from[1], from[2], 1);	//w column - we're good

	//do a cross to get the X & Y axii correct
	*m.x.vp() = (*m.y.vp()) * (*m.z.vp());		//x = y * z
	*m.y.vp() = (*m.z.vp()) * (*m.x.vp());		//y = z * x

	//normalize all
	*m.x.vp() = vecUnit(*m.x.vp());
	*m.y.vp() = vecUnit(*m.y.vp());
	*m.z.vp() = vecUnit(*m.z.vp());

	//and apply
	postMult(m);
}

const mat44f &getProjectionMatrix() {
	return *projectionMS.getCurrent();
}
const mat44f &getModelviewMatrix() {
	return *modelviewMS.getCurrent();
}
const mat44f &getTextureMatrix() {
	return *textureMS.getCurrent();
}
