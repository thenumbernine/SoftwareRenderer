#pragma once

class Viewport;
class View;

/**
 * The virtual class which all Views reference for rendering
 */
struct Scene {
	virtual void update() = 0;

	/**
	 * Renders a scene through the specified viewports and views
	 */
	virtual void render(const Viewport *viewport, const View *view) = 0;
	
	virtual void updateGUI() = 0;
};
