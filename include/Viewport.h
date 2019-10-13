#pragma once

#include "View.h"
#include "Scene.h"

/**
 * The Viewport class represents a 2D region on the window surface.
 * Viewports reference View objects which represent where within a Scene
 * to render our display.
 * Viewports also reference Scene objects which describe what data to display,
 * yet in the future the Scene pointer may be moved to the View class.
 * Viewports also hold frustum information of the viewing volume within
 * the scene.
 */
class Viewport {

protected:
	/**
	 * The position and size of the viewport -
	 * used for glViewport() calls
	 */
	int		x, y, width, height;

	/**
	 * The object used to view the Scene through this Viewport
	 */
	View	*view;

	/**
	 * The frustum of the Viewport's View object within the Viewport's Scene
	 */
	Frustum	frustum;

	/**
	 * Clear all variables and restore this object to its default state
	 */
	void	reset();

public:

	/**
	 * Construct an initial Viewport
	 */
	Viewport() {reset();}

	/**
	 * Render this Viewport's View
	 */
	void render();

	/**
	 * glViewport, glClear, and stencil enable
	 * TODO - make stencil optional
	 */
	void setupViewport();

	/**
	 * stencil disable
	 */
	void shutdownViewport();

	/**
	 * Returns 'true' if the 2D point is within the viewport, 'false' otherwise
	 * TODO - give the Viewport a rectangle class object representing <x,y,width,height>
	 * and give the user access to the rectangle to test whether it contains a point.
	 */
	bool containsPoint(int s, int t) {
		return (s >= x) && (s < x + width) && (t >= y) && (t < y + height);
	}

	/**
	 * Returns the position, in modelview space, of the 2D point, in screen coordinates
	 */
	vec3f getPixelPos(int x, int y) const;


	//// inline variable access:


	/**
	 * Sets the 2D region for the viewport
	 */
	void set(int x, int y, int width, int height) {
		this->x = x;
		this->y = y;
		this->width = width;
		this->height = height;
	}

	/**
	 * Returns individual parameters of the viewport's region
	 */
	int getX() { return x; }
	int getY() { return y; }
	int getWidth() const { return width; }
	int getHeight() const { return height; }

	/**
	 * Set or get the view pointer
	 */
	void setView(View *view) { this->view = view; }
	View *getView() const { return view; }

	/**
	 * returns our aspect ratio - currently height/width (FIX THIS)
	 */
	float getAspectRatio() const { return (float)height / (float)width; }

	/**
	 * return a pointer to the frustum object.
	 */
	const Frustum &getFrustum() const { return frustum; }
};
