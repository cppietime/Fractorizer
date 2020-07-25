/**
 * YAAKOV SCHECTMAN 2020
 * ifs.h
 * Types and functions for Iterated Function Systems
 */

#ifndef _H_FRACT_IFS
#define _H_FRACT_IFS

#include <stdio.h>
#include <SLAV/slavio.h>
#include "prng.h"

/**
 * An affine-transform in 2D
 *
 * linear_transform: a 2x2 matrix R (row-major) for the rotation/scale/shear
 *     transformation
 * translation: a 2D vector T for the translation transformation
 *
 * Transforming the point (x, y) by this transformation will yield the point
 * (R[0] * x + R[1] * y + T[0], R[2] * x + R[3] * y + T[1])
 */
typedef struct _fract_affine{
	double linear_transform[4];
	double translation[2];
} fract_affine;

/**
 * Apply affine transformation to (x, y)
 */
void
fract_affine_transform(
	fract_affine affine,
	double x, double y,
	double *ox, double *oy
);

/**
 * A function that transforms (x, y) to V(x, y)
 *
 * affine: transformation coefficients to be used in some functions
 * lcg: PRNG used by some functions for random values
 * parameters: array of 4 doubles used by some functions
 * ox, oy: pointers for output
 */
typedef void (*fract_variation)(
	double x, double y,
	fract_affine affine,
	fract_lcg *lcg,
	double *parameters,
	double *ox, double *oy
);

/**
 * NULL-terminated array of variation functions
 * defined in variations.c
 */
extern fract_variation fract_variations[];

#define VARIATION_DEF(name) void name(double x, double y, \
	fract_affine affine,\
	fract_lcg *lcg,\
	double *parameters,\
	double *ox, double *oy)

VARIATION_DEF(fract_var_linear);
VARIATION_DEF(fract_var_sin);
VARIATION_DEF(fract_var_sphere);
VARIATION_DEF(fract_var_swirl);
VARIATION_DEF(fract_var_horseshoe);
VARIATION_DEF(fract_var_polar);
VARIATION_DEF(fract_var_kerchief);
VARIATION_DEF(fract_var_heart);
VARIATION_DEF(fract_var_disc);
VARIATION_DEF(fract_var_spiral);
VARIATION_DEF(fract_var_hyperbolic);
VARIATION_DEF(fract_var_diamond);
VARIATION_DEF(fract_var_X);
VARIATION_DEF(fract_var_julia);
VARIATION_DEF(fract_var_bent);
VARIATION_DEF(fract_var_waves);
VARIATION_DEF(fract_var_fisheye);
VARIATION_DEF(fract_var_popcorn);
VARIATION_DEF(fract_var_exp);
VARIATION_DEF(fract_var_power);
VARIATION_DEF(fract_var_cos);
VARIATION_DEF(fract_var_rings);
VARIATION_DEF(fract_var_fan);
VARIATION_DEF(fract_var_blob);
VARIATION_DEF(fract_var_pdj);

/**
 * Datatype for system of iterated functions (variations)
 *
 * probability: array of probabilities for each variation, sums to 1
 * affines: array of affine-transforms
 * colors: array of RGB values, ordered:
 *     0.R, 0.G, 0.B, 1.R, 1.G, 1.B, ...
 *     Does not need to sum to 1
 * parameters: parameters for variations
 */
typedef struct _fract_ifs{
	float *probability;
	fract_affine *affines;
	uint8_t *color;
	double *parameters;
} fract_ifs;

/**
 * Initialize an IFS
 *
 * colors: RGB colors to select from
 * num_colors: palette size
 */
void
fract_ifs_init(
	fract_ifs *ifs,
	fract_lcg *lcg,
	uint8_t *colors,
	size_t num_colors
);

/**
 * Free memory allocated for an IFS
 */
void
fract_ifs_destroy(fract_ifs *ifs);

/**
 * Iterate an IFS one time
 *
 * lcg: used for PRNG
 * x, y: input point (x, y)
 * ox, oy: pointer to output (x, y)
 * colors: histogram of colors
 */
void
fract_ifs_iterate(
	fract_ifs *ifs,
	fract_lcg *lcg,
	double x, double y,
	double *ox, double *oy,
	uint8_t *r, uint8_t *g, uint8_t *b
);

/**
 * Generate an IFS flame image
 *
 * x0, y0: coordinates of top-left corner
 * width, height: range of image
 * img: initialized bitmap to store image
 * gamma: gamma correction (> 1)
 * super_sample: amt by which to supersample 1D
 */
void
fract_ifs_flame(
	fract_ifs *ifs,
	fract_lcg *lcg,
	double x0, double y0,
	double width, double height,
	Bitmap *img,
	unsigned long iterations,
	double gamma,
	int super_sample
);

#endif