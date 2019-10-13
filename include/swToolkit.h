#pragma once

#include <cstdint>

/* swutGetModifiers return mask. */
#define SWUT_ACTIVE_SHIFT               1
#define SWUT_ACTIVE_CTRL                2
#define SWUT_ACTIVE_ALT                 4

/* Mouse buttons. */
#define SWUT_LEFT_BUTTON		0
#define SWUT_MIDDLE_BUTTON		1
#define SWUT_RIGHT_BUTTON		2

/* Mouse button  state. */
#define SWUT_DOWN			0
#define SWUT_UP				1

void swutInit(int *argc, char **argv);	//nothing yet
void swutInitDisplayMode(int flags);		//nothing yet
void swutInitWindowSize(int width, int height);
void swutInitWindowPosition(int x, int y);
void swutCreateWindow(const char *title);
void swutDestroyWindow();
void swutSetWindowTitle(const char *title);
void swutSwapBuffers();

void swutDisplayFunc(void ( *display)());
void swutReshapeFunc(void ( *reshape)(int, int));
void swutIdleFunc(void ( *idle)());
void swutMouseFunc(void ( *mouse)(int button, int state, int x, int y));
void swutMotionFunc(void ( *motion)(int x, int y));
void swutPassiveMotionFunc(void ( *passiveMotion)(int x, int y));


void swutMainLoop();


int swutGetModifiers();

//functionality still in the works:
void swutPostRedisplay();
