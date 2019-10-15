#pragma once

//useful macros

/**
 * returns the number of elements in an array
 */
#define numberof(x)	(sizeof((x)) / sizeof((x)[0]))

/**
 * returns the element just after the last element in an array
 * useful for cycling pointers over arrays in for() loops
 */
#define endof(x)	((x) + numberof((x)))

#define bitflag(x)	((1<<(x)))

/**
 * returns the max of the two values
 */
#ifndef max
#define max(x,y)	(((x)>(y)?(x):(y)))
#endif
