#include "swMain.h"
#include <string.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <time.h>

#include "Texture.h"
#include "Main.h"

using namespace std;

//static int GL_Type_to_BPP(int type) {
//	switch (type) {
//	case GL_BITMAP:			return 1;
//	case GL_UNSIGNED_BYTE:
//	case GL_BYTE:			return 8;
//	case GL_UNSIGNED_SHORT:
//	case GL_SHORT:			return 16;
//	case GL_UNSIGNED_INT:
//	case GL_INT:
//	case GL_FLOAT:			return 32;
//	}
//	return 0;
//}
//
//static int GL_Format_to_Components(int format) {
//	switch (format) {
//	case GL_COLOR_INDEX:
//	case GL_RED:
//	case GL_GREEN:
//	case GL_BLUE:
//	case GL_ALPHA:
//	case GL_LUMINANCE:		return 1;
//	case GL_LUMINANCE_ALPHA:return 2;
//	case GL_RGB:			return 3;
//	case GL_RGBA:			return 4;
//	}
//	return 0;
//}

void Texture::reset() {
	texture_id = 0;
	target = 0;
}

void Texture::unload() {
	if (texture_id) {
		swDeleteTextures(1, &texture_id);
	}
	reset();
}

void Texture::initTextureID() {
	//required for our texture...
	swGenTextures(1, &texture_id);
}

//unsigned char *Texture::extract1DBuffer(int *_components, int width, int format, int type, const unsigned char *ptr, int colskip) {
//	//data type being the format of the data used to store the image
//	int bpp = GL_Type_to_BPP(type);
//	int components = GL_Format_to_Components(format);
//	//readback
//	if (_components) *_components = components;
//
//	int pixelsize = (bpp * components) >> 3;
//	int size = width * pixelsize;
//
//	if (colskip < pixelsize) {
//		colskip = pixelsize;
//	}
//
//	if (colskip <= pixelsize) return NULL;
//
//	//allocate our data
//	printf("allocating texture copy buffer...\n");
//	unsigned char *data = new unsigned char[size];
//	if (!data) {
//		printf("Texture::build1DMipmaps failed to allocate temporary buffer\n");
//		return false;
//	}
//
//	int i;
//	const unsigned char *psrc_x;
//	unsigned char *pdest;
//
//	pdest = data;
//	psrc_x = ptr;
//	for (i = 0; i < width; i++) {
//		memcpy(pdest, psrc_x, pixelsize);
//		pdest += pixelsize;
//		psrc_x += colskip;
//	}
//
//	return data;
//}
//
//unsigned char *Texture::extract2DBuffer(int *_components, int width, int height, int format, int type, const unsigned char *ptr, int colskip, int rowskip) {
//
//	//data type being the format of the data used to store the image
//	int bpp = GL_Type_to_BPP(type);
//	int components = GL_Format_to_Components(format);
//	//readback
//	if (_components) *_components = components;
//
//	int pixelsize = (bpp * components) >> 3;
//	int rowsize = pixelsize * width;
//	int size = rowsize * height;
//
//	if (colskip < pixelsize) {
//		colskip = pixelsize;
//	}
//	if (rowskip < rowsize) {
//		rowskip = rowsize;
//	}
//
//	if (colskip <= pixelsize && rowskip <= rowsize) return NULL;
//
//	//allocate our data
//	printf("allocating texture copy buffer...\n");
//	unsigned char *data = new unsigned char[size];
//	if (!data) {
//		printf("Texture::build2DMipmaps failed to allocate temporary buffer\n");
//		return false;
//	}
//
//	int i,j;
//	const unsigned char *psrc_x, *psrc_y;
//	unsigned char *pdest;
//
//	pdest = data;
//	psrc_y = ptr;
//	for (j = 0; j < height; j++) {
//		psrc_x = psrc_y;
//		for (i = 0; i < width; i++) {
//			memcpy(pdest, psrc_x, pixelsize);
//			pdest += pixelsize;
//			psrc_x += colskip;
//		}
//		psrc_y += rowskip;
//	}
//
//	return data;
//}

bool Texture::build1DMipmaps (
	int width,
	int format,
	int type,
	const unsigned char *ptr,
	int colskip)
{
	unload();

//	int components;	//read back through extraction
	unsigned char *data = NULL;//extract1DBuffer(&components, width, format, type, ptr, colskip);

	//create the OpenGL texture alias and init some of its required variables
	initTextureID();

	target = SW_TEXTURE_1D;
	bind();
//	gluBuild1DMipmaps (target, components, width, format, type, data ? data : ptr);
	swTexImage1D(target, width, data ? data : ptr);

	// free our data
	if (data) delete[] data;

	//default settings for the texture
	setWrap1D(SW_REPEAT);
	setFilter(SW_LINEAR, SW_LINEAR_MIPMAP_LINEAR);

	return true;
}

bool Texture::build2DMipmaps (
	int width,
	int height,
	int format,
	int type,
	const unsigned char *ptr,
	int colskip,
	int rowskip)
{
	unload();

	//create the OpenGL texture alias and init some of its required variables
	initTextureID();

	target = SW_TEXTURE_2D;
	bind();

//	int components;	//read back through extraction
	unsigned char *data = NULL;//extract2DBuffer(&components, width, height, format, type, ptr, colskip, rowskip);

//	gluBuild2DMipmaps (target, components, width, height, format, type, data ? data : ptr);
	swTexImage2D(target, width, height, data ? data : ptr);

	//todo - test for GL error & return false if we found one

	// free our data
	if (data) delete[] data;

	//default settings for the texture
	setWrap2D(SW_REPEAT, SW_REPEAT);
	setFilter(SW_LINEAR, SW_LINEAR_MIPMAP_LINEAR);

	return true;
}

Image *Texture::loadPPM(const char *filename) {
    int h,w;
    char sbuf[128];
    char * tok;

    FILE * iFile = fopen(filename, "r");

    fgets(sbuf, sizeof(sbuf), iFile);
    fgets(sbuf, sizeof(sbuf), iFile);
    fgets(sbuf, sizeof(sbuf), iFile);
    tok = strtok (sbuf, " "); w = atoi(tok);
    tok = strtok (NULL, " "); h = atoi(tok);
    fgets(sbuf, sizeof(sbuf), iFile);
	//int maxVal = atoi(sbuf);

	int size = w*h*3;
	
    unsigned char *image = new unsigned char[size];
	if (!image) return NULL;

    int i = 0;

	printf("beginning ppm texture load...\n");

	clock_t startTime = clock();
	int lastSecond = 0;

    while (!feof(iFile)) {

		//for complex models...
		{
			clock_t thisTime = clock();
			int second = (thisTime - startTime) / (10 * CLOCKS_PER_SEC);
			if (second != lastSecond) {
				lastSecond = second;
				printf(" %d of %d (%d%%) done \n", i, size, (i*100/size) );
			}
		}

        fgets(sbuf, 128, iFile);
        tok = strtok (sbuf," ");
        while (tok != NULL) {
            if (atoi(tok) != 0 || tok[0] == '0') image[i++] = (unsigned char)atoi(tok);
            tok = strtok (NULL, " ");

			if (i >= size) break;
        }

		if (i >= size) break;
    }
    fclose(iFile);

	printf("end ppm texture load.\n");

	Image *img = new Image();
	if (img) {
		img->sizeX = w;
		img->sizeY = h;
		img->data = image;
	}
	return img;
}

Image *Texture::loadBMP(const char *filename) {
	//list out resources we have to free here:
	unsigned char *imgdata = NULL;
	
	try {
	
		std::ostringstream s;

		//everything except the 2-byte sig ...
		//because everything ELSE is 4byte aligned...
		//and its either this or figure out the 'packed' attribute ...
		struct bitmapHeader_t {
			//freakin a, this is packing us . . .
			int fileSize;		//02:	//size of total file
			int h1;				//06:	//reserved / zero
			int imgOffset;		//0A:	//offset from file start to beginning of image data
			int extraSize;		//0E:	//size of header after the initial 14 == 0x0E bytes
			int width;			//12:
			int height;			//16:
			int h6;
//			union {
//				struct {
//					short bitsPerPixel;	//1A:	//seems like this was the hiword ... so maybe there's an endian issue here?
//					short planes;		//1C:
//				};
//				int h7;
//			};
			int compression;	//1E:
			int h8;				//22:
			int h9;				//26:
			int palIndexCount;	//2A:
		} *header;
		
		unsigned char *base = NULL;
		{
			FILE *file = fopen(filename, "rb");
			if (!file) {
				cerr << "failed to load file " << filename << endl;
				throw 0;
			}
			fseek(file, 0, SEEK_END);
			size_t size = ftell(file);
			fseek(file, 0, SEEK_SET);
			base = new unsigned char[size];
			if (!base) {
				fclose(file);
				cerr << "failed to alloc " << size << " bytes for file " << filename << endl;
				throw 0;
			}
			fread(base, 1, size, file);
			fclose(file);
		}
		
		const unsigned char *srcptr = base;	
		if (*(short*)srcptr != 0x4D42) {
			cerr << "got a bad sig:" << *(short*)srcptr << endl;
			throw 0;
		}
		srcptr += 2;	//skip the sig
		
		header = (bitmapHeader_t *)srcptr;	//get the rest of the header
		short bitsPerPixel = (header->h6 >> 16) & 0xFFFF;
		short planes = header->h6 & 0xFFFF;

		if (planes != 1) {
			cerr << "got bad # planes:" << planes << endl;
			throw 0;
		}
		if (bitsPerPixel != 24) {
			cerr << "got bad bitsPerPixel:" << bitsPerPixel << endl;
			throw 0;
		}
		if (header->compression) {
			cerr << "got bad compression: " << header->compression << endl;
			throw 0;
		}
			
		//now get our row skip:
		int rowsize = header->width * bitsPerPixel;
		rowsize = (rowsize >> 3) + !!(rowsize & 7);	//div 8, round up
		int rowofs = rowsize & 3;
		if (rowofs) rowofs = 4 - rowofs;	//switch up a dw if we dont align 4
		
		int imgsize = rowsize * header->height;
		imgdata = new unsigned char[imgsize];
		if (!imgdata) {
			cerr << "failed to alloc " << imgsize << " bytes for img!" << endl;
			throw 0;
		}

		//now read it through - getting rid of the row spacing
		srcptr = base + header->imgOffset;	//reset the src ptr to the data start
		unsigned char *dstptr = imgdata;
		for (int row = 0; row < header->height; row++, srcptr += rowofs) {
			//swap rows as we go =) bitmaps are upside-down
			dstptr = imgdata + (header->height - row - 1) * header->width * 3;
			//swap red and blue as we go =)
			for (int col = 0; col < header->width; col++, srcptr += 3, dstptr += 3) {
				dstptr[0] = srcptr[2];
				dstptr[1] = srcptr[1];
				dstptr[2] = srcptr[0];
			}
		}
		
		//do this last so img == null if anything goes wrong
		Image *img = new Image();
		if (!img) {
			delete[] imgdata;	//the only non-cleanup object ...
			cerr << "failed to alloc the Image" << endl;
			throw 0;
		}
		img->data = imgdata;
		img->sizeX = header->width;
		img->sizeX = header->height;
		return img;		
	} catch (int e) {
		if (imgdata) delete[] imgdata;
		imgdata = NULL;
		cerr << "ImageLoadBMP failed to load file " << filename << endl;
	}
	return NULL;
}

bool Texture::load2D(const char *filename) {
	bool ret = false;

	//check the filename extension
	const char *ext = filename;
	while (*ext) {ext++;}	//seek to end
	while (ext > filename) {
		ext--;
		if (*ext == '.') {
			ext++;
			break;
		}
	}

	Image *img = NULL;

	if (!strcasecmp(ext, "ppm")) {
		img = loadPPM(filename);
	} else {
		img = loadBMP(filename);
	}

	if (img) {
		if (img->data) {
			ret = build2DMipmaps(img->sizeX, img->sizeY, 0/*GL_RGB*/, 0/*GL_UNSIGNED_BYTE*/, img->data, 3, 3*img->sizeX);
			delete[] img->data;
		}
		delete[] img;
	}
	return ret;
}

bool Texture::initCubeMap() {

	initTextureID();

	target = SW_TEXTURE_CUBE_MAP;

	return true;
}

bool Texture::createEmptyCubeMap() {//int width, int height, int format, int type) {

	unload();

	if (!initCubeMap()) return false;

	bind();

	//default settings for the texture.
	//same as is found in loadCubeMap
	setWrap2D(SW_CLAMP, SW_CLAMP);
	setFilter(SW_LINEAR, SW_LINEAR);

	return true;
}

bool Texture::createCubeMapSide(
	int side,
	int width,
	int height,
	int format,
	int type,
	const unsigned char *ptr,
	int colskip,
	int rowskip)
{
	//TODO - complain if width != height, or if they are not power-of-two

	bind();

//	int components;	//read back through extraction
	unsigned char *data = NULL;//extract2DBuffer(&components, width, height, format, type, ptr, colskip, rowskip);

//	glTexImage2D(
//		side,			//GLenum target, 
//		0,				//GLint level, 
//		components,		//GLint components, 
//		width,			//GLsizei width, 
//		height,			//GLsizei height, 
//		0,				//GLint border, 
//		format,			//GLenum format, 
//		type,			//GLenum type, 
//		data ? data : ptr//const GLvoid *pixels 
//	); 

	//looks like 'target' is important after all
	swTexImage2D(side, width, height, data ? data : ptr);

	if (data) delete[] data;

	return true;
}

//bool Texture::updateCubeMapSide(
//	int side,
//	int width,
//	int height,
//	int format,
//	int type,
//	const unsigned char *ptr,
//	int colskip,
//	int rowskip)
//{
//	bind();
//	unsigned char *data = NULL;//extract2DBuffer(NULL, width, height, format, type, ptr, colskip, rowskip);
////	glTexSubImage2D(side, 0, 0, 0, width, height, format, type, data ? data : ptr);
//	swTexSubImage2D(side, 0, 0, width, height, data ? data : ptr);
//	if (data) delete[] data;
//	return true;
//}


bool Texture::loadCubeMap(
	const char *filename0,
	const char *filename1,
	const char *filename2,
	const char *filename3,
	const char *filename4,
	const char *filename5)
{
	unload();

	const char *filename[6] = {
		filename0,
		filename1,
		filename2,
		filename3,
		filename4,
		filename5
	};

	int side;

	//make sure all texture names are present
	for (side = 0; side < 6; side++) {
		if (!filename[side]) return false;
	}

	if (!initCubeMap()) return false;

	bind();

	for (side = 0; side < 6; side++) {

		bool success = false;

		//copy and mod of load2D
		//TODO - the functionality is so similar that we should have both these reference a sub-function
		Image *img = loadBMP(filename[side]);
		if (img) {
			if (img->data) {
				//TODO - make sure gluBuild2DMipmaps is using the right proxy state
				//and is resizing our texture to a square
//				gluBuild2DMipmaps (SW_TEXTURE_CUBE_MAP_POSITIVE_X + side,
//					3, img->sizeX, img->sizeY, GL_RGB, GL_UNSIGNED_BYTE, img->data);
				swTexImage2D(SW_TEXTURE_CUBE_MAP_POSITIVE_X + side, img->sizeX, img->sizeY, img->data);
				success = true;
				delete[] img->data;
			}
			delete img;
		}

		if (!success) {
			unload();
			return false;
		}
	}

	//default settings for the texture
	//TODO - find if these wrap calls are per-cubemap or per-face
	//		if they're per-face, move them to inside the side=0..5 loop
	//		if they're per-cubemap then shouldn't they require a GL_TEXTURE_WRAP_R parameter too?
	//TODO - get the GL_CLAMP_TO_EDGE extension from GL_EXT_texture_edge_clamp
	//		also check out GL_ARB_texture_border_clamp
	//		use whichever is best here. until then use GL_CLAMP
	setWrap2D(SW_CLAMP, SW_CLAMP);	
	setFilter(SW_LINEAR, SW_LINEAR_MIPMAP_LINEAR);

	return true;
}
