/**
 * YAAKOV SCHECTMAN 2020
 * utils.h
 * Helper functions and macros
 */

#ifndef _H_FRACT_UTILS
#define _H_FRACT_UTILS

#include <stdint.h>
#include <SLAV/slavio.h>

/*
x^3 * (1 - x) + (1 - (1 - x)^3) * x
*/
#define smooth3(x) ((x) * (x) * (3 - 2 * (x)))

/*
a * (1 - z) + b * z
*/
#define lin_interp(a, b, z) ((a) + ((b) - (a)) * (z))

/*
a.x * b.x + a.y * b.y
*/
#define dot2(ax, ay, bx, by) ((ax) * (bx) + (ay) * (by))

/**
 * Return the number of leading zeros in x
 */
int
fract_nlz(uint64_t x);

/**
 * Apply a box blur to img
 */
void
fract_box_blur(Bitmap *img, int x, int y);

#endif