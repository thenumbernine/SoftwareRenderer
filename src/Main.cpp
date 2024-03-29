#include "Viewport.h"
#include "Scene.h"
#include "Main.h"

#include "sw.h"
#include "swToolkit.h"

#include "Q1ModelScene.h"
#include "Common/Exception.h"
//#include "ImGuiCommon/ImGuiCommon.h"

//#include <stdio.h>
#include <time.h>

#include <vector>
#include <iostream>
#include <memory>
#include <map>

#define MULTIVIEW

/**
 * enable this if you're running with OpenGL on the Dells in Hovland
 */
#undef NEED_UPDATE_TWICE

////////////////// constants

#ifdef MULTIVIEW
enum {
	VIEWPORT_FULL,
	VIEWPORT_UR,
	VIEWPORT_UL,
	VIEWPORT_LR,
	VIEWPORT_LL,
	VIEWPORT_COUNT
};

enum {
	VIEW_XY,
	VIEW_XZ,
	VIEW_YZ,
	VIEW_3D,
	VIEW_COUNT
};
#else
enum {
	VIEWPORT_FULL,
	VIEWPORT_COUNT
};

enum {
	VIEW_3D,
	VIEW_COUNT
};
#endif

////////////////// global variables

static Viewport viewports[VIEWPORT_COUNT];
static View views[VIEW_COUNT];
static std::shared_ptr<Q1ModelScene> scene;

////////////////// screen functions -- will make a new class out of these soon

/**
 * set this variable in order to only update one viewport
 * if this variable is NULL then all viewports are updated
 * after display is called, the variable is reset to NULL
 */
//static Viewport *redisplayViewport = NULL;
static std::vector<Viewport *> activeViewports;

//hack to fix buffer flipping bug on XP systems
#ifdef NEED_UPDATE_TWICE
static bool updateTwice = true;
#endif

static bool fullscreen = true;

//if (fullscreen) then only VIEWPORT_FULL, if (!fullscreen) then add all else...
void screen_restoreViewports() {

	//correct me if i'm wrong, but doesnt .clear() delete elements as it comes to them?
	activeViewports.clear();

	for (Viewport *vp = viewports; vp < endof(viewports); vp++) {
		if ((vp == viewports + VIEWPORT_FULL) == fullscreen) {
			activeViewports.push_back(vp);
		}
	}
}

//find the viewport clicked upon
Viewport *screen_getTopViewAt(int x, int y) {
	for (int j = (int)activeViewports.size() - 1; j >= 0; j--) {
		Viewport *vp = activeViewports[j];
		//see if the view contains the point - last to first, in reverse rendering order
		if (vp->containsPoint(x,y)) return vp;
	}
	return NULL;
}

////////////////// glut callback pointers

void display() {

////FPS update:
	{
		static int frames = 0;
		static clock_t lastSec;

		frames++;
		clock_t thisSec = clock();
		if (thisSec > lastSec + CLOCKS_PER_SEC) {
			std::string s = "Software Renderer - "
				+ std::to_string( (float)(frames * CLOCKS_PER_SEC) / (float)(thisSec - lastSec) )
				+ " FPS"; 
			swutSetWindowTitle(s.c_str());

			lastSec = thisSec;
			frames = 0;
		}
	}

	//perform any once-per-display updates
	scene->update();

#ifdef NEED_UPDATE_TWICE
	for(;;) {
#endif
		for (int i = 0; i < (int)activeViewports.size(); i++) {
			Viewport *vp = activeViewports[i];
			vp->render();
		}

#ifdef SINGLE_BUFFERED
		swFlush();
#else
		swutSwapBuffers();
#endif

#ifdef NEED_UPDATE_TWICE
		if (!updateTwice) break;
		updateTwice = false;
	}
#endif

	scene->updateGUI();
}

static int displayWidth = 1;
static int displayHeight = 1;

void resize(int w, int h) {
	displayWidth = w;
	displayHeight = h;
	int hw = w >> 1;
	int hh = h >> 1;
	viewports[VIEWPORT_FULL].set(0,0,w,h);
#ifdef MULTIVIEW
	viewports[VIEWPORT_LL].set(0, 0, hw, hh);
	viewports[VIEWPORT_LR].set(hw, 0, hw, hh);
	viewports[VIEWPORT_UL].set(0, hh, hw, hh);
	viewports[VIEWPORT_UR].set(hw, hh, hw, hh);
#endif
}

static int mouseLastX, mouseLastY;

/**
 * bitflags for GLUT_***_BUTTON - whether they are down or up
 * written during mouse(), read during motion()
 */
static int mouseButtonFlags = 0;

static Viewport *mouseDownViewport;

static int keyModifiers = 0;

#define DOUBLE_CLICK_TIME_EPSILON	0.3f		//300 ms
#define CLICK_TIME_EPSILON			0.3f		//300 ms

void mouse(int button, int state, int x, int y) {

	//convert coordinate space
	y = displayHeight - y;

	//grab our modifiers upon each mouse press/release (as often as glut will let us)
	//(glut complains when we try to grab modifiers within the motion() callback)
	keyModifiers = swutGetModifiers();

	if (state == SWUT_DOWN) {
		mouseButtonFlags |= bitflag(button);
	} else if (state == SWUT_UP) {
		mouseButtonFlags &= ~(bitflag(button));
	}

	switch (state) {
	case SWUT_DOWN:

		//store the viewport clicked upon - this is what we'll be messin with
		mouseDownViewport = screen_getTopViewAt(x,y);

		break;

	case SWUT_UP:

		//double click to change between fullscreen / multiview mode
		//NOTICE - this will mess up when dealing with cube maps
		{

//	enable this for multiple viewports.
//	it wont look pretty until i implement view clipping planes
#if 0

			static clock_t lastClickClock = 0;
			clock_t thisClock = clock();
			float dt = (float)(thisClock - lastClickClock) / (float)CLOCKS_PER_SEC;
			lastClickClock = thisClock;

			//got a double click
			if (dt < DOUBLE_CLICK_TIME_EPSILON) {

				//if we got a double click then zero lastClickClock
				//so we dont capture the next close click (triple-click)
				lastClickClock = 0;

				//if the full view is empty then turn it on & all else off
				//else turn it off & all else on
				if (!fullscreen) {

					Viewport *vp = screen_getTopViewAt(x,y);

					if (vp) {
						//copy over the views
						viewports[VIEWPORT_FULL].setView(vp->getView());

						fullscreen = true;
						screen_restoreViewports();
					}
				} else {
					//fill activeViewport vector with all but the full view
					fullscreen = false;
					screen_restoreViewports();
				}

			//just any other mouse up
			//during the motion function, the activeViewports vector is reduced to
			//only the viewing object.  so here, restore it to all objects depending upon
			//whether we are in fullscreen or not
			//
			//they need to be restored because when the motion() function transforms our camera
			//it clears the active viewports list of all viewports except the one being manipulated.
			} else
#endif
			{
				screen_restoreViewports();
			}
		}

		break;
	}

#ifdef NEED_UPDATE_TWICE
	updateTwice = true;
#endif
	swutPostRedisplay();

	// store the mouseLast coordinates - to correct our dragging
	mouseLastX = x;
	mouseLastY = y;
}


void motion(int x, int y) {
	
	//convert coordinate space
	y = displayHeight - y;

	//calc our change in screenspace
	int dx = x - mouseLastX;
	int dy = y - mouseLastY;

	//only respond to left-clicks
	if (!(mouseButtonFlags & bitflag(SWUT_LEFT_BUTTON))) return;

	if (mouseDownViewport && mouseDownViewport->getView()) {
		Viewport *vp = mouseDownViewport;
		View *v = vp->getView();

		if (!(keyModifiers & (SWUT_ACTIVE_CTRL | SWUT_ACTIVE_ALT))) {
			//get our rotation axis by the perpendicular vector to our mouse movement in screen space
			quatf rotation = quatExp(vec3f(0, -dy * 0.01, dx * 0.01));
			//convert 'rotation' from screenspace to modelview space (orient via view angle)
			rotation = v->getAngle() * rotation * quatConj(v->getAngle());
			//apply 'rotation' to view, and normalize
			v->setAngle(quatUnit(rotation * v->getAngle()));
			//calculate offset of rotation if we are selected on a shape
			//rotate the position aroud - to keep the object in the same screenspace
			v->setPos(quatRotate(rotation, v->getPos()));
		} else if (!(keyModifiers & SWUT_ACTIVE_ALT)) {
			if (v->getOrtho()) {
				//convert the change in screen space to change in modelview space
				v->setPos(v->getPos() - (vp->getPixelPos(x, y) - vp->getPixelPos(mouseLastX, mouseLastY)));
			} else {
				//convert mouse movement vector to modelspace
				v->setPos(v->getPos() + quatRotate(v->getAngle(), vec3f(0, -dx, -dy) * 0.1f));
			}
		} else {
			if (v->getOrtho()) {	//scale the width in ortho mode
				v->setHalfWidth( v->getHalfWidth() * (float)exp(dy * 0.01f));
			} else {
				//zoom the camera in frustum mode
				//v->setPos( v->getPos() * (float)exp(dy * 0.01f));
				//travel forward camera in frustum mode
				v->setPos( v->getPos() - quatXAxis(v->getAngle()) * ((float)dy * 0.1f));
			}
		}

		//set it as the only to-be-displayed view
		if (activeViewports.size() != 1) {
			activeViewports.clear();
			activeViewports.push_back(vp);
		} else {
			activeViewports[0] = vp;
		}

		swutPostRedisplay();
	}

	mouseLastX = x;
	mouseLastY = y;
}

////////////////// init

static std::map<std::string, std::pair<int Q1ModelScene::*, std::map<std::string, int>>> sceneKVs = {
	{
		"model", 
		{
			&Q1ModelScene::modelMode,
			{
				{"quake", Q1ModelScene::MODEL_QUAKE},
				{"cube", Q1ModelScene::MODEL_CUBE},
				{"cone", Q1ModelScene::MODEL_CONE},
				{"torus", Q1ModelScene::MODEL_TORUS},
				{"sphere", Q1ModelScene::MODEL_SPHERE},
			}
		}
	},
	{
		"render",
		{
			&Q1ModelScene::renderMode,
			{
				{"none", Q1ModelScene::RENDER_NONE},
				{"linear", Q1ModelScene::RENDER_LINEAR},
				{"cylinder", Q1ModelScene::RENDER_CYL},
				{"sphere", Q1ModelScene::RENDER_SPH},
				{"cel", Q1ModelScene::RENDER_CEL},
			}
		}
	},
	{
		"texture",
		{
			&Q1ModelScene::textureMode,
			{
				{"default", Q1ModelScene::TEX_DEFAULT},
				{"checker", Q1ModelScene::TEX_CHECKER},
				{"contour", Q1ModelScene::TEX_CONTOUR},
				{"cel", Q1ModelScene::TEX_CEL},
				{"rgb", Q1ModelScene::TEX_RGB},
				{"normalize", Q1ModelScene::TEX_NORMALIZE},
			}
		}
	},
/*	{
		"border0",
		{
			&Q1ModelScene::borderMode[0],
			{
				{"repeat", BORDER_U_REP},
				{"clamp", BORDER_U_CLMP},
			},
		}
	},
	{
		"border1",
		{
			&Q1ModelScene::borderMode[1],
			{
				{"repeat", BORDER_V_REP},
				{"clamp", BORDER_V_CLMP},
			},
		}
	},
	{
		"border2",
		{
			&Q1ModelScene::borderMode[2],
			{
				{"repeat", BORDER_W_REP},
				{"clamp", BORDER_W_CLMP},
			},
		}
	},
*/	{
		"filter",
		{
			&Q1ModelScene::filterMode,
			{
				{"nearest", Q1ModelScene::FILL_NEAR},
				{"linear", Q1ModelScene::FILL_LIN},
			}
		}
	},
};

void init_scene(const std::vector<std::string>& args) {
	scene = std::make_shared<Q1ModelScene>();

	int n = (int)args.size();
	for (int i = 1; i < n; ++i) {
		const auto& key = args[i];
		if (i < n-1) {
			auto kit = sceneKVs.find(key);
			if (kit != sceneKVs.end()) {
				auto vp = kit->second.first;
				const auto& m = kit->second.second;

				const auto& value = args[++i];
				auto vit = m.find(value);
				if (vit == m.end()) {
					std::cerr << "couldn't find value " << value << " for argument " << key << std::endl;
				} else {
					(*scene).*vp = vit->second;
					continue;
				}
			}
		} else {
			if (key == "-?" || key == "-h" || key == "-help" || key == "--help") {
				std::cout << "command-line options:" << std::endl;
				for (const auto& p : sceneKVs) {
					std::cout << p.first << ": ";
					const char* sep = "";
					for (const auto& q : p.second.second) {
						std::cout << sep << q.first;
						sep = ", ";
					}
					std::cout << std::endl;
				}
				exit(1);
			}
		}
		//} else if (key == "texmat") {
	
		throw Common::Exception() << "unknown cmd-line argument " << key;
	}
}

void init_gl() {

	swClearColor(0,0,0,0);

	//enable depth testing by default
	swEnable(SW_DEPTH_TEST);
}

void init_views() {

	Scene *scene = ::scene.get();

#define _VIEW_INIT_DIST	100
#define _ORTHO_SIZE		50
#define _VIEW_ZNEAR		10.f
#define _VIEW_ZFAR		1000.f

#ifdef MULTIVIEW
	views[VIEW_XY].set(
		scene,
		true, 
		_ORTHO_SIZE,
		_ORTHO_SIZE,
		VIEW_ADJUST_HEIGHT,
		_VIEW_ZNEAR,
		_VIEW_ZFAR,
		vec3f(0,0,_VIEW_INIT_DIST),
		angleAxisToQuat(quatf(0,1,0,90)) * angleAxisToQuat(quatf(1,0,0,-90)));

	views[VIEW_XZ].set(
		scene,
		true, 
		_ORTHO_SIZE,
		_ORTHO_SIZE,
		VIEW_ADJUST_HEIGHT,
		_VIEW_ZNEAR,
		_VIEW_ZFAR,
		vec3f(0,-_VIEW_INIT_DIST,0),
		angleAxisToQuat(quatf(0,0,1,90)));

	views[VIEW_YZ].set(
		scene,
		true, 
		_ORTHO_SIZE,
		_ORTHO_SIZE,
		VIEW_ADJUST_HEIGHT,
		_VIEW_ZNEAR,
		_VIEW_ZFAR,
		vec3f(-_VIEW_INIT_DIST,0,0),
		quatf());
#endif
	views[VIEW_3D].set(
		scene,
		false, 
		_VIEW_ZNEAR,
		_VIEW_ZNEAR,
		VIEW_ADJUST_HEIGHT,
		_VIEW_ZNEAR,
		_VIEW_ZFAR,
//		vec3f(-3.289347f, -1.090798f, -14.594609f),
//		quatf(-0.461630f, 0.474959f, -0.405921f, -0.629714f));
		vec3f(-_VIEW_INIT_DIST,0,0),
		quatf());
}

//only call this after init_views() and scene.init() have been called
void init_viewports() {
	//set all and add non-full to activeViewport
	screen_restoreViewports();

#ifdef MULTIVIEW
	viewports[VIEWPORT_FULL].setView(&views[VIEW_3D]);
	viewports[VIEWPORT_LL].setView(&views[VIEW_XY]);
	viewports[VIEWPORT_UL].setView(&views[VIEW_XZ]);
	viewports[VIEWPORT_UR].setView(&views[VIEW_YZ]);
	viewports[VIEWPORT_LR].setView(&views[VIEW_3D]);
#else
	viewports[VIEWPORT_FULL].setView(&views[VIEW_3D]);
#endif
}

/**
 * four steps:
 *  1) read & store dataset
 *  2) discretize the data
 *  3) generate and color polygons
 *  4) generate & draw contours
 */
void init(const std::vector<std::string>& args) {

	init_gl();
	init_scene(args);
	init_views();
	init_viewports();

	std::cout << std::endl;
	std::cout << "instructions:" << std::endl;
	std::cout << "left click to rotate" << std::endl;
	std::cout << "ctrl + left click to pan" << std::endl;
}

////////////////// shutdown

void shutdown() {
	std::cout << "shutdown system..." << std::endl;
	scene = nullptr;
}

////////////////// main

int main(int argc, char ** argv) {
	std::vector<std::string> args;
	std::copy(argv, argv+argc, std::back_inserter<std::vector<std::string>>(args));

	swutInit(0,0);//&argc, argv);

	swutInitDisplayMode(0);
//#ifdef SINGLE_BUFFERED
//		GLUT_SINGLE
//#else
//		GLUT_DOUBLE
//#endif
//		| GLUT_DEPTH
//		| GLUT_STENCIL
//		| GLUT_RGB
//		| GLUT_ALPHA);

	swutInitWindowSize(360,360);
	swutInitWindowPosition(0,0);
	swutCreateWindow("CS 551 Final");

	swutDisplayFunc(display);
	swutReshapeFunc(resize);
	swutIdleFunc(display);			//in a perfect world i would accept 'display'
	swutMouseFunc(mouse);
	swutMotionFunc(motion);
//	glutKeyboardFunc(keyboard);

	//update loop
	try {
		init(args);
		swutMainLoop();
	} catch (const std::exception& e) {
		std::cerr << "failed with exception " << e.what() << std::endl;
	}

	shutdown();

	swutDestroyWindow();

	return 0;
}
