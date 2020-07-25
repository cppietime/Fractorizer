/**
 * YAAKOV SCHECTMAN 2020
 * prng.h
 * Header file for pseudo-random number generation
 */

#ifndef _H_FRACT_PRNG
#define _H_FRACT_PRNG

#define FRACT_LCG_MASK 0xffffffffUL

#include <stdio.h>
#include <SLAV/slavio.h>

/**
 * A Linear Congruential Generator (LCG) for generating pseudo-random numbers
 * (PRNG)
 *
 * multiplier: a positive integer of form 4 * (2 * K + 1) + 1, i.e. an odd
 * integer N such that N - 1 is divisible by 4 but not by 8
 * adden: a positive odd integer
 * seed: the current state/value of the LCG
 * box_muller: a buffered double for use in the Box-Muller Transform
 * has_muller: a flag indicating whether there is a stored Box-Muller value
 *
 * See `fract_lcg_next` for the use of members
 *
 */
typedef struct _fract_lcg {
	unsigned long multiplier, adden, seed;
	double box_muller;
	char has_muller;
} fract_lcg;

/**
 * Initialize an LCG with provided parameters
 * The way multiplier and adden are calculated from the arguments should ensure
 * a full period.
 * mul_s1d4s1d2: the value of K to use for lcg's multiplier
 * add_s1d2: the value K from which lcg's adden is derived as 2 * K + 1
 * seed: lcg's starting seed
 *
 */
void
fract_lcg_valid_init(
	fract_lcg *lcg,
	unsigned long mul_s1d4s1d2,
	unsigned long add_s1d2,
	unsigned long seed
);

/**
 * Initialize a default LCG based on current system time
 * This default LCG has multiplier = 76541, adden = 12345,
 * seed = 3 * seconds + 5 * nanoseconds.
 * These values ensure a full period.
 */
void
fract_lcg_default(fract_lcg *lcg);

/**
 * Get the next 32-bit unsigned integer value from an LCG
 * This function updates lcg's seed to:
 * (multiplier * seed + adden) & 0xffffffffUL
 * and returns it.
 *
 * returns: the new value of seed
 */
unsigned long
fract_lcg_next(fract_lcg *lcg);

/**
 * Returns a random double uniformly distributed over [0, 1)
 * Internally calls fract_lcg_next and divides by 2 ^ 32
 */
double
fract_lcg_uniform(fract_lcg *lcg);

/**
 * Returns a random double with normal distribution
 * mean = 0, standard deviation = 1
 * This value is derived using the Box-Muller Transform by first generating two
 * uniform values over [0, 1), u and v.
 * v is simply multiplied by 2 * M_PI and stored as theta
 * From u, a value r is computed as:
 * r = sqrt(-2 * log(u))
 * From these values, two normally-distributed values, r*sin(theta), and
 * r*cos(theta) are generated;
 * one of these is returned, while the other is buffered in box_muller for the
 * next call
 */
double
fract_lcg_gaussian(fract_lcg *lcg);

/**
 * Returns a random integer uniformly distributed over [0, lim)
 */
unsigned long
fract_lcg_int(fract_lcg *lcg, unsigned long lim);

/**
 * Randomly permutes a table of integers
 *
 * table: an array of integers, no terminator
 * size: size of table
 */
void
fract_lcg_permute(fract_lcg *lcg, int *table, size_t size);

/**
 * A type storing the parameters for a universal hashing function of integers
 *
 * multiplier and adden are the same as defined for `fract_lcg`
 * modulus: number of possible hash results
 *
 * See `fract_hash_digest` for their use.
 */
typedef struct _fract_hash {
	int multiplier, adden, modulus;
} fract_hash;

/**
 * Initializes a hashing function from an LCG
 *
 * modulus: number of possible hash digests
 */
void
fract_hash_random(fract_hash *hash, fract_lcg *lcg, int modulus);

/**
 * Calculate the hash of x
 * This is calculated as:
 * (multiplier * x + adden) % modulus
 *
 * returns: an integer in [0, modulus) that is deterministic for x
 */
int
fract_hash_digest(fract_hash *hash, int x);

/**
 * A data structure storing parameters for generation of Perlin Noise
 *
 * permutation: a permuted table of indices for coordinate hashing
 * gradients: a table of 2D unit vectors in random directions
 * digest: a hashing function to spatially hash coordinates
 * size: number of indices in permutation and vectors in gradients
 */
typedef struct _fract_perlin {
	int *permutation;
	double *gradients;
	fract_hash digest;
	size_t size;
} fract_perlin;

/**
 * Initialize a Perlin Noise datatype with an LCG
 *
 * size: number of unique unit vectors
 */
void
fract_perlin_init(fract_perlin *perl, fract_lcg *lcg, size_t size);

/**
 * Frees memory associated with a Perlin Noise datatype
 */
void
fract_perlin_destroy(fract_perlin *perl);

/**
 * Retrieve the gradient of a Perlin datatype at point (x, y)
 *
 * dx, dy: pointers in which to store x and y coordinates of gradient
 */
void
fract_perlin_gradient(fract_perlin *perl, int x, int y, double *dx, double *dy);

/**
 * Return the spatial hash of a 2D point for a Perlin datatype
 */
int
fract_perlin_index(fract_perlin *perl, int x, int y);

/**
 * Returns the value of Perlin Noise at the point (x, y)
 */
double
fract_perlin_at(fract_perlin *perl, double x, double y);

/**
 * Retrieve the coordinates of the grid point in a Perlin datatype that most
 * greatly contributes to the value at the point (x, y)
 *
 * ox, oy: pointers in which to store x and y coordinates of grid point
 */
void
fract_perlin_best(fract_perlin *perl, double x, double y, int *ox, int *oy);

/**
 * Generate and store a bitmap of a swirled image generated using Perlin Noise
 * and noise displacement
 *
 * perl: Perlin datatype to generate noise
 * pal_indices: Perlin datatype for spatially hashing palette indices
 * palette: array of 24-bit RGB colors
 * noise_resolution: number of noise grid points in x and y directions
 * swirl_resolution: number of color grid points
 * swirl: strength of noise displacement
 * offset_x, offset_y: 2D offset of noise to use for displacement
 * img: initialized image in which to store swirl
 */
void
fract_perlin_swirl(
	fract_perlin *perl,
	fract_perlin *pal_indices,
	uint32_t *palette,
	double noise_resolution,
	double swirl_resolution,
	double swirl,
	double offset_x, double offset_y,
	Bitmap *img
);

#endif