#pragma once

#include "Frustum.h"
#include "Scene.h"

/**
 * Flag for setupView()
 * Projection recalculation is optional for when rendering without changing projection information.
 * Rotation and translation are separately optional for when a view only needs to partially update
 * its transformation information (such as when rendering skycubes, where translation does not matter)
 */
enum {
	VIEW_FLAG_CLEAR_PROJECTION,		//glLoadIdentity() the projection matrix
	VIEW_FLAG_APPLY_PROJECTION,		//recalculate the projection matrix
	VIEW_FLAG_CLEAR_MODELVIEW,		//glLoadIdentity() the modelview matrix
	VIEW_FLAG_APPLY_ROTATION,		//apply rotations to the modelview matrix
	VIEW_FLAG_APPLY_TRANSLATION,		//apply translations to the modelview matrix
};

/**
 * States of adjustMethod
 */
enum {
	VIEW_ADJUST_NO,							//do not adjust for aspect ratio
	VIEW_ADJUST_HEIGHT,						//use halfWidth to determine field of view - and recalculate the height
	VIEW_ADJUST_WIDTH,						//use halfHeight to determine field of view - and recalculate the width
};

/**
 * The View class represents a virtual camera within a scene.  It is used to set up OpenGL matrices
 * for correctly rendering a scene from the projection informatino and orientation it contains.
 * 
 * TODO - consider separating the projection and modelview functionality into two view classes
 * Projection functionality only requires aspect ratio calculations, and therefore is the deepest
 * dependance of the Viewport's frustum calculations, while modelview functionality is all that
 * affects where in the scene the View is looking.
 *
 * TODO - Move the Scene variable pointer from Viewport to View (the modelview functionality in
 * specific) since the View pos and angle variables relate more to the Scene than the Viewport.
 */
class View {
protected:

	/**
	 * The scene which this Viewport will render
	 */
	Scene	*scene;

	/**
	 * Set this to 'true' for orthogonal viewing, or 'false' for frustum viewing
	 */
	bool	ortho;

	/**
	 * The desired half width & half height of the screen, in modelview space.
	 * note: in frustum mode, 2.f * atan(halfDist / znear) = the scene FOV
	 *	for halfDist being halfWidth when adjustMethod == VIEW_ADJUST_HEIGHT
	 *					or halfHeight when adjustMethod == VIEW_ADJUST_WIDTH
	 */
	float	halfWidth, halfHeight;	

	/**
	 * How to adjust the aspect ratio for non-square display dimensions.
	 * References one of the VIEW_ADJUST_*** values (see their definition above for details)
	 */
	int		adjustMethod;

	/**
	 * Near and far clip plane distances, in modelview space.
	 */
	float	znear, zfar;

	/** position / orientation of view */
	basis_t	basis;

	/**
	 * Clear all variables and restore this object to its default state
	 */
	void reset();

public:

	/**
	 * Constructs a View object containing all default values
	 */
	View()	{reset();}

	/**
	 * Render this View's Scene
	 */
	void render(const Viewport *vp);

	/**
	 * Sets up the correct projection and modelview matrices
	 */
	void setupView(int flags, float aspectRatio) const;

	/**
	 * Called by setupView() to reset the projection matrix
	 */
	void setProjection(int flags, float aspectRatio) const;

	/**
	 * Called by setupView() to reset the modelview matrix
	 */
	void setModelview(int flags) const;

	/**
	 * Called by setModelview() to apply the correct rotation to the scene
	 * shell for glRotatef with our inverse angle
	 */
	void rotateInverse() const;

	/**
	 * external function used for forward tranforming rotations
	 */
	void rotate() const;

	/**
	 * Called by setModelview() to apply the correct translation to the scene
	 */
	void translateInverse() const;

	/**
	 * This method corrects OpenGL modelview space by putting 'x' forward, 'y' left, and 'z' up.
	 * It is typically only used when correcting for the view angle with cube mapping
	 * or some other inverse modelview coordinate transform.
	 */
	void correctAngle() const;

	/**
	 * This method inversely corrects for OpenGL modelview space by putting
	 * 'x' forward, 'y' left, and 'z' up
	 * This is used to initialize the view transformation, and is called by rotateScene()
	 */
	void correctAngleInverse() const;

	/**
	 * This method calculates the frustum data for this View according to the specified aspect ratio.
	 */
	void calculateFrustum(Frustum *frustum, float aspectRatio);


	//// inline variable access:


	/**
	 * This method sets all the specified variables for the View object
	 */
	void set(
		Scene	*scene,
		bool	ortho,
		float	halfWidth,
		float	halfHeight,
		int		adjustMethod,
		float	znear,
		float	zfar,
		const	vec3f	&pos,
		const	vec4f &angle)
	{
		this->scene = scene;
		this->ortho = ortho;
		this->halfWidth = halfWidth;
		this->halfHeight = halfHeight;
		this->adjustMethod = adjustMethod;
		this->znear = znear;
		this->zfar = zfar;
		this->basis.t = pos;
		this->basis.r = angle;
	}

	/**
	 * Set or get the scene pointer
	 */
	void setScene(Scene *scene) { this->scene = scene; }
	Scene *getScene() { return scene; }

	/**
	 * Set or get the position coordinate
	 */
	const vec3f &getPos() const {		return basis.t;	}
	void setPos(const vec3f &v) {			basis.t = v;	}

	/**
	 * Set or get the angle quaternion
	 */
	const vec4f &getAngle() const {		return basis.r;	}
	void setAngle(const vec4f &q) {			basis.r = q;	}

	/**
	 * Set or get the whole basis
	 */
	const basis_t &getBasis() const {	return basis;	}
	void setBasis(const basis_t &b) {		basis = b;		}

	/**
	 * Returns whether the scene uses orthogonal projection or not
	 */
	bool getOrtho() const {
		return ortho;
	}

	/**
	 * Set or get the half-width of the View object
	 */
	float getHalfWidth() const {		return halfWidth;}
	void setHalfWidth(float hw) {			halfWidth = hw;	}

};
