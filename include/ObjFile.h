#pragma once

#include <vector>

#include "vec.h"
#include "Main.h"
//#include "texture.h"

/**
 * the data associated with each index
 */
typedef struct {

	/** 
	 * reference to the vertex array
	 */
	int v;

	/** 
	 * reference to the texture coordinate array
	 */
	int t;

	/** 
	 * reference to the normal array
	 */
	int n;

} triIndex_t;

/**
 * information associated with each triangle
 */
typedef struct {

	/**
	 * three vertex references
	 */
	triIndex_t	p[3];

	/** 
	 * surface plane equation
	 */
	plane_t		plane;

} tri_t;

/**
 * triangle connectivity information
 */
typedef struct {
	int t[2];		//two triangles between the edge
	int v[2];		//the first vertices of each triangle
					//triangle t[0] will have v[0]->(v[0]+1) in the correct right hand direction
					//triangle t[1] will be backwards: (v[0]+1)->v[0]
} edge_t;

enum {
	OBJFILE_FLAG_USE_TEXCOORDS,
	OBJFILE_FLAG_USE_NORMALS,
};

class ObjFile {
protected:

	/**
	 * identifier -- typically the filename
	 */
	char title[256];

	/**
	 * an array of the vertex coordinates
	 */
	std::vector<vec3f> vertex;

	/**
	 * an array of the normals, sized by vertexCount
	 */
	std::vector<vec3f> normal;

	/**
	 * an array of the texcoords, sized by vertexCount
	 */
	std::vector<vec2f> texCoord;

	//todo - for optimization with glDrawElements,
	//store one vector of unique <vertex/texcoord/normal> sets

	/**
	 * an array of the <vertex/texcoord/normal> index sets for each triangle vertex
	 */
	std::vector<tri_t> tri;

	/**
	 * array of pairs of integers referencing triangle indices
	 * ranged [0, 2 * tri.size() / 3 - 1]
	 */
	std::vector<edge_t> edge;

	/**
	 * Bounding box containing all the vertices of this object
	 */
	box_t bbox;

	/**
	 * Bounding radius containing all the vertices of this object
	 */
	float brad;
	
	/**
	 * Clear the object
	 */
	void reset();

public:

	ObjFile() {reset();}
	~ObjFile() {unload();}

	/**
	 * Unallocate all buffers and clear the object
	 */
	void unload();

	/**
	 * Loads the ObjFile object from a file
	 */
	bool load(const char *filename);

	/**
	 * Render an object using normals
	 */
	void render() const;

	/**
	 * constant access to the bounding box
	 */
	const box_t &getBBox() const { return bbox; }

	/**
	 * returns the radius of the smallest sphere centered at the origin 
	 * surrounding all vertices of this object
	 */
	const float getBoundingRadius() const { return brad; }

	/**
	 * access to the title
	 */
	const char *getTitle() { return title; }

	/**
	 * triangle array access
	 */
	const std::vector<tri_t> &getTris() const { return tri; }

	/**
	 * vertex array access
	 */
	const std::vector<vec3f> &getVertexes() const { return vertex; }

	/** 
	 * edge array access
	 */
	const std::vector<edge_t> &getEdges() const { return edge; }
};
