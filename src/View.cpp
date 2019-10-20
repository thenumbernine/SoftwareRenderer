#include <iostream>
#include "View.h"
#include "Viewport.h"
#include "Main.h"
#include "sw.h"

void View::reset() {
	scene = NULL;

	ortho = false;

	halfWidth = 1.f;
	halfHeight = 1.f;

	adjustMethod = VIEW_ADJUST_HEIGHT;
	znear = 1.f;
	zfar = 3000.f;

	basis.t = vec3f(0,0,0);
	basis.r = quatf(0,0,0,1);
}

void View::render(const Viewport *vp) {
	if (scene) {
		setupView(-1, vp->getAspectRatio());
		scene->render(vp, this);
	}
}

void View::setupView(int flags, float aspectRatio) const {

	if (flags & 
		(bitflag(VIEW_FLAG_CLEAR_PROJECTION) |
		 bitflag(VIEW_FLAG_APPLY_PROJECTION))
	) {
		setProjection(flags, aspectRatio);	//sets the field of vision / clip planes / etc
	}

	if (flags & 
		(bitflag(VIEW_FLAG_CLEAR_MODELVIEW) |
		bitflag(VIEW_FLAG_APPLY_ROTATION) |
		bitflag(VIEW_FLAG_APPLY_TRANSLATION))
	) {
		setModelview(flags);		//sets the translation / rotation
	}
}

//////// projection matrix transformation calls

void View::setProjection(int flags, float aspectRatio) const {

	swMatrixMode(SW_PROJECTION);
	
	//clear the projection if we are told to
	if (flags & bitflag(VIEW_FLAG_CLEAR_PROJECTION)) {
		swLoadIdentity();
	}

	//everything else in here is whether to apply the frustum or not
	if (!(flags & bitflag(VIEW_FLAG_APPLY_PROJECTION))) {
		return;
	}

	float glleft, glright, glbottom, gltop, glznear, glzfar;

	//use the camera defaults
	float trueHalfHeight = halfHeight;
	float trueHalfWidth = halfWidth;

	//. . .unless adjustMethod says otherwise. . .
	switch (adjustMethod) {
	case VIEW_ADJUST_HEIGHT:
		trueHalfHeight = halfWidth * aspectRatio;
		break;
	case VIEW_ADJUST_WIDTH:
		trueHalfWidth = halfHeight / aspectRatio;
		break;
	}

	glleft = -trueHalfWidth;
	glright = trueHalfWidth;
	glbottom = -trueHalfHeight;
	gltop = trueHalfHeight;

	glznear = znear;
	glzfar = zfar;

//std::cout << "aspectRatio " << aspectRatio << std::endl;
//std::cout << "glleft " << glleft << std::endl;
//std::cout << "glright " << glright << std::endl;
//std::cout << "glbottom " << glbottom << std::endl;
//std::cout << "gltop " << gltop << std::endl;
//std::cout << "glznear " << glznear << std::endl;
//std::cout << "glzfar " << glzfar << std::endl;
	if (!ortho)	swFrustum(glleft, glright, glbottom, gltop, glznear, glzfar);
	else		swOrtho(glleft, glright, glbottom, gltop, glznear, glzfar);
//exit(0);
}

//////// modelview matrix transformation calls

void View::setModelview(int flags) const {

	swMatrixMode(SW_MODELVIEW);

	if (flags & bitflag(VIEW_FLAG_CLEAR_MODELVIEW)) {
		swLoadIdentity();
	}

	//convert to the std. game coord system
	correctAngleInverse();

	if (flags & bitflag(VIEW_FLAG_APPLY_ROTATION)) {
		rotateInverse();
	}

	if (flags & bitflag(VIEW_FLAG_APPLY_TRANSLATION)) {
		translateInverse();
	}
}

void View::correctAngle() const {
	swRotatef(90,1,0,0);
	swRotatef(-90,0,1,0);
}

void View::correctAngleInverse() const {
	swRotatef(90,0,1,0);	//look down the 'x' axis
	swRotatef(-90,1,0,0);	//get the 'z' looking in the right dir
}

void View::translateInverse() const {
	swTranslatef(-basis.t.x, -basis.t.y, -basis.t.z);
}

void View::rotateInverse() const {
	quatf q = quatToAngleAxis(basis.r);
	swRotatef(q.w, -q.x, -q.y, -q.z);
}

void View::rotate() const {
	quatf q = quatToAngleAxis(basis.r);
	swRotatef(q.w, q.x, q.y, q.z);
}


/**
 * This method calculates:
 *	the frustum verticies
 *	the six border planes
 * TODO - move this method to (create a) Frustum.cpp ?
 */
void View::calculateFrustum(Frustum *frustum, float aspectRatio) {

	//default values
	float trueHalfHeight = halfHeight;
	float trueHalfWidth = halfWidth;

	//. . .unless adjustMethod says otherwise. . .
	switch (adjustMethod) {
	case VIEW_ADJUST_HEIGHT:
		trueHalfHeight = halfWidth * aspectRatio;
		break;
	case VIEW_ADJUST_WIDTH:
		trueHalfWidth = halfHeight / aspectRatio;
		break;
	}

	//calculate frustum vertices

	//use the bits of 'v' to determine which cube vertex to use
	//bitflag 0 reprents whether x is positive or negative
	//bitflag 1 does for y, and bitflag 2 does for z
	for (int v = 0; v < 8; v++) {
		//calc y value in viewspace
		frustum->vtx[v].y = (v & (1 << 1)) ? trueHalfWidth : -trueHalfWidth;

		//calc z value in viewspace
		frustum->vtx[v].z = (v & (1 << 2)) ? trueHalfHeight : -trueHalfHeight;

		//calc x value in viewspace
		if (!ortho) {
			if (!(v & (1 << 0))) {
				frustum->vtx[v].x = znear;

			//adjust for frustum shape
			} else {
				frustum->vtx[v].x = zfar;
				frustum->vtx[v].y *= zfar / znear;
				frustum->vtx[v].z *= zfar / znear;
			}
		} else {
			frustum->vtx[v].x = (v & (1 << 0)) ? zfar : znear;
		}

		frustum->vtx[v] = quatRotate(basis.r, frustum->vtx[v]) + basis.t;
	}

	//calculate the frustum planes

	/*
	0	znear
	1	zfar
	2	bottom
	3	left
	4	top
	5	right
	*/

	//quick front and back - normal is already same as camera dir

	int planeVtxs[] = {
		1,3,7,	//x+
		0,4,6,	//x-
		7,3,2,	//y+
		0,1,5,	//y-
		4,5,7,	//z+
		0,2,3,	//z-
	};

	for (int i = 0; i < 6; i++) {
		//very slow method of doing things:
		*frustum->plane[FRUSTUM_PLANE_XP + i].normal() = vecUnitNormal(
			frustum->vtx[planeVtxs[3*i]],
			frustum->vtx[planeVtxs[3*i+1]],
			frustum->vtx[planeVtxs[3*i+2]]);

		vec3f center = (
			frustum->vtx[planeVtxs[3*i]] +
			frustum->vtx[planeVtxs[3*i+1]] +
			frustum->vtx[planeVtxs[3*i+2]]) / 3.f;

		frustum->plane[FRUSTUM_PLANE_XP + i].calcDist(center);
	}
}
