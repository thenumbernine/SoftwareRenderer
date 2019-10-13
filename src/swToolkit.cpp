#include <assert.h>

#include "swMain.h"

#include "swToolkit.h"
#include "sw.h"

#include "SDL.h"	//what about SDL's 'main' redefinition?
#include "Common/Exception.h"

static int swutWindowWidth = 1;
static int swutWindowHeight = 1;

static int swutWindowX = 1;
static int swutWindowY = 1;


//bitflags of SWUT_ACTIVE_SHIFT, SWUT_ACTIVE_CTRL, SWUT_ACTIVE_ALT
static int swutModifiers = 0;

//organize these better
//static const char *swutWindowClassName = "SWUTWindowClass";
///*static*/ HWND swutWindow = NULL;
//static WNDCLASSEX swutWindowClass;
static SDL_Window *sdlWindow = nullptr;

static std::string swutWindowTitle;

void swutInit(int *argc, char **argv) {}	//not implemented yet

void swutInitDisplayMode(int flags) {}		//not implemented yet

void swutInitWindowSize(int width, int height) {
	swutWindowWidth = width;
	swutWindowHeight = height;
}

void swutInitWindowPosition(int x, int y) {
	swutWindowX = x;
	swutWindowY = y;
}

static void (*swutCallbackDisplay)() = NULL;
static void (*swutCallbackReshape)(int, int) = NULL;
static void (*swutCallbackIdle)() = NULL;
static void (*swutCallbackMouse)(int button, int state, int x, int y) = NULL;
static void (*swutCallbackMotion)(int x, int y) = NULL;
static void (*swutCallbackPassiveMotion)(int x, int y) = NULL;

void swutDisplayFunc(void (*display)()) { swutCallbackDisplay = display; }
void swutReshapeFunc(void (*reshape)(int, int)) { swutCallbackReshape = reshape; }
void swutIdleFunc(void (*idle)()) { swutCallbackIdle = idle; }
void swutMouseFunc(void (*mouse)(int button, int state, int x, int y)) { swutCallbackMouse = mouse; }
void swutMotionFunc(void (*motion)(int x, int y)) { swutCallbackMotion = motion; }
void swutPassiveMotionFunc(void (*passiveMotion)(int x, int y)) { swutCallbackPassiveMotion = passiveMotion; }

#if 0
static LRESULT CALLBACK SWWindowProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	static int captureCount = 0;

	switch (msg) {

	case WM_CREATE:
		swWindow_Init(hwnd);
		break;

	case WM_DESTROY:
		swWindow_Destroy();
		PostQuitMessage (0);		//now kill the message loop via WM_QUIT message
		return 0;

	case WM_SHOWWINDOW:		//todo - investigate this one.
		if (swutCallbackDisplay) {
			swutCallbackDisplay();
		}

		return 0;

	case WM_PAINT:
		swWindow_Paint();
		return 0;

	case WM_SIZE:

		{
			//get our sizes
			int winWidth = LOWORD(lParam);
			int winHeight = HIWORD(lParam);

			swWindow_Resize(winWidth, winHeight);

			if (swutCallbackReshape) {
				swutCallbackReshape(winWidth, winHeight);
			}

			if (swutCallbackDisplay) {
				swutCallbackDisplay();
			}

		}
		return 0;

	case WM_KEYUP:
		if (wParam == VK_MENU)		swutModifiers &= ~SWUT_ACTIVE_ALT;
		if (wParam == VK_CONTROL)	swutModifiers &= ~SWUT_ACTIVE_CTRL;
		if (wParam == VK_SHIFT)		swutModifiers &= ~SWUT_ACTIVE_SHIFT;
		return 0;

	case WM_KEYDOWN:
		if (wParam == VK_MENU)		swutModifiers |= SWUT_ACTIVE_ALT;
		if (wParam == VK_CONTROL)	swutModifiers |= SWUT_ACTIVE_CTRL;
		if (wParam == VK_SHIFT)		swutModifiers |= SWUT_ACTIVE_SHIFT;
		return 0;

	case WM_MOUSEMOVE:

		//TODO - HOW DO WE LISTEN IN ON THE ALT KEY STATUS?

		if (swutCallbackMotion) {
			swutCallbackMotion(LOWORD(lParam), HIWORD(lParam));
		}

		return 0;

	case WM_LBUTTONUP:
		if (swutCallbackMouse) swutCallbackMouse(SWUT_LEFT_BUTTON, SWUT_UP, LOWORD(lParam), HIWORD(lParam));
		captureCount--;
		if (!captureCount) ReleaseCapture();
		return 0;

	case WM_LBUTTONDOWN:
		if (swutCallbackMouse) swutCallbackMouse(SWUT_LEFT_BUTTON, SWUT_DOWN, LOWORD(lParam), HIWORD(lParam));
		captureCount--;
		if (!captureCount) ReleaseCapture();
		return 0;

	case WM_RBUTTONUP:
		if (swutCallbackMouse) swutCallbackMouse(SWUT_RIGHT_BUTTON, SWUT_UP, LOWORD(lParam), HIWORD(lParam));
		captureCount--;
		if (!captureCount) ReleaseCapture();
		return 0;

	case WM_RBUTTONDOWN:
		if (!captureCount) SetCapture(hwnd);
		captureCount++;
		if (swutCallbackMouse) swutCallbackMouse(SWUT_RIGHT_BUTTON, SWUT_DOWN, LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_MBUTTONUP:
		if (!captureCount) SetCapture(hwnd);
		captureCount++;
		if (swutCallbackMouse) swutCallbackMouse(SWUT_MIDDLE_BUTTON, SWUT_UP, LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_MBUTTONDOWN:
		if (!captureCount) SetCapture(hwnd);
		captureCount++;
		if (swutCallbackMouse) swutCallbackMouse(SWUT_MIDDLE_BUTTON, SWUT_DOWN, LOWORD(lParam), HIWORD(lParam));
		return 0;

	}

	//default windows proc:
	return DefWindowProc (hwnd, msg, wParam, lParam);
}

static bool swutWindow_InitClass() {
	HINSTANCE hInstance = GetModuleHandle(NULL);

	//setup the swutWindowClass
	swutWindowClass.cbSize			= sizeof(WNDCLASSEX);
	swutWindowClass.style			= CS_HREDRAW | CS_VREDRAW;
	swutWindowClass.lpfnWndProc		= SWWindowProc;
	swutWindowClass.cbClsExtra		= 0;
	swutWindowClass.cbWndExtra		= 0;
	swutWindowClass.hInstance		= hInstance;
	swutWindowClass.hIcon			= LoadIcon(NULL, IDI_WINLOGO);
	swutWindowClass.hCursor			= LoadCursor(NULL, IDC_ARROW);
	swutWindowClass.hbrBackground	= NULL;
	swutWindowClass.lpszMenuName	= NULL;
	swutWindowClass.lpszClassName	= swutWindowClassName;
	swutWindowClass.hIconSm			= LoadIcon(NULL, IDI_WINLOGO);

	//register the swutWindowClass
	if (!RegisterClassEx(&swutWindowClass)) {
		MessageBox(NULL, "Failed to Register Window Class", "Error", MB_OK);
		return false;
	}

	return true;
}

static HWND swutWindow_Create(const char *name) {

	HINSTANCE hInstance = GetModuleHandle(NULL);

	//create the window 
	HWND hWnd = CreateWindowEx (
		NULL,
		swutWindowClassName,
		name,
		WS_SIZEBOX | WS_OVERLAPPED | WS_VISIBLE | WS_CAPTION | WS_SYSMENU,
		swutWindowX,
		swutWindowY,
		swutWindowWidth,
		swutWindowHeight,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (!hWnd) {
		MessageBox(NULL, "CreateWindowEx Failed!", "Error", MB_OK);
		return NULL;
	}

	//show the window
	ShowWindow (hWnd, SW_SHOW);
	UpdateWindow (hWnd);

	return hWnd;
}
#endif

void swutCreateWindow(const char *name) {

#if 0
	//first create a window class
	if (!swutWindow_InitClass()) return;

	//resize the window ?
	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.bottom = swutWindowWidth;
	rect.right	= swutWindowHeight;
	if (!AdjustWindowRectEx(&rect, CS_HREDRAW | CS_VREDRAW, true, NULL)) {
		MessageBox(NULL, "Failed to Adjust Window Size", "Error", MB_OK);
		return;
	}

	//next create a window
	swutWindow = swutWindow_Create(name);
	if (!swutWindow) return;
#endif

	int sdlInitError = SDL_Init(SDL_INIT_VIDEO);
	if (sdlInitError) throw Common::Exception() << "SDL_Init failed with error code " << sdlInitError;

	sdlWindow = SDL_CreateWindow(
		swutWindowTitle.c_str(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		swutWindowWidth,
		swutWindowHeight, 
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
	if (!sdlWindow) throw Common::Exception() << "SDL_CreateWindow failed";
}

void swutDestroyWindow() {
#if 0
	assert(swutWindow);
	DestroyWindow(swutWindow);

	UnregisterClass( swutWindowClassName, swutWindowClass.hInstance );
#endif
	if (sdlWindow) {
		SDL_DestroyWindow(sdlWindow);
		sdlWindow = nullptr;
	}
	SDL_Quit();
}


void swutSetWindowTitle(const char *title) {
//	if (!swutWindow) return;
//	SetWindowText(swutWindow, title);
	swutWindowTitle = title;
	if (sdlWindow) {
		SDL_SetWindowTitle(sdlWindow, swutWindowTitle.c_str());
	}
}

void swutMainLoop() {

	bool done = false;
	int captureCount = 0;

#if 0
	//if reshape then do so
	if (swutCallbackReshape) {
		int w = 0, h = 0;
		if (swutWindow) {
			RECT rect;
			GetClientRect(swutWindow, &rect);
			w = rect.right;
			h = rect.bottom;
		}

		swutCallbackReshape(w, h);
	}

	//if display then do so
	if (swutCallbackDisplay) {
		swutCallbackDisplay();
	}

	 //while windows is looping...
	while (!done) {
		MSG msg;
		//cycle through all messages and process them
		while ( PeekMessage( &msg, swutWindow, 0, 0, PM_REMOVE ) )  {
			if( msg.message == WM_QUIT )  {
				done = true;
			} else {
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		}

		if (swutCallbackIdle) {
			swutCallbackIdle();
		}
	}
//#else
//	auxMainLoop(0);//swutAuxDisplayCallback);
//#endif
#endif



	if (swutCallbackReshape) {
		SDL_SetWindowSize(sdlWindow, swutWindowWidth, swutWindowHeight);
		swutCallbackReshape(swutWindowWidth, swutWindowHeight);
	}
	
	if (swutCallbackDisplay) {
		swutCallbackDisplay();
	}

	SDL_Event event;
	do {
		while (SDL_PollEvent(&event) > 0) {
			switch (event.type) {
			case SDL_QUIT:
				done = true;
				break;
			case SDL_WINDOWEVENT:
				switch (event.window.event) {
				case SDL_WINDOWEVENT_RESIZED:
					swutWindowWidth = event.window.data1;
					swutWindowHeight = event.window.data2;
					swutCallbackReshape(swutWindowWidth, swutWindowHeight);
					//fallthrough to display...
				case SDL_WINDOWEVENT_SHOWN:
					if (swutCallbackDisplay) {
						swutCallbackDisplay();
					}
					break;
				}
				break;
			
			case SDL_KEYUP:
				if (event.key.keysym.mod & KMOD_ALT)	swutModifiers &= ~SWUT_ACTIVE_ALT;
				if (event.key.keysym.mod & KMOD_CTRL)	swutModifiers &= ~SWUT_ACTIVE_CTRL;
				if (event.key.keysym.mod & KMOD_SHIFT)	swutModifiers &= ~SWUT_ACTIVE_SHIFT;
				break;

			case SDL_KEYDOWN:
				if (event.key.keysym.mod & KMOD_ALT)	swutModifiers |= SWUT_ACTIVE_ALT;
				if (event.key.keysym.mod & KMOD_CTRL)	swutModifiers |= SWUT_ACTIVE_CTRL;
				if (event.key.keysym.mod & KMOD_SHIFT)	swutModifiers |= SWUT_ACTIVE_SHIFT;
#if PLATFORM_windows
					if (event.key.keysym.sym == SDLK_F4 && (event.key.keysym.mod & KMOD_ALT) != 0) {
						done = true;
					}
#endif
#if PLATFORM_osx
					if (event.key.keysym.sym == SDLK_q && (event.key.keysym.mod & KMOD_GUI) != 0) {
						done = true;
					}
#endif
				break;
			case SDL_MOUSEMOTION:

				//TODO - HOW DO WE LISTEN IN ON THE ALT KEY STATUS?

				if (swutCallbackMotion) {
					swutCallbackMotion(event.motion.x, event.motion.y);
				}

				break;		
		
			case SDL_MOUSEBUTTONUP:
				if (swutCallbackMouse) {
					if (event.button.button == 0) swutCallbackMouse(SWUT_LEFT_BUTTON, SWUT_UP, event.button.x, event.button.y);
					if (event.button.button == 1) swutCallbackMouse(SWUT_RIGHT_BUTTON, SWUT_UP, event.button.x, event.button.y);
					if (event.button.button == 2) swutCallbackMouse(SWUT_MIDDLE_BUTTON, SWUT_UP, event.button.x, event.button.y);
				}
				captureCount--;
				if (!captureCount) SDL_CaptureMouse(SDL_FALSE);
				break;

			case SDL_MOUSEBUTTONDOWN:
				if (!captureCount) SDL_CaptureMouse(SDL_TRUE);
				captureCount++;
				if (swutCallbackMouse) {
					if (event.button.button == 0) swutCallbackMouse(SWUT_LEFT_BUTTON, SWUT_DOWN, event.button.x, event.button.y);
					if (event.button.button == 1) swutCallbackMouse(SWUT_RIGHT_BUTTON, SWUT_DOWN, event.button.x, event.button.y);
					if (event.button.button == 2) swutCallbackMouse(SWUT_MIDDLE_BUTTON, SWUT_DOWN, event.button.x, event.button.y);
				}
				break;
			}
		}

		if (swutCallbackIdle) {
			swutCallbackIdle();
		}

		//if (swap) SDL_GL_SwapWindow(window);
	} while (!done);
}

void swutPostRedisplay() {
//	InvalidateRect(swutWindow, NULL, TRUE);
//	UpdateWindow(swutWindow);
}

int swutGetModifiers() {
	return swutModifiers;
}

void swutSwapBuffers() {
	//force a WM_PAINT
//	InvalidateRect(swutWindow, NULL, TRUE);
//	UpdateWindow(swutWindow);
}
