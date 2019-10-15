#pragma once

#include "vec.h"

//#define USE_FRAGMENT_NORMALS

/**
 * structure for a fragment
 */
class swFragment {
public:
	//direct mapping within the vertex structure
	//but later these components are scaled by the inverse-z value
	//(which is calculated within the normalizedDeviceToWindow function)
	vec3fixed	color;
	vec3fixed	texcoord;

	//dangerous stuff here:
#ifdef USE_FRAGMENT_NORMALS
	vec3fixed	normal;
#endif

};

/**
 * structure for vertex data
 */
class Vertex {
public:
	vec4f		coord;		//vertex position  - x,y,z,w homogeneous coordinate system

	swFragment	fr;
//	vec3fixed	color;		//color of the vertex
//	vec3fixed	texcoord;	//texture coordinate at vertex

	Vertex() {}			//default constructor

	// quick variable initialization
	Vertex(
		const vec4f &coord,
		const vec3fixed &color,
		const vec3fixed &texcoord
#ifdef USE_FRAGMENT_NORMALS
		, const vec3fixed &normal
#endif
		)
	{
		this->coord = coord;

		this->fr.color = color;
		this->fr.texcoord = texcoord;
#ifdef USE_FRAGMENT_NORMALS
		this->fr.normal = normal;
#endif
	}

	// converts from clip coordinates to normalized device coordinates
	void clipToNormalizedDevice() {
		coord.x /= coord.w;
		coord.y /= coord.w;
		coord.z /= coord.w;
	}

	/**
	 * converts from normalized device coordinates to window coordinates
	 * depends upon:
	 *  centerX, centerY, viewportRect, swZNear, swZFar
	 */
	void normalizedDeviceToWindow();
};

#define SW_VERTEX_STACK_SIZE	32

//me working around header abstraction issue
extern Vertex swVertexStack[SW_VERTEX_STACK_SIZE];
extern int swVertexStackSize;
