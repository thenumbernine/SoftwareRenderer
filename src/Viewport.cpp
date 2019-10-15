#include <iostream>
#include "Viewport.h"
#include "Main.h"
#include "Texture.h"
#include "sw.h"

void Viewport::reset() {
	x = y = width = height = 0;
	view = NULL;
}

void Viewport::setupViewport() {
	swViewport(x,y,width,height);

	//TODO - should this be optional too?
	//it seems like it'd get in the way if the scene wanted to use the scissor test
//	if (useScissor) {
//		swScissor(x,y,width,height);
//		swEnable(GL_SCISSOR_TEST);
//	}

	//TODO - should clearing various buffers be optional?
	swClear(SW_COLOR_BUFFER_BIT | SW_DEPTH_BUFFER_BIT /*| SW_STENCIL_BUFFER_BIT*/);

//	if (useBorder) {
		swMatrixMode(SW_PROJECTION);
		swLoadIdentity();
		swOrtho(0,width,0,height,-1,1);
		swMatrixMode(SW_MODELVIEW);
		swLoadIdentity();

		Texture::unbind();
		swColor3f(0,1,0);	//red is getting tossed, and green getting mapped to red
		swBegin(SW_LINES);
		swVertex2f(0,0);
		swVertex2f(width-1,0);
		swVertex2f(width-1,0);
		swVertex2f(width-1,height-1);
		swVertex2f(width-1,height-1);
		swVertex2f(0,height-1);
		swVertex2f(0,height-1);
		swVertex2f(0,0);
		swEnd();
//	}

}

void Viewport::shutdownViewport() {
//	if (useScissor) swDisable(GL_SCISSOR_TEST);
}

void Viewport::render() {
//std::cout << "viewport width " << width << std::endl;
//std::cout << "viewport height " << height << std::endl;
//std::cout << "aspect ratio " << getAspectRatio() << std::endl;
//exit(1);

	setupViewport();

	if (view) {
		//TODO - should frustums be stored per-viewport or per-view?
		//I believe per-viewport since frustums depend on view states depend upon viewport states
		//if it were stored per-view then views targetted by multiple viewports could potentially
		//provide miscalculated frustum data to one of the many viewports (whichever was not calculated last)

		//TODO - only calculate this when it needs to be updated
		//but to do such a thing would require a state comparision of the viewport's view
		//or for the view to keep track of its own state changes
		//and communicate them to the viewport
		view->calculateFrustum(&frustum, getAspectRatio());

		view->render(this);
	}

	shutdownViewport();
}

vec3f Viewport::getPixelPos(int x, int y) const {
	x -= this->x;
	y -= this->y;
	float fx = (float)x / (float)width;
	float fy = (float)y / (float)height;
	float ifx = 1.f - fx;
	float ify = 1.f - fy;
	//screen <x,y> maps to modelview <0,-x,y>
	return (
		frustum.vtx[FRUSTUM_VTX_NNN] *  fx * ify +
		frustum.vtx[FRUSTUM_VTX_NPN] * ifx * ify +
		frustum.vtx[FRUSTUM_VTX_NNP] *  fx *  fy +
		frustum.vtx[FRUSTUM_VTX_NPP] * ifx *  fy);
}
