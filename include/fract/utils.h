#ifndef _H_FRACT_UTILS
#define _H_FRACT_UTILS

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

#endif