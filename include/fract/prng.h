#ifndef _H_FRACT_PRNG
#define _H_FRACT_PRNG

#define FRACT_LCG_MASK 0xffffffffUL

#include <stdio.h>

/* A linear congruential generator */
typedef struct _fract_lcg {
	unsigned long multiplier, adden, seed;
	double box_muller;
	char has_muller;
} fract_lcg;

void fract_lcg_valid_init(fract_lcg *lcg, unsigned long mul_s1d4s1d2, unsigned long add_s1d2, unsigned long seed); /* Custom LCG */
void fract_lcg_default(fract_lcg *lcg); /* Get default LCG */
unsigned long fract_lcg_next(fract_lcg *lcg); /* Next integer value */
double fract_lcg_uniform(fract_lcg *lcg); /* Double uniformly over [0, 1) */
double fract_lcg_gaussian(fract_lcg *lcg); /* Random normal value with mean=0, standard deviation=1 */
unsigned long fract_lcg_int(fract_lcg *lcg, unsigned long lim); /* Random int uniform over range [0, lim) */
void fract_lcg_permute(fract_lcg *lcg, int *table, size_t size); /* Permute a table with a given size */

/* Universal hashing function */
typedef struct _fract_hash {
	int multiplier, adden, modulus;
} fract_hash;

void fract_hash_random(fract_hash *hash, fract_lcg *lcg, int modulus); /* Initialize a random hash */
int fract_hash_digest(fract_hash *hash, int x); /* Get digest of x */

/* A Perlin noise-generating structure */
typedef struct _fract_perlin {
	int *permutation; /* A table of size */
	double *gradients; /* An array of size 2-vectors */
	fract_hash digest;
	size_t size;
} fract_perlin;

void fract_perlin_init(fract_perlin *perl, fract_lcg *lcg, size_t size); /* Generate a new perlin */
void fract_perlin_destroy(fract_perlin *perl); /* Free it */
void fract_perlin_gradient(fract_perlin *perl, int x, int y, double *dx, double *dy); /* Get the gradient at (x, y) */
double fract_perlin_at(fract_perlin *perl, double x, double y); /* Value at (x, y) */
//TODO add color

#endif