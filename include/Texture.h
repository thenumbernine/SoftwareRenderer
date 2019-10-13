#pragma once

#include "sw.h"

#include "Main.h"

class Image {
public:
	int sizeX;
	int sizeY;
	unsigned char *data;
	Image() : sizeX(0), sizeY(0), data(0) {}
};

/**
 * This class represents a texture object stored in OpenGL.
 * It is used as a wrapper for all OpenGL functionality, mostly to save a programmer some typing.
 */
class Texture {
protected:

	/**
	 * The index generated by glGenTextures
	 */
	unsigned int texture_id;

	/**
	 * Either TEXTURE_1D, TEXTURE_2D, TEXTURE_CUBE_MAP_ARB, or TEXTURE_3D
	 * Currently only TEXTURE_1D, TEXTURE_2D, and TEXTURE_CUBE_MAP_ARB are supported.
	 */
	int target;

	/**
	 * Clear all variables and restore this object to its default state
	 */
	void reset();

	/**
	 * the function used to initialize textures in OpenGL
	 * and to generate this's texture_id
	 */
	void initTextureID();

	/**
	 * initialization for cube maps
	 */
	bool initCubeMap();

	/**
	 * extracts the target from a 1D buffer
	 * - returns NULL if it doesnt need extraction
	 */
	unsigned char *extract1DBuffer(int *_components, int width, int format, int type, const unsigned char *ptr, int colskip = 0);

	/**
	 * extracts the target from a 2D buffer
	 * - returns NULL if it doesnt need extraction
	 */
	unsigned char *extract2DBuffer(int *_components, int width, int height, int format, int type, const unsigned char *ptr, int colskip = 0, int rowskip = 0);

	/**
	 * loads a PPM file
	 */
	Image *loadPPM(const char *filename);

	/**
	 * loads an uncompressed BMP file
	 */
	Image *loadBMP(const char *filename);

public:

	Texture() {reset();}
	~Texture() {unload();}

	/**
	 * This safely unloads all loaded resources from the object.
	 * Specifically this will free the allocated texture_id value from OpenGL.
	 */
	void unload();

	/**
	 * Shell function for gluBuild1DMipmaps
	 * if colskip < the size of a pixel then it will assume it is equal to the size of a pixel
	 * if colskip > the size of a pixel then it will allocate a new buffer for uploading the data
	 * TODO - have mipmapping optional
	 * (mipmap generation is unneeded when generateMipmapSGIS is set)
	 */
	bool build1DMipmaps(int width, int format, int type, const unsigned char *ptr, int colskip = 0);

	/**
	 * Shell function for gluBuild2DMipmaps
	 * TODO - have mipmapping optional
	 * (mipmap generation is unneeded when generateMipmapSGIS is set)
	 */
	bool build2DMipmaps(int width, int height, int format, int type, const unsigned char *ptr, int colskip = 0, int rowskip = 0);

	/**
	 * creates an empty cube map with all six surfaces of the specified dimensions
	 * with no texture data.
	 */
	bool createEmptyCubeMap();

	/**
	 * side = GL_TEXTURE_CUBE_MAP_{POSITIVE|NEGATIVE}_{X|Y|Z}_ARB constant
	 * all else - is passed along to glTexImage2D
	 * TODO - should 'side' be based upon GL_TEXTURE_CUBE_MAP_POSTIIVE_X_ARB + 0..5
	 *			or should 'side' just be 0..5?
	 * TODO - have mipmapping optional
	 */
	bool createCubeMapSide(int side, int width, int height, int format, int type, const unsigned char *ptr, int colskip = 0, int rowskip = 0);

	/**
	 * uses texSubImage for faster upload than creating a fully new texture would
	 * TODO - have mipmapping optional
	 */
	bool updateCubeMapSide(int side, int width, int height, int format, int type, const unsigned char *ptr, int colskip = 0, int rowskip = 0);

	/**
	 * Shell function for auxDIBImageLoad - passes data along to gluBuild2DMipmaps
	 */
	bool load2D(const char *filename);

	/**
	 * Shell function for auxDIBImageLoad/gluBuild2DMipmaps
	 * combined with GL_TEXTURE_CUBE_MAP_ARB and GL_TEXTURE_CUBE_MAP_{NEGATIVE|POSITIVE}_{X|Y|Z}_ARB
	 * fails if the GL_ARB_texture_cube_map extension was not detected
	 */
	bool loadCubeMap(
		const char *filename0,
		const char *filename1,
		const char *filename2,
		const char *filename3,
		const char *filename4,
		const char *filename5);

	/**
	 * Shell function for glBindTexture
	 * This also enables the texture's target
	 */
	void bind() {
		//disable any previous targets (to prevent any from overriding us)
		unbind();
		//enable the target we're looking for
		swEnable(target);
		//and bind the texture
		swBindTexture(target, texture_id);
	}

	/**
	 * Disables all glEnable(GL_TEXTURE***)'s
	 */
	static void unbind() {
		swDisable(SW_TEXTURE_1D);
		swDisable(SW_TEXTURE_2D);
		swDisable(SW_TEXTURE_CUBE_MAP);
		swDisable(SW_TEXTURE_3D);
	}

	/**
	 * Shell function for glTexParameteri
	 * for the GL_WRAP_S and GL_WRAP_T parameters
	 *
	 * TODO - add support for GL_WRAP_R and GL_WRAP_Q
	 */
	void setWrap1D(int mode_s) {
		bind();
		swTexParameteri(target, SW_TEXTURE_WRAP_S, mode_s);
	}

	void setWrap2D(int mode_s, int mode_t) {
		bind();
		swTexParameteri(target, SW_TEXTURE_WRAP_S, mode_s);
		swTexParameteri(target, SW_TEXTURE_WRAP_T, mode_t);
	}

	/**
	 * Shell function for glTexParameteri's GL_TEXTURE_MIN_FILTER and GL_TEXTURE_MAX_FILTER
	 */
	void setFilter(int mag, int min) {
		bind();
		swTexParameteri(target, SW_TEXTURE_MAG_FILTER, mag);
		swTexParameteri(target, SW_TEXTURE_MIN_FILTER, min);
	}

	/**
	 * Set to 'true' to enable glTexParameteri(target, GL_GENERATE_MIPMAP_SGIS, enable)
	 *	only works if GL_SGIS_generate_mipmap is found
	 *
	 * TODO - add support for glTexParameteri(target, GL_GENERATE_MIPMAP_HINT_SGIS, GL_{FASTEST|NICEST});
	 * TODO - add support for the GL_SGIS_texture_lod extension
	 */
	void generateMipmapSGIS(bool enable) {
		bind();
		swTexParameteri(target, SW_GENERATE_MIPMAP, enable);
	}

	/**
	 * specifies the quality of the mipmaps generated
	 * mode = either GL_NICEST or GL_FASTEST
	 */
	void generateMipmapHintSGIS(int mode) {
		bind();
		swTexParameteri(target, SW_GENERATE_MIPMAP_HINT, mode);
	}

};
