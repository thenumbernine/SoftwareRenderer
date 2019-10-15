#include <iostream>

#include "sw.h"
#include "Q1ModelScene.h"
#include "Texture.h"
#include "resource.h"
#include "Common/Exception.h"

#include <stdlib.h>

#if 0
#include <windows.h>

static HWND controlWnd = NULL;

BOOL WINAPI Q1Model_DlgProc(
	HWND hwnd, 
	UINT msg,
	WPARAM wParam,
	LPARAM lParam)
{
	static Q1ModelScene *scene = NULL;

	switch (msg) {

	case WM_INITDIALOG:
		scene = (Q1ModelScene *)lParam;

		{
			//set all initial buttons

			int activeButtons[] = {
				IDC_RADIO_MDL_CUBE,
				IDC_RADIO_REN_NONE,
				IDC_RADIO_TEX_NONE,
				IDC_RADIO_BOR_U_REP,
				IDC_RADIO_BOR_V_REP,
				IDC_RADIO_BOR_W_REP,
				IDC_RADIO_FIL_NEAR,
			};

			for (int i = 0; i < numberof(activeButtons); i++) {
				SendMessage(GetDlgItem(hwnd, activeButtons[i]), BM_SETCHECK,  1,0);
			}

			//init our textboxes

			for (int i = 0; i < 16; i++) {
				wchar_t buffer[2] = {0,0};

				if (!(i % 5)) {
					buffer[0] = L'1';
				} else {
					buffer[0] = L'0';
				}
				
				SetWindowText(GetDlgItem(hwnd, IDC_EDIT_TM1 + i), buffer);
			}
		}

		return false;

	case WM_COMMAND:
		{
			int wNotifyCode = HIWORD(wParam);
			int wID = LOWORD(wParam);
			HWND hwndCtl = (HWND) lParam;
			switch (wID) {
			case IDC_RADIO_MDL_Q:
			case IDC_RADIO_MDL_CUBE:
			case IDC_RADIO_MDL_CONE:
			case IDC_RADIO_MDL_TORUS:
			case IDC_RADIO_MDL_SPHERE:
				scene->setModelMode(wID);
				break;

			case IDC_RADIO_REN_NONE:
			case IDC_RADIO_REN_LINEAR:
			case IDC_RADIO_REN_CYL:
			case IDC_RADIO_REN_SPH:
			case IDC_RADIO_REN_CEL:
				scene->setRenderMode(wID);
				break;

			case IDC_RADIO_TEX_NONE:
			case IDC_RADIO_TEX_DEFAULT:
			case IDC_RADIO_TEX_CHECKER:
			case IDC_RADIO_TEX_CONTOUR:
			case IDC_RADIO_TEX_CEL:
			case IDC_RADIO_TEX_RGB:
			case IDC_RADIO_TEX_NORMALIZE:
				scene->setTextureMode(wID);
				break;

			case IDC_RADIO_BOR_U_REP:
			case IDC_RADIO_BOR_U_CLMP:
				scene->setBorderMode(0, wID);
				break;

			case IDC_RADIO_BOR_V_REP:
			case IDC_RADIO_BOR_V_CLMP:
				scene->setBorderMode(1, wID);
				break;

			case IDC_RADIO_BOR_W_REP:
			case IDC_RADIO_BOR_W_CLMP:
				scene->setBorderMode(2, wID);
				break;

			case IDC_RADIO_FIL_NEAR:
			case IDC_RADIO_FIL_LIN:
				scene->setFilterMode(wID);
				break;

			case IDC_EDIT_TM1:
			case IDC_EDIT_TM2:
			case IDC_EDIT_TM3:
			case IDC_EDIT_TM4:
			case IDC_EDIT_TM5:
			case IDC_EDIT_TM6:
			case IDC_EDIT_TM7:
			case IDC_EDIT_TM8:
			case IDC_EDIT_TM9:
			case IDC_EDIT_TM10:
			case IDC_EDIT_TM11:
			case IDC_EDIT_TM12:
			case IDC_EDIT_TM13:
			case IDC_EDIT_TM14:
			case IDC_EDIT_TM15:
			case IDC_EDIT_TM16:

				if (wNotifyCode == EN_UPDATE) {
					char buffer[256];

					int index = wID - IDC_EDIT_TM1;	//make sure all these TM's are in order!

					GetWindowText(GetDlgItem(hwnd, wID), buffer, sizeof(buffer));

					float val;
					swscanf(buffer, "%f", &val);
					scene->setTexMat(index, val);
				}
				break;

			}
		}
		break;

	case WM_CLOSE:
		exit(0);	//until I find the appropriate one..
		EndDialog(hwnd, 0);
		controlWnd = NULL;
		PostQuitMessage(0);
		return false;

	}

	return false;
}
#endif

// *************************** MD2SHADER RIP

static unsigned int normalizeCubeMapTex = 0;

static void getCubeVector(int i, int cubesize, int x, int y, float *v) {
	float s, t, sc, tc, mag;

	s = ((float)x + 0.5f) / (float)cubesize;
	t = ((float)y + 0.5f) / (float)cubesize;
	sc = s*2.0f - 1.0f;
	tc = t*2.0f - 1.0f;

	switch (i) {
	case 0:    v[0] = 1.0f;	v[1] = -tc;		v[2] = -sc;		break;
	case 1:    v[0] = -1.0f;	v[1] = -tc;		v[2] = sc;		break;
	case 2:    v[0] = sc;		v[1] = 1.0f;	v[2] = tc;		break;
	case 3:    v[0] = sc;		v[1] = -1.0f;	v[2] = -tc;		break;
	case 4:    v[0] = sc;		v[1] = -tc;		v[2] = 1.0f;	break;
	case 5:    v[0] = -sc;	v[1] = -tc;		v[2] = -1.0f;	break;
	}

	mag = 1.0f/(float)sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
	v[0] *= mag;
	v[1] *= mag;
	v[2] *= mag;
}

/* Initialize a cube map texture object that generates RGB values
 * that when expanded to a [-1,1] range in the register combiners
 * form a normalized vector matching the per-pixel vector used to
 * access the cube map.
 */
void makeNormalizeVectorCubeMap(int size)
{
	float v[3];
	int i, x, y;
	unsigned char *pixels;

	pixels = new unsigned char[size*size*3];

	for (i = 0; i < 6; i++) {
	for (y = 0; y < size; y++) {
		for (x = 0; x < size; x++) {
			getCubeVector(i, size, x, y, v);
			pixels[3*(y*size+x) + 0] = 128 + (unsigned char)(127.f*v[0]);
			pixels[3*(y*size+x) + 1] = 128 + (unsigned char)(127.f*v[1]);
			pixels[3*(y*size+x) + 2] = 128 + (unsigned char)(127.f*v[2]);
		}
	}

	swTexImage2D(SW_TEXTURE_CUBE_MAP_POSITIVE_X + i, //0, GL_RGB8,	//mipmap level, component number
		size, size, /*0, GL_RGB, GL_UNSIGNED_BYTE, */ pixels);			//border, internal(gl) format, external(mem) format
	}

	//in OpenGL these can go before the teximage upload... fixme?
	swTexParameteri(SW_TEXTURE_CUBE_MAP, SW_TEXTURE_WRAP_S, SW_CLAMP_TO_EDGE);
	swTexParameteri(SW_TEXTURE_CUBE_MAP, SW_TEXTURE_WRAP_T, SW_CLAMP_TO_EDGE);
	swTexParameteri(SW_TEXTURE_CUBE_MAP, SW_TEXTURE_MAG_FILTER, SW_LINEAR);
	swTexParameteri(SW_TEXTURE_CUBE_MAP, SW_TEXTURE_MIN_FILTER, SW_LINEAR);

	delete[] pixels;
}

// *************************** END MD2SHADER RIP

void Q1ModelScene::init() {

#if 0	
	//start controls...
	controlWnd = CreateDialogParam(
		GetModuleHandle(NULL),
		MAKEINTRESOURCE(IDD_DIALOG_OPTIONS),
		NULL,
		(DLGPROC)Q1Model_DlgProc,
		(long)this);

	ShowWindow(controlWnd, SW_SHOW);
#endif

	//rgbTex

	const int rgbTex_WIDTH = 2;
	const int rgbTex_HEIGHT = 2;
	const int rgbTex_DEPTH = 2;

	unsigned char rgbTexPixels[3 * rgbTex_WIDTH * rgbTex_HEIGHT * rgbTex_DEPTH];

	for (int i = 0; i < rgbTex_WIDTH; i++) {
		for (int j = 0; j < rgbTex_HEIGHT; j++) {
			for (int k = 0; k < rgbTex_DEPTH; k++) {
				unsigned char *p = &rgbTexPixels[3*(i + rgbTex_WIDTH*(j + rgbTex_HEIGHT*k))];
				p[0] = i * 255 / (rgbTex_WIDTH - 1);
				p[1] = j * 255 / (rgbTex_HEIGHT - 1);
				p[2] = k * 255 / (rgbTex_DEPTH - 1);
			}
		}
	}

	swGenTextures(1, &rgbTex);
	swEnable(SW_TEXTURE_3D);
	swBindTexture(SW_TEXTURE_3D, rgbTex);
	swTexImage3D(SW_TEXTURE_3D, rgbTex_WIDTH, rgbTex_HEIGHT, rgbTex_DEPTH, rgbTexPixels);
	swDisable(SW_TEXTURE_3D);

	//celTex

	const int celTex_TEX_DETAIL = 8;
	unsigned char celTexPixels[3 * celTex_TEX_DETAIL] = {
		0x3f, 0x00, 0x00,
		0x3f, 0x3f, 0x3f,
		0x7f, 0x3f, 0x3f,
		0x7f, 0x7f, 0x7f,
		0xbf, 0x7f, 0x7f,
		0xbf, 0xbf, 0xbf,
		0xff, 0xbf, 0xbf,
		0xff, 0xff, 0xff,
	};

	swGenTextures(1, &celTex);
	swEnable(SW_TEXTURE_2D);
	swBindTexture(SW_TEXTURE_2D, celTex);
	swTexImage2D(SW_TEXTURE_2D, celTex_TEX_DETAIL, 1, celTexPixels);
	swDisable(SW_TEXTURE_2D);

	//contourTex

	const int contourTex_TEX_DETAIL = 8;
	unsigned char contourTexPixels[3 * contourTex_TEX_DETAIL] = {
		0x4f, 0x4f, 0x4f,
		0x4f, 0x4f, 0x4f,
		0x4f, 0x4f, 0x4f,
		0x4f, 0x4f, 0x4f,
		0x4f, 0x4f, 0x4f,
		0x4f, 0x4f, 0x4f,
		0x4f, 0x4f, 0x4f,
		0xff, 0xff, 0xff,
	};

	swGenTextures(1, &contourTex);
	swEnable(SW_TEXTURE_2D);
	swBindTexture(SW_TEXTURE_2D, contourTex);
	swTexImage2D(SW_TEXTURE_2D, contourTex_TEX_DETAIL, 1, contourTexPixels);
	swDisable(SW_TEXTURE_2D);

	//checkerTex

	unsigned char checkerTexPixels[3*2*2] = {
		0x4f, 0x4f, 0x4f, 
		0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 
		0x4f, 0x4f, 0x4f, 
	};

	swGenTextures(1, &checkerTex);
	swEnable(SW_TEXTURE_2D);
	swBindTexture(SW_TEXTURE_2D, checkerTex);
	swTexImage2D(SW_TEXTURE_2D, 2, 2, checkerTexPixels);
	swDisable(SW_TEXTURE_2D);

	//normalization cubemap

	//init the light cubemap texture
    swGenTextures(1, &normalizeCubeMapTex);
	swEnable(SW_TEXTURE_CUBE_MAP);
    swBindTexture(SW_TEXTURE_CUBE_MAP, normalizeCubeMapTex);
    makeNormalizeVectorCubeMap(32);
	swDisable(SW_TEXTURE_CUBE_MAP);

	//quakeMdl

	quakeMdl = new Q1Model();
	quakeMdl->LoadFromFile("data/shambler.mdl");

	//cubeMdl

	cubeMdl = new ObjFile();
	cubeMdl->load("data/cube.obj");

	//coneMdl

	coneMdl = new ObjFile();
	coneMdl->load("data/cone.obj");

	//torusMdl

	torusMdl = new ObjFile();
	torusMdl->load("data/torus.obj");

	//sphereMdl

	sphereMdl = new ObjFile();
	sphereMdl->load("data/sphere.obj");

//	time = clock();
//	frames = 0;
//
////		swClearColor(0,0,0,0);	//clear scene
//	swClearColor(255,0,0,0);
//
//	swEnable(SW_DEPTH_TEST);
}

void Q1ModelScene::shutdown() {

	if (quakeMdl) delete quakeMdl;
	if (cubeMdl) delete cubeMdl;
	if (coneMdl) delete coneMdl;
	if (torusMdl) delete torusMdl;
	if (sphereMdl) delete sphereMdl;
	if (rgbTex) swDeleteTextures(1, &rgbTex);
	if (celTex) swDeleteTextures(1, &celTex);
	if (contourTex) swDeleteTextures(1, &contourTex);
	if (checkerTex) swDeleteTextures(1, &checkerTex);

	clear();
}

void Q1ModelScene::clear() {

	quakeMdl = NULL;
	cubeMdl = NULL;
	coneMdl = NULL;
	torusMdl = NULL;
	sphereMdl = NULL;
	rgbTex = celTex = contourTex = checkerTex = 0;
	controlThreadID = 0;

	modelMode = IDC_RADIO_MDL_CUBE;
	renderMode = IDC_RADIO_REN_NONE;
	textureMode = IDC_RADIO_TEX_NONE;
	borderMode[0] = IDC_RADIO_BOR_U_REP;
	borderMode[1] = IDC_RADIO_BOR_V_REP;
	borderMode[2] = IDC_RADIO_BOR_W_REP;
	filterMode = IDC_RADIO_FIL_NEAR;

	texMat = mat44f();	//ident
}

void Q1ModelScene::update() {
#if 0	
	MSG msg;
	while ( PeekMessage( &msg, controlWnd, 0, 0, PM_REMOVE ) )  {
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
#else
#endif
}
	
void Q1ModelScene::render(const Viewport *viewport, const View *view) {

	frames++;

	clock_t cur = clock();
	float sec = (float)(cur - time) / (float)CLOCKS_PER_SEC;

	float frame = sec * 10.f;

	//mod
	frame /= (float)quakeMdl->getMdl()->numframes;
	frame -= (int)frame;
	frame *= (float)quakeMdl->getMdl()->numframes;

	swRotatef(180,0,0,1);
	swColor3f(1,1,1);


	//TODO - ENCLOSE ALL TEXTURE SETUP IN A TEXTURE UNIT LOOP (put shutdown in a similar loop)
	//hello multitexturing

	//how does multitexture rendering work?
	//have a set of texcoords for each unit
	//interpolate the active ones across the face
	//per color grabbed, process the blend accordingly...

	//setup texture matrix

	swMatrixMode(SW_TEXTURE);
	swLoadIdentity();
	swLoadMatrixf(texMat.fp());
	swMatrixMode(SW_MODELVIEW);

	//setup texture mode

	Texture::unbind();

	unsigned int texTarget = (unsigned int)-1;
	unsigned int texIndex = (unsigned int)-1;

	switch (textureMode) {
	case IDC_RADIO_TEX_CHECKER:	texTarget = SW_TEXTURE_2D;	texIndex = checkerTex;			break;
	case IDC_RADIO_TEX_CONTOUR:	texTarget = SW_TEXTURE_2D;	texIndex = contourTex;			break;
	case IDC_RADIO_TEX_CEL:		texTarget = SW_TEXTURE_2D;	texIndex = celTex;				break;
	case IDC_RADIO_TEX_RGB:		texTarget = SW_TEXTURE_3D;	texIndex = rgbTex;				break;
	case IDC_RADIO_TEX_DEFAULT:	texTarget = SW_TEXTURE_2D;	texIndex = quakeMdl->skin[0];	break;
	case IDC_RADIO_TEX_NORMALIZE: texTarget = SW_TEXTURE_CUBE_MAP; texIndex = normalizeCubeMapTex; break;
//	case IDC_RADIO_TEX_NONE:default:							break;
	}

	if (texTarget != (unsigned int)-1 && texIndex != (unsigned int)-1) {
		swEnable(texTarget);
		swBindTexture(texTarget, texIndex);
	}

	//setup border S

	unsigned int texBorderOp = (unsigned int)-1;

	switch (borderMode[0]) {
	case IDC_RADIO_BOR_U_REP:	texBorderOp = SW_REPEAT; break;
	case IDC_RADIO_BOR_U_CLMP:	texBorderOp = SW_CLAMP; break;
	}

	if (texBorderOp != (unsigned int)-1) {
		swTexParameteri(texTarget, SW_TEXTURE_WRAP_S, texBorderOp);
	} else {
		printf("bad border U method\n");
	}

	//setup border T

	texBorderOp = (unsigned int)-1;

	switch (borderMode[1]) {
	case IDC_RADIO_BOR_V_REP:	texBorderOp = SW_REPEAT; break;
	case IDC_RADIO_BOR_V_CLMP:	texBorderOp = SW_CLAMP; break;
	}

	if (texBorderOp != (unsigned int)-1) {
		swTexParameteri(texTarget, SW_TEXTURE_WRAP_T, texBorderOp);
	} else {
		printf("bad border V method\n");
	}

	//setup border R

	texBorderOp = (unsigned int)-1;

	switch (borderMode[2]) {
	case IDC_RADIO_BOR_W_REP:	texBorderOp = SW_REPEAT; break;
	case IDC_RADIO_BOR_W_CLMP:	texBorderOp = SW_CLAMP; break;
	}

	if (texBorderOp != (unsigned int)-1) {
		swTexParameteri(texTarget, SW_TEXTURE_WRAP_R, texBorderOp);
	} else {
		printf("bad border W method\n");
	}

	//setup filter

	unsigned int texFilter = (unsigned int)-1;
	switch (filterMode) {
	case IDC_RADIO_FIL_NEAR:	texFilter = SW_NEAREST; break;
	case IDC_RADIO_FIL_LIN:		texFilter = SW_LINEAR; break;
	}

	if (texFilter != (unsigned int)-1) {
		swTexParameteri(texTarget, SW_TEXTURE_MAG_FILTER, texFilter);
	} else {
		printf("bad filter method\n");
	}


	//setup render mode

	swEnable(SW_TEXTURE_GEN_R);
	swEnable(SW_TEXTURE_GEN_S);
	swEnable(SW_TEXTURE_GEN_T);

	unsigned int texGenMode = (unsigned int)-1;

	switch(renderMode) {
	case IDC_RADIO_REN_LINEAR:	texGenMode = SW_OBJECT_LINEAR; break;
	case IDC_RADIO_REN_CYL:		texGenMode = SW_OBJECT_CYLINDER; break;
	case IDC_RADIO_REN_SPH:		texGenMode = SW_OBJECT_SPHERE; break;
	case IDC_RADIO_REN_CEL:		texGenMode = SW_NORMAL_MAP; break;
		//todo - introduce SW_REFLECTION_MAP

	case IDC_RADIO_REN_NONE:
	default:
		swDisable(SW_TEXTURE_GEN_R);
		swDisable(SW_TEXTURE_GEN_S);
		swDisable(SW_TEXTURE_GEN_T);
		break;
	}

	if (texGenMode != (unsigned int)-1) {
		swTexGeni(SW_R, SW_TEXTURE_GEN_MODE, texGenMode);
		swTexGeni(SW_S, SW_TEXTURE_GEN_MODE, texGenMode);
		swTexGeni(SW_T, SW_TEXTURE_GEN_MODE, texGenMode);
	}


	//model mode

	switch (modelMode) {
	case IDC_RADIO_MDL_Q:

		swTranslatef(0,0,-20);
		quakeMdl->DisplayInterpolate((int)frame, (int)frame + 1, frame - (int)frame);
		break;

	case IDC_RADIO_MDL_CUBE:

		swScalef(40,40,40);
		cubeMdl->render();
		break;

	case IDC_RADIO_MDL_CONE:

		swScalef(40,40,40);
		coneMdl->render();
		break;

	case IDC_RADIO_MDL_TORUS:

		swScalef(40,40,40);
		torusMdl->render();
		break;

	case IDC_RADIO_MDL_SPHERE:

		swScalef(40,40,40);
		sphereMdl->render();
		break;

	}

	//shutdown render mode

	swDisable(SW_TEXTURE_GEN_R);
	swDisable(SW_TEXTURE_GEN_S);
	swDisable(SW_TEXTURE_GEN_T);

	swDisable(texTarget);

#if 0

////	swTexGeni(SW_S, SW_TEXTURE_GEN_MODE, SW_OBJECT_LINEAR);
////	swTexGeni(SW_T, SW_TEXTURE_GEN_MODE, SW_OBJECT_LINEAR);
////	swTexGeni(SW_R, SW_TEXTURE_GEN_MODE, SW_OBJECT_LINEAR);
//
//	swEnable(SW_TEXTURE_GEN_S);
//	swEnable(SW_TEXTURE_GEN_T);
//	swEnable(SW_TEXTURE_GEN_R);
//	swMatrixMode(SW_TEXTURE);
//	swLoadIdentity();
//	swScalef(0.01f, 0.01f, 0.01f);
//	swMatrixMode(SW_MODELVIEW);


#if 0	//temp 3d texture
	swEnable(SW_TEXTURE_3D);
	swBindTexture(SW_TEXTURE_3D, testTex3D);
	swColor3f(1,1,1);
	quakeMdl->DisplayInterpolate((int)frame, (int)frame + 1, frame - (int)frame);
	swDisable(SW_TEXTURE_3D);
#endif

#if 0	//use texture
	swBindTexture(SW_TEXTURE_2D, quakeMdl->skin[0]);
	swColor3f(1,1,1);
	quakeMdl->DisplayInterpolate((int)frame, (int)frame + 1, frame - (int)frame);
#endif

#if 1	//cel shade

//	//normal.z => texture.r
	swEnable(SW_TEXTURE_GEN_S);
	swTexGeni(SW_S, SW_TEXTURE_GEN_MODE, SW_NORMAL_MAP);
//
//	//texture.r => texture.s
	swMatrixMode(SW_TEXTURE);
	swLoadIdentity();
//	swMultMatrixf(	mat44f(
//		0,0,1,0,
//		0,1,0,0,
//		1,0,0,0,
//		0,0,0,1	).fp());
	swTranslatef(0.5f, 0, 0);
	swScalef(-0.49f, 1, 1);
	swMatrixMode(SW_MODELVIEW);

	Texture::unbind();
	swEnable(SW_TEXTURE_2D);
	swBindTexture(SW_TEXTURE_2D, celTex);
	swColor3f(1,1,1);
	quakeMdl->DisplayInterpolate((int)frame, (int)frame + 1, frame - (int)frame);

	swMatrixMode(SW_TEXTURE);
	swLoadIdentity();
	swMatrixMode(SW_MODELVIEW);
//
	swDisable(SW_TEXTURE_GEN_S);

#endif

#if 0	//display surface normal colored
	swDisable(SW_DEPTH_TEST);
	swEnable(SW_BLEND);
	swDisable(SW_TEXTURE_2D);
	quakeMdl->DisplayInterpolateColorNormal((int)frame, (int)frame + 1, frame - (int)frame);
	swEnable(SW_TEXTURE_2D);
	swEnable(SW_DEPTH_TEST);
	swDisable(SW_BLEND);
#endif

#if 0	//draw wireframe overlay
	swColor3f(1,1,0);
	swDisable(SW_TEXTURE_2D);
	quakeMdl->DisplayInterpolateLines((int)frame, (int)frame + 1, frame - (int)frame);
	swEnable(SW_TEXTURE_2D);
#endif

//	swDisable(SW_TEXTURE_GEN_S);
//	swDisable(SW_TEXTURE_GEN_T);
//	swDisable(SW_TEXTURE_GEN_R);

#endif

}
