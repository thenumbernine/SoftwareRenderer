#pragma once

#include "vec.h"

/**
 * The location of the frustum vertices in projection space
 * N = negative, P = positive
 * The letters indicate the direction of the vertex in projection space
 * assuming the View's default transform has taken place
 * ('x' forward, 'y' left, and 'z' up)
 * i.e., NNN corresponds to the near right lower coord
 * PPP corresponds to the far left upper coord
 */
enum {
	FRUSTUM_VTX_NNN,
	FRUSTUM_VTX_PNN,
	FRUSTUM_VTX_NPN,
	FRUSTUM_VTX_PPN,
	FRUSTUM_VTX_NNP,
	FRUSTUM_VTX_PNP,
	FRUSTUM_VTX_NPP,
	FRUSTUM_VTX_PPP,
	FRUSTUM_VTX_COUNT
};

/**
 * the six planes of the view frustm
 * N = negative, P = positive
 * X,Y,Z denote which plane, in view space (not OpenGL space).
 * such that XP is the far view plane, XN is the near view plane,
 * YP is the left plane, YN is the right, ZP is top, ZN is bottom.
 * I picked the order to match up with the order of cube map faces.
 * No clue why I chose that really.
 */
enum {
	FRUSTUM_PLANE_XP,
	FRUSTUM_PLANE_XN,
	FRUSTUM_PLANE_YP,
	FRUSTUM_PLANE_YN,
	FRUSTUM_PLANE_ZP,
	FRUSTUM_PLANE_ZN,
	FRUSTUM_PLANE_COUNT
};

/**
 * Contains information on the frustum of a viewing volume.
 * Currently associated with Viewport's 
 */
class Frustum {
public:

	/**
	 * The eight vertices of the bounding volume of the frustum
	 */
	vec3f	vtx[FRUSTUM_VTX_COUNT];

	/**
	 * The six planes of the bounding volume of the frustum
	 */
	plane_t	plane[FRUSTUM_PLANE_COUNT];
};
