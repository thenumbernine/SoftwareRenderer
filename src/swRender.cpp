#include <assert.h>
#include <memory.h>
#include <iostream>

#include "sw.h"
#include "swMain.h"
#include "swRender.h"
#include "swState.h"
#include "swPoly.h"
#include "swTexture.h"
#include "vec.h"
#include "Main.h"

typedef struct edge_s {
	
	//index of vertex stack for the min and max y value
	//within the scanline renderer it looks like 'max' is all that is ever used
	int min, max;
	
	//current 'x' value for this edge
	float x, x_step;

	//inverse depth value at each pixel.  used for perspective texture coords
	float invz, invz_step;
	
	//current fragment value for this edge
	swFragment fr;

	//step value to linearly interpolate across the edge
	swFragment fr_step;

	//whether the edge is on left or right edge
//	bool left;

	//pointer for the linked-list
	struct edge_s *prev, *next;

} edge_t;

//the maximum # of edges per polygon
//this array exists to prevent heap allocation during rendering
static edge_t edgeBuffer[32];
static int edgeBufferSize = 0;

//the array of pointers mapping scanlines to entries in the edge buffer
static edge_t **edgeTable = NULL;
static int edgeTableSize = -1;

//static edge_t aet[32];
//static int aetSize = 0;
static edge_t *aetFirst = NULL, *aetLast = NULL;

static inline void drawPolygons_buildEdgeTable(
	int &ymin,
	int &ymax)
{

	ymin = 0x7FFFFFFF;	//initially as large as possible, so the next comparison will size it down
	ymax = -0x7FFFFFFF;	//as small as possible

//std::cout << "swVertexStackSize = " << swVertexStackSize << std::endl;

	//fill the edge table, ymin-sorted *HERE*
	for (int i = 0; i < swVertexStackSize; i++) {
		int ii = (i + 1) % swVertexStackSize;

		//sort the two vertices
		int min, max;
		if (swVertexStack[i].coord.y < swVertexStack[ii].coord.y) {
			min = i;
			max = ii;
		} else {
			min = ii;
			max = i;
		}

		Vertex *vtxMin = &(swVertexStack[min]);
		Vertex *vtxMax = &(swVertexStack[max]);

//std::cout << "vtx min = " << *vtxMin << ", max = " << *vtxMax << std::endl;

//		//store the minimum y value:
		int edge_ymin = (int)vtxMin->coord.y;
		int edge_ymax = (int)vtxMax->coord.y;

//std::cout << "edge ymin = " << edge_ymin << ", ymax = " << edge_ymax << std::endl;

		//if the horizontal line we're targetting is below the lowest screen bar then forget it
		if (edge_ymin >= edgeTableSize) continue;
		//if it ends before the screen begins then forget it
		if (edge_ymax < 0) continue;

		//calculate the slope across, from min to max
		float dy = vtxMax->coord.y - vtxMin->coord.y;
		float one_over_dy = 1.f / dy;

		//record our new destination 'y' value
		int dest_y = edge_ymin;
		float destFrac = 0.f;
		//if it lies above the screen then interpolate down into the screen
		if (dest_y < 0) {
			destFrac = (0 - vtxMin->coord.y) * one_over_dy;
			dest_y = 0;
		}

		//stretch our y min/max values to encompass the new y value
		if (dest_y < ymin) ymin = dest_y;	//use dest_y after it has been adjusted
		if (edge_ymax > ymax) ymax = edge_ymax;

		//TODO - interpolate Y values as we calculate them - depending upon srcFrac and destFrac

		//grab an empty edge off the edge table list
		assert(edgeBufferSize < (int)numberof(edgeBuffer));
		edge_t *e = edgeBuffer + edgeBufferSize;
		edgeBufferSize++;

		//calculate the edge minimum values
		e->fr = vtxMin->fr;

		//fill in our edge structure
		e->min = min;
		e->max = max;

		//calculate the edge initial values (interpolate to the screen if we must)
		if (!destFrac) {
			e->x = vtxMin->coord.x;
			e->invz = vtxMin->coord.z;
		} else {
			float srcFrac = 1.f - destFrac;

			e->x = vtxMin->coord.x * srcFrac + vtxMax->coord.x * destFrac;
			e->invz = vtxMin->coord.z * srcFrac + vtxMax->coord.z * destFrac;
			
			e->fr.color.x = e->fr.color.x * srcFrac + vtxMax->fr.color.x * destFrac;
			e->fr.color.y = e->fr.color.y * srcFrac + vtxMax->fr.color.y * destFrac;
			e->fr.color.z = e->fr.color.z * srcFrac + vtxMax->fr.color.z * destFrac;
			e->fr.texcoord.x = e->fr.texcoord.x * srcFrac + vtxMax->fr.texcoord.x * destFrac;
			e->fr.texcoord.y = e->fr.texcoord.y * srcFrac + vtxMax->fr.texcoord.y * destFrac;
			e->fr.texcoord.z = e->fr.texcoord.z * srcFrac + vtxMax->fr.texcoord.z * destFrac;
#ifdef USE_FRAGMENT_NORMALS
			//only optionally - for lighting and what not
			e->fr.normal.x = e->fr.normal.x * srcFrac + vtxMax->fr.normal.x * destFrac;
			e->fr.normal.y = e->fr.normal.y * srcFrac + vtxMax->fr.normal.y * destFrac;
			e->fr.normal.z = e->fr.normal.z * srcFrac + vtxMax->fr.normal.z * destFrac;
#endif
		}

		//calculate our edge step values

		e->x_step = (vtxMax->coord.x - e->x) * one_over_dy;
		e->invz_step = (vtxMax->coord.z - e->invz) * one_over_dy;

		e->fr_step.color.x  = (vtxMax->fr.color.x  - e->fr.color.x) * one_over_dy;
		e->fr_step.color.y  = (vtxMax->fr.color.y  - e->fr.color.y) * one_over_dy;
		e->fr_step.color.z  = (vtxMax->fr.color.z  - e->fr.color.z) * one_over_dy;
		e->fr_step.texcoord.x = (vtxMax->fr.texcoord.x - e->fr.texcoord.x) * one_over_dy;
		e->fr_step.texcoord.y = (vtxMax->fr.texcoord.y - e->fr.texcoord.y) * one_over_dy;
		e->fr_step.texcoord.z = (vtxMax->fr.texcoord.z - e->fr.texcoord.z) * one_over_dy;
#ifdef USE_FRAGMENT_NORMALS
		//only optionally - for lighting and what not
		e->fr_step.normal.x = (vtxMax->fr.normal.x - e->fr.normal.x) * one_over_dy;
		e->fr_step.normal.y = (vtxMax->fr.normal.y - e->fr.normal.y) * one_over_dy;
		e->fr_step.normal.z = (vtxMax->fr.normal.z - e->fr.normal.z) * one_over_dy;
#endif

//		e->left = ...

		//insert it in at the edge table
		if (!edgeTable[dest_y]) {
			edgeTable[dest_y] = e;
			e->prev = e->next = NULL;
		} else {
			edge_t *src = edgeTable[dest_y];
			if (src) src->prev = e;
			e->next = src;
			edgeTable[dest_y] = e;
			e->prev = NULL;
		}
	}
}

static inline void drawPolygon_updateEdgeTable(const int y) {
	edge_t *next;

	//for all edges that start at our current horizontal line, add them
	if (edgeTable[y]) {
		
		for (edge_t *e = edgeTable[y]; e; e = next) {
			next = e->next;

			if (!aetFirst) {
				aetFirst = aetLast = e;
				e->prev = e->next = NULL;
			} else {
				//sort as we insert edges into the AET
				//notice - in doing so we lose support for correctly rendering non-planar polygons
				edge_t *src = aetFirst;
				for (; src; src = src->next) {
					if (src->x + 0.1f * src->x_step > e->x + 0.1f * e->x_step) break;
				}
				if (src) {
					if (src->prev) { //intermediate
						e->prev = src->prev;
						e->next = src;
						src->prev->next = e;
						src->prev = e;
					} else {	//we're at the beginning
						e->prev = NULL;
						e->next = aetFirst;
						aetFirst->prev = e;
						aetFirst = e;
					}
				} else {	//we're at the end
					e->prev = aetLast;
					e->next = NULL;
					aetLast->next = e;
					aetLast = e;
				}
			}
		}
		edgeTable[y] = NULL;
	}

	//for all edges that end at our current horizontal line, remove them
	for (edge_t *e = aetFirst; e; e = next) {
		next = e->next;
		if ((int)swVertexStack[e->max].coord.y == y) {
			if (e->prev) e->prev->next = e->next;
			if (e->next) e->next->prev = e->prev;
			if (e == aetFirst) aetFirst = aetFirst->next;
			if (e == aetLast) aetLast = aetLast->prev;
		}
	}
}

/*

pathway states:
	color
	texture
		- nearest / linear
		- 1D, 2D, Cubemap, 3D
	texture + color
		- nearest / linear
		- 1D, 2D, Cubemap, 3D
*/
static inline void drawPolygon_calcColor(
	const SWTexture *tex,
	const vec3fixed &invc,
	const vec3fixed &invtc,
#ifdef USE_FRAGMENT_NORMALS
	const vec3fixed &invn,	//normals are also interpolated in inverse-z space
#endif
	const float invz,
	vec3fixed &c)
{
	if (tex) {

		fixed_t zNot = (float)FIXED_FRACTION_MASK / invz;
		vec3fixed tc;
		tc.x = fixedMul(invtc.x, zNot);
		tc.y = fixedMul(invtc.y, zNot);
		tc.z = fixedMul(invtc.z, zNot);

		int texel[3];
		tex->getTexCoordColor(tc, texel);

		//todo - test results based upon current texture environment parameter
		//our current implementation is for SW_MODULATE
		//SW_REPLACE would skip out the invc calculations

		c.x = fixedMulLo(invc.x, zNot);
		c.x *= (int)texel[0];
		c.x >>= (FIXED_FRACTION_BITS - 8);	//plus eight to adjust for the texel color range
		c.x &= 0xFF0000;

		c.y = fixedMulLo(invc.y, zNot);
		c.y *= (int)texel[1];
		c.y >>= (FIXED_FRACTION_BITS);	//plus eight to adjust for the texel color range
		c.y &= 0xFF00;

		c.z = fixedMulLo(invc.z, zNot);
		c.z *= (int)texel[2];
		c.z >>= (FIXED_FRACTION_BITS + 8);	//plus eight to adjust for the texel color range
		c.z &= 0xFF;

	} else {

		fixed_t zNot = (float)FIXED_FRACTION_MASK / invz;

		c.x = fixedMulLo(invc.x, zNot);
		c.x >>= (FIXED_FRACTION_BITS - 16);
		c.x &= 0xFF0000;
		
		c.y = fixedMulLo(invc.y, zNot);
		c.y >>= FIXED_FRACTION_BITS - 8;
		c.y &= 0xFF00;

		c.z = fixedMulLo(invc.z, zNot);
		c.z >>= FIXED_FRACTION_BITS;
		c.z &= 0xFF;
	}

#ifdef USE_FRAGMENT_NORMALS

	//currently the normals are still in modelview space.
	//oh shucks.

	//how to do proper bumpmapping anyways:
	//	setup:
	//1) greyscale the texture, then calc dx/dy/1 => r,g,b of the tangent space texture
	//2) create a basis for every triangle
	//	when rendering:
	//1) rotate the color of the tangent space texture into the triangle's basis.
	//2) dot3 this color with the vector to the light
	//
	//but this gives you per-triangle accurate light vectors.  what about per pixel?
	//it will be per pixel if the light vector is a lookup from a cubemap offset at the light's position
	//
	//there you have it, folks...
	//
	//but do we really have to store a unique matrix for each triangle?!
	//work arounds...
	//
	//im guessing the bump map shaders calculate the triangle basii on the fly
	//after all, they're recomputing the whole model every frame anyways...
	//how would it do that?
	//for every triangle rendered...
	//	compute the basis,
	//  then rotate the light source by the inverse basis (im still thinking ...)
	//		how do you get the light vector?
	//			bind the light texture to the normalized cubemap
	//			rotate the lightpos by the inverse basis
	//			then inverse translate the texture coordintse by the rotated light pos
	//			likewise, rotate all light vectors by the inverse basis... messy overall...
	//	and dot3 it with the surface tangent space map

	{
		fixed_t zNot = (float)FIXED_FRACTION_MASK / invz;	//this is recalc'd here - fixme
		vec3fixed n(
			fixedMul(invn.x, zNot),
			fixedMul(invn.y, zNot),
			fixedMul(invn.z, zNot));

		//now dot product with the unit vector to a phony light source
		//clamp the coeff to [0,1]
		//and scale our color accordingly

		//for now, phony light at zero...
		//or just assume z is our view vector...
		//so just use the z component...
		int l = n.x;
		if (l < 0) l = 0;
		c.x >>= 16;
//		c.x &= 0xFF;
		c.x *= l;
//		c.x >>= FIXED_FRACTION_BITS;
//		c.x &= 0xFF;
//		c.x <<= 16;

		c.y >>= 8;
//		c.y &= 0xFF;
		c.y *= l;
		c.y >>= (FIXED_FRACTION_BITS - 8);
//		c.y &= 0xFF;
//		c.y <<= 8;

//		c.z &= 0xFF;
		c.z *= l;
		c.z >>= FIXED_FRACTION_BITS;
//		c.z &= 0xFF;
	}
#endif
}

void swDrawPolygons() {
	if (!swBMPBuffer) return;

	//get the # of vertices
	int size = swVertexStackSize;

	//degenerate polygon - dont render it
	if (size < 3) return;

	//store here whether or not we're going to be using depth testing
	int useDepthTest = swDepthBuffer && getSWStateBit(SW_BIT_DEPTH_TEST);
	int useBlend = getSWStateBit(SW_BIT_BLEND);

	if (edgeTableSize != swBufferHeight) {
		edgeTableSize = swBufferHeight;
		if (edgeTable) delete[] edgeTable;
		edgeTable = (edgeTableSize > 0) ? new edge_t*[edgeTableSize] : NULL;
		if (edgeTable) {
			memset(edgeTable, 0, sizeof(edge_t *) * edgeTableSize);
		}
	}
	if (!edgeTable) return;

	//do some clipping beforehand?
	//normalize device coordinates
	//somewhere in here, Z gets inverted
	for (int i = 0; i < size; i++) {
		Vertex *vtx = &(swVertexStack[i]);
		vtx->normalizedDeviceToWindow();

		//now scale all vertex elements (color, texcoord, etc)
		//by the newly-calculated inverse-z value
		vtx->fr.color.x *= vtx->coord.z;
		vtx->fr.color.y *= vtx->coord.z;
		vtx->fr.color.z *= vtx->coord.z;
		vtx->fr.texcoord.x *= vtx->coord.z;
		vtx->fr.texcoord.y *= vtx->coord.z;
		vtx->fr.texcoord.z *= vtx->coord.z;
#ifdef USE_FRAGMENT_NORMALS
		vtx->fr.normal.x *= vtx->coord.z;
		vtx->fr.normal.y *= vtx->coord.z;
		vtx->fr.normal.z *= vtx->coord.z;
#endif
	}

	//check out our texture
	SWTexture *tex = swGetSelectedTexture();

	if (tex) {
		tex->initFilter();
	}

	//init the edge table
	edgeBufferSize = 0;

#if 0	//enable this for backface culling
	//using edges 0->1->2, calculate the normal z component in screen space
	if ((swVertexStack[1].coord.x - swVertexStack[0].coord.x) * 
		(swVertexStack[2].coord.y - swVertexStack[1].coord.y) -
		(swVertexStack[2].coord.x - swVertexStack[1].coord.x) *
		(swVertexStack[1].coord.y - swVertexStack[0].coord.y) > 0) return;
#endif

	//keep track of the minimum y value
	int ymin, ymax;

	drawPolygons_buildEdgeTable(ymin, ymax);

	int y = ymin;
	int rowIndex = y * swBufferWidth;
	uint32_t *rowBuffer = swBMPBuffer + rowIndex;
	float *rowDepthBuffer = swDepthBuffer + rowIndex;

	//init the active edge table
	aetFirst = aetLast = NULL;

	//ensure that we don't pass beneath the bottom of the screen
	if (ymax >= edgeTableSize) {
		ymax = edgeTableSize - 1;
	}
//std::cout << "y = " << y << ", ymax = " << ymax << std::endl;
	while (y <= ymax) {

		drawPolygon_updateEdgeTable(y);

		/*
		texture mapping (if i got this right)
		
		for each vertex
			let z' = 1/z
			let U' = U * z'
		for each hline
			lerp U' and z'
			for each pixel
				let U = U' * 1/z'
		*/

		vec3fixed finalColor;

		//fill pixels
		if (aetFirst && aetFirst->next) {
			//get all our 'x' values, start with the lowest, cycle across 'til we skip another - then flip our 'draw me' bit

			edge_t *ea = aetFirst;
			edge_t *eb = ea->next;

			//test left hand side of the screen scanline
			//first - cycle through the edge list until 'eb' is past the zero line
			while ((int)eb->x < 0) {
				ea = ea->next;
				eb = eb->next;
				if (!eb) break;
			}

			//if we went through the whole list without passing x=0 then skip this scanline
			if (eb) {

				int x = ea->x;

				//next see where within our current edge we are to start
				float destFrac = 0.f;
				if (ea->x < 0) {
					destFrac = (0.f - ea->x) / (eb->x - ea->x);
					x = 0;
				}

				uint32_t *colBuffer = rowBuffer + x;
				float *colDepthBuffer = rowDepthBuffer + x;

				float one_over_dx = 1.f / (float)(eb->x - ea->x);
				float dinvz = (eb->invz - ea->invz) * one_over_dx;

				swFragment fr_step;
				fr_step.color.x = (eb->fr.color.x - ea->fr.color.x) * one_over_dx;
				fr_step.color.y = (eb->fr.color.y - ea->fr.color.y) * one_over_dx;
				fr_step.color.z = (eb->fr.color.z - ea->fr.color.z) * one_over_dx;
				fr_step.texcoord.x = (eb->fr.texcoord.x - ea->fr.texcoord.x) * one_over_dx;
				fr_step.texcoord.y = (eb->fr.texcoord.y - ea->fr.texcoord.y) * one_over_dx;
				fr_step.texcoord.z = (eb->fr.texcoord.z - ea->fr.texcoord.z) * one_over_dx;
#ifdef USE_FRAGMENT_NORMALS
				fr_step.normal.x = (eb->fr.normal.x - ea->fr.normal.x) * one_over_dx;
				fr_step.normal.y = (eb->fr.normal.y - ea->fr.normal.y) * one_over_dx;
				fr_step.normal.z = (eb->fr.normal.z - ea->fr.normal.z) * one_over_dx;
#endif

				float invz;
				swFragment fr;

				if (!destFrac) {
					invz = ea->invz;
					fr = ea->fr;
				} else {
					float srcFrac = 1.f - destFrac;
					invz = ea->invz * srcFrac + eb->invz * destFrac;
					fr.color.x = ea->fr.color.x * srcFrac + eb->fr.color.x * destFrac;
					fr.color.y = ea->fr.color.y * srcFrac + eb->fr.color.y * destFrac;
					fr.color.z = ea->fr.color.z * srcFrac + eb->fr.color.z * destFrac;
					fr.texcoord.x = ea->fr.texcoord.x * srcFrac + eb->fr.texcoord.x * destFrac;
					fr.texcoord.y = ea->fr.texcoord.y * srcFrac + eb->fr.texcoord.y * destFrac;
					fr.texcoord.z = ea->fr.texcoord.z * srcFrac + eb->fr.texcoord.z * destFrac;
#ifdef USE_FRAGMENT_NORMALS
					fr.normal.x = ea->fr.normal.x * srcFrac + eb->fr.normal.x * destFrac;
					fr.normal.y = ea->fr.normal.y * srcFrac + eb->fr.normal.y * destFrac;
					fr.normal.z = ea->fr.normal.z * srcFrac + eb->fr.normal.z * destFrac;
#endif
				}

				while (eb) {

					//test right hand side of the screen scanline
					if (x >= swBufferWidth) break;

					if (x >= (int)eb->x) {
						ea = ea->next;
						eb = eb->next;
						if (!eb) break;

						//recalc dc
						one_over_dx = 1.f / (float)(eb->x - ea->x);
						dinvz = (eb->invz - ea->invz) * one_over_dx;
						fr_step.color.x = (eb->fr.color.x - ea->fr.color.x) * one_over_dx;
						fr_step.color.y = (eb->fr.color.y - ea->fr.color.y) * one_over_dx;
						fr_step.color.z = (eb->fr.color.z - ea->fr.color.z) * one_over_dx;
						fr_step.texcoord.x = (eb->fr.texcoord.x - ea->fr.texcoord.x) * one_over_dx;
						fr_step.texcoord.y = (eb->fr.texcoord.y - ea->fr.texcoord.y) * one_over_dx;
						fr_step.texcoord.z = (eb->fr.texcoord.z - ea->fr.texcoord.z) * one_over_dx;
#ifdef USE_FRAGMENT_NORMALS
						fr_step.normal.x = (eb->fr.normal.x - ea->fr.normal.x) * one_over_dx;
						fr_step.normal.y = (eb->fr.normal.y - ea->fr.normal.y) * one_over_dx;
						fr_step.normal.z = (eb->fr.normal.z - ea->fr.normal.z) * one_over_dx;
#endif

						//assert color == aet[curAet].color
						//but just in case...
						invz = ea->invz;
						fr = ea->fr;

						continue;	//reloop & recalc
					}

					assert(x >= 0 && x < swBufferWidth);
					{

						//if the current depth is greater than written depth
						// <-> if the inverse current depth is less than the written inverse depth
						if (!useDepthTest || *colDepthBuffer < invz) {

							drawPolygon_calcColor(tex, fr.color, fr.texcoord, 
#ifdef USE_FRAGMENT_NORMALS
								fr.normal,
#endif
								invz, finalColor);

							//currently additive blend only
							if (useBlend) {
								finalColor.x += *colBuffer & 0xFF0000;
								if (finalColor.x & 0xFF000000) {
									finalColor.x = 0xFF0000;
								} else {
									finalColor.x &= 0xFF0000;
								}

								finalColor.y += *colBuffer & 0xFF00;
								if (finalColor.y & 0xFFFF0000) {
									finalColor.y = 0xFF00;
								} else {
									finalColor.y &= 0xFF00;
								}

								finalColor.z += *colBuffer & 0xFF;
								if (finalColor.z & 0xFFFFFF00) {
									finalColor.z = 0xFF;
								} else {
									finalColor.z &= 0xFF;
								}
							}

							//no destination alpha writes, even though our buffer supports it.
							*colBuffer = finalColor.x | finalColor.y | finalColor.z;
							//write out the depth buffer
							*colDepthBuffer = invz;
						}
					}

					x++;
					colDepthBuffer++;
					colBuffer++;

					invz += dinvz;
					fr.color.x += fr_step.color.x;
					fr.color.y += fr_step.color.y;
					fr.color.z += fr_step.color.z;
					fr.texcoord.x += fr_step.texcoord.x;
					fr.texcoord.y += fr_step.texcoord.y;
					fr.texcoord.z += fr_step.texcoord.z;
#ifdef USE_FRAGMENT_NORMALS
					fr.normal.x += fr_step.normal.x;
					fr.normal.y += fr_step.normal.y;
					fr.normal.z += fr_step.normal.z;
#endif
				}
			}
		}

		//increment scanline #
		y++;

		rowBuffer += swBufferWidth;
		rowDepthBuffer += swBufferWidth;

//		//update 'x's for each edge
		for (edge_t *e = aetFirst; e; e = e->next) {
			e->x += e->x_step;
			e->invz += e->invz_step;
			e->fr.color.x += e->fr_step.color.x;
			e->fr.color.y += e->fr_step.color.y;
			e->fr.color.z += e->fr_step.color.z;
			e->fr.texcoord.x += e->fr_step.texcoord.x;
			e->fr.texcoord.y += e->fr_step.texcoord.y;
			e->fr.texcoord.z += e->fr_step.texcoord.z;
#ifdef USE_FRAGMENT_NORMALS
			e->fr.normal.x += e->fr_step.normal.x;
			e->fr.normal.y += e->fr_step.normal.y;
			e->fr.normal.z += e->fr_step.normal.z;
#endif
		}
	}
}

void swDrawLine(Vertex v0, Vertex v1) {

#if 1	//enable to enable all line rendering

	//do a midpoint rule thingy

	if (!swBMPBuffer) return;

	v0.normalizedDeviceToWindow();
	v1.normalizedDeviceToWindow();

	const Vertex *vmin = &v0, *vmax = &v1;
	if (vmin->coord.y > vmax->coord.y) {
		const Vertex *temp = vmin;
		vmin = vmax;
		vmax = temp;
	}

	int xmin = (int)vmin->coord.x;
	int ymin = (int)vmin->coord.y;
	int xmax = (int)vmax->coord.x;
	int ymax = (int)vmax->coord.y;

#if 0 //this is the left-edge scan from the handout:

	int x = xmin;
	int num = xmax - xmin;
	int dy = ymax - ymin;
	int denom = dy;
	int inc = denom;

	if (!dy) return;

	vec4f color = vmin->fr.color;
	vec4f dc = (vmax->fr.color - vmin->fr.color) / (float)(ymax - ymin + 1);

	for (int y = ymin; y <= ymax; y++) {

		//sw reference:
		swWritePixel(x,y,
			(int)(255.f * color.x),
			(int)(255.f * color.y),
			(int)(255.f * color.z));

		//what a mess...
		inc += num;
		if (num > 0) {
			while (inc > denom) {
				x++;
				inc -= denom;

				//sw reference:
				swWritePixel(x,y,
					(int)(255.f * color.x),
					(int)(255.f * color.y),
					(int)(255.f * color.z));

				if (x >= xmax) break;

			}
		} else {
			while (inc < -denom) {
				x--;
				inc += denom;

				//sw reference:
				swWritePixel(x,y,
					(int)(255.f * color.x),
					(int)(255.f * color.y),
					(int)(255.f * color.z));

				if (x <= xmin) break;
			}
		}

		color += dc;
	}
#else	//slow but steady

	int useDepthTest = swDepthBuffer && getSWStateBit(SW_BIT_DEPTH_TEST);

	int idx = xmax - xmin + 1;
	int idy = ymax - ymin + 1;

	int adx = idx < 0 ? -idx : idx;	//|dx|
	int ady = idy < 0 ? -idy : idy;	//|dy|

	int dt = adx > ady ? adx : ady;		//max(dx,dy)
	float invdt = 1.f / (float)dt;
	
	vec3f v = *vmin->coord.vp();
	vec3fixed c = vmin->fr.color;

	vec3f dv = (*vmax->coord.vp() - *vmin->coord.vp()) * invdt;
	vec3fixed dc = (vmax->fr.color - vmin->fr.color) * invdt;

	for (int i = 0; i < dt; i++) {

		int ix = (int)v.x;
		int iy = (int)v.y;
		if (ix >= 0 && ix < swBufferWidth &&
			iy >= 0 && iy < swBufferHeight)
		{
			int index = ix + iy * swBufferWidth;
			uint32_t *colBMPBuffer = swBMPBuffer + index;

			float *colDepthBuffer = swDepthBuffer + index;

			if (!useDepthTest || *colDepthBuffer >= v.z) {	//if the current depth is greater than written depth <-> if the inverse current depth is less than the written inverse depth

				*colBMPBuffer =	((c.z >> 16) & 0xFF) |
								(((c.y >> 8) & 0xFF00) << 8) |
								((c.x & 0xFF0000) << 16);

				*colDepthBuffer = v.z;
			}
		}
		v += dv;
		c += dc;
	}
	
#endif

#endif
}

void swDrawLines() {
	for (int i = 0; i < swVertexStackSize - 1; i+=2) {
		swDrawLine(swVertexStack[i], swVertexStack[i+1]);
	}
}

void swDrawLineLoop() {
	int i = 0;
	for (; i < swVertexStackSize-1; i++) {
		swDrawLine(swVertexStack[i], swVertexStack[i+1]);
	}
	swDrawLine(swVertexStack[i], swVertexStack[0]);
}

void swDrawPoints() {
	for (int i = 0; i < swVertexStackSize-1; i++) {
		//swDrawPoint(swVertexStack[i]);
	}
}
