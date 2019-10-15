#include <assert.h>

#include "swMain.h"

#include "swToolkit.h"
#include "sw.h"

#include "SDL.h"	//what about SDL's 'main' redefinition?
#include "Common/Exception.h"

static int width = 1;
static int height = 1;

static int swutWindowX = 1;
static int swutWindowY = 1;

static std::string swutWindowTitle;

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;
static SDL_Texture* framebuffer = nullptr;

//bitflags of SWUT_ACTIVE_SHIFT, SWUT_ACTIVE_CTRL, SWUT_ACTIVE_ALT
static int swutModifiers = 0;

static void (*swutCallbackDisplay)() = nullptr;
static void (*swutCallbackReshape)(int, int) = nullptr;
static void (*swutCallbackIdle)() = nullptr;
static void (*swutCallbackMouse)(int button, int state, int x, int y) = nullptr;
static void (*swutCallbackMotion)(int x, int y) = nullptr;
static void (*swutCallbackPassiveMotion)(int x, int y) = nullptr;


void swutInit(int *argc, char **argv) {}	//not implemented yet

void swutInitDisplayMode(int flags) {}		//not implemented yet

void swutInitWindowSize(int width_, int height_) {
	width = width_;
	height = height_;
}

void swutInitWindowPosition(int x, int y) {
	swutWindowX = x;
	swutWindowY = y;
}

void swutDisplayFunc(void (*display)()) { swutCallbackDisplay = display; }
void swutReshapeFunc(void (*reshape)(int, int)) { swutCallbackReshape = reshape; }
void swutIdleFunc(void (*idle)()) { swutCallbackIdle = idle; }
void swutMouseFunc(void (*mouse)(int button, int state, int x, int y)) { swutCallbackMouse = mouse; }
void swutMotionFunc(void (*motion)(int x, int y)) { swutCallbackMotion = motion; }
void swutPassiveMotionFunc(void (*passiveMotion)(int x, int y)) { swutCallbackPassiveMotion = passiveMotion; }

void swutCreateWindow(const char *title) {

	int sdlInitError = SDL_Init(SDL_INIT_VIDEO);
	if (sdlInitError) throw Common::Exception() << "SDL_Init failed with error code " << sdlInitError;

#if 0
	SDL_CreateWindowAndRenderer(width, height, 0, &window, &renderer);
	if (!window || !renderer) throw Common::Exception() << "SDL_CreateWindowAndRenderer failed";
#endif
#if 1
	window = SDL_CreateWindow(
		swutWindowTitle.c_str(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width,
		height, 
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
	if (!window) throw Common::Exception() << "SDL_CreateWindow failed";

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	if (!renderer) throw Common::Exception() << "SDL_CreateRenderer failed";
#endif

	framebuffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);

	swWindow_Resize(width, height);

#if 1
	int i = 0;
	for (int y = 0; y < swBufferWidth; ++y) {
		for (int x = 0; x < swBufferHeight; ++x) {
			swBMPBuffer[i++] = rand();
		}
	}
#endif

	swutPostRedisplay();
	swutSetWindowTitle(title);
}

void swutDestroyWindow() {
	if (window) {
		SDL_DestroyWindow(window);
		window = nullptr;
	}
	
	swWindow_Destroy();	
	SDL_Quit();
}


void swutSetWindowTitle(const char *title) {
	swutWindowTitle = title;
	if (window) {
		SDL_SetWindowTitle(window, swutWindowTitle.c_str());
	}
}

void swutMainLoop() {

	bool done = false;
	int captureCount = 0;

	if (swutCallbackReshape) {
		SDL_SetWindowSize(window, width, height);
		swutCallbackReshape(width, height);
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
//std::cout << "SDL_WINDOWEVENT_RESIZED" << std::endl;
					width = event.window.data1;
					height = event.window.data2;

					SDL_DestroyTexture(framebuffer);
					framebuffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
					
					swWindow_Resize(width, height);
					
					swutCallbackReshape(width, height);
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
				//TODO else if no buttons are down ...
				// swutCallbackPassiveMotion

				break;		
		
			case SDL_MOUSEBUTTONUP:
				if (swutCallbackMouse) {
					if (event.button.button == SDL_BUTTON_LEFT) swutCallbackMouse(SWUT_LEFT_BUTTON, SWUT_UP, event.button.x, event.button.y);
					if (event.button.button == SDL_BUTTON_RIGHT) swutCallbackMouse(SWUT_RIGHT_BUTTON, SWUT_UP, event.button.x, event.button.y);
					if (event.button.button == SDL_BUTTON_MIDDLE) swutCallbackMouse(SWUT_MIDDLE_BUTTON, SWUT_UP, event.button.x, event.button.y);
				}
				captureCount--;
				if (!captureCount) SDL_CaptureMouse(SDL_FALSE);
				break;

			case SDL_MOUSEBUTTONDOWN:
				if (!captureCount) SDL_CaptureMouse(SDL_TRUE);
				captureCount++;
				if (swutCallbackMouse) {
					if (event.button.button == SDL_BUTTON_LEFT) swutCallbackMouse(SWUT_LEFT_BUTTON, SWUT_DOWN, event.button.x, event.button.y);
					if (event.button.button == SDL_BUTTON_RIGHT) swutCallbackMouse(SWUT_RIGHT_BUTTON, SWUT_DOWN, event.button.x, event.button.y);
					if (event.button.button == SDL_BUTTON_MIDDLE) swutCallbackMouse(SWUT_MIDDLE_BUTTON, SWUT_DOWN, event.button.x, event.button.y);
				}
				break;
			}
		}

		if (swutCallbackIdle) {
			swutCallbackIdle();
		}
	} while (!done);
}

void swutPostRedisplay() {
	SDL_UpdateTexture(framebuffer, nullptr, swBMPBuffer, width * sizeof (uint32_t));
	SDL_RenderClear(renderer);
	SDL_RenderCopyEx(renderer, framebuffer, nullptr, nullptr, 0., nullptr, SDL_FLIP_VERTICAL);
	SDL_RenderPresent(renderer);
}

int swutGetModifiers() {
	return swutModifiers;
}

void swutSwapBuffers() {
	swutPostRedisplay();
}
