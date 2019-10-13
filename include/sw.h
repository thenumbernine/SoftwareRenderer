#pragma once

#include <stdio.h>

//#ifdef __cplusplus
//extern "C" {
//#endif

typedef struct {
   int w, h;
   int *values;
} swImage;

void swImageInit ( swImage *I );
void swImageDestroy ( swImage *I );

void swImageGetSize ( swImage *I, int *w, int *h );
void swImageSetSize ( swImage *I, int w, int h );

FILE* swImageWritePPM ( swImage *I, char *filename );

void swImageWritePixel ( swImage *I, int w, int h, int r, int g, int b );
void swImageReadPixel ( swImage *I, int w, int h, int *r, int *g, int *b );

enum SWWriteMode { SW_OVERWRITE, SW_REPLACE, SW_XOR };
enum SWColor { SW_RED, SW_GREEN, SW_BLUE };
enum SWEnable { SW_DEPTH_TEST };

void swBeginGraphics ( int w, int h );
void swEndGraphics ();
void swWriteFramebuffer ( char *filename );
void swWritePixel ( int x, int y, int r, int g, int b );
void swSetWriteMode ( int /* SWWriteMode */ mode );
void swWaitOnEscape ();
void swGetFramebufferSize ( int *w, int *h );

#define SW_NONE                           0
#define SW_POINTS                         0x0000
#define SW_LINES                          0x0001
#define SW_LINE_LOOP                      0x0002
#define SW_POLYGON                        0x0009


// GL header rip:

/* AttribMask */
#define SW_DEPTH_BUFFER_BIT				0x00000100
#define SW_COLOR_BUFFER_BIT				0x00004000

/* ErrorCode */
#define SW_NO_ERROR						0
#define SW_INVALID_ENUM					0x0500
#define SW_INVALID_VALUE					0x0501
#define SW_INVALID_OPERATION				0x0502
#define SW_STACK_OVERFLOW					0x0503
#define SW_STACK_UNDERFLOW					0x0504
#define SW_OUT_OF_MEMORY					0x0505

/* Enable, Disable */
#define SW_DEPTH_TEST						0x0B71
#define SW_NORMALIZE						0x0BA1
#define SW_TEXTURE_1D						0x0DE0
#define SW_TEXTURE_2D						0x0DE1
#define SW_TEXTURE_CUBE_MAP				0x8513
#define SW_TEXTURE_3D						0x806F
#define SW_TEXTURE_GEN_S					0x0C60
#define SW_TEXTURE_GEN_T					0x0C61
#define SW_TEXTURE_GEN_R					0x0C62
#define SW_TEXTURE_GEN_Q					0x0C63
#define SW_BLEND							0x0BE2

/* MatrixMode */
#define SW_MODELVIEW						0x1700
#define SW_PROJECTION						0x1701
#define SW_TEXTURE							0x1702

/* PolygonMode */
#define SW_POINT                          0x1B00
#define SW_LINE                           0x1B01
#define SW_FILL                           0x1B02

/* TextureCoordName */
#define SW_S								0x2000
#define SW_T								0x2001
#define SW_R								0x2002
#define SW_Q								0x2003

/* TextureGenMode */
#define SW_OBJECT_LINEAR					0x2401
#define SW_NORMAL_MAP						0x8511
#define SW_REFLECTION_MAP					0x8512
#define SW_OBJECT_CYLINDER					0xFFFE	//created by me.
#define SW_OBJECT_SPHERE					0xFFFD	//created by me.

/* TextureGenParameter */
#define SW_TEXTURE_GEN_MODE				0x2500

/* TextureMagFilter */
#define SW_NEAREST							0x2600
#define SW_LINEAR							0x2601

/* TextureMinFilter */
/*      GL_NEAREST */
/*      GL_LINEAR */
#define SW_NEAREST_MIPMAP_NEAREST			0x2700
#define SW_LINEAR_MIPMAP_NEAREST			0x2701
#define SW_NEAREST_MIPMAP_LINEAR			0x2702
#define SW_LINEAR_MIPMAP_LINEAR			0x2703

/* TextureParameterName */
#define SW_TEXTURE_MAG_FILTER				0x2800
#define SW_TEXTURE_MIN_FILTER				0x2801
#define SW_TEXTURE_WRAP_S					0x2802
#define SW_TEXTURE_WRAP_T					0x2803
#define SW_TEXTURE_WRAP_R					0x8072
/*      GL_TEXTURE_BORDER_COLOR */
/*      GL_TEXTURE_PRIORITY */

/* TextureWrapMode */
#define SW_CLAMP							0x2900
#define SW_REPEAT							0x2901
#define SW_CLAMP_TO_EDGE					0x812F


//cube map definitions
#define SW_TEXTURE_CUBE_MAP_POSITIVE_X     0x8515
#define SW_TEXTURE_CUBE_MAP_NEGATIVE_X     0x8516
#define SW_TEXTURE_CUBE_MAP_POSITIVE_Y     0x8517
#define SW_TEXTURE_CUBE_MAP_NEGATIVE_Y     0x8518
#define SW_TEXTURE_CUBE_MAP_POSITIVE_Z     0x8519
#define SW_TEXTURE_CUBE_MAP_NEGATIVE_Z     0x851A

//GL_SGIS_generate_mipmap
#define SW_GENERATE_MIPMAP					0x8191
#define SW_GENERATE_MIPMAP_HINT			0x8192

//former members:
void swClear ( int bits );
void swClearColor (float r, float g, float b, float a);
void swClearDepth (double depth);
void swFlush();

//poly
void swViewport(int x, int y, int w, int h);
void swBegin ( int drawable );
void swEnd ();
void swColor3f(float r, float g, float b);
void swColor4f(float r, float g, float b, float a);
void swVertex2f ( float x, float y );
void swVertex3f(float x, float y, float z);
void swVertex3fv(float *v);
void swVertex4f(float x, float y, float z, float w);
void swNormal3f(float x, float y, float z);
void swTexCoord2f(float x, float y);
void swTexCoord3f(float x, float y, float z);

void swPolygonMode(/* int face, */ int mode);

//matrix
void swOrtho(float l, float r, float b, float t, float n, float f);
void swFrustum(float l, float r, float b, float t, float n, float f);
void swLoadIdentity();
void swMatrixMode(int mode);
void swPushMatrix();
void swPopMatrix();
void swRotatef(float degrees, float x, float y, float z);
void swScalef(float x, float y, float z);
void swTranslatef(float x, float y, float z);
void swMultMatrixf(const float *m);	//column-major array of 16 floats
void swLoadMatrixf(const float *m);

//texture
void swGenTextures(int n, unsigned int *textures);
void swDeleteTextures(int n, unsigned int *textures);
void swTexImage1D(int target, int width, const void *pixels);
void swTexImage2D(int target, int width, int height, const void *pixels);
void swTexImage3D(int target, int width, int height, int depth, const void *pixels);
void swBindTexture(int target, int texture);
void swTexParameteri(int target, int pname, int param);
void swTexGeni(int coord, int pname, int param);
					  
//state
void swEnable(int cap);
void swDisable(int cap);

//glu rip:
void swPerspective(float fovy, float aspectRatio, float f, float n);
void swLookAt(float from[3], float at[3], float up[3]);

//not related to OpenGL:
void swInitialize();

//extra functions:
void swWriteFramebufferBMP(char *filename );

//TODO - replace this with a GLUT-style loop
void swWindow_NoGL_Update();

extern long* swBMPBuffer;

//#ifdef __cplusplus
//}
//#endif
