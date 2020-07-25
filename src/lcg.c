/**
 * YAAKOV SCHECTMAN 2020
 * lcg.c
 */

#include <time.h>
#include <math.h>
#include "prng.h"

enum
{
	MULTIPLIER = 76541,
	ADDEN = 12345,
	MUL_SECS = 3,
	MUL_NANOS = 5
};

void
fract_lcg_valid_init(
	fract_lcg *lcg,
	unsigned long mul_s1d4s1d2,
	unsigned long add_s1d2,
	unsigned long seed)
{
	*lcg = (fract_lcg){
		((mul_s1d4s1d2 * 2 + 1) * 4 + 1) & FRACT_LCG_MASK,
		(add_s1d2 * 2 + 1) & FRACT_LCG_MASK, seed,
		0,
		0};
}

void
fract_lcg_default(fract_lcg *lcg)
{
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	unsigned long seed = ((time.tv_sec * MUL_SECS)
		+ (time.tv_nsec * MUL_NANOS)) & FRACT_LCG_MASK;
	*lcg = (fract_lcg){MULTIPLIER, ADDEN, seed, 0, 0};
}

unsigned long
fract_lcg_next(fract_lcg *lcg)
{
	lcg->seed = (lcg->multiplier * lcg->seed + lcg->adden) & FRACT_LCG_MASK;
	return lcg->seed;
}

double
fract_lcg_uniform(fract_lcg *lcg)
{
	return fract_lcg_next(lcg) / (double)FRACT_LCG_MASK;
}

double
fract_lcg_gaussian(fract_lcg *lcg)
{
	if(lcg->has_muller){
		lcg->has_muller = 0;
		return lcg->box_muller;
	}
	lcg->has_muller = 1;
	double theta = 2 * M_PI * fract_lcg_uniform(lcg);
	double u = fract_lcg_uniform(lcg);
	double r;
	if(u == 0)
		r == 0;
	else
		r = sqrt(-2 * log(u));
	lcg->box_muller = r * sin(theta);
	return r * cos(theta);
}

unsigned long
fract_lcg_int(fract_lcg *lcg, unsigned long lim)
{
	return (unsigned long)(fract_lcg_uniform(lcg) * lim);
}

void
fract_lcg_permute(fract_lcg *lcg, int *table, size_t size)
{
	for(int i = 1; i < size; i++){
		int j = fract_lcg_int(lcg, size);
		int tmp = table[i];
		table[i] = table[j];
		table[j] = tmp;
	}
}
