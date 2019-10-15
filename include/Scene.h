#pragma once

class Viewport;
class View;

/**
 * The virtual class which all Views reference for rendering
 */
class Scene {
public:

	virtual bool init() = 0;
	virtual void shutdown() = 0;

	virtual void update() = 0;

	/**
	 * Renders a scene through the specified viewports and views
	 */
	virtual void render(const Viewport *viewport, const View *view) = 0;
};
