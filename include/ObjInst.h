#pragma once

#include <string>

//#include "texture.h"
#include "vec.h"
#include "ObjFile.h"

/**
 * our object instance's material settings:
 */
class Material {
public:
	Material() {}
	Material(const vec4f &amb, const vec4f &diff, const vec4f &spec, float shine) {
		set(amb,diff,spec,shine);
	}

	void set(const vec4f &amb, const vec4f &diff, const vec4f &spec, float shine) {
		this->amb = amb;
		this->diff = diff;
		this->spec = spec;
		this->shine = shine;	
	}	

	vec4f	amb;
	vec4f	diff;
	vec4f	spec;

	/**
	 * "love is in the water,
	 *  love is in the air.
	 *  teach me where to go,
	 *  tell me will love be there..."
	 *		--Collective Soul, "Shine".
	 */
	float	shine;

	//todo - vec3f emm

	void setup(int face);
};

/**
 * our object instance:
 */
class ObjInst {

	//instance model
	ObjFile *model;

	void	reset();

	std::string title;

public:

	//replacement for color/specular soon...
	Material mat;

	//orthonormal basis w/transform
	basis_t basis;

	//non-uniform scale components, relative to current orientation
	vec3f scale;

	ObjInst() { reset(); }
	~ObjInst() { unload(); }

	/**
	 * unload all data
	 */
	void unload();

	/**
	 * render
	 */
	void render();

	//interface
	ObjFile *getModel() const { return model; }
	bool setModel(ObjFile *model);

	const std::string& getTitle() { return title; }
};
