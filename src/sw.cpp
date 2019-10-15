#include "sw.h"
#include "swMain.h"

#include <memory.h>
#include <assert.h>

#include <stdio.h>


#include "Main.h"
#include "vec.h"

#include "swToolkit.h"

typedef unsigned char Byte;


long swArray3DTo1D ( int chan, int w, int sizeW, int h, int sizeH ) {
	// convert 3D array coordinates to a single lookup value
	return ( chan * sizeW * sizeH + w * sizeH + h );
}

long swImageValueCount ( swImage *I ) {
	return ( I->w * I->h * 3 );
}

void swImageInit ( swImage *I ) {
	I->w = I->h = 0;
	I->values = (int *) 0;
}

void swImageDestroy ( swImage *I ) {
	delete[] I->values;
}

void swImageGetSize ( swImage *I, int *w, int *h ) {
	*w = I->w;
	*h = I->h;
}

void swImageSetSize ( swImage *I, int w, int h ) {
	long count;

	delete[] I->values;

	I->w = w;
	I->h = h;
	count = swImageValueCount( I );
	I->values = new int[count];
	memset(I->values, 0, sizeof(I->values[0]) * count);
	if ( ! I->values ) {
		fprintf( stderr, "swImage not allocated\n" );
		exit( 2 );
	}
}

FILE* swImageWritePPM ( swImage *I, char *filename ) {

	int row, col;
	int r, g, b;
	Byte br, bg, bb;

	FILE *fp = fopen( filename, "w" );
	if ( ! fp ) return fp;

	// output header
	fprintf( fp, "P6\n" );
	fprintf( fp, "%d %d\n", I->w, I->h );
	fprintf( fp, "%d\n", 255 );

	// write image values (only handles 8 bits)
	for ( row = 0; row < I->h; row++ )
		for ( col = 0; col < I->w; col++ ) {
			swImageReadPixel( I, col, I->h - row - 1, &r, &g, &b );
			
		 r=r;
		 g=g;
		 b=b;

		 br = (Byte) r;
		 bg = (Byte) g;
		 bb = (Byte) b;
			
		 if ( fwrite( &br, sizeof( Byte ), 1, fp ) != sizeof( Byte ) )
				return (FILE*) NULL;
			if ( fwrite( &bg, sizeof( Byte ), 1, fp ) != sizeof( Byte ) )
				return (FILE*) NULL;
			if ( fwrite( &bb, sizeof( Byte ), 1, fp ) != sizeof( Byte ) )
				return (FILE*) NULL;

		
			//if ( fwrite( &br, sizeof( int ), 1, fp ) != sizeof( Byte ) )
			  // return (FILE*) NULL;
			//if ( fwrite( &bg, sizeof( Byte ), 1, fp ) != sizeof( Byte ) )
			  // return (FILE*) NULL;
			//if ( fwrite( &bb, sizeof( Byte ), 1, fp ) != sizeof( Byte ) )
			  // return (FILE*) NULL;
		//fprintf(fp, "%d %d %d %d", r, g, b,0);
		}

	fclose( fp );
	return fp;
}

void swImageReadPixel ( swImage *I, int w, int h, int *r, int *g, int *b ) {
	long index = swArray3DTo1D( SW_RED, w, I->w, h, I->h );
	*r = I->values[ index ];

	index = swArray3DTo1D( SW_GREEN, w, I->w, h, I->h );
	*g = I->values[ index ];

	index = swArray3DTo1D( SW_BLUE, w, I->w, h, I->h );
	*b = I->values[ index ];
}

void swImageWritePixel ( swImage *I, int w, int h, int r, int g, int b ) {
	long index;

	index = swArray3DTo1D( SW_RED, w, I->w, h, I->h );
	I->values[ index ] = r;

	index = swArray3DTo1D( SW_GREEN, w, I->w, h, I->h );
	I->values[ index ] = g;

	index = swArray3DTo1D( SW_BLUE, w, I->w, h, I->h );
	I->values[ index ] = b;
}

static int /* SWWriteMode */ SWCurrentMode;
swImage SWCurrentImage;

long *swBMPBuffer = NULL;
//BITMAPINFO swCurrentBMI;

float *swDepthBuffer = NULL;

void swResizeDepthBuffer(int w, int h) {
	if (swDepthBuffer) delete[] swDepthBuffer;
	swDepthBuffer = new float[w*h];
}

//i'm moving this routine into the swToolkit
// - since it is more OS-dependant than all the other routines

void swBeginGraphics ( int w, int h ) {

//	//create a window *here*
//	createSWWindow_NoGL(w, h);

	//CreateGLWindow("testWin", w,h, 16, 0);
	// set the single window state
	SWCurrentMode = SW_OVERWRITE;

	// build a frame buffer for reading and writing
	swImageInit( &SWCurrentImage );
	swImageSetSize( &SWCurrentImage, w, h );

	swClearColor( 0, 0, 0, 0);
	swClear(SW_COLOR_BUFFER_BIT);

	swViewport(0, 0, w, h);

}

void swGetFramebufferSize ( int *w, int *h ) {
	*w = SWCurrentImage.w;
	*h = SWCurrentImage.h;
}

void swEndGraphics ( )
{

//	if (displayWnd) {
//		DestroyWindow(displayWnd);
//		displayWnd = NULL;
//	}

	swImageDestroy( &SWCurrentImage );
}

#if 0
static HBITMAP swHBitmap = NULL;
static HBITMAP swHOldBitmap = NULL;
static HWND swHWnd = NULL;
static HDC swHDCBackBuffer = NULL;		//our window's device context
static int swWindowWidth = 1;
static int swWindowHeight = 1;

static void swWindow_createHBitmap(HDC hdc, int w, int h) {

	assert(w);
	assert(h);

	memset ( &swCurrentBMI, 0, sizeof(BITMAPINFO) );
	swCurrentBMI.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	swCurrentBMI.bmiHeader.biWidth = w;
	swCurrentBMI.bmiHeader.biHeight = h;
	swCurrentBMI.bmiHeader.biPlanes = 1;
	swCurrentBMI.bmiHeader.biBitCount = 32;
	swCurrentBMI.bmiHeader.biCompression = BI_RGB;

	//createDIBitmap

	swHBitmap = CreateDIBSection(hdc, &swCurrentBMI, DIB_RGB_COLORS, (void **)&swBMPBuffer, NULL, 0);

	swResizeDepthBuffer(swCurrentBMI.bmiHeader.biWidth, swCurrentBMI.bmiHeader.biHeight);
}

//routines to hopefully straighten it all out:
void swWindow_Init(HWND hWnd) {
	assert(hWnd);

	swHWnd = hWnd;

	//use this method if our window is already created
	RECT rect;
	GetClientRect(swHWnd, &rect);
	swWindowWidth = rect.right;
	swWindowHeight = rect.bottom;

	if (swWindowWidth < 1) swWindowWidth = 1;
	if (swWindowHeight < 1) swWindowHeight = 1;

//get the device context for the window
	HDC hdc = GetDC(swHWnd);

//create a graphics context for the device context
//make this device context current for the graphics context

	//create our dc
	swHDCBackBuffer = CreateCompatibleDC(hdc);

	//create our dc's bitmap
	swWindow_createHBitmap(hdc, swWindowWidth, swWindowHeight);

	//backup the new dc's old bitmap
	swHOldBitmap = (HBITMAP)SelectObject(swHDCBackBuffer, swHBitmap);

	//release our window's dc
	ReleaseDC(swHWnd, hdc);

	//and now for something completely different:
	//our sw graphics code:
	swImageInit( &SWCurrentImage );
	swImageSetSize( &SWCurrentImage, swWindowWidth, swWindowHeight );
}
#endif

void swWindow_Paint() {
#if 0	
	static PAINTSTRUCT swWindowPaintStruct;

	assert(swHWnd);

	BeginPaint(swHWnd, &swWindowPaintStruct);

	BitBlt(swWindowPaintStruct.hdc, 0, 0, swWindowWidth, swWindowHeight,
		swHDCBackBuffer, 0, 0, SRCCOPY);

	EndPaint(swHWnd, &swWindowPaintStruct);
#endif
}

void swWindow_Resize(int w, int h) {
#if 0	
	assert(swHWnd);

	//get our sizes
	swWindowWidth = w;
	swWindowHeight = h;

	if (swWindowWidth < 1) swWindowWidth = 1;
	if (swWindowHeight < 1) swWindowHeight = 1;

	//restore the old bitmap
	SelectObject(swHDCBackBuffer, swHOldBitmap);

	//delete ours
	DeleteObject(swHBitmap);
	swBMPBuffer = NULL;
	if (swDepthBuffer) {
		delete[] swDepthBuffer;
		swDepthBuffer = NULL;
	}

	//get the window's dc again
	HDC hdc = GetDC(swHWnd);

	//make a new bitmap
	swWindow_createHBitmap(hdc, swWindowWidth, swWindowHeight);

	//let go of the window dc
	ReleaseDC(swHWnd, hdc);

	//and give our dc its new bitmap
	SelectObject(swHDCBackBuffer, swHBitmap);

	//and now for something completely different:
	//our sw graphics code:
	swImageSetSize( &SWCurrentImage, swWindowWidth, swWindowHeight );
#endif
}

void swWindow_Destroy() {
#if 0	
	//restore the dc's old bitmap
	SelectObject(swHDCBackBuffer, swHOldBitmap);

	//delete the dc
	DeleteDC(swHDCBackBuffer);

	//and delete our bitmap for it
	DeleteObject(swHBitmap); 
	swBMPBuffer = NULL;
	if (swDepthBuffer) {
		delete[] swDepthBuffer;
		swDepthBuffer = NULL;
	}
#endif
}

#include <math.h>

static vec4f swClearColorValue(0,0,0,0);

static float swClearDepthValue = 0.f;

void swClear (int bits) {

#if 0
	const int size = swCurrentBMI.bmiHeader.biWidth * swCurrentBMI.bmiHeader.biHeight * sizeof(long);

	if (bits & SW_COLOR_BUFFER_BIT && swBMPBuffer) {

#if 1
		int write = (unsigned char)(swClearColorValue.z * 255.f) |
					(unsigned char)(swClearColorValue.y * 255.f) << 8 |
					(unsigned char)(swClearColorValue.x * 255.f) << 16 |
					(unsigned char)(swClearColorValue.w * 255.f) << 24;
		memset(swBMPBuffer, write, size);

#else
		BitBlt(swHDCBackBuffer, 0, 0, swWindowWidth, swWindowHeight, 0, 0, 0, BLACKNESS);

#endif

	}

	//for some odd reason, these memsets only look at the first 8 bits of the 32-bit fill value passed.  WHY?!?!?!

	if (bits & SW_DEPTH_BUFFER_BIT && swDepthBuffer) {
		memset(swDepthBuffer, 0 /*0x7f / * *(int *)&swClearDepthValue >> 24*/, size);
	}
#endif
}

void swClearColor (float r, float g, float b, float a) {
	swClearColorValue = vec4f(r,g,b,a);
}

void swClearDepth (double depth) {
	swClearDepthValue = 1.f - (float)depth;
}

void swFlush ()
{
	swutPostRedisplay();
}

void swSetWriteMode ( int /* SWWriteMode */ mode )
{
	SWCurrentMode = mode;
}

//not called
void swWriteFramebuffer ( char *filename ) {
}

//not called
void swWriteFramebufferBMP ( char *filename ) {
}

inline int swXOR ( int a, int b ) {
	return ( a ^ b);
}

//called by swRender
void swWritePixel ( int x, int y, int r, int g, int b )
{
printf("here\n");
#if 0	
	int or, og, ob;

//	printf("SWCurrentImage.h: %d\n", SWCurrentImage.h);

	if ( x < 0 || x >= SWCurrentImage.w || y < 0 || y >= SWCurrentImage.h )
	{
//	  fprintf( stderr, "Attempted to write a pixel outside the image " );
//	  fprintf( stderr, "pixel: x = %d, y = %d\n", x, y );
//	  fprintf( stderr, "bounds: x = %d, y = %d\n", SWCurrentImage.w, SWCurrentImage.h);
	  return;
//	} else {
//	  fprintf( stderr, "wrote : [%d, %d] = [%x, %x, %x]\n", x, y ,r,g,b);
	}

	if ( SWCurrentMode == SW_XOR )
	{
		swImageReadPixel( &SWCurrentImage, x, y, &or, &og, &ob );
		r = swXOR ( r, or );
		g = swXOR ( g, og );
		b = swXOR ( b, ob );
	}

	{
		assert(swBMPBuffer);
		assert(swCurrentBMI.bmiHeader.biWidth);
		assert(swCurrentBMI.bmiHeader.biHeight);
		if (x < swCurrentBMI.bmiHeader.biWidth && 
			y < swCurrentBMI.bmiHeader.biHeight)
		{
			int a = 0xFF;
			long color = (unsigned char)r |
						(unsigned char)g << 8 |
						(unsigned char)b << 16 |
						(unsigned char)a << 24;
			swBMPBuffer[x + y * swCurrentBMI.bmiHeader.biWidth] = color;
		}
	}

	swImageWritePixel( &SWCurrentImage, x, y, r, g, b );
#endif
}

